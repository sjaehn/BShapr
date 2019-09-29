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

#ifndef HORIZONWIDGET_HPP_
#define HORIZONWIDGET_HPP_

#include "BWidgets/Widget.hpp"

class HorizonWidget : public BWidgets::Widget
{
public:
	HorizonWidget () : HorizonWidget (0, 0, 0, 0, "horizon") {}
	HorizonWidget (const double x, const double y, const double width, const double height, const std::string& name) :
		Widget (x, y, width, height, name) {}

protected:
	virtual void draw (const double x, const double y, const double width, const double height) override
	{
		if ((!widgetSurface) || (cairo_surface_status (widgetSurface) != CAIRO_STATUS_SUCCESS)) return;

		Widget::draw (x, y, width, height);

		cairo_t* cr = cairo_create (widgetSurface);

		if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
		{
			// Limit cairo-drawing area
			cairo_rectangle (cr, x, y, width, height);
			cairo_clip (cr);

			double x0 = getXOffset ();
			double y0 = getYOffset ();
			double w = getEffectiveWidth ();
			double h = getEffectiveHeight ();

			// Draw fade out
			cairo_pattern_t* pat = cairo_pattern_create_linear (0, 0, w, 0);
			if (cairo_pattern_status (pat) == CAIRO_STATUS_SUCCESS)
			{
				cairo_pattern_add_color_stop_rgba (pat, 0.0, CAIRO_RGBA (BColors::black));
				cairo_pattern_add_color_stop_rgba (pat, 1.0, 0.0, 0.0, 0.0, 0.0);
				cairo_set_line_width (cr, 0.0);
				cairo_set_source (cr, pat);
				cairo_rectangle (cr, x0, y0, w, h);
				cairo_fill (cr);
				cairo_pattern_destroy (pat);
			}

			// Draw horizon line
			cairo_set_source_rgba (cr, CAIRO_RGBA (BColors::white));
			cairo_set_line_width (cr, 2);
			cairo_move_to (cr, 1.0, 0.0);
			cairo_line_to (cr, 1.0, h);
			cairo_stroke (cr);

			cairo_destroy (cr);
		}
	}
};



#endif /* HORIZONWIDGET_HPP_ */
