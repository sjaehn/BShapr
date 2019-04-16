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

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

#define NOTIFYBUFFERSIZE 256
#define MONITORBUFFERSIZE 256
#define MAXNODES 64
#define MAXSHAPES 4
#define BSHAPR_URI "https://www.jahnichen.de/plugins/lv2/BShapr"
#define BSHAPR_GUI_URI "https://www.jahnichen.de/plugins/lv2/BShapr#gui"

typedef enum
{
	DELETE	= 0,
	ADD		= 1,
	CHANGE	= 2
} NodeOperationType;

typedef struct
{
	float position;
	float inputMin;
	float inputMax;
	float outputMin;
	float outputMax;
} BShaprNotifications;

const BShaprNotifications defaultNotification = {0.0, 0.0, 0.0, 0.0, 0.0};

#endif /* DEFINITIONS_H_ */
