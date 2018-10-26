#include "RangeWidget.hpp"

#ifndef LIMIT
#define LIMIT(val, min, max) (val < min ? min : (val < max ? val : max))
#endif

namespace BWidgets
{
RangeWidget::RangeWidget () : RangeWidget (0.0, 0.0, 200.0, 200.0, "RangeWidget", 0.0, 0.0, 100.0, 0.0) {}

RangeWidget::RangeWidget (const double  x, const double y, const double width, const double height, const std::string& name,
						  const double value, const double min, const double max, const double step) :
		ValueWidget (x, y, width, height, name, value), rangeMin (min <= max ? min : max),
		rangeMax (max), rangeStep (step)
{
	this->value = LIMIT (value, min, max);
}

RangeWidget::RangeWidget (const RangeWidget& that) :
	ValueWidget (that), rangeMin (that.rangeMin <= that.rangeMax ? that.rangeMin : that.rangeMax), rangeMax (that.rangeMax), rangeStep (that.rangeStep) {}

RangeWidget::~RangeWidget () {}

RangeWidget& RangeWidget::operator= (const RangeWidget& that)
{
	Widget::operator= (that);
	rangeMin = that.rangeMin;
	rangeMax = that.rangeMax;
	rangeStep = that.rangeStep;
	setValue (that.value);

	return *this;
}

void RangeWidget::setValue (const double val)
{
	double valRounded = LIMIT (val, rangeMin, rangeMax);
	if ((rangeStep != 0.0) && (rangeMax >= rangeMin))
	{
		if (rangeStep > 0.0) valRounded = LIMIT (rangeMin + round ((val - rangeMin) / rangeStep) * rangeStep, rangeMin, rangeMax);
		else valRounded = LIMIT (rangeMax - round ((rangeMax - val) / rangeStep) * rangeStep, rangeMin, rangeMax);
	}

	if (value != valRounded) ValueWidget::setValue (valRounded);
}

void RangeWidget::setMin (const double min)
{
	double newMin = (min <= rangeMax ? min: rangeMax);
	if (rangeMin != newMin)
	{
		rangeMin = newMin;
		if (getValue () < rangeMin) setValue (rangeMin);
		update ();
	}
}

double RangeWidget::getMin () const {return rangeMin;}

void RangeWidget::setMax (const double max)
{
	double newMax = (max >= rangeMin ? max: rangeMin);
	if (rangeMax != newMax)
	{
		rangeMax = newMax;
		if (getValue () > rangeMax) setValue (rangeMax);
		update ();
	}
}

double RangeWidget::getMax () const {return rangeMax;}

void RangeWidget::setStep (const double step) {rangeStep = step;}

double RangeWidget::getStep () const {return rangeStep;}


}
