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

#ifndef SHAPEWIDGET_HPP_
#define SHAPEWIDGET_HPP_

#include "definitions.h"
#include "StaticArrayList.hpp"
#include "BWidgets/ValueWidget.hpp"
#include "Shape.hpp"

enum ToolType
{
	NO_TOOL											= 0,
	POINT_NODE_TOOL							= 1,
	AUTO_SMOOTH_NODE_TOOL				= 2,
	SYMMETRIC_SMOOTH_NODE_TOOL	= 3,
	CORNER_NODE_TOOL						= 4,
	DELETE_TOOL									= 5
};

class ShapeWidget : public Shape<MAXNODES>, public BWidgets::ValueWidget
{
public:
	ShapeWidget ();
	ShapeWidget (const double x, const double y, const double width, const double height, const std::string& name);
	ShapeWidget (const ShapeWidget& that);

	ShapeWidget& operator= (const ShapeWidget& that);

	void setTool (const ToolType tool);
	void setValueEnabled (const bool status);
	void setScaleParameters (double anchorYPos, double anchorValue, double ratio);
	void setMinorXSteps (double stepSize);
	void setMajorXSteps (double stepSize);
	void setPrefix (std::string text);
	void setUnit (std::string text);
	void setLowerLimit (double value, bool hard = false);
	void setHigherLimit (double value, bool hard = false);
	virtual void onButtonPressed (BEvents::PointerEvent* event) override;
	virtual void onButtonReleased (BEvents::PointerEvent* event) override;
	virtual void onPointerDragged (BEvents::PointerEvent* event) override;
	virtual void onWheelScrolled (BEvents::WheelEvent* event) override;
	virtual void onValueChanged (BEvents::ValueChangedEvent* event) override;
	virtual void applyTheme (BStyles::Theme& theme) override;
	virtual void applyTheme (BStyles::Theme& theme, const std::string& name) override;

protected:
	ToolType tool;
	int activeNode;
	int activeHandle;
	bool selected, dragged;
	bool valueEnabled;
	double scaleAnchorYPos;
	double scaleAnchorValue;
	double scaleRatio;
	double minorXSteps;
	double majorXSteps;
	double loLimit, hiLimit;
	bool hardLoLimit, hardHiLimit;	// TODO, not in use yet
	std::string prefix;
	std::string unit;
	BColors::ColorSet fgColors;
	BColors::ColorSet syColors;
	BColors::ColorSet bgColors;
	BStyles::Font lbfont;

	virtual void drawLineOnMap (Point p1, Point p2) override;
	virtual void draw (const double x, const double y, const double width, const double height) override;
};

#endif /* SHAPEWIDGET_HPP_ */
