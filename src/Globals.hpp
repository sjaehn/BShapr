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

#ifndef GLOBALS_HPP_
#define GLOBALS_HPP_

#define MIN_OPT_VAL -20000
#define MAX_OPT_VAL 20000

#include "Method.hpp"
#include "ports.h"

const Limit globalControllerLimits [SHAPERS] =
{
	{0, 1, 1},
	{0, 1, 0},
	{0, 1, 1},
	{0, 4095, 0},
	{0, 1, 1},
	{0, 2, 1},
	{1, 16, 0},
	{1, 4, 1}
};

const Limit shapeControllerLimits [SH_SIZE] =
{
	{0, 6, 1},
	{-1, 1, 0},
	{0, MAXEFFECTS - 1, 1},
	{0, 1, 0},
	{0, 1, 1},
	{0, 1, 0},
	{0, 100, 0},
	{MIN_OPT_VAL, MAX_OPT_VAL, 0},
	{MIN_OPT_VAL, MAX_OPT_VAL, 0},
	{MIN_OPT_VAL, MAX_OPT_VAL, 0},
	{MIN_OPT_VAL, MAX_OPT_VAL, 0},
	{MIN_OPT_VAL, MAX_OPT_VAL, 0},
	{MIN_OPT_VAL, MAX_OPT_VAL, 0},
	{MIN_OPT_VAL, MAX_OPT_VAL, 0},
	{MIN_OPT_VAL, MAX_OPT_VAL, 0}
};

enum Distortions
{
	HARDCLIP	= 0,
	SOFTCLIP	= 1,
	FOLDBACK	= 2,
	OVERDRIVE	= 3,
	FUZZ		= 4
};

#ifndef BSHAPRGUI_HPP_
#define DB_PER_OCT_OPT_PARAM 0
#define DISTORTION_OPT_PARAM 0
#define LIMIT_DB_OPT_PARAM 0
#define SEND_MIDI_CH_PARAM 0
#define SEND_MIDI_CC_PARAM 0
#else
#include "BWidgets/BItems.hpp"
#include "CCList.hpp"
#define DB_PER_OCT_OPT_PARAM std::string ("%1.0f")
#define DISTORTION_OPT_PARAM BItems::ItemList ({{0, "Hardclip"}, {1, "Softclip"}, {2, "Foldback"}, {3, "Overdrive"}, {4, "Fuzz"}})
#define LIMIT_DB_OPT_PARAM std::string ("%2.1f")
#define SEND_MIDI_CH_PARAM BItems::ItemList ( \
		{{0, "All"}, {1, "CH 1"}, {2, "CH 2"}, {3, "CH 3"}, {4, "CH 4"}, {5, "CH 5"}, {6, "CH 6"}, {7, "CH 7"}, {8, "CH 8"}, {9, "CH 9"}, \
		 {10, "CH 10"}, {11, "CH 11"}, {12, "CH 12"}, {13, "CH 13"}, {14, "CH 14"}, {15, "CH 15"}, {16, "CH 16"}})
#define SEND_MIDI_CC_PARAM BItems::ItemList (CCLIST_HPP_)
#endif /* BSHAPRGUI_HPP_ */

const Option options[MAXOPTIONS] =
{
        {DIAL_WIDGET, "-db/octave", 36, {12, 72, 12}, DB_PER_OCT_OPT_PARAM},
        {POPUP_WIDGET, "Method", 0, {0, 4, 1}, DISTORTION_OPT_PARAM},
	{DIAL_WIDGET, "Limit (db)", 0, {-60, 30, 0}, LIMIT_DB_OPT_PARAM},
	{POPUP_WIDGET, "MIDI Channel", 0, {0, 16, 1}, SEND_MIDI_CH_PARAM},
	{POPUP_WIDGET, "MIDI CC", 0, {0, 127, 1}, SEND_MIDI_CC_PARAM},
        {NO_WIDGET, "", 0, {0, 0, 0}, 0}
};

enum OptionIndex
{
        NO_OPT		= -1,
        DB_PER_OCT_OPT	= 0,
	DISTORTION_OPT	= 1,
	LIMIT_DB_OPT	= 2,
	SEND_MIDI_CH	= 3,
	SEND_MIDI_CC	= 4
};

const Method methods[MAXEFFECTS] =
{
	{0, {NO_OPT, NO_OPT, NO_OPT, NO_OPT}, {0, 2, 0}, 0.05, 0, 2.2, 1.0, 1.0, "", "", "inc/Level.png"},
	{2, {NO_OPT, NO_OPT, NO_OPT, NO_OPT}, {-1, 1, 0}, 0.5, 0, 2.2, 1.0, 0.0, "", "", "inc/Balance.png"},
	{3, {NO_OPT, NO_OPT, NO_OPT, NO_OPT}, {0, 100, 0}, 0.05, 0, 2.2, 1.0, 1.0, "", "", "inc/Width.png"},
	{4, {DB_PER_OCT_OPT, NO_OPT, NO_OPT, NO_OPT}, {20, 20000, 0}, 0, 0, 8100, 1000.0, 1000.0, "", "Hz", "inc/Low_pass.png"},
	{6, {DB_PER_OCT_OPT, NO_OPT, NO_OPT, NO_OPT}, {20, 20000, 0}, 0, 0, 8100, 1000.0, 1000.0, "", "Hz", "inc/High_pass.png"},
	{1, {NO_OPT, NO_OPT, NO_OPT, NO_OPT}, {-90, 12, 0}, 0.75, 0, 132, 30.0, 0.0, "", "dB", "inc/Amplify.png"},
	{5, {DB_PER_OCT_OPT, NO_OPT, NO_OPT, NO_OPT},  {1.301, 4.301, 0}, 0.1, 1.3, 3.5, 1.0, 3.0, "10^", "Hz", "inc/Low_pass_log.png"},
	{7, {DB_PER_OCT_OPT, NO_OPT, NO_OPT, NO_OPT}, {1.301, 4.301, 0}, 0.1, 1.3, 3.5, 1.0, 3.0, "10^", "Hz", "inc/High_pass_log.png"},
	{8, {NO_OPT, NO_OPT, NO_OPT, NO_OPT}, {-12, 12, 0}, 0.5, 0, 25, 12.0, 0.0, "", "semitones", "inc/Pitch_shift.png"},
	{9, {NO_OPT, NO_OPT, NO_OPT, NO_OPT}, {0, 800, 0}, 0.05, 0, 880, 200.0, 200.0, "", "ms", "inc/Delay.png"},
	{10, {NO_OPT, NO_OPT, NO_OPT, NO_OPT}, {0, 800, 0}, 0.05, 0, 880, 200.0, 200.0, "", "ms", "inc/Doppler_delay.png"},
	{11, {DISTORTION_OPT, NO_OPT, LIMIT_DB_OPT, NO_OPT}, {-30, 60, 0}, 0.33, 0, 100, 30.0, 0.0, "", "db", "inc/Distortion.png"},
	{12, {NO_OPT, NO_OPT, NO_OPT, NO_OPT}, {1, 96000, 0}, 0.05, 0, 110000, 48000.0, 48000.0, "", "Hz", "inc/Decimate.png"},
	{13, {NO_OPT, NO_OPT, NO_OPT, NO_OPT}, {1, 32, 0}, 0.5, 16, 34, 16.0, 16.0, "", "", "inc/Bitcrush.png"},
	{14, {SEND_MIDI_CH, SEND_MIDI_CC, NO_OPT, NO_OPT}, {0, 1, 0}, 0.05, 0, 1.1, 1.0, 0.5, "", "", "inc/Send.png"}
};

#endif /* GLOBALS_HPP_ */
