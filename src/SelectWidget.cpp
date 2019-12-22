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

#include "SelectWidget.hpp"

SelectWidget::SelectWidget () : SelectWidget (0, 0, 0, 0, "tool", 0, 0, 0, 0) {}

SelectWidget::SelectWidget (const double  x, const double y, const double width, const double height, const std::string& name,
							const double selectionWidth, const double selectionHeight, const double nrTools, const double value) :
	RangeWidget (x, y, width, height, name, value, 0, nrTools, 1),
	bgColors (BColors::greys), nrTools (nrTools),
	selectionWidth (selectionWidth), selectionHeight (selectionHeight) {}

void SelectWidget::resizeSelection (const double width, const double height)
{
	if ((selectionWidth != width) || (selectionHeight != height))
	{
		selectionWidth = width;
		selectionHeight = height;
		update ();
	}
}

void SelectWidget::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}

void SelectWidget::applyTheme (BStyles::Theme& theme, const std::string& name)
{
	Widget::applyTheme (theme, name);

	// Background colors (scale background, knob)
	void* bgPtr = theme.getStyle(name, BWIDGETS_KEYWORD_BGCOLORS);
	if (bgPtr) bgColors = *((BColors::ColorSet*) bgPtr);

	if (bgPtr) update ();
}

void SelectWidget::onButtonPressed (BEvents::PointerEvent* event)
{
	if (event->getButton () == BDevices::LEFT_BUTTON)
	{
		double w = selectionWidth;
		double sp = (nrTools >= 2 ? ((getWidth() - w * nrTools) / (nrTools - 1)) : 0);
		if (w + sp > 0)
		{
			int t = event->getPosition().x / (w + sp);
			if (event->getPosition().x <= t * (w + sp) + w)
			{
				if (t + 1 == getValue ()) setValue (0);
				else setValue (t + 1);
			}
		}
	}
}

void SelectWidget::draw (const BUtilities::RectArea& area)
{
	if ((!widgetSurface_) || (cairo_surface_status (widgetSurface_) != CAIRO_STATUS_SUCCESS)) return;

	if ((getWidth() >= 1) && (getHeight() >= 1))
	{
		// Draw super class widget elements first
		Widget::draw (area);

		cairo_t* cr = cairo_create (widgetSurface_);
		if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
		{
			if ((value != 0) && (nrTools >= 1))
			{
				double w = selectionWidth;
				double sp = (nrTools >= 2 ? (getWidth() - w * nrTools) / (nrTools - 1): 0);
				BColors::Color bColor = *bgColors.getColor (BColors::NORMAL);
				cairo_rectangle (cr, 0.5 + (value - 1) * (w + sp), 0.5, w - 1, selectionHeight - 1);
				cairo_set_line_width (cr, 1);
				cairo_set_source_rgba (cr, CAIRO_RGBA (bColor));
				cairo_stroke (cr);
			}

			cairo_destroy (cr);
		}
	}
}
