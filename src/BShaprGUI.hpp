/* B.Shapr
 * Step Sequencer Effect Plugin
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
#include <tuple>
#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#include <lv2/lv2plug.in/ns/extensions/ui/ui.h>
#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/atom/forge.h>
#include <lv2/lv2plug.in/ns/ext/time/time.h>
#include "BWidgets/BValues.hpp"
#include "BWidgets/Widget.hpp"
#include "BWidgets/Window.hpp"
#include "BWidgets/FocusWidget.hpp"
#include "BWidgets/RangeWidget.hpp"
#include "BWidgets/ImageIcon.hpp"
#include "BWidgets/Label.hpp"
#include "BWidgets/Dial.hpp"
#include "BWidgets/DisplayDial.hpp"
#include "BWidgets/PopupListBox.hpp"
#include "ShapeWidget.hpp"
#include "ValueSelect.hpp"
#include "HorizonWidget.hpp"
#include "definitions.h"
#include "ports.h"
#include "urids.h"

#include "screen.h"
#include "SelectWidget.hpp"

#define CAIRO_BG_COLOR 0.0, 0.0, 0.0, 1.0
#define CAIRO_BG_COLOR2 0.2, 0.2, 0.2, 1.0
#define CAIRO_TRANSPARENT 0.0, 0.0, 0.0, 0.0
#define CAIRO_FG_COLOR 1.0, 1.0, 1.0, 1.0
#define CAIRO_INK1 0.75, 0.0, 1.0
#define CAIRO_INK2 1.0, 0.0, 0.75

#define BG_FILE "surface.png"

#define RESIZE(widget, x, y, w, h, sz) widget.moveTo ((x) * (sz), (y) * (sz)); widget.resize ((w) * (sz), (h) * (sz));

struct ScaleParameters
{
	double anchorYPos;
	double anchorValue;
	double ratio;
	std::string prefix;
	std::string unit;
};

const ScaleParameters scaleParameters[MAXEFFECTS] = {{0.05, 0, 1.1, "", ""},
																										 {0.5, 0, 2.2, "", ""},
																										 {0.05, 0, 2.2, "", ""},
																										 {0, 0, 5050, "", "Hz"},
																										 {0, 0, 810, "", "Hz"},
																										 {0.75, 0, 90, "", "dB"},
																										 {0.1, 1.3, 3.5, "10^", "Hz"},
																										 {0.1, 1.3, 3.5, "10^", "Hz"},
																										 {0.5, 0, 25, "", "semitones"},
																										 {0.05, 0, 800, "", "ms"},
																										 {0.05, 0, 800, "", "ms"}
																									  };

const std::string messageStrings[MAXMESSAGES] = {"",
																								 "Msg: Jack transport off or halted. Plugin halted.",
																							 	 "Msg: Orphan active shape. Select an input for this shape!",
																							 	 "Msg: Orphan active shape. Select an output for this or a connected shape!"};

class BShaprGUI : public BWidgets::Window
{
public:
	BShaprGUI (const char *bundlePath, const LV2_Feature *const *features, PuglNativeWindow parentWindow);
	~BShaprGUI ();
	void portEvent (uint32_t port, uint32_t bufferSize, uint32_t format, const void *buffer);
	void sendGuiOn ();
	void sendGuiOff ();
	void sendShape (size_t shapeNr);
	virtual void onConfigure (BEvents::ExposeEvent* event) override;
	void applyChildThemes ();

	LV2UI_Controller controller;
	LV2UI_Write_Function write_function;

	static void valueChangedCallback (BEvents::Event* event);
	static void shapeChangedCallback (BEvents::Event* event);
	static void toolChangedCallback (BEvents::Event* event);
	static void tabClickedCallback (BEvents::Event* event);
	static void wheelScrolledCallback (BEvents::Event* event);


private:
	void resizeGUI ();
	void calculateXSteps ();
	void init_mainMonitor ();
	std::tuple<int, int> add_monitor_data (BShaprNotifications* notifications, uint32_t notificationsCount);
	void redrawMainMonitor (int start, int end);
	void drawData (cairo_t* cr, double x0, double y0, double width, double height, std::array<float, MONITORBUFFERSIZE>& data, int start, int end);

	// Controllers
	std::array<BWidgets::ValueWidget*, NR_CONTROLLERS> controllerWidgets;
	std::array<float, NR_CONTROLLERS> controllers;

	float beatsPerBar;
	int beatUnit;

	// Widgets
	BWidgets::ImageIcon mContainer;
	BWidgets::Label messageLabel;
	ValueSelect baseValueSelect;
	BWidgets::PopupListBox baseListBox;
	BWidgets::DrawingSurface monitorSurface;
	HorizonWidget monitorHorizon1;
	HorizonWidget monitorHorizon2;

	typedef struct
	{
		BWidgets::Widget shapeContainer;
		BWidgets::ImageIcon tabIcon;
		std::vector<BWidgets::ImageIcon> inputShapeLabelIcons;
		SelectWidget inputSelect;
		BWidgets::DisplayDial inputAmpDial;
		BWidgets::DisplayDial outputAmpDial;
		BWidgets::PopupListBox targetListBox;
		BWidgets::Label drywetLabel;
		BWidgets::Dial drywetDial;
		ShapeWidget shapeWidget;
		SelectWidget toolSelect;
		SelectWidget outputSelect;
		BWidgets::ImageIcon shapeLabelIcon;
	} ShapeGui;

	std::array<ShapeGui, MAXSHAPES> shapeGui;

	float shapeBuffer[MAXNODES * 7];

	std::array<float, MONITORBUFFERSIZE> input1Data;
	std::array<float, MONITORBUFFERSIZE> output1Data;
	std::array<float, MONITORBUFFERSIZE> input2Data;
	std::array<float, MONITORBUFFERSIZE> output2Data;

	double horizonPos;
	double monitorScale;
	double minorXSteps;
	double majorXSteps;

	std::string pluginPath;
	double sz;
	cairo_surface_t* bgImageSurface;

	LV2_Atom_Forge forge;
	BShaprURIDs urids;
	LV2_URID_Map* map;



	// Definition of styles
	BColors::ColorSet fgColors = {{{1.0, 0.0, 1.0, 1.0}, {1.0, 0.5, 1.0, 1.0}, {0.25, 0.0, 0.25, 1.0}, {0.0, 0.0, 0.0, 0.0}}};
	BColors::ColorSet txColors = {{{1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0}, {0.2, 0.2, 0.2, 1.0}, {0.0, 0.0, 0.0, 0.0}}};
	BColors::ColorSet bgColors = {{{0.15, 0.15, 0.15, 1.0}, {0.3, 0.3, 0.3, 1.0}, {0.05, 0.05, 0.05, 1.0}, {0.0, 0.0, 0.0, 0.0}}};
	BColors::ColorSet lbColors = {{{0, 0, 0, 1.0}, {0, 0, 0, 1.0}, {0, 0, 0, 1.0}, {0, 0, 0, 0.0}}};
	BColors::ColorSet clickColors = {{{0, 0, 0, 1.0}, {1, 1, 1, 1.0}, {0, 0, 0, 1.0}, {0, 0, 0, 0.0}}};
	BColors::Color ink = {0.0, 0.75, 0.2, 1.0};
	BStyles::Fill widgetBg = BStyles::noFill;
	BStyles::Fill tabBg = BStyles::Fill (BColors::Color (0.5, 0, 0.5, 0.375));
	BStyles::Fill activeTabBg = BStyles::Fill (BColors::Color (0.5, 0, 0.5, 0.75));
	BStyles::Font defaultFont = BStyles::Font ("Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, 12.0,
										       									 BStyles::TEXT_ALIGN_CENTER, BStyles::TEXT_VALIGN_MIDDLE);
 	BStyles::Font smFont = BStyles::Font ("Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, 10.0,
 										       									 BStyles::TEXT_ALIGN_CENTER, BStyles::TEXT_VALIGN_MIDDLE);
	BStyles::Font lfLabelFont = BStyles::Font ("Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, 12.0,
											   										 BStyles::TEXT_ALIGN_LEFT, BStyles::TEXT_VALIGN_MIDDLE);
	BStyles::StyleSet defaultStyles = {"default", {{"background", STYLEPTR (&BStyles::noFill)},
												   {"border", STYLEPTR (&BStyles::noBorder)}}};
	BStyles::StyleSet labelStyles = {"labels", 	  {{"background", STYLEPTR (&BStyles::noFill)},
												   {"border", STYLEPTR (&BStyles::noBorder)},
												   {"textcolors", STYLEPTR (&txColors)},
												   {"font", STYLEPTR (&lfLabelFont)}}};

	BStyles::Theme theme = BStyles::Theme ({
		defaultStyles,
		{"B.Shapr", 		{{"background", STYLEPTR (&BStyles::blackFill)},
							 			 {"border", STYLEPTR (&BStyles::noBorder)}}},
		{"widget", 			{{"uses", STYLEPTR (&defaultStyles)}}},
		{"tab", 				{{"background", STYLEPTR (&tabBg)},
							 			 {"border", STYLEPTR (&BStyles::noBorder)}}},
		{"activetab", 	{{"background", STYLEPTR (&activeTabBg)},
							 			 {"border", STYLEPTR (&BStyles::noBorder)}}},
		{"tool", 				{{"uses", STYLEPTR (&defaultStyles)},
							 			 {"bgcolors", STYLEPTR (&BColors::whites)}}},
		{"monitor", 		{{"background", STYLEPTR (&BStyles::blackFill)},
							 			 {"border", STYLEPTR (&BStyles::noBorder)}}},
		{"shape",	 		{{"background", STYLEPTR (&BStyles::noFill)},
							 		 {"border", STYLEPTR (&BStyles::noBorder)},
							 	 	 {"fgcolors", STYLEPTR (&BColors::whites)},
									 {"symbolcolors", STYLEPTR (&fgColors)},
									 {"font", STYLEPTR (&lfLabelFont)},
									 {"bgcolors", STYLEPTR (&bgColors)}}},
		{"dial", 			{{"uses", STYLEPTR (&defaultStyles)},
									 {"fgcolors", STYLEPTR (&fgColors)},
									 {"bgcolors", STYLEPTR (&bgColors)},
									 {"textcolors", STYLEPTR (&fgColors)},
									 {"font", STYLEPTR (&defaultFont)}}},
		{"label", 		{{"uses", STYLEPTR (&labelStyles)}}},
		{"smlabel",	 	{{"uses", STYLEPTR (&defaultStyles)},
									 {"textcolors", STYLEPTR (&fgColors)},
									 {"font", STYLEPTR (&smFont)}}},
		{"select",	 	{{"uses", STYLEPTR (&defaultStyles)}}},
		{"select/label",	{{"uses", STYLEPTR (&defaultStyles)},
									 {"textcolors", STYLEPTR (&BColors::whites)},
									 {"font", STYLEPTR (&defaultFont)}}},
		{"select/click",	{{"uses", STYLEPTR (&defaultStyles)},
									 {"bgcolors", STYLEPTR (&clickColors)}}},
		{"menu",	 		{{"border", STYLEPTR (&BStyles::greyBorder1pt)},
									 {"background", STYLEPTR (&BStyles::grey20Fill)}}},
		{"menu/item",	{{"uses", STYLEPTR (&defaultStyles)},
									 {"textcolors", STYLEPTR (&BColors::whites)},
									 {"font", STYLEPTR (&lfLabelFont)}}},
		{"menu/button",	 	{{"border", STYLEPTR (&BStyles::greyBorder1pt)},
							 		 {"background", STYLEPTR (&BStyles::grey20Fill)},
									 {"bgcolors", STYLEPTR (&BColors::darks)}}},
		{"menu/listbox",	{{"border", STYLEPTR (&BStyles::greyBorder1pt)},
									 {"background", STYLEPTR (&BStyles::grey20Fill)}}},
		{"menu/listbox/item",	{{"uses", STYLEPTR (&defaultStyles)},
									 {"textcolors", STYLEPTR (&BColors::whites)},
									 {"font", STYLEPTR (&lfLabelFont)}}},
		{"menu/listbox//button",	{{"border", STYLEPTR (&BStyles::greyBorder1pt)},
									 {"background", STYLEPTR (&BStyles::grey20Fill)},
							 	 	 {"bgcolors", STYLEPTR (&BColors::darks)}}}
	});
};



#endif /* BSHAPRGUI_HPP_ */
