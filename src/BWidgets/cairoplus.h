#ifndef CAIROPLUS_H_
#define CAIROPLUS_H_

#include <stdlib.h>
#include <cairo/cairo.h>

typedef struct {
	double red;
	double green;
	double blue;
	double alpha;
} cairo_rgba;

void cairo_rectangle_rounded (cairo_t* cr, double x, double y, double width, double height, double radius);

cairo_surface_t* cairo_image_surface_clone_from_image_surface (cairo_surface_t* sourceSurface);

void cairo_surface_clear (cairo_surface_t* surface);

#endif /* CAIROPLUS_H_ */
