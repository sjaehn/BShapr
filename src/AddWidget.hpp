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

#ifndef ADDWIDGET_HPP_
#define ADDWIDGET_HPP_

#include "BWidgets/Widget.hpp"

class AddWidget : public BWidgets::Widget
{
protected:
        BColors::ColorSet fgColors;

public:
        AddWidget () : AddWidget (0.0, 0.0, 0.0, 0.0, "symbol") {}
        AddWidget (const double x, const double y, const double width, const double height, const std::string& name) :
                Widget (x, y, width, height, name),
                fgColors (BColors::whites) {}

	virtual void applyTheme (BStyles::Theme& theme, const std::string& name) override
        {
                Widget::applyTheme (theme, name);

        	void* colorsPtr = theme.getStyle(name, BWIDGETS_KEYWORD_FGCOLORS);
        	if (colorsPtr)
                {
                        fgColors = *((BColors::ColorSet*) colorsPtr);
                        update ();
                }
        }

        virtual void applyTheme (BStyles::Theme& theme) override {applyTheme (theme, name_);}

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

                        cairo_move_to (cr, x0, y0 + h / 2);
                        cairo_line_to (cr, x0 + w, y0 + h / 2);
                        cairo_move_to (cr, x0 + w / 2, y0);
                        cairo_line_to (cr, x0 + w / 2, y0 + h);
                        cairo_set_line_width (cr, 2.0);
                        cairo_set_source_rgba (cr, CAIRO_RGBA (*fgColors.getColor (getState ())));
                        cairo_stroke (cr);

                        cairo_destroy (cr);
		}
	}

};

#endif /* ADDWIDGET_HPP_ */
