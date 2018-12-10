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

#ifndef MAIN_H_
#define MAIN_H_

#define NOTIFYBUFFERSIZE 64
#define MONITORBUFFERSIZE 64
#define MAXSTEPS 16
#define BSLICER_URI "https://www.jahnichen.de/plugins/lv2/BSlicer"
#define BSLICER_GUI_URI "https://www.jahnichen.de/plugins/lv2/BSlicer#gui"

typedef enum {
	Control_1		= 0,
	Control_2		= 1,
	Notify			= 2,
	AudioIn_1		= 3,
	AudioIn_2		= 4,
	AudioOut_1		= 5,
	AudioOut_2		= 6,
	Attack			= 7,
	Release			= 8,
	SequencesPerBar	= 9,
	NrSteps			= 10,
	Step_			= 11
} BSlicerPortIndex;

typedef struct
{
	LV2_URID atom_Float;
	LV2_URID atom_Int;
	LV2_URID atom_Object;
	LV2_URID atom_Blank;
	LV2_URID atom_eventTransfer;
	LV2_URID atom_Vector;
	LV2_URID time_Position;
	LV2_URID time_barBeat;
	LV2_URID time_beatsPerMinute;
	LV2_URID time_beatsPerBar;
	LV2_URID time_beatUnit;
	LV2_URID time_speed;
	LV2_URID ui_on;
	LV2_URID ui_off;
	LV2_URID notify_event;
	LV2_URID notify_key;
}  BSlicerURIs;

typedef struct {
	int position;
	float input;
	float output;
} BSlicerNotifications;

BSlicerNotifications defaultNotification = {0, 0.0, 0.0};

void getURIs (LV2_URID_Map* m, BSlicerURIs* uris)
{
	uris->atom_Float = m->map(m->handle, LV2_ATOM__Float);
	uris->atom_Int = m->map(m->handle, LV2_ATOM__Int);
	uris->atom_Object = m->map(m->handle, LV2_ATOM__Object);
	uris->atom_Blank = m->map(m->handle, LV2_ATOM__Blank);
	uris->atom_eventTransfer = m->map(m->handle, LV2_ATOM__eventTransfer);
	uris->atom_Vector = m->map(m->handle, LV2_ATOM__Vector);
	uris->time_Position = m->map(m->handle, LV2_TIME__Position);
	uris->time_barBeat = m->map(m->handle, LV2_TIME__barBeat);
	uris->time_beatsPerMinute = m->map(m->handle, LV2_TIME__beatsPerMinute);
	uris->time_beatUnit = m->map(m->handle, LV2_TIME__beatUnit);
	uris->time_beatsPerBar = m->map(m->handle, LV2_TIME__beatsPerBar);
	uris->time_speed = m->map(m->handle, LV2_TIME__speed);
	uris->ui_on = m->map(m->handle, BSLICER_URI "#UIon");
	uris->ui_off = m->map(m->handle, BSLICER_URI "#UIoff");
	uris->notify_event = m->map(m->handle, BSLICER_URI "#NOTIFYev");
	uris->notify_key = m->map(m->handle, BSLICER_URI "#NOTIFYkey");
}

#endif /* MAIN_H_ */
