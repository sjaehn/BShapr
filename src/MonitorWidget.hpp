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

 #ifndef MONITORWIDGET_HPP_
 #define MONITORWIDGET_HPP_

 #include "BWidgets/Widget.hpp"
 #include "definitions.h"

 class MonitorWidget : public BWidgets::Widget
 {
 public:
        MonitorWidget ();
        MonitorWidget (const double x, const double y, const double width, const double height, const std::string& name);
        ~MonitorWidget ();

        void clear ();
        void addData (const uint pos, const Range range);
        void setZoom (const double factor);
        double getZoom () const;
        void redrawRange (const uint start, const uint end);
        virtual void applyTheme (BStyles::Theme& theme) override;
	virtual void applyTheme (BStyles::Theme& theme, const std::string& name) override;
        virtual void update () override;

protected:
        void makePattern ();
        void drawData (const uint start, const uint end);
        virtual void draw (const double x, const double y, const double width, const double height) override;

        std::array<Range, MONITORBUFFERSIZE> data;
        BColors::ColorSet fgColors;
        double zoom;
        cairo_pattern_t* pat;
 };

 #endif /* MONITORWIDGET_HPP_ */
