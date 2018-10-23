#include "cairoplus.h"

#ifndef PI
#define PI 3.14159265
#endif

void cairo_rectangle_rounded (cairo_t* cr, double x, double y, double width, double height, double radius)
{
	if (radius == 0.0)
	{
		cairo_rectangle (cr, x, y, width, height);
	}
	else
	{
		cairo_arc (cr, x + width - radius, y + radius, radius, -PI / 2, 0);
		cairo_arc (cr, x + width - radius, y + height - radius, radius, 0, PI / 2);
		cairo_arc (cr, x + radius, y + height - radius, radius, PI / 2, PI);
		cairo_arc (cr, x + radius, y + radius, radius, PI, 1.5 * PI);
		cairo_close_path (cr);
	}
}

cairo_surface_t* cairo_image_surface_clone_from_image_surface (cairo_surface_t* sourceSurface)
{
	cairo_surface_t* targetSurface = NULL;

	if (sourceSurface && (cairo_surface_status (sourceSurface) == CAIRO_STATUS_SUCCESS))
	{
		cairo_format_t format = cairo_image_surface_get_format (sourceSurface);
		int width = cairo_image_surface_get_width (sourceSurface);
		int height = cairo_image_surface_get_height (sourceSurface);
		targetSurface = cairo_image_surface_create (format, width, height);
		cairo_t* cr = cairo_create (targetSurface);
		cairo_set_source_surface (cr, sourceSurface, 0, 0);
		cairo_paint (cr);
		cairo_destroy (cr);
	}

	return targetSurface;
}

void cairo_surface_clear (cairo_surface_t* surface)
{
	cairo_t* cr = cairo_create (surface);
	cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
	cairo_paint (cr);
	cairo_destroy (cr);
}
