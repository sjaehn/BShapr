#ifndef NODE_HPP_
#define NODE_HPP_

#include "Point.hpp"

enum NodeType
{
	END_NODE				= 0,	// End positions, only point data are used
	POINT_NODE				= 1,	// Only point data are used
	AUTO_SMOOTH_NODE		= 2,	// Uses two handles, both are calculated automatically
	SYMMETRIC_SMOOTH_NODE	= 3, 	// Uses two handles, the second handle is set to be symmetric to the first one
	SMOOTH_NODE				= 4,	// Uses two handles, the second handle points to the opposite direction of the first one
	CORNER_NODE				= 5		// Uses two independent handles
};

struct Node
{
	NodeType nodeType;
	Point point;
	Point handle1;
	Point handle2;

	Node () : Node (END_NODE, {0, 0}, {0, 0}, {0,0}) {}
	Node (NodeType nodeType, Point point, Point handle1, Point handle2) : nodeType (nodeType), point (point), handle1 (handle1), handle2 (handle2) {}
	Node (float* data) : nodeType ((NodeType) data[0]), point ({data[1], data[2]}), handle1 ({data[3], data[4]}), handle2 ({data[5], data[6]}) {}

	friend bool operator== (const Node& lhs, const Node& rhs)
		{return ((lhs.nodeType == rhs.nodeType) && (lhs.point == rhs.point) && (lhs.handle1 == rhs.handle1) && (lhs.handle2 == rhs.handle2));}
	friend bool operator!= (const Node& lhs, const Node& rhs) {return !(lhs == rhs);}
};

#endif /* NODE_HPP_ */
