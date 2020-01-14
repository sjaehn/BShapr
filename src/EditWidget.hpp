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

#ifndef EDITWIDGET_HPP_
#define EDITWIDGET_HPP_

#include "BWidgets/Widget.hpp"
#include "BWidgets/Focusable.hpp"
#include "BWidgets/Label.hpp"

class EditWidget : public BWidgets::Widget, public BWidgets::Focusable
{
public:
	EditWidget () : EditWidget (0, 0, 0, 0, "editwidget", "") {}

	EditWidget (const double x, const double y, const double width, const double height, const std::string& name, const std::string& focusText) :
		Widget (x, y, width, height, name),
		Focusable (std::chrono::milliseconds (BWIDGETS_DEFAULT_FOCUS_IN_MS),
			   std::chrono::milliseconds (BWIDGETS_DEFAULT_FOCUS_OUT_MS)),
			   focusLabel_ (0, 0, 40, 20, name_ + BWIDGETS_DEFAULT_FOCUS_NAME, focusText)
   	{
   		focusLabel_.setOversize (true);
   		focusLabel_.resize ();
   		focusLabel_.hide ();
   		add (focusLabel_);
   	}

	EditWidget (const EditWidget& that) :
		Widget (that), Focusable (that),
		focusLabel_ (that.focusLabel_)
	{
		focusLabel_.hide();
		add (focusLabel_);
	}

	EditWidget& operator= (const EditWidget& that)
	{
		release (&focusLabel_);
		focusLabel_ = that.focusLabel_;
		focusLabel_.hide();

		Widget::operator= (that);
		Focusable::operator= (that);

		add (focusLabel_);

		return *this;

	}

	virtual void applyTheme (BStyles::Theme& theme) override {applyTheme (theme, name_);}

	virtual void applyTheme (BStyles::Theme& theme, const std::string& name) override
	{
		Widget::applyTheme (theme, name);
		focusLabel_.applyTheme (theme, name + BWIDGETS_DEFAULT_FOCUS_NAME);
		focusLabel_.resize();
	}

	virtual void onFocusIn (BEvents::FocusEvent* event) override
	{
		if (event && event->getWidget())
		{
			BUtilities::Point pos = event->getPosition();
			focusLabel_.moveTo (pos.x - 0.5 * focusLabel_.getWidth(), pos.y - focusLabel_.getHeight());
			focusLabel_.show();
		}
		Widget::onFocusIn (event);
	}

	virtual void onFocusOut (BEvents::FocusEvent* event) override
	{
		if (event && event->getWidget()) focusLabel_.hide();
		Widget::onFocusOut (event);
	}

protected:
	BWidgets::Label focusLabel_;
};

#endif /* EDITWIDGET_HPP_ */
