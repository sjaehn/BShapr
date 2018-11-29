/* PopupListBox.cpp
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

#include "PopupListBox.hpp"

namespace BWidgets
{
PopupListBox::PopupListBox () : PopupListBox (0.0, 0.0, 0.0, 0.0, 0.0, 0.0, "itembox", {""}, 1.0) {}

PopupListBox::PopupListBox (const double x, const double y, const double itemWidth, const double itemHeight,
							const double width, const double height, const std::string& name,
							std::vector<std::string> strings, double preselection) :
		ValueWidget (x, y, width, height, name, 0.0),
		downButton (0, 0, 0, 0, name + BWIDGETS_DEFAULT_POPUPLISTBOX_BUTTON_NAME, 0.0),
		itemBox (0, 0, 0, 0, name + BWIDGETS_DEFAULT_POPUPLISTBOX_ITEMBOX_NAME, ""),
		listBox (0, 0, 0, 0, name + BWIDGETS_DEFAULT_POPUPLISTBOX_LISTBOX_NAME, strings, preselection),
		itemBoxWidth (itemWidth), itemBoxHeight (itemHeight)

{
	setClickable (false);
	setValue (preselection);

	downButton.setCallbackFunction (BEvents::EventType::BUTTON_PRESS_EVENT, PopupListBox::handleButtonClicked);
	itemBox.setCallbackFunction (BEvents::EventType::BUTTON_PRESS_EVENT, PopupListBox::handleButtonClicked);
	listBox.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, PopupListBox::handleValueChanged);

	listBox.hide ();

	add (itemBox);
	add (downButton);
	add (listBox);
}

PopupListBox::PopupListBox (const PopupListBox& that) :
		ValueWidget (that), downButton (that.downButton), itemBox (that.itemBox), listBox (that.listBox),
		itemBoxWidth (that.itemBoxWidth), itemBoxHeight (that.itemBoxHeight)
{
	add (itemBox);
	add (downButton);
	add (listBox);
	update ();
}

PopupListBox::~PopupListBox () {}

PopupListBox& PopupListBox::operator= (const PopupListBox& that)
{
	downButton = that.downButton;
	itemBox = that.itemBox;
	listBox = that.listBox;
	itemBoxWidth = that.itemBoxWidth;
	itemBoxHeight = that.itemBoxHeight;

	Widget::operator= (that);
	return *this;
}

std::vector<std::string>* PopupListBox::getItemList () {return listBox.getItemList ();}

std::string PopupListBox::getActiveItem () const {return itemBox.getText ();}

ItemBox* PopupListBox::getItemBox () {return &itemBox;}

ListBox* PopupListBox::getListBox () {return &listBox;}

void PopupListBox::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}

void PopupListBox::applyTheme (BStyles::Theme& theme, const std::string& name)
{
	Widget::applyTheme (theme, name);
	downButton.applyTheme (theme, name + BWIDGETS_DEFAULT_POPUPLISTBOX_BUTTON_NAME);
	itemBox.applyTheme (theme, name + BWIDGETS_DEFAULT_POPUPLISTBOX_ITEMBOX_NAME);
	listBox.applyTheme (theme, name + BWIDGETS_DEFAULT_POPUPLISTBOX_LISTBOX_NAME);
}

void PopupListBox::setValue (const double val)
{
	if (val !=listBox.getValue ()) listBox.setValue (val);
	ValueWidget::setValue (listBox.getValue ());
}
void PopupListBox::update ()
{
	// Update super widget first
	Widget::update ();

	// Calculate size and position of widget elements
	double x0 = getXOffset ();
	double y0 = getYOffset ();
	double w = getEffectiveWidth ();
	double h = getEffectiveHeight ();

	double iw = (w > itemBoxWidth ? itemBoxWidth : w);
	double ih = (h > itemBoxHeight ? itemBoxHeight : h);

	// Item box
	itemBox.moveTo (x0, y0);
	itemBox.setWidth (iw);
	itemBox.setHeight (ih);
	itemBox.setText (listBox.getActiveItem ());


	// Down button
	double x0i = itemBox.getXOffset ();
	double y0i = itemBox.getYOffset ();
	double wi = itemBox.getEffectiveWidth ();
	double hi = itemBox.getEffectiveHeight ();
	double dw = (wi > BWIDGETS_DEFAULT_POPUPLISTBOX_BUTTON_WIDTH ? BWIDGETS_DEFAULT_POPUPLISTBOX_BUTTON_WIDTH : wi);
	downButton.moveTo (x0 + x0i + wi - dw, y0 + y0i);
	downButton.setWidth (dw);
	downButton.setHeight (hi);

	// List box
	listBox.moveTo (x0, y0 + ih);
	listBox.setWidth (w);
	listBox.setHeight (h - ih);
}

void PopupListBox::handleButtonClicked (BEvents::Event* event)
{
	if (event && (event->getEventType () == BEvents::EventType::BUTTON_PRESS_EVENT) && event->getWidget ())
	{
		BEvents::PointerEvent* ev = (BEvents::PointerEvent*) event;
		Widget* w = (Widget*) ev->getWidget ();
		if (w->getParent ())
		{
			PopupListBox* p = (PopupListBox*) w->getParent ();
			if (p->getParent ())
			{
				if ((w == (Widget*) &(p->itemBox)) || (w == (Widget*) &(p->downButton)))
				{
					if (p->listBox.isVisible ()) p->listBox.hide ();
					else p->listBox.show ();
				}
			}
		}
	}
}

void PopupListBox::handleValueChanged (BEvents::Event* event)
{
if (event && (event->getEventType () == BEvents::EventType::VALUE_CHANGED_EVENT) && event->getWidget ())
	{
		BEvents::ValueChangedEvent* ev = (BEvents::ValueChangedEvent*) event;
		ValueWidget* w = (ValueWidget*) ev->getWidget ();
		if (w->getParent ())
		{
			PopupListBox* p = (PopupListBox*) w->getParent ();
			if ((p->getParent ()) && (w == (ValueWidget*) &(p->listBox)))
			{
				p->setValue (w->getValue ());
				p->listBox.hide ();
			}
		}
	}

}

}
