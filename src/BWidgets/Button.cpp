#include "Button.hpp"

namespace BWidgets
{
Button::Button () : Button (0.0, 0.0, 50.0, 50.0, "button", 0.0) {}

Button::Button (const double x, const double y, const double width, const double height, const std::string& name, const double defaultValue) :
		ValueWidget (x, y, width, height, name, defaultValue), buttonColors (BColors::greys), bgColors (BColors::greys)
{
	setBackground (BStyles::noFill);
	setClickable (true);
}

Button::Button (const Button& that) : ValueWidget (that), buttonColors (that.buttonColors), bgColors (that.buttonColors) {}

Button:: ~Button () {}

Button& Button::operator= (const Button& that)
{
	buttonColors = that.buttonColors;
	bgColors = that.bgColors;
	ValueWidget::operator= (that);
	return *this;
}

void Button::setButtonColors (const BColors::ColorSet& colors)
{
	buttonColors = colors;
	update ();
}

BColors::ColorSet* Button::getButtonColors () {return &buttonColors;}

void Button::update ()
{
	draw (0, 0, width_, height_);
	if (isVisible ()) postRedisplay ();
}

void Button::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}

void Button::applyTheme (BStyles::Theme& theme, const std::string& name)
{
	Widget::applyTheme (theme, name);

	void* btPtr = theme.getStyle(name, "buttoncolors");
	if (btPtr) buttonColors = *((BColors::ColorSet*) btPtr);

	void* bgPtr = theme.getStyle(name, "bgcolors");
	if (bgPtr) bgColors = *((BColors::ColorSet*) bgPtr);

	if (btPtr || bgPtr) update ();
}

void Button::onButtonPressed (BEvents::PointerEvent* event) {setValue (1.0);}

void Button::onButtonReleased (BEvents::PointerEvent* event) {setValue (0.0);}

void Button::draw (const double x, const double y, const double width, const double height)
{
	if ((width_ >= 6) && (height_ >= 6))
	{
		// Draw super class widget elements first
		Widget::draw (x, y, width, height);

		cairo_t* cr = cairo_create (widgetSurface);
		if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
		{
			cairo_pattern_t* pat;

			// Limit cairo-drawing area
			cairo_rectangle (cr, x, y, width, height);
			cairo_clip (cr);

			BColors::Color butColorLo = *buttonColors.getColor (BColors::NORMAL);
			BColors::Color butColorHi = *buttonColors.getColor (BColors::ACTIVE);
			BColors::Color bgColorHi = *bgColors.getColor (BColors::INACTIVE);
			BColors::Color bgColorLo = *bgColors.getColor (BColors::OFF);

			pat = cairo_pattern_create_linear (0, 0 , width_, height_);
			cairo_pattern_add_color_stop_rgba (pat, -3, butColorHi.getRed (), butColorHi.getGreen (), butColorHi.getBlue (), butColorHi.getAlpha ());
			cairo_pattern_add_color_stop_rgba (pat, 4, butColorLo.getRed (), butColorLo.getGreen (), butColorLo.getBlue (), butColorLo.getAlpha ());

			cairo_set_line_width (cr, 0.5);

			if (value)
			{
				cairo_rectangle (cr, 2.5, 2.5, width_ - 3, height_ - 3);
				cairo_set_source (cr, pat);
				cairo_fill (cr);

				cairo_move_to (cr, 2.5, height_ - 0.5);
				cairo_line_to (cr, 2.5, 2.5);
				cairo_line_to (cr, width_-0.5, 2.5);
				cairo_set_source_rgba (cr, bgColorHi.getRed (), bgColorHi.getGreen (), bgColorHi.getGreen (), bgColorHi.getAlpha ());
				cairo_stroke (cr);

				cairo_move_to (cr, width_ - 0.5, 2.5);
				cairo_line_to (cr, width_ - 0.5, height_ - 0.5);
				cairo_line_to (cr, 2.5 , height_ - 0.5);
				cairo_set_source_rgba (cr, bgColorLo.getRed (), bgColorLo.getGreen (), bgColorLo.getGreen (), bgColorLo.getAlpha ());
				cairo_stroke (cr);

			}
			else
			{
				cairo_rectangle (cr, 2.5, 2.5, width_ - 3, height_ - 3);
				cairo_set_source_rgba (cr, bgColorLo.getRed (), bgColorLo.getGreen (), bgColorLo.getBlue (), bgColorLo.getAlpha ());
				cairo_fill_preserve (cr);
				cairo_stroke (cr);

				cairo_rectangle (cr, 0.5, 0.5, width_ - 3, height_ - 3);
				cairo_set_source (cr, pat);
				cairo_fill (cr);

				cairo_move_to (cr, 0.5, height_ - 2.5);
				cairo_line_to (cr, 0.5, 0.5);
				cairo_line_to (cr, width_-2.5, 0.5);
				cairo_set_source_rgba (cr, bgColorHi.getRed (), bgColorHi.getGreen (), bgColorHi.getGreen (), bgColorHi.getAlpha ());
				cairo_stroke (cr);

				cairo_move_to (cr, width_ - 2.5, 0.5);
				cairo_line_to (cr, width_ - 2.5, height_ - 2.5);
				cairo_line_to (cr, 0.5 , height_ - 2.5);
				cairo_set_source_rgba (cr, bgColorLo.getRed (), bgColorLo.getGreen (), bgColorLo.getGreen (), bgColorLo.getAlpha ());
				cairo_stroke (cr);

			}

			cairo_pattern_destroy (pat);
		}
		cairo_destroy (cr);
	}
}

}
