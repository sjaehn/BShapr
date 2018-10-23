#include "VSlider.hpp"

#ifndef PI
#define PI 3.14159265
#endif

namespace BWidgets
{
VSlider::VSlider () : VSlider (0, 0, 12, 100, "vslider", 0.0, 0.0, 100.0, 0.0) {}

VSlider::VSlider (const double  x, const double y, const double width, const double height, const std::string& name,
				  const double value, const double min, const double max, const double step) :
		RangeWidget (x, y, width, height, name, value, min, max, step), fgColors (BColors::greens), bgColors (BColors::greys)
{
	setClickable (true);
	setDragable (true);
}

VSlider::VSlider (const VSlider& that) : RangeWidget (that), fgColors (that.fgColors), bgColors (that.bgColors) {}

VSlider::~VSlider () {}

VSlider& VSlider::operator= (const VSlider& that)
{
	fgColors = that.fgColors;
	bgColors = that.bgColors;
	RangeWidget::operator= (that);

	return *this;
}

void VSlider::update ()
{
	draw (0, 0, width_, height_);
	if (isVisible ()) postRedisplay ();
}

void VSlider::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}

void VSlider::applyTheme (BStyles::Theme& theme, const std::string& name)
{
	Widget::applyTheme (theme, name);

	// Foreground colors (scale)
	void* fgPtr = theme.getStyle(name, "fgcolors");
	if (fgPtr) fgColors = *((BColors::ColorSet*) fgPtr);

	// Background colors (scale background, knob)
	void* bgPtr = theme.getStyle(name, "bgcolors");
	if (bgPtr) bgColors = *((BColors::ColorSet*) bgPtr);

	if (fgPtr || bgPtr) update ();
}

void VSlider::onButtonPressed (BEvents::PointerEvent* event)
{
	// Perform only if minimum requirements are satisfied
	if (main_ && isVisible () && (height_ >= 4) && (width_ >= 4) && (event->getButton() == BEvents::LEFT_BUTTON))
	{
		// Get pointer coords
		double y = event->getY ();
		double x = event->getX ();

		// Calculate aspect ratios first
		double w = (width_ > 24.0 ? 12.0 : 0.5 * width_);
		double h = (height_ / width_ >= 2 ? height_ - 2 * w : height_ - (height_ / width_) * w);

		// Pointer within the scale area ? Set value!
		if ((y >= height_/2 - h/2) && (y <= height_/2 + h/2 - 1) && (x >= 0) && (x <= width_ - 1))
		{
			double frac = (y - height_/2 + h/2) / (h - 1);
			if (getStep () >= 0) frac = 1 - frac;

			double min = getMin ();
			double max = getMax ();
			setValue (min + frac * (max - min));
		}
	}
}

void VSlider::onPointerMotionWhileButtonPressed (BEvents::PointerEvent* event) {onButtonPressed (event);}

void VSlider::draw (const double x, const double y, const double width, const double height)
{
	// Draw super class widget elements first
	Widget::draw (x, y, width, height);

	// Draw slider
	// only if minimum requirements satisfied
	if ((height_ >= 4) && (width_ >= 4))
	{
		cairo_surface_clear (widgetSurface);
		cairo_t* cr = cairo_create (widgetSurface);

		if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
		{
			cairo_pattern_t* pat;

			// Limit cairo-drawing area
			cairo_rectangle (cr, x, y, width, height);
			cairo_clip (cr);

			// Calculate aspect ratios first
			double w = (width_ > 24.0 ? 12.0 : 0.5 * width_);
			double h = (height_ / width_ >= 2 ? height_ - 2 * w : height_ - (height_ / width_) * w);

			// Relative Value (0 .. 1) for calculation of value line
			double relVal;
			if (getMax () != getMin ()) relVal = (getValue () - getMin ()) / (getMax () - getMin ());
			else relVal = 0.5;							// min == max doesn't make any sense, but need to be handled
			if (getStep() < 0) relVal = 1 - relVal;		// Swap if reverse orientation

			double x1 = width_ / 2 - w / 2; double y1 = height_ / 2 - h / 2;									// Top left
			double x2 = width_ / 2 + w / 2 - 1; double y2 = height_ / 2 - h / 2 + (1 - relVal) * (h - 2) + 1; 	// Value line right
			double x3 = x1; double y3 = y2;																		// Value line left
			double x4 = x2; double y4 = height_ / 2 + h / 2 - 1; 												// Bottom right

			if (getStep () < 0) std::swap (y1, y4); // Swap top <-> bottom if reverse orientation

			// Colors uses within this method
			BColors::Color fgInact = *fgColors.getColor (BColors::INACTIVE);
			BColors::Color fgActive = *fgColors.getColor (BColors::ACTIVE);
			BColors::Color fgNormal = *fgColors.getColor (BColors::NORMAL);
			BColors::Color bgNormal = *bgColors.getColor (BColors::NORMAL);
			BColors::Color bgActive = *bgColors.getColor (BColors::ACTIVE);
			BColors::Color bgInact = *bgColors.getColor (BColors::INACTIVE);
			BColors::Color bgOff = *bgColors.getColor (BColors::OFF);

			// Frame background
			cairo_rectangle (cr, x1, y1, x4 - x1 + 1, y4 - y1 + 1);
			cairo_set_line_width (cr, 0.5);
			cairo_set_source_rgba (cr, bgInact.getRed (), bgInact.getGreen (), bgInact.getBlue (), bgInact.getAlpha ());
			cairo_fill_preserve (cr);
			cairo_stroke (cr);

			// Scale background
			cairo_rectangle (cr,x1, y1, x4 - x1, y4 - y1);
			cairo_set_source_rgba (cr, fgInact.getRed (), fgInact.getGreen (), fgInact.getBlue (), fgInact.getAlpha ());
			cairo_fill_preserve (cr);
			cairo_set_source_rgba (cr, bgOff.getRed (), bgOff.getGreen (), bgOff.getBlue (), bgOff.getAlpha ());
			cairo_stroke (cr);

			// Scale active
			pat = cairo_pattern_create_linear (x3, y3, x2, y2);
			cairo_pattern_add_color_stop_rgba (pat, 0.0, fgNormal.getRed (), fgNormal.getGreen (), fgNormal.getBlue (), fgNormal.getAlpha ());
			cairo_pattern_add_color_stop_rgba (pat, 0.25, fgActive.getRed (), fgActive.getGreen (), fgActive.getBlue (), fgActive.getAlpha ());
			cairo_pattern_add_color_stop_rgba (pat, 1, fgNormal.getRed (), fgNormal.getGreen (), fgNormal.getBlue (), fgNormal.getAlpha ());
			cairo_rectangle (cr, x3, y3, x4 - x3, y4 - y3);
			cairo_set_source (cr, pat);
			cairo_fill_preserve (cr);
			cairo_stroke (cr);
			cairo_pattern_destroy (pat);

			//Shadow
			cairo_move_to (cr, x1, (y4 > y1 ? y4 : y1));
			cairo_line_to (cr, x1, (y1 < y4 ? y1 : y4));
			cairo_line_to (cr, x4, (y1 < y4 ? y1 : y4));
			cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 1.0);
			cairo_set_line_width (cr, 1.0);
			cairo_set_source_rgba (cr, bgOff.getRed (), bgOff.getGreen (), bgOff.getBlue (), 0.5 * bgOff.getAlpha ());
			cairo_stroke (cr);


			// Knob
			pat = cairo_pattern_create_radial ((x2 + x3) / 2 - w / 4, (y2 + y3) / 2 - w / 4, 0.1 * w,
											   (x2 + x3) / 2, (y2 + y3) / 2, 1.5 * w);
			cairo_arc (cr, (x2 + x3) / 2, (y2 + y3) / 2, w - 1, 0, 2 * PI);
			cairo_pattern_add_color_stop_rgba (pat, 0, bgActive.getRed (), bgActive.getGreen (), bgActive.getBlue (), bgActive.getAlpha ());
			cairo_pattern_add_color_stop_rgba (pat, 1, bgNormal.getRed (), bgNormal.getGreen (), bgNormal.getBlue (), bgNormal.getAlpha ());
			cairo_set_source (cr, pat);
			cairo_fill_preserve (cr);
			cairo_pattern_destroy (pat);

			pat = cairo_pattern_create_radial ( (x2 + x3) / 2 - w / 4, (y2 + y3) / 2 - w / 4, 0.1 * w,
											    (x2 + x3) / 2, (y2 + y3) / 2, 1.2 * w);
			cairo_pattern_add_color_stop_rgba (pat, 0, bgInact.getRed (), bgInact.getGreen (), bgInact.getBlue (), bgInact.getAlpha ());
			cairo_pattern_add_color_stop_rgba (pat, 1, bgOff.getRed (), bgOff.getGreen (), bgOff.getBlue (), bgOff.getAlpha ());
			cairo_set_line_width (cr, 0.5);
			cairo_set_source (cr, pat);
			cairo_stroke (cr);
			cairo_pattern_destroy (pat);
		}

		cairo_destroy (cr);
	}
}

}
