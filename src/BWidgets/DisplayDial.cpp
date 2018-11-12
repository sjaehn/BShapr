/* DisplayDial.cpp
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

#include "DisplayDial.hpp"

namespace BWidgets
{

DisplayDial::DisplayDial () :
		DisplayDial (0.0, 0.0, BWIDGETS_DEFAULT_DISPLAYDIAL_WIDTH, BWIDGETS_DEFAULT_DISPLAYDIAL_HEIGHT,
							  "displaydial",
							  BWIDGETS_DEFAULT_VALUE, BWIDGETS_DEFAULT_RANGE_MIN, BWIDGETS_DEFAULT_RANGE_MAX, BWIDGETS_DEFAULT_RANGE_STEP,
							  BWIDGETS_DEFAULT_VALUE_FORMAT) {}

DisplayDial::DisplayDial (const double x, const double y, const double width, const double height, const std::string& name,
											const double value, const double min, const double max, const double step,
											const std::string& valueFormat) :
	RangeWidget (x, y, width, height, name, value, min, max, step),
	dial (0, 0, width, width, name, value, min, max, step),
	valueDisplay(0, 0.75 * height, width, 0.25 * height, name),
	valFormat (valueFormat)
{
	dial.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, DisplayDial::redirectPostValueChanged);
	valueDisplay.setText (BValues::toBString (valueFormat, value));
	update ();
	add (dial);
	add (valueDisplay);
}

DisplayDial::DisplayDial (const DisplayDial& that) :
		RangeWidget (that), dial (that.dial), valueDisplay (that.valueDisplay), valFormat (that.valFormat)
{
	add (dial);
	add (valueDisplay);
}

DisplayDial::~DisplayDial () {}

DisplayDial& DisplayDial::operator= (const DisplayDial& that)
{
	valFormat = that.valFormat;
	RangeWidget::operator= (that);
	dial = that.dial;
	valueDisplay = that.valueDisplay;

	return *this;
}

void DisplayDial::setValue (const double val)
{
	RangeWidget::setValue (val);

	// Pass changed value to dial and display
	if (value != dial.getValue ()) dial.setValue (value);
	valueDisplay.setText(BValues::toBString (valFormat, value));
}

void DisplayDial::setMin (const double min)
{
	RangeWidget::setMin (min);

	if (rangeMin != dial.getMin ()) dial.setMin (rangeMin);
}

void DisplayDial::setMax (const double max)
{
	RangeWidget::setMin (max);

	if (rangeMax != dial.getMax ()) dial.setMax (rangeMax);
}

void DisplayDial::setStep (const double step)
{
	RangeWidget::setStep (step);

	if (rangeStep != dial.getStep ()) dial.setStep (rangeStep);
}

void DisplayDial::setValueFormat (const std::string& valueFormat)
{
	valFormat = valueFormat;
	update ();
}

std::string DisplayDial::getValueFormat () const {return valFormat;}

Dial* DisplayDial::getDial () {return &dial;}

Label* DisplayDial::getValueDisplay () {return &valueDisplay;}

void DisplayDial::update ()
{
	updateChildCoords ();
	draw (0, 0, width_, height_);
	if (isVisible ()) postRedisplay ();
}

void DisplayDial::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}
void DisplayDial::applyTheme (BStyles::Theme& theme, const std::string& name)
{
	Widget::applyTheme (theme, name);
	dial.applyTheme (theme, name);
	valueDisplay.applyTheme (theme, name);
	update ();
}

void DisplayDial::updateChildCoords ()
{
	double w = (height_ > width_ ? width_ : height_);
	double h = (height_ > width_ ? width_ : height_);
	dial.moveTo (0.5 * width_ - 0.5 * w, 0.5 * height_ - 0.5 * w);
	dial.setWidth (w);
	dial.setHeight (w);

	valueDisplay.moveTo (0.5 * width_ - 0.5 * w, 0.5 * height_ + 0.3 * h);
	valueDisplay.setWidth (w);
	valueDisplay.setHeight (0.25 * h);
	valueDisplay.getFont ()->setFontSize (0.2 * w);
	valueDisplay.setText (BValues::toBString (valFormat, value));
	valueDisplay.update ();
}

void DisplayDial::redirectPostValueChanged (BEvents::Event* event)
{
	if (event && (event->getEventType () == BEvents::EventType::VALUE_CHANGED_EVENT) && event->getWidget ())
	{
		BEvents::ValueChangedEvent* ev = (BEvents::ValueChangedEvent*) event;
		RangeWidget* w = (RangeWidget*) ev->getWidget ();
		if (w->getParent ())
		{
			DisplayDial* p = (DisplayDial*) w->getParent ();

			// Get value and range from dial
			if (p->getValue () != w->getValue ()) p->setValue (w->getValue ());
			if (p->getMin () != w->getMin ()) p->setMin (w->getMin ());
			if (p->getMax () != w->getMax ()) p->setMax (w->getMax ());
			if (p->getStep () != w->getStep ()) p->setStep (w->getStep ());
		}
	}
}

void DisplayDial::draw (const double x, const double y, const double width, const double height)
{
	// Draw super class widget elements only
	Widget::draw (x, y, width, height);
}

}
