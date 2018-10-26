#include "Label.hpp"

namespace BWidgets
{
Label::Label () : Label (0.0, 0.0, 0.0, 0.0, "Label") {}

Label::Label (const std::string& text) : Label (0.0, 0.0, 200.0, 20.0, text) {}

Label::Label (const double x, const double y, const double width, const double height, const std::string& text) :
		Widget (x, y, width, height, text), labelColors (BColors::whites), labelFont (BStyles::sans12pt), labelText (text) {}

Label::Label (const Label& that) : Widget (that)
{
	labelColors = that.labelColors;
	labelFont = that.labelFont;
	labelText = that.labelText;

	draw (0, 0, width_, height_);
}

Label::~Label () {}

Label& Label::operator= (const Label& that)
{
	labelColors = that.labelColors;
	labelFont = that.labelFont;
	labelText = that.labelText;
	Widget::operator= (that);
	return *this;
}

void Label::setText (const std::string& text)
{
	if (text != labelText)
	{
		labelText = text;
		update ();
	}
}
std::string Label::getText () const {return labelText;}

void Label::setTextColors (const BColors::ColorSet& colorset)
{
	labelColors = colorset;
	update ();
}
BColors::ColorSet* Label::getTextColors () {return &labelColors;}

void Label::setFont (const BStyles::Font& font)
{
	labelFont = font;
	update ();
}
BStyles::Font* Label::getFont () {return &labelFont;}

void Label::update ()
{
	draw (0, 0, width_, height_);
	if (isVisible ()) postRedisplay ();
}

void Label::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}

void Label::applyTheme (BStyles::Theme& theme, const std::string& name)
{
	Widget::applyTheme (theme, name);

	// Color
	void* colorsPtr = theme.getStyle(name, "textcolors");
	if (colorsPtr) labelColors = *((BColors::ColorSet*) colorsPtr);

	// Font
	void* fontPtr = theme.getStyle(name, "font");
	if (fontPtr) labelFont = *((BStyles::Font*) fontPtr);

	if (colorsPtr || fontPtr) update ();
}



void Label::draw (const double x, const double y, const double width, const double height)
{
	// Draw super class widget elements first
	Widget::draw (x, y, width, height);

	cairo_t* cr = cairo_create (widgetSurface);

	if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
	{
		// Limit cairo-drawing area
		cairo_rectangle (cr, x, y, width, height);
		cairo_clip (cr);

		cairo_text_extents_t ext = labelFont.getTextExtents(cr, labelText);
		BColors::Color lc = *labelColors.getColor (BColors::NORMAL);
		cairo_set_source_rgba (cr, lc.getRed (), lc.getGreen (), lc.getBlue (), lc.getAlpha ());
		cairo_select_font_face (cr, labelFont.getFontFamily ().c_str (), labelFont.getFontSlant (), labelFont.getFontWeight ());
		cairo_set_font_size (cr, labelFont.getFontSize ());
		cairo_move_to (cr, width_ / 2 - ext.width / 2 - ext.x_bearing, height_ / 2 - ext.height / 2 - ext.y_bearing);
		cairo_show_text (cr, labelText.c_str ());
	}

	cairo_destroy (cr);
}

}
