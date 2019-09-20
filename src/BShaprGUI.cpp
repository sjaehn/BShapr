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
	Window (1200, 710, "B.Shapr", parentWindow, true),
	controller (NULL), write_function (NULL),
	beatsPerBar (4.0), beatUnit (4),

	mContainer (0, 0, 1200, 710, "widget"),
	messageLabel (600, 45, 600, 20, "label", ""),
	midiSwitch (720, 580, 20, 40, "dial", 0),
	midiPiano (900, 580, 140, 55, "widget", 0, 11),
	midiLabel (950, 565, 40, 10, "smlabel", "Filter"),
	baseValueSelect (480, 660, 100, 20, "select", 1.0, 1.0, 16.0, 0.01),
	baseListBox (620, 660, 100, 20, 0, -80, 100, 80, "menu", BItems::ItemList ({{0, "Seconds"}, {1, "Beats"}, {2, "Bars"}})),
	monitorContainer (24, 134, 1152, 352, "monitor"),
	monitorHorizon1 (0, 0, 64, 352, "horizon"),
	monitorHorizon2 (-1152, 0, 64, 352, "horizon"),
	input1Monitor (0, 0, 1152, 176, "monitor.in"),
	output1Monitor (0, 0, 1152, 176, "monitor.out"),
	input2Monitor (0, 176, 1152, 176, "monitor.in"),
	output2Monitor (0, 176, 1152, 176, "monitor.out"),

	horizonPos (0), monitorScale (0.25), minorXSteps (1.0), majorXSteps (1.0),
	pluginPath (bundlePath ? std::string (bundlePath) : std::string ("")),
	sz (1.0),
	bgImageSurface (nullptr), forge (), urids (), map (NULL)

{
	// Init shapes
	for (int i = 0; i < MAXSHAPES; ++i)
	{
		shapeGui[i].tabIcon = BWidgets::ImageIcon (20 + i * 120, 90, 119, 40, "tab", pluginPath + "inc/Shape" + std::to_string (i + 1) + ".png");
		shapeGui[i].shapeContainer = BWidgets::Widget (20, 130, 1160, 510, "widget");

		// Method menu
		BItems::ItemList il;
		shapeGui[i].methodIcons = {};
		for (int j = 0; j < MAXEFFECTS; ++j)
		{
			std::string iconPath = "";
			int index = 0;
			for (int k = 0; k < MAXEFFECTS; ++k)
			{
				if (j == methods[k].listIndex)
				{
					iconPath = pluginPath + methods[k].iconFileName;
					index = k;
					break;
				}
			}

			shapeGui[i].methodIcons.push_back (BWidgets::ImageIcon (0, 0, 154, 54, "icon", {iconPath, iconPath}));

			BWidgets::ImageIcon* icon = &*std::prev (shapeGui[i].methodIcons.end ());
			cairo_t* cr = cairo_create (icon->getIconSurface (BColors::NORMAL));
			cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 0.5);
			cairo_paint (cr);
			cairo_destroy (cr);

			il.push_back (BItems::Item (index, icon));
		}

		shapeGui[i].targetListBox = BWidgets::PopupListBox (20, 443, 174, 54, 0, -380, 184, 380, "menu2", il, 0);

		shapeGui[i].shapeWidget = ShapeWidget (4, 4, 1152, 352, "shape");
		shapeGui[i].focusText = BWidgets::Text (0, 0, 400, 80, "label", focusString);
		shapeGui[i].toolSelect = SelectWidget (333, 368, 284, 44, "tool", 44, 44, 5, 1);
		shapeGui[i].drywetLabel = BWidgets::Label (500, 494, 50, 16, "smlabel", "dry/wet");
		shapeGui[i].drywetDial = BWidgets::DialValue (500, 434, 50, 60, "dial", 1.0, 0.0, 1.0, 0, "%1.2f");

		for (int j = 0; j < MAXOPTIONS; ++j)
		{
			switch (options[j].widgetType)
			{
				case NO_WIDGET:
					shapeGui[i].optionWidgets[j] = nullptr;
					break;

				case DIAL_WIDGET:
					shapeGui[i].optionWidgets[j] = new BWidgets::DialValue
					(
						0, 0, 50, 60, "dial",
						options[j].value, options[j].limit.min, options[j].limit.max, options[j].limit.step,
						options[j].format
					);
					if (!shapeGui[i].optionWidgets[j]) throw std::bad_alloc();
					shapeGui[i].optionWidgets[j]->setHardChangeable (false);
					break;

				default:
					shapeGui[i].optionWidgets[j] = nullptr;
					break;
			}
			shapeGui[i].optionLabels[j] = BWidgets::Label (220 + j * 70, 494, 60, 16, "smlabel", "");
			shapeGui[i].optionLabels[j].rename ("smlabel");
		}

		shapeGui[i].shapeContainer.rename ("widget");
		shapeGui[i].tabIcon.rename ("tab");
		shapeGui[i].targetListBox.rename ("menu2");
		shapeGui[i].shapeWidget.rename ("shape");
		shapeGui[i].focusText.rename ("label");
		shapeGui[i].toolSelect.rename ("tool");
		shapeGui[i].drywetLabel.rename ("smlabel");
		shapeGui[i].drywetDial.rename ("dial");
	}

	// Init main monitor
	initMonitors ();

	// Link controllers
	controllerWidgets.fill (nullptr);
	controllerWidgets[MIDI_CONTROL] = (BWidgets::ValueWidget*) &midiSwitch;
	controllerWidgets[BASE] = (BWidgets::ValueWidget*) &baseListBox;
	controllerWidgets[BASE_VALUE] = (BWidgets::ValueWidget*) &baseValueSelect;
	for (int i = 0; i < MAXSHAPES; ++i)
	{
		controllerWidgets[SHAPERS + i * SH_SIZE + SH_TARGET] = (BWidgets::ValueWidget*) &shapeGui[i].targetListBox;
		controllerWidgets[SHAPERS + i * SH_SIZE + SH_DRY_WET] = (BWidgets::ValueWidget*) &shapeGui[i].drywetDial;

		for (int j = 0 ; j < MAXOPTIONS; ++j)
		{
			controllerWidgets[SHAPERS + i * SH_SIZE + SH_OPTION + j] = (shapeGui[i].optionWidgets[j] ? shapeGui[i].optionWidgets[j] : nullptr);
		}
	}

	// Set callbacks
	midiSwitch.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BShaprGUI::valueChangedCallback);
	midiPiano.setCallbackFunction (BEvents::EventType::BUTTON_PRESS_EVENT, BShaprGUI::pianoCallback);
	midiPiano.setCallbackFunction (BEvents::EventType::BUTTON_RELEASE_EVENT, BShaprGUI::pianoCallback);
	midiPiano.setCallbackFunction (BEvents::EventType::POINTER_DRAG_EVENT, BShaprGUI::pianoCallback);
	baseValueSelect.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BShaprGUI::valueChangedCallback);
	baseListBox.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BShaprGUI::valueChangedCallback);
	monitorContainer.setCallbackFunction (BEvents::EventType::WHEEL_SCROLL_EVENT, BShaprGUI::wheelScrolledCallback);
	for (int i = 0; i < MAXSHAPES; ++i)
	{
		shapeGui[i].tabIcon.setCallbackFunction (BEvents::EventType::BUTTON_PRESS_EVENT, BShaprGUI::tabClickedCallback);
		shapeGui[i].targetListBox.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BShaprGUI::valueChangedCallback);
		shapeGui[i].drywetDial.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BShaprGUI::valueChangedCallback);
		shapeGui[i].shapeWidget.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BShaprGUI::shapeChangedCallback);
		shapeGui[i].toolSelect.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BShaprGUI::toolChangedCallback);

		for (int j = 0 ; j < MAXOPTIONS; ++j)
		{
			if (shapeGui[i].optionWidgets[j]) shapeGui[i].optionWidgets[j]->setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BShaprGUI::valueChangedCallback);
		}

		shapeGui[i].shapeWidget.setFocusable (true);
		BWidgets::FocusWidget* focus = new BWidgets::FocusWidget (this, "screen");
		if (!focus) throw std::bad_alloc ();
		shapeGui[i].shapeWidget.setFocusWidget (focus);
		focus->add (shapeGui[i].focusText);
		focus->resize ();
		shapeGui[i].shapeWidget.setMergeable (BEvents::POINTER_DRAG_EVENT, false);
	}

	// Configure widgets
	calculateXSteps ();
	mContainer.loadImage (BColors::NORMAL, pluginPath + BG_FILE);
	std::vector<bool> keys (12, true);
	midiPiano.setKeysToggleable (true);
	midiPiano.pressKeys (keys);
	midiPiano.hide();
	midiLabel.hide();
	monitorContainer.setScrollable (true);
	shapeGui[0].tabIcon.rename ("activetab");
	for (unsigned int i = 0; i < MAXSHAPES; ++i)
	{
		shapeGui[i].shapeWidget.setTool (ToolType::POINT_NODE_TOOL);
		shapeGui[i].shapeWidget.setLowerLimit (methods[0].limit.min);
		shapeGui[i].shapeWidget.setHigherLimit (methods[0].limit.max);
		shapeGui[i].drywetDial.setHardChangeable (false);

		for (int j = 0; j < MAXOPTIONS; ++j)
		{
			if (shapeGui[i].optionWidgets[j]) shapeGui[i].optionWidgets[j]->hide ();
			shapeGui[i].optionLabels[j].hide ();
		}

		if (i >= 1) shapeGui[i].shapeContainer.hide ();
	}
	applyChildThemes ();
	setKeyGrab (this);

	// Pack widgets
	monitorContainer.add (input1Monitor);
	monitorContainer.add (output1Monitor);
	monitorContainer.add (input2Monitor);
	monitorContainer.add (output2Monitor);
	monitorContainer.add (monitorHorizon1);
	monitorContainer.add (monitorHorizon2);
	mContainer.add (monitorContainer);
	for (unsigned int i = 0; i < MAXSHAPES; ++i)
	{
		mContainer.add (shapeGui[i].tabIcon);
		shapeGui[i].shapeContainer.add (shapeGui[i].targetListBox);
		shapeGui[i].shapeContainer.add (shapeGui[i].drywetLabel);
		shapeGui[i].shapeContainer.add (shapeGui[i].drywetDial);
		shapeGui[i].shapeContainer.add (shapeGui[i].shapeWidget);
		shapeGui[i].shapeContainer.add (shapeGui[i].toolSelect);

		for (int j = 0; j < MAXOPTIONS; ++j)
		{
			if (shapeGui[i].optionWidgets[j]) shapeGui[i].shapeContainer.add (*shapeGui[i].optionWidgets[j]);
			shapeGui[i].shapeContainer.add (shapeGui[i].optionLabels[j]);
		}

		mContainer.add (shapeGui[i].shapeContainer);
	}
	mContainer.add (midiSwitch);
	mContainer.add (midiPiano);
	mContainer.add (midiLabel);
	mContainer.add (messageLabel);
	mContainer.add (baseValueSelect);
	mContainer.add (baseListBox);
	add (mContainer);

	// Post addition configurations
	for (int i = 0; i < MAXSHAPES; ++i) shapeGui[i].shapeWidget.setDefaultShape(methods[0].defaultEndNode);
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
	for (int i = 0; i < MAXSHAPES; ++i)
	{
		BWidgets::FocusWidget* focus = shapeGui[i].shapeWidget.getFocusWidget();
		if (focus) delete focus;

		for (int j = 0; j < MAXOPTIONS; ++j)
		{
			if (shapeGui[i].optionWidgets[j]) delete shapeGui[i].optionWidgets[j];
		}
	}
	sendGuiOff ();
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
							std::pair<int, int> pos = translateNotification (notifications, notificationsCount);
							int p1 = LIMIT (pos.first, 0, MONITORBUFFERSIZE - 1);
							int p2 = LIMIT (pos.second, 0, MONITORBUFFERSIZE - 1);

							if (p1 <= p2) updateMonitors (p1, p2);
							else
							{
								updateMonitors (p1, MONITORBUFFERSIZE - 1);
								updateMonitors (0, p2);
							}
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
				lv2_atom_object_get(obj, urids.time_beatsPerBar, &oBpb, urids.time_beatUnit, &oBu, 0);
				if (oBpb && (oBpb->type == urids.atom_Float))
				{
					float value = ((LV2_Atom_Float*)oBpb)->body;
					if (value != 0.0f)
					{
						beatsPerBar = value;
						calculateXSteps ();
					}
				}

				if (oBu && (oBu->type == urids.atom_Int))
				{
					float value = ((LV2_Atom_Float*)oBu)->body;
					if (int (value) != 0)
					{
						beatUnit = value;
						calculateXSteps ();
					}
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
							for (unsigned int nodeNr = 0; (nodeNr < vecSize) && (nodeNr < MAXNODES); ++nodeNr)
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


	// Scan controller ports
	else if ((format == 0) && (port >= CONTROLLERS) && (port < CONTROLLERS + NR_CONTROLLERS))
	{
		// Calulate controller nr and validate controller value
		int controllerNr = port - CONTROLLERS;
		float* pval = (float*) buffer;

		if (controllerNr < SHAPERS)
		{
			float oldValue = controllers[controllerNr];
			float value = globalControllerLimits[controllerNr].validate (*pval);
			controllers[controllerNr] = value;

			// Active shape
			if (controllerNr == ACTIVE_SHAPE)
			{
				int sh = LIMIT (oldValue, 1, MAXSHAPES) - 1;
				shapeGui[sh].tabIcon.rename ("tab");
				shapeGui[sh].tabIcon.applyTheme (theme);
				shapeGui[sh].shapeContainer.hide();
				int nsh = value - 1;
				shapeGui[nsh].tabIcon.rename ("activetab");
				shapeGui[nsh].tabIcon.applyTheme (theme);
				shapeGui[nsh].shapeContainer.show();
			}

			// Keys
			else if (controllerNr == MIDI_KEYS)
			{
				uint32_t bits = value;
				std::vector<bool> keys (12, false);
				for (int i = 0; i < 12; ++i)
				{
					if (bits & (1 << i)) keys[i] = true;
				}
				midiPiano.pressKeys (keys);
			}

			// MIDI connected ?
			// Show or hide piano
			else if (controllerNr == MIDI_CONTROL)
			{
				if (controllerWidgets[MIDI_CONTROL]) controllerWidgets[MIDI_CONTROL]->setValue (value);

				if (value == 1.0f)
				{
					midiPiano.show ();
					midiLabel.show ();
				}
				else
				{
					midiPiano.hide ();
					midiLabel.hide ();
				}
			}

			// Base, base value
			// Update recalculate shaper grid steps
			else if ((controllerNr == BASE) || (controllerNr == BASE_VALUE))
			{
				if (controllerWidgets[controllerNr]) controllerWidgets[controllerNr]->setValue (value);
				calculateXSteps ();
			}
		}

		// Shapers
		else if ((controllerNr >= SHAPERS) && (controllerNr < SHAPERS + SH_SIZE * MAXSHAPES))
		{
			int shapeNr = (port - CONTROLLERS - SHAPERS) / SH_SIZE;
			int shapeWidgetNr = port - CONTROLLERS - SHAPERS - shapeNr * SH_SIZE;
			float value = shapeControllerLimits[shapeWidgetNr].validate (*pval);
			// controllers[controllerNr] = value;
			if (controllerWidgets[controllerNr]) controllerWidgets[controllerNr]->setValue (value);
		}
	}
}

void BShaprGUI::resizeGUI()
{
	hide ();

	// Resize Fonts
	defaultFont.setFontSize (12 * sz);
	smFont.setFontSize (10 * sz);
	lfLabelFont.setFontSize (12 * sz);

	// Resize widgets
	RESIZE (mContainer, 0, 0, 1200, 710, sz);
	RESIZE (messageLabel, 600, 45, 600, 20, sz);
	RESIZE (midiSwitch, 720, 580, 20, 40, sz);
	RESIZE (midiPiano, 900, 580, 140, 55, sz);
	RESIZE (midiLabel, 950, 565, 40, 10, sz);
	RESIZE (baseValueSelect, 480, 660, 100, 20, sz);
	RESIZE (baseListBox, 620, 660, 100, 20, sz);
	baseListBox.resizeListBox (100 * sz, 80 * sz);
	baseListBox.moveListBox (0, -80 * sz);
	RESIZE (monitorContainer, 24, 134, 1152, 352, sz);
	RESIZE (monitorHorizon1, monitorHorizon1.getX(), 0, 64, 352, sz);
	RESIZE (monitorHorizon2, monitorHorizon1.getX(), 0, 64, 352, sz);
	RESIZE (input1Monitor, 0, 0, 1152, 176, sz);
	RESIZE (output1Monitor, 0, 0, 1152, 176, sz);
	RESIZE (input2Monitor, 0, 176, 1152, 176, sz);
	RESIZE (output2Monitor, 0, 176, 1152, 176, sz);
	for (int i = 0; i < MAXSHAPES; ++i)
	{
		RESIZE (shapeGui[i].tabIcon, 20 + i * 120, 90, 119, 40, sz);
		RESIZE (shapeGui[i].shapeContainer, 20, 130, 1160, 590, sz);
		RESIZE (shapeGui[i].targetListBox, 20, 443, 174, 54, sz);
		shapeGui[i].targetListBox.resizeListBox (154 * sz, 380 * sz);
		shapeGui[i].targetListBox.moveListBox (0, -380 * sz);
		RESIZE (shapeGui[i].drywetLabel, 500, 494, 50, 16, sz);
		RESIZE (shapeGui[i].drywetDial, 500, 434, 50, 60, sz);
		RESIZE (shapeGui[i].shapeWidget, 4, 4, 1152, 352, sz);
		RESIZE (shapeGui[i].focusText, 0, 0, 400, 80, sz);
		shapeGui[i].shapeWidget.getFocusWidget()->resize();
		RESIZE (shapeGui[i].toolSelect, 333, 368, 284, 44, sz);
		shapeGui[i].toolSelect.resizeSelection (44 * sz, 44 * sz);

		int methodNr = shapeGui[i].targetListBox.getValue ();
		for (int j = 0; j < MAXOPTIONWIDGETS; ++j)
		{
			int optionNr = methods[methodNr].optionIndexes[j];
			if (optionNr != NO_OPTION)
			{
				if (shapeGui[i].optionWidgets[optionNr]) RESIZE ((*shapeGui[i].optionWidgets[optionNr]), 225 + j * 70, 434, 50, 60, sz);
				RESIZE (shapeGui[i].optionLabels[optionNr], 220 + j * 70, 494, 60, 16, sz);
			}
		}
	}

	// Update monitor, const std::string& name
	initMonitors ();
	updateMonitors (0, MONITORBUFFERSIZE - 1);

	// Apply changes
	applyChildThemes ();
	show ();
}

void BShaprGUI::applyChildThemes ()
{
	mContainer.applyTheme (theme);
	messageLabel.applyTheme (theme);
	midiSwitch.applyTheme (theme);
	midiPiano.applyTheme (theme);
	midiLabel.applyTheme (theme);
	baseValueSelect.applyTheme (theme);
	baseListBox.applyTheme (theme);
	monitorContainer.applyTheme (theme);
	input1Monitor.applyTheme (theme);
	output1Monitor.applyTheme (theme);
	input2Monitor.applyTheme (theme);
	output2Monitor.applyTheme (theme);
	for (unsigned int i = 0; i < MAXSHAPES; ++i)
	{
		shapeGui[i].shapeContainer.applyTheme (theme);
		shapeGui[i].tabIcon.applyTheme (theme);
		shapeGui[i].targetListBox.applyTheme (theme);
		shapeGui[i].drywetLabel.applyTheme (theme);
		shapeGui[i].drywetDial.applyTheme (theme);
		shapeGui[i].shapeWidget.applyTheme (theme);
		shapeGui[i].shapeWidget.getFocusWidget()->applyTheme (theme);
		shapeGui[i].focusText.applyTheme (theme);
		shapeGui[i].toolSelect.applyTheme (theme);

		int methodNr = shapeGui[i].targetListBox.getValue ();
		for (int j = 0; j < MAXOPTIONWIDGETS; ++j)
		{
			int optionNr = methods[methodNr].optionIndexes[j];
			if (optionNr != NO_OPTION)
			{
				if (shapeGui[i].optionWidgets[optionNr]) shapeGui[i].optionWidgets[optionNr]->applyTheme (theme);
				shapeGui[i].optionLabels[optionNr].applyTheme (theme);
			}
		}
	}
}

void BShaprGUI::onConfigure (BEvents::ExposeEvent* event)
{
	Window::onConfigure (event);

	sz = (width_ / 1200 > height_ / 710 ? height_ / 710 : width_ / 1200);
	resizeGUI ();
}

void BShaprGUI::onKeyPressed (BEvents::KeyEvent* event)
{
	if ((event) && (event->getKey() == BEvents::KEY_SHIFT))
	{
		for (int i = 0; i < MAXSHAPES; ++i)
		{
			shapeGui[i].shapeWidget.setScrollable (false);
		}
	}
}

void BShaprGUI::onKeyReleased (BEvents::KeyEvent* event)
{
	if ((event) && (event->getKey() == BEvents::KEY_SHIFT))
	{
		for (int i = 0; i < MAXSHAPES; ++i)
		{
			shapeGui[i].shapeWidget.setScrollable (true);
		}
	}
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
	for (unsigned int i = 0; i < size; ++i)
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
				if (widgetNr == MIDI_CONTROL)
				{
					if (value == 1.0f)
					{
						ui->midiPiano.show ();
						ui->midiLabel.show ();
					}
					else
					{
						ui->midiPiano.hide ();
						ui->midiLabel.hide ();
					}
				}

				else if (widgetNr >= SHAPERS)
				{
					int shapeNr = (widgetNr - SHAPERS) / SH_SIZE;
					int shapeWidgetNr = (widgetNr - SHAPERS) - shapeNr * SH_SIZE;

					// Target
					if (shapeWidgetNr == SH_TARGET)
					{
						int nr = LIMIT (value, 0, MAXEFFECTS - 1);

						// Set default end notes (if not manually set before)
						if
						(
							(ui->shapeGui[shapeNr].shapeWidget.isDefault ()) &&
							(ui->shapeGui[shapeNr].shapeWidget.getNode (0).point.y != methods[nr].defaultEndNode.point.y))
						{
							ui->shapeGui[shapeNr].shapeWidget.setDefaultShape (methods[nr].defaultEndNode);
						}

						// Set shapeWidget display parameters (limits, unit, prefix, ...)
						ui->shapeGui[shapeNr].shapeWidget.setScaleParameters
						(
							methods[nr].anchorYPos,
							methods[nr].anchorValue,
							methods[nr].ratio
						);
						ui->shapeGui[shapeNr].shapeWidget.setUnit (methods[nr].unit);
						ui->shapeGui[shapeNr].shapeWidget.setPrefix (methods[nr].prefix);
						ui->shapeGui[shapeNr].shapeWidget.setLowerLimit (methods[nr].limit.min);
						ui->shapeGui[shapeNr].shapeWidget.setHigherLimit (methods[nr].limit.max);

						// Hide old controllers
						int oldMethodNr = ui->controllers[widgetNr];
						for (int i = 0; i < MAXOPTIONWIDGETS; ++i)
						{
							int optionNr = methods[oldMethodNr].optionIndexes[i];
							if (optionNr != NO_OPTION)
							{
								if (ui->shapeGui[shapeNr].optionWidgets[optionNr]) ui->shapeGui[shapeNr].optionWidgets[optionNr]->hide ();
								ui->shapeGui[shapeNr].optionLabels[optionNr].hide ();
							}
						}

						// Configure and show new controllers
						int methodNr = value;
						for (int i = 0; i < MAXOPTIONWIDGETS; ++i)
						{
							int optionNr = methods[methodNr].optionIndexes[i];
							if (optionNr != NO_OPTION)
							{
								if (ui->shapeGui[shapeNr].optionWidgets[optionNr])
								{
									RESIZE ((*ui->shapeGui[shapeNr].optionWidgets[optionNr]), 225 + i * 70, 434, 50, 60, ui->sz);
									ui->shapeGui[shapeNr].optionWidgets[optionNr]->applyTheme (ui->theme);
									ui->shapeGui[shapeNr].optionWidgets[optionNr]->show ();
								}
								ui->shapeGui[shapeNr].optionLabels[optionNr].show ();
							}
						}
					}
				}

				ui->controllers[widgetNr] = value;
				ui->write_function(ui->controller, CONTROLLERS + widgetNr, sizeof(float), 0, &ui->controllers[widgetNr]);
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

			ui->monitorScale = ui->monitorScale * (1 + 0.01 * we->getDeltaY ());
			if (ui->monitorScale < 0.01) ui->monitorScale = 0.01;
			ui->input1Monitor.setZoom (ui->monitorScale);
			ui->output1Monitor.setZoom (ui->monitorScale);
			ui->input2Monitor.setZoom (ui->monitorScale);
			ui->output2Monitor.setZoom (ui->monitorScale);
		}
	}
}

void BShaprGUI::pianoCallback (BEvents::Event* event)
{
	if ((event) && (event->getWidget ()))
	{
		BWidgets::Widget* widget = event->getWidget ();

		if (widget->getMainWindow ())
		{
			BShaprGUI* ui = (BShaprGUI*) widget->getMainWindow ();

			std::vector<bool> keys = ui->midiPiano.getPressedKeys ();
			size_t sz = (keys.size () < 12 ? keys.size () : 12);
			uint32_t bits = 0;

			for (unsigned int i = 0; i < sz; ++i)
			{
				if (keys[i]) bits += (1 << i);
			}

			if (bits != uint32_t (ui->controllers[MIDI_KEYS]))
			{
				ui->controllers[MIDI_KEYS] = bits;
				ui->write_function(ui->controller, CONTROLLERS + MIDI_KEYS, sizeof(float), 0, &ui->controllers[MIDI_KEYS]);
			}
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

		case BEATS:	if (beatUnit != 0) minorXSteps = majorXSteps / (16.0 / ((double)beatUnit));
				else minorXSteps = majorXSteps / 4;
				break;

		case BARS:	if (beatsPerBar != 0.0f) minorXSteps = majorXSteps / beatsPerBar;
				else minorXSteps = majorXSteps / 4;
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

void BShaprGUI::initMonitors ()
{
	input1Monitor.clear ();
	output1Monitor.clear ();
	input2Monitor.clear ();
	output2Monitor.clear ();
	horizonPos = 0;
}

std::pair<int, int> BShaprGUI::translateNotification (BShaprNotifications* notifications, uint32_t notificationsCount)
{
	if (notificationsCount == 0) return std::make_pair (0, 0);

	int startpos = notifications[0].position;
	int monitorpos = startpos;
	for (unsigned int i = 0; i < notificationsCount; ++i)
	{
		monitorpos = LIMIT (notifications[i].position, 0, MONITORBUFFERSIZE - 1);

		input1Monitor.addData (monitorpos, notifications[i].input1);
		output1Monitor.addData (monitorpos, notifications[i].output1);
		input2Monitor.addData (monitorpos, notifications[i].input2);
		output2Monitor.addData (monitorpos, notifications[i].output2);

		horizonPos = double (monitorpos) / MONITORBUFFERSIZE;
	}
	return std::make_pair (startpos, monitorpos);
}

void BShaprGUI::updateMonitors (int start, int end)
{
	input1Monitor.redrawRange (start, end);
	output1Monitor.redrawRange (start, end);
	input2Monitor.redrawRange (start, end);
	output2Monitor.redrawRange (start, end);

	double width = monitorContainer.getEffectiveWidth ();
	monitorHorizon1.moveTo (horizonPos * width, 0);
	monitorHorizon2.moveTo ((horizonPos - 1) * width, 0);
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
	if ((screenWidth < 1240) || (screenHeight < 720)) sz = 0.66;
	if ((screenWidth < 840) || (screenHeight < 530)) sz = 0.50;

	if (resize) resize->ui_resize (resize->handle, 1200 * sz, 710 * sz);

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
