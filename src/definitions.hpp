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

#ifndef DEFINITIONS_HPP_
#define DEFINITIONS_HPP_

#define NOTIFYBUFFERSIZE 128
#define MONITORBUFFERSIZE 128
#define MAXNODES 64
#define MAXSHAPES 4
#define MAXOPTIONS 6
#define MAXOPTIONWIDGETS 4
#define MAXEFFECTS 16
#define MAXMESSAGES 4
#define MAXUNDO 20
#define GRIDSIZE 2.0

#ifdef SUPPORTS_CV
#define BSHAPR_URI "https://www.jahnichen.de/plugins/lv2/BShapr-cv"
#define BSHAPR_GUI_URI "https://www.jahnichen.de/plugins/lv2/BShapr-cv#gui"
#else
#define BSHAPR_URI "https://www.jahnichen.de/plugins/lv2/BShapr"
#define BSHAPR_GUI_URI "https://www.jahnichen.de/plugins/lv2/BShapr#gui"
#endif

enum MessageNr
{
	NO_MSG		= 0,
	JACK_STOP_MSG	= 1,
	NO_INPUT_MSG	= 2,
	NO_OUTPUT_MSG	= 3,
	MAX_MSG		= 3
};

enum NodeOperationType
{
	DELETE	= 0,
	ADD	= 1,
	CHANGE	= 2
};

#endif /* DEFINITIONS_HPP_ */
