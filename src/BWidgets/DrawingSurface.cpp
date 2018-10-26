#include "DrawingSurface.hpp"

namespace BWidgets
{
DrawingSurface::DrawingSurface () : DrawingSurface (0.0, 0.0, 50.0, 50.0, "dial") {}

DrawingSurface::DrawingSurface (const double x, const double y, const double width, const double height, const std::string& name) :
		Widget (x, y, width, height, name)
{
	drawingSurface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, getEffectiveWidth (), getEffectiveHeight ());
	draw (0, 0, width_, height_);
}

DrawingSurface::DrawingSurface (const DrawingSurface& that) :
		Widget (that)
{
	drawingSurface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, getEffectiveWidth (), getEffectiveHeight ());
	//TODO copy surface data
	draw (0, 0, width_, height_);
}

DrawingSurface::~DrawingSurface ()
{
	cairo_surface_destroy (drawingSurface);
}

DrawingSurface& DrawingSurface::operator= (const DrawingSurface& that)
{
	Widget::operator= (that);
	if (drawingSurface) cairo_surface_destroy (drawingSurface);
	drawingSurface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, getEffectiveWidth (), getEffectiveHeight ());
	//TODO copy surface data

	return *this;
}

cairo_surface_t* DrawingSurface::getDrawingSurface () {return drawingSurface;}

void DrawingSurface::setWidth (const double width)
{
	double oldEffectiveWidth = getEffectiveWidth ();
	Widget::setWidth (width);

	if (oldEffectiveWidth != getEffectiveWidth ())
	{
		if (drawingSurface) cairo_surface_destroy (drawingSurface);
		drawingSurface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, getEffectiveWidth (), getEffectiveHeight ());
		//TODO copy surface data
	}

	update ();
}

void DrawingSurface::setHeight (const double height)
{
	double oldEffectiveHeight = getEffectiveHeight ();
	Widget::setHeight (height);

	if (oldEffectiveHeight != getEffectiveHeight ())
	{
		if (drawingSurface) cairo_surface_destroy (drawingSurface);
		drawingSurface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, getEffectiveWidth (), getEffectiveHeight ());
		//TODO copy surface data
	}

	update ();
}

void DrawingSurface::setBorder (const BStyles::Border& border)
{
	double oldTotalBorderWidth = getXOffset ();
	border_ = border;

	if (oldTotalBorderWidth != getXOffset ())
	{
		if (drawingSurface) cairo_surface_destroy (drawingSurface);
		drawingSurface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, getEffectiveWidth (), getEffectiveHeight ());
		//TODO copy surface data
	}

	update ();
}

void DrawingSurface::update ()
{
	draw (0, 0, width_, height_);
	if (isVisible ()) postRedisplay ();
}

void DrawingSurface::draw (const double x, const double y, const double width, const double height)
{
	if ((width_ >= 4) && (height_ >= 4))
	{
		// Draw super class widget elements first
		Widget::draw (x, y, width, height);

		cairo_t* cr = cairo_create (widgetSurface);
		if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
		{
			// Limit cairo-drawing area
			cairo_rectangle (cr, x, y, width, height);
			cairo_clip (cr);
			//TODO also clip to inner borders

			cairo_set_source_surface (cr, drawingSurface, getXOffset (), getYOffset ());
			cairo_paint (cr);
		}
		cairo_destroy (cr);
	}
}

}
