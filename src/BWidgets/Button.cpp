/* Button.cpp
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

#include "Button.hpp"

namespace BWidgets
{
Button::Button () : Button (0.0, 0.0, BWIDGETS_DEFAULT_BUTTON_WIDTH, BWIDGETS_DEFAULT_BUTTON_HEIGHT, "button", 0.0) {}

Button::Button (const double x, const double y, const double width, const double height, const std::string& name, double defaultValue) :
		ValueWidget (x, y, width, height, name, defaultValue),
		buttonColors (BWIDGETS_DEFAULT_BGCOLORS), bgColors (BWIDGETS_DEFAULT_BGCOLORS)
{
	setClickable (true);
}

Button::Button (const Button& that) : ValueWidget (that), buttonColors (that.buttonColors), bgColors (that.buttonColors) {}

Button:: ~Button () {}

Button& Button::operator= (const Button& that)
{
	buttonColors = that.buttonColors;
	bgColors = that.bgColors;
	ValueWidget::operator= (that);
	return *this;
}

void Button::setButtonColors (const BColors::ColorSet& colors)
{
	buttonColors = colors;
	update ();
}

BColors::ColorSet* Button::getButtonColors () {return &buttonColors;}

void Button::update ()
{
	draw (0, 0, width_, height_);
	if (isVisible ()) postRedisplay ();
}

void Button::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}

void Button::applyTheme (BStyles::Theme& theme, const std::string& name)
{
	Widget::applyTheme (theme, name);

	void* btPtr = theme.getStyle(name, BWIDGETS_KEYWORD_BUTTONCOLORS);
	if (btPtr) buttonColors = *((BColors::ColorSet*) btPtr);

	void* bgPtr = theme.getStyle(name, BWIDGETS_KEYWORD_BGCOLORS);
	if (bgPtr) bgColors = *((BColors::ColorSet*) bgPtr);

	if (btPtr || bgPtr) update ();
}

void Button::onButtonPressed (BEvents::PointerEvent* event) {setValue (1.0);}

void Button::onButtonReleased (BEvents::PointerEvent* event) {setValue (0.0);}

void Button::draw (const double x, const double y, const double width, const double height)
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

			BColors::Color butColorLo = *bgColors.getColor (BColors::NORMAL); butColorLo.applyBrightness (BWIDGETS_DEFAULT_NORMALLIGHTED);
			BColors::Color butColorHi = *bgColors.getColor (BColors::NORMAL); butColorHi.applyBrightness (BWIDGETS_DEFAULT_ILLUMINATED);
			BColors::Color butColorMid = *bgColors.getColor (BColors::NORMAL); butColorMid.applyBrightness ((BWIDGETS_DEFAULT_NORMALLIGHTED + BWIDGETS_DEFAULT_ILLUMINATED) / 2);
			BColors::Color butColorSh = *bgColors.getColor (BColors::NORMAL); butColorSh.applyBrightness (BWIDGETS_DEFAULT_SHADOWED);


			double xb, yb, wb, hb;
			double xf, yf, wf, hf;

			cairo_set_line_width (cr, 0.0);

			if (value)
			{
				xb = 0.5 + BWIDGETS_DEFAULT_BUTTON_DEPTH;
				yb = 0.5 + BWIDGETS_DEFAULT_BUTTON_DEPTH;
				hf = 0.0;
			}
			else
			{
				xb = 0.5;
				yb = 0.5;
				hf = BWIDGETS_DEFAULT_BUTTON_DEPTH;
			}

			wb = width_ - 1;
			hb = height_ - 1 - BWIDGETS_DEFAULT_BUTTON_DEPTH;
			xf = xb;
			yf = yb + hb;
			wf = wb;

			// Button top
			pat = cairo_pattern_create_radial (0.75 * width_, 0.75 * height_, 0.1 * width_,  0.75 * width_, 0.75 * height_, 1.5 * width_);
			if (pat && (cairo_pattern_status (pat) == CAIRO_STATUS_SUCCESS))
			{
				cairo_pattern_add_color_stop_rgba (pat, 0, butColorMid.getRed (), butColorMid.getGreen (), butColorMid.getBlue (), butColorMid.getAlpha ());
				cairo_pattern_add_color_stop_rgba (pat, 1, butColorLo.getRed (), butColorLo.getGreen (), butColorLo.getBlue (), butColorLo.getAlpha ());
				cairo_rectangle (cr, xb, yb, wb, hb);
				cairo_set_source (cr, pat);
				cairo_fill (cr);
				cairo_pattern_destroy (pat);
			}

			// Button front
			pat = cairo_pattern_create_linear (xb, yb, xf + wf, yf + hf);
			if (pat && (cairo_pattern_status (pat) == CAIRO_STATUS_SUCCESS))
			{
				cairo_pattern_add_color_stop_rgba (pat, 1, butColorHi.getRed (), butColorHi.getGreen (), butColorHi.getBlue (), butColorHi.getAlpha ());
				cairo_pattern_add_color_stop_rgba (pat, 0, butColorSh.getRed (), butColorSh.getGreen (), butColorSh.getBlue (), butColorSh.getAlpha ());
				cairo_move_to (cr, xb, yf);
				cairo_line_to (cr, xb + wb, yf);
				cairo_line_to (cr, xb + wb, yb);
				cairo_line_to (cr, xb + wb + hf, yb + hf);
				cairo_line_to (cr, xb + wb + hf, yf + hf);
				cairo_line_to (cr, xb + hf, yf + hf);
				cairo_close_path (cr);
				cairo_set_source (cr, pat);
				cairo_fill (cr);

				// Button edges
				cairo_set_line_width (cr, 0.2 * BWIDGETS_DEFAULT_BUTTON_DEPTH);
				cairo_rectangle (cr, xb, yb, wb, hb);
				cairo_set_source (cr, pat);
				cairo_stroke (cr);

				cairo_move_to (cr, xb + wb, yf);
				cairo_line_to (cr, xb + wb + hf, yf + hf);
				cairo_set_source_rgba (cr, butColorHi.getRed (), butColorHi.getGreen (), butColorHi.getBlue (), butColorHi.getAlpha ());
				cairo_stroke (cr);

				cairo_pattern_destroy (pat);
			}
		}
		cairo_destroy (cr);
	}
}

}
