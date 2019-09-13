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

#ifndef METHOD_HPP_
#define METHOD_HPP_

#include "Node.hpp"
#include "definitions.hpp"
#include "Option.hpp"

struct Method
{
	// Position in ListBox
	int index;

	// Optional dials / sliders / ...
	Option options[MAXOPTIONS];

	// Shape / ShapeWidget parameters
	Node defaultEndNode;
	Limit limit;
	double anchorYPos;
	double anchorValue;
	double ratio;
	std::string prefix;
	std::string unit;
	std::string iconFileName;
};

const Method methods[MAXEFFECTS] =
{
	{0, {{NO_WIDGET, "", 0, {0, 0, 0}}, {NO_WIDGET, "", 0, {0, 0, 0}}, {NO_WIDGET, "", 0, {0, 0, 0}}, {NO_WIDGET, "", 0, {0, 0, 0}}}, {NodeType::END_NODE, {0, 1}, {0, 0}, {0, 0}}, {0, 10, 0}, 0.05, 0, 1.1, "", "", "inc/Level.png"},
	{2, {{NO_WIDGET, "", 0, {0, 0, 0}}, {NO_WIDGET, "", 0, {0, 0, 0}}, {NO_WIDGET, "", 0, {0, 0, 0}}, {NO_WIDGET, "", 0, {0, 0, 0}}}, {NodeType::END_NODE, {0, 0}, {0, 0}, {0, 0}}, {-1, 1, 0}, 0.5, 0, 2.2, "", "", "inc/Balance.png"},
	{3, {{NO_WIDGET, "", 0, {0, 0, 0}}, {NO_WIDGET, "", 0, {0, 0, 0}}, {NO_WIDGET, "", 0, {0, 0, 0}}, {NO_WIDGET, "", 0, {0, 0, 0}}}, {NodeType::END_NODE, {0, 1}, {0, 0}, {0, 0}}, {0, 100, 0}, 0.05, 0, 2.2, "", "", "inc/Width.png"},
	{4, {{NO_WIDGET, "", 0, {0, 0, 0}}, {NO_WIDGET, "", 0, {0, 0, 0}}, {NO_WIDGET, "", 0, {0, 0, 0}}, {NO_WIDGET, "", 0, {0, 0, 0}}}, {NodeType::END_NODE, {0, 2000}, {0, 0}, {0, 0}}, {20, 20000, 0}, 0, 0, 5050, "", "Hz", "inc/Low_pass.png"},
	{6, {{NO_WIDGET, "", 0, {0, 0, 0}}, {NO_WIDGET, "", 0, {0, 0, 0}}, {NO_WIDGET, "", 0, {0, 0, 0}}, {NO_WIDGET, "", 0, {0, 0, 0}}}, {NodeType::END_NODE, {0, 200}, {0, 0}, {0, 0}}, {20, 20000, 0}, 0, 0, 810, "", "Hz", "inc/High_pass.png"},
	{1, {{NO_WIDGET, "", 0, {0, 0, 0}}, {NO_WIDGET, "", 0, {0, 0, 0}}, {NO_WIDGET, "", 0, {0, 0, 0}}, {NO_WIDGET, "", 0, {0, 0, 0}}}, {NodeType::END_NODE, {0, 0}, {0, 0}, {0, 0}}, {-90, 30, 0}, 0.75, 0, 90, "", "dB", "inc/Amplify.png"},
	{5, {{NO_WIDGET, "", 0, {0, 0, 0}}, {NO_WIDGET, "", 0, {0, 0, 0}}, {NO_WIDGET, "", 0, {0, 0, 0}}, {NO_WIDGET, "", 0, {0, 0, 0}}}, {NodeType::END_NODE, {0, 3.301}, {0, 0}, {0, 0}}, {1.301, 4.301, 0}, 0.1, 1.3, 3.5, "10^", "Hz", "inc/Low_pass_log.png"},
	{7, {{NO_WIDGET, "", 0, {0, 0, 0}}, {NO_WIDGET, "", 0, {0, 0, 0}}, {NO_WIDGET, "", 0, {0, 0, 0}}, {NO_WIDGET, "", 0, {0, 0, 0}}}, {NodeType::END_NODE, {0, 2.301}, {0, 0}, {0, 0}}, {1.301, 4.301, 0}, 0.1, 1.3, 3.5, "10^", "Hz", "inc/High_pass_log.png"},
	{8, {{NO_WIDGET, "", 0, {0, 0, 0}}, {NO_WIDGET, "", 0, {0, 0, 0}}, {NO_WIDGET, "", 0, {0, 0, 0}}, {NO_WIDGET, "", 0, {0, 0, 0}}}, {NodeType::END_NODE, {0, 0}, {0, 0}, {0, 0}}, {-12, 12, 0}, 0.5, 0, 25, "", "semitones", "inc/Pitch_shift.png"},
	{9, {{NO_WIDGET, "", 0, {0, 0, 0}}, {NO_WIDGET, "", 0, {0, 0, 0}}, {NO_WIDGET, "", 0, {0, 0, 0}}, {NO_WIDGET, "", 0, {0, 0, 0}}}, {NodeType::END_NODE, {0, 0}, {0, 0}, {0, 0}}, {0, 800, 0}, 0.05, 0, 800, "", "ms", "inc/Delay.png"},
	{10, {{NO_WIDGET, "", 0, {0, 0, 0}}, {NO_WIDGET, "", 0, {0, 0, 0}}, {NO_WIDGET, "", 0, {0, 0, 0}}, {NO_WIDGET, "", 0, {0, 0, 0}}}, {NodeType::END_NODE, {0, 0}, {0, 0}, {0, 0}}, {0, 800, 0}, 0.05, 0, 800, "", "ms", "inc/Doppler_delay.png"}
};

#endif /* METHOD_HPP_ */
