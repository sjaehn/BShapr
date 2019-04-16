#ifndef POINT_HPP_
#define POINT_HPP_

struct Point
{
	double x, y;

	Point () : Point (0, 0) {}
	Point (double x, double y) : x (x), y (y) {}

	Point& operator+= (const Point& rhs)
	{
		this->x += rhs.x;
		this->y += rhs.y;
		return *this;
	}

	Point& operator-= (const Point& rhs)
	{
		this->x -= rhs.x;
		this->y -= rhs.y;
		return *this;
	}

	friend bool operator== (const Point& lhs, const Point& rhs) {return ((lhs.x == rhs.x) && (lhs.y == rhs.y));}
	friend bool operator!= (const Point& lhs, const Point& rhs) {return !(lhs == rhs);}
	friend Point operator+ (Point lhs, const Point& rhs) {return (lhs += rhs);}
	friend Point operator- (Point lhs, const Point& rhs) {return (lhs -= rhs);}

};

#endif /* POINT_HPP_ */
