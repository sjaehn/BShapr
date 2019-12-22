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

#ifndef SYMBOLWIDGET_HPP_
#define SYMBOLWIDGET_HPP_

#include "BWidgets/Widget.hpp"

enum SWSymbol
{
        NOSYMBOL        = -1,
        ADDSYMBOL       = 0,
        CLOSESYMBOL     = 1,
        LEFTSYMBOL      = 2,
        RIGHTSYMBOL     = 3,
        NRSYMBOLS       = 4
};

class SymbolWidget : public BWidgets::Widget
{
protected:
        BColors::ColorSet fgColors;
        SWSymbol symbol;

public:
        SymbolWidget () : SymbolWidget (0.0, 0.0, 0.0, 0.0, "symbol", NOSYMBOL) {}
        SymbolWidget (const double x, const double y, const double width, const double height, const std::string& name, SWSymbol symbol) :
                Widget (x, y, width, height, name),
                fgColors (BColors::whites),
                symbol (symbol) {}

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

                        double x0 = getXOffset ();
			double y0 = getYOffset ();
			double w = getEffectiveWidth ();
			double h = getEffectiveHeight ();

                        switch (symbol)
                        {
                                case ADDSYMBOL:         cairo_move_to (cr, x0, y0 + h / 2);
                                                        cairo_line_to (cr, x0 + w, y0 + h / 2);
                                                        cairo_move_to (cr, x0 + w / 2, y0);
                                                        cairo_line_to (cr, x0 + w / 2, y0 + h);
                                                        break;

                                case CLOSESYMBOL:       cairo_move_to (cr, x0, y0 + h / 2);
                                                        cairo_line_to (cr, x0 + w, y0 + h / 2);
                                                        break;

                                case LEFTSYMBOL:        cairo_move_to (cr, x0 + 0.75 * w, y0);
                                                        cairo_line_to (cr, x0 + 0.25 * w, y0 + 0.5 * h);
                                                        cairo_line_to (cr, x0 + 0.75 * w, y0 + h);
                                                        break;

                                case RIGHTSYMBOL:       cairo_move_to (cr, x0 + 0.25 * w, y0);
                                                        cairo_line_to (cr, x0 + 0.75 * w, y0 + 0.5 * h);
                                                        cairo_line_to (cr, x0 + 0.25 * w, y0 + h);
                                                        break;

                                default:                break;
                        }

                        cairo_set_line_width (cr, 2.0);
                        cairo_set_source_rgba (cr, CAIRO_RGBA (*fgColors.getColor (getState ())));
                        cairo_stroke (cr);

                        cairo_destroy (cr);
		}
	}

};

#endif /* SYMBOLWIDGET_HPP_ */
