#ifndef BWIDGETS_TOGGLEBUTTON_HPP_
#define BWIDGETS_TOGGLEBUTTON_HPP_

#include "Button.hpp"

namespace BWidgets
{
/**
 * Class BWidgets::ToggleButton
 *
 * Basic toggle button widget. Is is a BWidgets::Button and thus a
 * BWidgets::ValueWidget having two conditions: on (value != 0) or off
 * (value == 0)
 */
class ToggleButton : public Button
{
public:
	ToggleButton ();
	ToggleButton (const double x, const double y, const double width, const double height, const std::string& name, const double defaultValue);

	/**
	 * Creates a new (orphan) button and copies the button properties from a
	 * source button. This method doesn't copy any parent or child widgets.
	 * @param that Source button
	 */
	ToggleButton (const ToggleButton& that);

	~ToggleButton ();

	/**
	 * Assignment. Copies the widget properties from a source button and keeps
	 * its name and its position within the widget tree. Emits a
	 * BEvents::ExposeEvent if the widget is visible.
	 * @param that Source button
	 */
	ToggleButton& operator= (const ToggleButton& that);

	/**
	 * Handles the BEvents::BUTTON_PRESS_EVENT.
	 * @param event Pointer to a pointer event emitted by the same widget.
	 */
	virtual void onButtonPressed (BEvents::PointerEvent* event) override;

	/**
	 * Overrides the BEvents::BUTTON_RELEASED_EVENT handled by
	 * BWidgets::Button.
	 * @param event Pointer to a pointer event emitted by the same widget.
	 */
	virtual void onButtonReleased (BEvents::PointerEvent* event) override;
};

}




#endif /* BWIDGETS_TOGGLEBUTTON_HPP_ */
