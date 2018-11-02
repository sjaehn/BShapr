#ifndef BWIDGETS_HSWITCH_HPP_
#define BWIDGETS_HSWITCH_HPP_

#include "ToggleButton.hpp"

namespace BWidgets
{
/**
 * Class BWidgets::HSwitch
 *
 * On/OFF switch widget. Is is a BWidgets::ToggleButton and thus a
 * BWidgets::ValueWidget having two conditions: on (value != 0) or off
 * (value == 0)
 */
class HSwitch : public ToggleButton
{
public:
	HSwitch ();
	HSwitch (const double x, const double y, const double width, const double height, const std::string& name, const double defaultValue);

	/**
	 * Creates a new (orphan) switch and copies the button properties from a
	 * source switch. This method doesn't copy any parent or child widgets.
	 * @param that Source switch
	 */
	HSwitch (const HSwitch& that);

	~HSwitch ();

	/**
	 * Assignment. Copies the widget properties from a source switch and keeps
	 * its name and its position within the widget tree. Emits a
	 * BEvents::ExposeEvent if the widget is visible.
	 * @param that Source switch
	 */
	HSwitch& operator= (const HSwitch& that);

	/**
	 * Scans theme for widget properties and applies these properties.
	 * @param theme Theme to be scanned.
	 * @param name Name of the BStyles::StyleSet within the theme to be
	 * 		  	   applied.
	 */
	virtual void applyTheme (BStyles::Theme& theme);
	virtual void applyTheme (BStyles::Theme& theme, const std::string& name);

protected:
	virtual void draw (const double x, const double y, const double width, const double height) override;

	BColors::ColorSet labelColors;
};

}

#endif /* BWIDGETS_TOGGLEBUTTON_HPP_ */
