/* VSlider.cpp
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

#include "VSlider.hpp"

#ifndef PI
#define PI 3.14159265
#endif

namespace BWidgets
{
VSlider::VSlider () : VSlider (0.0, 0.0, BWIDGETS_DEFAULT_VSLIDER_WIDTH, BWIDGETS_DEFAULT_VSLIDER_HEIGHT, "vslider",
		  	  	  	  	  	   BWIDGETS_DEFAULT_VALUE, BWIDGETS_DEFAULT_RANGE_MIN, BWIDGETS_DEFAULT_RANGE_MAX, BWIDGETS_DEFAULT_RANGE_STEP) {}

VSlider::VSlider (const double  x, const double y, const double width, const double height, const std::string& name,
				  const double value, const double min, const double max, const double step) :
		RangeWidget (x, y, width, height, name, value, min, max, step),
		scale (0, 0, 0, 0, name, value, min, max, step),
		knob (0, 0, 0, 0, BWIDGETS_DEFAULT_KNOB_DEPTH, name)
{
	scale.setClickable (false);
	scale.setDragable (false);
	add (scale);
	knob.setClickable (false);
	knob.setDragable (false);
	add (knob);
	setClickable (true);
	setDragable (true);
}

VSlider::VSlider (const VSlider& that) : RangeWidget (that), scale (that.scale), knob (that.knob) {}

VSlider::~VSlider () {}

VSlider& VSlider::operator= (const VSlider& that)
{
	scale = that.scale;
	knob = that.knob;
	RangeWidget::operator= (that);

	return *this;
}

void VSlider::setValue (const double val)
{
	RangeWidget::setValue (val);

	// Pass changed value to scale
	if (value != scale.getValue ()) scale.setValue (value);
}

void VSlider::setMin (const double min)
{
	RangeWidget::setMin (min);
	if (rangeMin != scale.getMin ()) scale.setMin (rangeMin);
}

void VSlider::setMax (const double max)
{
	RangeWidget::setMin (max);
	if (rangeMax != scale.getMax ()) scale.setMax (rangeMax);
}

void VSlider::setStep (const double step)
{
	RangeWidget::setStep (step);
	if (rangeStep != scale.getStep ()) scale.setStep (rangeStep);
}

void VSlider::update ()
{
	draw (0, 0, width_, height_);

	// Position of knob and scale
	// Calculate aspect ratios first
	double h = getEffectiveHeight ();
	double w = getEffectiveWidth ();
	double x0 = getXOffset ();
	double y0 = getYOffset ();

	double scw = (w > 24.0 ? 12.0 : 0.5 * w);
	if (2 * scw > h) scw = h / 2;
	double knw = 2 * scw;
	double knh = knw;
	double sch = h - knh;

	scale.setHeight (sch);
	scale.setWidth (scw);
	scale.moveTo (x0 + w/2 - scw/2, y0 + h/2 - sch/2);

	double relVal = getRelativeValue ();
	double y1 = y0 + h/2 + sch/2 - relVal * sch - knh/2;
	double x1 = x0 + w/2 - knw/2;
	knob.setWidth (knw);
	knob.setHeight (knh);
	knob.moveTo (x1, y1);

	if (isVisible ()) postRedisplay ();
}

void VSlider::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}

void VSlider::applyTheme (BStyles::Theme& theme, const std::string& name)
{
	Widget::applyTheme (theme, name);
	knob.applyTheme (theme, name);
	scale.applyTheme (theme, name);
}

void VSlider::onButtonPressed (BEvents::PointerEvent* event)
{
	// Perform only if minimum requirements are satisfied
	if (main_ && isVisible () && (height_ >= 4) && (width_ >= 4) && (event->getButton() == BEvents::LEFT_BUTTON))
	{
		// Get pointer coords
		double y = event->getY ();
		double x = event->getX ();

		// Calculate aspect ratios first
		double h = getEffectiveHeight ();
		double w = getEffectiveWidth ();
		double x0 = getXOffset ();
		double y0 = getYOffset ();

		double scw = (w > 24.0 ? 12.0 : 0.5 * w);
		if (2 * scw > h) scw = h / 2;
		double knh = 2 * scw;
		double sch = h - knh;

		// Pointer within the scale area ? Set value!
		if ((y >= y0 + h/2 - sch/2) && (y <= y0 + h/2 + sch/2) && (x >= x0) && (x <= x0 + w))
		{
			double frac = (y0 + h/2 + sch/2 - y) / sch;
			if (getStep () < 0) frac = 1 - frac;

			double min = getMin ();
			double max = getMax ();
			setValue (min + frac * (max - min));
		}
	}
}

void VSlider::onPointerMotionWhileButtonPressed (BEvents::PointerEvent* event) {onButtonPressed (event);}

void VSlider::draw (const double x, const double y, const double width, const double height)
{
	Widget::draw (x, y, width, height);
}

}
