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
	{0, 1, 0},
	{0, 4095, 0},
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

const Option options[MAXOPTIONS] =
{
        {DIAL_WIDGET, "-db/octave", 36, {12, 72, 12}, "%1.0f"},
        {POPUP_WIDGET, "", 0, {0, 4, 1}, ""},
	{DIAL_WIDGET, "Limit (db)", 0, {-60, 30, 0}, "%2.1f"},
        {NO_WIDGET, "", 0, {0, 0, 0}, ""}
};

enum OptionIndex
{
        NO_OPT		= -1,
        DB_PER_OCT_OPT	= 0,
	DISTORTION_OPT	= 1,
	LIMIT_DB_OPT	= 2
};

const Method methods[MAXEFFECTS] =
{
	{0, {NO_OPT, NO_OPT, NO_OPT, NO_OPT}, {NodeType::END_NODE, {0, 1}, {0, 0}, {0, 0}}, {0, 2, 0}, 0.05, 0, 1.1, "", "", "inc/Level.png"},
	{2, {NO_OPT, NO_OPT, NO_OPT, NO_OPT}, {NodeType::END_NODE, {0, 0}, {0, 0}, {0, 0}}, {-1, 1, 0}, 0.5, 0, 2.2, "", "", "inc/Balance.png"},
	{3, {NO_OPT, NO_OPT, NO_OPT, NO_OPT}, {NodeType::END_NODE, {0, 1}, {0, 0}, {0, 0}}, {0, 100, 0}, 0.05, 0, 2.2, "", "", "inc/Width.png"},
	{4, {DB_PER_OCT_OPT, NO_OPT, NO_OPT, NO_OPT}, {NodeType::END_NODE, {0, 2000}, {0, 0}, {0, 0}}, {20, 20000, 0}, 0, 0, 5050, "", "Hz", "inc/Low_pass.png"},
	{6, {DB_PER_OCT_OPT, NO_OPT, NO_OPT, NO_OPT}, {NodeType::END_NODE, {0, 200}, {0, 0}, {0, 0}}, {20, 20000, 0}, 0, 0, 810, "", "Hz", "inc/High_pass.png"},
	{1, {NO_OPT, NO_OPT, NO_OPT, NO_OPT}, {NodeType::END_NODE, {0, 0}, {0, 0}, {0, 0}}, {-90, 12, 0}, 0.85, 0, 112, "", "dB", "inc/Amplify.png"},
	{5, {DB_PER_OCT_OPT, NO_OPT, NO_OPT, NO_OPT}, {NodeType::END_NODE, {0, 3.301}, {0, 0}, {0, 0}}, {1.301, 4.301, 0}, 0.1, 1.3, 3.5, "10^", "Hz", "inc/Low_pass_log.png"},
	{7, {DB_PER_OCT_OPT, NO_OPT, NO_OPT, NO_OPT}, {NodeType::END_NODE, {0, 2.301}, {0, 0}, {0, 0}}, {1.301, 4.301, 0}, 0.1, 1.3, 3.5, "10^", "Hz", "inc/High_pass_log.png"},
	{8, {NO_OPT, NO_OPT, NO_OPT, NO_OPT}, {NodeType::END_NODE, {0, 0}, {0, 0}, {0, 0}}, {-12, 12, 0}, 0.5, 0, 25, "", "semitones", "inc/Pitch_shift.png"},
	{9, {NO_OPT, NO_OPT, NO_OPT, NO_OPT}, {NodeType::END_NODE, {0, 0}, {0, 0}, {0, 0}}, {0, 800, 0}, 0.05, 0, 800, "", "ms", "inc/Delay.png"},
	{10, {NO_OPT, NO_OPT, NO_OPT, NO_OPT}, {NodeType::END_NODE, {0, 0}, {0, 0}, {0, 0}}, {0, 800, 0}, 0.05, 0, 800, "", "ms", "inc/Doppler_delay.png"},
	{11, {DISTORTION_OPT, NO_OPT, LIMIT_DB_OPT, NO_OPT}, {NodeType::END_NODE, {0, 0}, {0, 0}, {0, 0}}, {-30, 60, 0}, 0.33, 0, 100, "", "db", "inc/Distortion.png"},
	{12, {NO_OPT, NO_OPT, NO_OPT, NO_OPT}, {NodeType::END_NODE, {0, 48000}, {0, 0}, {0, 0}}, {1, 96000, 0}, 0.05, 0, 100000, "", "Hz", "inc/Decimate.png"},
	{13, {NO_OPT, NO_OPT, NO_OPT, NO_OPT}, {NodeType::END_NODE, {0, 16}, {0, 0}, {0, 0}}, {1, 32, 0}, 0.5, 16, 34, "", "", "inc/Bitcrush.png"},
};

#endif /* GLOBALS_HPP_ */
