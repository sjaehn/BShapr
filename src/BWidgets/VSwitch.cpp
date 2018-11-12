/* VSwitch.cpp
 * Copyright (C) 2018  Sven Jähnichen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "VSwitch.hpp"

#ifndef PI
#define PI 3.14159265
#endif

namespace BWidgets
{
VSwitch::VSwitch () : VSwitch (0.0, 0.0, BWIDGETS_DEFAULT_VSWITCH_WIDTH, BWIDGETS_DEFAULT_VSWITCH_HEIGHT, "vswitch", BWIDGETS_DEFAULT_VALUE) {}

VSwitch::VSwitch (const double  x, const double y, const double width, const double height, const std::string& name,
				  const double defaultvalue) :
		ToggleButton (x, y, width, height, name, value),
		scale (0, 0, 0, 0, name, defaultvalue, 0.0, 1.0, 0.0),
		knob (0, 0, 0, 0, BWIDGETS_DEFAULT_KNOB_DEPTH, name)
{
	scale.setClickable (false);
	scale.setDragable (false);
	add (scale);
	knob.setClickable (false);
	knob.setDragable (false);
	add (knob);
}

VSwitch::VSwitch (const VSwitch& that) : ToggleButton (that), knob (that.knob), scale (that.scale) {}

VSwitch::~VSwitch () {}

VSwitch& VSwitch::operator= (const VSwitch& that)
{
	knob = that.knob;
	scale = that.scale;
	ToggleButton::operator= (that);

	return *this;
}

void VSwitch::update ()
{
	draw (0, 0, width_, height_);

	// Position of knob and scale
	// Calculate aspect ratios first
	double d = knob.getDepth ();
	double h = getEffectiveHeight ();
	double w = getEffectiveWidth ();
	double x0 = getXOffset ();
	double y0 = getYOffset ();
	double relVal = getValue ();

	double scw = (w > 24.0 ? 24.0 : w);
	if (2 * scw > h) scw = h / 2;
	double knw = scw;
	double knh = knw;
	double sch = h;

	scale.setValue (relVal);
	scale.setHeight (sch);
	scale.setWidth (scw);
	scale.moveTo (x0 + w/2 - scw/2, y0 + h/2 - sch/2);

	double x1 = x0 + w/2 - knw/2 + d/2;
	double y1 = y0 + h/2 - sch/2 + relVal * (sch - knh) + d/2;
	knob.setWidth (knw - d/2);
	knob.setHeight (knh - d/2);
	knob.moveTo (x1, y1);

	if (isVisible ()) postRedisplay ();
}

void VSwitch::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}

void VSwitch::applyTheme (BStyles::Theme& theme, const std::string& name)
{
	Widget::applyTheme (theme, name);
	knob.applyTheme (theme, name);
	scale.applyTheme (theme, name);
}

void VSwitch::draw (const double x, const double y, const double width, const double height)
{
	Widget::draw (x, y, width, height);
}

}
