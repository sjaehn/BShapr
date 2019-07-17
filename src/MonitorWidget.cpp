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

 #include "MonitorWidget.hpp"

MonitorWidget::MonitorWidget () : MonitorWidget (0, 0, 0, 0, "monitor") {}

MonitorWidget::MonitorWidget (const double x, const double y, const double width, const double height, const std::string& name) :
        Widget (x, y, width, height, name),
        fgColors (BColors::whites), zoom (1)
{
        clear ();
        clickable = false;
}

void MonitorWidget::clear () {data.fill (0);}

void MonitorWidget::addData (const uint pos, const double value)
{
        uint nr = LIMIT (pos, 0, MONITORBUFFERSIZE - 1);
        data[nr] = value;
}

void MonitorWidget::setZoom (const double factor)
{
        zoom = factor;
        update ();
}
double MonitorWidget::getZoom () const {return zoom;}

void MonitorWidget::redrawRange (const uint start, const uint end)
{
        double xabs = getOriginX ();
        double yabs = getOriginY ();
        double x1 = width_ * start / (MONITORBUFFERSIZE - 1);
        double w1 = width_ * (end - start) / (MONITORBUFFERSIZE - 1);

        draw (x1, 0, w1, height_);
        if (isVisible ()) postRedisplay (xabs + x1, yabs, w1, height_);
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

void MonitorWidget::drawData (cairo_t* cr, const double x, const double y, const double width, const double height)
{
        int start = x / width_ * (MONITORBUFFERSIZE - 1) - 1;
        int end = (x + width) / width_ * (MONITORBUFFERSIZE - 1) + 1;
        start = LIMIT (start, 0, MONITORBUFFERSIZE - 1);
        end = LIMIT (end, 0, MONITORBUFFERSIZE - 1);

        cairo_set_line_width (cr, 1.5);
        cairo_move_to (cr, width_ * double (start) / (MONITORBUFFERSIZE - 1), height_ * (0.5  - (0.48 * LIMIT ((data[start] / zoom), -1, 1))));
        for (int i = start + 1; (i < MONITORBUFFERSIZE) && (i <= end); ++i)
        {
                cairo_line_to (cr, width_ * double (i) / (MONITORBUFFERSIZE - 1), height_ * (0.5  - (0.48 * LIMIT ((data[i] / zoom), -1, 1))));
        }
        cairo_stroke (cr);
}

void MonitorWidget::draw (const double x, const double y, const double width, const double height)
{
        if ((!widgetSurface) || (cairo_surface_status (widgetSurface) != CAIRO_STATUS_SUCCESS)) return;

	// Draw super class widget elements first
	// Widget::draw (x, y, width, height);

	cairo_t* cr = cairo_create (widgetSurface);

	if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
	{
		// Limit cairo-drawing area
                cairo_set_line_width (cr, 0);
		cairo_rectangle (cr, x, y, width, height);
		cairo_clip (cr);

		cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 0.0);
		cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
		cairo_paint (cr);

                cairo_set_source_rgba (cr, CAIRO_RGBA ((*fgColors.getColor (getState ()))));
                drawData (cr, x, y, width, height);

                cairo_destroy (cr);
        }
}
