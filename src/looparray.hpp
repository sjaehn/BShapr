/* B.Slicer
 * Step Sequencer Effect Plugin
 *
 * Copyright (C) 2018 by Sven Jähnichen
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

#ifndef LOOPARRAY_HPP_
#define LOOPARRAY_HPP_

#include <array>

template<typename T, size_t nm> class looparray : public std::array<T, nm>
{
public:
	T& operator[] (int n)
	{
		if (std::array<T,nm>::size () > 0)
		{
			int modulo = n % std::array<T,nm>::size ();
			if (modulo >= 0) return std::array<T, nm>::operator[] (modulo);
			else return std::array<T, nm>::operator[] (std::array<T,nm>::size () + modulo);
		}
		else return std::array<T,nm>::operator[] (0);

	}
};




#endif /* LOOPARRAY_HPP_ */
