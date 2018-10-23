/* Copyright (C) 2018 by Sven Jähnichen
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

#ifndef BCOLORS_HPP_
#define BCOLORS_HPP_

#include <stdint.h>
#include <vector>
#include <iostream>

#define LIMIT(val, min, max) (val < min ? min : (val < max ? val : max))

namespace BColors
{

/**
 * Color states
 */
typedef enum {
	NORMAL		= 0,
	ACTIVE		= 1,
	INACTIVE	= 2,
	OFF			= 3,
	USER_DEFINED= 4
} State;

/**
 * Class BColors::Color
 *
 * Color management class. Uses relative (0.0 .. 1.0) color and alpha values
 */
class Color
{
public:
	Color ();
	Color (const double red, const double green, const double blue, const double alpha);
	Color (const uint32_t red32, const uint32_t green32, const uint32_t blue32, const uint32_t alpha32);

	bool operator== (const Color& that) const;
	bool operator!= (const Color& that) const;

	/**
	 * Sets colors rgb and alpha of a BColors::Color
	 * @param red, green, blue, alpha Relative values (0.0 .. 1.0)
	 */
	void setRGBA (const double red, const double green, const double blue, const double alpha);

	/**
	 * Sets colors rgb of a BColors::Color
	 * @param red, green, blue Relative values (0.0 .. 1.0)
	 */
	void setRGB (const double red, const double green, const double blue);

	/**
	 * Sets alpha value of a BColors::Color
	 * @param alpha Relative value (0.0 .. 1.0)
	 */
	void setAlpha (const double alpha);

	/**
	 * Gets red value of a BColors::Color
	 * @return Relative red value (0.0 .. 1.0)
	 */
	double getRed () const;

	/**
	 * Gets green value of a BColors::Color
	 * @return Relative green value (0.0 .. 1.0)
	 */
	double getGreen () const;

	/**
	 * Gets blue value of a BColors::Color
	 * @return Relative blue value (0.0 .. 1.0)
	 */
	double getBlue () const;

	/**
	 * Gets alpha value of a BColors::Color
	 * @return Relative alpha value (0.0 .. 1.0)
	 */
	double getAlpha () const;

	/**
	 * Compares the color of this object with the color of the given object by
	 * comparison of all RGB and alpha values.
	 * @param that Color to compare with
	 * @return 0, if both colors are equal
	 * 		   1, if the two colors are not equal
	 */
	int compare (const Color& that) const;

private:
	double red_, green_, blue_, alpha_;
};
/*
 * End of class BColors::Color
 *****************************************************************************/

const Color white = Color (1.0, 1.0, 1.0, 1.0);
const Color black = Color (0.0, 0.0, 0.0, 1.0);
const Color red = Color (1.0, 0.0, 0.0, 1.0);
const Color green = Color (0.0, 1.0, 0.0, 1.0);
const Color blue = Color (0.0, 0.0, 1.0, 1.0);
const Color yellow = Color (1.0, 1.0, 0.0, 1.0);
const Color grey = Color (0.5, 0.5, 0.5, 1.0);
const Color lightred = Color (1.0, 0.5, 0.5, 1.0);
const Color darkred = Color (0.5, 0.0, 0.0, 1.0);
const Color lightgreen = Color (0.5, 1.0, 0.5, 1.0);
const Color darkgreen = Color (0.0, 0.5, 0.0, 1.0);
const Color lightblue = Color (0.5, 0.5, 1.0, 1.0);
const Color darkblue = Color (0.0, 0.0, 0.5, 1.0);
const Color lightgrey = Color (0.75, 0.75, 0.75, 1.0);
const Color darkgrey = Color (0.25, 0.25, 0.25, 1.0);
const Color invisible = Color (0.0, 0.0, 0.0, 0.0);

/**
 * Class BColors::ColorSet
 *
 * Defines a set of colors for different states. I can be used in combination
 * with BColors::Style.
 */
class ColorSet
{
public:
	ColorSet ();
	ColorSet (const std::vector<Color> vectorOfColors);

	/**
	 * Adds (or overwrites) a BColors::Color to the ColorSet
	 * @param state BColors::State of the color to be added to the set. The
	 * 				set will be extended automatically if needed.
	 * @param color BColors::Color to be added
	 */
	void addColor (const State state, const Color& color);

	/**
	 * Removes a BColors::Color from the ColorSet
	 * @param state BColors::State of the color to be removed from the set. The
	 * 				set will be shrinked automatically if needed.
	 */
	void removeColor (const State state);

	/**
	 * Gets a (pointer to) BColors::Color from the ColorSet
	 * @param state BColors::State of the color to be returned.
	 * @return Pointer to BColors::Color of the respective state or pointer to
	 * 		   a copy of BColors::invisible if the states color is not within
	 * 		   the ColorSet.
	 */
	Color* getColor (const State state);

private:
	std::vector<Color> colors;
	Color noColor = invisible;
};
/*
 * End of class BColors::ColorSet
 *****************************************************************************/

const ColorSet reds = ColorSet ({red, lightred, darkred, black});
const ColorSet greens = ColorSet ({green, lightgreen, darkgreen, black});
const ColorSet blues = ColorSet ({blue, lightblue, darkblue, black});
const ColorSet greys = ColorSet ({grey, lightgrey, darkgrey, black});
const ColorSet whites = ColorSet ({lightgrey, white, grey, black});
const ColorSet darks = ColorSet ({darkgrey, grey, black, black});

}

#endif /* BCOLORS_HPP_ */
