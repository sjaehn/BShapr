#include "ToggleButton.hpp"

namespace BWidgets
{
ToggleButton::ToggleButton () : ToggleButton (0.0, 0.0, 50.0, 50.0, "togglebutton", 0.0) {}

ToggleButton::ToggleButton (const double x, const double y, const double width, const double height, const std::string& name, const double defaultValue) :
		Button (x, y, width, height, name, defaultValue) {}

ToggleButton::ToggleButton (const ToggleButton& that) : Button (that) {}

ToggleButton:: ~ToggleButton () {}

ToggleButton& ToggleButton::operator= (const ToggleButton& that)
{
	Button::operator= (that);
	return *this;
}

void ToggleButton::onButtonPressed (BEvents::PointerEvent* event)
{
	if (getValue ()) setValue (0.0);
	else setValue (1.0);
}

void ToggleButton::onButtonReleased (BEvents::PointerEvent* event) {}

}
