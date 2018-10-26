#ifndef BWIDGETS_LABEL_HPP_
#define BWIDGETS_LABEL_HPP_

#include "Widget.hpp"

namespace BWidgets
{

/**
 * Class BWidgets::Label
 *
 * Simple text output widget.
 */
class Label : public Widget
{
public:
	Label ();
	Label (const std::string& text);
	Label (const double x, const double y, const double width, const double height, const std::string& text);

	/**
	 * Creates a new (orphan) label and copies the label properties from a
	 * source label. This method doesn't copy any parent or child widgets.
	 * @param that Source label
	 */
	Label (const Label& that);

	~Label ();

	/**
	 * Assignment. Copies the label properties from a source label and keeps
	 * its name and its position within the widget tree. Emits a
	 * BEvents::ExposeEvent if the label is visible.
	 * @param that Source label
	 */
	Label& operator= (const Label& that);

	/**
	 * Sets the output text.
	 * @param text Output text
	 */
	void setText (const std::string& text);

	/**
	 * Gets the output text
	 * @return Output text
	 */
	std::string getText () const;

	/**
	 * Sets the BColors::ColorSet for this widget
	 * @param colors Color set.
	 */
	void setTextColors (const BColors::ColorSet& colorset);

	/**
	 * Gets (a pointer to) the BColors::ColorSet of this widget.
	 * @return Pointer to the color set.
	 */
	BColors::ColorSet* getTextColors ();

	/**
	 * Sets the font for the text output.
	 * @param font Font
	 */
	void setFont (const BStyles::Font& font);

	/**
	 * Gets (a pointer to) the font for the text output.
	 * @return Pointer to font
	 */
	BStyles::Font* getFont ();

	/**
	 * Calls a redraw of the widget and calls postRedisplay () if the the
	 * Widget is visible.
	 * This method should be called if the widgets properties are indirectly
	 * changed.
	 */
	virtual void update () override;

	/**
	 * Scans theme for widget properties and applies these properties.
	 * @param theme Theme to be scanned.
	 * 				Styles used are:
	 * 				"textcolors" for BColors::ColorSet
	 * 				"font" for BStyles::Font
	 * @param name Name of the BStyles::StyleSet within the theme to be
	 * 		  	   applied.
	 */
	virtual void applyTheme (BStyles::Theme& theme);
	virtual void applyTheme (BStyles::Theme& theme, const std::string& name);

protected:
	virtual void draw (const double x, const double y, const double width, const double height) override;

	BColors::ColorSet labelColors;
	BStyles::Font labelFont;
	std::string labelText;
};

}

#endif /* BWIDGETS_LABEL_HPP_ */
