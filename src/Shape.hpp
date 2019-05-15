#ifndef SHAPE_HPP_
#define SHAPE_HPP_

#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <cmath>
#include "Point.hpp"
#include "Node.hpp"
#include "StaticArrayList.hpp"

#define MAPRES 1024

template<size_t sz>
class Shape
{
public:
	Shape ();
	Shape (StaticArrayList<Node, sz> nodes);
	virtual ~Shape ();

	bool operator== (const Shape<sz>& rhs);
	bool operator!= (const Shape<sz>& rhs);

	void clearShape ();
	void setDefaultShape ();
	void setDefaultShape (const Node& endNode);
	bool isDefault ();
	size_t size ();
	Node getNode (size_t nr);
	size_t findNode (Node& node);

	bool validateNode (size_t nr);
	bool validateShape ();
	bool appendNode (Node& node);
	bool insertNode (size_t pos, Node& node);
	bool insertNode (Node& node);
	bool changeNode (size_t pos, Node& newnode);
	bool deleteNode (size_t pos);

	double getMapValue (double x);
	double* getMap ();

protected:
	virtual void drawLineOnMap (Point p1, Point p2);
	Point getPointPerc (Point p1, Point p2, double perc);
	void renderBezier (Node& n1, Node& n2);

	StaticArrayList<Node, sz> nodes;
	double map[MAPRES];
	Node defaultEndNode;

};

template<size_t sz> Shape<sz>::Shape () : nodes (), defaultEndNode ()
{
	for (size_t i = 0; i < MAPRES; ++i) map[i] = 0;
}

template<size_t sz> Shape<sz>::Shape (StaticArrayList<Node, sz> nodes) : nodes (nodes), defaultEndNode ()
{
	for (size_t i = 0; i < MAPRES; ++i) map[i] = 0;
}

template<size_t sz> Shape<sz>::~Shape () {}

template<size_t sz> bool Shape<sz>::operator== (const Shape<sz>& rhs)
{
	if (size != rhs.size) return false;
	for (int i = 0; i < size; ++i) if (nodes[i] != rhs.nodes[i]) return false;
	return true;
}

template<size_t sz> bool Shape<sz>::operator!= (const Shape<sz>& rhs) {return !(*this == rhs);}

template<size_t sz> void Shape<sz>::clearShape ()
{
	while (!nodes.empty ()) nodes.pop_back ();
	for (int i = 0; i < MAPRES; ++i) map[i] = 0;
}

template<size_t sz> void Shape<sz>::setDefaultShape ()
{
	clearShape ();
	defaultEndNode = {NodeType::END_NODE, {0, 0}, {0, 0}, {0, 0}};
	nodes.push_back (defaultEndNode);
	nodes.push_back ({NodeType::END_NODE, {1, 0}, {0, 0}, {0, 0}});
	renderBezier (nodes[0], nodes[1]);
}

template<size_t sz> void Shape<sz>::setDefaultShape (const Node& endNode)
{
	clearShape ();
	defaultEndNode = {NodeType::END_NODE, {0, endNode.point.y}, {0, 0}, {0, 0}};
	nodes.push_back (defaultEndNode);
	nodes.push_back ({NodeType::END_NODE, {1, endNode.point.y}, {0, 0}, {0, 0}});
	renderBezier (nodes[0], nodes[1]);
}

template<size_t sz>bool Shape<sz>::isDefault ()
{
	return ((nodes.size == 2) && (nodes[0] == defaultEndNode));
}

template<size_t sz>size_t Shape<sz>::size () {return nodes.size;}

template<size_t sz>Node Shape<sz>::getNode (size_t nr) {return nodes[nr];}

template<size_t sz>size_t Shape<sz>::findNode (Node& node)
{
	for (int i = 0; i < nodes.size; ++i)
	{
		if (nodes[i] == node) return i;
	}
	return nodes.size;
}

template<size_t sz> bool Shape<sz>::appendNode (Node& node)
{
	if (nodes.size < sz)
	{
		nodes.push_back (node);
		return true;
	}

	return false;
}

template<size_t sz> bool Shape<sz>::insertNode (size_t pos, Node& node)
{
	// Nodes full => errorNode
	if (nodes.size >= sz) return false;

	// Insert
	if (pos < nodes.size) nodes.insert (nodes.begin() + pos, node);

	// Otherwise append
	else
	{
		pos = nodes.size;
		nodes.push_back (node);
	}

	// Validate node and its neighbors
	if (!validateNode (pos)) return false;
	if ((pos > 0) && (!validateNode (pos - 1))) return false;
	if ((pos + 1 < nodes.size) && (!validateNode (pos + 1))) return false;

	// Update map
	for (int i = (pos >= 2 ? pos - 2 : 0); (i <= pos + 1) && (i < nodes.size - 1); ++ i) renderBezier (nodes[i], nodes[i + 1]);
	return true;
}

template<size_t sz> bool Shape<sz>::insertNode (Node& node)
{
	// Find position
	size_t pos = nodes.size;
	for (int i = 0; i < nodes.size; ++i)
	{
		if (node.point.x < nodes[i].point.x)
		{
			// Add if not redundant
			if ((i > 0) && (node != nodes[i - 1]))
			{
				pos = i;
				break;
			}

			else return false;
		}
	}

	// Insert
	return insertNode (pos, node);
}

template<size_t sz> bool Shape<sz>::changeNode (size_t pos, Node& node)
{
	if (pos >= nodes.size) return false;
	nodes[pos] = node;

	// Validate node and its neighbors
	if (!validateNode (pos)) return false;
	if ((pos > 0) && (!validateNode (pos - 1))) return false;
	if ((pos + 1 < nodes.size) && (!validateNode (pos + 1))) return false;

	// Update map
	for (int i = (pos >= 2 ? pos - 2 : 0); (i <= pos + 1) && (i < nodes.size - 1); ++i) renderBezier (nodes[i], nodes[i + 1]);

	return true;
}

template<size_t sz> bool Shape<sz>::deleteNode (size_t pos)
{
	// Only deletion of middle nodes allowed
	if ((pos == 0) || (pos >= nodes.size - 1)) return false;

	nodes.erase (nodes.begin() + pos);

	// Validate neighbor nodes
	if (!validateNode (pos - 1)) return false;
	if (!validateNode (pos)) return false;

	// Update map
	for (int i = (pos >= 2 ? pos - 2 : 0); (i <= pos) && (i < nodes.size - 1); ++ i) renderBezier (nodes[i], nodes[i + 1]);
	return true;
}

template<size_t sz> bool Shape<sz>::validateNode (size_t nr)
{
	// Exception: Invalid parameters
	if (nr >= nodes.size)
	{
		fprintf (stderr, "BShapr.lv2: Node validation called with invalid parameters (node: %li).\n", nr);
		return false;
	}

	// Exception: Invalid node order
	if ((nodes.size >= 3) && (nr > 1) && (nr < nodes.size - 1) && (nodes[nr-1].point.x > nodes[nr+1].point.x))
	{
		fprintf (stderr, "BShapr.lv2: Corrupt node data at node %li. Reset shape.\n", nr);
		setDefaultShape ();
		return false;
	}

	// Start node
	if (nr == 0)
	{
		// Check: Only end nodes on start position
		if  (nodes[0].nodeType != NodeType::END_NODE) nodes[0] = {NodeType::END_NODE, {0, 0}, {0, 0}, {0, 0}};

		// Check: Start position
		if (nodes[0].point.x != 0) nodes[0].point.x = 0;

		// Check: No handles
		nodes[0].handle1 = Point (0, 0);
		nodes[0].handle2 = Point (0, 0);
	}

	// End node
	else if (nr == nodes.size - 1)
	{
		// Check: Only end nodes on end position
		if (nodes[nr].nodeType != NodeType::END_NODE)
		{
			nodes[nr] = {NodeType::END_NODE, {1, nodes[0].point.y}, {0, 0}, {0, 0}};
		}

		// Check: No handles
		nodes[nr].handle1 = Point (0, 0);
		nodes[nr].handle2 = Point (0, 0);
	}

	// Middle nodes
	else
	{
		// Check: No end nodes in the middle
		if (nodes[nr].nodeType == NodeType::END_NODE) nodes[nr].nodeType = NodeType::CORNER_NODE;

		// Check: Nodes point order
		if (nodes[nr].point.x < nodes[nr - 1].point.x) nodes[nr].point.x = nodes[nr - 1].point.x;
		if (nodes[nr].point.x > nodes[nr + 1].point.x) nodes[nr].point.x = nodes[nr + 1].point.x;

		// Check: POINT_NODE without handles
		if (nodes[nr].nodeType == NodeType::POINT_NODE)
		{
			nodes[nr].handle1 = Point (0, 0);
			nodes[nr].handle2 = Point (0, 0);
		}

		// Check: Handles order
		if (nodes[nr].handle1.x > 0) nodes[nr].handle1.x = 0;
		if (nodes[nr].handle2.x < 0) nodes[nr].handle2.x = 0;

		// Check: AUTO_SMOOTH_NODE with symmetric handles with the half size of the distance to the closest neighbor point
		if (nodes[nr].nodeType == NodeType::AUTO_SMOOTH_NODE)
		{
			double dist = (nodes[nr].point.x - nodes[nr-1].point.x > nodes[nr+1].point.x - nodes[nr].point.x ?
						   nodes[nr+1].point.x - nodes[nr].point.x :
						   nodes[nr].point.x - nodes[nr-1].point.x);
			double ydist = (nodes[nr + 1].point.y - nodes[nr - 1].point.y);
			double yamp = (fabs (nodes[nr - 1].point.y != 0) && fabs (nodes[nr + 1].point.y != 0) ?
								(fabs (nodes[nr - 1].point.y) < fabs (nodes[nr + 1].point.y) ?
										fabs (nodes[nr - 1].point.y) / (fabs (nodes[nr - 1].point.y) + fabs (nodes[nr + 1].point.y)) :
										fabs (nodes[nr + 1].point.y) / (fabs (nodes[nr - 1].point.y) + fabs (nodes[nr + 1].point.y))) :
								0);
			nodes[nr].handle1.x = -dist / 2;
			nodes[nr].handle1.y = -ydist * yamp;
			nodes[nr].handle2.x = dist / 2;
			nodes[nr].handle2.y = ydist * yamp;
		}

		// Check: SYMMETRIC_SMOOTH_NODE must be symmetric
		else if (nodes[nr].nodeType == NodeType::SYMMETRIC_SMOOTH_NODE)
		{
			//Check if handle1 overlaps neighbor point
			if (nodes[nr].point.x + nodes[nr].handle1.x < nodes[nr-1].point.x)
			{
				double f = (nodes[nr-1].point.x - nodes[nr].point.x) / nodes[nr].handle1.x;
				nodes[nr].handle1.x *= f;
				nodes[nr].handle1.y *= f;
			}

			// Make handele2 symmetric to handle1
			nodes[nr].handle2 = Point (0, 0) - nodes[nr].handle1;

			//Check if handle2 overlaps neighbor point
			if (nodes[nr].point.x + nodes[nr].handle2.x > nodes[nr+1].point.x)
			{
				double f = (nodes[nr+1].point.x - nodes[nr].point.x) / nodes[nr].handle2.x;
				nodes[nr].handle2.x *= f;
				nodes[nr].handle2.y *= f;
				nodes[nr].handle1 = Point (0, 0) - nodes[nr].handle2;
			}
		}

		// Check: SMOOTH_NODE handles point to opposite directions
		else if (nodes[nr].nodeType == NodeType::SMOOTH_NODE)
		{
			//Check if handle1 overlaps neighbor point
			if (nodes[nr].point.x + nodes[nr].handle1.x < nodes[nr-1].point.x)
			{
				double f = (nodes[nr-1].point.x - nodes[nr].point.x) / nodes[nr].handle1.x;
				nodes[nr].handle1.x *= f;
				nodes[nr].handle1.y *= f;
			}

			// Calculate handle distances
			double dist1 = sqrt (pow (nodes[nr].handle1.x, 2) + pow (nodes[nr].handle1.y, 2));
			double dist2 = sqrt (pow (nodes[nr].handle2.x, 2) + pow (nodes[nr].handle2.y, 2));

			// Recalculate handle2
			if ((dist1 != 0) && (dist2 != 0))
			{
				nodes[nr].handle2.x = -(nodes[nr].handle1.x * dist2 / dist1);
				nodes[nr].handle2.y = -(nodes[nr].handle1.y * dist2 / dist1);
			}

			//Check if handle2 overlaps neighbor point
			if (nodes[nr].point.x + nodes[nr].handle2.x > nodes[nr+1].point.x)
			{
				double f = (nodes[nr+1].point.x - nodes[nr].point.x) / nodes[nr].handle2.x;
				nodes[nr].handle2.x *= f;
				nodes[nr].handle2.y *= f;
			}
		}

		// Check: CORNER_NODE
		else if (nodes[nr].nodeType == NodeType::CORNER_NODE)
		{
			//Check if handle1 overlaps neighbor point
			if (nodes[nr].point.x + nodes[nr].handle1.x < nodes[nr-1].point.x)
			{
				double f = (nodes[nr-1].point.x - nodes[nr].point.x) / nodes[nr].handle1.x;
				nodes[nr].handle1.x *= f;
				nodes[nr].handle1.y *= f;
			}

			//Check if handle2 overlaps neighbor point
			if (nodes[nr].point.x + nodes[nr].handle2.x > nodes[nr+1].point.x)
			{
				double f = (nodes[nr+1].point.x - nodes[nr].point.x) / nodes[nr].handle2.x;
				nodes[nr].handle2.x *= f;
				nodes[nr].handle2.y *= f;
			}
		}
	}

	return true;
}

template<size_t sz> bool Shape<sz>::validateShape ()
{
	// TODO Sort ???

	// Validate nodes
	bool status = true;
	for (int i = 0; i < nodes.size; ++i)
	{
		if (!validateNode (i)) status = false;
	}

	// Update map
	for (int i = 0; i < nodes.size - 1; ++i) renderBezier (nodes[i], nodes[i+1]);

	return status;
}


template<size_t sz> void Shape<sz>::drawLineOnMap (Point p1, Point p2)
{
	if (p1.x < p2.x)
	{
		for (double x = p1.x; (x <= p2.x) && (x <= 1.0); x += (1.0 / MAPRES))
		{
			uint32_t i = ((uint32_t) (x * MAPRES)) % MAPRES;
			map[i] = p1.y + (p2.y - p1.y) * (x - p1.x) / (p2.x - p1.x);
		}
	}

	else
	{
		uint32_t i = ((uint32_t) (p1.x * MAPRES)) % MAPRES;
		map [i] = p1.y;
	}
}

template<size_t sz> Point Shape<sz>::getPointPerc (Point p1, Point p2 , double perc)
{
	Point p;
	p.x = p1.x + (p2.x - p1.x) * perc;
	p.y = p1.y + (p2.y - p1.y) * perc;
	return p;
}

template<size_t sz> void Shape<sz>::renderBezier (Node& n1, Node& n2)
{
	// Interpolate Bezier curve
	Point p1 = n1.point;
	Point p2 = n1.point + n1.handle2;
	Point p4 = n2.point;
	Point p3 = n2.point + n2.handle1;
	Point py = p1;
	double step = 1 / (fabs (n2.point.x - n1.point.x) * MAPRES + 1);

	for (double t = 0; t < 1; t += step)
	{
	    Point pa = getPointPerc (p1, p2, t);
	    Point pb = getPointPerc (p2, p3, t);
	    Point pc = getPointPerc (p3, p4, t);
	    Point pm = getPointPerc (pa, pb, t);
	    Point pn = getPointPerc (pb, pc, t);
	    Point pz = getPointPerc (pm, pn, t);

	    drawLineOnMap (py, pz);
	    py = pz;
	}
	drawLineOnMap (py,p4);
}

template<size_t shapesize> double Shape<shapesize>::getMapValue (double x)
{
	int mapx = ((int)(x * MAPRES)) % MAPRES;
	int mapx1 = (mapx + 1) % MAPRES;
	double xmod = (((int)(x * MAPRES)) % MAPRES) - mapx;

	return map[mapx] + (map[mapx1] - map[mapx]) * xmod;
}

template<size_t sz> double* Shape<sz>::getMap () {return &map[0];}

#endif /* SHAPE_HPP_ */
