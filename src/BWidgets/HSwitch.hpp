/* HSwitch.hpp
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

#ifndef BWIDGETS_HSWITCH_HPP_
#define BWIDGETS_HSWITCH_HPP_

#include "ToggleButton.hpp"
#include "Knob.hpp"
#include "HScale.hpp"

#define BWIDGETS_DEFAULT_HSWITCH_WIDTH 40.0
#define BWIDGETS_DEFAULT_HSWITCH_HEIGHT 20.0
#define BWIDGETS_DEFAULT_HSWITCH_DEPTH 1.0

namespace BWidgets
{
/**
 * Class BWidgets::HSwitch
 *
 * On/OFF switch widget. Is is a BWidgets::ToggleButton and thus a
 * BWidgets::ValueWidget having two conditions: on (value != 0) or off
 * (value == 0)
 */
class HSwitch : public ToggleButton
{
public:
	HSwitch ();
	HSwitch (const double x, const double y, const double width, const double height, const std::string& name, const double defaultvalue);

	/**
	 * Creates a new (orphan) switch and copies the switch properties from a
	 * source switch. This method doesn't copy any parent or child widgets.
	 * @param that Source slider
	 */
	HSwitch (const HSwitch& that);

	~HSwitch ();

	/**
	 * Assignment. Copies the switch properties from a source slider and keeps
	 * its name and its position within the widget tree. Emits an expose event
	 * if the widget is visible and a value changed event.
	 * @param that Source slider
	 */
	HSwitch& operator= (const HSwitch& that);

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

protected:
	virtual void draw (const double x, const double y, const double width, const double height) override;

	HScale scale;
	Knob knob;
};

}

#endif /* BWIDGETS_HSWITCH_HPP_ */
