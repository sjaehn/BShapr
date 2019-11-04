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

#include "ShapeWidget.hpp"
#include <cmath>
#include <string>

ShapeWidget::ShapeWidget () : ShapeWidget (0, 0, 0, 0, "") {}

ShapeWidget::ShapeWidget (const double x, const double y, const double width, const double height, const std::string& name) :
		Shape (), ValueWidget (x, y, width, height, name, 0),
		clickMode (NEW_NODE), selection (), tool (NO_TOOL), grabbedNode (-1), grabbedHandle (-1),
		valueEnabled (false),
		scaleAnchorYPos (0), scaleAnchorValue (0), scaleRatio (1),
		minorXSteps (1), majorXSteps (1),
		loLimit (-1000000), hiLimit (1000000), hardLoLimit (false), hardHiLimit (false),
		gridVisible (true), gridSnap (true),
		prefix (""), unit (""),
		fgColors (BColors::reds), bgColors (BColors::darks), lbfont (BWIDGETS_DEFAULT_FONT)
{
	setDraggable (true);
	setScrollable (true);
}

void ShapeWidget::setTool (const ToolType tool) {this->tool = tool;}

void ShapeWidget::setValueEnabled (const bool status) {valueEnabled = status;}

void ShapeWidget::setScaleParameters (double anchorYPos, double anchorValue, double ratio)
{
	if ((scaleAnchorYPos != anchorYPos) || (scaleAnchorValue != anchorValue) || (scaleRatio != ratio))
	{
		scaleAnchorYPos = anchorYPos;
		scaleAnchorValue = anchorValue;
		scaleRatio = ratio;
		update ();
	}
}

void ShapeWidget::setMinorXSteps (double stepSize)
{
	if (minorXSteps != stepSize)
	{
		minorXSteps = stepSize;
		update ();
	}
}

void ShapeWidget::setMajorXSteps (double stepSize)
{
	if (majorXSteps != stepSize)
	{
		majorXSteps = stepSize;
		update ();
	}
}

void ShapeWidget::setPrefix (std::string text)
{
	if (prefix != text)
	{
		prefix = text;
		update ();
	}
}

void ShapeWidget::setUnit (std::string text)
{
	if (unit != text)
	{
		unit = text;
		update ();
	}
}

void ShapeWidget::setLowerLimit (double value, bool hard)
{
	if ((value != loLimit) || (hard != hardLoLimit))
	{
		loLimit = value;
		hardLoLimit = hard;
		update ();
	}
}
void ShapeWidget::setHigherLimit (double value, bool hard)
{
	if ((value != hiLimit) || (hard != hardHiLimit))
	{
		hiLimit = value;
		hardHiLimit = hard;
		update ();
	}
}

void ShapeWidget::showGrid ()
{
	gridVisible = true;
	update ();
}

void ShapeWidget::hideGrid ()
{
	gridVisible = false;
	update ();
}

void ShapeWidget::setSnap (const bool status)
{
	gridSnap = status;
}

std::vector<Node> ShapeWidget::cutSelection ()
{
	std::vector<Node> selectedNodes = copySelection ();
	deleteSelection ();
	return selectedNodes;
}

std::vector<Node> ShapeWidget::copySelection ()
{
	std::vector<Node> selectedNodes;
	for (size_t i = 0; i < nodes.size; ++i)
	{
		if (selection[i]) selectedNodes.push_back (nodes[i]);
	}
	return selectedNodes;
}

void ShapeWidget::pasteSelection (const std::vector<Node>& newNodes)
{
	selection.clear ();

	for (Node const& n : newNodes)
	{
		Node node = n;

		// END_NODEs: Overwrite
		if (node.nodeType == END_NODE)
		{
			if (nodes.size >= 2)
			{
				if (node.point.x == 0.0) selection[0] = true;
				else selection[nodes.size - 1] = true;

				node.point.x = nodes[0].point.x;
				changeNode (0, node);
				node.point.x = nodes[nodes.size - 1].point.x;
				changeNode (nodes.size - 1, node);
			}
		}

		// Other nodes: Insert
		else
		{
			// Find insert position
			size_t pos = 0;
			for (size_t i = 0; i < nodes.size; ++i)
			{
				if (node.point.x < nodes[i].point.x)
				{
					// Add if not redundant
					if ((i > 0) && (node != nodes[i - 1]))
					{
						pos = i;
						break;
					}

					else break;
				}
			}

			// Insert and select
			if ((pos != 0) && (nodes.size < MAXNODES))
			{
				insertNode (pos, node);
				selection[pos] = true;
			}
		}
	}

	undoSnapshots.push (*this);
}

void ShapeWidget::deleteSelection ()
{
	grabbedNode = -1;
	bool selected = false;
	for (int i = nodes.size; i >= 0; --i)
	{
		if (selection[i])
		{
			selected = true;
			deleteNode (i);
		}
	}

	if (selected)
	{
		selection.clear ();
		update ();
	}

	undoSnapshots.push (*this);
}

void ShapeWidget::unselect ()
{
	grabbedNode = -1;
	selection.clear ();
}

void ShapeWidget::reset (const Node& endNode)
{
	unselect ();
	Shape::setDefaultShape (endNode);
	undoSnapshots.push (*this);
}

void ShapeWidget::undo ()
{
	unselect ();
	clearShape ();
	this->Shape::operator= (undoSnapshots.undo ());
	validateShape ();
}

void ShapeWidget::redo ()
{
	unselect ();
	clearShape ();
	this->Shape::operator= (undoSnapshots.redo ());
	validateShape ();
}

void ShapeWidget::pushToSnapshots ()
{
	undoSnapshots.push (*this);
}

void ShapeWidget::resetSnapshots ()
{
	undoSnapshots.clear ();
	undoSnapshots.push (*this);
}

void ShapeWidget::setDefaultShape ()
{
	unselect ();
	Shape::setDefaultShape ();
	resetSnapshots ();
}

void ShapeWidget::setDefaultShape (const Node& endNode)
{
	unselect ();
	Shape::setDefaultShape (endNode);
	resetSnapshots ();
}

void ShapeWidget::onButtonPressed (BEvents::PointerEvent* event)
{
	double x0 = getXOffset ();
	double y0 = getYOffset ();
	double w = getEffectiveWidth ();
	double h = getEffectiveHeight ();
	double ymin = scaleAnchorValue - scaleRatio * scaleAnchorYPos;
	double ymax = ymin + scaleRatio;
	double px;
	double py;

	if ((w == 0) || (h == 0) || (ymax == ymin)) return;

	// Left button: select / deselect nodes or handles
	if (event->getButton() == BEvents::InputDevice::LEFT_BUTTON)
	{
		// Node already activated => Select handles
		if ((grabbedNode >= 0) && (grabbedNode < MAXNODES))
		{
			// Handle2 => Select
			px = x0 + w * (nodes[grabbedNode].point.x + nodes[grabbedNode].handle2.x);
			py = y0 + h - h * (nodes[grabbedNode].point.y + nodes[grabbedNode].handle2.y - ymin) / (ymax - ymin);
			if ((nodes[grabbedNode].nodeType != NodeType::END_NODE) &&		// No END_NODEs
				(nodes[grabbedNode].nodeType != NodeType::POINT_NODE) &&	// No POINT_NODEs
				(nodes[grabbedNode].nodeType != NodeType::AUTO_SMOOTH_NODE) &&	// No AUTO_SMOOTH_NODEs
				(event->getX() >= px - 3) &&					// Within the handle2 position
				(event->getX() <= px + 3) &&
				(event->getY() >= py - 3) &&
				(event->getY() <= py + 3))
			{
				clickMode = DRAG_HANDLE;
				grabbedHandle = 2;
				selection[grabbedNode] = true;
				update ();
				return;
			}

			// Handle1 => Select
			px = x0 + w * (nodes[grabbedNode].point.x + nodes[grabbedNode].handle1.x);
			py = y0 + h - h * (nodes[grabbedNode].point.y + nodes[grabbedNode].handle1.y - ymin) / (ymax - ymin);
			if ((nodes[grabbedNode].nodeType != NodeType::END_NODE) &&		// No END_NODEs
				(nodes[grabbedNode].nodeType != NodeType::POINT_NODE) &&	// No POINT_NODEs
				(nodes[grabbedNode].nodeType != NodeType::AUTO_SMOOTH_NODE) &&	// No AUTO_SMOOTH_NODEs
				(event->getX() >= px - 3) &&					// Within the handle1 position
				(event->getX() <= px + 3) &&
				(event->getY() >= py - 3) &&
				(event->getY() <= py + 3))
			{
				clickMode = DRAG_HANDLE;
				grabbedHandle = 1;
				selection[grabbedNode] = true;
				update ();
				return;
			}
		}

		// Point => Select
		for (unsigned int i = 0; i < nodes.size; ++i)
		{
			px = x0 + w * nodes[i].point.x;
			py = y0 + h - h * (nodes[i].point.y - ymin) / (ymax - ymin);
			if ((event->getX() >= px - 6) &&	// Within the point position
				(event->getX() <= px + 6) &&
				(event->getY() >= py - 6) &&
				(event->getY() <= py + 6))
			{
				clickMode = DRAG_NODE;
				grabbedNode = i;
				grabbedHandle = -1;

				// Click on not selected point => clear selection
				if (!selection[i])
				{
					selection.clear ();
					selection[i] = true;
					selection.setOrigin ({px, py});
				}

				update ();
				return;
			}
		}

		// New selection box in edit mode
		if (tool == EDIT_TOOL)
		{
			clickMode = DRAG_SELECTION;
			grabbedNode = -1;
			selection.clear ();
			py = ((y0 + h - event->getY ()) / h) * scaleRatio + ymin;
			px = (event->getX () - x0) / w;
			selection.setOrigin ({px, py});
			update ();
			return;
		}

		else
		{
			clickMode = NEW_NODE;
			grabbedNode = -1;
		}
	}
}

void ShapeWidget::onButtonReleased (BEvents::PointerEvent* event)
{
	if (event->getButton() == BEvents::InputDevice::LEFT_BUTTON)
	{
		double x0 = getXOffset ();
		double y0 = getYOffset ();
		double w = getEffectiveWidth ();
		double h = getEffectiveHeight ();
		double ymin = scaleAnchorValue - scaleRatio * scaleAnchorYPos;

		if ((w == 0) || (h == 0)) return;

		double px = (event->getX () - x0) / w;
		double py = ((y0 + h - event->getY ()) / h) * scaleRatio + ymin;

		// Snap to grid
		if (gridSnap)
		{
			px = snapX (px);
			py = snapY (py);
		}

		// Add new nodes, but not on top of a selected node or handle
		if (clickMode == NEW_NODE)
		{
			selection.clear ();

			switch (tool)
			{
				case ToolType::POINT_NODE_TOOL:
				{
					Node node = Node(NodeType::POINT_NODE, Point (px, py), Point (0, 0), Point (0, 0));
					insertNode (node);
				}
				break;

				case ToolType::AUTO_SMOOTH_NODE_TOOL:
				{
					Node node = Node(NodeType::AUTO_SMOOTH_NODE, Point (px, py), Point (0, 0), Point (0, 0));
					insertNode (node);
				}
				break;

				case ToolType::SYMMETRIC_SMOOTH_NODE_TOOL:
				{
					Node node = Node(NodeType::SYMMETRIC_SMOOTH_NODE, Point (px, py), Point (-0.02, 0), Point (0.02, 0));
					insertNode (node);
				}
				break;

				case ToolType::CORNER_NODE_TOOL:
				{
					Node node = Node(NodeType::CORNER_NODE, Point (px, py), Point (-0.02, 0), Point (0.02, 0));
					insertNode (node);
				}
				break;

				default: break;
			}

			undoSnapshots.push (*this);
		}

		else if (clickMode == DRAG_SELECTION)
		{
			selection.setOrigin ({0, 0});
			selection.setExtend ({0, 0});
			undoSnapshots.push (*this);
			update ();
		}
	}
}

void ShapeWidget::onPointerDragged (BEvents::PointerEvent* event)
{
	if (event->getButton() == BEvents::InputDevice::LEFT_BUTTON)
	{
		double x0 = getXOffset ();
		double y0 = getYOffset ();
		double w = getEffectiveWidth ();
		double h = getEffectiveHeight ();
		double ymin = scaleAnchorValue - scaleRatio * scaleAnchorYPos;

		if ((w == 0) || (h == 0)) return;

		double py = ((y0 + h - event->getY ()) / h) * scaleRatio + ymin;
		double px = (event->getX () - x0) / w;

		// Node or handle dragged
		if ((grabbedNode >= 0) && (grabbedNode < ((int)nodes.size)))
		{
			// Snap to grid
			if (gridSnap)
			{
				px = snapX (px);
				py = snapY (py);
			}

			// Handle: Drag only grabbed handle
			if ((clickMode == DRAG_HANDLE) && (grabbedHandle > 0))
			{
				// Drag right handle
				if (grabbedHandle == 2)
				{
					Node node = nodes[grabbedNode];
					node.handle2.x = px - nodes[grabbedNode].point.x;
					node.handle2.y = py - nodes[grabbedNode].point.y;

					if (node.nodeType == SYMMETRIC_SMOOTH_NODE) node.handle1 = Point (0, 0) - node.handle2;

					changeNode (grabbedNode, node);
				}

				// Drag left handle
				else if (grabbedHandle == 1)
				{
					Node node = nodes[grabbedNode];
					node.handle1.x = px - nodes[grabbedNode].point.x;
					node.handle1.y = py - nodes[grabbedNode].point.y;
					changeNode (grabbedNode, node);
				}
			}

			// Point: Drag all selected points
			else if (clickMode == DRAG_NODE)
			{
				double dpy = py - nodes[grabbedNode].point.y;
				double dpx = px - nodes[grabbedNode].point.x;

				// Get sure that selected points are not moved over their unselected antecessors / successors
				for (int i = 0; (i < int (nodes.size)) && (dpx != 0.0); ++i)
				{
					if (selection[i])
					{
						// END_NODEs: No X movement allowed
						if (nodes[i].nodeType == NodeType::END_NODE) dpx = 0;

						// Find antecessor
						else if (dpx < 0)
						{
							size_t ant = selection.getUnselectedAntecessor (i);
							if (ant < nodes.size)
							{
								if (nodes[i].point.x + dpx < nodes[ant].point.x) dpx = nodes[ant].point.x - nodes[i].point.x;
							}
						}

						// Find successor
						else if (dpx > 0)
						{
							size_t suc = selection.getUnselectedSuccessor (i);
							if (suc < nodes.size)
							{
								if (nodes[i].point.x + dpx > nodes[suc].point.x) dpx = nodes[suc].point.x - nodes[i].point.x;
							}
						}
					}
				}

				// Move selected points
				for (size_t i = 0; i < nodes.size; ++i)
				{
					if (selection[i])
					{
						//Y drag both END_NODEs
						if (nodes[i].nodeType == NodeType::END_NODE)
						{
							if (nodes.size >= 2)
							{
								Node node = nodes[0];
								node.point.x = nodes[0].point.x;
								node.point.y = nodes[0].point.y + dpy;
								changeNode (0, node);
								node.point.x = nodes[nodes.size - 1].point.x;
								changeNode (nodes.size - 1, node);
							}
						}

						else
						{
							Node node = nodes[grabbedNode];
							node.point.x = nodes[i].point.x + dpx;
							node.point.y = nodes[i].point.y + dpy;
							changeNode (i, node);
						}
					}
				}
			}
		}

		// Drag selection box in edit mode
		else if (clickMode == DRAG_SELECTION)
		{
			Point p1 = selection.getOrigin ();
			selection.setExtend ({px - p1.x, py - p1.y});

			Point p2 = p1 + selection.getExtend ();
			if (p2.x < p1.x) std::swap (p1.x, p2.x);
			if (p2.y < p1.y) std::swap (p1.y, p2.y);

			// Select / deselect selection[i]
			selection.fill (false);
			for (int i = 0; i < int (nodes.size); ++i)
			{
				Point p = nodes[i].point;

				if ((p.x >= p1.x) && (p.x <= p2.x) && (p.y >= p1.y) && (p.y <= p2.y))
				{
					selection[i] = true;
				}
			}

			update ();
			return;
		}

		// Scale dragged
		else
		{
			clickMode = DRAG_SCREEN;
			scaleAnchorYPos += (-event->getDeltaY()) / h;
			update ();
		}
	}
}

void ShapeWidget::onWheelScrolled (BEvents::WheelEvent* event)
{
	double ygrid = pow (10, floor (log10 (scaleRatio)));
	scaleRatio += 0.1 * ygrid * event->getDeltaY ();
	if (scaleRatio < 0.01) scaleRatio = 0.01;
	update ();
}

void ShapeWidget::onValueChanged (BEvents::ValueChangedEvent* event)
{
	if (event->getValue() == 1)
	{
		update ();
		cbfunction[BEvents::EventType::VALUE_CHANGED_EVENT] (event);
		setValue (0);
	}
}

void ShapeWidget::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}

void ShapeWidget::applyTheme (BStyles::Theme& theme, const std::string& name)
{
	Widget::applyTheme (theme, name);

	// Foreground colors (curve)
	void* fgPtr = theme.getStyle(name, BWIDGETS_KEYWORD_FGCOLORS);
	if (fgPtr) fgColors = *((BColors::ColorSet*) fgPtr);

	// Foreground colors (curve)
	void* syPtr = theme.getStyle(name, "symbolcolors");
	if (syPtr) syColors = *((BColors::ColorSet*) syPtr);

	// Background colors (grid)
	void* bgPtr = theme.getStyle(name, BWIDGETS_KEYWORD_BGCOLORS);
	if (bgPtr) bgColors = *((BColors::ColorSet*) bgPtr);

	// Font
	void* fontPtr = theme.getStyle(name, BWIDGETS_KEYWORD_FONT);
	if (fontPtr) lbfont = *((BStyles::Font*) fontPtr);

	if (fgPtr || bgPtr || syPtr || fontPtr) update ();

}

double ShapeWidget::snapX (const double x)
{
	if (minorXSteps == 0) return x;

	double nrXDashes = (minorXSteps < 0.05 ? 4 : (minorXSteps < 0.1 ? 8 : 16));
	double xDash = minorXSteps / nrXDashes;
	return round (x / xDash) * xDash;
}

double ShapeWidget::snapY (const double y)
{
	if (scaleRatio == 0.0) return y;

	double ymin = scaleAnchorValue - scaleRatio * scaleAnchorYPos;
	double ymax = ymin + scaleRatio;
	double ygrid = pow (10, floor (log10 (scaleRatio / 1.5)));
	double nrYDashes = (ygrid / (ymax - ymin) < 0.2 ? 4 : 8);
	double yDash = ygrid / nrYDashes;
	return round (y / yDash) * yDash;
}

void ShapeWidget::drawLineOnMap (Point p1, Point p2)
{
	Shape::drawLineOnMap (p1, p2);
	if (valueEnabled) setValue (1);	// Value changed
}

void ShapeWidget::draw (const double x, const double y, const double width, const double height)
{
	if ((!widgetSurface) || (cairo_surface_status (widgetSurface) != CAIRO_STATUS_SUCCESS)) return;

	ValueWidget::draw (x, y, width, height);

	double x0 = getXOffset ();
	double y0 = getYOffset ();
	double w = getEffectiveWidth ();
	double h = getEffectiveHeight ();
	double ymin = scaleAnchorValue - scaleRatio * scaleAnchorYPos;
	double ymax = ymin + scaleRatio;
	BColors::Color lineColor = *fgColors.getColor (BColors::NORMAL);
	BColors::Color fillColor = *fgColors.getColor (BColors::NORMAL);
	BColors::Color nodeColor = *syColors.getColor (BColors::NORMAL);
	BColors::Color activeNodeColor = *syColors.getColor (BColors::ACTIVE);
	BColors::Color gridColor = *bgColors.getColor (BColors::NORMAL);

	if (ymin == ymax) return;

	cairo_t* cr = cairo_create (widgetSurface);

	if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
	{
		// Limit cairo-drawing area
		cairo_rectangle (cr, x, y, width, height);
		cairo_clip (cr);

		double ygrid = pow (10, floor (log10 (scaleRatio / 1.5)));
		int ldYgrid = log10 (ygrid);
		std::string nrformat = "%" + ((ygrid < 1) ? ("1." + std::to_string (-ldYgrid)) : (std::to_string (ldYgrid + 1) + ".0")) + "f";
		cairo_text_extents_t ext;
		cairo_select_font_face (cr, lbfont.getFontFamily ().c_str (), lbfont.getFontSlant (), lbfont.getFontWeight ());
		cairo_set_font_size (cr, lbfont.getFontSize ());

		// Draw Y steps
		for (double yp = ceil (ymin / ygrid) * ygrid; yp <= ymax; yp += ygrid)
		{
			cairo_move_to (cr, x0, y0 + h - h * (yp - ymin) / (ymax - ymin));
			cairo_line_to (cr, x0 + 0.02 * w, y0 + h - h * (yp - ymin) / (ymax - ymin));

			std::string label = prefix + BValues::toBString (nrformat, yp) + ((unit != "") ? (" " + unit) : "");
			cairo_text_extents (cr, label.c_str(), &ext);
			cairo_move_to (cr, x0 + 0.025 * w - ext.x_bearing, y0 + h - h * (yp - ymin) / (ymax - ymin) - ext.height / 2 - ext.y_bearing);
			cairo_set_source_rgba (cr, CAIRO_RGBA (gridColor));
			cairo_show_text (cr, label.c_str ());

			cairo_move_to (cr, x0 + 0.03 * w + ext.width, y0 + h - h * (yp - ymin) / (ymax - ymin));
			cairo_line_to (cr, x0 + w, y0 + h - h * (yp - ymin) / (ymax - ymin));
		}

		cairo_set_source_rgba (cr, CAIRO_RGBA (gridColor));
		cairo_set_line_width (cr, 1);
		cairo_stroke (cr);

		// Draw lower Y limits
		if ((loLimit >= ymin) && (loLimit <= ymax))
		{
			std::string label = "Lower limit";
			cairo_text_extents (cr, label.c_str(), &ext);
			cairo_move_to (cr, x0 + 0.5 * w - 0.5 * ext.width - ext.x_bearing, y0 + h - h * (loLimit - ymin) / (ymax - ymin) - ext.height / 2 - ext.y_bearing);
			cairo_set_source_rgba (cr, CAIRO_RGBA (lineColor));
			cairo_show_text (cr, label.c_str ());

			cairo_move_to (cr, x0 + 0.1 * w, y0 + h - h * (loLimit - ymin) / (ymax - ymin));
			cairo_line_to (cr, x0 + 0.48 * w - 0.5 * ext.width - ext.x_bearing, y0 + h - h * (loLimit - ymin) / (ymax - ymin));

			cairo_move_to (cr, x0 + 0.52 * w + 0.5 * ext.width - ext.x_bearing, y0 + h - h * (loLimit - ymin) / (ymax - ymin));
			cairo_line_to (cr, x0 + 0.9 * w, y0 + h - h * (loLimit - ymin) / (ymax - ymin));

			cairo_set_source_rgba (cr, CAIRO_RGBA (lineColor));
			cairo_set_line_width (cr, 1);
			cairo_stroke (cr);
		}

		// Draw upper Y limits
		if ((hiLimit >= ymin) && (hiLimit <= ymax))
		{
			std::string label = "Upper limit";
			cairo_text_extents (cr, label.c_str(), &ext);
			cairo_move_to (cr, x0 + 0.5 * w - 0.5 * ext.width - ext.x_bearing, y0 + h - h * (hiLimit - ymin) / (ymax - ymin) - ext.height / 2 - ext.y_bearing);
			cairo_set_source_rgba (cr, CAIRO_RGBA (lineColor));
			cairo_show_text (cr, label.c_str ());

			cairo_move_to (cr, x0 + 0.1 * w, y0 + h - h * (hiLimit - ymin) / (ymax - ymin));
			cairo_line_to (cr, x0 + 0.48 * w - 0.5 * ext.width - ext.x_bearing, y0 + h - h * (hiLimit - ymin) / (ymax - ymin));

			cairo_move_to (cr, x0 + 0.52 * w + 0.5 * ext.width - ext.x_bearing, y0 + h - h * (hiLimit - ymin) / (ymax - ymin));
			cairo_line_to (cr, x0 + 0.9 * w, y0 + h - h * (hiLimit - ymin) / (ymax - ymin));

			cairo_set_source_rgba (cr, CAIRO_RGBA (lineColor));
			cairo_set_line_width (cr, 1);
			cairo_stroke (cr);
		}

		// Draw X steps
		for (double x = 0; x < 1; x += minorXSteps)
		{
			cairo_move_to (cr, x * w, 0);
			cairo_line_to (cr, x * w, h);
		}
		cairo_set_source_rgba (cr, CAIRO_RGBA (gridColor));
		cairo_set_line_width (cr, 1.0);
		cairo_stroke (cr);

		for (double x = 0; x < 1; x += majorXSteps)
		{
			cairo_move_to (cr, x * w, 0);
			cairo_line_to (cr, x * w, h);
		}
		cairo_set_source_rgba (cr, CAIRO_RGBA (gridColor));
		cairo_set_line_width (cr, 2.0);
		cairo_stroke (cr);

		// Draw grid
		if (gridVisible)
		{
			if (w * minorXSteps > 16)
			{
				cairo_save (cr);
				double nrXDashes = (minorXSteps < 0.05 ? 4 : (minorXSteps < 0.1 ? 8 : 16));
				double xDash = minorXSteps / nrXDashes;
				double nrYDashes = (ygrid / (ymax - ymin) < 0.2 ? 4 : 8);
				double yDash = ygrid / nrYDashes;
				double dashes[2] = {1.0, w * xDash - 1.0};
				cairo_set_dash (cr, dashes, 2, 0.0);
				for (double yp = ceil (ymin / yDash) * yDash; yp <= ymax; yp += yDash)
				{
					double y = (yp - ymin) / (ymax - ymin) * h;
					cairo_move_to (cr, x0, y0 + h - y);
					cairo_line_to (cr, x0 + w, y0 + h - y);
				}
				cairo_set_source_rgba (cr, CAIRO_RGBA (gridColor));
				cairo_set_line_width (cr, 1.0);
				cairo_stroke (cr);
				cairo_restore (cr);
			}
		}

		// Draw curve
		cairo_move_to (cr, x0, y0 + h - h * (map[0] - ymin) / (ymax - ymin));
		for (int i = 1; i < MAPRES; ++i) cairo_line_to (cr, x0 + w * i / MAPRES, y0 + h - h * (map[i] - ymin) / (ymax - ymin));
		cairo_set_line_width (cr, 2);
		cairo_set_source_rgba (cr, CAIRO_RGBA (lineColor));
		cairo_stroke_preserve (cr);

		// Fill area under the curve
		cairo_line_to (cr, x0 + w, y0 + h);
		cairo_line_to (cr, x0, y0 + h);
		cairo_close_path (cr);
		cairo_set_line_width (cr, 0);
		cairo_pattern_t* pat = cairo_pattern_create_linear (0, y0 + h, 0, y0);
		cairo_pattern_add_color_stop_rgba (pat, 0, fillColor.getRed (), fillColor.getGreen (), fillColor.getBlue (), 0);
		cairo_pattern_add_color_stop_rgba (pat, 1, fillColor.getRed (), fillColor.getGreen (), fillColor.getBlue (), 0.5 * fillColor.getAlpha ());
		cairo_set_source (cr, pat);
		cairo_fill (cr);
		cairo_pattern_destroy (pat);

		// Draw nodes
		for (unsigned int i = 0; i < nodes.size; ++i)
		{
			double xp = nodes[i].point.x;
			double yp = nodes[i].point.y;

			if ((nodes[i].nodeType == NodeType::END_NODE) || (nodes[i].nodeType == NodeType::POINT_NODE))
			{
				cairo_move_to (cr, x0 + xp * w - 6, y0 + h - h * (yp - ymin) / (ymax - ymin));
				cairo_line_to (cr, x0 + xp * w, y0 + h - h * (yp - ymin) / (ymax - ymin) - 6);
				cairo_line_to (cr, x0 + xp * w + 6, y0 + h - h * (yp - ymin) / (ymax - ymin));
				cairo_line_to (cr, x0 + xp * w, y0 + h - h * (yp - ymin) / (ymax - ymin) + 6);
				cairo_close_path (cr);
			}

			else cairo_rectangle (cr, x0 + xp * w - 6, y0 + h - h * (yp - ymin) / (ymax - ymin) - 6, 12, 12);

			if (selection[i])
			{
				cairo_set_source_rgba (cr, CAIRO_RGBA (activeNodeColor));
				cairo_set_line_width (cr, 2);
				cairo_stroke (cr);
			}

			else
			{
				cairo_set_source_rgba (cr, CAIRO_RGBA (nodeColor));
				cairo_set_line_width (cr, 1);
				cairo_stroke (cr);
			}

			if (((int)i) == grabbedNode)
			{
				// Draw handles
				if ((nodes[i].nodeType == SYMMETRIC_SMOOTH_NODE) || (nodes[i].nodeType == CORNER_NODE))
				{
					if (i != 0)
					{
						double nx = nodes[i].handle1.x;
						double ny = nodes[i].handle1.y;
						cairo_move_to (cr, x0 + xp * w, y0 + h - h * (yp - ymin) / (ymax - ymin));
						cairo_line_to (cr, x0 + (xp + nx) * w, y0 + h - h * (yp + ny - ymin) / (ymax - ymin));
						cairo_arc (cr, x0 + (xp + nx) * w, y0 + h - h * (yp + ny - ymin) / (ymax - ymin), 3.0, 0.0, 2 * M_PI);
						cairo_set_line_width (cr, 1);
						cairo_stroke (cr);
					}

					if (i != nodes.size - 1)
					{
						double nx = nodes[i].handle2.x;
						double ny = nodes[i].handle2.y;
						cairo_move_to (cr, x0 + xp * w, y0 + h - h * (yp - ymin) / (ymax - ymin));
						cairo_line_to (cr, x0 + (xp + nx) * w, y0 + h - h * (yp + ny - ymin) / (ymax - ymin));
						cairo_arc (cr, x0 + (xp + nx) * w, y0 + h - h * (yp + ny - ymin) / (ymax - ymin), 3.0, 0.0, 2 * M_PI);
						cairo_set_line_width (cr, 1);
						cairo_stroke (cr);
					}
				}
			}
		}

		// Draw selection box
		if (selection.getExtend () != Point (0, 0))
		{
			Point p1 = selection.getOrigin ();
			Point ext = selection.getExtend ();
			cairo_rectangle (cr, x0 + p1.x * w, y0 + h - h * (p1.y - ymin) / (ymax - ymin), ext.x * w, - h * ext.y / (ymax - ymin));
			cairo_set_source_rgba (cr, CAIRO_RGBA (lineColor));
			cairo_set_line_width (cr, 1);
			double dashes[2] = {2.0, 2.0};
			cairo_set_dash (cr, dashes, 2, 0);
			cairo_stroke (cr);
		}

		cairo_destroy (cr);

	}

}
