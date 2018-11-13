/* HSlider.hpp
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

#ifndef BWIDGETS_HSLIDER_HPP_
#define BWIDGETS_HSLIDER_HPP_

#include "RangeWidget.hpp"
#include "Knob.hpp"
#include "HScale.hpp"

#define BWIDGETS_DEFAULT_HSLIDER_WIDTH BWIDGETS_DEFAULT_HSCALE_WIDTH
#define BWIDGETS_DEFAULT_HSLIDER_HEIGHT BWIDGETS_DEFAULT_HSCALE_HEIGHT * 2
#define BWIDGETS_DEFAULT_HSLIDER_DEPTH 1.0

namespace BWidgets
{
/**
 * Class BWidgets::HSlider
 *
 * RangeWidget class for a horizontal slider.
 * The Widget is clickable by default.
 */
class HSlider : public RangeWidget
{
public:
	HSlider ();
	HSlider (const double x, const double y, const double width, const double height, const std::string& name,
			 const double value, const double min, const double max, const double step);

	/**
	 * Creates a new (orphan) slider and copies the slider properties from a
	 * source slider.
	 * @param that Source slider
	 */
	HSlider (const HSlider& that);

	~HSlider ();

	/**
	 * Assignment. Copies the slider properties from a source slider and keeps
	 * its name and its position within the widget tree. Emits an expose event
	 * if the widget is visible and a value changed event.
	 * @param that Source slider
	 */
	HSlider& operator= (const HSlider& that);

	/**
	 * Changes the value of the widget and keeps it within the defined range.
	 * Passes the value to its predefined child widgets.
	 * Emits a value changed event and (if visible) an expose event.
	 * @param val Value
	 */
	virtual void setValue (const double val) override;

	/**
	 * Sets the lower limit. Forces the value into the new range. Passes the
	 * min to its predefined child widgets. Emits a value changed event (if
	 * value changed) and (if visible) an expose event.
	 * @param min Lower limit
	 */
	virtual void setMin (const double min) override;

	/**
	 * Sets the upper limit. Forces the value into the new range. Passes the
	 * max to its predefined child widgets. Emits a value changed event (if
	 * value changed) and (if visible) an expose event.
	 * @param max Upper limit
	 */
	virtual void setMax (const double min) override;

	/**
	 * Sets the increment steps for the value. Passes the
	 * increment to its predefined child widgets.
	 * @param step Increment steps.
	 */
	virtual void setStep (const double step);

	/**
	 * Calls a redraw of the widget and calls postRedisplay () if the the
	 * Widget is visible.
	 * This method should be called if the widgets properties are indirectly
	 * changed.
	 */
	virtual void update ();

	/**
	 * Scans theme for widget properties and applies these properties.
	 * @param theme Theme to be scanned.
	 * 				Styles used are:
	 * 				"fgcolors" for BColors::ColorSet (scale active)
	 * 				"bgcolors" for BStyles::ColorSet (knob and scale passive)
	 * @param name Name of the BStyles::StyleSet within the theme to be
	 * 		  	   applied.
	 */
	virtual void applyTheme (BStyles::Theme& theme);
	virtual void applyTheme (BStyles::Theme& theme, const std::string& name);

	/**
	 * Handles the BEvents::BUTTON_PRESS_EVENT to move the slider.
	 * @param event Pointer to a pointer event emitted by the same widget.
	 */
	virtual void onButtonPressed (BEvents::PointerEvent* event) override;

	/**
	 * Handles the BEvents::POINTER_MOTION_WHILE_BUTTON_PRESSED_EVENT to move
	 * the slider.
	 * @param event Pointer to a pointer event emitted by the same widget.
	 */
	virtual void onPointerMotionWhileButtonPressed (BEvents::PointerEvent* event) override;

protected:
	virtual void draw (const double x, const double y, const double width, const double height) override;

	HScale scale;
	Knob knob;
};

}

#endif /* BWIDGETS_HSLIDER_HPP_ */
