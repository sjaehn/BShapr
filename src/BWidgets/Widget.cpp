/* Copyright (C) 2018 by Sven Jähnichen
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "Widget.hpp"

namespace BWidgets
{

Widget::Widget () : Widget (0.0, 0.0, 200.0, 200.0, "Widget") {}

Widget::Widget (const double x, const double y, const double width, const double height) : Widget (x, y, width, height, "Widget") {}

Widget::Widget(const double x, const double y, const double width, const double height, const std::string& name) :
		x_ (x), y_ (y), width_ (width), height_ (height), visible (true), clickable (true), dragable (false),
		main_ (nullptr), parent_ (nullptr), children_ (), border_ (BStyles::noBorder), background_ (BStyles::blackFill), name_ (name)
{
	cbfunction.fill (Widget::defaultCallback);

	widgetSurface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
	draw (0, 0, width_, height_);
}

Widget::Widget (const Widget& that) :
		x_ (that.x_), y_ (that.y_), width_ (that.width_), height_ (that.height_),
		visible (that.visible), clickable (that.clickable), dragable (that.dragable),
		main_ (nullptr), parent_ (nullptr), children_ (), border_ (that.border_), background_ (that.background_), name_ (that.name_),
		cbfunction (that.cbfunction)
{
	widgetSurface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, that.width_, that.height_);
	draw (0, 0, width_, height_);
}

Widget::~Widget()
{
	// Release from parent (and main) if still linked
	if (parent_) parent_->release (this);

	//Release children
	for (Widget* w : children_) release (w);

	cairo_surface_destroy (widgetSurface);
}

Widget& Widget::operator= (const Widget& that)
{
	x_ = that.x_;
	y_ = that.y_;
	width_ = that.width_;
	height_ = that.height_;
	visible = that.visible;
	clickable = that.clickable;
	dragable = that.dragable;
	border_ = that.border_;
	background_ = that.background_;
	cbfunction = that.cbfunction;

	if (widgetSurface) cairo_surface_destroy (widgetSurface);
	widgetSurface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, that.width_, that.height_);
	update ();
	return *this;
}

void Widget::show ()
{
	visible = true;

	if (isVisible ())
	{
		// (Re-)draw children as they may become visible too
		std::vector<Widget*> queue = getChildrenAsQueue ();
		for (Widget* w : queue)
		{
			w->main_ = main_;
			if (w->isVisible ()) w->draw (0, 0, w->width_, w->height_);
		}

		// (Re-)draw this widget and post redisplay
		update ();
	}
}

void Widget::hide ()
{
	visible = false;
	if ((parent_) && parent_->isVisible ()) postRedisplay ();
}

void Widget::add (Widget& child)
{
	child.main_ = main_;
	child.parent_ = this;
	children_.push_back (&child);

	// (Re-)draw children of child as they may become visible too
	if (child.isVisible ())
	{
		std::vector<Widget*> queue = child.getChildrenAsQueue ();
		for (Widget* w : queue)
		{
			w->main_ = main_;
			if (w->isVisible ()) w->draw (0, 0, w->width_, w->height_);
		}

		// (Re-)draw child widget and post redisplay
		child.update ();
	}
}

void Widget::release (Widget* child)
{
	if (child)
	{
		// Delete child's connection to this widget
		child->parent_ = nullptr;

		if (child->main_)
		{
			// Release child from main window input connections
			for (int i = (int) BEvents::NO_BUTTON; i < (int) BEvents::NR_OF_BUTTONS; ++i)
			{
				if (child->main_-> getInput ((BEvents::InputDevice) i) == child) child->main_-> setInput ((BEvents::InputDevice) i, nullptr);
			}

			// Remove connection to main window
			child->main_ = nullptr;
		}

		// Delete connection to released child
		for (std::vector<Widget*>::iterator it = children_.begin (); it !=children_.end (); ++it)
		{
			if ((Widget*) *it == child)
			{
				if (((Widget*) *it)->isVisible ())
				{
					children_.erase (it);
					postRedisplay ();
				}
				else children_.erase (it);
				return;
			}
		}

		std::cerr << "Msg from BWidgets::Widget::release(): Child " << child->name_ << " is already released." << std::endl;
	}
}

void Widget::moveTo (const double x, const double y)
{
	if ((x_ != x) || (y_ != y))
	{
		if (isVisible ())
		{
			bool vis = visible;
			visible = false;
			postRedisplay ();
			x_ = x;
			y_ = y;
			visible = vis;
			postRedisplay ();
		}
		else
		{
			x_ = x;
			y_ = y;
		}
	}
}

double Widget::getX () const {return x_;}

double Widget::getY () const {return y_;}

double Widget::getOriginX ()
{
	double x = 0.0;
	for (Widget* w = this; w->parent_; w = w->parent_) x += w->x_;
	return x;
}

double Widget::getOriginY ()
{
	double y = 0.0;
	for (Widget* w = this; w->parent_; w = w->parent_) y += w->y_;
	return y;
}

void Widget::moveFrontwards ()
{
	if (parent_)
	{
		int size = parent_->children_.size ();
		for (int i = 0; (i + 1) < size; ++i)
		{
			if (parent_->children_[i] == this)
			{
				// Swap
				Widget* w = parent_->children_[i + 1];
				parent_->children_[i + 1] = parent_->children_[i];
				parent_->children_[i] = w;

				if (parent_->isVisible ()) parent_->postRedisplay ();
				return;
			}
		}
	}
}

void Widget::moveBackwards ()
{
	if (parent_)
	{
		int size = parent_->children_.size ();
		for (int i = 1; i < size; ++i)
		{
			if (parent_->children_[i] == this)
			{
				// Swap
				Widget* w = parent_->children_[i];
				parent_->children_[i] = parent_->children_[i - 1];
				parent_->children_[i - 1] = w;

				if (parent_->isVisible ()) parent_->postRedisplay ();
				return;
			}
		}
	}
}

void Widget::setWidth (const double width)
{
	if (width_ != width)
	{
		if (isVisible ())
		{
			bool vis = visible;
			visible = false;
			postRedisplay ();
			width_ = width;
			visible = vis;
			cairo_surface_destroy (widgetSurface);	// destroy old surface first
			widgetSurface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width_, height_);
			draw (0, 0, width_, height_);
			postRedisplay ();
		}
		else
		{
			width_ =  width;
			cairo_surface_destroy (widgetSurface);	// destroy old surface first
			widgetSurface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width_, height_);
			draw (0, 0, width_, height_);
		}
	}
}

double Widget::getWidth () const {return width_;}

void Widget::setHeight (const double height)
{
	if (height_ != height)
	{
		if (isVisible ())
		{
			bool vis = visible;
			visible = false;
			postRedisplay ();
			height_ = height;
			visible = vis;
			cairo_surface_destroy (widgetSurface);	// destroy old surface first
			widgetSurface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width_, height_);
			draw (0, 0, width_, height_);
			postRedisplay ();
		}
		else
		{
			height_ = height;
			cairo_surface_destroy (widgetSurface);	// destroy old surface first
			widgetSurface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width_, height_);
			draw (0, 0, width_, height_);
		}
	}
}

double Widget::getHeight () const {return height_;}

void Widget::setBorder (const BStyles::Border& border)
{
	border_ = border;
	update ();
}

BStyles::Border* Widget::getBorder () {return &border_;}

void Widget::setBackground (const BStyles::Fill& background)
{
	background_ = background;
	update ();
}

BStyles::Fill* Widget::getBackground () {return &background_;}

Window* Widget::getMainWindow () const {return main_;}

Widget* Widget::getParent () const {return parent_;}

bool Widget::hasChildren () const {return (children_.size () > 0 ? true : false);}

std::vector<Widget*> Widget::getChildren () const {return children_;}

void Widget::rename (const std::string& name) {name_ = name;}

std::string Widget::getName () const {return name_;}

void Widget::setCallbackFunction (const BEvents::EventType eventType, const std::function<void (BEvents::Event*)>& callbackFunction)
{
	if (eventType <= BEvents::EventType::NO_EVENT) cbfunction[eventType] = callbackFunction;
}

bool Widget::isVisible()
{
	for (Widget* w = this; w; w = w->parent_)
	{
		if (!w->visible) return false;
	}
	return true;
}

void Widget::setClickable (const bool status) {clickable = status;}

bool Widget::isClickable () const {return clickable;}

void Widget::setDragable (const bool status) {dragable = status;}

bool Widget::isDragable () const {return dragable;}

void Widget::update ()
{
	draw (0, 0, width_, height_);
	if (isVisible ()) postRedisplay ();
}

bool Widget::isPointInWidget (const double x, const double y) const {return ((x >= 0.0) && (x <= width_) && (y >= 0.0) && (y <= height_));}

Widget* Widget::getWidgetAt (const double x, const double y, const bool checkVisibility, const bool checkClickability, const bool checkDragability)
{
	if (main_ && isPointInWidget (x, y) && ((!checkVisibility) || visible))
	{
		Widget* finalw = ((!checkClickability) || clickable ? this : nullptr);
		for (Widget* w : children_)
		{
			double xNew = x - w->x_;
			double yNew = y - w->y_;
			Widget* nextw = w->getWidgetAt (xNew, yNew, checkVisibility, checkClickability, checkDragability);
			if (nextw)
			{
				finalw = nextw;
			}
		}
		return finalw;
	}

	else return nullptr;
}

void Widget::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}

void Widget::applyTheme (BStyles::Theme& theme, const std::string& name)
{
	// Border
	void* borderPtr = theme.getStyle(name, "border");
	if (borderPtr) border_ = *((BStyles::Border*) borderPtr);

	// Background
	void* backgroundPtr = theme.getStyle(name, "background");
	if (backgroundPtr) background_ = *((BStyles::Fill*) backgroundPtr);

	if (borderPtr || backgroundPtr)
	{
		update ();
	}
}

void Widget::onConfigure (BEvents::ExposeEvent* event) {} // Empty, only Windows handle configure events
void Widget::onExpose (BEvents::ExposeEvent* event) {} // Empty, only Windows handle expose events
void Widget::onClose () {} // Empty, only Windows handle close events
void Widget::onButtonPressed (BEvents::PointerEvent* event) {cbfunction[BEvents::EventType::BUTTON_PRESS_EVENT] (event);}
void Widget::onButtonReleased (BEvents::PointerEvent* event) {cbfunction[BEvents::EventType::BUTTON_RELEASE_EVENT] (event);}
void Widget::onPointerMotion (BEvents::PointerEvent* event) {cbfunction[BEvents::EventType::POINTER_MOTION_EVENT] (event);}

void Widget::onPointerMotionWhileButtonPressed (BEvents::PointerEvent* event)
{
	moveTo (x_ + event->getDeltaX (), y_ + event->getDeltaY ());
	cbfunction[BEvents::EventType::POINTER_MOTION_WHILE_BUTTON_PRESSED_EVENT] (event);
}

void Widget::onValueChanged (BEvents::ValueChangedEvent* event) {cbfunction[BEvents::EventType::VALUE_CHANGED_EVENT] (event);}

void Widget::defaultCallback (BEvents::Event* event) {}

std::vector <Widget*> Widget::getChildrenAsQueue (std::vector <Widget*> queue) const
{
	for (Widget* w : children_)
	{
		queue.push_back (w);
		if (!w->children_.empty()) queue = w->getChildrenAsQueue (queue);
	}
	return queue;
}

void Widget::postRedisplay () {postRedisplay (getOriginX (), getOriginY (), width_, height_);}

void Widget::postRedisplay (const double xabs, const double yabs, const double width, const double height)
{
	if (main_)
	{
		BEvents::ExposeEvent* event = new BEvents::ExposeEvent (this, BEvents::EXPOSE_EVENT, xabs, yabs, width, height);
		main_->addEventToQueue (event);
	}
}

void Widget::redisplay (cairo_surface_t* surface, double x, double y, double width, double height)
{
	if (main_ && visible && fitToArea (x, y, width, height))
	{
		// Copy widgets surface onto main surface
		double x0 = getOriginX ();
		double y0 = getOriginY ();

		cairo_t* cr = cairo_create (surface);
		cairo_set_source_surface (cr, widgetSurface, x0, y0);
		cairo_rectangle (cr, x + x0, y + y0, width, height);
		cairo_fill (cr);
		cairo_destroy (cr);

		for (Widget* w : children_)
		{
			double xNew = x - w->x_;
			double yNew = y - w->y_;
			w->redisplay (surface, xNew, yNew, width, height);
		}
	}
}

void Widget::draw (const double x, const double y, const double width, const double height)
{
	cairo_surface_clear (widgetSurface);
	cairo_t* cr = cairo_create (widgetSurface);

	if (cairo_status (cr) == CAIRO_STATUS_SUCCESS)
	{
		// Limit cairo-drawing area
		cairo_rectangle (cr, x, y, width, height);
		cairo_clip (cr);

		double radius = border_.getRadius ();

		// Draw background
		double innerBorders = border_.getMargin () + border_.getLine()->getWidth() + border_.getPadding ();
		double innerRadius = (radius > border_.getPadding () ? radius - border_.getPadding () : 0);
		cairo_surface_t* fillSurface = background_.getCairoSurface ();
		BColors::Color bc = *background_.getColor();

		if ((width_ >= 2 * innerBorders) && (height_ >= 2 * innerBorders))
		{
			// Background_image ?
			if (fillSurface && cairo_surface_status (fillSurface) == CAIRO_STATUS_SUCCESS)
			{
				cairo_set_source_surface (cr, fillSurface, 0, 0);

				// If drawing area < background are, draw only a rectangle for the drawing area (faster)
				if ((x >= innerBorders) && (x + width <= width_ - 2 * innerBorders) &&
					(y >= innerBorders) && (y + height <= height_ - 2 * innerBorders))
				{
					cairo_rectangle (cr, x, y, width, height);
				}
				else
				{
					cairo_rectangle_rounded (cr, innerBorders, innerBorders,
											 width_ - 2 * innerBorders, height_ - 2 * innerBorders, innerRadius);
				}
				cairo_fill (cr);
			}

			// Plain Background color ?
			else if (bc.getAlpha() != 0.0)
			{
				cairo_set_source_rgba (cr, bc.getRed(), bc.getGreen(), bc.getBlue(), bc.getAlpha());

				// If drawing area < background are, draw only a rectangle for the drawing area (faster)
				if ((x >= innerBorders) && (x + width <= width_ - 2 * innerBorders) &&
					(y >= innerBorders) && (y + height <= height_ - 2 * innerBorders))
				{
					cairo_rectangle (cr, x, y, width, height);
				}
				else
				{
					cairo_rectangle_rounded (cr, innerBorders, innerBorders,
											 width_ - 2 * innerBorders, height_ - 2 * innerBorders, innerRadius);
				}
				cairo_fill (cr);
			}

		}

		// Draw border frame
		double outerBorders = border_.getMargin ();
		BColors::Color lc = *border_.getLine()->getColor();

		if ((lc.getAlpha() != 0.0) &&
			(border_.getLine()->getWidth() != 0.0) &&
			(width_ >= 2 * outerBorders) &&
			(height_ >= 2 * outerBorders))
		{
			double lw = border_.getLine()->getWidth();
			cairo_rectangle_rounded (cr, outerBorders + lw / 2, outerBorders + lw / 2,
									 width_ - 2 * outerBorders - lw, height_ - 2 * outerBorders - lw, radius);

			cairo_set_source_rgba (cr, lc.getRed(), lc.getGreen(), lc.getBlue(), lc.getAlpha());
			cairo_set_line_width (cr, lw);
			cairo_stroke (cr);
		}
	}

	cairo_destroy (cr);
}

bool Widget::fitToArea (double& x, double& y, double& width, double& height)
{
	bool isInArea = true;
	if (x < 0.0)
	{
		if (x + width < 0.0)
		{
			x = 0.0;
			width = 0.0;
			isInArea = false;
		}
		else
		{
			width = x + width;
			x = 0.0;

		}
	}
	if (x + width > width_)
	{
		if (x > width_)
		{
			x = width_;
			width = 0.0;
			isInArea = false;
		}
		else
		{
			width = width_ - x;
		}
	}
	if (y < 0.0)
	{
		if (y + height < 0.0)
		{
			y = 0.0;
			height = 0.0;
			isInArea = false;
		}
		else
		{
			height = y + height;
			y = 0.0;

		}
	}
	if (y + height > height_)
	{
		if (y > height_)
		{
			y = height_;
			height = 0.0;
			isInArea = false;
		}
		else
		{
			height = height_ - y;
		}
	}

	return isInArea;
}

/*****************************************************************************/

Window::Window () : Window (200.0, 200.0, "Main Window", 0.0) {}

Window::Window (const double width, const double height, const std::string& title, PuglNativeWindow nativeWindow, bool resizable) :
		Widget (0.0, 0.0, width, height, title), title_ (title), view_ (NULL), nativeWindow_ (nativeWindow), quit_ (false),
		input ({nullptr, nullptr, nullptr, nullptr})
{
	main_ = this;
	view_ = puglInit(NULL, NULL);

	if (nativeWindow_ != 0)
	{
		puglInitWindowParent(view_, nativeWindow_);
	}

	puglInitWindowSize (view_, width_, height_);
	puglInitResizable (view_, resizable);
	puglInitContextType (view_, PUGL_CAIRO);
	puglIgnoreKeyRepeat (view_, true);
	puglCreateWindow (view_, title.c_str ());
	puglShowWindow (view_);
	puglSetHandle (view_, this);

	puglSetEventFunc (view_, Window::translatePuglEvent);
}

Window::~Window ()
{
	purgeEventQueue ();
	puglDestroy(view_);
}

PuglView* Window::getPuglView () {return view_;}

cairo_t* Window::getPuglContext ()
{
	if (view_) return (cairo_t*) puglGetContext (view_);
	else return NULL;
}

void Window::run ()
{
	while (!quit_)
	{
		puglWaitForEvent (view_);
		handleEvents ();
	}
}

void Window::onConfigure (BEvents::ExposeEvent* event)
{
	if (width_ != event->getWidth ()) setWidth (event->getWidth ());
	if (height_ != event->getHeight ()) setHeight (event->getHeight ());
}

void Window::onClose ()
{
	quit_ = true;
}

void Window::onExpose (BEvents::ExposeEvent* event)
{
	if (event)
	{
		Widget* widget = (Widget*) event->getWidget ();

		// Create a temporal storage surface and store all children surfaces on this
		cairo_surface_t* storageSurface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width_, height_);
		redisplay (storageSurface, event->getX (), event->getY (),
				   event->getWidth (), event->getHeight ());

		// Copy storage surface onto pugl provided surface
		cairo_t* cr = main_->getPuglContext ();
		cairo_save (cr);
		cairo_set_source_surface (cr, storageSurface, 0, 0);
		cairo_paint (cr);
		cairo_restore (cr);

		cairo_surface_destroy (storageSurface);
	}
}

void Window::addEventToQueue (BEvents::Event* event)
{
	eventQueue.push_back (event);
}

void Window::setInput (const BEvents::InputDevice device, Widget* widget)
{
	if ((device > BEvents::NO_BUTTON) && (device < BEvents::NR_OF_BUTTONS)) input[device] = widget;
}

Widget* Window::getInput (BEvents::InputDevice device) const
{
	if ((device > BEvents::NO_BUTTON) && (device < BEvents::NR_OF_BUTTONS)) return input[device];
	else return nullptr;
}

void Window::handleEvents ()
{
	puglProcessEvents (view_);

	while (eventQueue.size () > 0)
	{
		BEvents::Event* event = eventQueue.front ();
		if (event)
		{
			Widget* widget = (Widget*) event->getWidget ();
			if (widget)
			{
				BEvents::EventType eventType = event->getEventType ();

				switch (eventType)
				{
				case BEvents::CONFIGURE_EVENT:
					onConfigure ((BEvents::ExposeEvent*) event);
					break;

				case BEvents::EXPOSE_EVENT:
					onExpose ((BEvents::ExposeEvent*) event);
					break;

				case BEvents::CLOSE_EVENT:
					onClose ();
					break;

				case BEvents::BUTTON_PRESS_EVENT:
					{
						BEvents::PointerEvent* be = (BEvents::PointerEvent*) event;
						setInput (be->getButton (), widget);
						widget->onButtonPressed (be);
					}
					break;

				case BEvents::BUTTON_RELEASE_EVENT:
					{
						BEvents::PointerEvent* be = (BEvents::PointerEvent*) event;
						setInput (be->getButton (), nullptr);
						widget->onButtonReleased (be);
					}
					break;

				case BEvents::POINTER_MOTION_EVENT:
					widget->onPointerMotion((BEvents::PointerEvent*) event);
					break;

				case BEvents::POINTER_MOTION_WHILE_BUTTON_PRESSED_EVENT:
					widget->onPointerMotionWhileButtonPressed((BEvents::PointerEvent*) event);
					break;

				case BEvents::VALUE_CHANGED_EVENT:
					widget->onValueChanged((BEvents::ValueChangedEvent*) event);
					break;

				default:
					break;
				}

			}
			delete event;
		}
		eventQueue.erase (eventQueue.begin ());
	}
}

void Window::translatePuglEvent (PuglView* view, const PuglEvent* event)
{
	Window* w = (Window*) puglGetHandle (view);
	switch (event->type) {
	case PUGL_BUTTON_PRESS:
		{
			Widget* widget = w->getWidgetAt (event->button.x, event->button.y, true, true, false);
			if (widget)
			{
				w->addEventToQueue (new BEvents::PointerEvent (widget,
															  BEvents::BUTTON_PRESS_EVENT,
															  event->button.x - widget->getOriginX (),
															  event->button.y - widget->getOriginY (),
															  0, 0,
															  (BEvents::InputDevice) event->button.button));
			}

			w->pointerX = event->button.x;
			w->pointerY = event->button.y;
		}
		break;

	case PUGL_BUTTON_RELEASE:
		{
			Widget* widget = w->getInput ((BEvents::InputDevice) event->button.button);
			if (widget)
			{
				w->addEventToQueue (new BEvents::PointerEvent (widget,
															  BEvents::BUTTON_RELEASE_EVENT,
															  event->button.x - widget->getOriginX (),
															  event->button.y - widget->getOriginY (),
															  0, 0,
															  (BEvents::InputDevice) event->button.button));
			}

			w->pointerX = event->button.x;
			w->pointerY = event->button.y;
		}
		break;

	case PUGL_MOTION_NOTIFY:
		{
			BEvents::InputDevice device = BEvents::NO_BUTTON;

			// Scan for pressed buttons associated with a widget
			for (int i = BEvents::NO_BUTTON + 1; i < BEvents::NR_OF_BUTTONS; ++i)
			{
				if (w->getInput ((BEvents::InputDevice) i))
				{
					device = (BEvents::InputDevice) i;
					Widget* widget = w->getInput (device);
					if (widget->isDragable ())
					{
						w->addEventToQueue (new BEvents::PointerEvent (widget,
																	   BEvents::POINTER_MOTION_WHILE_BUTTON_PRESSED_EVENT,
																	   event->motion.x - widget->getOriginX (),
																	   event->motion.y - widget->getOriginY (),
																	   event->motion.x - w->pointerX,
																	   event->motion.y - w->pointerY,
																	   device));
					}
				}
			}

			// No button associated with a widget? Only POINTER_MOTION_EVENT
			if (device == BEvents::NO_BUTTON)
			{
				Widget* widget = w->getWidgetAt (event->motion.x, event->motion.y, true, false, false);
				if (widget)
				{
					w->addEventToQueue (new BEvents::PointerEvent (widget,
																   BEvents::POINTER_MOTION_EVENT,
																   event->motion.x - widget->getOriginX (),
																   event->motion.y - widget->getOriginY (),
																   event->motion.x - w->pointerX,
																   event->motion.y - w->pointerY,
																   device));
				}

			}

			w->pointerX = event->motion.x;
			w->pointerY = event->motion.y;
		}
		break;

	case PUGL_CONFIGURE:
		w->addEventToQueue (new BEvents::ExposeEvent (w,
													  BEvents::CONFIGURE_EVENT,
													  event->configure.x,
													  event->configure.y,
													  event->configure.width,
													  event->configure.height));
		break;

	case PUGL_EXPOSE:
		w->postRedisplay ();
		break;

	case PUGL_CLOSE:
		w->addEventToQueue (new BEvents::Event (w, BEvents::CLOSE_EVENT));
		break;

	default: break;
	}

}

void Window::purgeEventQueue ()
{
	while (eventQueue.size () > 0)
	{
		BEvents::Event* event = eventQueue.front ();
		if (event) delete event;
		eventQueue.erase (eventQueue.begin ());
	}
}

}
