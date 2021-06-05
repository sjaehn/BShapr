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

#ifndef BSHAPR_HPP_
#define BSHAPR_HPP_

#include <cmath>
#include <array>
#include <vector>
#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/atom/util.h>
#include <lv2/lv2plug.in/ns/ext/atom/forge.h>
#include <lv2/lv2plug.in/ns/ext/urid/urid.h>
#include <lv2/lv2plug.in/ns/ext/time/time.h>
#include <lv2/lv2plug.in/ns/ext/state/state.h>
#include "Globals.hpp"
#include "Urids.hpp"
#include "BUtilities/Point.hpp"
#include "Node.hpp"
#include "Shape.hpp"
#include "BShaprNotifications.hpp"


#define MAX_F_ORDER 12
#define P_ORDER 6
#define PITCHBUFFERTIME 20
#define PITCHFADERTIME 2
#define DELAYBUFFERTIME 20
#define MINOPTIONVALUE -20000
#define MAXOPTIONVALUE 20000

struct AudioBuffer
{
	AudioBuffer ();
	AudioBuffer (const uint32_t size);
	~AudioBuffer ();
	float* frames;
	double wPtr1, wPtr2, rPtr1, rPtr2;
	uint32_t size;
	void resize (const uint32_t size);
	void reset ();
};

class Message
{
public:
	Message ();
	void clearMessages ();
	void setMessage (MessageNr messageNr);
	void deleteMessage (MessageNr messageNr);
	bool isMessage (MessageNr messageNr);
	MessageNr loadMessage ();
	bool isScheduled ();
private:
	uint32_t messageBits;
	bool scheduled;
};

class Fader
{
public:
	Fader ();
	Fader (const float value, const float speed);
	void setTarget (const float target);
	void setSpeed (const float speed);
	float proceed ();
	float getValue () const;

protected:
	float value;
	float target;
	float speed;
};

class BShapr
{
public:
	BShapr (double samplerate, const LV2_Feature* const* features);
	~BShapr();
	void connect_port (uint32_t port, void *data);
	void run (uint32_t n_samples);
	LV2_State_Status state_save(LV2_State_Store_Function store, LV2_State_Handle handle, uint32_t flags, const LV2_Feature* const* features);
	LV2_State_Status state_restore(LV2_State_Retrieve_Function retrieve, LV2_State_Handle handle, uint32_t flags, const LV2_Feature* const* features);

	LV2_URID_Map* map;

private:
	void fillFilterBuffer (float filterBuffer[MAXSHAPES] [MAX_F_ORDER / 2], const float value);
	bool isAudioOutputConnected (int shapeNr);
	void audioLevel (const float input1, const float input2, float* output1, float* output2, const float amp);
	void stereoBalance (const float input1, const float input2, float* output1, float* output2, const float balance);
	void stereoWidth (const float input1, const float input2, float* output1, float* output2, const float width);
	void lowPassFilter (const float input1, const float input2, float* output1, float* output2, const float cutoffFreq, const int shape);
	void highPassFilter (const float input1, const float input2, float* output1, float* output2, const float cutoffFreq, const int shape);
	void pitch (const float input1, const float input2, float* output1, float* output2, const float semitone, const int shape);
	void delay (const float input1, const float input2, float* output1, float* output2, const float delaytime, const int shape);
	void doppler (const float input1, const float input2, float* output1, float* output2, const float delaytime, const int shape);
	void decimate (const float input1, const float input2, float* output1, float* output2, const float hz, const int shape);
	void distortion (const float input1, const float input2, float* output1, float* output2, const int mode, const float drive, const float limit);
	void bitcrush (const float input1, const float input2, float* output1, float* output2, const float bitNr);

#ifdef SUPPORTS_CV
	void sendCv (const float input1, const float input2, float* output1, float* output2, float* cv, const float amp);
#else
	void sendMidi (const float input1, const float input2, float* output1, float* output2, const uint8_t midiCh, const uint8_t midiCC, const float amp, const uint32_t frames, const int shape);
#endif

	void play(uint32_t start, uint32_t end);
	void notifyMonitorToGui ();
	void notifyShapeToGui (int shapeNr);
	void notifyMessageToGui ();
	void notifyStatusToGui ();
	double getPositionFromBeats (double beats);
	double getPositionFromFrames (uint64_t frames);
	double getPositionFromSeconds (double seconds);

	double rate;
	float bpm;
	float speed;
	uint64_t bar;
	float barBeat;
	float beatsPerBar;
	uint32_t beatUnit;

	double position;
	double offset;
	uint64_t refFrame;

	// Audio buffers
	float* audioInput1;
	float* audioInput2;
	float* audioOutput1;
	float* audioOutput2;
	AudioBuffer audioBuffer1 [MAXSHAPES];
	AudioBuffer audioBuffer2 [MAXSHAPES];
	float filter1Buffer1 [MAXSHAPES] [MAX_F_ORDER / 2];
	float filter1Buffer2 [MAXSHAPES] [MAX_F_ORDER / 2];
	float filter2Buffer1 [MAXSHAPES] [MAX_F_ORDER / 2];
	float filter2Buffer2 [MAXSHAPES] [MAX_F_ORDER / 2];
	float decimateBuffer1 [MAXSHAPES];
	float decimateBuffer2 [MAXSHAPES];
	double decimateCounter [MAXSHAPES];
	uint8_t sendValue [MAXSHAPES];

	Fader factors[MAXSHAPES];

	// Controllers
	float* new_controllers[NR_CONTROLLERS];
	float controllers [NR_CONTROLLERS];

	// Nodes and Maps
	Shape<MAXNODES> shapes[MAXSHAPES];
	StaticArrayList<Node, MAXNODES> tempNodes[MAXSHAPES];

	// Atom port
	BShaprURIDs urids;

	LV2_Atom_Sequence* controlPort;
	LV2_Atom_Sequence* notifyPort;

#ifdef SUPPORTS_CV
	float* cvOutputs[MAXSHAPES];
#endif

	LV2_Atom_Forge forge;
	LV2_Atom_Forge_Frame notify_frame;

	// Data buffers
	float nodeBuffer[7];
	float shapeBuffer[MAXNODES * 7];

	// MIDI
	uint8_t key;

	// Internals
	bool ui_on;
	Message message;
	int monitorPos;
	unsigned int notificationsCount;
	float stepCount;
	std::array<BShaprNotifications, NOTIFYBUFFERSIZE> notifications;
	bool scheduleNotifyShapes[MAXSHAPES];
	bool scheduleNotifyStatus;

};

#endif /* BSHAPR_HPP_ */
