#ifndef CAIROPLUS_H_
#define CAIROPLUS_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cairo/cairo.h>

typedef struct {
	double red;
	double green;
	double blue;
	double alpha;
} cairo_rgba;

typedef struct {
	const char *family;
	double size;
	cairo_font_slant_t slant;
	cairo_font_weight_t weight;
} cairo_text_decorations;

/**
 * Draws a rectangle with rounded edges.
 * @param cr	 Cairo context.
 * @param x		 X coordinate upper right
 * @param y		 Y coordinate upper right
 * @param width  Rectangle width
 * @param height Rectangle height
 * @param radius Radius of edges
 */
void cairo_rectangle_rounded (cairo_t* cr, double x, double y, double width, double height, double radius);

/**
 * Creates a new Cairo image surface and copies the content from a source Cairo
 * image surface.
 * @param sourceSurface Source Cairo (image) surface.
 * @return 				Created new Cairo image surface.
 */
cairo_surface_t* cairo_image_surface_clone_from_image_surface (cairo_surface_t* sourceSurface);

/**
 * Clears a Cairo surface.
 * @param surface Cairo surface.
 */
void cairo_surface_clear (cairo_surface_t* surface);

/**
 * Splits off a text that fits within an output area defined by its width.
 * @param cr 		  Cairo context.
 * @param decorations Cairo plus font decorations
 * @param text		  Text to be parsed. The output text will be clipped off
 * 					  and thus the text will be shortened
 * @return			  New created output text. Note, it will never return NULL.
 * 					  If memory allocation fails, a pointer to a nil text will
 * 					  be returned.
 */
char* cairo_create_text_fitted (cairo_t* cr, double width, cairo_text_decorations decorations, char* text);

/**
 * Destroys a Cairo plus text and frees memory.
 * @param text Cairo plus text.
 */
void cairo_text_destroy (char* text);

#endif /* CAIROPLUS_H_ */
