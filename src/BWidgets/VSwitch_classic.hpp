/* VSwitch_classic.hpp
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

#ifndef BWIDGETS_VSWITCH_CLASSIC_HPP_
#define BWIDGETS_VSWITCH_CLASSIC_HPP_

#include "ToggleButton.hpp"

#define BWIDGETS_DEFAULT_VSWITCH_CLASSIC_WIDTH 20.0
#define BWIDGETS_DEFAULT_VSWITCH_CLASSIC_HEIGHT 40.0

namespace BWidgets
{
/**
 * Class BWidgets::VSwitch_classic
 *
 * On/OFF switch widget. Is is a BWidgets::ToggleButton and thus a
 * BWidgets::ValueWidget having two conditions: on (value != 0) or off
 * (value == 0)
 */
class VSwitch_classic : public ToggleButton
{
public:
	VSwitch_classic ();
	VSwitch_classic (const double x, const double y, const double width, const double height, const std::string& name, const double defaultValue);

	/**
	 * Creates a new (orphan) switch and copies the button properties from a
	 * source switch. This method doesn't copy any parent or child widgets.
	 * @param that Source switch
	 */
	VSwitch_classic (const VSwitch_classic& that);

	~VSwitch_classic ();

	/**
	 * Assignment. Copies the widget properties from a source switch and keeps
	 * its name and its position within the widget tree. Emits a
	 * BEvents::ExposeEvent if the widget is visible.
	 * @param that Source switch
	 */
	VSwitch_classic& operator= (const VSwitch_classic& that);

	/**
	 * Scans theme for widget properties and applies these properties.
	 * @param theme Theme to be scanned.
	 * @param name Name of the BStyles::StyleSet within the theme to be
	 * 		  	   applied.
	 */
	virtual void applyTheme (BStyles::Theme& theme);
	virtual void applyTheme (BStyles::Theme& theme, const std::string& name);

protected:
	virtual void draw (const double x, const double y, const double width, const double height) override;

	BColors::ColorSet labelColors;
};

}

#endif /* BWIDGETS_VSWITCH_CLASSIC_HPP_ */
