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

#ifndef URIDS_H_
#define URIDS_H_

#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#include <lv2/lv2plug.in/ns/ext/atom/util.h>
#include <lv2/lv2plug.in/ns/ext/urid/urid.h>
#include <lv2/lv2plug.in/ns/ext/time/time.h>
#include "definitions.h"

typedef struct
{
	LV2_URID atom_Float;
	LV2_URID atom_Int;
	LV2_URID atom_Long;
	LV2_URID atom_Object;
	LV2_URID atom_Blank;
	LV2_URID atom_eventTransfer;
	LV2_URID atom_Vector;
	LV2_URID time_Position;
	LV2_URID time_barBeat;
	LV2_URID time_bar;
	LV2_URID time_beatsPerMinute;
	LV2_URID time_beatsPerBar;
	LV2_URID time_beatUnit;
	LV2_URID time_speed;
	LV2_URID ui_on;
	LV2_URID ui_off;
	LV2_URID notify_shapeEvent;
	LV2_URID notify_shapeNr;
	LV2_URID notify_shapeData;
	LV2_URID notify_nodeEvent;
	LV2_URID notify_nodeNr;
	LV2_URID notify_nodeOperation;
	LV2_URID notify_nodeData;
	LV2_URID notify_monitorEvent;
	LV2_URID notify_monitor;
}  BShaprURIDs;

void mapURIDs (LV2_URID_Map* m, BShaprURIDs* uris)
{
	uris->atom_Float = m->map(m->handle, LV2_ATOM__Float);
	uris->atom_Int = m->map(m->handle, LV2_ATOM__Int);
	uris->atom_Long = m->map(m->handle, LV2_ATOM__Long);
	uris->atom_Object = m->map(m->handle, LV2_ATOM__Object);
	uris->atom_Blank = m->map(m->handle, LV2_ATOM__Blank);
	uris->atom_eventTransfer = m->map(m->handle, LV2_ATOM__eventTransfer);
	uris->atom_Vector = m->map(m->handle, LV2_ATOM__Vector);
	uris->time_Position = m->map(m->handle, LV2_TIME__Position);
	uris->time_barBeat = m->map(m->handle, LV2_TIME__barBeat);
	uris->time_bar = m->map(m->handle, LV2_TIME__bar);
	uris->time_beatsPerMinute = m->map(m->handle, LV2_TIME__beatsPerMinute);
	uris->time_beatUnit = m->map(m->handle, LV2_TIME__beatUnit);
	uris->time_beatsPerBar = m->map(m->handle, LV2_TIME__beatsPerBar);
	uris->time_speed = m->map(m->handle, LV2_TIME__speed);
	uris->ui_on = m->map(m->handle, BSHAPR_URI "#UIon");
	uris->ui_off = m->map(m->handle, BSHAPR_URI "#UIoff");
	uris->notify_shapeEvent = m->map(m->handle, BSHAPR_URI "#NOTIFYshapeEvent");
	uris->notify_shapeNr = m->map(m->handle, BSHAPR_URI "#NOTIFYshapeNr");
	uris->notify_shapeData = m->map(m->handle, BSHAPR_URI "#NOTIFYshapeData");
	uris->notify_nodeEvent = m->map(m->handle, BSHAPR_URI "#NOTIFYnodeEvent");
	uris->notify_nodeNr = m->map(m->handle, BSHAPR_URI "#NOTIFYnodeNr");
	uris->notify_nodeOperation = m->map(m->handle, BSHAPR_URI "#NOTIFYnodeOperation");
	uris->notify_nodeData = m->map(m->handle, BSHAPR_URI "#NOTIFYnodeData");
	uris->notify_monitorEvent = m->map(m->handle, BSHAPR_URI "#NOTIFYmonitorEvent");
	uris->notify_monitor = m->map(m->handle, BSHAPR_URI "#NOTIFYmonitor");
}

#endif /* URIDS_H_ */
