/* B.Shapr
 * Beat / envelope shaper LV2 plugin
 *
 * Copyright (C) 2019 by Sven Jähnichen
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <cstdio>
#include <string>
#include <exception>
#include "BShapr.hpp"

#define LIM(g , min, max) ((g) > (max) ? (max) : ((g) < (min) ? (min) : (g)))
#define SGN(a) ((a) < 0 ? -1 : 1)
#define SQR(a) ((a) * (a))

inline float db2co (const float value) {return pow (10, 0.05 * value);}

inline double floorfrac (const double value) {return value - floor (value);}

AudioBuffer::AudioBuffer () : AudioBuffer (0) {}

AudioBuffer::AudioBuffer (const uint32_t size) : frames (nullptr), wPtr1 (0), wPtr2 (0), rPtr1 (0), rPtr2 (0), size (0)
{
	if (size !=0)
	{
		try {resize (size);}
		catch (std::bad_alloc& ba) {throw ba;}
	}
}

AudioBuffer::~AudioBuffer ()
{
	if (frames) delete[] (frames);
}

void AudioBuffer::resize (const uint32_t size)
{
	if (frames) delete[] (frames);
	frames = nullptr;
	try {frames = new float[size];}
	catch (std::bad_alloc& ba)
	{
		this->size = 0;
		throw ba;
	}

	this->size = size;
}

void AudioBuffer::reset ()
{
	if (frames)
	{
		memset (frames, 0, size * sizeof (float));
		memset (frames, 0, size * sizeof (float));
		wPtr1 = wPtr2 = rPtr1 = rPtr2 = 0;
	}
}

Message::Message () : messageBits (0), scheduled (true) {}

void Message::clearMessages ()
{
	messageBits = 0;
	scheduled = true;
}

void Message::setMessage (MessageNr messageNr)
{
	if ((messageNr > NO_MSG) && (messageNr <= MAX_MSG) && (!isMessage (messageNr)))
	{
		messageBits = messageBits | (1 << (messageNr - 1));
		scheduled = true;
	}
}

void Message::deleteMessage (MessageNr messageNr)
{
	if ((messageNr > NO_MSG) && (messageNr <= MAX_MSG) && (isMessage (messageNr)))
	{
		messageBits = messageBits & (~(1 << (messageNr - 1)));
		scheduled = true;
	}
}

bool Message::isMessage (MessageNr messageNr)
{
	if ((messageNr > NO_MSG) && (messageNr <= MAX_MSG)) return ((messageBits & (1 << (messageNr - 1))) != 0);
	else if (messageNr == NO_MSG) return (messageBits == 0);
	else return false;
}

MessageNr Message::loadMessage ()
{
	scheduled = false;
	for (int i = NO_MSG + 1; i <= MAX_MSG; ++i)
	{
		MessageNr messageNr = MessageNr (i);
		if (isMessage (messageNr)) return messageNr;
	}
	return NO_MSG;
}

bool Message::isScheduled () {return scheduled;}

BShapr::BShapr (double samplerate, const LV2_Feature* const* features) :
	map(NULL),
	rate(samplerate), bpm(120.0f), speed(1), bar (0), barBeat (0), beatsPerBar (4), beatUnit (4),
	position(0), offset(0), refFrame(0),
	audioInput1(NULL), audioInput2(NULL), audioOutput1(NULL), audioOutput2(NULL),
	sendValue{0xFF},
	new_controllers {NULL}, controllers {0},
	shapes {SmoothShape<MAXNODES> ()}, tempNodes {StaticArrayList<Node, MAXNODES> ()},
	urids (), controlPort(NULL), notifyPort(NULL), forge (), notify_frame (),
	key (0xFF),
	ui_on(false), message (), monitorPos(-1), notificationsCount(0), stepCount (0),
	scheduleNotifyStatus (true)

{
	for (int i = 0; i < MAXSHAPES; ++i)
	{
		shapes[i].setDefaultShape ();
		shapes[i].setTransformation (methods[0].transformFactor, methods[0].transformOffset);

		try {audioBuffer1[i].resize (samplerate);}
		catch (std::bad_alloc& ba) {throw ba;}

		try {audioBuffer2[i].resize (samplerate);}
		catch (std::bad_alloc& ba) {throw ba;}
	}
	notifications.fill ({0.0f, {0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}});
	fillFilterBuffer (filter1Buffer1, 0);
	fillFilterBuffer (filter1Buffer2, 0);
	fillFilterBuffer (filter2Buffer1, 0);
	fillFilterBuffer (filter2Buffer2, 0);

	//Scan host features for URID map
	LV2_URID_Map* m = NULL;
	for (int i = 0; features[i]; ++i)
	{
		if (strcmp(features[i]->URI, LV2_URID__map) == 0)
		{
			m = (LV2_URID_Map*) features[i]->data;
		}
	}
	if (!m) throw std::invalid_argument ("Host does not support urid:map");

	//Map URIS
	map = m;
	mapURIDs (m, &urids);

	// Initialize forge
	lv2_atom_forge_init (&forge, map);

	for (int i = 0; i < MAXSHAPES; ++i) scheduleNotifyShapes[i] = true;
}

BShapr::~BShapr () {}

void BShapr::connect_port(uint32_t port, void *data)
{
	switch (port) {
	case CONTROL:
		controlPort = (LV2_Atom_Sequence*) data;
		break;
	case NOTIFY:
		notifyPort = (LV2_Atom_Sequence*) data;
		break;
	case AUDIO_IN_1:
		audioInput1 = (float*) data;
		break;
	case AUDIO_IN_2:
		audioInput2 = (float*) data;
		break;
	case AUDIO_OUT_1:
		audioOutput1 = (float*) data;
		break;
	case AUDIO_OUT_2:
		audioOutput2 = (float*) data;
		break;
	default:
		if ((port >= CONTROLLERS) && (port < CONTROLLERS + NR_CONTROLLERS)) new_controllers[port - CONTROLLERS] = (float*) data;
	}
}

void BShapr::fillFilterBuffer (float filterBuffer[MAXSHAPES] [MAX_F_ORDER / 2], const float value)
{
	for (int i = 0; i < MAXSHAPES; ++ i)
	{
		for (int j = 0; j < MAX_F_ORDER / 2; ++ j)
		{
			filterBuffer[i][j] = value;
		}
	}
}

bool BShapr::isAudioOutputConnected (int shapeNr)
{
	if (controllers[SHAPERS + shapeNr * SH_SIZE + SH_OUTPUT] != 0) return true;

	for (int i = shapeNr + 1; i < MAXSHAPES; ++i)
	{
		bool status = false;
		if (controllers[SHAPERS + i * SH_SIZE + SH_INPUT] == shapeNr + 3) status = isAudioOutputConnected (i);
		if (status) {return true;}
	}
	return false;
}

double BShapr::getPositionFromBeats (double beats)
{
	if (controllers[BASE_VALUE] == 0.0) return 0.0;

	switch (int (controllers[BASE]))
	{
		case SECONDS: 	return (bpm ? beats / (controllers[BASE_VALUE] * (bpm / 60.0)) : 0.0);
		case BEATS:	return beats / controllers[BASE_VALUE];
		case BARS:	return (beatsPerBar ? beats / (controllers[BASE_VALUE] * beatsPerBar) : 0.0);
		default:	return 0.0;
	}
}

double BShapr::getPositionFromFrames (uint64_t frames)
{
	if ((controllers[BASE_VALUE] == 0.0) || (rate == 0)) return 0.0;

	switch (int (controllers[BASE]))
	{
		case SECONDS: 	return frames * (1.0 / rate) / controllers[BASE_VALUE] ;
		case BEATS:	return (bpm ? frames * (speed / (rate / (bpm / 60))) / controllers[BASE_VALUE] : 0.0);
		case BARS:	return (bpm && beatsPerBar ? frames * (speed / (rate / (bpm / 60))) / (controllers[BASE_VALUE] * beatsPerBar) : 0.0);
		default:	return 0.0;
	}
}

double BShapr::getPositionFromSeconds (double seconds)
{
	if (controllers[BASE_VALUE] == 0.0) return 0.0;

	switch (int (controllers[BASE]))
	{
		case SECONDS :	return seconds / controllers[BASE_VALUE];
		case BEATS:	return seconds * (bpm / 60.0) / controllers[BASE_VALUE];
		case BARS:	return (beatsPerBar ? seconds * (bpm / 60.0 / beatsPerBar) / controllers[BASE_VALUE] : 0.0);
		default:	return 0;
	}
}

void BShapr::run (uint32_t n_samples)
{
	// Check ports
	if ((!controlPort) || (!notifyPort) || (!audioInput1) || (!audioInput2) || (!audioOutput1) || (!audioOutput2)) return;

	for (int i = 0; i < NR_CONTROLLERS; ++i) if (!new_controllers[i]) return;

	// Prepare forge buffer and initialize atom sequence
	const uint32_t space = notifyPort->atom.size;
	lv2_atom_forge_set_buffer(&forge, (uint8_t*) notifyPort, space);
	lv2_atom_forge_sequence_head(&forge, &notify_frame, 0);

	// Update controller values
	for (int i = 0; i < NR_CONTROLLERS; ++i)
	{
		if (controllers[i] != *new_controllers[i])
		{
			float newValue = *new_controllers[i];
			int shapeNr = ((i >= SHAPERS) ? ((i - SHAPERS) / SH_SIZE) : -1);
			int shapeControllerNr = ((i >= SHAPERS) ? ((i - SHAPERS) % SH_SIZE) : -1);

			// Global controllers
			if (i < SHAPERS)
			{
				newValue = globalControllerLimits[i].validate (newValue);

				if (i == MIDI_CONTROL)
				{
					if (newValue == 0.0f)
					{
						// Hard set position back to offset-independent position
						position = floorfrac (position + offset);
						offset = 0;
					}

					else key = 0xFF;
				}

				else if (i == BASE)
				{
					if (newValue == SECONDS)
					{
						if (bpm < 1.0) message.setMessage (JACK_STOP_MSG);
						else message.deleteMessage (JACK_STOP_MSG);
					}
					else
					{
						if ((speed == 0) || (bpm < 1.0)) message.setMessage (JACK_STOP_MSG);
						else message.deleteMessage (JACK_STOP_MSG);
					}
				}
			}

			// Shape controllers
			else
			{
				newValue = shapeControllerLimits[shapeControllerNr].validate (newValue);

				// Target
				if (shapeControllerNr == SH_TARGET)
				{
					// Change transformation
					shapes[shapeNr].setTransformation (methods[int(newValue)].transformFactor, methods[int(newValue)].transformOffset);

					// Clear audiobuffers, if needed
					if
					(
						(newValue == BShaprTargetIndex::PITCH) ||
						(newValue == BShaprTargetIndex::DELAY) ||
						(newValue == BShaprTargetIndex::DOPPLER)
					)
					{
						audioBuffer1[shapeNr].reset ();
						audioBuffer2[shapeNr].reset ();
					}
				}

				else if (shapeControllerNr == SH_SMOOTHING)
				{
					shapes[shapeNr].setSmoothing (getPositionFromSeconds (newValue / 1000));
				}

				// Options
				else if ((shapeControllerNr >= SH_OPTION) && (shapeControllerNr < SH_OPTION + MAXOPTIONS))
				{
					int optionNr = shapeControllerNr - SH_OPTION;
					newValue = options[optionNr].limit.validate (newValue);
				}
			}

			controllers[i] = newValue;

			// Re-smoothing required ?
			if ((i == BASE) || (i == BASE_VALUE))
			{
				for (int j = 0; j < MAXSHAPES; ++j)
				{
					double smoothing = getPositionFromSeconds (controllers[SHAPERS + j * SH_SIZE + SH_SMOOTHING] / 1000);
					shapes[j].setSmoothing (smoothing);
				}
			}
		}
	}

	// Check for waiting tempNodes
	for (int i = 0; i < MAXSHAPES; ++i)
	{
		while (!tempNodes[i].empty())
		{
			Node n = tempNodes[i].back();
			shapes[i].insertNode (n);
			tempNodes[i].pop_back();
		}
	}

	// Check activeShape input
	int activeShape = LIM (controllers[ACTIVE_SHAPE], 1, MAXSHAPES) - 1;
	if (controllers[SHAPERS + activeShape * SH_SIZE + SH_INPUT] == 0) message.setMessage (NO_INPUT_MSG);
	else message.deleteMessage (NO_INPUT_MSG);

	// Check activeShape output
	if (!isAudioOutputConnected (activeShape)) message.setMessage (NO_OUTPUT_MSG);
	else message.deleteMessage (NO_OUTPUT_MSG);

	// Control and MIDI messages
	uint32_t last_t = 0;
	LV2_ATOM_SEQUENCE_FOREACH(controlPort, ev)
	{
		// Read host & GUI events
		if ((ev->body.type == urids.atom_Object) || (ev->body.type == urids.atom_Blank))
		{
			const LV2_Atom_Object* obj = (const LV2_Atom_Object*)&ev->body;

			// Process GUI on status data
			if (obj->body.otype == urids.ui_on)
			{
				ui_on = true;
				for (int i = 0; i < MAXSHAPES; ++i) scheduleNotifyShapes[i] = true;
			}

			// Process GUI off status data
			else if (obj->body.otype == urids.ui_off) ui_on = false;

			// Process (full) shape data
			else if (obj->body.otype == urids.notify_shapeEvent)
			{
				LV2_Atom *sNr = NULL, *sData = NULL;
				lv2_atom_object_get
				(
					obj,
					urids.notify_shapeNr, &sNr,
					urids.notify_shapeData, &sData,
					NULL
				);

				if (sNr && (sNr->type == urids.atom_Int) &&
					sData && (sData->type == urids.atom_Vector))
				{
					int shapeNr = ((LV2_Atom_Int*)sNr)->body;

					if ((shapeNr >= 0) && (shapeNr < MAXSHAPES))
					{
						const LV2_Atom_Vector* vec = (const LV2_Atom_Vector*) sData;
						size_t vecSize = (sData->size - sizeof(LV2_Atom_Vector_Body)) / (7 * sizeof (float));
						if (vec->body.child_type == urids.atom_Float)
						{
							shapes[shapeNr].clearShape ();
							float* data = (float*)(&vec->body + 1);
							for (unsigned int nodeNr = 0; (nodeNr < vecSize) && (nodeNr < MAXNODES); ++nodeNr)
							{
								Node node (&data[nodeNr * 7]);
								shapes[shapeNr].appendRawNode (node);
							}
							shapes[shapeNr].validateShape();
						}
					}
				}
			}

			// Process time / position data
			else if (obj->body.otype == urids.time_Position)
			{
				bool scheduleUpdatePosition = false;

				// Update bpm, speed, position
				LV2_Atom *oBbeat = NULL, *oBpm = NULL, *oSpeed = NULL, *oBpb = NULL, *oBu = NULL, *oBar = NULL;
				const LV2_Atom_Object* obj = (const LV2_Atom_Object*)&ev->body;
				lv2_atom_object_get
				(
					obj, urids.time_bar, &oBar,
					urids.time_barBeat, &oBbeat,
					urids.time_beatsPerMinute,  &oBpm,
					urids.time_beatsPerBar,  &oBpb,
					urids.time_beatUnit,  &oBu,
					urids.time_speed, &oSpeed,
					NULL
				);

				// BPM changed?
				if (oBpm && (oBpm->type == urids.atom_Float))
				{
					float nbpm = ((LV2_Atom_Float*)oBpm)->body;

					if (nbpm != bpm)
					{
						bpm = nbpm;

						if (nbpm < 1.0)
						{
							message.setMessage (JACK_STOP_MSG);
							fillFilterBuffer (filter1Buffer1, 0);
							fillFilterBuffer (filter1Buffer2, 0);
							fillFilterBuffer (filter2Buffer1, 0);
							fillFilterBuffer (filter2Buffer2, 0);
						}

						else message.deleteMessage (JACK_STOP_MSG);
					}
				}

				// Beats per bar changed?
				if (oBpb && (oBpb->type == urids.atom_Float) && (((LV2_Atom_Float*)oBpb)->body > 0))
				{
					beatsPerBar = ((LV2_Atom_Float*)oBpb)->body;
					scheduleNotifyStatus = true;
				}

				// BeatUnit changed?
				if (oBu && (oBu->type == urids.atom_Int) && (((LV2_Atom_Int*)oBu)->body > 0))
				{
					beatUnit = ((LV2_Atom_Int*)oBu)->body;
					scheduleNotifyStatus = true;
				}

				// Speed changed?
				if (oSpeed && (oSpeed->type == urids.atom_Float))
				{
					float nspeed = ((LV2_Atom_Float*)oSpeed)->body;

					if (nspeed != speed)
					{

						if (controllers[BASE] != SECONDS)
						{

							// Started ?
							if (speed == 0)
							{
								for (int i = 0; i < MAXSHAPES; ++i)
								{
									audioBuffer1[i].reset ();
									audioBuffer2[i].reset ();
								}
							}

							// Stopped ?
							if (nspeed == 0)
							{
								message.setMessage (JACK_STOP_MSG);
								fillFilterBuffer (filter1Buffer1, 0);
								fillFilterBuffer (filter1Buffer2, 0);
								fillFilterBuffer (filter2Buffer1, 0);
								fillFilterBuffer (filter2Buffer2, 0);
							}

							// Not stopped ?
							else message.deleteMessage (JACK_STOP_MSG);
						}

						speed = nspeed;
					}
				}

				// Bar position changed
				if (oBar && (oBar->type == urids.atom_Long) && (bar != ((uint64_t)((LV2_Atom_Long*)oBar)->body)))
				{
					bar = ((LV2_Atom_Long*)oBar)->body;
					scheduleUpdatePosition = true;
				}

				// Beat position changed (during playing) ?
				if (oBbeat && (oBbeat->type == urids.atom_Float))
				{
					barBeat = ((LV2_Atom_Float*)oBbeat)->body;
					scheduleUpdatePosition = true;
				}

				if (scheduleUpdatePosition)
				{
					// Hard set new position if new data received
					double pos = getPositionFromBeats (barBeat + beatsPerBar * bar);
					position = floorfrac (pos - offset);
					refFrame = ev->time.frames;
				}
			}
		}

		// Read incoming MIDI events
		if (ev->body.type == urids.midi_Event)
		{
			const uint8_t* const msg = (const uint8_t*)(ev + 1);

			// Forward MIDI event
			/*LV2_Atom midiatom;
			midiatom.type = urids.midi_Event;
			midiatom.size = 3;

			lv2_atom_forge_frame_time (&forge, ev->time.frames);
			lv2_atom_forge_raw (&forge, &midiatom, sizeof (LV2_Atom));
			lv2_atom_forge_raw (&forge, msg, 3);
			lv2_atom_forge_pad (&forge, sizeof (LV2_Atom) + 3);*/

			// Analyze MIDI event
			if (controllers[MIDI_CONTROL] == 1.0f)
			{
				uint8_t typ = lv2_midi_message_type(msg);
				// uint8_t chn = msg[0] & 0x0F;
				uint8_t note = msg[1];
				uint32_t filter = controllers[MIDI_KEYS];

				switch (typ)
				{
					case LV2_MIDI_MSG_NOTE_ON:
					{
						if (filter & (1 << (note % 12)))
						{
							key = note;
							offset = position;
							position = 0;
							refFrame = ev->time.frames;
						}
					}
					break;

					case LV2_MIDI_MSG_NOTE_OFF:
					{
						if (key == note)
						{
							key = 0xFF;
						}
					}
					break;

					case LV2_MIDI_MSG_CONTROLLER:
					{
						if ((note == LV2_MIDI_CTL_ALL_NOTES_OFF) ||
						    (note == LV2_MIDI_CTL_ALL_SOUNDS_OFF))
						{
							key = 0xFF;
						}
					}
					break;

					default: break;
				}
			}
		}

		uint32_t next_t = (ev->time.frames < n_samples ? ev->time.frames : n_samples);
		play (last_t, next_t);
		last_t = next_t;
	}

	// Play remaining samples
	if (last_t < n_samples) play (last_t, n_samples);

	// Update position in case of no new barBeat submitted on next call
	double relpos = getPositionFromFrames (n_samples - refFrame);	// Position relative to reference frame
	position = floorfrac (position + relpos);
	refFrame = 0;

	// Send collected data to GUI
	if (ui_on)
	{
		notifyMonitorToGui ();
		for (int i = 0; i < MAXSHAPES; ++i) if (scheduleNotifyShapes[i]) notifyShapeToGui (i);
		if (message.isScheduled ()) notifyMessageToGui ();
		if (scheduleNotifyStatus) notifyStatusToGui ();
	}
	lv2_atom_forge_pop (&forge, &notify_frame);
}

void BShapr::notifyMonitorToGui()
{
	if (notificationsCount > 0)
	{
		if (notificationsCount > NOTIFYBUFFERSIZE) notificationsCount = NOTIFYBUFFERSIZE;
		LV2_Atom_Forge_Frame frame;
		lv2_atom_forge_frame_time(&forge, 0);
		lv2_atom_forge_object(&forge, &frame, 0, urids.notify_monitorEvent);
		lv2_atom_forge_key(&forge, urids.notify_monitor);
		lv2_atom_forge_vector(&forge, sizeof(float), urids.atom_Float, (uint32_t) (9 * notificationsCount), &notifications);
		lv2_atom_forge_pop(&forge, &frame);

		memset (&notifications, 0, notificationsCount * sizeof (BShaprNotifications));
		notificationsCount = 0;
		stepCount = 0;
	}
}

void BShapr::notifyShapeToGui (int shapeNr)
{
	size_t size = shapes[shapeNr].size ();

	// Load shapeBuffer
	for (unsigned int i = 0; i < size; ++i)
	{
		Node node = shapes[shapeNr].getRawNode (i);
		shapeBuffer[i * 7] = (float)node.nodeType;
		shapeBuffer[i * 7 + 1] = (float)node.point.x;
		shapeBuffer[i * 7 + 2] = (float)node.point.y;
		shapeBuffer[i * 7 + 3] = (float)node.handle1.x;
		shapeBuffer[i * 7 + 4] = (float)node.handle1.y;
		shapeBuffer[i * 7 + 5] = (float)node.handle2.x;
		shapeBuffer[i * 7 + 6] = (float)node.handle2.y;
	}

	// Notify shapeBuffer
	LV2_Atom_Forge_Frame frame;
	lv2_atom_forge_frame_time(&forge, 0);
	lv2_atom_forge_object(&forge, &frame, 0, urids.notify_shapeEvent);
	lv2_atom_forge_key(&forge, urids.notify_shapeNr);
	lv2_atom_forge_int(&forge, shapeNr);
	lv2_atom_forge_key(&forge, urids.notify_shapeData);
	lv2_atom_forge_vector(&forge, sizeof(float), urids.atom_Float, (uint32_t) (7 * size), &shapeBuffer);
	lv2_atom_forge_pop(&forge, &frame);

	scheduleNotifyShapes[shapeNr] = false;
}

void BShapr::notifyMessageToGui()
{
	uint32_t messageNr = message.loadMessage ();

	// Send notifications
	LV2_Atom_Forge_Frame frame;
	lv2_atom_forge_frame_time(&forge, 0);
	lv2_atom_forge_object(&forge, &frame, 0, urids.notify_messageEvent);
	lv2_atom_forge_key(&forge, urids.notify_message);
	lv2_atom_forge_int(&forge, messageNr);
	lv2_atom_forge_pop(&forge, &frame);
}

void BShapr::notifyStatusToGui()
{
	// Send notifications
	LV2_Atom_Forge_Frame frame;
	lv2_atom_forge_frame_time(&forge, 0);
	lv2_atom_forge_object(&forge, &frame, 0, urids.notify_statusEvent);
	lv2_atom_forge_key(&forge, urids.time_beatsPerBar);
	lv2_atom_forge_float(&forge, beatsPerBar);
	lv2_atom_forge_key(&forge, urids.time_beatUnit);
	lv2_atom_forge_int(&forge, beatUnit);
	lv2_atom_forge_key(&forge, urids.time_beatsPerMinute);
	lv2_atom_forge_float(&forge, bpm);
	lv2_atom_forge_pop(&forge, &frame);

	scheduleNotifyStatus = false;
}

void BShapr::audioLevel (const float input1, const float input2, float* output1, float* output2, const float amp)
{
	*output1 = input1 * LIM (amp, methods[LEVEL].limit.min, methods[LEVEL].limit.max);
	*output2 = input2 * LIM (amp, methods[LEVEL].limit.min, methods[LEVEL].limit.max);
}

void BShapr::stereoBalance (const float input1, const float input2, float* output1, float* output2, const float balance)
{
	float f = LIM (balance, methods[BALANCE].limit.min, methods[BALANCE].limit.max);
	if (f < 0)
	{
		*output1 = input1 + (0 - f) * input2;
		*output2 = (f + 1) * input2;
	}

	else
	{
		*output1 = (1 - f) * input1;
		*output2 = input2 + f * input1;
	}
}

void BShapr::stereoWidth (const float input1, const float input2, float* output1, float* output2, const float width)
{
	float f = LIM (width, methods[WIDTH].limit.min, methods[WIDTH].limit.max);
	float m = (input1 + input2) / 2;
	float s = (input1 - input2) * f / 2;

	*output1 = m + s;
	*output2 = m - s;
}

// Butterworth algorithm
void BShapr::lowPassFilter (const float input1, const float input2, float* output1, float* output2, const float cutoffFreq, const int shape)
{
	int order = controllers[SHAPERS + shape * SH_SIZE + SH_OPTION + DB_PER_OCT_OPT] / 6;
	float f = LIM (cutoffFreq, methods[LOW_PASS].limit.min, methods[LOW_PASS].limit.max);
	float a = tan (M_PI * f / rate);
	float a2 = a * a;
	float coeff0 [MAX_F_ORDER / 2];
	float coeff1 [MAX_F_ORDER / 2];
	float coeff2 [MAX_F_ORDER / 2];
	float filter1Buffer0 [MAX_F_ORDER / 2];
	float filter2Buffer0 [MAX_F_ORDER / 2];

	for (int i = 0; i < int (order / 2); ++i)
	{
		float r = sin (M_PI * (2.0f * i + 1.0f) / (2.0f * order));
		float s = a2 + 2.0f * a * r + 1.0f;
		coeff0[i] = a2 / s;
		coeff1[i] = 2.0f * (1 - a2) / s;
		coeff2[i] = -(a2 - 2.0f * a * r + 1.0f) / s;
	}

	double f1 = input1;
	double f2 = input2;
	for (int i = 0; i < int (order / 2); ++i)
	{
		filter1Buffer0[i] = coeff1[i] * filter1Buffer1[shape][i] + coeff2[i] * filter1Buffer2[shape][i] + f1;
		filter2Buffer0[i] = coeff1[i] * filter2Buffer1[shape][i] + coeff2[i] * filter2Buffer2[shape][i] + f2;
		f1 = coeff0[i] * (filter1Buffer0[i] + 2.0f * filter1Buffer1[shape][i] + filter1Buffer2[shape][i]);
		f2 = coeff0[i] * (filter2Buffer0[i] + 2.0f * filter2Buffer1[shape][i] + filter2Buffer2[shape][i]);
		filter1Buffer2[shape][i] = filter1Buffer1[shape][i];
		filter1Buffer1[shape][i] = filter1Buffer0[i];
		filter2Buffer2[shape][i] = filter2Buffer1[shape][i];
		filter2Buffer1[shape][i] = filter2Buffer0[i];
	}

	*output1 = f1;
	*output2 = f2;
}

// Butterworth algorithm
void BShapr::highPassFilter (const float input1, const float input2, float* output1, float* output2, const float cutoffFreq, const int shape)
{
	int order = controllers[SHAPERS + shape * SH_SIZE + SH_OPTION + DB_PER_OCT_OPT] / 6;
	float f = LIM (cutoffFreq, methods[HIGH_PASS].limit.min, methods[HIGH_PASS].limit.max);
	float a = tan (M_PI * f / rate);
	float a2 = a * a;
	float coeff0 [MAX_F_ORDER / 2];
	float coeff1 [MAX_F_ORDER / 2];
	float coeff2 [MAX_F_ORDER / 2];
	float filter1Buffer0 [MAX_F_ORDER / 2];
	float filter2Buffer0 [MAX_F_ORDER / 2];

	for (int i = 0; i < int (order / 2); ++i)
	{
		float r = sin (M_PI * (2.0f * i + 1.0f) / (2.0f * order));
		float s = a2 + 2.0f * a * r + 1.0f;
		coeff0[i] = 1 / s;
		coeff1[i] = 2.0f * (1 - a2) / s;
		coeff2[i] = -(a2 - 2.0f * a * r + 1.0f) / s;
	}

	double f1 = input1;
	double f2 = input2;
	for (int i = 0; i < int (order / 2); ++i)
	{
		filter1Buffer0[i] = coeff1[i] * filter1Buffer1[shape][i] + coeff2[i] * filter1Buffer2[shape][i] + f1;
		filter2Buffer0[i] = coeff1[i] * filter2Buffer1[shape][i] + coeff2[i] * filter2Buffer2[shape][i] + f2;
		f1 = coeff0[i] * (filter1Buffer0[i] - 2.0f * filter1Buffer1[shape][i] + filter1Buffer2[shape][i]);
		f2 = coeff0[i] * (filter2Buffer0[i] - 2.0f * filter2Buffer1[shape][i] + filter2Buffer2[shape][i]);
		filter1Buffer2[shape][i] = filter1Buffer1[shape][i];
		filter1Buffer1[shape][i] = filter1Buffer0[i];
		filter2Buffer2[shape][i] = filter2Buffer1[shape][i];
		filter2Buffer1[shape][i] = filter2Buffer0[i];
	}

	*output1 = f1;
	*output2 = f2;
}

// Ring buffer method with least squares ring closure
void BShapr::pitch (const float input1, const float input2, float* output1, float* output2, const float semitone, const int shape)
{
	const int pitchBufferSize = rate * PITCHBUFFERTIME / 1000;
	const int pitchFaderSize = rate * PITCHFADERTIME / 1000;
	const float p  = LIM (semitone, methods[PITCH].limit.min, methods[PITCH].limit.max);
	const double pitchFactor = pow (2, p / 12);
	const uint32_t wPtr = audioBuffer1[shape].wPtr1;
	const double rPtr = audioBuffer1[shape].rPtr1;
	const uint32_t rPtrInt = uint32_t (rPtr);
	const double rPtrFrac = fmod (rPtr, 1);
	double diff = rPtr - wPtr;
	if (diff > pitchBufferSize / 2) diff = diff - pitchBufferSize;
	if (diff < -pitchBufferSize / 2) diff = diff + pitchBufferSize;

	// Write to buffers and output
	audioBuffer1[shape].frames[wPtr % pitchBufferSize] = input1;
	audioBuffer2[shape].frames[wPtr % pitchBufferSize] = input2;
	*output1 = (1 - rPtrFrac) * audioBuffer1[shape].frames[rPtrInt % pitchBufferSize] +
						 rPtrFrac * audioBuffer1[shape].frames[(rPtrInt + 1) % pitchBufferSize];
	*output2 = (1 - rPtrFrac) * audioBuffer2[shape].frames[rPtrInt % pitchBufferSize] +
						 rPtrFrac * audioBuffer2[shape].frames[(rPtrInt + 1) % pitchBufferSize];

	// Update pointers
 	const double newWPtr = (wPtr + 1) % pitchBufferSize;
 	double newRPtr = fmod (rPtr + pitchFactor, pitchBufferSize);

 	double newDiff = newRPtr - newWPtr;
 	if (newDiff > pitchBufferSize / 2) newDiff = newDiff - pitchBufferSize;
 	if (newDiff < -pitchBufferSize / 2) newDiff = newDiff + pitchBufferSize;

	// Run into new data area on positive pitch or
	// run into old data area on negative pitch => find best point to continue
	if (((diff < 0) && (newDiff >= 0) && (p > 0)) ||
			((diff >= 1) && (newDiff < 1) && (p < 0)))
	{
		int sig = (p > 0 ? -1 : 1);
		double bestOverlayScore = 9999;
		int bestI = 0;

		// Calulate slopes for the reference sample points
		double slope11[P_ORDER];
		double slope12[P_ORDER];
		for (int j = 0; j < P_ORDER; ++j)
		{
			double jpos = double (pitchBufferSize * (1 << j)) / 1000;
			uint32_t jptr = rPtrInt + pitchBufferSize + sig * jpos;
			slope11[j] = audioBuffer1[shape].frames[(jptr + 1) % pitchBufferSize] -
									 audioBuffer1[shape].frames[jptr % pitchBufferSize];
			slope12[j] = audioBuffer2[shape].frames[(jptr + 1) % pitchBufferSize] -
									 audioBuffer2[shape].frames[jptr % pitchBufferSize];
		}

		// Iterate through the buffer to find the best match
		for (int i = pitchFaderSize + 1; i < pitchBufferSize - pitchFaderSize; ++i)
		{
			double posDiff1 = audioBuffer1[shape].frames[rPtrInt % pitchBufferSize] - audioBuffer1[shape].frames[(rPtrInt + i) % pitchBufferSize];
			double posDiff2 = audioBuffer2[shape].frames[rPtrInt % pitchBufferSize] - audioBuffer2[shape].frames[(rPtrInt + i) % pitchBufferSize];
			double overlayScore = SQR (posDiff1) + SQR (posDiff2);

			for (int j = 0; j < P_ORDER; ++j)
			{
				if (overlayScore > bestOverlayScore) break;

				double jpos = double (pitchBufferSize * (1 << j)) / 1000;
				uint32_t jptr = rPtrInt + pitchBufferSize + i + sig * jpos;
				double slope21 = audioBuffer1[shape].frames[(jptr + 1) % pitchBufferSize] -
												 audioBuffer1[shape].frames[jptr % pitchBufferSize];
				double slope22 = audioBuffer2[shape].frames[(jptr + 1) % pitchBufferSize] -
												 audioBuffer2[shape].frames[jptr % pitchBufferSize];
				double slopeDiff1 = slope11[j] - slope21;
				double slopeDiff2 = slope12[j] - slope22;
				overlayScore += SQR (slopeDiff1) + SQR (slopeDiff2);
			}

			if (overlayScore < bestOverlayScore)
			{
				bestI = i;
				bestOverlayScore = overlayScore;
			}
		}

		newRPtr = fmod (rPtr + bestI + pitchFactor, pitchBufferSize);
	}

	audioBuffer1[shape].wPtr1 = newWPtr;
	audioBuffer1[shape].rPtr1 = newRPtr;
	audioBuffer2[shape].wPtr1 = newWPtr;
	audioBuffer2[shape].rPtr1 = newRPtr;
}

// Ring buffer method with least squares ring closure
void BShapr::delay (const float input1, const float input2, float* output1, float* output2, const float delaytime, const int shape)
{
	const int audioBufferSize = rate;
	const int delayBufferSize = rate * DELAYBUFFERTIME / 1000;
	float param = LIM (delaytime, methods[DELAY].limit.min, methods[DELAY].limit.max) * rate / 1000;
	const int delayframes = LIM (param, 0, audioBufferSize);

	const uint32_t wPtr = uint32_t (audioBuffer1[shape].wPtr1) % audioBufferSize;
	const uint32_t rPtr1 = uint32_t (audioBuffer1[shape].rPtr1) % audioBufferSize;
	const uint32_t rPtr2 = uint32_t (audioBuffer1[shape].rPtr2) % audioBufferSize;
	const int diff = (rPtr2 > rPtr1 ? rPtr2 - rPtr1 : rPtr2 + audioBufferSize - rPtr1);

	// Write to buffers and output
	audioBuffer1[shape].frames[wPtr] = input1;
	audioBuffer2[shape].frames[wPtr] = input2;
	*output1 = audioBuffer1[shape].frames[rPtr2];
	*output2 = audioBuffer2[shape].frames[rPtr2];

	// Update pointers
	uint32_t newRPtr1 = rPtr1;
	uint32_t newRPtr2 = rPtr2;

	// End of block? Find best point to continue.
	if (diff >= delayBufferSize)
	{
		double bestOverlayScore = 9999;
		int bestI = 0;

		// Calulate slopes for the reference sample points
		double slope11[P_ORDER];
		double slope12[P_ORDER];
		for (int j = 0; j < P_ORDER; ++j)
		{
			double jpos = double (delayBufferSize * (1 << j)) / 1000;
			uint32_t jptr = rPtr2 + audioBufferSize - jpos;
			slope11[j] = audioBuffer1[shape].frames[(jptr + 1) % audioBufferSize] -
									 audioBuffer1[shape].frames[jptr % audioBufferSize];
			slope12[j] = audioBuffer2[shape].frames[(jptr + 1) % audioBufferSize] -
									 audioBuffer2[shape].frames[jptr % audioBufferSize];
		}

		// Iterate through the buffer to find the best match
		for (int i = 0; (i < delayBufferSize) && (i < delayframes); ++i)
		{
			int32_t iPtr = (wPtr + 2 * audioBufferSize - delayframes - i) % audioBufferSize;
			double posDiff1 = audioBuffer1[shape].frames[rPtr2] - audioBuffer1[shape].frames[iPtr];
			double posDiff2 = audioBuffer2[shape].frames[rPtr2] - audioBuffer2[shape].frames[iPtr];
			double overlayScore = SQR (posDiff1) + SQR (posDiff2);

			for (int j = 0; j < P_ORDER; ++j)
			{
				if (overlayScore > bestOverlayScore) break;

				double jpos = double (delayBufferSize * (1 << j)) / 1000;
				uint32_t jptr = iPtr + audioBufferSize - jpos;
				double slope21 = audioBuffer1[shape].frames[(jptr + 1) % audioBufferSize] -
												 audioBuffer1[shape].frames[jptr % audioBufferSize];
				double slope22 = audioBuffer2[shape].frames[(jptr + 1) % audioBufferSize] -
												 audioBuffer2[shape].frames[jptr % audioBufferSize];
				double slopeDiff1 = slope11[j] - slope21;
				double slopeDiff2 = slope12[j] - slope22;
				overlayScore += SQR (slopeDiff1) + SQR (slopeDiff2);
			}

			if (overlayScore < bestOverlayScore)
			{
				bestI = i;
				bestOverlayScore = overlayScore;
			}
		}

		newRPtr1 = (wPtr + 2 * audioBufferSize - delayframes - bestI) % audioBufferSize;
		newRPtr2 = newRPtr1;
	}

	// Write back pointers
	audioBuffer1[shape].wPtr1 = (wPtr + 1) % audioBufferSize;
	audioBuffer2[shape].wPtr1 = audioBuffer1[shape].wPtr1;
	audioBuffer1[shape].rPtr1 = newRPtr1;
	audioBuffer2[shape].rPtr1 = newRPtr1;
	audioBuffer1[shape].rPtr2 = (newRPtr2 + 1) % audioBufferSize;
	audioBuffer2[shape].rPtr2 = audioBuffer1[shape].rPtr2;
}

// Delay with Doppler effect
void BShapr::doppler (const float input1, const float input2, float* output1, float* output2, const float delaytime, const int shape)
{
	const int audioBufferSize = rate;
	float param = LIM (delaytime, methods[DELAY].limit.min, methods[DELAY].limit.max) * rate / 1000;
	const float delayframes = LIM (param, 0, audioBufferSize);

	const uint32_t wPtr = uint32_t (audioBuffer1[shape].wPtr1) % audioBufferSize;
	const uint32_t rPtrInt = uint32_t (audioBuffer1[shape].rPtr1) % audioBufferSize;
	const double rPtrFrac = fmod (audioBuffer1[shape].rPtr1, 1);

	// Write to buffers and output
	audioBuffer1[shape].frames[wPtr] = input1;
	audioBuffer2[shape].frames[wPtr] = input2;
	*output1 = (1 - rPtrFrac) * audioBuffer1[shape].frames[rPtrInt] +
						 rPtrFrac * audioBuffer1[shape].frames[(rPtrInt + 1) % audioBufferSize];
	*output2 = (1 - rPtrFrac) * audioBuffer2[shape].frames[rPtrInt] +
						 rPtrFrac * audioBuffer2[shape].frames[(rPtrInt + 1) % audioBufferSize];

	// Update pointers
	audioBuffer1[shape].wPtr1 = (wPtr + 1) % audioBufferSize;
	audioBuffer2[shape].wPtr1 = audioBuffer1[shape].wPtr1;
	audioBuffer1[shape].rPtr1 = fmod (audioBuffer1[shape].wPtr1 + audioBufferSize - delayframes, audioBufferSize);
	audioBuffer2[shape].rPtr1 = audioBuffer1[shape].rPtr1;
}

void BShapr::distortion (const float input1, const float input2, float* output1, float* output2, const int mode, const float drive, const float limit)
{
	const float f = db2co (LIM (drive, methods[DISTORTION].limit.min, methods[DISTORTION].limit.max));
	const float l = db2co (LIM (limit, options[LIMIT_DB_OPT].limit.min, options[LIMIT_DB_OPT].limit.max));
	double i1 = input1 * f / l;
	double i2 = input2 * f / l;

	switch (mode)
	{
		case HARDCLIP:
			*output1 = LIM (l * i1, -l, l);
			*output2 = LIM (l * i2, -l, l);
			break;

		case SOFTCLIP:
			*output1 = SGN (i1) * l * sqrt (SQR (i1) / (1 + SQR (i1)));
			*output2 = SGN (i2) * l * sqrt (SQR (i2) / (1 + SQR (i2)));
			break;

		case FOLDBACK:
			*output1 = (fabs (i1) <= 1 ? l * i1 : (SGN (i1) * l * double (2 * (int ((abs (i1) + 1) / 2) % 2) - 1) * (1.0 - fmod (fabs (i1) + 1, 2))));
			*output2 = (fabs (i2) <= 1 ? l * i2 : (SGN (i2) * l * double (2 * (int ((abs (i2) + 1) / 2) % 2) - 1) * (1.0 - fmod (fabs (i2) + 1, 2))));
			break;

		case OVERDRIVE:
			*output1 = ((fabs (i1) < (1.0/3.0)) ? (2.0 * l * i1) : ((fabs (i1) < (2.0/3.0)) ? (SGN (i1) * l * (3.0 - SQR (2.0 - 3.0 * fabs (i1))) / 3.0) : l * SGN (i1)));
			*output2 = ((fabs (i2) < (1.0/3.0)) ? (2.0 * l * i2) : ((fabs (i2) < (2.0/3.0)) ? (SGN (i2) * l * (3.0 - SQR (2.0 - 3.0 * fabs (i2))) / 3.0) : l * SGN (i2)));
			break;

		case FUZZ:
			*output1 = SGN (i1) * l * (1 - exp (- fabs (i1)));
			*output2 = SGN (i2) * l * (1 - exp (- fabs (i2)));
			break;

		default:
			*output1 = input1;
			*output2 = input2;
			break;
	}
}

void BShapr::decimate (const float input1, const float input2, float* output1, float* output2, const float hz, const int shape)
{
	const double f = LIM (hz, methods[DECIMATE].limit.min, methods[DECIMATE].limit.max);
	if (decimateCounter[shape] + 1 >= double (rate) / f)
	{
		decimateBuffer1[shape] = input1;
		decimateBuffer2[shape] = input2;
		float c0 = double (rate) / f - decimateCounter[shape];
		decimateCounter[shape] = (c0 > 0 ? c0 : 0);
	}

	else decimateCounter[shape]++;

	*output1 = decimateBuffer1[shape];
	*output2 = decimateBuffer2[shape];
}

void BShapr::bitcrush (const float input1, const float input2, float* output1, float* output2, const float bitNr)
{
	const double f = pow (2, LIM (bitNr, methods[BITCRUSH].limit.min, methods[BITCRUSH].limit.max) - 1);
	const int64_t bits1 = round (double (input1) * f);
	const int64_t bits2 = round (double (input2) * f);
	*output1 = double (bits1) / f;
	*output2 = double (bits2) / f;
}

void BShapr::sendMidi (const float input1, const float input2, float* output1, float* output2, const uint8_t midiCh, const uint8_t midiCC, const float amp, uint32_t frames, const int shape)
{
	*output1 = input1;
	*output2 = input2;

	uint8_t newValue = amp * 128;
	newValue = LIM (newValue, 0, 127);

	if (newValue != sendValue[shape])
	{
		LV2_Atom midiatom;
		midiatom.type = urids.midi_Event;
		midiatom.size = 3;

		uint8_t startCh = (midiCh == 0 ? 0 : midiCh - 1);
		uint8_t endCh = (midiCh == 0 ? 15 : midiCh - 1);

		for (uint8_t ch = startCh; ch <= endCh; ++ ch)
		{
			uint8_t status = LV2_MIDI_MSG_CONTROLLER + ch;
			uint8_t msg[3] = {status, midiCC, newValue};

			lv2_atom_forge_frame_time (&forge, frames);
			lv2_atom_forge_raw (&forge, &midiatom, sizeof (LV2_Atom));
			lv2_atom_forge_raw (&forge, &msg, 3);
			lv2_atom_forge_pad (&forge, sizeof (LV2_Atom) + 3);
		}
		sendValue[shape] = newValue;
	}
}

void BShapr::play (uint32_t start, uint32_t end)
{
	if (end < start) return;

	// Return if halted or bpm == 0
	if (((speed == 0.0f) && (controllers[BASE] != SECONDS)) || (bpm < 1.0f))
	{
		memset(&audioOutput1[start], 0, (end - start) * sizeof(float));
		memset(&audioOutput2[start], 0, (end - start) * sizeof(float));
		return;
	}

	for (uint32_t i = start; i < end; ++i)
	{
		// Interpolate position within the loop
		double relpos = getPositionFromFrames (i - refFrame);	// Position relative to reference frame
		double pos = floorfrac (position + relpos);		// 0..1 position

		float output1 = 0;
		float output2 = 0;

		// Bypass
		if (controllers[BYPASS] != 0.0f)
		{
			output1 = audioInput1[i];
			output2 = audioInput2[i];
		}

		// Audio calculations only if MIDI-independent or key pressed
		else if ((controllers[MIDI_CONTROL] == 0.0f) || (key != 0xFF))
		{
			float input1;
			float input2;
			float shapeOutput1[MAXSHAPES];
			memset (shapeOutput1, 0, MAXSHAPES * sizeof (float));
			float shapeOutput2[MAXSHAPES];
			memset (shapeOutput2, 0, MAXSHAPES * sizeof (float));

			for (int sh = 0; sh < MAXSHAPES; ++sh)
			{
				if (controllers[SHAPERS + sh * SH_SIZE + SH_INPUT] != BShaprInputIndex::OFF)
				{
					// Connect to shaper input
					switch (int (controllers[SHAPERS + sh * SH_SIZE + SH_INPUT]))
					{
						case BShaprInputIndex::AUDIO_IN:
							input1 = audioInput1[i] * controllers[SHAPERS + sh * SH_SIZE + SH_INPUT_AMP];
							input2 = audioInput2[i] * controllers[SHAPERS + sh * SH_SIZE + SH_INPUT_AMP];
							break;

						case BShaprInputIndex::CONSTANT:
							input1 = controllers[SHAPERS + sh * SH_SIZE + SH_INPUT_AMP];
							input2 = controllers[SHAPERS + sh * SH_SIZE + SH_INPUT_AMP];
							break;

						default:
							if ((controllers[SHAPERS + sh * SH_SIZE + SH_INPUT] >= BShaprInputIndex::OUTPUT) &&
										(controllers[SHAPERS + sh * SH_SIZE + SH_INPUT] < BShaprInputIndex::OUTPUT + MAXSHAPES))
							{
								int inputSh = controllers[SHAPERS + sh * SH_SIZE + SH_INPUT] - BShaprInputIndex::OUTPUT;
								input1 = shapeOutput1[inputSh] * controllers[SHAPERS + sh * SH_SIZE + SH_INPUT_AMP];
								input2 = shapeOutput2[inputSh] * controllers[SHAPERS + sh * SH_SIZE + SH_INPUT_AMP];
							}
							else
							{
								input1 = 0;
								input2 = 0;
							}
					}

					// Get shaper value for the actual position
					float iFactor = shapes[sh].getSmoothMapValue (pos);

					float drywet = controllers[SHAPERS + sh * SH_SIZE + SH_DRY_WET];
					float wet1 = 0;
					float wet2 = 0;

					// Apply shaper on target
					switch (int (controllers[SHAPERS + sh * SH_SIZE + SH_TARGET]))
					{
						case BShaprTargetIndex::LEVEL:
							audioLevel (input1, input2, &wet1, &wet2, iFactor);
							break;

						case BShaprTargetIndex::GAIN:
							audioLevel (input1, input2, &wet1, &wet2, db2co (LIM (iFactor, methods[GAIN].limit.min, methods[GAIN].limit.max)));
							break;

						case BShaprTargetIndex::BALANCE:
							stereoBalance (input1, input2, &wet1, &wet2, iFactor);
							break;

						case BShaprTargetIndex::WIDTH:
							stereoWidth (input1, input2, &wet1, &wet2, iFactor);
							break;

						case BShaprTargetIndex::LOW_PASS:
							lowPassFilter (input1, input2, &wet1, &wet2, iFactor, sh);
							break;

						case BShaprTargetIndex::LOW_PASS_LOG:
							lowPassFilter (input1, input2, &wet1, &wet2, pow (10, LIM (iFactor, methods[LOW_PASS_LOG].limit.min, methods[LOW_PASS_LOG].limit.max)), sh);
							break;

						case BShaprTargetIndex::HIGH_PASS:
							highPassFilter (input1, input2, &wet1, &wet2, iFactor, sh);
							break;

						case BShaprTargetIndex::HIGH_PASS_LOG:
							highPassFilter (input1, input2, &wet1, &wet2, pow (10, LIM (iFactor, methods[HIGH_PASS_LOG].limit.min, methods[HIGH_PASS_LOG].limit.max)), sh);
							break;

						case BShaprTargetIndex::PITCH:
							pitch (input1, input2, &wet1, &wet2, iFactor, sh);
							break;

						case BShaprTargetIndex::DELAY:
							delay (input1, input2, &wet1, &wet2, iFactor, sh);
							break;

						case BShaprTargetIndex::DOPPLER:
							doppler (input1, input2, &wet1, &wet2, iFactor, sh);
							break;

						case BShaprTargetIndex::DISTORTION:
							distortion
							(
								input1, input2, &wet1, &wet2,
								controllers[SHAPERS + sh * SH_SIZE + SH_OPTION + DISTORTION_OPT],
								iFactor,
								controllers[SHAPERS + sh * SH_SIZE + SH_OPTION + LIMIT_DB_OPT]
							);
							break;

						case BShaprTargetIndex::DECIMATE:
							decimate (input1, input2, &wet1, &wet2, iFactor, sh);
							break;

						case BShaprTargetIndex::BITCRUSH:
							bitcrush (input1, input2, &wet1, &wet2, iFactor);
							break;

						case BShaprTargetIndex::SEND_MIDI:
							sendMidi
							(
								input1, input2, &wet1, &wet2,
								controllers[SHAPERS + sh * SH_SIZE + SH_OPTION + SEND_MIDI_CH],
								controllers[SHAPERS + sh * SH_SIZE + SH_OPTION + SEND_MIDI_CC],
								iFactor, i, sh
							);
							break;
					}

					shapeOutput1[sh] = (1 - drywet) * input1 + drywet * wet1;
					shapeOutput2[sh] = (1 - drywet) * input2 + drywet * wet2;

					if (controllers[SHAPERS + sh * SH_SIZE + SH_OUTPUT] == BShaprOutputIndex::AUDIO_OUT)
					{
						output1 += shapeOutput1[sh] * controllers[SHAPERS + sh * SH_SIZE + SH_OUTPUT_AMP];
						output2 += shapeOutput2[sh] * controllers[SHAPERS + sh * SH_SIZE + SH_OUTPUT_AMP];
					}
				}
			}
		}

		// Analyze input and output data for GUI notification
		if (ui_on)
		{
			// Calculate position in monitor
			int newMonitorPos = pos * MONITORBUFFERSIZE;
			unsigned int nr = notificationsCount % NOTIFYBUFFERSIZE;

			notifications[nr].position = newMonitorPos;

			// Position changed? => Next nr
			if (newMonitorPos != monitorPos)
			{
				++notificationsCount;
				stepCount = 0;
				nr = notificationsCount % NOTIFYBUFFERSIZE;
				memset(&notifications[nr], 0, sizeof (BShaprNotifications));
				monitorPos = newMonitorPos;
			}

			++stepCount;
			float fstep = 1 / stepCount;
			float fprev = (stepCount - 1) * fstep;

			if (audioInput1[i] < 0) notifications[nr].input1.min = fprev * notifications[nr].input1.min + fstep * audioInput1[i];
			else notifications[nr].input1.max = fprev * notifications[nr].input1.max + fstep * audioInput1[i];
			if (output1 < 0) notifications[nr].output1.min = fprev * notifications[nr].output1.min + fstep * output1;
			else notifications[nr].output1.max = fprev * notifications[nr].output1.max + fstep * output1;
			if (audioInput2[i] < 0) notifications[nr].input2.min = fprev * notifications[nr].input2.min + fstep * audioInput2[i];
			else notifications[nr].input2.max = fprev * notifications[nr].input2.max + fstep * audioInput2[i];
			if (output2 < 0) notifications[nr].output2.min = fprev * notifications[nr].output2.min + fstep * output2;
			else notifications[nr].output2.max = fprev * notifications[nr].output2.max + fstep * output2;
		}

		// Store in audio out
		audioOutput1[i] = audioInput1[i] * (1 - controllers[DRY_WET]) + output1 * controllers[DRY_WET];
		audioOutput2[i] = audioInput2[i] * (1 - controllers[DRY_WET]) + output2 * controllers[DRY_WET];
	}
}

LV2_State_Status BShapr::state_save (LV2_State_Store_Function store, LV2_State_Handle handle, uint32_t flags,
			const LV2_Feature* const* features)
{
	char shapesDataString[0x8010] = "Shape data:\n";

	for (unsigned int sh = 0; sh < MAXSHAPES; ++sh)
	{
		for (unsigned int nd = 0; nd < shapes[sh].size (); ++nd)
		{
			char valueString[160];
			Node node = shapes[sh].getNode (nd);
			snprintf
			(
				valueString,
				126,
				"shp:%d; met:%d; typ:%d; ptx:%f; pty:%f; h1x:%f; h1y:%f; h2x:%f; h2y:%f",
				sh,
				int (controllers[SHAPERS + sh * SH_SIZE + SH_TARGET]),
				int (node.nodeType),
				node.point.x,
				node.point.y,
				node.handle1.x,
				node.handle1.y,
				node.handle2.x,
				node.handle2.y
			);
			if ((sh < MAXSHAPES - 1) || nd < shapes[sh].size ()) strcat (valueString, ";\n");
			else strcat(valueString, "\n");
			strcat (shapesDataString, valueString);
		}
	}
	store (handle, urids.state_shape, shapesDataString, strlen (shapesDataString) + 1, urids.atom_String, LV2_STATE_IS_POD);

	return LV2_STATE_SUCCESS;
}

LV2_State_Status BShapr::state_restore (LV2_State_Retrieve_Function retrieve, LV2_State_Handle handle, uint32_t flags,
			const LV2_Feature* const* features)
{
	size_t   size;
	uint32_t type;
	uint32_t valflags;
	const void* shapesData = retrieve(handle, urids.state_shape, &size, &type, &valflags);

	if (shapesData && (type == urids.atom_String))
	{
		// Clear old data
		for (int i = 0; i < MAXSHAPES; ++i) shapes[i].clearShape ();

		// Parse retrieved data
		std::string shapesDataString = (char*) shapesData;
		const std::string keywords[9] = {"shp:", "met:", "typ:", "ptx:", "pty:", "h1x:", "h1y:", "h2x:", "h2y:"};
		while (!shapesDataString.empty())
		{
			// Look for next "shp:"
			size_t strPos = shapesDataString.find ("shp:");
			size_t nextPos = 0;
			if (strPos == std::string::npos) break;	// No "shp:" found => end
			if (strPos + 4 > shapesDataString.length()) break;	// Nothing more after id => end
			shapesDataString.erase (0, strPos + 4);

			int sh;
			try {sh = std::stof (shapesDataString, &nextPos);}
			catch  (const std::exception& e)
			{
				fprintf (stderr, "BShapr.lv2: Restore shape state incomplete. Can't parse shape number from \"%s...\"", shapesDataString.substr (0, 63).c_str());
				break;
			}

			if (nextPos > 0) shapesDataString.erase (0, nextPos);
			if ((sh < 0) || (sh >= MAXSHAPES))
			{
				fprintf (stderr, "BShapr.lv2: Restore shape state incomplete. Invalid matrix data block loaded for shape %i.\n", sh);
				break;
			}

			// Look for shape data
			Node node = {NodeType::POINT_NODE, {0, 0}, {0, 0}, {0, 0}};
			bool isTypeDef = false;
			int methodNr = -1;
			for (int i = 1; i < 9; ++i)
			{
				strPos = shapesDataString.find (keywords[i]);
				if (strPos == std::string::npos) continue;	// Keyword not found => next keyword
				if (strPos + 4 >= shapesDataString.length())	// Nothing more after keyword => end
				{
					shapesDataString ="";
					break;
				}
				if (strPos > 0) shapesDataString.erase (0, strPos + 4);
				float val;
				try {val = std::stof (shapesDataString, &nextPos);}
				catch  (const std::exception& e)
				{
					fprintf (stderr, "BShapr.lv2: Restore shape state incomplete. Can't parse %s from \"%s...\"",
							 keywords[i].substr(0,3).c_str(), shapesDataString.substr (0, 63).c_str());
					break;
				}

				if (nextPos > 0) shapesDataString.erase (0, nextPos);
				switch (i)
				{
					case 1: methodNr = LIM (val, 0, MAXEFFECTS - 1);
						break;
					case 2: node.nodeType = (NodeType)((int)val);
						isTypeDef = true;
						break;
					case 3: node.point.x = val;
						break;
					case 4:	node.point.y = val;
						break;
					case 5:	node.handle1.x = val;
						break;
					case 6:	node.handle1.y = val;
						break;
					case 7:	node.handle2.x = val;
						break;
					case 8:	node.handle2.y = val;
						break;
					default:break;
				}
			}

			// Set data
			if (isTypeDef)
			{
				if (methodNr >=0)
				{
					shapes[sh].setTransformation (methods[methodNr].transformFactor, methods[methodNr].transformOffset);
					shapes[sh].appendNode (node);
				}

				// Old versions (< 0.7): temp. store node until method is set
				else
				{
					tempNodes[sh].push_back (node);
				}
			}
		}

		// Validate all shapes
		for (int i = 0; i < MAXSHAPES; ++i)
		{
			if (shapes[i].size () < 2) shapes[i].setDefaultShape ();
			else if (!shapes[i].validateShape ()) shapes[i].setDefaultShape ();
		}

		// Force GUI notification
		for (int i = 0; i < MAXSHAPES; ++i) scheduleNotifyShapes[i] = true;
	}

	return LV2_STATE_SUCCESS;
}

LV2_Handle instantiate (const LV2_Descriptor* descriptor, double samplerate, const char* bundle_path, const LV2_Feature* const* features)
{
	// New instance
	BShapr* instance;
	try {instance = new BShapr(samplerate, features);}
	catch (std::exception& exc)
	{
		fprintf (stderr, "BShapr.lv2: Plugin instantiation failed. %s\n", exc.what ());
		return NULL;
	}

	if (!instance)
	{
		fprintf(stderr, "BShapr.lv2: Plugin instantiation failed.\n");
		return NULL;
	}

	if (!instance->map)
	{
		fprintf(stderr, "BShapr.lv2: Host does not support urid:map.\n");
		delete (instance);
		return NULL;
	}

	return (LV2_Handle)instance;
}

void connect_port (LV2_Handle instance, uint32_t port, void *data)
{
	BShapr* inst = (BShapr*) instance;
	inst->connect_port (port, data);
}

void run (LV2_Handle instance, uint32_t n_samples)
{
	BShapr* inst = (BShapr*) instance;
	inst->run (n_samples);
}

void cleanup (LV2_Handle instance)
{
	BShapr* inst = (BShapr*) instance;
	delete inst;
}

static LV2_State_Status state_save(LV2_Handle instance, LV2_State_Store_Function store, LV2_State_Handle handle, uint32_t flags,
           const LV2_Feature* const* features)
{
	BShapr* inst = (BShapr*)instance;
	if (!inst) return LV2_STATE_SUCCESS;

	inst->state_save (store, handle, flags, features);
	return LV2_STATE_SUCCESS;
}

static LV2_State_Status state_restore(LV2_Handle instance, LV2_State_Retrieve_Function retrieve, LV2_State_Handle handle, uint32_t flags,
           const LV2_Feature* const* features)
{
	BShapr* inst = (BShapr*)instance;
	inst->state_restore (retrieve, handle, flags, features);
	return LV2_STATE_SUCCESS;
}

static const void* extension_data(const char* uri)
{
  static const LV2_State_Interface  state  = {state_save, state_restore};
  if (!strcmp(uri, LV2_STATE__interface)) {
    return &state;
  }
  return NULL;
}

const LV2_Descriptor descriptor =
{
		BSHAPR_URI,
		instantiate,
		connect_port,
		NULL, //activate,
		run,
		NULL, //deactivate,
		cleanup,
		extension_data
};

// LV2 Symbol Export
LV2_SYMBOL_EXPORT const LV2_Descriptor* lv2_descriptor(uint32_t index)
{
	switch (index)
	{
	case 0: return &descriptor;
	default: return NULL;
	}
}
