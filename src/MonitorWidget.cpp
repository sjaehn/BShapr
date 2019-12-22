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

#include "MonitorWidget.hpp"
#include <cmath>

MonitorWidget::MonitorWidget () : MonitorWidget (0, 0, 0, 0, "monitor") {}

MonitorWidget::MonitorWidget (const double x, const double y, const double width, const double height, const std::string& name) :
        Widget (x, y, width, height, name),
        fgColors (BColors::whites), zoom (0.25), pat (nullptr)
{
        clear ();
        setClickable (false);
        makePattern ();
}

MonitorWidget::~MonitorWidget ()
{
        if (pat) cairo_pattern_destroy (pat);
}

void MonitorWidget::clear () {data.fill ({0.0f, 0.0f});}

void MonitorWidget::addData (const unsigned int pos, const Range range)
{
        unsigned int nr = LIMIT (pos, 0, MONITORBUFFERSIZE - 1);
        data[nr] = range;
}

void MonitorWidget::setZoom (const double factor)
{
        zoom = factor;
        update ();
}
double MonitorWidget::getZoom () const {return zoom;}

void MonitorWidget::redrawRange (const unsigned int start, const unsigned int end)
{
        unsigned int s = LIMIT (int (start) - 1, 0, MONITORBUFFERSIZE - 1);
        unsigned int e = LIMIT (end + 1, 0, MONITORBUFFERSIZE - 1);
        double xabs = getAbsolutePosition().x;
        double yabs = getAbsolutePosition().y;
        double x1 = getWidth() * s / (MONITORBUFFERSIZE - 1);
        double w = getWidth() * (e - s) / (MONITORBUFFERSIZE - 1);

        drawData (s, e);
        if (isVisible ()) postRedisplay (BUtilities::RectArea (xabs + x1, yabs, w, getHeight()));
}

void MonitorWidget::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}

void MonitorWidget::applyTheme (BStyles::Theme& theme, const std::string& name)
{
	Widget::applyTheme (theme, name);

	void* fgPtr = theme.getStyle(name, BWIDGETS_KEYWORD_FGCOLORS);
	if (fgPtr)
	{
		fgColors = *((BColors::ColorSet*) fgPtr);
		update ();
	}
}

void MonitorWidget::update ()
{
        makePattern ();
        Widget::update ();
}

void MonitorWidget::makePattern ()
{
        if (pat) cairo_pattern_destroy (pat);
        pat = cairo_pattern_create_linear (0, 0, 0, getHeight());
        BColors::Color col = *fgColors.getColor (getState ());
        cairo_pattern_add_color_stop_rgba (pat, 1, col.getRed (), col.getGreen (), col.getBlue (), 0.6 * col.getAlpha ());
        cairo_pattern_add_color_stop_rgba (pat, 0.5, col.getRed (), col.getGreen (), col.getBlue (), 0.1 * col.getAlpha ());
        cairo_pattern_add_color_stop_rgba (pat, 0, col.getRed (), col.getGreen (), col.getBlue (), 0.6 * col.getAlpha ());
}

void MonitorWidget::drawData (const unsigned int start, const unsigned int end)
{
        if ((!widgetSurface_) || (cairo_surface_status (widgetSurface_) != CAIRO_STATUS_SUCCESS)) return;

        BColors::Color col = *fgColors.getColor (getState ());
	cairo_t* cr = cairo_create (widgetSurface_);

	if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
	{
		// Limit cairo-drawing area
                cairo_set_line_width (cr, 0);
                double x0 = ceil (getWidth() * start / (MONITORBUFFERSIZE - 1));
                double x1 = floor (getWidth() * end / (MONITORBUFFERSIZE - 1));
		cairo_rectangle (cr, x0, 0, x1 - x0, getHeight());
		cairo_clip (cr);

		cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 0.0);
		cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
		cairo_paint (cr);

                cairo_set_line_width (cr, 2);
                cairo_move_to (cr, getWidth() * double (start) / (MONITORBUFFERSIZE - 1), getHeight() * (0.5  - (0.48 * LIMIT ((data[start].max / zoom), -1, 1))));
                for (int i = start + 1; i <= int (end); ++i)
                {
                        cairo_line_to (cr, getWidth() * double (i) / (MONITORBUFFERSIZE - 1), getHeight() * (0.5  - (0.48 * LIMIT ((data[i].max / zoom), -1, 1))));
                }
                cairo_set_source_rgba (cr, CAIRO_RGBA (col));
                cairo_stroke_preserve (cr);
                cairo_set_line_width (cr, 0);
                cairo_line_to (cr, getWidth() * double (end) / (MONITORBUFFERSIZE - 1), getHeight() * 0.5);
                cairo_line_to (cr, getWidth() * double (start) / (MONITORBUFFERSIZE - 1), getHeight() * 0.5);
                cairo_close_path (cr);
                cairo_set_source (cr, pat);
                cairo_fill (cr);

                cairo_set_line_width (cr, 2);
                cairo_move_to (cr, getWidth() * double (start) / (MONITORBUFFERSIZE - 1), getHeight() * (0.5  - (0.48 * LIMIT ((data[start].min / zoom), -1, 1))));
                for (int i = start + 1; i <= int (end); ++i)
                {
                        cairo_line_to (cr, getWidth() * double (i) / (MONITORBUFFERSIZE - 1), getHeight() * (0.5  - (0.48 * LIMIT ((data[i].min / zoom), -1, 1))));
                }
                cairo_set_source_rgba (cr, CAIRO_RGBA (col));
                cairo_stroke_preserve (cr);
                cairo_set_line_width (cr, 0);
                cairo_line_to (cr, getWidth() * double (end) / (MONITORBUFFERSIZE - 1), getHeight() * 0.5);
                cairo_line_to (cr, getWidth() * double (start) / (MONITORBUFFERSIZE - 1), getHeight() * 0.5);
                cairo_close_path (cr);
                cairo_set_source (cr, pat);
                cairo_fill (cr);

                cairo_destroy (cr);
        }
}

void MonitorWidget::draw (const BUtilities::RectArea& area)
{
        drawData (0, MONITORBUFFERSIZE - 1);
}
