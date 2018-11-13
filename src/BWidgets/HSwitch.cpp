/* HSwitch.cpp
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

#include "HSwitch.hpp"

#ifndef PI
#define PI 3.14159265
#endif

namespace BWidgets
{
HSwitch::HSwitch () : HSwitch (0.0, 0.0, BWIDGETS_DEFAULT_HSWITCH_WIDTH, BWIDGETS_DEFAULT_HSWITCH_HEIGHT, "hswitch", BWIDGETS_DEFAULT_VALUE) {}

HSwitch::HSwitch (const double  x, const double y, const double width, const double height, const std::string& name,
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

HSwitch::HSwitch (const HSwitch& that) : ToggleButton (that), knob (that.knob), scale (that.scale)
{
	add (scale);
	add (knob);
}

HSwitch::~HSwitch () {}

HSwitch& HSwitch::operator= (const HSwitch& that)
{
	release (&scale);
	release (&knob);

	knob = that.knob;
	scale = that.scale;
	ToggleButton::operator= (that);

	add (scale);
	add (knob);

	return *this;
}

void HSwitch::update ()
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

	double sch = (h > 24.0 ? 24.0 : h);
	if (2 * sch > w) sch = w / 2;
	double knw = sch;
	double knh = knw;
	double scw = w;

	scale.setValue (relVal);
	scale.setHeight (sch);
	scale.setWidth (scw);
	scale.moveTo (x0 + w/2 - scw/2, y0 + h/2 - sch/2);

	double x1 = x0 + w/2 - scw/2 + relVal * (scw - knw) + d/2;
	double y1 = y0 + h/2 - knh/2 + d/2;
	knob.setWidth (knw - d/2);
	knob.setHeight (knh - d/2);
	knob.moveTo (x1, y1);

	if (isVisible ()) postRedisplay ();
}

void HSwitch::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}

void HSwitch::applyTheme (BStyles::Theme& theme, const std::string& name)
{
	Widget::applyTheme (theme, name);
	knob.applyTheme (theme, name);
	scale.applyTheme (theme, name);
}

void HSwitch::draw (const double x, const double y, const double width, const double height)
{
	Widget::draw (x, y, width, height);
}

}
