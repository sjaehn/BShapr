#include "TextButton.hpp"

namespace BWidgets
{
TextButton::TextButton () : TextButton (0.0, 0.0, 50.0, 50.0, "textbutton", 0.0) {}

TextButton::TextButton (const double x, const double y, const double width, const double height, const std::string& name, const double defaultValue) :
		Button (x, y, width, height, name, defaultValue), buttonLabel (0, 0, width - 2, height - 2, name)
{
	buttonLabel.setClickable  (false);
	//setValue (defaultValue);
	add (buttonLabel);
}

TextButton::TextButton (const TextButton& that) : Button (that), buttonLabel (that.buttonLabel)
{
	add (buttonLabel);
}

TextButton:: ~TextButton () {}

TextButton& TextButton::operator= (const TextButton& that)
{
	Button::operator= (that);
	buttonLabel = that.buttonLabel;
	return *this;
}

void TextButton::setValue (const double val)
{
	if (val) buttonLabel.moveTo (2, 2);
	else buttonLabel.moveTo (0, 0);
	Button::setValue (val);
}

Label* TextButton::getLabel () {return &buttonLabel;}

void TextButton::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}
void TextButton::applyTheme (BStyles::Theme& theme, const std::string& name)
{
	Button::applyTheme (theme, name);
	buttonLabel.applyTheme (theme, name);
	update ();
}

}
