#ifndef BWIDGETS_VALUEWIDGET_HPP_
#define BWIDGETS_VALUEWIDGET_HPP_

#include "Widget.hpp"

namespace BWidgets
{

/**
 * Class BWidgets::ValueWidget
 *
 * Root class for all widgets that deal with values (dials, sliders, ...).
 */
class ValueWidget : public Widget
{
public:
	ValueWidget ();
	ValueWidget (const double x, const double y, const double width, const double height, const std::string& name, const double value);

	/**
	 * Creates a new (orphan) widget and copies the widget properties from a
	 * source widget. This method doesn't copy any parent or child widgets.
	 * @param that Source widget
	 */
	ValueWidget (const ValueWidget& that);

	~ValueWidget ();

	/**
	 * Assignment. Copies the widget properties from a source widget and keeps
	 * its name and its position within the widget tree. Emits an expose event
	 * if the widget is visible and a value changed event.
	 * @param that Source widget
	 */
	ValueWidget& operator= (const ValueWidget& that);

	/**
	 * Changes the value of the widget. Emits a value changed event and (if
	 * visible) an expose event.
	 * @param val Value
	 */
	virtual void setValue (const double val);

	/**
	 * Gets the value of the widget.
	 * @return Value
	 */
	double getValue () const;

protected:
	void postValueChanged ();
	double value;
};

}

#endif /* BWIDGETS_VALUEWIDGET_HPP_ */
