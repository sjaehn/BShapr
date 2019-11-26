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

#ifndef SMOOTHSHAPE_HPP_
#define SMOOTHSHAPE_HPP_

#include <iostream>
#include "Shape.hpp"

template<size_t sz>
class SmoothShape : public Shape<sz>
{
public:
	SmoothShape ();
	SmoothShape (StaticArrayList<Node, sz> nodes);

	bool operator== (const SmoothShape<sz>& rhs);
	bool operator!= (const SmoothShape<sz>& rhs);

	virtual void clearShape () override;
	void setSmoothing (const double smoothing);
	double getSmoothMapValue (double x);

protected:
	virtual void renderBezier (Node& n1, Node& n2) override;
	void smooth ();
	void smooth (const int x1, const int x2);

	double smoothMap[MAPRES];
	int smoothSz;

};

template<size_t sz> SmoothShape<sz>::SmoothShape () : Shape<sz> (), smoothSz (0)
{
	for (size_t i = 0; i < MAPRES; ++i) smoothMap[i] = 0;
}

template<size_t sz> SmoothShape<sz>::SmoothShape (StaticArrayList<Node, sz> nodes) : Shape<sz> (nodes), smoothSz (0)
{
	for (size_t i = 0; i < MAPRES; ++i) smoothMap[i] = 0;
}

template<size_t sz> bool SmoothShape<sz>::operator== (const SmoothShape<sz>& rhs)
{
	return Shape<sz>::operator== (rhs);
}

template<size_t sz> bool SmoothShape<sz>::operator!= (const SmoothShape<sz>& rhs) {return !(*this == rhs);}

template<size_t sz> void SmoothShape<sz>::clearShape ()
{
	Shape<sz>::clearShape ();
	for (int i = 0; i < MAPRES; ++i) smoothMap[i] = 0;
}

template<size_t sz> void SmoothShape<sz>:: setSmoothing (const double smoothing)
{
	smoothSz = smoothing * MAPRES;
	smooth ();
}

template<size_t sz> void SmoothShape<sz>::renderBezier (Node& n1, Node& n2)
{
	Shape<sz>::renderBezier (n1, n2);
	smooth ();
}

template<size_t sz> void SmoothShape<sz>:: smooth () {smooth (0, MAPRES - 1);}

template<size_t sz> void SmoothShape<sz>::smooth (const int x1, const int x2)
{
	int i1 = x1;
	int i2 = x2;
	if (i2 < i1) i2 = i1;

	if (smoothSz != 0)
	{
		int count = 0;
		double mean = 0;
		for (int i = i1 - smoothSz; i <= i2 + smoothSz; ++i)
		{
			double iVal = this->map [(i + MAPRES) % MAPRES];
			mean = (mean * double (count) + iVal) / (double (count + 1));
			++count;

			if (i >= i1 + smoothSz)
			{
				smoothMap[i - smoothSz] = mean;
				double rmVal = this->map [(i - 2 * smoothSz + 2 * MAPRES) % MAPRES];
				mean = (mean * double (count) - rmVal) / (double (count - 1));
				--count;
			}
		}
	}

	else for (int i = i1; i <= i2; ++i) smoothMap[i] = Shape<sz>::map[i];
}

template<size_t shapesize> double SmoothShape<shapesize>::getSmoothMapValue (double x)
{
	double mapx = fmod (x * MAPRES, MAPRES);
	double xmod = mapx - int (mapx);

	return (1 - xmod) * smoothMap[int (mapx)] + xmod * smoothMap[int (mapx + 1) % MAPRES];
}

#endif /* SMOOTHSHAPE_HPP_ */
