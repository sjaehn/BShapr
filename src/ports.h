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

#ifndef PORTS_H_
#define PORTS_H_

typedef enum
{
	CONTROL		= 0,
	NOTIFY		= 1,
	AUDIO_IN_1	= 2,
	AUDIO_IN_2	= 3,
	AUDIO_OUT_1	= 4,
	AUDIO_OUT_2	= 5,

#ifdef SUPPORTS_CV
	CV_OUT		= 6,
	CONTROLLERS	= 10,
#else
	CONTROLLERS	= 6,
#endif

	BYPASS		= 0,
	DRY_WET		= 1,
	MIDI_CONTROL	= 2,
	MIDI_KEYS	= 3,
	MIDI_THRU	= 4,
	BASE		= 5,
	BASE_VALUE	= 6,
	ACTIVE_SHAPE	= 7,

	SHAPERS		= 8,
	SH_INPUT	= 0,
	SH_INPUT_AMP	= 1,
	SH_TARGET	= 2,
	SH_DRY_WET	= 3,
	SH_OUTPUT	= 4,
	SH_OUTPUT_AMP	= 5,
	SH_SMOOTHING	= 6,
	SH_OPTION	= 7,
	SH_SIZE		= 15,

	NR_CONTROLLERS	= 68
} BShaprPortIndex;

typedef enum
{
	SECONDS		= 0,
	BEATS		= 1,
	BARS		= 2
} BShaprBaseIndex;

typedef enum
{
	OFF		= 0,
	AUDIO_IN	= 1,
	CONSTANT	= 2,
	OUTPUT		= 3
} BShaprInputIndex;

typedef enum
{
	LEVEL		= 0,
	BALANCE		= 1,
	WIDTH		= 2,
	LOW_PASS	= 3,
	HIGH_PASS	= 4,
	GAIN		= 5,
	LOW_PASS_LOG	= 6,
	HIGH_PASS_LOG	= 7,
	PITCH		= 8,
	DELAY		= 9,
	DOPPLER		= 10,
	DISTORTION	= 11,
	DECIMATE	= 12,
	BITCRUSH	= 13,

#ifdef SUPPORTS_CV
	SEND_CV		= 14,
#else
	SEND_MIDI	= 14,
#endif

	REVERB		= 15

} BShaprTargetIndex;

typedef enum
{
	INTERNAL	= 0,
	AUDIO_OUT	= 1
} BShaprOutputIndex;

#endif /* PORTS_H_ */
