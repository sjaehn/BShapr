#include "Dial.hpp"

#ifndef PI
#define PI 3.14159265
#endif

namespace BWidgets
{
Dial::Dial () : Dial (0.0, 0.0, 50.0, 50.0, "dial", 0.0, 0.0, 100.0, 1.0) {}

Dial::Dial (const double x, const double y, const double width, const double height, const std::string& name,
			const double value, const double min, const double max, const double step) :
		RangeWidget (x, y, width, height, name, value, min, max, step), fgColors (BColors::greens), bgColors (BColors::greys)
{
	setClickable (true);
	setDragable (true);
}

Dial::Dial (const Dial& that) : RangeWidget (that), fgColors (that.fgColors), bgColors (that.bgColors) {}

Dial:: ~Dial () {}

Dial& Dial::operator= (const Dial& that)
{
	fgColors = that.fgColors;
	bgColors = that.bgColors;
	RangeWidget::operator= (that);

	return *this;
}

void Dial::update ()
{
	draw (0, 0, width_, height_);
	if (isVisible ()) postRedisplay ();
}

void Dial::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}

void Dial::applyTheme (BStyles::Theme& theme, const std::string& name)
{
	Widget::applyTheme (theme, name);

	// Foreground colors (outer arc, dot)
	void* fgPtr = theme.getStyle(name, "fgcolors");
	if (fgPtr) fgColors = *((BColors::ColorSet*) fgPtr);

	// Background colors (dial)
	void* bgPtr = theme.getStyle(name, "bgcolors");
	if (bgPtr) bgColors = *((BColors::ColorSet*) bgPtr);

	if (fgPtr || bgPtr)
	{
		update ();
	}
}

void Dial::onButtonPressed (BEvents::PointerEvent* event)
{
	// Perform only if minimum requirements are satisfied
	if (main_ && isVisible () && (event->getButton () == BEvents::LEFT_BUTTON))
	{
		double x = event->getX ();
		double y = event->getY ();
		double size = (height_ < width_ ? height_ : width_);
		double dist = (sqrt (pow (x - width_ / 2, 2) + pow (y - height_ / 2, 2)));

		if ((dist >= 0.1 * size) && (dist <= 0.7 * size))
		{
			double angle = atan2 (x - width_ / 2, height_ / 2 - y) + PI;
			if ((angle >= 0.2 * PI) && (angle <= 1.8 * PI))
			{
				double corrAngle = LIMIT (angle, 0.3 * PI, 1.7 * PI);
				if (angle < 0.3 * PI) corrAngle = 0.3 * PI;
				if (angle > 1.7 * PI) corrAngle = 1.7 * PI;
				double frac = (corrAngle - 0.3 * PI) / (1.4 * PI);
				if (getStep () < 0) frac = 1 - frac;
				setValue (getMin () + frac * (getMax () - getMin ()));

			}
		}
	}
}

void Dial::onPointerMotionWhileButtonPressed (BEvents::PointerEvent* event) {onButtonPressed (event);}

void Dial::draw (const double x, const double y, const double width, const double height)
{

	// Draw dial
	// only if minimum requirements satisfied
	if ((height_ >= 12) && (width_ >= 12))
	{
		double size = (height_ < width_ ? height_ : width_);

		// Draw super class widget elements first
		Widget::draw (x, y, width, height);

		cairo_t* cr = cairo_create (widgetSurface);

		if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
		{
			cairo_pattern_t* pat;

			// Limit cairo-drawing area
			cairo_rectangle (cr, x, y, width, height);
			cairo_clip (cr);

			// Relative Value (0 .. 1) for calculation of value line
			double relVal;
			if (getMax () != getMin ()) relVal = (getValue () - getMin ()) / (getMax () - getMin ());
			else relVal = 0.5;							// min == max doesn't make any sense, but need to be handled
			if (getStep() < 0) relVal = 1 - relVal;		// Swap if reverse orientation

			// Colors uses within this method
			BColors::Color fgInact = *fgColors.getColor (BColors::INACTIVE);
			BColors::Color fgActive = *fgColors.getColor (BColors::ACTIVE);
			BColors::Color fgNormal = *fgColors.getColor (BColors::NORMAL);
			BColors::Color bgNormal = *bgColors.getColor (BColors::NORMAL);
			BColors::Color bgActive = *bgColors.getColor (BColors::ACTIVE);
			BColors::Color bgInact = *bgColors.getColor (BColors::INACTIVE);
			BColors::Color bgOff = *bgColors.getColor (BColors::OFF);

			// Outer arc
			cairo_set_source_rgba (cr, fgInact.getRed (), fgInact.getGreen (), fgInact.getBlue (), fgInact.getAlpha ());
			cairo_set_line_width (cr, 0.5);
			cairo_arc (cr, 0.5 * width_, 0.5 * height_, 0.48 * size, PI * 0.8, PI * 2.2);
			cairo_arc_negative (cr, 0.5 * width_, 0.5 * height_,  0.35 * size, PI * 2.2, PI * 0.8);
			cairo_close_path (cr);
			cairo_fill_preserve (cr);
			cairo_set_source_rgba (cr, bgInact.getRed (), bgInact.getGreen (), bgInact.getBlue (), bgInact.getAlpha ());
			cairo_stroke (cr);

			// Filled part (= active) of outer arc
			pat = cairo_pattern_create_linear (0.5 * width_ - 0.5 * size, 0.5 * height - 0.5 * size,
											   0.5 * width_ + 0.5 * size, 0.5 * height + 0.5 * size);
			cairo_pattern_add_color_stop_rgba (pat, 0.0, fgNormal.getRed (), fgNormal.getGreen (), fgNormal.getBlue (), fgNormal.getAlpha ());
			cairo_pattern_add_color_stop_rgba (pat, 0.25, fgActive.getRed (), fgActive.getGreen (), fgActive.getBlue (), fgActive.getAlpha ());
			cairo_pattern_add_color_stop_rgba (pat, 1, fgNormal.getRed (), fgNormal.getGreen (), fgNormal.getBlue (), fgNormal.getAlpha ());
			if (getStep () >= 0)
			{
				cairo_arc (cr, 0.5 * width_, 0.5 * height_,  0.48 * size, PI * 0.8, PI * (0.8 + 1.4 * relVal));
				cairo_arc_negative (cr, 0.5 * width_, 0.5 * height_, 0.35 * size, PI * (0.8 + 1.4 * relVal), PI * 0.8);
			}
			else
			{
				cairo_arc (cr, 0.5 * width_, 0.5 * height_,  0.48 * size, PI * (0.8 + 1.4 * relVal), PI * 2.2);
				cairo_arc_negative (cr, 0.5 * width_, 0.5 * height_, 0.35 * size, PI * 2.2, PI * (0.8 + 1.4 * relVal));
			}
			cairo_close_path (cr);
			cairo_set_source (cr, pat);
			cairo_fill (cr);

			// Inner circle
			cairo_arc (cr, 0.5 * width_, 0.5 * height_, 0.3 * size, 0, 2 * PI);
			cairo_close_path (cr);


			pat = cairo_pattern_create_radial (0.5 * width_ - 0.25 * size, 0.5 * height_ - 0.25 * size, 0.1 * size,
											   0.5 * width_, 0.5 * height_, 1.5 * size);
			cairo_pattern_add_color_stop_rgba (pat, 0, bgActive.getRed (), bgActive.getGreen (), bgActive.getBlue (), bgActive.getAlpha ());
			cairo_pattern_add_color_stop_rgba (pat, 1, bgNormal.getRed (), bgNormal.getGreen (), bgNormal.getBlue (), bgNormal.getAlpha ());
			cairo_set_source (cr, pat);
			cairo_fill_preserve (cr);
			cairo_pattern_destroy (pat);

			pat = cairo_pattern_create_radial (0.5 * width_ - 0.25 * size, 0.5 * height_ - 0.25 * size, 0.1 * size,
											   0.5 * width_, 0.5 * height_, 1.5 * size);
			cairo_pattern_add_color_stop_rgba (pat, 0, bgInact.getRed (), bgInact.getGreen (), bgInact.getBlue (), bgInact.getAlpha ());
			cairo_pattern_add_color_stop_rgba (pat, 1, bgOff.getRed (), bgOff.getGreen (), bgOff.getBlue (), bgOff.getAlpha ());
			cairo_set_line_width (cr, 0.5);
			cairo_set_source (cr, pat);
			cairo_stroke (cr);
			cairo_pattern_destroy (pat);

			// Dot
			cairo_set_source_rgba (cr, fgActive.getRed (), fgActive.getGreen (), fgActive.getBlue (), fgActive.getAlpha ());
			cairo_arc (cr, 0.5 * width_ + 0.20 * size * cos (PI * (0.8 + 1.4 *relVal)),
					   0.5 * height_ + 0.20 * size * sin (PI * (0.8 + 1.4 *relVal)),
					   0.05 * size, 0, PI * 2);
			cairo_close_path (cr);
			cairo_fill (cr);
		}

		cairo_destroy (cr);
	}
}

}
