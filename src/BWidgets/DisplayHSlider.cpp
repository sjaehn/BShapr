/* DisplayHSlider.cpp
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

#include "DisplayHSlider.hpp"

namespace BWidgets
{
DisplayHSlider::DisplayHSlider () :
		DisplayHSlider (0.0, 0.0, BWIDGETS_DEFAULT_DISPLAYHSLIDER_WIDTH, BWIDGETS_DEFAULT_DISPLAYHSLIDER_HEIGHT,
								 "displayhslider",
								 BWIDGETS_DEFAULT_VALUE, BWIDGETS_DEFAULT_RANGE_MIN, BWIDGETS_DEFAULT_RANGE_MAX, BWIDGETS_DEFAULT_RANGE_STEP,
								 BWIDGETS_DEFAULT_VALUE_FORMAT) {}

DisplayHSlider::DisplayHSlider (const double x, const double y, const double width, const double height, const std::string& name,
												  const double value, const double min, const double max, const double step,
												  const std::string& valueFormat) :
	RangeWidget (x, y, width, height, name, value, min, max, step),
	slider (0, 0, width, width, name, value, min, max, step),
	valueDisplay(0, 0.75 * height, width, 0.25 * height, name),
	valFormat (valueFormat)
{
	slider.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, DisplayHSlider::redirectPostValueChanged);
	valueDisplay.setText (BValues::toBString (valueFormat, value));
	update ();
	add (slider);
	add (valueDisplay);
}

DisplayHSlider::DisplayHSlider (const DisplayHSlider& that) :
		RangeWidget (that),
		slider (that.slider), valueDisplay (that.valueDisplay), valFormat (that.valFormat)
{
	add (slider);
	add (valueDisplay);
}

DisplayHSlider& DisplayHSlider::operator= (const DisplayHSlider& that)
{
	release (&slider);
	release (&valueDisplay);

	valFormat = that.valFormat;
	RangeWidget::operator= (that);
	slider = that.slider;
	valueDisplay = that.valueDisplay;

	add (slider);
	add (valueDisplay);

	return *this;
}

void DisplayHSlider::setValue (const double val)
{
	RangeWidget::setValue (val);

	// Pass changed value to dial and display
	if (value != slider.getValue ()) slider.setValue (value);
	valueDisplay.setText(BValues::toBString (valFormat, value));
}

void DisplayHSlider::setMin (const double min)
{
	RangeWidget::setMin (min);

	if (rangeMin != slider.getMin ()) slider.setMin (rangeMin);
}

void DisplayHSlider::setMax (const double max)
{
	RangeWidget::setMin (max);

	if (rangeMax != slider.getMax ()) slider.setMax (rangeMax);
}

void DisplayHSlider::setStep (const double step)
{
	RangeWidget::setStep (step);

	if (rangeStep != slider.getStep ()) slider.setStep (rangeStep);
}

DisplayHSlider::~DisplayHSlider () {}

void DisplayHSlider::setValueFormat (const std::string& valueFormat) {valFormat = valueFormat;}
std::string DisplayHSlider::getValueFormat () const {return valFormat;}
HSlider* DisplayHSlider::getSlider () {return &slider;}
Label* DisplayHSlider::getValueDisplay () {return &valueDisplay;}

void DisplayHSlider::update ()
{
	updateChildCoords ();
	draw (0, 0, width_, height_);
	if (isVisible ()) postRedisplay ();
}

void DisplayHSlider::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}
void DisplayHSlider::applyTheme (BStyles::Theme& theme, const std::string& name)
{
	Widget::applyTheme (theme, name);
	slider.applyTheme (theme, name);
	valueDisplay.applyTheme (theme, name);
	update ();
}

void DisplayHSlider::redirectPostValueChanged (BEvents::Event* event)
{

	if (event && (event->getEventType () == BEvents::EventType::VALUE_CHANGED_EVENT) && event->getWidget ())
	{
		BEvents::ValueChangedEvent* ev = (BEvents::ValueChangedEvent*) event;
		RangeWidget* w = (RangeWidget*) ev->getWidget ();
		if (w->getParent ())
		{
			DisplayHSlider* p = (DisplayHSlider*) w->getParent ();

			// Get value and range from slider
			if (p->getValue () != w->getValue ()) p->setValue (w->getValue ());
			if (p->getMin () != w->getMin ()) p->setMin (w->getMin ());
			if (p->getMax () != w->getMax ()) p->setMax (w->getMax ());
			if (p->getStep () != w->getStep ()) p->setStep (w->getStep ());

			p->updateChildCoords ();
		}
	}
}

void DisplayHSlider::updateChildCoords ()
{
	double th = getHeight () / 2;
	double tw = th * 2.2;
	double ty = 0;
	//double tx = getWidth () / 2 - tw / 2;

	double sh = th;
	double sw = getWidth ();
	double sx = 0;
	double sy = th;

	// Calculate horizontal value display position
	double h = (sh > 24.0 ? 12.0 : 0.5 * sh);							// Slider height ( = 0.5 * knob height)
	double w = (sw / sh >= 2 ? sw - 2 * h : sw - (sw / sh) * h);		// Effective width of the slider
	double relVal;
	if (getMax () != getMin ()) relVal = (getValue () - getMin ()) / (getMax () - getMin ());
	else relVal = 0.5;													// min == max doesn't make any sense, but need to be handled
	if (getStep() < 0) relVal = 1 - relVal;								// Swap if reverse orientation
	double tx = sw / 2 - w / 2 + relVal * (w - 2) - tw / 2 + 1;
	if (tx > getWidth () - tw) tx = getWidth () - tw;
	if (tx < 0) tx = 0;

	slider.moveTo (sx, sy);
	slider.setWidth (sw);
	slider.setHeight (sh);
	//slider.update ();

	valueDisplay.moveTo (tx, ty);
	valueDisplay.setWidth (tw);
	valueDisplay.setHeight (th);
	valueDisplay.getFont ()->setFontSize (th * 0.8);
	valueDisplay.update ();
}

void DisplayHSlider::draw (const double x, const double y, const double width, const double height)
{
	// Draw super class widget elements only
	Widget::draw (x, y, width, height);
}

}
