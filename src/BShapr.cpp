/* B.Shapr
 * Step Sequencer Effect Plugin
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
#include "BShapr.h"

#define LIM(g , min, max) ((g) > (max) ? (max) : ((g) < (min) ? (min) : (g)))

inline double floorfrac (const double value) {return value - floor (value);}

BShapr::BShapr (double samplerate, const LV2_Feature* const* features) :
	map(NULL), controlPort(NULL), notifyPort(NULL),
	audioInput1(NULL), audioInput2(NULL), audioOutput1(NULL), audioOutput2(NULL),
	rate(samplerate), bpm(120.0f), speed(1), bar (0), barBeat (0), position(0), refFrame(0), beatUnit (4), beatsPerBar (4),
	ui_on(false), monitorpos(-1), message (), scheduleNotifyMessage (false)

{
	for (int i = 0; i < MAXSHAPES; ++i) shapes[i].setDefaultShape (defaultEndNodes[0]);
	notifications.fill (defaultNotification);
	monitor.fill (defaultMonitorData);
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

void BShapr::fillFilterBuffer (float filterBuffer[MAXSHAPES] [F_ORDER / 2], const float value)
{
	for (int i = 0; i < MAXSHAPES; ++ i)
	{
		for (int j = 0; j < F_ORDER / 2; ++ j)
		{
			filterBuffer[i][j] = value;
		}
	}
}

float BShapr::validateValue (float value, const Limit limit)
{
	float ltdValue = ((limit.step != 0) ? (limit.min + round ((value - limit.min) / limit.step) * limit.step) : value);
	return LIM (ltdValue, limit.min, limit.max);
}

double BShapr::getPositionFromBeats (double beats)
{
	switch (int (controllers[BASE]))
	{
		case SECONDS: 	return beats / (controllers[BASE_VALUE] * (bpm / 60));
		case BEATS:		return beats / controllers[BASE_VALUE];
		case BARS:		return beats / (controllers[BASE_VALUE] * beatsPerBar);
		default:		return 0;
	}
}

double BShapr::getPositionFromFrames (uint64_t frames)
{
	switch (int (controllers[BASE]))
	{
		case SECONDS: 	return frames * (speed / rate) / controllers[BASE_VALUE] ;
		case BEATS:		return frames * (speed / (rate / (bpm / 60))) / controllers[BASE_VALUE];
		case BARS:		return frames * (speed / (rate / (bpm / 60))) / (controllers[BASE_VALUE] * beatsPerBar);
		default:		return 0;
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
			float newValue;
			int shapeNr = ((i >= SHAPERS) ? ((i - SHAPERS) / SH_SIZE) : -1);
			int shapeControllerNr = ((i >= SHAPERS) ? ((i - SHAPERS) % SH_SIZE) : -1);

			if (i < SHAPERS) newValue = validateValue (*new_controllers[i], globalControllerLimits[i]);
			else
			{
				newValue = validateValue (*new_controllers[i], shapeControllerLimits[shapeControllerNr]);

				if (shapeControllerNr == SH_TARGET)
				{
					// Keep a default shape as a default shape but with new default values
					if (shapes[shapeNr].isDefault ()) shapes[shapeNr].setDefaultShape (defaultEndNodes[(int)newValue]);
				}
			}

			controllers[i] = newValue;
		}
	}

	uint32_t last_t = 0;
	LV2_ATOM_SEQUENCE_FOREACH (controlPort, ev)
	{
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

			// Process (single) node data
/*		else if (obj->body.otype == urids.notify_nodeEvent)
			{
				LV2_Atom *sNr = NULL, *nNr = NULL, *nOp = NULL, *nData = NULL;
				lv2_atom_object_get (obj, urids.notify_shapeNr, &sNr,
										  urids.notify_nodeNr, &nNr,
										  urids.notify_nodeOperation, &nOp,
										  urids.notify_nodeData, &nData,
										  NULL);

				if (sNr && (sNr->type == urids.atom_Int) &&
					nNr && (nNr->type == urids.atom_Int) &&
					nOp && (nOp->type == urids.atom_Int) &&
					nData && (nData->type == urids.atom_Vector))
				{
					int shapeNr = ((LV2_Atom_Int*)nNr)->body;
					int nodeNr = ((LV2_Atom_Int*)nNr)->body;
					int op = ((LV2_Atom_Int*)nOp)->body;
					NodeOperationType operation = (NodeOperationType) op;

					if ((shapeNr >= 0) && (shapeNr < MAXSHAPES) &&
						(nodeNr >= 0) && (nodeNr < MAXNODES) &&
						(op >= 0) && (op < 3))
					{
						const LV2_Atom_Vector* vec = (const LV2_Atom_Vector*) nData;
						size_t vecSize = (nData->size - sizeof(LV2_Atom_Vector_Body)) / sizeof (float);
						if ((vec->body.child_type == urids.atom_Float) && (vecSize == 7))
						{
							Node node ((float*)(&vec->body + 1));

							switch (operation)
							{
								case NodeOperationType::DELETE:	shapes[shapeNr].deleteNode (nodeNr);
																break;

								case NodeOperationType::ADD:	shapes[shapeNr].insertNode (nodeNr, node);
																break;

								case NodeOperationType::CHANGE:	shapes[shapeNr].changeNode (nodeNr, node);
																break;
							}
							//scheduleNotifyShapes[shapeNr] = true;
						}
					}
				}
			}
*/

			// Process (full) shape data
			else if (obj->body.otype == urids.notify_shapeEvent)
			{
				LV2_Atom *sNr = NULL, *sData = NULL;
				lv2_atom_object_get (obj, urids.notify_shapeNr, &sNr,
										  urids.notify_shapeData, &sData,
										  NULL);

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
							for (int nodeNr = 0; (nodeNr < vecSize) && (nodeNr < MAXNODES); ++nodeNr)
							{
								Node node (&data[nodeNr * 7]);
								shapes[shapeNr].appendNode (node);
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
				lv2_atom_object_get (obj, urids.time_bar, &oBar,
											urids.time_barBeat, &oBbeat,
										  urids.time_beatsPerMinute,  &oBpm,
										  urids.time_beatsPerBar,  &oBpb,
										  urids.time_beatUnit,  &oBu,
										  urids.time_speed, &oSpeed,
										  NULL);

				// BPM changed?
				if (oBpm && (oBpm->type == urids.atom_Float))
				{
					float nbpm = ((LV2_Atom_Float*)oBpm)->body;

					if (nbpm != bpm)
					{
						bpm = nbpm;

						if (nbpm < 1.0)
						{
							strcpy (message, "Msg: Jack transport off or halted. Plugin halted.");
							scheduleNotifyMessage = true;
							fillFilterBuffer (filter1Buffer1, 0);
							fillFilterBuffer (filter1Buffer2, 0);
							fillFilterBuffer (filter2Buffer1, 0);
							fillFilterBuffer (filter2Buffer2, 0);
						}

						else
						{
							strcpy (message, "");
							scheduleNotifyMessage = true;
						}
					}
				}

				// Beats per bar changed?
				if (oBpb && (oBpb->type == urids.atom_Float) && (((LV2_Atom_Float*)oBpb)->body > 0)) beatsPerBar = ((LV2_Atom_Float*)oBpb)->body;

				// BeatUnit changed?
				if (oBu && (oBu->type == urids.atom_Int) && (((LV2_Atom_Int*)oBu)->body > 0)) beatUnit = ((LV2_Atom_Int*)oBu)->body;

				// Speed changed?
				if (oSpeed && (oSpeed->type == urids.atom_Float))
				{
					float nspeed = ((LV2_Atom_Float*)oSpeed)->body;

					if (nspeed != speed)
					{
						speed = nspeed;

						if (nspeed == 0)
						{
							strcpy (message, "Msg: Jack transport off or halted. Plugin halted.");
							scheduleNotifyMessage = true;
							fillFilterBuffer (filter1Buffer1, 0);
							fillFilterBuffer (filter1Buffer2, 0);
							fillFilterBuffer (filter2Buffer1, 0);
							fillFilterBuffer (filter2Buffer2, 0);
						}

						else
						{
							strcpy (message, "");
							scheduleNotifyMessage = true;
						}
					}
				}

				// Bar position changed
				if (oBar && (oBar->type == urids.atom_Long) && (bar != ((LV2_Atom_Long*)oBar)->body))
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
					double pos = getPositionFromBeats (barBeat + beatsPerBar * bar);
					position = floorfrac (pos);
					refFrame = ev->time.frames;
				}
			}
		}

		play(last_t, ev->time.frames);
		last_t = ev->time.frames;
	}

	// Play remaining samples
	if (last_t < n_samples) play(last_t, n_samples);

	// Update position in case of no new barBeat submitted on next call
	double relpos = getPositionFromFrames (n_samples - refFrame);	// Position relative to reference frame
	position = floorfrac (position + relpos);
	refFrame = 0;

	// Send collected data to GUI
	if (ui_on)
	{
		notifyMonitorToGui();
		for (int i = 0; i < MAXSHAPES; ++i) if (scheduleNotifyShapes[i]) notifyShapeToGui (i);
		if (scheduleNotifyMessage) notifyMessageToGui ();
	}
	lv2_atom_forge_pop(&forge, &notify_frame);
}

void BShapr::notifyMonitorToGui()
{
	int notificationsCount = 0;
	// Scan monitor and build notifications
	for (int i = 0; i < MONITORBUFFERSIZE; ++i)
	{
		if (monitor[i].ready)
		{
			// Copy data monitor -> notifications
			if (notificationsCount < NOTIFYBUFFERSIZE - 1)
			{
				notifications[notificationsCount].position = i;
				notifications[notificationsCount].inputMin = monitor[i].inputMin;
				notifications[notificationsCount].inputMax = monitor[i].inputMax;
				notifications[notificationsCount].outputMin = monitor[i].outputMin;
				notifications[notificationsCount].outputMax = monitor[i].outputMax;
				notificationsCount++;
			}

			// Reset monitor data
			monitor[i].ready = false;
			monitor[i].inputMin = 0.0;
			monitor[i].inputMax = 0.0;
			monitor[i].outputMin = 0.0;
			monitor[i].outputMax = 0.0;
		}
	}

	// And build one closing notification block for submission of current position (horizon)
	notifications[notificationsCount].position = monitorpos;
	notifications[notificationsCount].inputMin = monitor[monitorpos].inputMin;
	notifications[notificationsCount].inputMax = monitor[monitorpos].inputMax;
	notifications[notificationsCount].outputMin = monitor[monitorpos].outputMin;
	notifications[notificationsCount].outputMax = monitor[monitorpos].outputMax;

	// Send notifications
	LV2_Atom_Forge_Frame frame;
	lv2_atom_forge_frame_time(&forge, 0);
	lv2_atom_forge_object(&forge, &frame, 0, urids.notify_monitorEvent);
	lv2_atom_forge_key(&forge, urids.notify_monitor);
	lv2_atom_forge_vector(&forge, sizeof(float), urids.atom_Float, (uint32_t) (5 * notificationsCount), &notifications);
	lv2_atom_forge_pop(&forge, &frame);

	notificationsCount = 0;
}

void BShapr::notifyShapeToGui (int shapeNr)
{
	size_t size = shapes[shapeNr].size ();

	// Load shapeBuffer
	for (int i = 0; i < size; ++i)
	{
		Node node = shapes[shapeNr].getNode (i);
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
	// Send notifications
	LV2_Atom_Forge_Frame frame;
	lv2_atom_forge_frame_time(&forge, 0);
	lv2_atom_forge_object(&forge, &frame, 0, urids.notify_messageEvent);
	lv2_atom_forge_key(&forge, urids.notify_message);
	lv2_atom_forge_string(&forge, message, strlen (message));
	lv2_atom_forge_pop(&forge, &frame);

	scheduleNotifyMessage = false;
}

void BShapr::audioLevel (const float input1, const float input2, float* output1, float* output2, const float amp)
{
	*output1 = input1 * LIM (amp, 0, 100);
	*output2 = input2 * LIM (amp, 0, 100);
}

void BShapr::stereoBalance (const float input1, const float input2, float* output1, float* output2, const float balance)
{
	float f = LIM (balance, -1, 1);
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
	float f = LIM (width, 0, 100);
	float m = (input1 + input2) / 2;
	float s = (input1 - input2) * f / 2;

	*output1 = m + s;
	*output2 = m - s;
}

void BShapr::lowPassFilter (const float input1, const float input2, float* output1, float* output2, const float cutoffFreq, const int shape)
{
	float f = LIM (cutoffFreq, 0, 20000);
	float a = tan (M_PI * f / rate);
  float a2 = a * a;
	float coeff0 [F_ORDER / 2];
	float coeff1 [F_ORDER / 2];
	float coeff2 [F_ORDER / 2];
	float filter1Buffer0 [F_ORDER / 2];
	float filter2Buffer0 [F_ORDER / 2];

	for (int i = 0; i < F_ORDER / 2; ++i)
	{
    float r = sin (M_PI * (2.0f * i + 1.0f) / (2.0f * F_ORDER));
    float s = a2 + 2.0f * a * r + 1.0f;
    coeff0[i] = a2 / s;
    coeff1[i] = 2.0f * (1 - a2) / s;
    coeff2[i] = -(a2 - 2.0f * a * r + 1.0f) / s;
	}

	double f1 = input1;
	double f2 = input2;
	for (int i = 0; i < F_ORDER / 2; ++i)
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

void BShapr::highPassFilter (const float input1, const float input2, float* output1, float* output2, const float cutoffFreq, const int shape)
{
	float f = LIM (cutoffFreq, 0, 20000);
	float a = tan (M_PI * f / rate);
  float a2 = a * a;
	float coeff0 [F_ORDER / 2];
	float coeff1 [F_ORDER / 2];
	float coeff2 [F_ORDER / 2];
	float filter1Buffer0 [F_ORDER / 2];
	float filter2Buffer0 [F_ORDER / 2];

	for (int i = 0; i < F_ORDER / 2; ++i)
	{
    float r = sin (M_PI * (2.0f * i + 1.0f) / (2.0f * F_ORDER));
    float s = a2 + 2.0f * a * r + 1.0f;
    coeff0[i] = 1 / s;
    coeff1[i] = 2.0f * (1 - a2) / s;
    coeff2[i] = -(a2 - 2.0f * a * r + 1.0f) / s;
	}

	double f1 = input1;
	double f2 = input2;
	for (int i = 0; i < F_ORDER / 2; ++i)
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

void BShapr::play(uint32_t start, uint32_t end)
{
	// Return if halted or bpm == 0
	if ((speed == 0.0f) || (bpm < 1.0f))
	{
		memset(audioOutput1,0,(end-start)*sizeof(float));
		memset(audioOutput2,0,(end-start)*sizeof(float));
		return;
	}

	for (uint32_t i = start; i < end; ++i)
	{
		// Interpolate position within the loop
		double relpos = getPositionFromFrames (i - refFrame);	// Position relative to reference frame
		double pos = floorfrac (position + relpos);				// 0..1 position

		float input1;
		float input2;
		float output1 = 0;
		float output2 = 0;
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
				float iFactor = shapes[sh].getMapValue (pos);

				// Apply shaper on target
				switch (int (controllers[SHAPERS + sh * SH_SIZE + SH_TARGET]))
				{
					case BShaprTargetIndex::LEVEL:
						audioLevel (input1, input2, &shapeOutput1[sh], &shapeOutput2[sh], iFactor);
						break;

					case BShaprTargetIndex::BALANCE:
						stereoBalance (input1, input2, &shapeOutput1[sh], &shapeOutput2[sh], iFactor);
						break;

					case BShaprTargetIndex::WIDTH:
						stereoWidth (input1, input2, &shapeOutput1[sh], &shapeOutput2[sh], iFactor);
						break;

					case BShaprTargetIndex::LOW_PASS:
						lowPassFilter (input1, input2, &shapeOutput1[sh], &shapeOutput2[sh], iFactor, sh);
						break;

					case BShaprTargetIndex::HIGH_PASS:
						highPassFilter (input1, input2, &shapeOutput1[sh], &shapeOutput2[sh], iFactor, sh);
						break;

					default:
						shapeOutput1[sh] = 0;
						shapeOutput2[sh] = 0;

				}

				if (controllers[SHAPERS + sh * SH_SIZE + SH_OUTPUT] == BShaprOutputIndex::AUDIO_OUT)
				{
					output1 += shapeOutput1[sh] * controllers[SHAPERS + sh * SH_SIZE + SH_OUTPUT_AMP];
					output2 += shapeOutput2[sh] * controllers[SHAPERS + sh * SH_SIZE + SH_OUTPUT_AMP];
				}
			}
		}

		// Analyze input and output data for GUI notification
		if (ui_on)
		{
			// Calculate position in monitor
			int newmonitorpos = pos * MONITORBUFFERSIZE;

			// Position changed? => Ready to send
			if (newmonitorpos != monitorpos)
			{
				if (monitorpos >= 0) monitor[monitorpos].ready = true;
				monitorpos = newmonitorpos;
			}

			// Get max input and output values for a block
			if (output1 < monitor[monitorpos].outputMin) monitor[monitorpos].outputMin = output1;
			if (output1 > monitor[monitorpos].outputMax) monitor[monitorpos].outputMax = output1;
			if (output2 < monitor[monitorpos].outputMin) monitor[monitorpos].outputMin = output2;
			if (output2 > monitor[monitorpos].outputMax) monitor[monitorpos].outputMax = output2;
			if (audioInput1[i] < monitor[monitorpos].inputMin) monitor[monitorpos].inputMin = audioInput1[i];
			if (audioInput1[i] > monitor[monitorpos].inputMax) monitor[monitorpos].inputMax = audioInput1[i];
			if (audioInput2[i] < monitor[monitorpos].inputMin) monitor[monitorpos].inputMin = audioInput2[i];
			if (audioInput2[i] > monitor[monitorpos].inputMax) monitor[monitorpos].inputMax = audioInput2[i];

			monitor[monitorpos].ready = false;
		}

		// Store in audio out
		audioOutput1[i] = output1;
		audioOutput2[i] = output2;
	}
}

LV2_State_Status BShapr::state_save (LV2_State_Store_Function store, LV2_State_Handle handle, uint32_t flags,
			const LV2_Feature* const* features)
{
	char shapesDataString[0x8010] = "Shape data:\n";

	for (int sh = 0; sh < MAXSHAPES; ++sh)
	{
		for (int nd = 0; nd < shapes[sh].size (); ++nd)
		{
			char valueString[128];
			Node node = shapes[sh].getNode (nd);
			snprintf (valueString, 126, "shp:%d; typ:%d; ptx:%f; pty:%f; h1x:%f; h1y:%f; h2x:%f; h2y:%f",
								sh, (int) node.nodeType, node.point.x, node.point.y, node.handle1.x, node.handle1.y, node.handle2.x, node.handle2.y);
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
		const std::string keywords[8] = {"shp:", "typ:", "ptx:", "pty:", "h1x:", "h1y:", "h2x:", "h2y:"};
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
			for (int i = 1; i < 8; ++i)
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
					case 1: node.nodeType = (NodeType)((int)val);
									isTypeDef = true;
									break;
					case 2: node.point.x = val;
									break;
					case 3:	node.point.y = val;
									break;
					case 4:	node.handle1.x = val;
									break;
					case 5:	node.handle1.y = val;
									break;
					case 6:	node.handle2.x = val;
									break;
					case 7:	node.handle2.y = val;
									break;
					default:break;
				}
			}

			if (isTypeDef) shapes[sh].appendNode (node);
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
