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

#include "BShaprGUI.hpp"

BShaprGUI::BShaprGUI (const char *bundlePath, const LV2_Feature *const *features, PuglNativeWindow parentWindow) :
	Window (1200, 780, "B.Shapr", parentWindow, true),
	sz (1.0), horizonPos (0), monitorScale (1), beatsPerBar (4.0), beatUnit (4), minorXSteps (1.0), majorXSteps (1.0),
	pluginPath (bundlePath ? std::string (bundlePath) : std::string ("")), controller (NULL), write_function (NULL), map (NULL),
	surface (NULL), cr1 (NULL), cr2 (NULL), cr3 (NULL), cr4 (NULL), pat1 (NULL), pat2 (NULL), pat3 (NULL), pat4 (NULL), pat5 (NULL),
	bgImageSurface (nullptr),

	mContainer (0, 0, 1200, 780, "widget"),
	messageLabel (600, 45, 600, 20, "label", ""),
	baseValueSelect (480, 280, 100, 20, "select", 1.0, 1.0, 16.0, 0.01),
	baseListBox (620, 280, 100, 20, 100, 80, "menu", {{0, "Seconds"}, {1, "Beats"}, {2, "Bars"}}),
	monitorSurface (24, 94, 1152, 172, "monitor"),
	monitorHorizon1 (0, 0, 64, 172, "horizon"),
	monitorHorizon2 (-1152, 0, 64, 172, "horizon")

{
	// Init shapes
	for (int i = 0; i < MAXSHAPES; ++i)
	{
		shapeGui[i].tabIcon = BWidgets::ImageIcon (20 + i * 120, 330, 119, 40, "tab", pluginPath + "Shape" + std::to_string (i + 1) + ".png");

		shapeGui[i].shapeContainer = BWidgets::Widget (20, 370, 1160, 390, "widget");

		for (int j = 0; j < i; ++j)
		{
			shapeGui[i].inputShapeLabelIcons.push_back (
					BWidgets::ImageIcon (260 + j * 100, 30, 100, 20, "widget", pluginPath + "Shape" + std::to_string (j + 1) + ".png"));
		}

		shapeGui[i].inputSelect = SelectWidget (60, 20, 200 + i * 100, 40, "tool", 100, 40, 2 + i, 1);
		shapeGui[i].inputAmpDial = BWidgets::DisplayDial (1000, 14, 50, 56, "dial", 1.0, -1.0, 1.0, 0, "%1.3f");
		shapeGui[i].targetListBox = BWidgets::PopupListBox (120, 280, 180, 20, 0, -220, 180, 220, "menu",
															{{0, "Level"},
															 {5, "Gain"},
															 {1, "Balance"},
															 {2, "Width"},
															 {3, "Low pass filter"},
															 {6, "Low pass filter (log)"},
															 {4, "High pass filter"},
														 	 {7, "High pass filter (log)"},
														 	 {8, "Pitch"},
														 	 {9, "Delay"}});
		shapeGui[i].shapeWidget = ShapeWidget (4, 94, 1152, 172, "shape");
		shapeGui[i].toolSelect = SelectWidget (437, 272, 276, 36, "tool", 36, 36, 5, 1);
		shapeGui[i].shapeLabelIcon = BWidgets::ImageIcon (1000, 280, 160, 20, "widget", pluginPath + "Shape" + std::to_string (i + 1) + ".png");
		shapeGui[i].outputSelect = SelectWidget (60, 340, 100, 40, "tool", 100, 40, 1, 1);
		shapeGui[i].outputAmpDial = BWidgets::DisplayDial (1000, 334, 50, 56, "dial", 1.0, 0.0, 1.0, 0, "%1.3f");

		shapeGui[i].shapeContainer.rename ("widget");
		shapeGui[i].tabIcon.rename ("tab");
		shapeGui[i].inputSelect.rename ("tool");
		shapeGui[i].inputAmpDial.rename ("dial");
		shapeGui[i].targetListBox.rename ("menu");
		shapeGui[i].shapeWidget.rename ("shape");
		shapeGui[i].toolSelect.rename ("tool");
		shapeGui[i].shapeLabelIcon.rename ("widget");
		shapeGui[i].outputSelect.rename ("tool");
		shapeGui[i].outputAmpDial.rename ("dial");
	}

	// Init main monitor
	if (!init_mainMonitor ())
	{
		std::cerr << "BShapr.lv2#GUI: Failed to init monitor." <<  std::endl;
		destroy_mainMonitor ();
		throw std::bad_alloc ();
	}

	// Link controllers
	controllerWidgets[BASE] = (BWidgets::ValueWidget*) &baseListBox;
	controllerWidgets[BASE_VALUE] = (BWidgets::ValueWidget*) &baseValueSelect;
	for (int i = 0; i < MAXSHAPES; ++i)
	{
		controllerWidgets[SHAPERS + i * SH_SIZE + SH_INPUT] = (BWidgets::ValueWidget*) &shapeGui[i].inputSelect;
		controllerWidgets[SHAPERS + i * SH_SIZE + SH_INPUT_AMP] = (BWidgets::ValueWidget*) &shapeGui[i].inputAmpDial;
		controllerWidgets[SHAPERS + i * SH_SIZE + SH_TARGET] = (BWidgets::ValueWidget*) &shapeGui[i].targetListBox;
		controllerWidgets[SHAPERS + i * SH_SIZE + SH_OUTPUT] = (BWidgets::ValueWidget*) &shapeGui[i].outputSelect;
		controllerWidgets[SHAPERS + i * SH_SIZE + SH_OUTPUT_AMP] = (BWidgets::ValueWidget*) &shapeGui[i].outputAmpDial;
	}

	// Set callbacks
	baseValueSelect.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BShaprGUI::valueChangedCallback);
	baseListBox.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BShaprGUI::valueChangedCallback);
	monitorSurface.setCallbackFunction (BEvents::EventType::WHEEL_SCROLL_EVENT, BShaprGUI::wheelScrolledCallback);
	for (int i = 0; i < MAXSHAPES; ++i)
	{
		shapeGui[i].tabIcon.setCallbackFunction (BEvents::EventType::BUTTON_PRESS_EVENT, BShaprGUI::tabClickedCallback);
		shapeGui[i].inputSelect.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BShaprGUI::valueChangedCallback);
		shapeGui[i].inputAmpDial.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BShaprGUI::valueChangedCallback);
		shapeGui[i].targetListBox.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BShaprGUI::valueChangedCallback);
		shapeGui[i].outputSelect.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BShaprGUI::valueChangedCallback);
		shapeGui[i].outputAmpDial.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BShaprGUI::valueChangedCallback);
		shapeGui[i].shapeWidget.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BShaprGUI::shapeChangedCallback);
		shapeGui[i].toolSelect.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BShaprGUI::toolChangedCallback);
	}

	// Configure widgets
	calculateXSteps ();
	mContainer.loadImage (pluginPath + BG_FILE);
	monitorSurface.setScrollable (true);
	shapeGui[0].tabIcon.rename ("activetab");
	for (int i = 0; i < MAXSHAPES; ++i)
	{
		for (int j = 0; j < shapeGui[i].inputShapeLabelIcons.size(); ++j) shapeGui[i].inputShapeLabelIcons[j].setClickable (false);
		shapeGui[i].shapeWidget.setTool (ToolType::POINT_NODE_TOOL);
		shapeGui[i].inputAmpDial.setScrollable (true);
		shapeGui[i].inputAmpDial.setHardChangeable (false);
		//shapeGui[i].inputAmpDial.hide ();
		shapeGui[i].outputAmpDial.setScrollable (true);
		shapeGui[i].outputAmpDial.setHardChangeable (false);
		//shapeGui[i].outputAmpDial.hide ();
		if (i >= 1) shapeGui[i].shapeContainer.hide();
	}
	applyChildThemes ();

	// Pack widgets
	monitorSurface.add (monitorHorizon1);
	monitorSurface.add (monitorHorizon2);
	mContainer.add (monitorSurface);
	for (int i = 0; i < MAXSHAPES; ++i)
	{
		mContainer.add (shapeGui[i].tabIcon);
		for (int j = 0; j < shapeGui[i].inputShapeLabelIcons.size(); ++j) shapeGui[i].shapeContainer.add (shapeGui[i].inputShapeLabelIcons[j]);
		shapeGui[i].shapeContainer.add (shapeGui[i].inputSelect);
		shapeGui[i].shapeContainer.add (shapeGui[i].inputAmpDial);
		shapeGui[i].shapeContainer.add (shapeGui[i].targetListBox);
		shapeGui[i].shapeContainer.add (shapeGui[i].shapeWidget);
		shapeGui[i].shapeContainer.add (shapeGui[i].toolSelect);
		shapeGui[i].shapeContainer.add (shapeGui[i].shapeLabelIcon);
		shapeGui[i].shapeContainer.add (shapeGui[i].outputSelect);
		shapeGui[i].shapeContainer.add (shapeGui[i].outputAmpDial);
		mContainer.add (shapeGui[i].shapeContainer);
	}
	mContainer.add (messageLabel);
	mContainer.add (baseValueSelect);
	mContainer.add (baseListBox);
	add (mContainer);

	// Post addition configurations
	for (int i = 0; i < MAXSHAPES; ++i) shapeGui[i].shapeWidget.setDefaultShape(defaultEndNodes[0]);
	for (int i = 0; i < MAXSHAPES; ++i) shapeGui[i].shapeWidget.setValueEnabled (true);

	//Scan host features for URID map
	LV2_URID_Map* m = NULL;
	for (int i = 0; features[i]; ++i)
	{
		if (strcmp(features[i]->URI, LV2_URID__map) == 0) m = (LV2_URID_Map*) features[i]->data;
	}
	if (!m) throw std::invalid_argument ("Host does not support urid:map");

	//Map URIS
	map = m;
	mapURIDs (map, &urids);

	// Initialize forge
	lv2_atom_forge_init (&forge, map);
}

BShaprGUI::~BShaprGUI()
{
	sendGuiOff ();
	destroy_mainMonitor ();
}

void BShaprGUI::portEvent(uint32_t port, uint32_t bufferSize, uint32_t format, const void* buffer)
{
	// Notify port
	if ((format == urids.atom_eventTransfer) && (port == NOTIFY))
	{
		const LV2_Atom* atom = (const LV2_Atom*) buffer;
		if ((atom->type == urids.atom_Blank) || (atom->type == urids.atom_Object))
		{
			const LV2_Atom_Object* obj = (const LV2_Atom_Object*) atom;

			// Monitor notification
			if (obj->body.otype == urids.notify_monitorEvent)
			{
				const LV2_Atom* data = NULL;
				lv2_atom_object_get(obj, urids.notify_monitor, &data, 0);
				if (data && (data->type == urids.atom_Vector))
				{
					const LV2_Atom_Vector* vec = (const LV2_Atom_Vector*) data;
					if (vec->body.child_type == urids.atom_Float)
					{
						uint32_t notificationsCount = (uint32_t) ((data->size - sizeof(LV2_Atom_Vector_Body)) / sizeof (BShaprNotifications));
						BShaprNotifications* notifications = (BShaprNotifications*) (&vec->body + 1);
						if (notificationsCount > 0)
						{
							add_monitor_data (notifications, notificationsCount, horizonPos);
							redrawMainMonitor ();
						}
					}
				}
				else std::cerr << "BShapr.lv2#GUI: Corrupt audio message." << std::endl;
			}

			// Message notification
			else if (obj->body.otype == urids.notify_messageEvent)
			{
				const LV2_Atom* data = NULL;
				lv2_atom_object_get(obj, urids.notify_message, &data, 0);
				if (data && (data->type == urids.atom_Int))
				{
					const int messageNr = ((LV2_Atom_Int*)data)->body;
					std::string msg = ((messageNr >= NO_MSG) && (messageNr <= MAX_MSG) ? messageStrings[messageNr] : "");
					messageLabel.setText (msg);
				}
			}

			// Status notification
			else if (obj->body.otype == urids.notify_statusEvent)
			{
				const LV2_Atom *oBpb = NULL, *oBu = NULL;
				lv2_atom_object_get(obj, urids.time_beatsPerBar, &oBpb,
					 											 urids.time_beatUnit, &oBu, 0);
				if (oBpb && (oBpb->type == urids.atom_Float))
				{
					beatsPerBar = ((LV2_Atom_Float*)oBpb)->body;
					calculateXSteps ();
				}

				if (oBu && (oBu->type == urids.atom_Int))
				{
					beatUnit = ((LV2_Atom_Float*)oBu)->body;
					calculateXSteps ();
				}
			}

			// Shape notification
			else if (obj->body.otype == urids.notify_shapeEvent)
			{
				LV2_Atom *sNr = NULL, *sData = NULL;
				lv2_atom_object_get (obj, urids.notify_shapeNr, &sNr,
										  urids.notify_shapeData, &sData, 0);

				if (sNr && (sNr->type == urids.atom_Int) &&
					sData && (sData->type == urids.atom_Vector))
				{
					int shapeNr = ((LV2_Atom_Int*)sNr)->body;

					if ((shapeNr >= 0) && (shapeNr < MAXSHAPES))
					{
						const LV2_Atom_Vector* vec = (const LV2_Atom_Vector*) sData;
						size_t vecSize = (sData->size - sizeof(LV2_Atom_Vector_Body)) / (7 * sizeof (float));
						if (vec->body.child_type == urids.atom_Float)
						{
							shapeGui[shapeNr].shapeWidget.setValueEnabled (false);
							shapeGui[shapeNr].shapeWidget.clearShape ();
							float* data = (float*)(&vec->body + 1);
							for (int nodeNr = 0; (nodeNr < vecSize) && (nodeNr < MAXNODES); ++nodeNr)
							{
								Node node (&data[nodeNr * 7]);
								shapeGui[shapeNr].shapeWidget.appendNode (node);
							}
							shapeGui[shapeNr].shapeWidget.validateShape();
							shapeGui[shapeNr].shapeWidget.update ();
							shapeGui[shapeNr].shapeWidget.setValueEnabled (true);
						}
					}
				}
			}

		}
	}


	// Scan remaining ports
	else if ((format == 0) && (port >= CONTROLLERS) && (port < CONTROLLERS + NR_CONTROLLERS))
	{
		float* pval = (float*) buffer;
		if (port == CONTROLLERS + ACTIVE_SHAPE)
		{
			int sh = LIMIT (controllers[ACTIVE_SHAPE], 1, MAXSHAPES) - 1;
			shapeGui[sh].tabIcon.rename ("tab");
			shapeGui[sh].tabIcon.applyTheme (theme);
			shapeGui[sh].shapeContainer.hide();
			controllers[ACTIVE_SHAPE] = LIMIT (*pval, 1, MAXSHAPES);
			int nsh = controllers[ACTIVE_SHAPE] - 1;
			shapeGui[nsh].tabIcon.rename ("activetab");
			shapeGui[nsh].tabIcon.applyTheme (theme);
			shapeGui[nsh].shapeContainer.show();
		}

		else
		{
			controllerWidgets[port - CONTROLLERS]->setValue (*pval);
			controllers[port - CONTROLLERS] = *pval;

			if ((port == CONTROLLERS + BASE) || (port == CONTROLLERS + BASE_VALUE)) calculateXSteps ();
		}
	}
}

void BShaprGUI::resizeGUI()
{
	hide ();

	// Resize Fonts
	defaultFont.setFontSize (12 * sz);
	lfLabelFont.setFontSize (12 * sz);

	// Resize widgets
	RESIZE (mContainer, 0, 0, 1200, 780, sz);
	RESIZE (messageLabel, 600, 45, 600, 20, sz);
	RESIZE (baseValueSelect, 480, 280, 100, 20, sz);
	RESIZE (baseListBox, 620, 280, 100, 20, sz);
	baseListBox.resizeListBox (100 * sz, 80 * sz);
	RESIZE (monitorSurface, 24, 94, 1152, 172, sz);
	RESIZE (monitorHorizon1, monitorHorizon1.getX(), 0, 64, 172, sz);
	RESIZE (monitorHorizon2, monitorHorizon1.getX(), 0, 64, 172, sz);
	for (int i = 0; i < MAXSHAPES; ++i)
	{
		RESIZE (shapeGui[i].tabIcon, 20 + i * 120, 330, 119, 40, sz);
		RESIZE (shapeGui[i].shapeContainer, 20, 370, 1160, 390, sz);
		for (int j = 0; j < shapeGui[i].inputShapeLabelIcons.size(); ++j)
		{
			RESIZE (shapeGui[i].inputShapeLabelIcons[j], 260 + j * 100, 30, 100, 20, sz);
		}

		RESIZE (shapeGui[i].inputSelect, 60, 20, 200 + i * 100, 40, sz);
		shapeGui[i].inputSelect.resizeSelection (100 * sz, 40 * sz);
		RESIZE (shapeGui[i].inputAmpDial, 1000, 14, 50, 56, sz);
		RESIZE (shapeGui[i].targetListBox, 120, 280, 180, 20, sz);
		shapeGui[i].targetListBox.resizeListBox (180 * sz, 220 * sz);
		shapeGui[i].targetListBox.moveListBox (0, -220 * sz);
		RESIZE (shapeGui[i].shapeWidget, 4, 94, 1152, 172, sz);
		RESIZE (shapeGui[i].toolSelect, 437, 272, 276, 36, sz);
		shapeGui[i].toolSelect.resizeSelection (36 * sz, 36 * sz);
		RESIZE (shapeGui[i].shapeLabelIcon, 1000, 280, 160, 20, sz);
		RESIZE (shapeGui[i].outputSelect, 60, 340, 100, 40, sz);
		shapeGui[i].outputSelect.resizeSelection (100 * sz, 40 * sz);
		RESIZE (shapeGui[i].outputAmpDial, 1000, 334, 50, 56, sz);
	}

	// Update monitor, const std::string& name
	destroy_mainMonitor ();
	init_mainMonitor ();
	redrawMainMonitor ();

	// Apply changes
	applyChildThemes ();
	show ();
}

void BShaprGUI::applyChildThemes ()
{
	mContainer.applyTheme (theme);
	messageLabel.applyTheme (theme);
	baseValueSelect.applyTheme (theme);
	baseListBox.applyTheme (theme);
	monitorSurface.applyTheme (theme);
	for (int i = 0; i < MAXSHAPES; ++i)
	{
		shapeGui[i].shapeContainer.applyTheme (theme);
		shapeGui[i].tabIcon.applyTheme (theme);
		for (int j = 0; j < shapeGui[i].inputShapeLabelIcons.size(); ++j) shapeGui[i].inputShapeLabelIcons[j].applyTheme (theme);
		shapeGui[i].inputSelect.applyTheme (theme);
		shapeGui[i].inputAmpDial.applyTheme (theme);
		shapeGui[i].inputAmpDial.getDisplayLabel()->setTextColors (lbColors);
		shapeGui[i].targetListBox.applyTheme (theme);
		shapeGui[i].shapeWidget.applyTheme (theme);
		shapeGui[i].toolSelect.applyTheme (theme);
		shapeGui[i].shapeLabelIcon.applyTheme (theme);
		shapeGui[i].outputSelect.applyTheme (theme);
		shapeGui[i].outputAmpDial.applyTheme (theme);
		shapeGui[i].outputAmpDial.getDisplayLabel()->setTextColors (lbColors);
	}
}

void BShaprGUI::onConfigure (BEvents::ExposeEvent* event)
{
	Window::onConfigure (event);

	sz = (width_ / 1200 > height_ / 780 ? height_ / 780 : width_ / 1200);
	resizeGUI ();
}

void BShaprGUI::sendGuiOn ()
{
	uint8_t obj_buf[64];
	lv2_atom_forge_set_buffer(&forge, obj_buf, sizeof(obj_buf));

	LV2_Atom_Forge_Frame frame;
	LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object(&forge, &frame, 0, urids.ui_on);
	lv2_atom_forge_pop(&forge, &frame);
	write_function(controller, CONTROL, lv2_atom_total_size(msg), urids.atom_eventTransfer, msg);
}

void BShaprGUI::sendGuiOff ()
{
	uint8_t obj_buf[64];
	lv2_atom_forge_set_buffer(&forge, obj_buf, sizeof(obj_buf));

	LV2_Atom_Forge_Frame frame;
	LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object(&forge, &frame, 0, urids.ui_off);
	lv2_atom_forge_pop(&forge, &frame);
	write_function(controller, CONTROL, lv2_atom_total_size(msg), urids.atom_eventTransfer, msg);
}

void BShaprGUI::sendShape (size_t shapeNr)
{
	size_t size = shapeGui[shapeNr].shapeWidget.size ();

	uint8_t obj_buf[4096];
	lv2_atom_forge_set_buffer(&forge, obj_buf, sizeof(obj_buf));

	// Load shapeBuffer
	for (int i = 0; i < size; ++i)
	{
		Node node = shapeGui[shapeNr].shapeWidget.getNode (i);
		shapeBuffer[i * 7 + 0] = (float)node.nodeType;
		shapeBuffer[i * 7 + 1] = (float)node.point.x;
		shapeBuffer[i * 7 + 2] = (float)node.point.y;
		shapeBuffer[i * 7 + 3] = (float)node.handle1.x;
		shapeBuffer[i * 7 + 4] = (float)node.handle1.y;
		shapeBuffer[i * 7 + 5] = (float)node.handle2.x;
		shapeBuffer[i * 7 + 6] = (float)node.handle2.y;
	}

	// Notify shapeBuffer
	LV2_Atom_Forge_Frame frame;
	LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object (&forge, &frame, 0, urids.notify_shapeEvent);
	lv2_atom_forge_key(&forge, urids.notify_shapeNr);
	lv2_atom_forge_int(&forge, shapeNr);
	lv2_atom_forge_key(&forge, urids.notify_shapeData);
	lv2_atom_forge_vector(&forge, sizeof(float), urids.atom_Float, (uint32_t) (7 * size), &shapeBuffer);
	lv2_atom_forge_pop(&forge, &frame);
	write_function (controller, CONTROL, lv2_atom_total_size(msg), urids.atom_eventTransfer, msg);
}

void BShaprGUI::valueChangedCallback (BEvents::Event* event)
{
	if ((event) && (event->getWidget ()))
	{
		BWidgets::ValueWidget* widget = (BWidgets::ValueWidget*) event->getWidget ();
		float value = widget->getValue ();

		if (widget->getMainWindow ())
		{
			BShaprGUI* ui = (BShaprGUI*) widget->getMainWindow ();
			int widgetNr = -1;

			for (int i = 0; i < NR_CONTROLLERS; ++i)
			{
				if (widget == ui->controllerWidgets[i])
				{
					widgetNr = i;
					break;
				}
			}

			// Controllers (menus, dials, selectors, ...)
			if (widgetNr >= 0)
			{
				ui->controllers[widgetNr] = value;
				ui->write_function(ui->controller, CONTROLLERS + widgetNr, sizeof(float), 0, &ui->controllers[widgetNr]);

				if (widgetNr >= SHAPERS)
				{
					// Target
					if ((widgetNr - SHAPERS) % SH_SIZE == SH_TARGET)
					{
						int sh = (widgetNr - SHAPERS) / SH_SIZE;
						int nr = value;
						if ((ui->shapeGui[sh].shapeWidget.isDefault ()) && (ui->shapeGui[sh].shapeWidget.getNode (0).point.y != defaultEndNodes[nr].point.y))
						{
							ui->shapeGui[sh].shapeWidget.setDefaultShape (defaultEndNodes[nr]);
						}

						ui->shapeGui[sh].shapeWidget.setScaleParameters (scaleParameters[nr].anchorYPos, scaleParameters[nr].anchorValue, scaleParameters[nr].ratio);
						ui->shapeGui[sh].shapeWidget.setUnit (scaleParameters[nr].unit);
						ui->shapeGui[sh].shapeWidget.setPrefix (scaleParameters[nr].prefix);
					}

					// Input
					if ((widgetNr - SHAPERS) % SH_SIZE == SH_INPUT)
					{
						int sh = (widgetNr - SHAPERS) / SH_SIZE;

						// Input: Constant => show level dial (range -1..1)
						if (value == BShaprInputIndex::CONSTANT)
						{
							ui->shapeGui[sh].inputAmpDial.setMin (-1);
							ui->shapeGui[sh].inputAmpDial.setMax (1);
							ui->shapeGui[sh].inputAmpDial.show();
						}

						// Input: AUDIO or ShapeX => show level dial (range 0..1)
						else if (value != BShaprInputIndex::OFF)
						{
							ui->shapeGui[sh].inputAmpDial.setMin (0);
							ui->shapeGui[sh].inputAmpDial.setMax (1);
							ui->shapeGui[sh].inputAmpDial.show();
						}

						// Input: OFF => hide dial
						else ui->shapeGui[sh].inputAmpDial.hide();
					}

					// Output
					if ((widgetNr - SHAPERS) % SH_SIZE == SH_OUTPUT)
					{
						int sh = (widgetNr - SHAPERS) / SH_SIZE;

						if (value != BShaprInputIndex::OFF) ui->shapeGui[sh].outputAmpDial.show ();
						else ui->shapeGui[sh].outputAmpDial.hide();

					}
				}
			}
		}
	}
}

void BShaprGUI::tabClickedCallback (BEvents::Event* event)
{
	if ((event) && (event->getWidget ()))
	{
		BWidgets::ImageIcon* widget = (BWidgets::ImageIcon*) event->getWidget ();
		if (widget->getMainWindow ())
		{
			BShaprGUI* ui = (BShaprGUI*) widget->getMainWindow ();
			for (int i = 0; i < MAXSHAPES; ++i)
			{
				if (widget == &ui->shapeGui[i].tabIcon)
				{
					int sh = LIMIT (ui->controllers[ACTIVE_SHAPE], 1, MAXSHAPES) - 1;
					ui->shapeGui[sh].tabIcon.rename ("tab");
					ui->shapeGui[sh].tabIcon.applyTheme (ui->theme);
					ui->shapeGui[sh].shapeContainer.hide();
					ui->controllers[ACTIVE_SHAPE] = i + 1;
					ui->write_function(ui->controller, CONTROLLERS + ACTIVE_SHAPE, sizeof(float), 0, &ui->controllers[ACTIVE_SHAPE]);
					ui->shapeGui[i].tabIcon.rename ("activetab");
					ui->shapeGui[i].tabIcon.applyTheme (ui->theme);
					ui->shapeGui[i].shapeContainer.show();
					break;
				}
			}
		}
	}
}

void BShaprGUI::shapeChangedCallback (BEvents::Event* event)
{
	if ((event) && (event->getWidget ()))
	{
		BWidgets::ValueWidget* widget = (BWidgets::ValueWidget*) event->getWidget ();

		if (widget->getMainWindow () && (widget->getValue () == 1))
		{
			BShaprGUI* ui = (BShaprGUI*) widget->getMainWindow ();

			for (int i = 0; i < MAXSHAPES; ++i)
			{
				if (widget == (BWidgets::ValueWidget*) &ui->shapeGui[i].shapeWidget)
				{
					ui->sendShape(i);
					break;
				}
			}
		}
	}
}


void BShaprGUI::toolChangedCallback (BEvents::Event* event)
{
	if ((event) && (event->getWidget ()))
	{
		BWidgets::ValueWidget* widget = (BWidgets::ValueWidget*) event->getWidget ();

		if (widget->getMainWindow ())
		{
			BShaprGUI* ui = (BShaprGUI*) widget->getMainWindow ();

			for (int i = 0; i < MAXSHAPES; ++i)
			{
				if (widget == (BWidgets::ValueWidget*) &ui->shapeGui[i].toolSelect)
				{
					ui->shapeGui[i].shapeWidget.setTool((ToolType) ui->shapeGui[i].toolSelect.getValue ());
					break;
				}
			}
		}
	}
}

void BShaprGUI::wheelScrolledCallback (BEvents::Event* event)
{
	if ((event) && (event->getWidget ()))
	{
		BEvents::WheelEvent* we = (BEvents::WheelEvent*) event;
		BWidgets::Widget* widget = event->getWidget ();

		if (widget->getMainWindow ())
		{
			BShaprGUI* ui = (BShaprGUI*) widget->getMainWindow ();

			ui->monitorScale += 0.01 * we->getDeltaY ();
			if (ui->monitorScale < 0.01) ui->monitorScale = 0.01;
		}
	}
}

void BShaprGUI::calculateXSteps ()
{
	majorXSteps = 1.0 / controllers[BASE_VALUE];

	switch ((BShaprBaseIndex)controllers[BASE])
	{
		case SECONDS:	minorXSteps = majorXSteps / 10.0;
									break;

		case BEATS:		minorXSteps = majorXSteps / (16.0 / ((double)beatUnit));
									break;

		case BARS:		minorXSteps = majorXSteps / beatsPerBar;
									break;
	}

	if (controllers[BASE_VALUE] >= 10.0) minorXSteps = majorXSteps;

	for (int sh = 0; sh < MAXSHAPES; ++sh)
	{
		shapeGui[sh].shapeWidget.setMinorXSteps (minorXSteps);
		shapeGui[sh].shapeWidget.setMajorXSteps (majorXSteps);
		shapeGui[sh].shapeWidget.update ();
	}
}

bool BShaprGUI::init_mainMonitor ()
{
	//Initialize mainMonitor
	mainMonitorData.fill (defaultNotification);
	horizonPos = 0;

	//Initialize mainMonitors cairo data
	double width = monitorSurface.getEffectiveWidth ();
	double height = monitorSurface.getEffectiveHeight ();
	surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
	cr1 = cairo_create (surface);
	cr2 = cairo_create (surface);
	cr3 = cairo_create (surface);
	cr4 = cairo_create (surface);
	pat1 = cairo_pattern_create_linear (0, 0, 0, height);
	cairo_pattern_add_color_stop_rgba (pat1, 0.1, CAIRO_INK1, 1);
	cairo_pattern_add_color_stop_rgba (pat1, 0.6, CAIRO_INK1, 0);
	pat2 = cairo_pattern_create_linear (0, 0, 0, height);
	cairo_pattern_add_color_stop_rgba (pat2, 0.1, CAIRO_INK2, 1);
	cairo_pattern_add_color_stop_rgba (pat2, 0.6, CAIRO_INK2, 0);
	pat3 = cairo_pattern_create_linear (0, height, 0, 0);
	cairo_pattern_add_color_stop_rgba (pat3, 0.1, CAIRO_INK1, 1);
	cairo_pattern_add_color_stop_rgba (pat3, 0.6, CAIRO_INK1, 0);
	pat4 = cairo_pattern_create_linear (0, height, 0, 0);
	cairo_pattern_add_color_stop_rgba (pat4, 0.1, CAIRO_INK2, 1);
	cairo_pattern_add_color_stop_rgba (pat4, 0.6, CAIRO_INK2, 0);

	return (pat4 && (cairo_pattern_status (pat4) == CAIRO_STATUS_SUCCESS) &&
			pat3 && (cairo_pattern_status (pat3) == CAIRO_STATUS_SUCCESS) &&
			pat2 && (cairo_pattern_status (pat2) == CAIRO_STATUS_SUCCESS) &&
			pat1 && (cairo_pattern_status (pat1) == CAIRO_STATUS_SUCCESS) &&
			cr4 && (cairo_status (cr4) == CAIRO_STATUS_SUCCESS) &&
			cr3 && (cairo_status (cr3) == CAIRO_STATUS_SUCCESS)&&
			cr2 && (cairo_status (cr2) == CAIRO_STATUS_SUCCESS) &&
			cr1 && (cairo_status (cr1) == CAIRO_STATUS_SUCCESS) &&
			surface && (cairo_surface_status (surface) == CAIRO_STATUS_SUCCESS));
}

void BShaprGUI::destroy_mainMonitor ()
{
	//Destroy also mainMonitors cairo data
	if (pat4 && (cairo_pattern_status (pat4) == CAIRO_STATUS_SUCCESS)) cairo_pattern_destroy (pat4);
	if (pat3 && (cairo_pattern_status (pat3) == CAIRO_STATUS_SUCCESS)) cairo_pattern_destroy (pat3);
	if (pat2 && (cairo_pattern_status (pat2) == CAIRO_STATUS_SUCCESS)) cairo_pattern_destroy (pat2);
	if (pat1 && (cairo_pattern_status (pat1) == CAIRO_STATUS_SUCCESS)) cairo_pattern_destroy (pat1);
	if (cr4 && (cairo_status (cr4) == CAIRO_STATUS_SUCCESS)) cairo_destroy (cr4);
	if (cr3 && (cairo_status (cr3) == CAIRO_STATUS_SUCCESS)) cairo_destroy (cr3);
	if (cr2 && (cairo_status (cr2) == CAIRO_STATUS_SUCCESS)) cairo_destroy (cr2);
	if (cr1 && (cairo_status (cr1) == CAIRO_STATUS_SUCCESS)) cairo_destroy (cr1);
	if (surface && (cairo_surface_status (surface) == CAIRO_STATUS_SUCCESS)) cairo_surface_destroy (surface);
}

void BShaprGUI::add_monitor_data (BShaprNotifications* notifications, uint32_t notificationsCount, double& end)
{
	for (int i = 0; i < notificationsCount; ++i)
	{
		int monitorpos = notifications[i].position;
		if (monitorpos >= MONITORBUFFERSIZE) monitorpos = MONITORBUFFERSIZE;
		if (monitorpos < 0) monitorpos = 0;

		mainMonitorData[monitorpos].inputMin = notifications[i].inputMin;
		mainMonitorData[monitorpos].inputMax = notifications[i].inputMax;
		mainMonitorData[monitorpos].outputMin = notifications[i].outputMin;
		mainMonitorData[monitorpos].outputMax = notifications[i].outputMax;
		horizonPos = ((double)monitorpos) / MONITORBUFFERSIZE;
	}
}

void BShaprGUI::redrawMainMonitor ()
{
	double width = monitorSurface.getEffectiveWidth ();
	double height = monitorSurface.getEffectiveHeight ();

	cairo_t* cr = cairo_create (monitorSurface.getDrawingSurface ());
	if (cairo_status (cr) != CAIRO_STATUS_SUCCESS) return;

	// Clear background
	cairo_set_source_rgba (cr, CAIRO_BG_COLOR);
	cairo_rectangle (cr, 0, 0, width, height);
	cairo_fill (cr);

	// Draw grid
	cairo_set_source_rgba (cr, CAIRO_BG_COLOR2);
	cairo_set_line_width (cr, 1);
	cairo_move_to (cr, 0, (0.5 - 0.4 / monitorScale) * height);
	cairo_line_to (cr, width, (0.5 - 0.4 / monitorScale) * height);
	cairo_move_to (cr, 0, 0.5 * height);
	cairo_line_to (cr, width, 0.5 * height);
	cairo_move_to (cr, 0, (0.5 + 0.4 / monitorScale) * height);
	cairo_line_to (cr, width, (0.5 + 0.4 / monitorScale) * height);
	cairo_stroke (cr);

	cairo_set_line_width (cr, 0.5);
	for (double x = 0; x < 1; x += minorXSteps)
	{
		cairo_move_to (cr, x * width, 0);
		cairo_line_to (cr, x * width, height);
	}
	cairo_stroke (cr);

	cairo_set_line_width (cr, 1);
	for (double x = 0; x < 1; x += majorXSteps)
	{
		cairo_move_to (cr, x * width, 0);
		cairo_line_to (cr, x * width, height);
	}
	cairo_stroke (cr);

	cairo_surface_clear (surface);

	// Draw input (cr, cr3) and output (cr2, cr4) curves
	cairo_move_to (cr1, 0, height * (0.5  - (0.4 * LIMIT ((mainMonitorData[0].inputMax / monitorScale), 0, 1))));
	cairo_move_to (cr2, 0, height * (0.5  - (0.4 * LIMIT ((mainMonitorData[0].outputMax / monitorScale), 0, 1))));
	cairo_move_to (cr3, 0, height * (0.5  + (0.4 * LIMIT (-(mainMonitorData[0].inputMin / monitorScale), 0, 1))));
	cairo_move_to (cr4, 0, height * (0.5  + (0.4 * LIMIT (-(mainMonitorData[0].outputMin / monitorScale), 0, 1))));

	for (int i = 0; i < MONITORBUFFERSIZE; ++i)
	{
		double pos = ((double) i) / (MONITORBUFFERSIZE - 1.0f);
		cairo_line_to (cr1, pos * width, height * (0.5  - (0.4 * LIMIT ((mainMonitorData[i].inputMax / monitorScale), 0, 1))));
		cairo_line_to (cr2, pos * width, height * (0.5  - (0.4 * LIMIT ((mainMonitorData[i].outputMax / monitorScale), 0, 1))));
		cairo_line_to (cr3, pos * width, height * (0.5  + (0.4 * LIMIT (-(mainMonitorData[i].inputMin / monitorScale), 0, 1))));
		cairo_line_to (cr4, pos * width, height * (0.5  + (0.4 * LIMIT (-(mainMonitorData[i].outputMin / monitorScale), 0, 1))));
	}

	// Visualize input (cr, cr3) and output (cr2, cr4) curves
	cairo_set_source_rgba (cr1, CAIRO_INK1, 1.0);
	cairo_set_line_width (cr1, 3);
	cairo_set_source_rgba (cr2, CAIRO_INK2, 1.0);
	cairo_set_line_width (cr2, 3);
	cairo_stroke_preserve (cr1);
	cairo_stroke_preserve (cr2);
	cairo_set_source_rgba (cr3, CAIRO_INK1, 1.0);
	cairo_set_line_width (cr3, 3);
	cairo_set_source_rgba (cr4, CAIRO_INK2, 1.0);
	cairo_set_line_width (cr4, 3);
	cairo_stroke_preserve (cr3);
	cairo_stroke_preserve (cr4);

	// Visualize input (cr, cr3) and output (cr2, cr4) areas under the curves
	cairo_line_to (cr1, width, height * 0.5);
	cairo_line_to (cr1, 0, height * 0.5);
	cairo_close_path (cr1);
	cairo_line_to (cr2, width, height * 0.5);
	cairo_line_to (cr2, 0, height * 0.5);
	cairo_close_path (cr2);
	cairo_set_source (cr1, pat1);
	cairo_set_line_width (cr1, 0);
	cairo_set_source (cr2, pat2);
	cairo_set_line_width (cr2, 0);
	cairo_fill (cr1);
	cairo_fill (cr2);
	cairo_line_to (cr3, width, height * 0.5);
	cairo_line_to (cr3, 0, height * 0.5);
	cairo_close_path (cr3);
	cairo_line_to (cr4, width, height * 0.5);
	cairo_line_to (cr4, 0, height * 0.5);
	cairo_close_path (cr4);
	cairo_set_source (cr3, pat3);
	cairo_set_line_width (cr3, 0);
	cairo_set_source (cr4, pat4);
	cairo_set_line_width (cr4, 0);
	cairo_fill (cr3);
	cairo_fill (cr4);

	cairo_set_source_surface (cr, surface, 0, 0);
	cairo_paint (cr);

	cairo_destroy (cr);

	monitorHorizon1.moveTo (horizonPos * width, 0);
	monitorHorizon2.moveTo ((horizonPos - 1) * width, 0);

	monitorSurface.update ();
}

LV2UI_Handle instantiate (const LV2UI_Descriptor *descriptor, const char *plugin_uri, const char *bundle_path,
						  LV2UI_Write_Function write_function, LV2UI_Controller controller, LV2UI_Widget *widget,
						  const LV2_Feature *const *features)
{
	PuglNativeWindow parentWindow = 0;
	LV2UI_Resize* resize = NULL;

	if (strcmp(plugin_uri, BSHAPR_URI) != 0)
	{
		std::cerr << "BShapr.lv2#GUI: GUI does not support plugin with URI " << plugin_uri << std::endl;
		return NULL;
	}

	for (int i = 0; features[i]; ++i)
	{
		if (!strcmp(features[i]->URI, LV2_UI__parent)) parentWindow = (PuglNativeWindow) features[i]->data;
		else if (!strcmp(features[i]->URI, LV2_UI__resize)) resize = (LV2UI_Resize*)features[i]->data;
	}
	if (parentWindow == 0) std::cerr << "BShapr.lv2#GUI: No parent window.\n";

	// New instance
	BShaprGUI* ui;
	try {ui = new BShaprGUI (bundle_path, features, parentWindow);}
	catch (std::exception& exc)
	{
		std::cerr << "BShapr.lv2#GUI: Instantiation failed. " << exc.what () << std::endl;
		return NULL;
	}

	ui->controller = controller;
	ui->write_function = write_function;

	// Reduce min GUI size for small displays
	double sz = 1.0;
	int screenWidth  = getScreenWidth ();
	int screenHeight = getScreenHeight ();
	if ((screenWidth < 1240) || (screenHeight < 800)) sz = 0.66;
	if ((screenWidth < 840) || (screenHeight < 530)) sz = 0.50;
	std::cerr << "BShapr_GUI.lv2 screen size " << screenWidth << " x " << screenHeight <<
			". Set GUI size to " << 1200 * sz << " x " << 780 * sz << ".\n";

	if (resize) resize->ui_resize (resize->handle, 1200, 780);

	*widget = (LV2UI_Widget) puglGetNativeWindow (ui->getPuglView ());
	ui->sendGuiOn();

	return (LV2UI_Handle) ui;
}

void cleanup(LV2UI_Handle ui)
{
	BShaprGUI* pluginGui = (BShaprGUI*) ui;
	delete pluginGui;
}

void portEvent(LV2UI_Handle ui, uint32_t port_index, uint32_t buffer_size,
	uint32_t format, const void* buffer)
{
	BShaprGUI* pluginGui = (BShaprGUI*) ui;
	pluginGui->portEvent(port_index, buffer_size, format, buffer);
}

static int callIdle (LV2UI_Handle ui)
{
	BShaprGUI* pluginGui = (BShaprGUI*) ui;
	pluginGui->handleEvents ();
	return 0;
}

static const LV2UI_Idle_Interface idle = { callIdle };

static const void* extensionData(const char* uri)
{
	if (!strcmp(uri, LV2_UI__idleInterface)) return &idle;
	else return NULL;
}

const LV2UI_Descriptor guiDescriptor = {
		BSHAPR_GUI_URI,
		instantiate,
		cleanup,
		portEvent,
		extensionData
};

// LV2 Symbol Export
LV2_SYMBOL_EXPORT const LV2UI_Descriptor *lv2ui_descriptor(uint32_t index)
{
	switch (index) {
	case 0: return &guiDescriptor;
	default:return NULL;
    }
}
