/* B.Slicer
 * Step Sequencer Effect Plugin
 *
 * Copyright (C) 2018 by Sven Jähnichen
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

#include "BSlicer.h"

#include <cstdio>
#include <string>
#include <exception>

#define LIM(g , min, max) ((g) > (max) ? (max) : ((g) < (min) ? (min) : (g)))

BSlicer::BSlicer (double samplerate, const LV2_Feature* const* features) :
	map(NULL), controlPort1(NULL), controlPort2(NULL),  notifyPort(NULL),
	audioInput1(NULL), audioInput2(NULL), audioOutput1(NULL), audioOutput2(NULL),
	nrSteps(16), attack(0.2), release (0.2), sequencesperbar (4),
	rate(samplerate), bpm(120.0f), speed(1), position(0), refFrame(0), beatUnit (4), beatsPerBar (4),
	prevStep(NULL), actStep(NULL), nextStep(NULL),
	record_on(true), monitorpos(-1)

{
	notifications.fill (defaultNotification);
	monitor.fill (defaultMonitorData);

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
	getURIs (m, &uris);

	// Initialize forge
	lv2_atom_forge_init (&forge,map);

}

BSlicer::~BSlicer () {}

void BSlicer::connect_port(uint32_t port, void *data)
{
	switch (port) {
	case Control_1:
		controlPort1 = (LV2_Atom_Sequence*) data;
		break;
	case Control_2:
		controlPort2 = (LV2_Atom_Sequence*) data;
		break;
	case Notify:
		notifyPort = (LV2_Atom_Sequence*) data;
		break;
	case AudioIn_1:
		audioInput1 = (float*) data;
		break;
	case AudioIn_2:
		audioInput2 = (float*) data;
		break;
	case AudioOut_1:
		audioOutput1 = (float*) data;
		break;
	case AudioOut_2:
		audioOutput2 = (float*) data;
		break;
	default:
		if ((port >= Controllers) && (port < Controllers + NrControllers)) controllers[port - Controllers] = (float*) data;
	}
}

void BSlicer::run (uint32_t n_samples)
{

	// Check ports
	if ((!controlPort1) || (!controlPort2) || (!notifyPort) || (!audioInput1) || (!audioInput2) || (!audioOutput1) || (!audioOutput2)) return;
	for (int i = 0; i < NrControllers; ++i)
	{
		if (!controllers[i]) return;
	}

	// Update controller values
	attack = LIM (*(controllers[Attack - Controllers]), 0.01, 1.0);
	release = LIM (*(controllers[Release - Controllers]), 0.01, 1.0);
	sequencesperbar = LIM (round (*(controllers[SequencesPerBar - Controllers])), 1, 8);
	nrSteps = LIM (round(*(controllers[NrSteps - Controllers])), 1, 16);
	for (int i = 0; i < MAXSTEPS; ++i) step[i] = LIM (*(controllers[i + Step_ - Controllers]), 0.0, 1.0);

	// Process GUI data
	const LV2_Atom_Event* ev2 = lv2_atom_sequence_begin(&(controlPort2)->body);
	while (!lv2_atom_sequence_is_end (&controlPort2->body, controlPort2->atom.size, ev2))
	{
		if (lv2_atom_forge_is_object_type(&forge, ev2->body.type))
		{
			const LV2_Atom_Object* obj2 = (const LV2_Atom_Object*)&ev2->body;
			if (obj2->body.otype == uris.ui_on) record_on = true;
			else if (obj2->body.otype == uris.ui_off) record_on = false;
		}
		ev2 = lv2_atom_sequence_next(ev2);
	}

	// Prepare forge buffer and initialize atom sequence
	if (record_on)
	{
		const uint32_t space = notifyPort->atom.size;
		lv2_atom_forge_set_buffer(&forge, (uint8_t*) notifyPort, space);
		lv2_atom_forge_sequence_head(&forge, &notify_frame, 0);
	}

	const LV2_Atom_Sequence* in = controlPort1;
	uint32_t last_t =0;

	// Process audio data
	for (const LV2_Atom_Event* ev1 = lv2_atom_sequence_begin(&in->body);
		 !lv2_atom_sequence_is_end(&in->body, in->atom.size, ev1);
		 ev1 = lv2_atom_sequence_next(ev1))
	{
		if ((ev1->body.type == uris.atom_Object) || (ev1->body.type == uris.atom_Blank))
		{
			//update bpm, speed, position
			LV2_Atom *oBbeat = NULL, *oBpm = NULL, *oSpeed = NULL, *oBpb = NULL, *oBu = NULL;
			const LV2_Atom_Object* obj = (const LV2_Atom_Object*)&ev1->body;
			lv2_atom_object_get (obj, uris.time_barBeat, &oBbeat,
									  uris.time_beatsPerMinute,  &oBpm,
									  uris.time_beatsPerBar,  &oBpb,
									  uris.time_beatUnit,  &oBu,
									  uris.time_speed, &oSpeed,
									  NULL);

			// BPM changed?
			if (oBpm && (oBpm->type == uris.atom_Float)) bpm = ((LV2_Atom_Float*)oBpm)->body;

			// Beats per bar changed?
			if (oBpb && (oBpb->type == uris.atom_Float) && (((LV2_Atom_Float*)oBpb)->body > 0)) beatsPerBar = ((LV2_Atom_Float*)oBpb)->body;

			// BeatUnit changed?
			if (oBu && (oBu->type == uris.atom_Int) && (((LV2_Atom_Int*)oBu)->body > 0)) beatUnit = ((LV2_Atom_Int*)oBu)->body;

			// Speed changed? (not really implemented
			if (oSpeed && (oSpeed->type == uris.atom_Float)) speed = ((LV2_Atom_Float*)oSpeed)->body;

			// Beat position changed (during playing) ?
			if (oBbeat && (oBbeat->type == uris.atom_Float))
			{
				// Get position within a sequence (0..1)
				float barsequencepos = ((LV2_Atom_Float*)oBbeat)->body * sequencesperbar / beatsPerBar; // Position within a bar (0..sequencesperbar)
				position = MODFL (barsequencepos);			// Position within a sequence
				refFrame = ev1->time.frames;				// Reference frame
			}
		}

		play(last_t, ev1->time.frames);
		last_t = ev1->time.frames;
	}
	if (last_t < n_samples) play(last_t, n_samples);		// Play remaining samples

	// Update position in case of no new barBeat submitted on next call
	double relpos = (n_samples - refFrame) * speed / (rate / (bpm / 60)) * sequencesperbar / beatsPerBar;	// Position relative to reference frame
	position = MODFL (position + relpos);
	refFrame = 0;

	if (record_on) notifyGUI();						// Send collected data to GUI

}

void BSlicer::notifyGUI()
{
	if (record_on)
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
					int count = monitor[i].count;
					notifications[notificationsCount].position = i;
					notifications[notificationsCount].input = monitor[i].input;
					notifications[notificationsCount].output = monitor[i].output;
					notificationsCount++;
				}

				// Reset monitor data
				monitor[i].ready = false;
				monitor[i].input = 0.0;
				monitor[i].output = 0.0;
			}
		}

		// And build one closing notification block for submission of current position (horizon)
		notifications[notificationsCount].position = monitorpos;
		notifications[notificationsCount].input = monitor[monitorpos].input;
		notifications[notificationsCount].output = monitor[monitorpos].output;

		// Send notifications
		LV2_Atom_Forge_Frame frame;
		lv2_atom_forge_frame_time(&forge, 0);
		lv2_atom_forge_object(&forge, &frame, 0, uris.notify_event);
		lv2_atom_forge_key(&forge, uris.notify_key);
		lv2_atom_forge_vector(&forge, sizeof(float), uris.atom_Float, (uint32_t) (3 * notificationsCount), &notifications);
		lv2_atom_forge_pop(&forge, &frame);

		// Close off sequence
		lv2_atom_forge_pop(&forge, &notify_frame);
		notificationsCount = 0;
	}
}

void BSlicer::play(uint32_t start, uint32_t end)
{
	float vol, relpos, pos, iStepf, iStepFrac, effect1, effect2;
	uint32_t steps = (uint32_t) nrSteps;
	uint32_t iStep;

	//Silence if halted or bpm == 0
	if ((speed == 0.0f) || (bpm < 1.0f))
	{
		memset(audioOutput1,0,(end-start)*sizeof(float));
		memset(audioOutput2,0,(end-start)*sizeof(float));
		return;
	}

	for (uint32_t i = start; i < end; ++i)
	{
		// Interpolate position within the loop
		relpos = (i - refFrame) * speed / (rate / (bpm / 60)) * sequencesperbar / beatsPerBar;	// Position relative to reference frame
		pos = MODFL (position + relpos);
		iStepf = (pos * steps);
		iStep = (uint32_t)iStepf;											// Calculate step number
		iStepFrac = iStepf - iStep;											// Calculate fraction of active step

		// Move to the next step?
		if (actStep != &(step[iStep]))
		{
			prevStep = actStep;
			actStep = &(step[iStep]);
			if (iStep < (steps-1)) nextStep = &(step[iStep+1]);
			else nextStep = &(step[0]);
		}
		if (!prevStep) prevStep = actStep;									// prevStep not initialized (= on Start)?

		// Calculate effect (vol) for the position
		vol = *actStep;
		if (iStepFrac < attack)									// On attack
		{
			if (*prevStep < *actStep) vol = *prevStep + (iStepFrac / attack) * (*actStep - *prevStep);

		}
		if (iStepFrac > (1 - release))									// On release
		{
			if (*nextStep < *actStep) vol = vol - ((iStepFrac - (1 - release)) / release) * (*actStep - *nextStep);
		}

		// Apply effect on input
		effect1 = audioInput1[i] * vol;
		effect2 = audioInput2[i] * vol;

		// Analyze input and output data for GUI notification
		if (record_on)
		{
			// Calculate position in monitor
			int newmonitorpos = (int) (pos * MONITORBUFFERSIZE);
			if (newmonitorpos >= MONITORBUFFERSIZE) newmonitorpos = MONITORBUFFERSIZE;
			if (newmonitorpos < 0) newmonitorpos = 0;

			// Position changed? => Ready to send
			if (newmonitorpos != monitorpos)
			{
				if (monitorpos >= 0) monitor[monitorpos].ready = true;
				monitorpos = newmonitorpos;
			}

			// Get max input and output values for a block
			if (fabs(effect1) > monitor[monitorpos].output) monitor[monitorpos].output = fabs(effect1);
			if (fabs(effect2) > monitor[monitorpos].output) monitor[monitorpos].output = fabs(effect2);
			if (fabs(audioInput1[i]) > monitor[monitorpos].input) monitor[monitorpos].input = fabs(audioInput1[i]);
			if (fabs(audioInput2[i]) > monitor[monitorpos].input) monitor[monitorpos].input = fabs(audioInput2[i]);

			monitor[monitorpos].ready = false;
		}

		// Send effect to audio output
		audioOutput1[i] = effect1;
		audioOutput2[i] = effect2;
	}
}

LV2_Handle instantiate (const LV2_Descriptor* descriptor, double samplerate, const char* bundle_path, const LV2_Feature* const* features)
{
	// New instance
	BSlicer* instance;
	try {instance = new BSlicer(samplerate, features);}
	catch (std::exception& exc)
	{
		fprintf (stderr, "BSlicer.lv2: Plugin instantiation failed. %s\n", exc.what ());
		return NULL;
	}

	if (!instance)
	{
		fprintf(stderr, "BSlicer.lv2: Plugin instantiation failed.\n");
		return NULL;
	}

	if (!instance->map)
	{
		fprintf(stderr, "BSlicer.lv2: Host does not support urid:map.\n");
		delete (instance);
		return NULL;
	}

	return (LV2_Handle)instance;
}

void connect_port (LV2_Handle instance, uint32_t port, void *data)
{
	BSlicer* inst = (BSlicer*) instance;
	inst->connect_port (port, data);
}

void run (LV2_Handle instance, uint32_t n_samples)
{
	BSlicer* inst = (BSlicer*) instance;
	inst->run (n_samples);
}

void cleanup (LV2_Handle instance)
{
	BSlicer* inst = (BSlicer*) instance;
	delete inst;
}

const LV2_Descriptor descriptor =
{
		BSLICER_URI,
		instantiate,
		connect_port,
		NULL, //activate,
		run,
		NULL, //deactivate,
		cleanup,
		NULL //extension_data
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
