#include "DialWithValueDisplay.hpp"

namespace BWidgets
{

DialWithValueDisplay::DialWithValueDisplay () :
		DialWithValueDisplay (0.0, 0.0, 50.0, 60.0, "valuedial", 0.0, 0.0, 100.0, 1.0, "%3.2f") {}

DialWithValueDisplay::DialWithValueDisplay (const double x, const double y, const double width, const double height, const std::string& name,
											const double value, const double min, const double max, const double step,
											const std::string& valueFormat) :
	RangeWidget (x, y, width, height, name, value, min, max, step),
	dial (0, 0, width, width, name, value, min, max, step),
	valueDisplay(0, 0.75 * height, width, 0.25 * height, name),
	valFormat (valueFormat)
{
	dial.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, DialWithValueDisplay::redirectPostValueChanged);
	valueDisplay.setText (BValues::toBString (valueFormat, value));
	update ();
	add (dial);
	add (valueDisplay);
}

DialWithValueDisplay::DialWithValueDisplay (const DialWithValueDisplay& that) :
		RangeWidget (that), dial (that.dial), valueDisplay (that.valueDisplay), valFormat (that.valFormat)
{
	add (dial);
	add (valueDisplay);
}

DialWithValueDisplay::~DialWithValueDisplay () {}

DialWithValueDisplay& DialWithValueDisplay::operator= (const DialWithValueDisplay& that)
{
	valFormat = that.valFormat;
	RangeWidget::operator= (that);
	dial = that.dial;
	valueDisplay = that.valueDisplay;

	return *this;
}

void DialWithValueDisplay::setValue (const double val)
{
	RangeWidget::setValue (val);

	// Pass changed value to dial and display
	if (value != dial.getValue ()) dial.setValue (value);
	valueDisplay.setText(BValues::toBString (valFormat, value));
}

void DialWithValueDisplay::setMin (const double min)
{
	RangeWidget::setMin (min);

	if (rangeMin != dial.getMin ()) dial.setMin (rangeMin);
}

void DialWithValueDisplay::setMax (const double max)
{
	RangeWidget::setMin (max);

	if (rangeMax != dial.getMax ()) dial.setMax (rangeMax);
}

void DialWithValueDisplay::setStep (const double step)
{
	RangeWidget::setStep (step);

	if (rangeStep != dial.getStep ()) dial.setStep (rangeStep);
}

void DialWithValueDisplay::setValueFormat (const std::string& valueFormat)
{
	valFormat = valueFormat;
	update ();
}

std::string DialWithValueDisplay::getValueFormat () const {return valFormat;}

Dial* DialWithValueDisplay::getDial () {return &dial;}

Label* DialWithValueDisplay::getValueDisplay () {return &valueDisplay;}

void DialWithValueDisplay::update ()
{
	updateChildCoords ();
	draw (0, 0, width_, height_);
	if (isVisible ()) postRedisplay ();
}

void DialWithValueDisplay::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}
void DialWithValueDisplay::applyTheme (BStyles::Theme& theme, const std::string& name)
{
	Widget::applyTheme (theme, name);
	dial.applyTheme (theme, name);
	valueDisplay.applyTheme (theme, name);
	update ();
}

void DialWithValueDisplay::updateChildCoords ()
{
	double w = (height_ > width_ ? width_ : height_);
	double h = (height_ > width_ ? width_ : height_);
	dial.moveTo (0.5 * width_ - 0.5 * w, 0.5 * height_ - 0.5 * w);
	dial.setWidth (w);
	dial.setHeight (w);

	valueDisplay.moveTo (0.5 * width_ - 0.5 * w, 0.5 * height_ + 0.3 * h);
	valueDisplay.setWidth (w);
	valueDisplay.setHeight (0.25 * h);
	valueDisplay.getFont ()->setFontSize (0.2 * w);
	valueDisplay.setText (BValues::toBString (valFormat, value));
	valueDisplay.update ();
}

void DialWithValueDisplay::redirectPostValueChanged (BEvents::Event* event)
{
	if (event && (event->getEventType () == BEvents::EventType::VALUE_CHANGED_EVENT) && event->getWidget ())
	{
		BEvents::ValueChangedEvent* ev = (BEvents::ValueChangedEvent*) event;
		RangeWidget* w = (RangeWidget*) ev->getWidget ();
		if (w->getParent ())
		{
			DialWithValueDisplay* p = (DialWithValueDisplay*) w->getParent ();

			// Get value and range from dial
			if (p->getValue () != w->getValue ()) p->setValue (w->getValue ());
			if (p->getMin () != w->getMin ()) p->setMin (w->getMin ());
			if (p->getMax () != w->getMax ()) p->setMax (w->getMax ());
			if (p->getStep () != w->getStep ()) p->setStep (w->getStep ());
		}
	}
}

void DialWithValueDisplay::draw (const double x, const double y, const double width, const double height)
{
	// Draw super class widget elements only
	Widget::draw (x, y, width, height);
}

}
