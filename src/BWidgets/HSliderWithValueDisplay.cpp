#include "HSliderWithValueDisplay.hpp"

namespace BWidgets
{
HSliderWithValueDisplay::HSliderWithValueDisplay () :
		HSliderWithValueDisplay (0.0, 0.0, 50.0, 60.0, "valueslider", 0.0, 0.0, 100.0, 1.0, "%3d", ON_TOP) {}

HSliderWithValueDisplay::HSliderWithValueDisplay (const double x, const double y, const double width, const double height, const std::string& name,
												  const double value, const double min, const double max, const double step,
												  const std::string& valueFormat, const ValueDisplayPosition position) :
	RangeWidget (x, y, width, height, name, value, min, max, step),
	slider (0, 0, width, width, name, value, min, max, step),
	valueDisplay(0, 0.75 * height, width, 0.25 * height, name),
	valFormat (valueFormat), valPosition (position)
{
	slider.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, HSliderWithValueDisplay::redirectPostValueChanged);
	valueDisplay.setText (BValues::toBString (valueFormat, value));
	update ();
	add (slider);
	add (valueDisplay);
}

HSliderWithValueDisplay::HSliderWithValueDisplay (const HSliderWithValueDisplay& that) :
		RangeWidget (that),
		slider (that.slider), valueDisplay (that.valueDisplay), valFormat (that.valFormat), valPosition (that.valPosition)
{
	add (slider);
	add (valueDisplay);
}

HSliderWithValueDisplay& HSliderWithValueDisplay::operator= (const HSliderWithValueDisplay& that)
{
	valFormat = that.valFormat;
	valPosition = that.valPosition;
	RangeWidget::operator= (that);
	slider = that.slider;
	valueDisplay = that.valueDisplay;

	return *this;
}

void HSliderWithValueDisplay::setValue (const double val)
{
	RangeWidget::setValue (val);

	// Pass changed value to dial and display
	if (value != slider.getValue ()) slider.setValue (value);
	valueDisplay.setText(BValues::toBString (valFormat, value));
}

void HSliderWithValueDisplay::setMin (const double min)
{
	RangeWidget::setMin (min);

	if (rangeMin != slider.getMin ()) slider.setMin (rangeMin);
}

void HSliderWithValueDisplay::setMax (const double max)
{
	RangeWidget::setMin (max);

	if (rangeMax != slider.getMax ()) slider.setMax (rangeMax);
}

void HSliderWithValueDisplay::setStep (const double step)
{
	RangeWidget::setStep (step);

	if (rangeStep != slider.getStep ()) slider.setStep (rangeStep);
}

HSliderWithValueDisplay::~HSliderWithValueDisplay () {}

void HSliderWithValueDisplay::setValueFormat (const std::string& valueFormat) {valFormat = valueFormat;}
std::string HSliderWithValueDisplay::getValueFormat () const {return valFormat;}
void HSliderWithValueDisplay::setValuePosition (const ValueDisplayPosition position) {valPosition = position;}
ValueDisplayPosition HSliderWithValueDisplay::getValuePosition () const {return valPosition;}
HSlider* HSliderWithValueDisplay::getSlider () {return &slider;}
Label* HSliderWithValueDisplay::getValueDisplay () {return &valueDisplay;}

void HSliderWithValueDisplay::update ()
{
	updateChildCoords ();
	draw (0, 0, width_, height_);
	if (isVisible ()) postRedisplay ();
}

void HSliderWithValueDisplay::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}
void HSliderWithValueDisplay::applyTheme (BStyles::Theme& theme, const std::string& name)
{
	Widget::applyTheme (theme, name);
	slider.applyTheme (theme, name);
	valueDisplay.applyTheme (theme, name);
	update ();
}

void HSliderWithValueDisplay::redirectPostValueChanged (BEvents::Event* event)
{

	if (event && (event->getEventType () == BEvents::EventType::VALUE_CHANGED_EVENT) && event->getWidget ())
	{
		BEvents::ValueChangedEvent* ev = (BEvents::ValueChangedEvent*) event;
		RangeWidget* w = (RangeWidget*) ev->getWidget ();
		if (w->getParent ())
		{
			HSliderWithValueDisplay* p = (HSliderWithValueDisplay*) w->getParent ();

			// Get value and range from slider
			if (p->getValue () != w->getValue ()) p->setValue (w->getValue ());
			if (p->getMin () != w->getMin ()) p->setMin (w->getMin ());
			if (p->getMax () != w->getMax ()) p->setMax (w->getMax ());
			if (p->getStep () != w->getStep ()) p->setStep (w->getStep ());

			p->updateChildCoords ();
		}
	}
}

void HSliderWithValueDisplay::updateChildCoords ()
{
	double th = 0;
	double tw = 0;
	double ty = 0;
	double tx = 0;

	double sh = height_;
	double sw = width_;
	double sx = 0;
	double sy = 0;

	if ((valPosition == ON_TOP) || (valPosition == ON_BOTTOM))
	{
		if (height_ <= 8) height_ = 8;							// Force minimum height

		th = (height_ <= 32 ? height_ / 2 : 16);				// Value display half height, but max. 16 px
		tw = th * 2.2;
		ty = (valPosition == ON_TOP ? 0 : height_ - th);
		tx = width_ / 2 - tw / 2;
		sh = height_ - th;
		sy = (valPosition == ON_TOP ? th : 0);
	}

	if ((valPosition == ON_LEFT) || (valPosition == ON_RIGHT))
	{
		if (width_ <= 16) width_ = 16;							// Force minimum width

		th = (height_ <= 16 ? height_ : 16);					// Value display max. 16 px
		tw = th * 2.2;
		ty = height_ / 2 - th / 2;
		tx = (valPosition == ON_LEFT ? 0: width_ - tw);
		sw = width_ - tw;
		sx = (valPosition == ON_LEFT ? tw: 0);
	}

	slider.moveTo (sx, sy);
	slider.setWidth (sw);
	slider.setHeight (sh);

	valueDisplay.moveTo (tx, ty);
	valueDisplay.setWidth (tw);
	valueDisplay.setHeight (th);
	BStyles::Font* font = valueDisplay.getFont ();
	font->setFontSize (th);
	valueDisplay.update ();
}

void HSliderWithValueDisplay::draw (const double x, const double y, const double width, const double height)
{
	// Draw super class widget elements only
	Widget::draw (x, y, width, height);
}

}
