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

#include "ValueSelect.hpp"

ValueSelect::ValueSelect () : ValueSelect (0, 0, 0, 0, "select", 0, 0, 1, 0) {}

ValueSelect::ValueSelect (const double  x, const double y, const double width, const double height, const std::string& name,
		 	 	 	 	  const double value, const double min, const double max, const double step) :
	RangeWidget (x, y, width, height, name, value, min, max, step),
	upClick (0, 0, height, height, name + "/click"),
	downClick (width - height, 0, height, height, name + "/click"),
	display (height, 0, width - 2 * height, height, name + "/label", "0")
{
	setDraggable (true);
	setScrollable (true);
	display.setClickable (false);

	upClick.setCallbackFunction(BEvents::EventType::BUTTON_PRESS_EVENT, ValueSelect::buttonPressedCallback);
	downClick.setCallbackFunction(BEvents::EventType::BUTTON_PRESS_EVENT, ValueSelect::buttonPressedCallback);

	add (upClick);
	add (display);
	add (downClick);
}

void ValueSelect::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}

void ValueSelect::applyTheme (BStyles::Theme& theme, const std::string& name)
{
	//Widget::applyTheme (theme, name);
	upClick.applyTheme (theme, name + "/click");
	display.applyTheme (theme, name + "/label");
	downClick.applyTheme (theme, name + "/click");
}

void ValueSelect::update ()
{
	upClick.resize (height_, height_);

	display.moveTo (height_, 0);
	display.resize (width_ - 2 * height_, height_);
	display.setText (BValues::toBString ("%2.2f", value));

	downClick.moveTo (width_ - height_, 0);
	downClick.resize (height_, height_);
}

void ValueSelect::onPointerDragged (BEvents::PointerEvent* event)
{
	if (event->getButton () == BEvents::LEFT_BUTTON) setValue (getValue () - event->getDeltaY () * getStep ());
}

void ValueSelect::onWheelScrolled (BEvents::WheelEvent* event)
{
	setValue (getValue () + event->getDeltaY () * getStep ());
}

void ValueSelect::buttonPressedCallback (BEvents::Event* event)
{
	if ((event) && (((BEvents::PointerEvent*)event)->getButton () == BEvents::LEFT_BUTTON))
	{
		Widget* w = event->getWidget();

		if (w)
		{
			ValueSelect* p = (ValueSelect*) w->getParent();

			if (p)
			{
				if (w == (Widget*) &p->upClick) p->setValue (p->getValue () + p->getStep());
				else if (w == (Widget*) &p->downClick) p->setValue (p->getValue () - p->getStep());
			}
		}
	}
}