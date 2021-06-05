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

#ifndef METHOD_HPP_
#define METHOD_HPP_

#include "Node.hpp"
#include "definitions.hpp"
#include "Option.hpp"

struct Method
{
	// Position in ListBox
	int listIndex;

	// Optional dials / sliders / ...
	int optionIndexes[MAXOPTIONWIDGETS];

	// Shape / ShapeWidget parameters
	Limit limit;
	double anchorYPos;
	double anchorValue;
	double ratio;
	double transformFactor;
	double transformOffset;
	double step;
	std::string prefix;
	std::string unit;
	std::string iconFileName;
};

#endif /* METHOD_HPP_ */
