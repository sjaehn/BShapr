/* HSwitch_classic.cpp
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

#include "HSwitch_classic.hpp"

#ifndef PI
#define PI 3.14159265
#endif

namespace BWidgets
{
HSwitch_classic::HSwitch_classic () : HSwitch_classic (0.0, 0.0, BWIDGETS_DEFAULT_HSWITCH_CLASSIC_WIDTH, BWIDGETS_DEFAULT_HSWITCH_CLASSIC_HEIGHT, "hswitch", 0.0) {}

HSwitch_classic::HSwitch_classic (const double x, const double y, const double width, const double height, const std::string& name, const double defaultValue) :
		ToggleButton (x, y, width, height, name, defaultValue), labelColors (BWIDGETS_DEFAULT_FGCOLORS) {}

HSwitch_classic::HSwitch_classic (const HSwitch_classic& that) : ToggleButton (that), labelColors (that.labelColors) {}

HSwitch_classic:: ~HSwitch_classic () {}

HSwitch_classic& HSwitch_classic::operator= (const HSwitch_classic& that)
{
	labelColors = that.labelColors;
	Button::operator= (that);
	return *this;
}

void HSwitch_classic::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}

void HSwitch_classic::applyTheme (BStyles::Theme& theme, const std::string& name)
{
	Widget::applyTheme (theme, name);

	void* btPtr = theme.getStyle(name, BWIDGETS_KEYWORD_BUTTONCOLORS);
	if (btPtr) buttonColors = *((BColors::ColorSet*) btPtr);

	void* bgPtr = theme.getStyle(name, BWIDGETS_KEYWORD_BGCOLORS);
	if (bgPtr) bgColors = *((BColors::ColorSet*) bgPtr);

	void* lbPtr = theme.getStyle(name, BWIDGETS_KEYWORD_TEXTCOLORS);
	if (lbPtr) labelColors = *((BColors::ColorSet*) lbPtr);

	if (btPtr || bgPtr || lbPtr) update ();
}

void HSwitch_classic::draw (const double x, const double y, const double width, const double height)
{
	if ((!widgetSurface) || (cairo_surface_status (widgetSurface) != CAIRO_STATUS_SUCCESS)) return;

	if ((width_ >= 6) && (height_ >= 6))
	{
		// Draw super class widget elements first
		Widget::draw (x, y, width, height);

		cairo_t* cr = cairo_create (widgetSurface);
		if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
		{
			cairo_pattern_t* pat;

			// Limit cairo-drawing area
			cairo_rectangle (cr, x, y, width, height);
			cairo_clip (cr);

			BColors::Color butColorMid = *buttonColors.getColor (BColors::NORMAL);
			BColors::Color butColorHi = *buttonColors.getColor (BColors::ACTIVE);
			BColors::Color butColorLo = *buttonColors.getColor (BColors::INACTIVE);
			BColors::Color bgColorHi = *bgColors.getColor (BColors::INACTIVE);
			BColors::Color bgColorLo = *bgColors.getColor (BColors::OFF);
			BColors::Color txColorHi = *labelColors.getColor (BColors::ACTIVE);
			BColors::Color txColorLo = *labelColors.getColor (BColors::INACTIVE);

			double w = getWidth () - 3;
			double h = getHeight () - 3;

			// Frame
			cairo_set_line_width (cr, 1.0);

			cairo_move_to (cr, 0.5, 2.5 + h);
			cairo_line_to (cr, 0.5, 0.5 + 0.05 * h);
			cairo_line_to (cr, 2.5 + w, 0.5 + 0.05 * h);
			cairo_set_source_rgba (cr, bgColorHi.getRed (), bgColorHi.getGreen (), bgColorHi.getGreen (), bgColorHi.getAlpha ());
			cairo_stroke (cr);

			cairo_move_to (cr, 2.5 + w, 0.5 + 0.05 * h);
			cairo_line_to (cr, 2.5 + w, 2.5 + h);
			cairo_line_to (cr, 0.5, 2.5 + h);
			cairo_set_source_rgba (cr, bgColorLo.getRed (), bgColorLo.getGreen (), bgColorLo.getGreen (), bgColorLo.getAlpha ());
			cairo_stroke (cr);

			pat = cairo_pattern_create_linear (1.5, 0, w, 0);
			if (pat && (cairo_pattern_status (pat) == CAIRO_STATUS_SUCCESS))
			{
				if (value)
				{
					cairo_move_to (cr, 1.5 + 0.45 * w, 1.5 + h);
					cairo_line_to (cr, 1.5 + 0.9 * w, 1.5 + 0.95 * h);
					cairo_line_to (cr, 1.5 + w, 1.5 + h);
					cairo_close_path (cr);
					cairo_set_source_rgba (cr, butColorLo.getRed (), butColorLo.getGreen (), butColorLo.getGreen (), butColorLo.getAlpha ());
					cairo_fill (cr);

					cairo_pattern_add_color_stop_rgba (pat, 0.0, butColorMid.getRed (), butColorMid.getGreen (), butColorMid.getBlue (), butColorMid.getAlpha ());
					cairo_pattern_add_color_stop_rgba (pat, 0.45, butColorMid.getRed (), butColorMid.getGreen (), butColorMid.getBlue (), butColorMid.getAlpha ());
					cairo_pattern_add_color_stop_rgba (pat, 0.9, butColorHi.getRed (), butColorHi.getGreen (), butColorHi.getBlue (), butColorHi.getAlpha ());
					cairo_pattern_add_color_stop_rgba (pat, 0.95, butColorLo.getRed (), butColorLo.getGreen (), butColorLo.getBlue (), butColorLo.getAlpha ());
					cairo_pattern_add_color_stop_rgba (pat, 1.0, butColorLo.getRed (), butColorLo.getGreen (), butColorLo.getBlue (), butColorLo.getAlpha ());

					cairo_move_to (cr, 1.5, 1.5 + 0.05 * h);
					cairo_line_to (cr, 1.5, 1.5 + h);
					cairo_line_to (cr, 1.5 + 0.45 * w, 1.5 + h);
					cairo_line_to (cr, 1.5 + 0.9 * w, 1.5 + 0.95 * h);
					cairo_line_to (cr, 1.5 + w, 1.5 + h);
					cairo_line_to (cr, 1.5 + w, 1.5 + 0.05 * h);
					cairo_line_to (cr, 1.5 + 0.9 * w, 1.5);
					cairo_line_to (cr, 1.5 + 0.45 * w, 1.5 + 0.05 * h);
					cairo_close_path (cr);
					cairo_set_source (cr, pat);
					cairo_fill (cr);

					// I
					cairo_set_source_rgba (cr, txColorHi.getRed (), txColorHi.getGreen (), txColorHi.getGreen (), txColorHi.getAlpha ());
					cairo_move_to (cr, 1.5 + 0.225 * w, 1.5 + 0.3 * h);
					cairo_line_to (cr, 1.5 + 0.225 * w, 1.5 + 0.75 * h);
					cairo_stroke (cr);

					// O
					cairo_set_source_rgba (cr, txColorLo.getRed (), txColorLo.getGreen (), txColorLo.getGreen (), txColorLo.getAlpha ());
					cairo_arc (cr, 1.5 + 0.675 * w, 1.5 + 0.525 * h, 0.2 * h, 0, 2 * PI);
					cairo_close_path (cr);
					cairo_stroke (cr);

				}
				else
				{
					cairo_move_to (cr, 1.5, 1.5 + h);
					cairo_line_to (cr, 1.5 + 0.1 * w, 1.5 + 0.95 * h);
					cairo_line_to (cr, 1.5 + 0.55 * w, 1.5 + h);
					cairo_close_path (cr);
					cairo_set_source_rgba (cr, butColorLo.getRed (), butColorLo.getGreen (), butColorLo.getGreen (), butColorLo.getAlpha ());
					cairo_fill (cr);

					cairo_pattern_add_color_stop_rgba (pat, 0.0, butColorMid.getRed (), butColorMid.getGreen (), butColorMid.getBlue (), butColorMid.getAlpha ());
					cairo_pattern_add_color_stop_rgba (pat, 0.05, butColorMid.getRed (), butColorMid.getGreen (), butColorMid.getBlue (), butColorMid.getAlpha ());
					cairo_pattern_add_color_stop_rgba (pat, 0.1, butColorHi.getRed (), butColorHi.getGreen (), butColorHi.getBlue (), butColorHi.getAlpha ());
					cairo_pattern_add_color_stop_rgba (pat, 0.55, butColorMid.getRed (), butColorMid.getGreen (), butColorMid.getBlue (), butColorMid.getAlpha ());
					cairo_pattern_add_color_stop_rgba (pat, 1.0, butColorMid.getRed (), butColorMid.getGreen (), butColorMid.getBlue (), butColorMid.getAlpha ());

					cairo_move_to (cr, 1.5, 1.5 + 0.05 * h);
					cairo_line_to (cr, 1.5, 1.5 + h);
					cairo_line_to (cr, 1.5 + 0.1 * w, 1.5 + 0.95 * h);
					cairo_line_to (cr, 1.5 + 0.55 * w, 1.5 + h);
					cairo_line_to (cr, 1.5 + w, 1.5 + h);
					cairo_line_to (cr, 1.5 + w, 1.5 + 0.05 * h);
					cairo_line_to (cr, 1.5 + 0.55 * w, 1.5 + 0.05 * h);
					cairo_line_to (cr, 1.5 + 0.1 * w, 1.5);
					cairo_close_path (cr);
					cairo_set_source (cr, pat);
					cairo_fill (cr);

					// I
					cairo_set_source_rgba (cr, txColorLo.getRed (), txColorLo.getGreen (), txColorLo.getGreen (), txColorLo.getAlpha ());
					cairo_move_to (cr, 1.5 + 0.325 * w, 1.5 + 0.275 * h);
					cairo_line_to (cr, 1.5 + 0.325 * w, 1.5 + 0.725 * h);
					cairo_stroke (cr);

					// O
					cairo_set_source_rgba (cr, txColorHi.getRed (), txColorHi.getGreen (), txColorHi.getGreen (), txColorHi.getAlpha ());
					cairo_arc (cr, 1.5 + 0.775 * w, 1.5 + 0.55 * h, 0.2 * h, 0, 2 * PI);
					cairo_close_path (cr);
					cairo_stroke (cr);
				}

				cairo_pattern_destroy (pat);
			}
		}
		cairo_destroy (cr);
	}
}

}
