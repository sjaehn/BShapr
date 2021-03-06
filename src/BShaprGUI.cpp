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

#include "BShaprGUI.hpp"

BShaprGUI::BShaprGUI (const char *bundlePath, const LV2_Feature *const *features, PuglNativeView parentWindow) :
	Window (1200, 710, "B.Shapr", parentWindow, true, PUGL_MODULE, 0),
	controller (NULL), write_function (NULL),
	bpm (120), beatsPerBar (4.0), beatUnit (4),

	mContainer (0, 0, 1200, 710, "widget"),
	messageLabel (500, 45, 500, 20, "label", ""),
	bypassButton (1090, 15, 16, 16, "redbutton"),
	drywetDial (1144, 5, 33, 40, "dial", 1.0, 0.0, 1.0, 0.0, "%1.2f"),
	midiTriggerSwitch (760, 568, 30, 12, "dial", 0),
	midiPiano (760, 585, 150, 30, "widget", 0, 11),
	midiThruSwitch (1044, 575, 12, 30, "dial", 0),
	baseValueSelect (480, 660, 100, 20, "select", 1.0, 1.0, 16.0, 0.01),
	baseListBox (620, 660, 100, 20, 0, -80, 100, 80, "menu", BItems::ItemList ({{0, "Seconds"}, {1, "Beats"}, {2, "Bars"}})),
	monitorContainer (24, 134, 1152, 352, "monitor"),
	monitorHorizon1 (0, 0, 0, 64, 352, "horizon"),
	monitorHorizon2 (-1152, 0, 0, 64, 352, "horizon"),
	input1Monitor (0, 0, 1152, 176, "monitor.in"),
	output1Monitor (0, 0, 1152, 176, "monitor.out"),
	input2Monitor (0, 176, 1152, 176, "monitor.in"),
	output2Monitor (0, 176, 1152, 176, "monitor.out"),

	shapeBuffer {0.0},
	horizonPos (0), monitorScale (0.25), minorXSteps (1.0), majorXSteps (1.0),
	clipboard (),
	pluginPath (bundlePath ? std::string (bundlePath) : std::string ("")),
	sz (1.0),
	bgImageSurface (nullptr), forge (), urids (), map (NULL)

{
	// Init shapes
	for (int i = 0; i < MAXSHAPES; ++i)
	{
		shapeGui[i].tabContainer = BWidgets::Widget (20 + i * 148, 90, 147, 40, "tab");
		shapeGui[i].tabIcon = BWidgets::ImageIcon (0, 12.5, 120, 15, "widget", pluginPath + "inc/Shape" + std::to_string (i + 1) + ".png");
		for (int j = 0; j< NRSYMBOLS; ++j) shapeGui[i].tabSymbol[j] = SymbolWidget (120 + (j % 2) * 14 , 8 + int (j / 2) * 14, 10, 10, "symbol", SWSymbol(j));
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

			shapeGui[i].methodIcons.push_back (BWidgets::ImageIcon (0, 0, 154, 54, "icon", std::vector<std::string>{iconPath, iconPath}));

			BWidgets::ImageIcon* icon = &*std::prev (shapeGui[i].methodIcons.end ());
			cairo_t* cr = cairo_create (icon->getIconSurface (BColors::NORMAL));
			cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 0.5);
			cairo_paint (cr);
			cairo_destroy (cr);

			il.push_back (BItems::Item (index, icon));
		}

		shapeGui[i].targetListBox = BWidgets::PopupListBox (20, 443, 174, 54, 0, -380, 154, 380, "menu2", il, 0);

		shapeGui[i].shapeWidget = ShapeWidget (4, 4, 1152, 352, "shape");
		shapeGui[i].tabMsgBox = nullptr;
		shapeGui[i].tabMsgBoxBg = nullptr;
		shapeGui[i].smoothingLabel = BWidgets::Label (960, 410, 40, 10, "ssmlabel", "Smooth");
		shapeGui[i].smoothingDial = BWidgets::DialValue (960, 366, 40, 48, "dial", 20.0, 0.0, 100.0, 0, "%3.1f ms");
		shapeGui[i].toolSelect = SelectWidget (133, 368, 284, 44, "tool", 44, 44, 5, 2,
			{"Select", "Point node", "Auto Bezier node", "Symmetric Bezier node", "Asymmetric Bezier node"});
		for (int j = 0; j < 4; ++j) shapeGui[i].editWidgets[j] = EditWidget (463 + j * 60, 368, 44, 44, "widget", editWidgetLabels[j]);
		for (int j = 4; j < 7; ++j) shapeGui[i].editWidgets[j] = EditWidget (503 + j * 60, 368, 44, 44, "widget", editWidgetLabels[j]);
		shapeGui[i].gridSelect = SelectWidget (1043, 368, 104, 44, "tool", 44, 44, 2, 2, {"Show grid", "Snap to grid"});
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
						options[j].param.get<std::string>()
					);
					if (!shapeGui[i].optionWidgets[j]) throw std::bad_alloc();
					shapeGui[i].optionWidgets[j]->setHardChangeable (false);
					shapeGui[i].optionLabels[j] = BWidgets::Label (220 + j * 70, 494, 70, 16, "smlabel", options[j].name);
					break;

				case POPUP_WIDGET:
					{
						BItems::ItemList il = options[j].param.get<BItems::ItemList>();
						size_t max = 0;
						for (BItems::Item const& it : il)
						{
							if (it.getWidget())
							{
								BWidgets::Label* l = (BWidgets::Label*) it.getWidget();
								if (l->getText().size() > max) max = l->getText().size();
							}
						}
						int w = max * 9 + 20;
						w = LIMIT (w, 50, 270 - j * 70);
						int h = (il.size() + 1) * 20;
						h = LIMIT (h, 20, 400);
						shapeGui[i].optionWidgets[j] = new BWidgets::PopupListBox
						(
							0, 0, w, 20, 0, -h, w, h, "menu", il,
							options[j].value
						);
						if (!shapeGui[i].optionWidgets[j]) throw std::bad_alloc();
						shapeGui[i].optionLabels[j] = BWidgets::Label (220 + j * 70, 494, w, 16, "smlabel", options[j].name);
					}
					break;

				default:
					shapeGui[i].optionWidgets[j] = nullptr;
					break;
			}
		}
	}

	// Init main monitor
	initMonitors ();

	// Link controller widgets
	controllerWidgets.fill (nullptr);
	controllerWidgets[BYPASS] = (BWidgets::ValueWidget*) &bypassButton;
	controllerWidgets[DRY_WET] = (BWidgets::ValueWidget*) &drywetDial;
	controllerWidgets[MIDI_CONTROL] = (BWidgets::ValueWidget*) &midiTriggerSwitch;
	controllerWidgets[MIDI_THRU] = (BWidgets::ValueWidget*) &midiThruSwitch;
	controllerWidgets[BASE] = (BWidgets::ValueWidget*) &baseListBox;
	controllerWidgets[BASE_VALUE] = (BWidgets::ValueWidget*) &baseValueSelect;
	for (int i = 0; i < MAXSHAPES; ++i)
	{
		controllerWidgets[SHAPERS + i * SH_SIZE + SH_TARGET] = (BWidgets::ValueWidget*) &shapeGui[i].targetListBox;
		controllerWidgets[SHAPERS + i * SH_SIZE + SH_SMOOTHING] = (BWidgets::ValueWidget*) &shapeGui[i].smoothingDial;
		controllerWidgets[SHAPERS + i * SH_SIZE + SH_DRY_WET] = (BWidgets::ValueWidget*) &shapeGui[i].drywetDial;

		for (int j = 0 ; j < MAXOPTIONS; ++j)
		{
			controllerWidgets[SHAPERS + i * SH_SIZE + SH_OPTION + j] = (shapeGui[i].optionWidgets[j] ? shapeGui[i].optionWidgets[j] : nullptr);
		}
	}

	// Init controllers
	for (int i = 0; i < NR_CONTROLLERS; ++i) controllers[i] = (controllerWidgets[i] ? controllerWidgets[i]->getValue () : 0);

	// Set callbacks
	bypassButton.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BShaprGUI::valueChangedCallback);
	drywetDial.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BShaprGUI::valueChangedCallback);
	midiTriggerSwitch.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BShaprGUI::valueChangedCallback);
	midiPiano.setCallbackFunction (BEvents::EventType::BUTTON_PRESS_EVENT, BShaprGUI::pianoCallback);
	midiPiano.setCallbackFunction (BEvents::EventType::BUTTON_RELEASE_EVENT, BShaprGUI::pianoCallback);
	midiPiano.setCallbackFunction (BEvents::EventType::POINTER_DRAG_EVENT, BShaprGUI::pianoCallback);
	midiThruSwitch.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BShaprGUI::valueChangedCallback);
	baseValueSelect.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BShaprGUI::valueChangedCallback);
	baseListBox.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BShaprGUI::valueChangedCallback);
	monitorContainer.setCallbackFunction (BEvents::EventType::WHEEL_SCROLL_EVENT, BShaprGUI::wheelScrolledCallback);
	for (int i = 0; i < MAXSHAPES; ++i)
	{
		shapeGui[i].tabContainer.setCallbackFunction (BEvents::EventType::BUTTON_PRESS_EVENT, BShaprGUI::tabClickedCallback);
		shapeGui[i].tabIcon.setCallbackFunction (BEvents::EventType::BUTTON_PRESS_EVENT, BShaprGUI::tabClickedCallback);
		for (int j = 0; j< NRSYMBOLS; ++j) shapeGui[i].tabSymbol[j].setCallbackFunction (BEvents::EventType::BUTTON_PRESS_EVENT, BShaprGUI::tabClickedCallback);
		shapeGui[i].smoothingDial.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BShaprGUI::valueChangedCallback);
		shapeGui[i].targetListBox.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BShaprGUI::valueChangedCallback);
		shapeGui[i].drywetDial.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BShaprGUI::valueChangedCallback);
		shapeGui[i].shapeWidget.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BShaprGUI::shapeChangedCallback);
		shapeGui[i].toolSelect.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BShaprGUI::toolChangedCallback);
		for (int j = 0; j < 7; ++j) shapeGui[i].editWidgets[j].setCallbackFunction (BEvents::EventType::BUTTON_PRESS_EVENT, BShaprGUI::editClickedCallback);
		for (int j = 0; j < 7; ++j) shapeGui[i].editWidgets[j].setCallbackFunction (BEvents::EventType::BUTTON_RELEASE_EVENT, BShaprGUI::editReleasedCallback);
		shapeGui[i].gridSelect.setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BShaprGUI::gridChangedCallback);

		for (int j = 0 ; j < MAXOPTIONS; ++j)
		{
			if (shapeGui[i].optionWidgets[j]) shapeGui[i].optionWidgets[j]->setCallbackFunction (BEvents::EventType::VALUE_CHANGED_EVENT, BShaprGUI::valueChangedCallback);
		}

		shapeGui[i].shapeWidget.setMergeable (BEvents::POINTER_DRAG_EVENT, false);
	}

	// Configure widgets
	calculateXSteps ();
	mContainer.loadImage (BColors::NORMAL, pluginPath + BG_FILE);
	std::vector<bool> keys (12, true);
	drywetDial.setHardChangeable (false);
	midiPiano.setKeysToggleable (true);
	midiPiano.pressKeys (keys);
	midiPiano.hide();
	monitorContainer.setScrollable (true);
	input1Monitor.setScrollable (false);
	output1Monitor.setScrollable (false);
	input2Monitor.setScrollable (false);
	output2Monitor.setScrollable (false);
	shapeGui[0].tabContainer.rename ("activetab");
	for (unsigned int i = 0; i < MAXSHAPES; ++i)
	{
		for (int j = 0; j< NRSYMBOLS; ++j)
		{
			if (j != CLOSESYMBOL) shapeGui[i].tabSymbol[j].hide ();
		}
		shapeGui[i].shapeWidget.setTool (ToolType::POINT_NODE_TOOL);
		shapeGui[i].shapeWidget.setScaleParameters (methods[0].anchorYPos, methods[0].anchorValue, methods[0].ratio);
		shapeGui[i].shapeWidget.setLowerLimit (methods[0].limit.min);
		shapeGui[i].shapeWidget.setHigherLimit (methods[0].limit.max);
		shapeGui[i].shapeWidget.setTransformation (methods[0].transformFactor, methods[0].transformOffset);
		shapeGui[i].smoothingDial.setHardChangeable (false);
		shapeGui[i].drywetDial.setHardChangeable (false);

		for (int j = 0; j < MAXOPTIONS; ++j)
		{
			if (shapeGui[i].optionWidgets[j]) shapeGui[i].optionWidgets[j]->hide ();
			shapeGui[i].optionLabels[j].hide ();
		}

		shapeGui[i].shapeContainer.setScrollable (false);
		if (i >= 1) shapeGui[i].shapeContainer.hide ();
	}
	applyChildThemes ();
	getKeyGrabStack()->add (this);

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
		shapeGui[i].tabContainer.add (shapeGui[i].tabIcon);
		for (int j = 0; j< NRSYMBOLS; ++j) shapeGui[i].tabContainer.add (shapeGui[i].tabSymbol[j]);
		mContainer.add (shapeGui[i].tabContainer);
		shapeGui[i].shapeContainer.add (shapeGui[i].smoothingLabel);
		shapeGui[i].shapeContainer.add (shapeGui[i].smoothingDial);
		shapeGui[i].shapeContainer.add (shapeGui[i].drywetLabel);
		shapeGui[i].shapeContainer.add (shapeGui[i].drywetDial);
		shapeGui[i].shapeContainer.add (shapeGui[i].shapeWidget);
		shapeGui[i].shapeContainer.add (shapeGui[i].toolSelect);
		for (int j = 0; j < 7; ++j) shapeGui[i].shapeContainer.add (shapeGui[i].editWidgets[j]);
		shapeGui[i].shapeContainer.add (shapeGui[i].gridSelect);
		shapeGui[i].shapeContainer.add (shapeGui[i].targetListBox);

		for (int j = 0; j < MAXOPTIONS; ++j)
		{
			if (shapeGui[i].optionWidgets[j]) shapeGui[i].shapeContainer.add (*shapeGui[i].optionWidgets[j]);
			shapeGui[i].shapeContainer.add (shapeGui[i].optionLabels[j]);
		}

		mContainer.add (shapeGui[i].shapeContainer);
	}
	mContainer.add (bypassButton);
	mContainer.add (drywetDial);
	mContainer.add (midiTriggerSwitch);
	mContainer.add (midiPiano);
	mContainer.add (midiThruSwitch);
	mContainer.add (messageLabel);
	mContainer.add (baseValueSelect);
	mContainer.add (baseListBox);
	add (mContainer);

	// Post addition configurations
	for (int i = 0; i < MAXSHAPES; ++i)
	{
		shapeGui[i].shapeWidget.setDefaultShape();
		shapeGui[i].shapeWidget.setValueEnabled (true);
	}

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
		for (int j = 0; j < MAXOPTIONS; ++j)
		{
			if (shapeGui[i].optionWidgets[j]) delete shapeGui[i].optionWidgets[j];
		}

		if (shapeGui[i].tabMsgBox) delete shapeGui[i].tabMsgBox;
		if (shapeGui[i].tabMsgBoxBg) delete shapeGui[i].tabMsgBoxBg;
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

							if (p1 <= p2)
							{
								updateMonitors (p1, p2);
								updateHorizon ();
							}
							else
							{
								updateMonitors (p1, MONITORBUFFERSIZE - 1);
								updateMonitors (0, p2);
								updateHorizon ();
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
				const LV2_Atom *oBpb = NULL, *oBu = NULL, *oBpm = NULL;
				lv2_atom_object_get
				(
					obj,
					urids.time_beatsPerBar, &oBpb,
					urids.time_beatUnit, &oBu,
					urids.time_beatsPerMinute, &oBpm,
					0);
				if (oBpb && (oBpb->type == urids.atom_Float))
				{
					float value = ((LV2_Atom_Float*)oBpb)->body;
					if (value != 0.0f)
					{
						beatsPerBar = value;
						calculateXSteps ();
						updateHorizon ();
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

				if (oBpm && (oBpm->type == urids.atom_Float))
				{
					float value = ((LV2_Atom_Float*)oBpm)->body;
					if (value != 0.0f)
					{
						bpm = value;
						updateHorizon ();
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
								shapeGui[shapeNr].shapeWidget.appendRawNode (node);
							}
							shapeGui[shapeNr].shapeWidget.validateShape();
							shapeGui[shapeNr].shapeWidget.pushToSnapshots ();
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
				shapeGui[sh].tabContainer.rename ("tab");
				shapeGui[sh].tabContainer.applyTheme (theme);
				shapeGui[sh].shapeContainer.hide();
				int nsh = value - 1;
				shapeGui[nsh].tabContainer.rename ("activetab");
				shapeGui[nsh].tabContainer.applyTheme (theme);
				shapeGui[nsh].shapeContainer.show();
				updateHorizon ();
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

				if (value == 1.0f) midiPiano.show ();
				else midiPiano.hide ();
			}

			// Base, base value
			// Update recalculate shaper grid steps
			else if ((controllerNr == BASE) || (controllerNr == BASE_VALUE))
			{
				if (controllerWidgets[controllerNr]) controllerWidgets[controllerNr]->setValue (value);
			}
		}

		// Shapers
		else if ((controllerNr >= SHAPERS) && (controllerNr < SHAPERS + SH_SIZE * MAXSHAPES))
		{
			int shapeNr = (port - CONTROLLERS - SHAPERS) / SH_SIZE;
			int shapeWidgetNr = port - CONTROLLERS - SHAPERS - shapeNr * SH_SIZE;
			float value = shapeControllerLimits[shapeWidgetNr].validate (*pval);
			controllers[controllerNr] = value;
			if (controllerWidgets[controllerNr]) controllerWidgets[controllerNr]->setValue (value);

			// SH_OUTPUT
			if (shapeWidgetNr == SH_OUTPUT) updateTabs ();
		}
	}
}

void BShaprGUI::updateTabs ()
{
	int lastShape = 0;
	for (int i = 0; i < MAXSHAPES; ++i)
	{
		if (controllers[SHAPERS + i * SH_SIZE + SH_OUTPUT] == AUDIO_OUT) lastShape = i;
	}

	// Show or hide tabs
	for (int i = 0; i < MAXSHAPES; ++i)
	{
		if (i <= lastShape) shapeGui[i].tabContainer.show ();
		else shapeGui[i].tabContainer.hide ();
	}

	// Show or hide tabSymbols
	for (int i = 0; i < MAXSHAPES; ++i)
	{
		if (lastShape + 1 < MAXSHAPES) shapeGui[i].tabSymbol[ADDSYMBOL].show ();
		else shapeGui[i].tabSymbol[ADDSYMBOL].hide ();

		if ((i > 0) && (i <= lastShape)) shapeGui[i].tabSymbol[LEFTSYMBOL].show ();
		else shapeGui[i].tabSymbol[LEFTSYMBOL].hide ();

		if (i < lastShape) shapeGui[i].tabSymbol[RIGHTSYMBOL].show ();
		else shapeGui[i].tabSymbol[RIGHTSYMBOL].hide ();
	}
}

void BShaprGUI::resizeGUI(const double sz)
{
	hide ();

	// Resize Fonts
	defaultFont.setFontSize (12.0 * sz);
	smFont.setFontSize (10.0 * sz);
	ssmFont.setFontSize (8.0 * sz);
	lfLabelFont.setFontSize (12.0 * sz);

	// Resize widgets
	RESIZE (mContainer, 0, 0, 1200, 710, sz);
	RESIZE (messageLabel, 500, 45, 500, 20, sz);
	RESIZE (bypassButton, 1090, 15, 16, 16, sz);
	RESIZE (drywetDial, 1144, 5, 33, 40, sz);
	RESIZE (midiTriggerSwitch, 760, 568, 30, 12, sz);
	RESIZE (midiPiano, 760, 585, 150, 30, sz);
	RESIZE (midiThruSwitch, 1044, 575, 12, 30, sz);
	RESIZE (baseValueSelect, 480, 660, 100, 20, sz);
	RESIZE (baseListBox, 620, 660, 100, 20, sz);
	baseListBox.resizeListBox (BUtilities::Point (100 * sz, 80 * sz));
	baseListBox.moveListBox (BUtilities::Point (0, -80 * sz));
	RESIZE (monitorContainer, 24, 134, 1152, 352, sz);

	// monitorHorizon1/2
	monitorHorizon1.setFadeoutWidth (64 * sz);
	monitorHorizon2.setFadeoutWidth (64 * sz);
	monitorHorizon1.setHeight (352 * sz);
	monitorHorizon2.setHeight (352 * sz);

	RESIZE (input1Monitor, 0, 0, 1152, 176, sz);
	RESIZE (output1Monitor, 0, 0, 1152, 176, sz);
	RESIZE (input2Monitor, 0, 176, 1152, 176, sz);
	RESIZE (output2Monitor, 0, 176, 1152, 176, sz);
	for (int i = 0; i < MAXSHAPES; ++i)
	{
		RESIZE (shapeGui[i].tabContainer, 20 + i * 148, 90, 147, 40, sz);
		RESIZE (shapeGui[i].tabIcon, 0, 12.5, 120, 15, sz);
		for (int j = 0; j< NRSYMBOLS; ++j) RESIZE (shapeGui[i].tabSymbol[j], 120 + (j % 2) * 14 , 8 + int (j / 2) * 14, 10, 10, sz);
		if (shapeGui[i].tabMsgBox)
		{
			RESIZE
			(
				*shapeGui[i].tabMsgBox,
				shapeGui[i].tabMsgBox->getPosition().x / this->sz,
				shapeGui[i].tabMsgBox->getPosition().y / this->sz,
				shapeGui[i].tabMsgBox->getWidth () / this->sz,
				shapeGui[i].tabMsgBox->getHeight () / this->sz,
				sz
			);
		}
		if (shapeGui[i].tabMsgBoxBg) RESIZE (*shapeGui[i].tabMsgBoxBg, 0, 0, 1200, 710, sz);
		RESIZE (shapeGui[i].shapeContainer, 20, 130, 1160, 590, sz);
		RESIZE (shapeGui[i].smoothingLabel, 960, 410, 40, 10, sz);
		RESIZE (shapeGui[i].smoothingDial, 960, 366, 40, 48, sz);
		RESIZE (shapeGui[i].targetListBox, 20, 443, 174, 54, sz);
		shapeGui[i].targetListBox.resizeListBox (BUtilities::Point (174 * sz - 20, 380 * sz));
		shapeGui[i].targetListBox.moveListBox (BUtilities::Point (0, -380 * sz));
		shapeGui[i].targetListBox.resizeListBoxItems (BUtilities::Point (174 * sz - 20, 54 * sz));
		RESIZE (shapeGui[i].drywetLabel, 500, 494, 50, 16, sz);
		RESIZE (shapeGui[i].drywetDial, 500, 434, 50, 60, sz);
		RESIZE (shapeGui[i].shapeWidget, 4, 4, 1152, 352, sz);
		RESIZE (shapeGui[i].toolSelect, 133, 368, 284, 44, sz);
		shapeGui[i].toolSelect.resizeSelection (44 * sz, 44 * sz);
		for (int j = 0; j < 4; ++j) RESIZE (shapeGui[i].editWidgets[j], 463 + j * 60, 368, 44, 44, sz);
		for (int j = 4; j < 7; ++j) RESIZE (shapeGui[i].editWidgets[j], 503 + j * 60, 368, 44, 44, sz);
		RESIZE (shapeGui[i].gridSelect, 1043, 368, 104, 44, sz);
		shapeGui[i].gridSelect.resizeSelection (44 * sz, 44 * sz);

		int methodNr = shapeGui[i].targetListBox.getValue ();

		for (int j = 0; j < MAXOPTIONWIDGETS; ++j)
		{
			int optionNr = methods[methodNr].optionIndexes[j];
			if (optionNr != NO_OPT)
			{
				if (shapeGui[i].optionWidgets[optionNr])
				{
					switch (options[optionNr].widgetType)
					{
						case DIAL_WIDGET:
							RESIZE ((*shapeGui[i].optionWidgets[optionNr]), 230 + j * 70, 434, 50, 60, sz);
							RESIZE ((shapeGui[i].optionLabels[optionNr]), 220 + j * 70, 494, 70, 16, sz);
							break;

						case POPUP_WIDGET:
							{
								BItems::ItemList il = options[optionNr].param.get<BItems::ItemList>();
								size_t max = 0;
								for (BItems::Item const& it : il)
								{
									if (it.getWidget())
									{
										BWidgets::Label* l = (BWidgets::Label*) it.getWidget();
										if (l->getText().size() > max) max = l->getText().size();
									}
								}
								int w = max * 9 + 20;
								w = LIMIT (w, 50, 270 - j * 70);
								int h = (il.size() + 1) * 20;
								h = LIMIT (h, 20, 400);

								RESIZE ((*shapeGui[i].optionWidgets[optionNr]), 220 + j * 70, 455, w, 20, sz);
								((BWidgets::PopupListBox*) shapeGui[i].optionWidgets[optionNr])->resizeListBox (BUtilities::Point (w * sz, h * sz));
								((BWidgets::PopupListBox*) shapeGui[i].optionWidgets[optionNr])->moveListBox (BUtilities::Point (0, -h * sz));

								RESIZE ((shapeGui[i].optionLabels[optionNr]), 220 + j * 70, 494, w, 16, sz);
							}
							break;

						default:
							break;
					}
				}
			}
		}
	}

	this-> sz = sz;

	// Update monitor, const std::string& name
	initMonitors ();
	updateMonitors (0, MONITORBUFFERSIZE - 1);
	updateHorizon ();

	// Apply changes
	applyChildThemes ();
	show ();
}

void BShaprGUI::applyChildThemes ()
{
	mContainer.applyTheme (theme);
	messageLabel.applyTheme (theme);
	bypassButton.applyTheme (theme);
	drywetDial.applyTheme (theme);
	midiTriggerSwitch.applyTheme (theme);
	midiPiano.applyTheme (theme);
	midiThruSwitch.applyTheme (theme);
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
		shapeGui[i].tabContainer.applyTheme (theme);
		shapeGui[i].tabIcon.applyTheme (theme);
		for (int j = 0; j< NRSYMBOLS; ++j) shapeGui[i].tabSymbol[j].applyTheme (theme);
		if (shapeGui[i].tabMsgBox) shapeGui[i].tabMsgBox->applyTheme (theme);
		if (shapeGui[i].tabMsgBoxBg) shapeGui[i].tabMsgBoxBg->applyTheme (theme);
		shapeGui[i].smoothingLabel.applyTheme (theme);
		shapeGui[i].smoothingDial.applyTheme (theme);
		shapeGui[i].targetListBox.applyTheme (theme);
		shapeGui[i].drywetLabel.applyTheme (theme);
		shapeGui[i].drywetDial.applyTheme (theme);
		shapeGui[i].shapeWidget.applyTheme (theme);
		shapeGui[i].toolSelect.applyTheme (theme);
		for (int j = 0; j < 7; ++j) shapeGui[i].editWidgets[j].applyTheme (theme);
		shapeGui[i].gridSelect.applyTheme (theme);

		int methodNr = shapeGui[i].targetListBox.getValue ();
		for (int j = 0; j < MAXOPTIONWIDGETS; ++j)
		{
			int optionNr = methods[methodNr].optionIndexes[j];
			if (optionNr != NO_OPT)
			{
				if (shapeGui[i].optionWidgets[optionNr]) shapeGui[i].optionWidgets[optionNr]->applyTheme (theme);
				shapeGui[i].optionLabels[optionNr].applyTheme (theme);
			}
		}
	}
}

void BShaprGUI::onConfigureRequest (BEvents::ExposeEvent* event)
{
	Window::onConfigureRequest (event);

	resizeGUI (getWidth() / 1200 > getHeight() / 710 ? getHeight() / 710 : getWidth() / 1200);
}

void BShaprGUI::onCloseRequest (BEvents::WidgetEvent* event)
{
	if (event)
	{
		Widget* requestWidget = event->getRequestWidget ();
		if (requestWidget)
		{
			for (int i = 0; i < MAXSHAPES; ++i)
			{
				if (requestWidget == ((BWidgets::Widget*)shapeGui[i].tabMsgBox))
				{
					double value = shapeGui[i].tabMsgBox->getValue ();

					// Yes
					if (value == 2) deleteShape (i);

					delete shapeGui[i].tabMsgBox;
					shapeGui[i].tabMsgBox = nullptr;
					delete shapeGui[i].tabMsgBoxBg;
					shapeGui[i].tabMsgBoxBg = nullptr;

					return;
				}
			}

			Window::onCloseRequest (event);
		}
	}
}

void BShaprGUI::onKeyPressed (BEvents::KeyEvent* event)
{
	if ((event) && (event->getKey() == BDevices::KEY_SHIFT))
	{
		for (int i = 0; i < MAXSHAPES; ++i)
		{
			shapeGui[i].shapeWidget.setScrollable (false);
		}
	}
}

void BShaprGUI::onKeyReleased (BEvents::KeyEvent* event)
{
	if ((event) && (event->getKey() == BDevices::KEY_SHIFT))
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
		Node node = shapeGui[shapeNr].shapeWidget.getRawNode (i);
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



void BShaprGUI::setController (const int controllerNr, const float value)
{
	if ((controllerNr < 0) || (controllerNr >= NR_CONTROLLERS)) return;

	if (controllerWidgets[controllerNr]) controllerWidgets[controllerNr]->setValue (value);
	else
	{
		controllers[controllerNr] = value;
		write_function(controller, CONTROLLERS + controllerNr, sizeof(float), 0, &controllers[controllerNr]);
	}
}

void BShaprGUI::deleteShape (const int shapeNr)
{
	if ((shapeNr < 0) || (shapeNr >= MAXSHAPES)) return;

	// Scan SH_OUTPUT for the last shaper sending data to audio out
	// Limited backward compatibilty to v0.3.2 or older
	int lastShape = 0;
	for (int i = 0; i < MAXSHAPES; ++i)
	{
		if (controllers[SHAPERS + i * SH_SIZE + SH_OUTPUT] == AUDIO_OUT) lastShape = i;
	}

	// Deletion behind last shape (shouldn't happen)
	if (shapeNr > lastShape)
	{
		// Hide all tabContainers > lastShape
		for (int i = 0; i < MAXSHAPES; ++i)
		{
			if (i <= lastShape) shapeGui[i].tabContainer.show ();
			else shapeGui[i].tabContainer.hide ();
		}

		// Jump to last valid shape
		if (controllers[ACTIVE_SHAPE] - 1 > lastShape) switchShape (lastShape);

		// Get sure that SH_INPUT of shape 0 == AUDIO_IN
		if (controllers[SHAPERS + SH_INPUT] != AUDIO_IN) setController (SHAPERS + SH_INPUT, AUDIO_IN);
	}

	// Deletion of last shape
	else if (shapeNr == lastShape)
	{
		// shapeNr == 0: Reset shape
		if (shapeNr == 0)
		{
			// Reset shape 0 controllers
			setController (SHAPERS + SH_INPUT, AUDIO_IN);
			setController (SHAPERS + SH_INPUT_AMP, 1.0f);
			shapeGui[0].targetListBox.setValue (0.0);
			shapeGui[0].drywetDial.setValue (1.0);
			setController (SHAPERS + SH_OUTPUT, AUDIO_OUT);
			setController (SHAPERS + SH_OUTPUT_AMP, 1.0f);

			for (int j = 0; j < MAXOPTIONS; ++j)
			{
				if (shapeGui[0].optionWidgets[j]) shapeGui[0].optionWidgets[j]->setValue (options[j].value);
			}

			// Reset shape 0
			shapeGui[0].shapeWidget.setDefaultShape ();

			// Hide all tabContainers >= 1
			for (int i = 0; i < MAXSHAPES; ++i)
			{
				if (i == 0) shapeGui[i].tabContainer.show ();
				else shapeGui[i].tabContainer.hide ();
			}

			// Jump to shape 0
			switchShape (0);
		}

		// Otherwise: delete shapeNr
		else
		{
			// Link shapeNr - 1 to audio out; unlink shapeNr
			setController (SHAPERS + (shapeNr - 1) * SH_SIZE + SH_OUTPUT, AUDIO_OUT);
			setController (SHAPERS + shapeNr * SH_SIZE + SH_OUTPUT, INTERNAL);

			// Hide all tabContainers >= shapeNr
			for (int i = 0; i < MAXSHAPES; ++i)
			{
				if (i < shapeNr) shapeGui[i].tabContainer.show ();
				else shapeGui[i].tabContainer.hide ();
			}

			// Jump to last valid shape
			if (controllers[ACTIVE_SHAPE] - 1 > shapeNr - 1) switchShape (shapeNr - 1);

			// Get sure that SH_INPUT of shape 0 == AUDIO_IN
			if (controllers[SHAPERS + SH_INPUT] != AUDIO_IN) setController (SHAPERS + SH_INPUT, AUDIO_IN);
		}
	}

	// Deletion of start or middle shape
	else
	{
		// Shift shapers
		for (int i = shapeNr; i < lastShape; ++i)
		{
			// Copy controllers
			int destNr = SHAPERS + i * SH_SIZE;
			int srcNr = SHAPERS + (i + 1) * SH_SIZE;

			if (i == 0) setController (destNr + SH_INPUT, AUDIO_IN);
			else setController (destNr + SH_INPUT, OUTPUT + i - 1);

			setController (destNr + SH_INPUT_AMP, controllers[srcNr + SH_INPUT_AMP]);
			shapeGui[i].targetListBox.setValue (shapeGui[i + 1].targetListBox.getValue ());
			shapeGui[i].drywetDial.setValue (shapeGui[i + 1].drywetDial.getValue ());

			if (i == lastShape - 1) setController (destNr + SH_OUTPUT, AUDIO_OUT);
			else setController (destNr + SH_OUTPUT, controllers[srcNr + SH_OUTPUT]);

			setController (destNr + SH_OUTPUT_AMP, controllers[srcNr + SH_OUTPUT_AMP]);

			for (int j = 0; j < MAXOPTIONS; ++j)
			{
				if (shapeGui[i].optionWidgets[j] && shapeGui[i + 1].optionWidgets[j])
				{
					shapeGui[i].optionWidgets[j]->setValue (shapeGui[i + 1].optionWidgets[j]->getValue ());
				}
			}

			// Copy shapes
			shapeGui[i].shapeWidget = shapeGui [i + 1].shapeWidget;
		}

		// Unlink lastShape
		setController (SHAPERS + lastShape * SH_SIZE + SH_OUTPUT, INTERNAL);

		// Hide all tabContainers >= lastShape
		for (int i = 0; i < MAXSHAPES; ++i)
		{
			if (i < lastShape) shapeGui[i].tabContainer.show ();
			else shapeGui[i].tabContainer.hide ();
		}

		// Jump to last valid shape
		if (controllers[ACTIVE_SHAPE] - 1 > lastShape - 1) switchShape (lastShape - 1);

		// Get sure that SH_INPUT of shape 0 == AUDIO_IN
		if (controllers[SHAPERS + SH_INPUT] != AUDIO_IN) setController (SHAPERS + SH_INPUT, AUDIO_IN);
	}

	updateTabs ();
}

void BShaprGUI::insertShape (const int shapeNr)
{
	if ((shapeNr < 0) || (shapeNr >= MAXSHAPES - 1)) return;

	// Scan SH_OUTPUT for the last shaper sending data to audio out
	// Limited backward compatibilty to v0.3.2 or older
	int lastShape = 0;
	for (int i = 0; i < MAXSHAPES; ++i)
	{
		if (controllers[SHAPERS + i * SH_SIZE + SH_OUTPUT] == AUDIO_OUT) lastShape = i;
	}

	if (lastShape >= MAXSHAPES - 1) return;

	// Insertion just behind last shape
	if (shapeNr >= lastShape)
	{
		// Init shape lastShape + 1
		int destNr = SHAPERS + (lastShape + 1) * SH_SIZE;
		int srcNr = SHAPERS + lastShape * SH_SIZE;

		setController (destNr + SH_INPUT, OUTPUT + lastShape);
		setController (destNr + SH_INPUT_AMP, 1.0f);
		shapeGui[lastShape + 1].targetListBox.setValue (0.0);
		shapeGui[lastShape + 1].drywetDial.setValue (1.0);
		setController (destNr + SH_OUTPUT, AUDIO_OUT);
		setController (destNr + SH_OUTPUT_AMP, 1.0f);

		for (int j = 0; j < MAXOPTIONS; ++j)
		{
			if (shapeGui[lastShape + 1].optionWidgets[j]) shapeGui[lastShape + 1].optionWidgets[j]->setValue (options[j].value);
		}

		// Reset shape
		shapeGui[lastShape + 1].shapeWidget.setDefaultShape ();

		// Link lastShape
		setController (srcNr + SH_OUTPUT, INTERNAL);

		// Show lastShape + 1
		for (int i = 0; i < MAXSHAPES; ++i)
		{
			if (i <= lastShape + 1) shapeGui[i].tabContainer.show ();
			else shapeGui[i].tabContainer.hide ();
		}

		switchShape (lastShape + 1);
	}

	// Insertion in the middlde
	else
	{
		// Shift shapers
		for (int i = lastShape; i > shapeNr; --i)
		{
			int destNr = SHAPERS + (i + 1) * SH_SIZE;
			int srcNr = SHAPERS + i * SH_SIZE;

			setController (destNr + SH_INPUT, OUTPUT + i);
			setController (destNr + SH_INPUT_AMP, controllers[srcNr + SH_INPUT_AMP]);
			shapeGui[i + 1].targetListBox.setValue (shapeGui[i].targetListBox.getValue ());
			shapeGui[i + 1].drywetDial.setValue (shapeGui[i].drywetDial.getValue ());
			setController (destNr + SH_OUTPUT, controllers[srcNr + SH_OUTPUT]);
			setController (destNr + SH_OUTPUT_AMP, controllers[srcNr + SH_OUTPUT_AMP]);

			for (int j = 0; j < MAXOPTIONS; ++j)
			{
				if (shapeGui[i].optionWidgets[j] && shapeGui[i + 1].optionWidgets[j])
				{
					shapeGui[i + 1].optionWidgets[j]->setValue (shapeGui[i].optionWidgets[j]->getValue ());
				}
			}

			// Copy shapes
			shapeGui[i + 1].shapeWidget = shapeGui[i].shapeWidget;
		}

		// Init shape widgets shapeNr + 1
		int destNr = SHAPERS + (shapeNr + 1) * SH_SIZE;
		setController (destNr + SH_INPUT, OUTPUT + shapeNr);
		setController (destNr + SH_INPUT_AMP, 1.0f);
		shapeGui[shapeNr + 1].targetListBox.setValue (0.0f);
		shapeGui[shapeNr + 1].drywetDial.setValue (1.0f);
		setController (destNr + SH_OUTPUT, INTERNAL);
		setController (destNr + SH_OUTPUT_AMP, 1.0f);

		// Reset shape shapeNr + 1
		shapeGui[shapeNr + 1].shapeWidget.setDefaultShape ();

		// Show lastShape + 1
		for (int i = 0; i < MAXSHAPES; ++i)
		{
			if (i <= lastShape + 1) shapeGui[i].tabContainer.show ();
			else shapeGui[i].tabContainer.hide ();
		}

		switchShape (shapeNr + 1);
	}

	updateTabs ();
}

void BShaprGUI::swapShapes (const int source, const int dest)
{
	if ((source < 0) || (dest < 0)) return;

	// Scan SH_OUTPUT for the last shaper sending data to audio out
	// Limited backward compatibilty to v0.3.2 or older
	int lastShape = 0;
	for (int i = 0; i < MAXSHAPES; ++i)
	{
		if (controllers[SHAPERS + i * SH_SIZE + SH_OUTPUT] == AUDIO_OUT) lastShape = i;
	}

	if ((source > lastShape) || (dest > lastShape)) return;

	float value;
	int destNr = SHAPERS + dest * SH_SIZE;
	int srcNr = SHAPERS + source * SH_SIZE;

	value = controllers[destNr + SH_TARGET];
	shapeGui[dest].targetListBox.setValue (controllers[srcNr + SH_TARGET]);
	shapeGui[source].targetListBox.setValue (value);

	value = controllers[dest + SH_DRY_WET];
	shapeGui[dest].drywetDial.setValue (controllers[srcNr + SH_DRY_WET]);
	shapeGui[source].drywetDial.setValue (value);

	for (int j = 0; j < MAXOPTIONS; ++j)
	{
		if (shapeGui[source].optionWidgets[j] && shapeGui[dest].optionWidgets[j])
		{
			value = shapeGui[dest].optionWidgets[j]->getValue ();
			shapeGui[dest].optionWidgets[j]->setValue (shapeGui[source].optionWidgets[j]->getValue ());
			shapeGui[source].optionWidgets[j]->setValue (value);
		}
	}

	// Swap shapes
	ShapeWidget shBuffer;
	shBuffer = shapeGui[dest].shapeWidget;
	shapeGui[dest].shapeWidget = shapeGui[source].shapeWidget;
	shapeGui[source].shapeWidget = shBuffer;

	if (controllers[ACTIVE_SHAPE] - 1 == source) switchShape (dest);
	else if (controllers[ACTIVE_SHAPE] - 1 == dest) switchShape (source);
	updateTabs ();
}

void BShaprGUI::switchShape (const int shapeNr)
{
	if ((shapeNr < 0) || (shapeNr >= MAXSHAPES) || shapeNr == controllers[ACTIVE_SHAPE] - 1) return;

	int oldShapeNr = LIMIT (controllers[ACTIVE_SHAPE], 1, MAXSHAPES) - 1;
	shapeGui[oldShapeNr].tabContainer.rename ("tab");
	shapeGui[oldShapeNr].tabContainer.applyTheme (theme);
	shapeGui[oldShapeNr].shapeContainer.hide();
	setController (ACTIVE_SHAPE, shapeNr + 1);
	shapeGui[shapeNr].tabContainer.rename ("activetab");
	shapeGui[shapeNr].tabContainer.applyTheme (theme);
	shapeGui[shapeNr].shapeContainer.show();
	updateHorizon ();
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
					if (value == 1.0f) ui->midiPiano.show ();
					else ui->midiPiano.hide ();
				}

				else if ((widgetNr == BASE) || (widgetNr == BASE_VALUE))
				{
					ui->controllers[widgetNr] = value;
					ui->calculateXSteps ();
					ui->updateHorizon ();
				}

				else if (widgetNr >= SHAPERS)
				{
					int shapeNr = (widgetNr - SHAPERS) / SH_SIZE;
					int shapeWidgetNr = (widgetNr - SHAPERS) - shapeNr * SH_SIZE;

					// Target
					if (shapeWidgetNr == SH_TARGET)
					{
						int nr = LIMIT (value, 0, MAXEFFECTS - 1);

						// Change transformation
						ui->shapeGui[shapeNr].shapeWidget.setTransformation (methods[nr].transformFactor, methods[nr].transformOffset);

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
						ui->shapeGui[shapeNr].shapeWidget.update();

						// Hide old controllers
						int oldMethodNr = ui->controllers[widgetNr];
						for (int i = 0; i < MAXOPTIONWIDGETS; ++i)
						{
							int optionNr = methods[oldMethodNr].optionIndexes[i];
							if (optionNr != NO_OPT)
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

							if (optionNr != NO_OPT)
							{
								if (options[optionNr].widgetType != NO_WIDGET)
								{
									if (options[optionNr].widgetType == DIAL_WIDGET)
									{
										if (ui->shapeGui[shapeNr].optionWidgets[optionNr])
										{
											RESIZE ((*ui->shapeGui[shapeNr].optionWidgets[optionNr]), 230 + i * 70, 434, 50, 60, ui->sz);
											RESIZE ((ui->shapeGui[shapeNr].optionLabels[optionNr]), 220 + i * 70, 494, 70, 16, ui->sz);
										}
									}

									else if (options[optionNr].widgetType == POPUP_WIDGET)
									{
										if (ui->shapeGui[shapeNr].optionWidgets[optionNr])
										{
											BItems::ItemList il = options[optionNr].param.get<BItems::ItemList>();
											size_t max = 0;
											for (BItems::Item const& it : il)
											{
												if (it.getWidget())
												{
													BWidgets::Label* l = (BWidgets::Label*) it.getWidget();
													if (l->getText().size() > max) max = l->getText().size();
												}
											}
											int w = max * 9 + 20;
											w = LIMIT (w, 50, 270 - i * 70);
											int h = (il.size() + 1) * 20;
											h = LIMIT (h, 20, 400);

											RESIZE ((*ui->shapeGui[shapeNr].optionWidgets[optionNr]), 220 + i * 70, 455, w, 20, ui->sz);
											((BWidgets::PopupListBox*) ui->shapeGui[shapeNr].optionWidgets[optionNr])->resizeListBox (BUtilities::Point (w * ui->sz, h * ui->sz));
											((BWidgets::PopupListBox*) ui->shapeGui[shapeNr].optionWidgets[optionNr])->moveListBox (BUtilities::Point (0, -h * ui->sz));

											RESIZE ((ui->shapeGui[shapeNr].optionLabels[optionNr]), 220 + i * 70, 494, w, 16, ui->sz);
										}
									}

									// Show option controller
									if (ui->shapeGui[shapeNr].optionWidgets[optionNr])
									{
										ui->shapeGui[shapeNr].optionWidgets[optionNr]->applyTheme (ui->theme);
										ui->shapeGui[shapeNr].optionWidgets[optionNr]->show ();
									}

									// Show option label
									ui->shapeGui[shapeNr].optionLabels[optionNr].applyTheme (ui->theme);
									//RESIZE ((ui->shapeGui[shapeNr].optionLabels[optionNr]), 220 + i * 70, 494, 60, 16, ui->sz);
									ui->shapeGui[shapeNr].optionLabels[optionNr].show ();
								}
							}

						}
					}

					else if (shapeWidgetNr == SH_SMOOTHING)
					{
						if (shapeNr == ui->controllers[ACTIVE_SHAPE] - 1) ui->updateHorizon ();
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
		BWidgets::Widget* widget = (BWidgets::Widget*) event->getWidget ();
		if (widget->getMainWindow ())
		{
			BShaprGUI* ui = (BShaprGUI*) widget->getMainWindow ();
			for (int i = 0; i < MAXSHAPES; ++i)
			{
				if ((widget == &ui->shapeGui[i].tabIcon) || (widget == &ui->shapeGui[i].tabContainer))
				{
					ui->switchShape (i);
					break;
				}

				else if (widget == &ui->shapeGui[i].tabSymbol[CLOSESYMBOL])
				{
					if (ui->shapeGui[i].tabMsgBox) delete ui->shapeGui[i].tabMsgBox;
					ui->shapeGui[i].tabMsgBox = nullptr;
					if (ui->shapeGui[i].tabMsgBoxBg) delete ui->shapeGui[i].tabMsgBoxBg;
					ui->shapeGui[i].tabMsgBoxBg = nullptr;

					ui->shapeGui[i].tabMsgBox = new BWidgets::MessageBox (500 * ui->sz, 240 * ui->sz, 200 * ui->sz, 120 * ui->sz, "msgbox", "Delete shape " + std::to_string (i + 1), "Do you really want to delete this shape and all its content and settings ?", {"No", "Yes"});
					if (ui->shapeGui[i].tabMsgBox)
					{
						ui->shapeGui[i].tabMsgBox->applyTheme (ui->theme);

						ui->shapeGui[i].tabMsgBoxBg = new BWidgets::Widget (0, 0, 1200 * ui->sz, 710 * ui->sz, "widget");
						if (ui->shapeGui[i].tabMsgBoxBg)
						{
							ui->shapeGui[i].tabMsgBoxBg->applyTheme (ui->theme);
							ui->shapeGui[i].tabMsgBoxBg->add (*ui->shapeGui[i].tabMsgBox);
							ui->mContainer.add (*ui->shapeGui[i].tabMsgBoxBg);
						}
					}
					break;
				}

				else if (widget == &ui->shapeGui[i].tabSymbol[ADDSYMBOL])
				{
					ui->insertShape (i);
					break;
				}

				else if (widget == &ui->shapeGui[i].tabSymbol[LEFTSYMBOL])
				{
					ui->swapShapes (i, i - 1);
					break;
				}

				else if (widget == &ui->shapeGui[i].tabSymbol[RIGHTSYMBOL])
				{
					ui->swapShapes (i, i + 1);
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

void BShaprGUI::editClickedCallback (BEvents::Event* event)
{
	if ((event) && (event->getWidget ()))
	{
		BWidgets::Widget* widget = (BWidgets::Widget*) event->getWidget ();

		if (widget->getMainWindow ())
		{
			BShaprGUI* ui = (BShaprGUI*) widget->getMainWindow ();

			for (int i = 0; i < MAXSHAPES; ++i)
			{
				for (int j = 0; j < 7; ++j)
				{
					if (widget == &ui->shapeGui[i].editWidgets[j])
					{
						ui->shapeGui[i].editWidgets[j].rename ("frame");
						ui->shapeGui[i].editWidgets[j].applyTheme (ui->theme);

						switch (j)
						{
							case 0:		ui->clipboard = ui->shapeGui[i].shapeWidget.cutSelection ();
									return;

							case 1:		ui->clipboard = ui->shapeGui[i].shapeWidget.copySelection ();
									return;

							case 2:		ui->shapeGui[i].shapeWidget.pasteSelection (ui->clipboard);
									return;

							case 3:		ui->shapeGui[i].shapeWidget.deleteSelection ();
									return;

							case 4:		ui->shapeGui[i].shapeWidget.reset ();
									return;

							case 5:		ui->shapeGui[i].shapeWidget.undo ();
									return;

							case 6:		ui->shapeGui[i].shapeWidget.redo ();
									return;

							default:	return;
						}
					}
				}
			}
		}
	}
}

void BShaprGUI::editReleasedCallback (BEvents::Event* event)
{
	if ((event) && (event->getWidget ()))
	{
		BWidgets::Widget* widget = (BWidgets::Widget*) event->getWidget ();

		if (widget->getMainWindow ())
		{
			BShaprGUI* ui = (BShaprGUI*) widget->getMainWindow ();

			for (int i = 0; i < MAXSHAPES; ++i)
			{
				for (int j = 0; j < 7; ++j)
				{
					if (widget == &ui->shapeGui[i].editWidgets[j])
					{
						ui->shapeGui[i].editWidgets[j].rename ("widget");
						ui->shapeGui[i].editWidgets[j].applyTheme (ui->theme);
					}
				}
			}
		}
	}
}

void BShaprGUI::gridChangedCallback (BEvents::Event* event)
{
	if ((event) && (event->getWidget ()))
	{
		BWidgets::ValueWidget* widget = (BWidgets::ValueWidget*) event->getWidget ();

		if (widget->getMainWindow ())
		{
			BShaprGUI* ui = (BShaprGUI*) widget->getMainWindow ();

			for (int i = 0; i < MAXSHAPES; ++i)
			{
				if (widget == (BWidgets::ValueWidget*) &ui->shapeGui[i].gridSelect)
				{
					int value =  ui->shapeGui[i].gridSelect.getValue ();
					switch (value)
					{
						case 0: ui->shapeGui[i].shapeWidget.hideGrid ();
							ui->shapeGui[i].shapeWidget.setSnap (false);
							break;

						case 1: ui->shapeGui[i].shapeWidget.showGrid ();
							ui->shapeGui[i].shapeWidget.setSnap (false);
							break;

						case 2: ui->shapeGui[i].shapeWidget.showGrid ();
							ui->shapeGui[i].shapeWidget.setSnap (true);
							break;

						default:break;
					}
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

			ui->monitorScale = ui->monitorScale * (1 + 0.01 * we->getDelta().y);
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
	majorXSteps = (controllers[BASE_VALUE] != 0.0 ? 1.0 / controllers[BASE_VALUE] : 1.0);

	switch ((BShaprBaseIndex)controllers[BASE])
	{
		case SECONDS:	minorXSteps = majorXSteps / 10.0;
				break;

		case BEATS:	if (beatUnit != 0) minorXSteps = majorXSteps / (16.0 / ((double)beatUnit));
				else minorXSteps = majorXSteps / 4.0;
				break;

		case BARS:	if (beatsPerBar != 0.0f) minorXSteps = majorXSteps / beatsPerBar;
				else minorXSteps = majorXSteps / 4.0;
				break;

		default:	minorXSteps = 1.0;
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
}

void BShaprGUI::updateHorizon ()
{
	double width = monitorContainer.getEffectiveWidth ();
	int shapeNr = LIMIT (controllers[ACTIVE_SHAPE], 1, MAXSHAPES) - 1;
	double smTime = shapeGui[shapeNr].smoothingDial.getValue () / 1000;

	double smWidth;
	switch (int (controllers[BASE]))
	{
		case SECONDS :	smWidth = width * smTime / controllers[BASE_VALUE];
				break;
		case BEATS:	smWidth = width * smTime * (bpm / 60.0) / controllers[BASE_VALUE];
				break;
		case BARS:	smWidth = width * smTime * (bpm / 60.0 / beatsPerBar) / controllers[BASE_VALUE];
				break;
		default:	break;
	}

	monitorHorizon1.setSmoothingWidth (smWidth);
	monitorHorizon1.moveLineTo (horizonPos * width, 0);
	monitorHorizon2.setSmoothingWidth (smWidth);
	monitorHorizon2.moveLineTo ((horizonPos - 1) * width, 0);
}

static LV2UI_Handle instantiate (const LV2UI_Descriptor *descriptor, const char *plugin_uri, const char *bundle_path,
						  LV2UI_Write_Function write_function, LV2UI_Controller controller, LV2UI_Widget *widget,
						  const LV2_Feature *const *features)
{
	PuglNativeView parentWindow = 0;
	LV2UI_Resize* resize = NULL;

	if (strcmp(plugin_uri, BSHAPR_URI) != 0)
	{
		std::cerr << "BShapr.lv2#GUI: GUI does not support plugin with URI " << plugin_uri << std::endl;
		return NULL;
	}

	for (int i = 0; features[i]; ++i)
	{
		if (!strcmp(features[i]->URI, LV2_UI__parent)) parentWindow = (PuglNativeView) features[i]->data;
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

static void cleanup(LV2UI_Handle ui)
{
	BShaprGUI* pluginGui = (BShaprGUI*) ui;
	if (pluginGui) delete pluginGui;
}

static void portEvent(LV2UI_Handle ui, uint32_t port_index, uint32_t buffer_size,
	uint32_t format, const void* buffer)
{
	BShaprGUI* pluginGui = (BShaprGUI*) ui;
	if (pluginGui) pluginGui->portEvent(port_index, buffer_size, format, buffer);
}

static int callIdle (LV2UI_Handle ui)
{
	BShaprGUI* pluginGui = (BShaprGUI*) ui;
	if (pluginGui) pluginGui->handleEvents ();
	return 0;
}

static int callResize (LV2UI_Handle ui, int width, int height)
{
	BShaprGUI* self = (BShaprGUI*) ui;
	if (!self) return 0;
	
	BEvents::ExposeEvent* ev = new BEvents::ExposeEvent (self, self, BEvents::CONFIGURE_REQUEST_EVENT, self->getPosition().x, self->getPosition().y, width, height);
	self->addEventToQueue (ev);
	return 0;
}

static const LV2UI_Idle_Interface idle = {callIdle};
static const LV2UI_Resize resize = {nullptr, callResize};

static const void* extensionData(const char* uri)
{
	if (!strcmp(uri, LV2_UI__idleInterface)) return &idle;
	else if(!strcmp(uri, LV2_UI__resize)) return &resize;
	else return NULL;
}

static const LV2UI_Descriptor guiDescriptor = {
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
