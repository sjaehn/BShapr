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
protected:
	double smoothingWidth;
	double fadeoutWidth;
public:
	HorizonWidget () : HorizonWidget (0, 0, 0, 0, 0, "horizon") {}
	HorizonWidget (const double x, const double y, const double smoothingWidth, const double fadeoutWidth, const double height, const std::string& name) :
		Widget (x, y, 2 * smoothingWidth + fadeoutWidth, height, name),
		smoothingWidth (smoothingWidth),
		fadeoutWidth (fadeoutWidth)
	{
		setFocusable (false);
	}

	void setSmoothingWidth (const double smoothingWidth)
	{
		if (this->smoothingWidth != smoothingWidth)
		{
			this->smoothingWidth = smoothingWidth;
			setWidth (2 * smoothingWidth + fadeoutWidth);
		}
	}

	void setFadeoutWidth (const double fadeoutWidth)
	{
		if (this->fadeoutWidth != fadeoutWidth)
		{
			this->fadeoutWidth = fadeoutWidth;
			setWidth (2 * smoothingWidth + fadeoutWidth);
		}
	}

	void moveLineTo (const double x, const double y) {moveTo (x - smoothingWidth, y);}

	double getLineX () const {return area_.getX() + smoothingWidth;}

protected:
	virtual void draw (const BUtilities::RectArea& area) override
	{
		if ((!widgetSurface_) || (cairo_surface_status (widgetSurface_) != CAIRO_STATUS_SUCCESS)) return;

		Widget::draw (area);

		cairo_t* cr = cairo_create (widgetSurface_);

		if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
		{
			// Limit cairo-drawing area
			cairo_rectangle (cr, area.getX(), area.getY(), area.getWidth(), area.getHeight());
			cairo_clip (cr);

			// Draw fade out
			if (fadeoutWidth > 0)
			{
				cairo_pattern_t* pat1 = cairo_pattern_create_linear (0, 0, getWidth(), 0);
				if (cairo_pattern_status (pat1) == CAIRO_STATUS_SUCCESS)
				{
					cairo_pattern_add_color_stop_rgba (pat1, 0.0, 0.0, 0.0, 0.0, 0.0);
					cairo_pattern_add_color_stop_rgba (pat1, smoothingWidth / getWidth(), 0.0, 0.0, 0.0, 0.0);
					cairo_pattern_add_color_stop_rgba (pat1, smoothingWidth / getWidth() + 0.001, CAIRO_RGBA (BColors::black));
					cairo_pattern_add_color_stop_rgba (pat1, 1.0 - (fadeoutWidth / getWidth()), CAIRO_RGBA (BColors::black));
					cairo_pattern_add_color_stop_rgba (pat1, 1.0, 0.0, 0.0, 0.0, 0.0);
					cairo_set_line_width (cr, 0.0);
					cairo_set_source (cr, pat1);
					cairo_rectangle (cr, 0, 0, getWidth(), getHeight());
					cairo_fill (cr);
					cairo_pattern_destroy (pat1);
				}
			}

			// Draw smoothing
			if (smoothingWidth > 0)
			{
				cairo_pattern_t* pat2 = cairo_pattern_create_linear (0, 0, 2 * smoothingWidth, 0);
				if (cairo_pattern_status (pat2) == CAIRO_STATUS_SUCCESS)
				{
					cairo_pattern_add_color_stop_rgba (pat2, 0.0, 1.0, 1.0, 1.0, 0.0);
					cairo_pattern_add_color_stop_rgba (pat2, 0.5, 1.0, 1.0, 1.0, 0.5);
					cairo_pattern_add_color_stop_rgba (pat2, 1.0, 1.0, 1.0, 1.0, 0.0);
					cairo_set_line_width (cr, 0.0);
					cairo_set_source (cr, pat2);
					cairo_rectangle (cr, 0, 0, 2 * smoothingWidth, getHeight());
					cairo_fill (cr);
					cairo_pattern_destroy (pat2);
				}
			}

			// Draw horizon line
			cairo_set_source_rgba (cr, CAIRO_RGBA (BColors::white));
			cairo_set_line_width (cr, 2.0);
			cairo_move_to (cr, smoothingWidth, 0.0);
			cairo_line_to (cr, smoothingWidth, getHeight());
			cairo_stroke (cr);

			cairo_destroy (cr);
		}
	}
};



#endif /* HORIZONWIDGET_HPP_ */
