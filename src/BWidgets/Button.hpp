#ifndef BUTTON_HPP_
#define BUTTON_HPP_

#include "ValueWidget.hpp"

namespace BWidgets
{
/**
 * Class BWidgets::Button
 *
 * Basic button widget drawing a button. Is is a BWidgets::ValueWidget having
 * two conditions: on (value != 0) or off (value == 0). The Widget is clickable
 * by default.
 */
class Button : public ValueWidget
{
public:
	Button ();
	Button (const double x, const double y, const double width, const double height, const std::string& name, const double defaultValue);

	/**
	 * Creates a new (orphan) button and copies the button properties from a
	 * source button. This method doesn't copy any parent or child widgets.
	 * @param that Source button
	 */
	Button (const Button& that);

	~Button ();

	/**
	 * Assignment. Copies the widget properties from a source button and keeps
	 * its name and its position within the widget tree. Emits a
	 * BEvents::ExposeEvent if the widget is visible.
	 * @param that Source button
	 */
	Button& operator= (const Button& that);

	/**
	 * Sets the colors of the button
	 * @param colors BColors::ColorSet
	 */
	void setButtonColors (const BColors::ColorSet& colors);

	/**
	 * Gets the colors of the button
	 * @return BColors::ColorSet
	 */
	BColors::ColorSet* getButtonColors ();

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
	 * 				"buttoncolors" for BStyles::ColorSet
	 * @param name Name of the BStyles::StyleSet within the theme to be
	 * 		  	   applied.
	 */
	virtual void applyTheme (BStyles::Theme& theme);
	virtual void applyTheme (BStyles::Theme& theme, const std::string& name);

	/**
	 * Handles the BEvents::BUTTON_PRESS_EVENT.
	 * @param event Pointer to a pointer event emitted by the same widget.
	 */
	virtual void onButtonPressed (BEvents::PointerEvent* event) override;

	/**
	 * Handles the BEvents::BUTTON_RELEASED_EVENT.
	 * @param event Pointer to a pointer event emitted by the same widget.
	 */
	virtual void onButtonReleased (BEvents::PointerEvent* event) override;

protected:
	virtual void draw (const double x, const double y, const double width, const double height) override;

	BColors::ColorSet buttonColors;
	BColors::ColorSet bgColors;
};

}

#endif /* BUTTON_HPP_ */
