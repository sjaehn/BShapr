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

#ifndef OPTION_HPP_
#define OPTION_HPP_

#include <string>
#include "Limit.hpp"

enum WidgetType
{
	NO_WIDGET,
	DIAL_WIDGET,
	SLIDER_WIDGET,
	SWITCH_WIDGET,
	POPUP_WIDGET
};

struct Option
{
	WidgetType widgetType;
	std::string name;
	double value;
	Limit limit;
	std::string format;
};

#endif /* OPTION_HPP_ */
