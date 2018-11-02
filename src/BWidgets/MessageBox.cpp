#include "MessageBox.hpp"

namespace BWidgets
{
MessageBox::MessageBox () : MessageBox (0.0, 0.0, 0.0, 0.0, "MessageBox", "", "MessageBox") {}

MessageBox::MessageBox (const double x, const double y, const double width, const double height,
						const std::string& text, std::vector<std::string> buttons) :
		MessageBox (x, y, width, height, text, "", text, buttons) {}

MessageBox::MessageBox (const double x, const double y, const double width, const double height,
						const std::string& title, const std::string& text, std::vector<std::string> buttons) :
		MessageBox (x, y, width, height, title, title, text, buttons) {}

MessageBox::MessageBox (const double x, const double y, const double width, const double height,
						const std::string& name, const std::string& title, const std::string& text, std::vector<std::string> buttonLabels) :
		ValueWidget (x, y, width, height, name, 0.0),
		titleBox (0, 0, 0, 0, name, ""), textBox (0, 0, 0, 0, name, ""), okButton (0, 0, 40, 20, "OK", 0.0), buttons (),
		v (0.0)
{
	// Title
	setTitle (title);
	titleBox.setBackground (BStyles::noFill);
	titleBox.setTextColors (BColors::darks);
	titleBox.setClickable (false);
	add (titleBox);

	// Text
	setText (text);
	textBox.setBackground (BStyles::noFill);
	textBox.setTextColors (BColors::darks);
	textBox.setClickable (false);
	add (textBox);

	// Buttons
	if (!buttonLabels. empty ()) addButtons (buttonLabels);

	// Or at least the OK button
	if (buttons.empty ())
	{
		okButton.getLabel ()->setTextColors(BColors::darks);
		okButton.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, MessageBox::redirectPostValueChanged);
		buttons.push_back (&okButton);
		rearrangeButtons ();
		add (okButton);
	}

	setBackground (BStyles::greyFill);
	setBorder (BStyles::whiteBorder1pt);
	setDragable (true);
}

MessageBox::MessageBox (const MessageBox& that) : ValueWidget (that)
{
	titleBox = that.titleBox;
	add (titleBox);
	textBox = that.textBox;
	add (textBox);

	for (TextButton* b : that.buttons)
	{
		addButton (b->getLabel ()->getText ());
	}

	okButton = that.okButton;
	if (buttons.empty ())
	{
		okButton.getLabel ()->setTextColors(BColors::darks);
		okButton.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, MessageBox::redirectPostValueChanged);
		buttons.push_back (&okButton);
		rearrangeButtons ();
		add (okButton);
	}

	update ();
}

MessageBox::~MessageBox ()
{
	for (std::vector<TextButton*>::iterator it = buttons.end (); !buttons.empty (); it = buttons.end ())
	{
		release (buttons.back ());
		if (buttons.back () != &okButton)
		{
			TextButton* b = buttons.back ();
			delete b;
		}
		buttons.pop_back ();
	}
}

MessageBox& MessageBox::operator= (const MessageBox& that)
{
	titleBox = that.titleBox;
	textBox = that.textBox;
	okButton = that.okButton;

	// Clean buttons first
	for (TextButton* b : buttons) delete b;

	// Hard copy buttons
	for (TextButton* b : that.buttons)
	{
		addButton (b->getLabel ()->getText ());
	}

	// At least an OK button
	if (buttons.empty ())
	{
		okButton.getLabel ()->setTextColors(BColors::darks);
		okButton.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, MessageBox::redirectPostValueChanged);
		buttons.push_back (&okButton);
		rearrangeButtons ();
		add (okButton);
	}

	Widget::operator= (that);
	return *this;
}

void MessageBox::setTitle (const std::string& title)
{
	if (title != "")
	{
		double width = getEffectiveWidth ();
		double height = getEffectiveHeight ();
		titleBox.setWidth (width > 20 ? width - 20 : 0);
		titleBox.setHeight (height > 50 ? height - 50 : 0);
		titleBox.getFont ()->setFontWeight (CAIRO_FONT_WEIGHT_BOLD);
		titleBox.setText (title);
		double titleheight = titleBox.getTextBlockHeight(titleBox.getTextBlock ());
		titleBox.setHeight (titleheight < height - 50 ? titleheight : (height > 50 ? height - 50 : 0));
		titleBox.moveTo (10, 10);
	}
	else
	{
		titleBox.setWidth (0);
		titleBox.setHeight (0);
		titleBox.moveTo (0, 0);
	}
}

std::string MessageBox::getTitle () const {return titleBox.getText ();}

void MessageBox::setText (const std::string& text)
{
	if (text != "")
	{
		double titleheight = titleBox.getHeight () + 10;
		double width = getEffectiveWidth ();
		double height = getEffectiveHeight ();
		textBox.setWidth (width > 20 ? width - 20 : 0);
		textBox.setHeight (height > 50 + titleheight ? height - (50 + titleheight) : 0);
		textBox.setText (text);
		textBox.moveTo (10, 10 + titleheight);
	}
	else
	{
		textBox.setWidth (0);
		textBox.setHeight (0);
		textBox.moveTo (0, 0);
	}
}

std::string MessageBox::getText () const {return textBox.getText ();}

void MessageBox::addButton (const std::string& label)
{
	TextButton* b = new TextButton(0, 0, 40, 20, label, 0.0);
	if (b)
	{
		cairo_t* cr = cairo_create (widgetSurface);
		cairo_text_extents_t ext = b->getLabel()->getFont()->getTextExtents (cr, label);
		cairo_destroy (cr);
		b->setWidth (ext.width > 30 ? ext.width + 10 : 40);
		b->getLabel()->setTextColors(BColors::darks);
		b->setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, MessageBox::redirectPostValueChanged);
		buttons.push_back (b);
		rearrangeButtons ();
		add (*b);
	}
}

void MessageBox::addButtons (std::vector<std::string> labels)
{
	for (std::string label : labels) addButton (label);
}

void MessageBox::removeButton  (const std::string& label)
{
	for (std::vector<TextButton*>::iterator it = buttons.begin (); it != buttons.end (); ++it)
	{
		TextButton* b = (TextButton*) *it;
		if (b->getLabel ()->getText () == label)
		{
			delete b;
			buttons.erase (it);
			rearrangeButtons ();
			return;
		}
	}
}

double MessageBox::getButtonValue  (const std::string& label) const
{
	double nr = 1;
	for (TextButton* b : buttons)
	{
		if (b->getLabel ()->getText () == label) return nr;
		nr++;
	}
	return 0;
}

void MessageBox::setTextColors (const BColors::ColorSet& colorset) {textBox.setTextColors (colorset);}

BColors::ColorSet* MessageBox::getTextColors () {return textBox.getTextColors ();}

void MessageBox::setFont (const BStyles::Font& font) {textBox.setFont (font);}

BStyles::Font* MessageBox::getFont () {return textBox.getFont ();}

void MessageBox::update ()
{
	draw (0, 0, width_, height_);
	if (isVisible ()) postRedisplay ();
}

void MessageBox::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}

void MessageBox::applyTheme (BStyles::Theme& theme, const std::string& name)
{
	/*Widget::applyTheme (theme, name);
	textBox.applyTheme (theme, name);
	okButton.applyTheme (theme, name);
	update ();*/
}

void MessageBox::rearrangeButtons ()
{
	double width = getEffectiveWidth ();
	double height = getEffectiveHeight ();

	// Calculate total width
	int nrbuttons = 0;
	double totalbuttonwidth = 0.0;
	for (TextButton* b : buttons)
	{
		++nrbuttons;
		totalbuttonwidth += b->getWidth ();
	}

	// Calculate spaces and offset
	double buttonspace = (width - totalbuttonwidth) / (nrbuttons + 1);
	if (buttonspace < 10) buttonspace = 10;
	double buttonxpos = (totalbuttonwidth + (nrbuttons + 1) * buttonspace < width ?
						 buttonspace :
						 width / 2 - (totalbuttonwidth + (nrbuttons - 1) * buttonspace) / 2);

	// Rearrange
	for (TextButton* b : buttons)
	{
		b->moveTo (buttonxpos, height - 30);
		buttonxpos = buttonxpos + buttonspace + b->getWidth ();
	}
}

void MessageBox::redirectPostValueChanged (BEvents::Event* event)
{
	if (event && (event->getEventType () == BEvents::EventType::VALUE_CHANGED_EVENT) && event->getWidget ())
	{
		BEvents::ValueChangedEvent* ev = (BEvents::ValueChangedEvent*) event;
		TextButton* w = (TextButton*) ev->getWidget ();
		if (w->getParent ())
		{
			std::string label = w->getLabel ()->getText ();
			MessageBox* p = (MessageBox*) w->getParent ();
			if (p->getParent ())
			{
				// Set v on click
				if (w->getValue ()) p->v = p->getButtonValue (label);

				// Emit value changed event with v and close (release) message box
				if (!w->getValue () && p->v)
				{
					p->setValue (p->v);
					p->getParent ()->release (p);
				}
			}
		}
	}
}

void MessageBox::draw (const double x, const double y, const double width, const double height)
{
	// Draw super class widget elements only
	Widget::draw (x, y, width, height);
}

}
