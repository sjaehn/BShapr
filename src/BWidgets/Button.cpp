#include "Button.hpp"

namespace BWidgets
{
Button::Button () : Button (0.0, 0.0, 50.0, 50.0, "button", 0.0) {}

Button::Button (const double x, const double y, const double width, const double height, const std::string& name, const double defaultValue) :
		ValueWidget (x, y, width, height, name, defaultValue), buttonColors (BColors::greys)
{
	setClickable (true);
}

Button::Button (const Button& that) : ValueWidget (that), buttonColors (that.buttonColors) {}

Button:: ~Button () {}

Button& Button::operator= (const Button& that)
{
	buttonColors = that.buttonColors;
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

	void* bcPtr = theme.getStyle(name, "buttoncolors");
	if (bcPtr)
	{
		buttonColors = *((BColors::ColorSet*) bcPtr);
		update ();
	}
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

			BColors::Color bcNormal = *buttonColors.getColor (BColors::NORMAL);
			BColors::Color bcActive = *buttonColors.getColor (BColors::ACTIVE);
			BColors::Color bcInactive = *buttonColors.getColor (BColors::INACTIVE);
			BColors::Color bcOFF = *buttonColors.getColor (BColors::OFF);

			pat = cairo_pattern_create_linear (0, 0 , width_, height_);
			cairo_pattern_add_color_stop_rgba (pat, 0.0, bcActive.getRed (), bcActive.getGreen (), bcActive.getBlue (), bcActive.getAlpha ());
			cairo_pattern_add_color_stop_rgba (pat, 1, bcNormal.getRed (), bcNormal.getGreen (), bcNormal.getBlue (), bcNormal.getAlpha ());

			cairo_set_line_width (cr, 0.5);

			if (value)
			{
				cairo_rectangle (cr, 2.5, 2.5, width_ - 3, height_ - 3);
				cairo_set_source (cr, pat);
				cairo_fill (cr);

				cairo_move_to (cr, 2.5, height_ - 0.5);
				cairo_line_to (cr, 2.5, 2.5);
				cairo_line_to (cr, width_-0.5, 2.5);
				cairo_set_source_rgba (cr, bcInactive.getRed (), bcInactive.getGreen (), bcInactive.getGreen (), bcInactive.getAlpha ());
				cairo_stroke (cr);

				cairo_move_to (cr, width_ - 0.5, 2.5);
				cairo_line_to (cr, width_ - 0.5, height_ - 0.5);
				cairo_line_to (cr, 2.5 , height_ - 0.5);
				cairo_set_source_rgba (cr, bcOFF.getRed (), bcOFF.getGreen (), bcOFF.getGreen (), bcOFF.getAlpha ());
				cairo_stroke (cr);

			}
			else
			{
				cairo_rectangle (cr, 2.5, 2.5, width_ - 3, height_ - 3);
				cairo_set_source_rgba (cr, bcOFF.getRed (), bcOFF.getGreen (), bcOFF.getBlue (), bcOFF.getAlpha ());
				cairo_fill_preserve (cr);
				cairo_stroke (cr);

				cairo_rectangle (cr, 0.5, 0.5, width_ - 3, height_ - 3);
				cairo_set_source (cr, pat);
				cairo_fill (cr);

				cairo_move_to (cr, 0.5, height_ - 2.5);
				cairo_line_to (cr, 0.5, 0.5);
				cairo_line_to (cr, width_-2.5, 0.5);
				cairo_set_source_rgba (cr, bcInactive.getRed (), bcInactive.getGreen (), bcInactive.getGreen (), bcInactive.getAlpha ());
				cairo_stroke (cr);

				cairo_move_to (cr, width_ - 2.5, 0.5);
				cairo_line_to (cr, width_ - 2.5, height_ - 2.5);
				cairo_line_to (cr, 0.5 , height_ - 2.5);
				cairo_set_source_rgba (cr, bcOFF.getRed (), bcOFF.getGreen (), bcOFF.getGreen (), bcOFF.getAlpha ());
				cairo_stroke (cr);

			}

			cairo_pattern_destroy (pat);
		}
		cairo_destroy (cr);
	}
}

}
