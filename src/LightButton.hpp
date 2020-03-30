/* B.Shapr
 * Step Sequencer Effect Plugin
 *
 * Copyright (C) 2018 - 2020 by Sven Jähnichen
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

#ifndef LIGHTBUTTON_HPP_
#define LIGHTBUTTON_HPP_

#include "BWidgets/ToggleButton.hpp"
#include <cmath>

class LightButton : public BWidgets::ToggleButton
{
public:
        LightButton () :
        LightButton (0.0, 0.0, BWIDGETS_DEFAULT_BUTTON_WIDTH, BWIDGETS_DEFAULT_BUTTON_HEIGHT, "togglebutton", 0.0) {}

        LightButton (const double x, const double y, const double width, const double height, const std::string& name, double defaultValue = 0.0) :
        		ToggleButton (x, y, width, height, name, defaultValue),
                        fgColors (BWIDGETS_DEFAULT_FGCOLORS)
        {}

        virtual Widget* clone () const override {return new LightButton (*this);}

        virtual void applyTheme (BStyles::Theme& theme) override {applyTheme (theme, name_);}
	virtual void applyTheme (BStyles::Theme& theme, const std::string& name) override
        {
                ToggleButton::applyTheme (theme, name);

        	void* fgPtr = theme.getStyle(name, BWIDGETS_KEYWORD_FGCOLORS);
        	if (fgPtr)
        	{
        		fgColors = *((BColors::ColorSet*) fgPtr);
        		update ();
        	}
        }

protected:
        BColors::ColorSet fgColors;

        virtual void draw (const BUtilities::RectArea& area) override
        {
                if ((!widgetSurface_) || (cairo_surface_status (widgetSurface_) != CAIRO_STATUS_SUCCESS)) return;

        	// Draw super class widget elements first
        	Widget::draw (area);

        	double heff = getEffectiveHeight ();
        	double weff = getEffectiveWidth ();

        	// Draw knob
        	// only if minimum requirements satisfied
        	if ((getHeight () >= 1) && (getWidth () >= 1))
        	{
        		cairo_surface_clear (widgetSurface_);
        		cairo_t* cr = cairo_create (widgetSurface_);

        		if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
        		{
        			// Limit cairo-drawing area
        			cairo_rectangle (cr, area.getX (), area.getY (), area.getWidth (), area.getHeight ());
        			cairo_clip (cr);

                                // Calculate aspect ratios first
                                const double knobDepth = 1.0;
        			double size = (heff < weff ? heff - 2 * knobDepth : weff - 2 * knobDepth);
        			double x0 = weff / 2 - size / 2;
        			double y0 = heff / 2 - size / 2;

        			// Colors uses within this method
                                BColors::Color tLo = *fgColors.getColor (getState ()); tLo.applyBrightness (BWIDGETS_DEFAULT_NORMALLIGHTED);
                                BColors::Color tMid = *fgColors.getColor (getState ()); tMid.applyBrightness (BWIDGETS_DEFAULT_ILLUMINATED);
        			BColors::Color cHi = *bgColors.getColor (getState ()); cHi.applyBrightness (BWIDGETS_DEFAULT_ILLUMINATED);
                                BColors::Color cLo = *bgColors.getColor (getState ()); cLo.applyBrightness (BWIDGETS_DEFAULT_NORMALLIGHTED);
        			BColors::Color cMid = *bgColors.getColor (getState ()); cMid.applyBrightness ((BWIDGETS_DEFAULT_ILLUMINATED + BWIDGETS_DEFAULT_NORMALLIGHTED) / 2);
        			BColors::Color cSh = *bgColors.getColor (getState ()); cSh.applyBrightness (BWIDGETS_DEFAULT_SHADOWED);

        			cairo_set_line_width (cr, 0.0);

        			// Knob top
        			cairo_pattern_t* pat = cairo_pattern_create_radial (x0 + 0.75 * size - knobDepth, y0 + 0.75 * size - knobDepth, 0.1 * size,
        					       x0 + 0.5 * size - knobDepth, y0 + 0.5 * size - knobDepth, 0.75 * size);
        			if (pat && (cairo_pattern_status (pat) == CAIRO_STATUS_SUCCESS))
        			{
        				cairo_arc (cr,  x0 + 0.5 * size - knobDepth, y0 + 0.5 * size - knobDepth, size / 2, 0, 2 * M_PI);
        				cairo_close_path (cr);
                                        if (value)
                                        {
                				cairo_pattern_add_color_stop_rgba (pat, 0, CAIRO_RGBA (tMid));
                				cairo_pattern_add_color_stop_rgba (pat, 1, CAIRO_RGBA (tLo));
                                        }
                                        else
                                        {
                                                cairo_pattern_add_color_stop_rgba (pat, 0, CAIRO_RGBA (cMid));
                				cairo_pattern_add_color_stop_rgba (pat, 1, CAIRO_RGBA (cLo));
                                        }
        				cairo_set_source (cr, pat);
        				cairo_fill (cr);
        				cairo_pattern_destroy (pat);
        			}

        			// Knob side
        			pat = cairo_pattern_create_linear (x0 - knobDepth, y0, x0 + size, y0);
        			if (pat && (cairo_pattern_status (pat) == CAIRO_STATUS_SUCCESS))
        			{
        				cairo_pattern_add_color_stop_rgba (pat, 0, cMid.getRed (), cMid.getGreen (), cMid.getBlue (), cMid.getAlpha ());
        				cairo_pattern_add_color_stop_rgba (pat, 0.825, cHi.getRed (), cHi.getGreen (), cHi.getBlue (), cHi.getAlpha ());
        				cairo_pattern_add_color_stop_rgba (pat, 1, cMid.getRed (), cMid.getGreen (), cMid.getBlue (), cMid.getAlpha ());
        				cairo_arc (cr,  x0 + 0.5 * size - knobDepth, y0 + 0.5 * size - knobDepth, size / 2, -0.25 * M_PI, 0.75 * M_PI);
        				cairo_line_to (cr, x0, y0 + 0.5 * size);
        				cairo_arc_negative (cr, x0 + 0.5 * size, y0 + 0.5 * size, size / 2, 0.75 * M_PI, -0.25 * M_PI);
        				cairo_close_path (cr);
        				cairo_set_line_width (cr, 0.5);
        				cairo_set_source (cr, pat);
        				cairo_fill (cr);
        				cairo_pattern_destroy (pat);
        			}

        			// Knob edge
        			pat = cairo_pattern_create_linear
        			(
        				x0 + 0.5 * size * (1 + cos (0.25 * M_PI)) - knobDepth,
        				y0 + 0.5 * size * (1 + sin (0.25 * M_PI)) - knobDepth,
        				x0 + 0.5 * size * (1 - cos (0.25 * M_PI)) - knobDepth,
        				y0 + 0.5 * size * (1 - sin (0.25 * M_PI)) - knobDepth
        			);

        			if (pat && (cairo_pattern_status (pat) == CAIRO_STATUS_SUCCESS))
        			{
        				cairo_pattern_add_color_stop_rgba (pat, 0, cHi.getRed (), cHi.getGreen (), cHi.getBlue (), cHi.getAlpha ());
        				cairo_pattern_add_color_stop_rgba (pat, 1, cSh.getRed (), cSh.getGreen (), cSh.getBlue (), cSh.getAlpha ());
        				cairo_arc (cr,  x0 + 0.5 * size - knobDepth, y0 + 0.5 * size - knobDepth, size / 2, 0, 2 * M_PI);
        				cairo_set_line_width (cr, 0.2 * knobDepth);
        				cairo_set_source (cr, pat);
        				cairo_stroke (cr);
        				cairo_pattern_destroy (pat);
        			}
        		}

        		cairo_destroy (cr);
        	}
        }
};

#endif /* LIGHTBUTTON_HPP_ */
