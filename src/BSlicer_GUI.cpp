/* B.Slicer
 * Step Sequencer Effect Plugin
 *
 * Copyright (C) 2018 by Sven Jähnichen
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

#include <string>
#include <iostream>
#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#include <lv2/lv2plug.in/ns/extensions/ui/ui.h>
#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/atom/forge.h>
#include <lv2/lv2plug.in/ns/ext/time/time.h>
#include <gtkmm.h>
#include <cairomm/cairomm.h>
#include "main.h"

#define SCALEMIN -60
#define SCALEMAX 30
#define CAIRO_BG_COLOR 0.0, 0.0, 0.0, 1.0
#define CAIRO_BG_COLOR2 0.2, 0.2, 0.2, 1.0
#define CAIRO_FG_COLOR 1.0, 1.0, 1.0, 1.0
#define CAIRO_INK1 0.0, 1.0, 0.4
#define CAIRO_INK2 0.8, 0.6, 0.2

#define BG_COLOR {0, 0x0000, 0x0000, 0x0000}
#define WHITE {0, 0xFFFF, 0xFFFF, 0xFFFF}
#define FG_COLOR {0, 0x0000, 0xffff, 0x5fff}
#define FG_COLOR2 {0, 0xcfff, 0x9fff, 0x2fff}

#define BG_FILE "surface.jpeg"

#define DB_CO(g) ((g) > -90.0f ? powf(10.0f, (g) * 0.05f) : 0.0f)
#define CO_DB(g) ((g) > 0.0001f ? logf((g)) / 0.05f : -90.0f)
#define LIMIT(g , max) ((g) > (max) ? (max) : (g))
#define INT(g) (int) (g + 0.5)

class BSlicerScale: public Gtk::VScale
{
public:
	BSlicerScale () : VScale (0.0, 1.01, 0.01) {}
	virtual ~BSlicerScale () {}
};

class BSlicer_GUI: public Gtk::Fixed
{
public:
	BSlicer_GUI (const char *bundle_path, const LV2_Feature *const *features);
	virtual ~BSlicer_GUI ();
	void portEvent(LV2UI_Handle ui, uint32_t port_index, uint32_t buffer_size, uint32_t format, const void* buffer);

	LV2UI_Controller controller;
	LV2UI_Write_Function write_function;

protected:
	void monitorOnOff_changed ();
	void scale_changed ();
	void attack_changed ();
	void release_changed ();
	void stepsize_changed ();
	void nrSteps_changed ();
	void step_changed ();
	void value_changed_cb(/*GtkWidget* widget, void* data*/);
	bool stepshape_monitor_expose_event (GdkEventExpose* ev);
	bool main_monitor_expose_event (GdkEventExpose* ev);
	void send_record_on ();
	void send_record_off ();


private:
	void rearrange_step_controllers (float nrSteps_newf);
	void clear_monitor_data ();
	void add_monitor_data (BSlicerNotifications* notifications, uint32_t notificationsCount, uint32_t* end);
	void make_gui();

	Gtk::Image* bgImage;
	Gtk::HBox container5;
	Gtk::Label attackLabel;
	Gtk::Label releaseLabel;
	Gtk::Label stepsizeLabel;
	Gtk::Frame stepshapeDisplayFrame;
	Gtk::DrawingArea stepshapeDisplayBox;
	Gtk::Label stepControlLabel;
	Gtk::Label nrStepsLabel;
	Gtk::Label onLabel;
	Gtk::Label offLabel;
	Gtk::Label monitorLabel;
	Gtk::HScale monitorOnOffControl;
	Gtk::VScale scaleControl;
	Gtk::Frame monitorDisplayFrame;
	Gtk::DrawingArea monitorDisplayBox;
	Gtk::HScale nrStepsControl;
	Gtk::HScale attackControl;
	Gtk::HScale releaseControl;
	Gtk::HScale stepsizeControl;
	std::vector <BSlicerScale> stepControl;

	struct 	{
		bool record_on;
		uint32_t width;
		uint32_t height;
		BSlicerNotifications* data;
		uint32_t horizonPos;
	} mainMonitor;

	std::string pluginPath;

	float scale;
	float attack;
	float release;
	float nrSteps;
	float stepsize;
	std::vector<float> step;

	GdkColor gdkwhite = WHITE;
	GdkColor gdkfgColor = FG_COLOR;
	GdkColor gdkfgColor2 = FG_COLOR2;
	GdkColor gdkbgColor = BG_COLOR;
	Gdk::Color white = Gdk::Color (&gdkwhite, true);
	Gdk::Color fgColor = Gdk::Color (&gdkfgColor, true);
	Gdk::Color fgColor2 = Gdk::Color (&gdkfgColor2, true);
	Gdk::Color bgColor = Gdk::Color (&gdkbgColor, true);

	LV2_Atom_Forge forge;
	BSlicerURIs uris;
	LV2_URID_Map* map;

};

BSlicer_GUI::BSlicer_GUI (const char *bundle_path, const LV2_Feature *const *features) :
	pluginPath (bundle_path ? std::string (bundle_path) : std::string ("")), controller (NULL), write_function (NULL),
	bgImage (NULL), container5 (false, 2),
	onLabel ("on"), offLabel ("off"), monitorLabel ("Monitor"),  attackLabel ("Attack"), releaseLabel ("Release"),
	stepControlLabel ("Step control"), stepsizeLabel ("\u2669 = 1/x"), nrStepsLabel ("Number of steps"),
	stepshapeDisplayFrame ("Step shape"), stepshapeDisplayBox (), monitorDisplayFrame ("Monitor"), monitorDisplayBox (),
	monitorOnOffControl (0, 2, 1), scaleControl (SCALEMIN, SCALEMAX + 0.1, 0.1), nrStepsControl (1, MAXSTEPS + 1, 1),
	attackControl (0.0, 1.01, 0.01), releaseControl (0.0, 1.01, 0.01),
	stepsizeControl (1, 9, 1), stepControl (MAXSTEPS),
	scale (1.0f), attack (0.2f), release (0.2f), nrSteps (MAXSTEPS), stepsize (1), step (MAXSTEPS, 1.0f),
	map (NULL)

{
	//Initialize mainMonitor
	mainMonitor.record_on = true;
	mainMonitor.width = 0;
	mainMonitor.height = 0;
	mainMonitor.data = new BSlicerNotifications[MONITORBUFFERSIZE];
	clear_monitor_data();
	mainMonitor.horizonPos = 0;

	//Scan host features for URID map
	LV2_URID_Map* m = NULL;
	for (int i = 0; features[i]; ++i)
	{
		if (strcmp(features[i]->URI, LV2_URID__map) == 0)
		{
			m = (LV2_URID_Map*) features[i]->data;
		}
	}
	if (!m)
	{
		std::cerr << "BSlicer.lv2#GUI: Host does not support urid:map.\n";
		return;
	}

	//Map URIS
	map = m;
	getURIs (map, &uris);

	// Initialize forge
	lv2_atom_forge_init (&forge,map);

	//Initialize GUI
	make_gui();
}

BSlicer_GUI::~BSlicer_GUI()
{
	send_record_off ();
	delete bgImage;
	delete mainMonitor.data;
}

void BSlicer_GUI::portEvent(LV2UI_Handle ui, uint32_t port_index, uint32_t buffer_size,
	uint32_t format, const void* buffer)
{

	// Notify port
	if ((format == uris.atom_eventTransfer) && (port_index == Notify))
	{
		const LV2_Atom* atom = (const LV2_Atom*) buffer;
		if ((atom->type == uris.atom_Blank) || (atom->type == uris.atom_Object))
		{
			const LV2_Atom_Object* obj = (const LV2_Atom_Object*) atom;
			if (obj->body.otype == uris.notify_event)
			{
				const LV2_Atom* data = NULL;
				lv2_atom_object_get(obj, uris.notify_key, &data, 0);
				if (data && (data->type == uris.atom_Vector))
				{
					const LV2_Atom_Vector* vec = (const LV2_Atom_Vector*) data;
					if (vec->body.child_type == uris.atom_Float)
					{
						uint32_t notificationsCount = (uint32_t) ((data->size - sizeof(LV2_Atom_Vector_Body)) / sizeof (BSlicerNotifications));
						BSlicerNotifications* notifications = (BSlicerNotifications*) (&vec->body + 1);
						if (notificationsCount > 0)
						{
							add_monitor_data (notifications, notificationsCount, &mainMonitor.horizonPos);
							monitorDisplayBox.queue_draw ();
						}
					}
				}
				else fprintf(stderr, "BSlicer.lv2#GUI: Corrupt audio message\n");
			}
		}
	}

	// Scan remaining ports
	else if ((format == 0) && (port_index >= Attack) && (port_index < Step_ + MAXSTEPS))
	{
	float* pval = (float*) buffer;
	switch (port_index) {
		case Attack:
			attackControl.set_value (*pval);
			stepshapeDisplayBox.queue_draw ();
			break;
		case Release:
			releaseControl.set_value (*pval);
			stepshapeDisplayBox.queue_draw ();
			break;
		case Stepsize:
			stepsizeControl.set_value (*pval);
		break;
		case NrSteps:
			if (nrSteps != *pval)
			{
				rearrange_step_controllers (*pval);
				nrSteps = *pval;
			}
			monitorDisplayBox.queue_draw ();
			nrStepsControl.set_value (*pval);
			break;
		default:
			if ((port_index >= Step_) and (port_index < Step_ + MAXSTEPS))
			{
				stepControl[port_index-Step_].set_value (*pval);
			}
		}
	}
}


void BSlicer_GUI::send_record_on ()
{
	uint8_t obj_buf[64];
	lv2_atom_forge_set_buffer(&forge, obj_buf, sizeof(obj_buf));

	LV2_Atom_Forge_Frame frame;
	LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object(&forge, &frame, 0, uris.ui_on);
	lv2_atom_forge_pop(&forge, &frame);
	write_function(controller, Control_2, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
}

void BSlicer_GUI::send_record_off ()
{
	uint8_t obj_buf[64];
	lv2_atom_forge_set_buffer(&forge, obj_buf, sizeof(obj_buf));

	LV2_Atom_Forge_Frame frame;
	LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object(&forge, &frame, 0, uris.ui_off);
	lv2_atom_forge_pop(&forge, &frame);
	write_function(controller, Control_2, lv2_atom_total_size(msg), uris.atom_eventTransfer, msg);
}

void BSlicer_GUI::rearrange_step_controllers (float nrSteps_newf)
{
	int nrSteps_old = INT (nrSteps);
	int nrSteps_new = INT (nrSteps_newf);

	if ((nrSteps_old < 1) || (nrSteps_old > MAXSTEPS) || (nrSteps_new < 1) || (nrSteps_old > MAXSTEPS)) return;

	if (nrSteps_new > nrSteps_old)
	{
		for (int i = nrSteps_old; i < nrSteps_new; i++)
		{
			stepControl[i].show ();
		}
	}
	else if (nrSteps_new < nrSteps_old)
	{
		for (int i = nrSteps_new; i < nrSteps_old; i++)
		{
			stepControl[i].hide ();
		}
	}
}

void BSlicer_GUI::monitorOnOff_changed ()
{
	int value = INT (monitorOnOffControl.get_value ());
	monitorOnOffControl.set_value (value);
	if (value == 1)
	{
		mainMonitor.record_on = true;
		send_record_on ();
	}
	else
	{
		mainMonitor.record_on = false;
		send_record_off ();
	}
	return;
}

void BSlicer_GUI::scale_changed ()
{
	float value = (float) scaleControl.get_value ();
	scale = DB_CO (value);
	if (scale < 0.0001f) scale = 0.0001f;
	monitorDisplayBox.queue_draw ();
}

void BSlicer_GUI::attack_changed ()
{
	attack = (float) attackControl.get_value ();
	write_function(controller, Attack, sizeof(attack), 0, &attack);
	stepshapeDisplayBox.queue_draw ();
	return;
}

void BSlicer_GUI::release_changed ()
{
	release = (float) releaseControl.get_value ();
	write_function(controller, Release, sizeof(release), 0, &release);
	stepshapeDisplayBox.queue_draw ();
	return;
}

void BSlicer_GUI::stepsize_changed ()
{
	stepsize = (float) stepsizeControl.get_value ();
	write_function(controller, Stepsize, sizeof(stepsize), 0, &stepsize);
	return;
}

void BSlicer_GUI::nrSteps_changed ()
{
	float nrSteps_new = (float) nrStepsControl.get_value ();
	if (nrSteps_new != nrSteps)
	{
		rearrange_step_controllers (nrSteps_new);
		nrSteps = nrSteps_new;
	}
	write_function(controller, NrSteps, sizeof(nrSteps), 0, &nrSteps);
	monitorDisplayBox.queue_draw ();
	return;
}

void BSlicer_GUI::step_changed ()
{
	int maxsteps = INT (nrSteps);
	for (int i = 0; i < maxsteps; i++)
	{
		float step_new = (float) stepControl[i].get_value ();
		if (step_new != step[i])
		{
			step[i] = step_new;
			write_function(controller, Step_ + i , sizeof(step[i]), 0, &step[i]);
		}
	}
	return;
}


void BSlicer_GUI::clear_monitor_data ()
{
	for (int i = 0; i < MONITORBUFFERSIZE; i++) mainMonitor.data[i] = endNote;
}

void BSlicer_GUI::add_monitor_data (BSlicerNotifications* notifications, uint32_t notificationsCount, uint32_t* end)
{
	int offset = *end + 1;
	int i = 0;
	while ((i < notificationsCount) && (notifications[i].position >= 0.0f))
	{
		if ((offset + i) >= MONITORBUFFERSIZE) offset = -i;
		mainMonitor.data[offset + i] = notifications[i];
		*end = (uint32_t) (offset + i);
		i++;
	}
}

bool BSlicer_GUI::stepshape_monitor_expose_event (GdkEventExpose* ev)
{
	const uint32_t width = ev->area.width;
	const uint32_t height = ev->area.height;

	Cairo::RefPtr<Cairo::Context> cr = stepshapeDisplayBox.get_window()->create_cairo_context();

	// Draw background
	cr->set_source_rgba (CAIRO_BG_COLOR);
	cr->rectangle (0.0, 0.0, width, height);
	cr->fill ();
	cr->set_source_rgba (CAIRO_BG_COLOR2);
	cr->set_line_width (1);
	cr->move_to (0, 0.1 * height);
	cr->line_to (width, 0.1 * height);
	cr->move_to (0, 0.5 * height);
	cr->line_to (width, 0.5 * height);
	cr->move_to (0, 0.9 * height);
	cr->line_to (width, 0.9 * height);
	cr->move_to (0.25 * width, 0);
	cr->line_to (0.25 * width, height);
	cr->move_to (0.5 * width, 0);
	cr->line_to (0.5 * width, height);
	cr->move_to (0.75 * width, 0);
	cr->line_to (0.75 * width, height);
	cr->stroke ();

	// Draw step shape
	cr->set_source_rgba (CAIRO_INK1, 1.0);
	cr->set_line_width (3);

	cr->move_to (0, 0.9 * height);
	cr->line_to (width * 0.25, 0.9 * height);
	if ((attack + release) > 1)
	{
		float crosspointX = attack / (attack + release);
		float crosspointY = crosspointX / attack - (crosspointX - (1 - release)) / release;
		cr->line_to (width* 0.25 + crosspointX * width * 0.5, 0.9 * height - 0.8 * height * crosspointY);
	}
	else
	{
		cr->line_to (width * 0.25 + attack * width * 0.5 , 0.1 * height);
		cr->line_to (width * 0.75  - release * width * 0.5, 0.1 * height);

	}
	cr->line_to (width * 0.75, 0.9 * height);
	cr->line_to (width, 0.9 * height);

	cr->stroke_preserve ();

	Cairo::RefPtr<Cairo::LinearGradient> pat = Cairo::LinearGradient::create (0, 0, 0, height);
	pat->add_color_stop_rgba (0.1, CAIRO_INK1, 1);
	pat->add_color_stop_rgba (0.9, CAIRO_INK1, 0);
	cr->set_source (pat);
	cr->line_to(0, 0.9 * height);
	cr->set_line_width (0);
	cr->fill ();

	return TRUE;
}

bool BSlicer_GUI::main_monitor_expose_event (GdkEventExpose* ev)
{
	const uint32_t width = ev->area.width;
	const uint32_t height = ev->area.height;
	uint32_t i;
	bool lineBreak;
	float pos, nextpos, linebreakpos;

	Cairo::RefPtr<Cairo::Context> cr = monitorDisplayBox.get_window()->create_cairo_context();
	Cairo::RefPtr<Cairo::Context> cr2 = monitorDisplayBox.get_window()->create_cairo_context();
	Cairo::RefPtr<Cairo::LinearGradient> pat = Cairo::LinearGradient::create (0, 0, 0, height);
	Cairo::RefPtr<Cairo::LinearGradient> pat2 = Cairo::LinearGradient::create (0, 0, 0, height);
	pat->add_color_stop_rgba (0.1, CAIRO_INK1, 1);
	pat->add_color_stop_rgba (0.9, CAIRO_INK1, 0);
	pat2->add_color_stop_rgba (0.1, CAIRO_INK2, 1);
	pat2->add_color_stop_rgba (0.9, CAIRO_INK2, 0);

	// Draw background
	cr->set_source_rgba (CAIRO_BG_COLOR);
	cr->rectangle (0, 0, width, height);
	cr->fill ();

	cr->set_source_rgba (CAIRO_BG_COLOR2);
	cr->set_line_width (1);
	cr->move_to (0, 0.1 * height);
	cr->line_to (width, 0.1 * height);
	cr->move_to (0, 0.9 * height);
	cr->line_to (width, 0.9 * height);

	i = 1;
	uint32_t steps = (uint32_t) nrSteps;
	while (i < steps)
	{
		uint32_t x = (uint32_t) (i * width / steps);
		cr->move_to (x, 0);
		cr->line_to (x, height);
		i++;
	}
	cr->stroke ();

	if (mainMonitor.record_on)
	{
		uint32_t horizoni = mainMonitor.horizonPos;
		float horizonpos = mainMonitor.data[horizoni].position;
		if (horizonpos >= 0.0f)
		{
			// Draw horizon line
			cr->set_source_rgba (CAIRO_FG_COLOR);
			cr->set_line_width (1);
			cr->move_to ((uint32_t) (horizonpos * width), 0);
			cr->line_to ((uint32_t) (horizonpos * width), height);
			cr->stroke ();

			// Draw data: input@cr, output@cr2
			cr->move_to ((uint32_t) (horizonpos * width), (uint32_t) (height * (0.9  - (0.8 * LIMIT ((mainMonitor.data[horizoni].input / scale), 1.0f)))));
			cr2->move_to ((uint32_t) (horizonpos * width), (uint32_t) (height * (0.9  - (0.8 * LIMIT ((mainMonitor.data[horizoni].output / scale), 1.0f)))));
			lineBreak = false;
			i = horizoni;
			nextpos = horizonpos;
			linebreakpos = horizonpos;

			do
			{
				pos = nextpos;
				cr->line_to ((uint32_t) (pos * width), (uint32_t) (height * (0.9  - (0.8 * LIMIT ((mainMonitor.data[i].input / scale), 1.0f)))));
				cr2->line_to ((uint32_t) (pos * width), (uint32_t) (height * (0.9  - (0.8 * LIMIT ((mainMonitor.data[i].output / scale), 1.0f)))));
				if (i == 0) i = MONITORBUFFERSIZE - 1;
				else i--;
				nextpos = mainMonitor.data[i].position;

				// Line break in raw data?
				if (nextpos > pos)
				{
					// Visualize input (cr) and output (cr2) curves between horizon and breakpoint
					cr->set_source_rgba (CAIRO_INK1, 1.0);
					cr->set_line_width (3);
					cr2->set_source_rgba (CAIRO_INK2, 1.0);
					cr2->set_line_width (3);
					cr->stroke_preserve ();
					cr2->stroke_preserve ();

					// Visualize input (cr) and output (cr2) areas under the curves between horizon and breakpoint
					cr->line_to ((uint32_t) (pos * width), (uint32_t) (height * 0.9));
					cr->line_to ((uint32_t) (horizonpos * width), (uint32_t) (height * 0.9));
					cr->close_path ();
					cr2->line_to ((uint32_t) (pos * width), (uint32_t) (height * 0.9));
					cr2->line_to ((uint32_t) (horizonpos * width), (uint32_t) (height * 0.9));
					cr2->close_path ();
					cr->set_source (pat);
					cr->set_line_width (0);
					cr2->set_source (pat2);
					cr2->set_line_width (0);
					cr->fill ();
					cr2->fill ();

					lineBreak = true;
					linebreakpos = nextpos;
					cr->move_to ((uint32_t) (nextpos * width), (uint32_t) (height * (0.9  - (0.8 * LIMIT ((mainMonitor.data[i].input / scale), 1.0f)))));
					cr2->move_to ((uint32_t) (nextpos * width), (uint32_t) (height * (0.9  - (0.8 * LIMIT ((mainMonitor.data[i].output / scale), 1.0f)))));
				}
			} while ((nextpos >= 0.0f) && (((nextpos < horizonpos) && !lineBreak) || ((nextpos > horizonpos) && lineBreak)));

			// Visualize (remaining parts of) input (cr) and output (cr2) curves
			cr->set_source_rgba (CAIRO_INK1, 1.0);
			cr->set_line_width (3);
			cr2->set_source_rgba (CAIRO_INK2, 1.0);
			cr2->set_line_width (3);


			if (nextpos > pos)
			{
				cr->stroke ();
				cr2->stroke ();
				// Do not visualize areas under the curves if do while loop ended exactly on line break
			}
			else
			{
				cr->stroke_preserve ();
				cr2->stroke_preserve ();

				// Visualize (remaining parts of) input (cr) and output (cr2) areas under the curves
				cr->line_to ((uint32_t) (pos * width), (uint32_t) (height * 0.9));
				cr->line_to ((uint32_t) (linebreakpos * width), (uint32_t) (height * 0.9));
				cr->close_path ();
				cr2->line_to ((uint32_t) (pos * width), (uint32_t) (height * 0.9));
				cr2->line_to ((uint32_t) (linebreakpos * width), (uint32_t) (height * 0.9));
				cr2->close_path ();
				cr->set_source (pat);
				cr->set_line_width (0);
				cr2->set_source (pat2);
				cr2->set_line_width (0);
				cr->fill ();
				cr2->fill ();
			}
		}
	}
	return TRUE;
}

void BSlicer_GUI::make_gui()
{
	try
	{
		bgImage = new Gtk::Image (pluginPath + BG_FILE);
	}
	catch (const std::bad_alloc &allocexception)
	{
		std::cerr << "BSlicer.lv2#GUI: Memory allocation failed: " << allocexception.what() << std::endl;
	}

	// Main container
	set_size_request(800, 560);

	// Background image
	if (bgImage)
	{
		bgImage->set_size_request(800,560);
		put(*bgImage, 0, 0);
	}

	// Monitor on/off switch
	offLabel.set_size_request(40,20);
	offLabel.modify_fg (Gtk::STATE_NORMAL, white);
	put (offLabel, 640, 30);

	monitorOnOffControl.set_value (1);
	monitorOnOffControl.set_draw_value(false);
	monitorOnOffControl.set_size_request (60,20);
	monitorOnOffControl.modify_fg (Gtk::STATE_NORMAL, white);
	monitorOnOffControl.modify_bg (Gtk::STATE_SELECTED, fgColor2);
	monitorOnOffControl.signal_value_changed().connect (sigc::mem_fun (*this, &BSlicer_GUI::monitorOnOff_changed));
	put (monitorOnOffControl, 680, 30);

	onLabel.set_size_request (40,20);
	onLabel.modify_fg (Gtk::STATE_NORMAL, white);
	put (onLabel, 740, 30);

	monitorLabel.set_size_request (60,20);
	monitorLabel.modify_fg (Gtk::STATE_NORMAL, white);
	put (monitorLabel, 680, 50);

	// Scale control
	scaleControl.set_inverted (true);
	scaleControl.set_value (CO_DB (scale));
	scaleControl.set_draw_value (false);
	scaleControl.set_size_request (20, 230);
	scaleControl.modify_fg (Gtk::STATE_NORMAL, white);
	scaleControl.modify_bg (Gtk::STATE_SELECTED, fgColor2);
	scaleControl.signal_value_changed().connect (sigc::mem_fun (*this, &BSlicer_GUI::scale_changed));
	put (scaleControl, 760, 80);

	// Step shape monitor
	stepshapeDisplayFrame.set_border_width (2);
	stepshapeDisplayFrame.modify_bg (Gtk::STATE_NORMAL, fgColor);
	stepshapeDisplayFrame.get_label_widget()->modify_fg(Gtk::STATE_NORMAL, white);
	stepshapeDisplayFrame.set_size_request (180,140);
	put (stepshapeDisplayFrame, 30, 320);

	stepshapeDisplayBox.set_size_request (180,120);
	stepshapeDisplayBox.signal_expose_event().connect (sigc::mem_fun (*this, &BSlicer_GUI::stepshape_monitor_expose_event));
	stepshapeDisplayFrame.add (stepshapeDisplayBox);

	// Attack
	attackControl.set_size_request (90,40);
	attackControl.modify_fg (Gtk::STATE_NORMAL, white);
	attackControl.modify_bg (Gtk::STATE_SELECTED, fgColor2);
	attackControl.signal_value_changed().connect (sigc::mem_fun (*this, &BSlicer_GUI::attack_changed));
	put (attackControl, 20, 480);

	// Attack label
	attackLabel.set_size_request (90,20);
	attackLabel.modify_fg (Gtk::STATE_NORMAL, white);
	put (attackLabel, 20, 520);

	// Release
	releaseControl.set_size_request (90,40);
	releaseControl.modify_fg (Gtk::STATE_NORMAL, white);
	releaseControl.modify_bg (Gtk::STATE_SELECTED, fgColor2);
	releaseControl.signal_value_changed().connect (sigc::mem_fun (*this, &BSlicer_GUI::release_changed));
	put (releaseControl, 130, 480);

	// Release label
	releaseLabel.set_size_request (90,20);
	releaseLabel.modify_fg (Gtk::STATE_NORMAL, white);
	put (releaseLabel, 130, 520);

	// Monitor
	monitorDisplayFrame.set_border_width (2);
	monitorDisplayFrame.modify_bg (Gtk::STATE_NORMAL, fgColor);
	monitorDisplayFrame.get_label_widget()->modify_fg(Gtk::STATE_NORMAL, white);
	monitorDisplayFrame.set_size_request (480, 240);
	put (monitorDisplayFrame, 260, 70);

	monitorDisplayBox.set_size_request (480,220);
	monitorDisplayBox.signal_expose_event().connect (sigc::mem_fun (*this, &BSlicer_GUI::main_monitor_expose_event));
	monitorDisplayFrame.add (monitorDisplayBox);

	// Steps
	container5.set_size_request (480,130);
	put (container5, 260, 330);
	for (int i = 0; i < MAXSTEPS; i++)
	{
		stepControl[i].set_inverted (true);
	    stepControl[i].modify_fg (Gtk::STATE_NORMAL, white);
		stepControl[i].modify_bg (Gtk::STATE_SELECTED, fgColor2);
	    stepControl[i].set_size_request (28,130);
	    stepControl[i].signal_value_changed().connect (sigc::mem_fun (*this, &BSlicer_GUI::step_changed));
	    container5.pack_start (stepControl[i], true, false, 0);
	}

	// Step controller label
	stepControlLabel.set_angle (90);
	stepControlLabel.set_size_request (20,130);
	stepControlLabel.modify_fg (Gtk::STATE_NORMAL, white);
	put (stepControlLabel, 760, 330);

	// Step size
	stepsizeControl.set_inverted (true);
	stepsizeControl.set_size_request (120,40);
	stepsizeControl.modify_fg (Gtk::STATE_NORMAL, white);
	stepsizeControl.modify_bg (Gtk::STATE_SELECTED, fgColor2);
	stepsizeControl.signal_value_changed().connect (sigc::mem_fun (*this, &BSlicer_GUI::stepsize_changed));
	put (stepsizeControl, 260, 480);

	// Step size label
	stepsizeLabel.set_size_request (120,20);
	stepsizeLabel.modify_fg (Gtk::STATE_NORMAL, white);
	put (stepsizeLabel, 260, 520);

	// nrSteps
	nrStepsControl.set_increments (1.0, 1.0);
	nrStepsControl.set_size_request (380,40);
	nrStepsControl.modify_fg (Gtk::STATE_NORMAL, white);
	nrStepsControl.modify_bg (Gtk::STATE_SELECTED, fgColor2);
	nrStepsControl.signal_value_changed().connect (sigc::mem_fun (*this, &BSlicer_GUI::nrSteps_changed));
	put (nrStepsControl, 400, 480);

	// Number of steps label
	nrStepsLabel.set_size_request (380,20);
	nrStepsLabel.modify_fg (Gtk::STATE_NORMAL, white);
	put (nrStepsLabel, 400, 520);

	return;
}

LV2UI_Handle instantiate (const LV2UI_Descriptor *descriptor,
									 	 	  const char *plugin_uri,
											  const char *bundle_path,
											  LV2UI_Write_Function write_function,
											  LV2UI_Controller controller,
											  LV2UI_Widget *widget,
											  const LV2_Feature *const *features)
{
	if (strcmp(plugin_uri, BSLICER_URI) != 0)
	{
		std::cerr << "BSlicer.lv2#GUI: GUI does not support plugin with URI " << plugin_uri << std::endl;
		return NULL;
	}

	Gtk::Main::init_gtkmm_internals();
	BSlicer_GUI* ui = new BSlicer_GUI (bundle_path, features);

	if (!ui)
	{
		std::cerr << "BSlicer.lv2#GUI: Instantiation failed.\n";
		return NULL;
	}

	ui->controller = controller;
	ui->write_function = write_function;
	*widget = (LV2UI_Widget) ui->gobj();

	return (LV2UI_Handle) ui;
}

void cleanup(LV2UI_Handle ui)
{
	BSlicer_GUI* pluginGui = (BSlicer_GUI*) ui;
	delete pluginGui;
}

void port_event(LV2UI_Handle ui, uint32_t port_index, uint32_t buffer_size, uint32_t format, const void* buffer)
{
	BSlicer_GUI* pluginGui = (BSlicer_GUI*) ui;
	pluginGui->portEvent(ui, port_index, buffer_size, format, buffer);
}

const LV2UI_Descriptor guiDescriptor = {
		BSLICER_GUI_URI,
		instantiate,
		cleanup,
		port_event,
		NULL	// Extension data
};

// LV2 Symbol Export
LV2_SYMBOL_EXPORT const LV2UI_Descriptor *lv2ui_descriptor(uint32_t index)
{
	switch (index) {
	case 0: return &guiDescriptor;
	default:return NULL;
    }
}
