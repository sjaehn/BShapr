/* B.Shapr
 * Beat / envelope shaper LV2 plugin
 *
 * Copyright (C) 2019 by Sven Jähnichen
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

#ifndef BSHAPRGUI_HPP_
#define BSHAPRGUI_HPP_

#include <string>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <exception>
#include <utility>
#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#include <lv2/lv2plug.in/ns/extensions/ui/ui.h>
#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/atom/forge.h>
#include <lv2/lv2plug.in/ns/ext/time/time.h>
#include "BWidgets/Widget.hpp"
#include "BWidgets/Window.hpp"
#include "BWidgets/RangeWidget.hpp"
#include "BWidgets/ImageIcon.hpp"
#include "BWidgets/Label.hpp"
#include "BWidgets/Dial.hpp"
#include "BWidgets/DialValue.hpp"
#include "BWidgets/PopupListBox.hpp"
#include "BWidgets/HSwitch.hpp"
#include "BWidgets/VSwitch.hpp"
#include "BWidgets/HPianoRoll.hpp"
#include "BWidgets/MessageBox.hpp"
#include "ShapeWidget.hpp"
#include "ValueSelect.hpp"
#include "HorizonWidget.hpp"
#include "MonitorWidget.hpp"
#include "SymbolWidget.hpp"
#include "EditWidget.hpp"
#include "LightButton.hpp"
#include "Globals.hpp"
#include "Urids.hpp"
#include "BShaprNotifications.hpp"

#include "screen.h"
#include "SelectWidget.hpp"

#define BG_FILE "inc/surface.png"
#define RESIZE(widget, x, y, w, h, sz) {(widget).moveTo ((x) * (sz), (y) * (sz)); (widget).resize ((w) * (sz), (h) * (sz));}

const std::string messageStrings[MAXMESSAGES] =
{
	"",
	"Msg: Jack transport off or halted.",
	"Msg: Orphan active shape. Select an input for this shape!",
	"Msg: Orphan active shape. Select an output for this or a connected shape!"
};

const std::string editWidgetLabels[7] = {"Cut", "Copy", "Paste", "Delete", "Reset", "Undo", "Redo"};

class BShaprGUI : public BWidgets::Window
{
public:
	BShaprGUI (const char *bundlePath, const LV2_Feature *const *features, PuglNativeView parentWindow);
	~BShaprGUI ();
	void portEvent (uint32_t port, uint32_t bufferSize, uint32_t format, const void *buffer);
	void sendGuiOn ();
	void sendGuiOff ();
	void sendShape (size_t shapeNr);
	virtual void onConfigureRequest (BEvents::ExposeEvent* event) override;
	virtual void onCloseRequest (BEvents::WidgetEvent* event) override;
	virtual void onKeyPressed (BEvents::KeyEvent* event) override;
	virtual void onKeyReleased (BEvents::KeyEvent* event) override;
	void applyChildThemes ();

	LV2UI_Controller controller;
	LV2UI_Write_Function write_function;

	static void valueChangedCallback (BEvents::Event* event);
	static void shapeChangedCallback (BEvents::Event* event);
	static void toolChangedCallback (BEvents::Event* event);
	static void editClickedCallback (BEvents::Event* event);
	static void editReleasedCallback (BEvents::Event* event);
	static void gridChangedCallback (BEvents::Event* event);
	static void tabClickedCallback (BEvents::Event* event);
	static void wheelScrolledCallback (BEvents::Event* event);
	static void pianoCallback (BEvents::Event* event);


private:
	void setController (const int controllerNr, const float value);
	void deleteShape (const int shapeNr);
	void insertShape (const int shapeNr);
	void swapShapes (const int source, const int dest);
	void switchShape (const int shapeNr);
	void resizeGUI (const double sz);
	void updateTabs ();
	void calculateXSteps ();
	void initMonitors ();
	std::pair<int, int> translateNotification (BShaprNotifications* notifications, uint32_t notificationsCount);
	void updateMonitors (int start, int end);
	void updateHorizon ();

	// Controllers
	std::array<BWidgets::ValueWidget*, NR_CONTROLLERS> controllerWidgets;
	std::array<float, NR_CONTROLLERS> controllers;

	float bpm;
	float beatsPerBar;
	int beatUnit;

	// Widgets
	struct ShapeGui
	{
		BWidgets::Widget shapeContainer;
		BWidgets::Widget tabContainer;
		std::array<SymbolWidget, 4> tabSymbol;
		BWidgets::Widget* tabMsgBoxBg;
		BWidgets::MessageBox* tabMsgBox;
		BWidgets::ImageIcon tabIcon;
		BWidgets::Label smoothingLabel;
		BWidgets::DialValue smoothingDial;
		BWidgets::PopupListBox targetListBox;
		BWidgets::Label drywetLabel;
		BWidgets::DialValue drywetDial;
		ShapeWidget shapeWidget;
		std::list<BWidgets::ImageIcon> methodIcons;
		std::array<BWidgets::ValueWidget*, MAXOPTIONS> optionWidgets;
		std::array<BWidgets::Label, MAXOPTIONS> optionLabels;
		SelectWidget toolSelect;
		std::array<EditWidget, 7> editWidgets;
		SelectWidget gridSelect;
	} ;

	BWidgets::ImageIcon mContainer;
	BWidgets::Label messageLabel;
	LightButton bypassButton;
        BWidgets::DialValue drywetDial;
	BWidgets::HSwitch midiTriggerSwitch;
	BWidgets::HPianoRoll midiPiano;
	BWidgets::VSwitch midiThruSwitch;
	ValueSelect baseValueSelect;
	BWidgets::PopupListBox baseListBox;
	BWidgets::Widget monitorContainer;
	HorizonWidget monitorHorizon1;
	HorizonWidget monitorHorizon2;
	MonitorWidget input1Monitor;
	MonitorWidget output1Monitor;
	MonitorWidget input2Monitor;
	MonitorWidget output2Monitor;
	std::array<ShapeGui, MAXSHAPES> shapeGui;

	float shapeBuffer[MAXNODES * 7];

	double horizonPos;
	double monitorScale;
	double minorXSteps;
	double majorXSteps;

	std::vector<Node> clipboard;

	std::string pluginPath;
	double sz;
	cairo_surface_t* bgImageSurface;

	LV2_Atom_Forge forge;
	BShaprURIDs urids;
	LV2_URID_Map* map;

	// Definition of styles
	BColors::ColorSet fgColors = {{{0.75, 0.0, 0.75, 1.0}, {1.0, 0.0, 1.0, 1.0}, {0.25, 0.0, 0.25, 1.0}, {0.0, 0.0, 0.0, 0.0}}};
	BColors::ColorSet rdColors = {{{0.75, 0.0, 0.0, 1.0}, {1.0, 0.25, 0.25, 1.0}, {0.2, 0.0, 0.0, 1.0}, {0.0, 0.0, 0.0, 0.0}}};
	BColors::ColorSet txColors = {{{1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0}, {0.2, 0.2, 0.2, 1.0}, {0.0, 0.0, 0.0, 0.0}}};
	BColors::ColorSet bgColors = {{{0.15, 0.15, 0.15, 1.0}, {0.3, 0.3, 0.3, 1.0}, {0.05, 0.05, 0.05, 1.0}, {0.0, 0.0, 0.0, 0.0}}};
	BColors::ColorSet ltbgColors = {{{0.3, 0.3, 0.3, 1.0}, {0.5, 0.5, 0.5, 1.0}, {0.1, 0.1, 0.1, 1.0}, {0.0, 0.0, 0.0, 0.0}}};
	BColors::ColorSet blkColors = {{{0.0, 0.0, 0.0, 1.0}, {0.0, 0.0, 0.0, 1.0}, {0.0, 0.0, 0.0, 1.0}, {0.0, 0.0, 0.0, 0.0}}};
	BColors::ColorSet menuBgColors = {{{0.05, 0.05, 0.05, 1.0}, {0.05, 0.05, 0.05, 1.0}, {0.05, 0.05, 0.05, 1.0}, {0.0, 0.0, 0.0, 1.0}}};
	BColors::ColorSet buttonBgColors = {{{0.1, 0.1, 0.1, 1.0}, {0.2, 0.2, 0.2, 1.0}, {0.05, 0.05, 0.05, 1.0}, {0.0, 0.0, 0.0, 1.0}}};
	BColors::ColorSet clickColors = {{{0.2, 0.2, 0.2, 1.0}, {1, 1, 1, 1.0}, {0, 0, 0, 1.0}, {0, 0, 0, 0.0}}};
	BColors::ColorSet ink1 = {{{0.75, 0.0, 1.0, 1.0}, {0.9, 0.5, 1.0, 1.0}, {0.1, 0.0, 0.25, 1.0}, {0.0, 0.0, 0.0, 0.0}}};
	BColors::ColorSet ink2 = {{{1.0, 0.0, 0.75, 1.0}, {1.0, 0.5, 0.9, 1.0}, {0.25, 0.0, 0.1, 1.0}, {0.0, 0.0, 0.0, 0.0}}};
	BStyles::Border labelBorder = BStyles::Border (BStyles::noLine, 0.0, 4.0);
	BStyles::Border menuBorder = BStyles::Border (BStyles::Line (BColors::Color (0.2, 0.2, 0.2, 1.0), 1.0));
	BStyles::Border screenBorder = BStyles::Border (BStyles::Line (BColors::Color (0.0, 0.0, 0.0, 0.75), 4.0));
	BStyles::Border itemBorder = BStyles::Border (BStyles::noLine, 0.0, 2.0);
	BStyles::Fill tabBg = BStyles::Fill (BColors::Color (0.5, 0, 0.5, 0.5));
	BStyles::Fill activeTabBg = BStyles::Fill (BColors::Color (0.5, 0, 0.5, 0.875));
	BStyles::Fill screenBg = BStyles::Fill (BColors::Color (0.0, 0.0, 0.0, 0.75));
	BStyles::Font defaultFont = BStyles::Font ("Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, 12.0,
						   BStyles::TEXT_ALIGN_CENTER, BStyles::TEXT_VALIGN_MIDDLE);
 	BStyles::Font smFont = BStyles::Font ("Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, 10.0,
 					      BStyles::TEXT_ALIGN_CENTER, BStyles::TEXT_VALIGN_MIDDLE);
	BStyles::Font ssmFont = BStyles::Font ("Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, 8.0,
					      BStyles::TEXT_ALIGN_CENTER, BStyles::TEXT_VALIGN_MIDDLE);
	BStyles::Font lfLabelFont = BStyles::Font ("Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, 12.0,
						   BStyles::TEXT_ALIGN_LEFT, BStyles::TEXT_VALIGN_MIDDLE);
	BStyles::StyleSet defaultStyles = {"default", {{"background", STYLEPTR (&BStyles::noFill)},
						       {"border", STYLEPTR (&BStyles::noBorder)}}};
	BStyles::StyleSet labelStyles = {"labels", {{"background", STYLEPTR (&BStyles::noFill)},
						   {"border", STYLEPTR (&labelBorder)},
						   {"textcolors", STYLEPTR (&txColors)},
						   {"font", STYLEPTR (&lfLabelFont)}}};

	BStyles::Theme theme = BStyles::Theme ({
		defaultStyles,
		{"B.Shapr", 		{{"background", STYLEPTR (&BStyles::blackFill)},
					 {"border", STYLEPTR (&BStyles::noBorder)}}},
		{"widget", 		{{"uses", STYLEPTR (&defaultStyles)}}},
		{"widget/focus", 	{{"background", STYLEPTR (&screenBg)},
				 	 {"border", STYLEPTR (&screenBorder)},
					 {"textcolors", STYLEPTR (&txColors)},
					 {"font", STYLEPTR (&smFont)}}},
		{"frame", 		{{"uses", STYLEPTR (&defaultStyles)},
					 {"border", STYLEPTR (&BStyles::whiteBorder1pt)}}},
		{"screen", 		{{"background", STYLEPTR (&screenBg)}}},
		{"icon", 		{{"uses", STYLEPTR (&defaultStyles)},
					 {"border", STYLEPTR (&labelBorder)}}},
		{"tab", 		{{"background", STYLEPTR (&tabBg)},
					 {"border", STYLEPTR (&BStyles::noBorder)}}},
		{"activetab", 		{{"background", STYLEPTR (&activeTabBg)},
					 {"border", STYLEPTR (&BStyles::noBorder)}}},
		{"tool", 		{{"uses", STYLEPTR (&defaultStyles)},
					 {"bgcolors", STYLEPTR (&BColors::whites)}}},
		{"tool/focus", 		{{"background", STYLEPTR (&screenBg)},
				 	 {"border", STYLEPTR (&screenBorder)},
					 {"textcolors", STYLEPTR (&txColors)},
					 {"font", STYLEPTR (&smFont)}}},
		{"monitor.in", 		{{"background", STYLEPTR (&BStyles::blackFill)},
					 {"border", STYLEPTR (&BStyles::noBorder)},
				 	 {"fgcolors", STYLEPTR (&ink1)}}},
		{"monitor.out", 	{{"background", STYLEPTR (&BStyles::blackFill)},
					 {"border", STYLEPTR (&BStyles::noBorder)},
				 	 {"fgcolors", STYLEPTR (&ink2)}}},
		{"shape",	 	{{"background", STYLEPTR (&BStyles::noFill)},
			 		 {"border", STYLEPTR (&BStyles::noBorder)},
			 	 	 {"fgcolors", STYLEPTR (&BColors::whites)},
					 {"symbolcolors", STYLEPTR (&fgColors)},
					 {"font", STYLEPTR (&lfLabelFont)},
					 {"bgcolors", STYLEPTR (&ltbgColors)}}},
		{"shape/focus", 	{{"uses", STYLEPTR (&labelStyles)},
					 {"background", STYLEPTR (&screenBg)},
				 	 {"border", STYLEPTR (&screenBorder)}}},
		{"redbutton", 		{{"uses", STYLEPTR (&defaultStyles)},
				 	 {"fgcolors", STYLEPTR (&rdColors)},
				 	 {"bgcolors", STYLEPTR (&bgColors)}}},
		{"dial", 		{{"uses", STYLEPTR (&defaultStyles)},
					 {"fgcolors", STYLEPTR (&fgColors)},
					 {"bgcolors", STYLEPTR (&bgColors)},
					 {"textcolors", STYLEPTR (&fgColors)},
					 {"font", STYLEPTR (&defaultFont)}}},
		{"dial/focus", 		{{"background", STYLEPTR (&screenBg)},
				 	 {"border", STYLEPTR (&screenBorder)},
 					 {"textcolors", STYLEPTR (&txColors)},
 					 {"font", STYLEPTR (&smFont)}}},
		{"symbol", 		{{"uses", STYLEPTR (&defaultStyles)},
					 {"fgcolors", STYLEPTR (&blkColors)}}},
		{"label", 		{{"uses", STYLEPTR (&labelStyles)}}},
		{"smlabel",	 	{{"uses", STYLEPTR (&defaultStyles)},
					 {"textcolors", STYLEPTR (&txColors)},
					 {"font", STYLEPTR (&smFont)}}},
		{"ssmlabel",	 	{{"uses", STYLEPTR (&defaultStyles)},
 					 {"textcolors", STYLEPTR (&txColors)},
 					 {"font", STYLEPTR (&ssmFont)}}},
		{"select",	 	{{"uses", STYLEPTR (&defaultStyles)}}},
		{"select/label",	{{"uses", STYLEPTR (&defaultStyles)},
					 {"textcolors", STYLEPTR (&BColors::whites)},
					 {"font", STYLEPTR (&defaultFont)}}},
		{"select/click",	{{"uses", STYLEPTR (&defaultStyles)},
					 {"bgcolors", STYLEPTR (&clickColors)}}},
		{"msgbox",	 	{{"border", STYLEPTR (&menuBorder)},
					 {"background", STYLEPTR (&BStyles::blackFill)}}},
 		{"msgbox/title",	{{"uses", STYLEPTR (&defaultStyles)},
 					 {"textcolors", STYLEPTR (&BColors::whites)},
 					 {"font", STYLEPTR (&defaultFont)}}},
 		{"msgbox/text",		{{"uses", STYLEPTR (&defaultStyles)},
 					 {"textcolors", STYLEPTR (&BColors::whites)},
 					 {"font", STYLEPTR (&lfLabelFont)}}},
		{"msgbox/button", 	{{"border", STYLEPTR (&menuBorder)},
			 		 {"background", STYLEPTR (&BStyles::blackFill)},
					 {"textcolors", STYLEPTR (&BColors::whites)},
					 {"font", STYLEPTR (&defaultFont)},
					 {"bgcolors", STYLEPTR (&buttonBgColors)}}},
		{"menu",	 	{{"border", STYLEPTR (&menuBorder)},
					 {"background", STYLEPTR (&BStyles::blackFill)}}},
		{"menu/item",		{{"uses", STYLEPTR (&defaultStyles)},
					 {"border", STYLEPTR (&labelBorder)},
					 {"textcolors", STYLEPTR (&BColors::whites)},
					 {"font", STYLEPTR (&lfLabelFont)}}},
		{"menu/button",	 	{{"border", STYLEPTR (&menuBorder)},
			 		 {"background", STYLEPTR (&BStyles::noFill)},
					 {"bgcolors", STYLEPTR (&buttonBgColors)}}},
		{"menu/listbox",	{{"border", STYLEPTR (&menuBorder)},
					 {"background", STYLEPTR (&BStyles::blackFill)}}},
		{"menu/listbox/item",	{{"uses", STYLEPTR (&defaultStyles)},
					 {"border", STYLEPTR (&labelBorder)},
					 {"textcolors", STYLEPTR (&BColors::whites)},
					 {"font", STYLEPTR (&lfLabelFont)}}},
		{"menu/listbox/button",	{{"border", STYLEPTR (&menuBorder)},
					 {"background", STYLEPTR (&BStyles::blackFill)},
			 	 	 {"bgcolors", STYLEPTR (&buttonBgColors)}}},
		{"menu2",	 	{{"border", STYLEPTR (&menuBorder)},
					 {"background", STYLEPTR (&BStyles::blackFill)}}},
		{"menu2/item",		{{"uses", STYLEPTR (&defaultStyles)},
					 {"border", STYLEPTR (&itemBorder)},
					 {"textcolors", STYLEPTR (&BColors::whites)},
					 {"font", STYLEPTR (&lfLabelFont)}}},
		{"menu2/button",	{{"border", STYLEPTR (&menuBorder)},
			 		 {"background", STYLEPTR (&BStyles::blackFill)},
					 {"bgcolors", STYLEPTR (&buttonBgColors)}}},
		{"menu2/listbox",	{{"border", STYLEPTR (&menuBorder)},
					 {"background", STYLEPTR (&BStyles::blackFill)}}},
		{"menu2/listbox/item",	{{"uses", STYLEPTR (&defaultStyles)},
					 {"border", STYLEPTR (&itemBorder)},
					 {"textcolors", STYLEPTR (&BColors::whites)},
					 {"font", STYLEPTR (&lfLabelFont)}}},
		{"menu2/listbox/button",{{"border", STYLEPTR (&menuBorder)},
					 {"background", STYLEPTR (&BStyles::blackFill)},
			 	 	 {"bgcolors", STYLEPTR (&buttonBgColors)}}}
	});
};



#endif /* BSHAPRGUI_HPP_ */
