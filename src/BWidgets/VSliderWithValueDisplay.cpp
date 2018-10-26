#include "VSliderWithValueDisplay.hpp"

namespace BWidgets
{
VSliderWithValueDisplay::VSliderWithValueDisplay () :
		VSliderWithValueDisplay (0.0, 0.0, 50.0, 60.0, "valueslider", 0.0, 0.0, 100.0, 1.0, "%3d") {}

VSliderWithValueDisplay::VSliderWithValueDisplay (const double x, const double y, const double width, const double height, const std::string& name,
												  const double value, const double min, const double max, const double step,
												  const std::string& valueFormat) :
	RangeWidget (x, y, width, height, name, value, min, max, step),
	slider (0, 0, width, width, name, value, min, max, step),
	valueDisplay(0, 0.75 * height, width, 0.25 * height, name),
	valFormat (valueFormat)
{
	slider.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, VSliderWithValueDisplay::redirectPostValueChanged);
	valueDisplay.setText (BValues::toBString (valueFormat, value));
	update ();
	add (slider);
	add (valueDisplay);
}

VSliderWithValueDisplay::VSliderWithValueDisplay (const VSliderWithValueDisplay& that) :
		RangeWidget (that),
		slider (that.slider), valueDisplay (that.valueDisplay), valFormat (that.valFormat)
{
	add (slider);
	add (valueDisplay);
}

VSliderWithValueDisplay::~VSliderWithValueDisplay () {}

VSliderWithValueDisplay& VSliderWithValueDisplay::operator= (const VSliderWithValueDisplay& that)
{
	valFormat = that.valFormat;
	RangeWidget::operator= (that);
	slider = that.slider;
	valueDisplay = that.valueDisplay;


	return *this;
}

void VSliderWithValueDisplay::setValue (const double val)
{
	RangeWidget::setValue (val);

	// Pass changed value to dial and display
	if (value != slider.getValue ()) slider.setValue (value);
	valueDisplay.setText(BValues::toBString (valFormat, value));
}

void VSliderWithValueDisplay::setMin (const double min)
{
	RangeWidget::setMin (min);

	if (rangeMin != slider.getMin ()) slider.setMin (rangeMin);
}

void VSliderWithValueDisplay::setMax (const double max)
{
	RangeWidget::setMin (max);

	if (rangeMax != slider.getMax ()) slider.setMax (rangeMax);
}

void VSliderWithValueDisplay::setStep (const double step)
{
	RangeWidget::setStep (step);

	if (rangeStep != slider.getStep ()) slider.setStep (rangeStep);
}


void VSliderWithValueDisplay::setValueFormat (const std::string& valueFormat)
{
	valFormat = valueFormat;
	update ();
}

std::string VSliderWithValueDisplay::getValueFormat () const {return valFormat;}

VSlider* VSliderWithValueDisplay::getSlider () {return &slider;}

Label* VSliderWithValueDisplay::getValueDisplay () {return &valueDisplay;}

void VSliderWithValueDisplay::update ()
{
	updateChildCoords ();
	draw (0, 0, width_, height_);
	if (isVisible ()) postRedisplay ();
}

void VSliderWithValueDisplay::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}
void VSliderWithValueDisplay::applyTheme (BStyles::Theme& theme, const std::string& name)
{
	Widget::applyTheme (theme, name);
	slider.applyTheme (theme, name);
	valueDisplay.applyTheme (theme, name);
	update ();
}

void VSliderWithValueDisplay::redirectPostValueChanged (BEvents::Event* event)
{

	if (event && (event->getEventType () == BEvents::EventType::VALUE_CHANGED_EVENT) && event->getWidget ())
	{
		BEvents::ValueChangedEvent* ev = (BEvents::ValueChangedEvent*) event;
		RangeWidget* w = (RangeWidget*) ev->getWidget ();
		if (w->getParent ())
		{
			VSliderWithValueDisplay* p = (VSliderWithValueDisplay*) w->getParent ();

			// Get value and range from slider
			if (p->getValue () != w->getValue ()) p->setValue (w->getValue ());
			if (p->getMin () != w->getMin ()) p->setMin (w->getMin ());
			if (p->getMax () != w->getMax ()) p->setMax (w->getMax ());
			if (p->getStep () != w->getStep ()) p->setStep (w->getStep ());

			p->updateChildCoords ();
		}
	}
}

void VSliderWithValueDisplay::updateChildCoords ()
{
	double tw = getWidth ();
	double th = tw / 2.2;
	double ty = 0;
	double tx = 0;

	double sh = getHeight () - th;
	double sw = getWidth () / 2;
	double sx = getWidth () / 2 - sw / 2;
	double sy = th;

	slider.moveTo (sx, sy);
	slider.setWidth (sw);
	slider.setHeight (sh);

	valueDisplay.getFont ()->setFontSize (th * 0.8);
	valueDisplay.moveTo (tx, ty);
	valueDisplay.setWidth (tw);
	valueDisplay.setHeight (th);
	valueDisplay.update ();

}

void VSliderWithValueDisplay::draw (const double x, const double y, const double width, const double height)
{
	// Draw super class widget elements only
	Widget::draw (x, y, width, height);
}

}
