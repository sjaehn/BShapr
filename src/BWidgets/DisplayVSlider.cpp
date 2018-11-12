/* DisplayVSlider.cpp
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

#include "DisplayVSlider.hpp"

namespace BWidgets
{
DisplayVSlider::DisplayVSlider () :
		DisplayVSlider (0.0, 0.0, BWIDGETS_DEFAULT_DISPLAYVSLIDER_WIDTH, BWIDGETS_DEFAULT_DISPLAYVSLIDER_HEIGHT,
								 "displayvslider",
								 BWIDGETS_DEFAULT_VALUE, BWIDGETS_DEFAULT_RANGE_MIN, BWIDGETS_DEFAULT_RANGE_MAX, BWIDGETS_DEFAULT_RANGE_STEP,
								 BWIDGETS_DEFAULT_VALUE_FORMAT) {}

DisplayVSlider::DisplayVSlider (const double x, const double y, const double width, const double height, const std::string& name,
												  const double value, const double min, const double max, const double step,
												  const std::string& valueFormat) :
	RangeWidget (x, y, width, height, name, value, min, max, step),
	slider (0, 0, width, width, name, value, min, max, step),
	valueDisplay(0, 0.75 * height, width, 0.25 * height, name),
	valFormat (valueFormat)
{
	slider.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, DisplayVSlider::redirectPostValueChanged);
	valueDisplay.setText (BValues::toBString (valueFormat, value));
	update ();
	add (slider);
	add (valueDisplay);
}

DisplayVSlider::DisplayVSlider (const DisplayVSlider& that) :
		RangeWidget (that),
		slider (that.slider), valueDisplay (that.valueDisplay), valFormat (that.valFormat)
{
	add (slider);
	add (valueDisplay);
}

DisplayVSlider::~DisplayVSlider () {}

DisplayVSlider& DisplayVSlider::operator= (const DisplayVSlider& that)
{
	valFormat = that.valFormat;
	RangeWidget::operator= (that);
	slider = that.slider;
	valueDisplay = that.valueDisplay;


	return *this;
}

void DisplayVSlider::setValue (const double val)
{
	RangeWidget::setValue (val);

	// Pass changed value to dial and display
	if (value != slider.getValue ()) slider.setValue (value);
	valueDisplay.setText(BValues::toBString (valFormat, value));
}

void DisplayVSlider::setMin (const double min)
{
	RangeWidget::setMin (min);

	if (rangeMin != slider.getMin ()) slider.setMin (rangeMin);
}

void DisplayVSlider::setMax (const double max)
{
	RangeWidget::setMin (max);

	if (rangeMax != slider.getMax ()) slider.setMax (rangeMax);
}

void DisplayVSlider::setStep (const double step)
{
	RangeWidget::setStep (step);

	if (rangeStep != slider.getStep ()) slider.setStep (rangeStep);
}


void DisplayVSlider::setValueFormat (const std::string& valueFormat)
{
	valFormat = valueFormat;
	update ();
}

std::string DisplayVSlider::getValueFormat () const {return valFormat;}

VSlider* DisplayVSlider::getSlider () {return &slider;}

Label* DisplayVSlider::getValueDisplay () {return &valueDisplay;}

void DisplayVSlider::update ()
{
	updateChildCoords ();
	draw (0, 0, width_, height_);
	if (isVisible ()) postRedisplay ();
}

void DisplayVSlider::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}
void DisplayVSlider::applyTheme (BStyles::Theme& theme, const std::string& name)
{
	Widget::applyTheme (theme, name);
	slider.applyTheme (theme, name);
	valueDisplay.applyTheme (theme, name);
	update ();
}

void DisplayVSlider::redirectPostValueChanged (BEvents::Event* event)
{

	if (event && (event->getEventType () == BEvents::EventType::VALUE_CHANGED_EVENT) && event->getWidget ())
	{
		BEvents::ValueChangedEvent* ev = (BEvents::ValueChangedEvent*) event;
		RangeWidget* w = (RangeWidget*) ev->getWidget ();
		if (w->getParent ())
		{
			DisplayVSlider* p = (DisplayVSlider*) w->getParent ();

			// Get value and range from slider
			if (p->getValue () != w->getValue ()) p->setValue (w->getValue ());
			if (p->getMin () != w->getMin ()) p->setMin (w->getMin ());
			if (p->getMax () != w->getMax ()) p->setMax (w->getMax ());
			if (p->getStep () != w->getStep ()) p->setStep (w->getStep ());

			p->updateChildCoords ();
		}
	}
}

void DisplayVSlider::updateChildCoords ()
{
	double tw = getWidth ();
	double th = tw / 2.2;
	double ty = 0;
	double tx = 0;

	double sh = getHeight () - th;
	double sw = getWidth () / 2;
	double sx = getWidth () / 2 - sw / 2;
	double sy = th;

	slider.moveTo (sx, sy);
	slider.setWidth (sw);
	slider.setHeight (sh);

	valueDisplay.getFont ()->setFontSize (th * 0.8);
	valueDisplay.moveTo (tx, ty);
	valueDisplay.setWidth (tw);
	valueDisplay.setHeight (th);
	valueDisplay.update ();

}

void DisplayVSlider::draw (const double x, const double y, const double width, const double height)
{
	// Draw super class widget elements only
	Widget::draw (x, y, width, height);
}

}
