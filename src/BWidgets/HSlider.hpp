#ifndef BWIDGETS_HSLIDER_HPP_
#define BWIDGETS_HSLIDER_HPP_

#include "RangeWidget.hpp"

namespace BWidgets
{
/**
 * Class BWidgets::HSlider
 *
 * RangeWidget class for a horizontal slider.
 * The Widget is clickable by default.
 */
class HSlider : public RangeWidget
{
public:
	HSlider ();
	HSlider (const double x, const double y, const double width, const double height, const std::string& name,
			 const double value, const double min, const double max, const double step);

	/**
	 * Creates a new (orphan) slider and copies the slider properties from a
	 * source slider. This method doesn't copy any parent or child widgets.
	 * @param that Source slider
	 */
	HSlider (const HSlider& that);

	~HSlider ();

	/**
	 * Assignment. Copies the slider properties from a source slider and keeps
	 * its name and its position within the widget tree. Emits an expose event
	 * if the widget is visible and a value changed event.
	 * @param that Source slider
	 */
	HSlider& operator= (const HSlider& that);

	/**
	 * Calls a redraw of the widget and calls postRedisplay () if the the
	 * Widget is visible.
	 * This method should be called if the widgets properties are indirectly
	 * changed.
	 */
	virtual void update ();

	/**
	 * Scans theme for widget properties and applies these properties.
	 * @param theme Theme to be scanned.
	 * 				Styles used are:
	 * 				"fgcolors" for BColors::ColorSet (scale)
	 * 				"bgcolors" for BStyles::ColorSet (knob and background)
	 * @param name Name of the BStyles::StyleSet within the theme to be
	 * 		  	   applied.
	 */
	virtual void applyTheme (BStyles::Theme& theme);
	virtual void applyTheme (BStyles::Theme& theme, const std::string& name);

	/**
	 * Handles the BEvents::BUTTON_PRESS_EVENT to move the slider.
	 * @param event Pointer to a poiter event emitted by the same widget.
	 */
	virtual void onButtonPressed (BEvents::PointerEvent* event) override;

	/**
	 * Handles the BEvents::POINTER_MOTION_WHILE_BUTTON_PRESSED_EVENT to move
	 * the slider.
	 * @param event Pointer to a poiter event emitted by the same widget.
	 */
	virtual void onPointerMotionWhileButtonPressed (BEvents::PointerEvent* event) override;

protected:
	virtual void draw (const double x, const double y, const double width, const double height) override;

	BColors::ColorSet fgColors;
	BColors::ColorSet bgColors;
};

}

#endif /* BWIDGETS_HSLIDER_HPP_ */
