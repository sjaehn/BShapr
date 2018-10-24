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
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#include <lv2/lv2plug.in/ns/extensions/ui/ui.h>
#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/atom/forge.h>
#include <lv2/lv2plug.in/ns/ext/time/time.h>
#include <gtk/gtk.h>
#include <cairo.h>

#include "main.h"

#define FONTSIZE 11264
#define SCALEMIN -60
#define SCALEMAX 30
#define CAIRO_BG_COLOR 0.0, 0.0, 0.0, 1.0
#define CAIRO_BG_COLOR2 0.2, 0.2, 0.2, 1.0
#define GDK_BG_COLOR {0, 0x0000, 0x0000, 0x0000}
#define GDK_WHITE {0, 0xFFFF, 0xFFFF, 0xFFFF}
#define GDK_FG_COLOR {0, 0x0000, 0xffff, 0x5fff}
#define GDK_FG_COLOR2 {0, 0xcfff, 0x9fff, 0x2fff}
#define CAIRO_FG_COLOR 1.0, 1.0, 1.0, 1.0
#define CAIRO_INK1 0.0, 1.0, 0.4
#define CAIRO_INK2 0.8, 0.6, 0.2

#define BG_FILE "surface.jpeg"

#define DB_CO(g) ((g) > -90.0f ? powf(10.0f, (g) * 0.05f) : 0.0f)
#define CO_DB(g) ((g) > 0.0001f ? logf((g)) / 0.05f : -90.0f)
#define LIMIT(g , max) ((g) > (max) ? (max) : (g))
#define INT(g) (int) (g + 0.5)

class BSlicer_GUI
{
public:
	BSlicer_GUI (const char *bundle_path, const LV2_Feature *const *features);
	~BSlicer_GUI ();
	void portEvent (uint32_t port_index, uint32_t buffer_size, uint32_t format, const void *buffer);
	GtkWidget* make_gui();
	void send_record_on ();
	void send_record_off ();

	LV2UI_Controller controller;
	LV2UI_Write_Function write_function;


private:
	void rearrange_step_controllers (float nrSteps_newf);
	static void value_changed_cb(GtkWidget* widget, void* data);
	static gboolean stepshape_monitor_expose_event (GtkWidget* widget, GdkEventExpose* ev, gpointer data);
	void clear_monitor_data ();
	void add_monitor_data (BSlicerNotifications* notifications, uint32_t notificationsCount, uint32_t* end);
	static gboolean main_monitor_expose_event (GtkWidget* widget, GdkEventExpose* ev, gpointer data);


	GtkWidget* bgImage;
	GtkWidget* mContainer;
	GtkWidget* container5;
	GtkWidget* attackLabel;
	GtkWidget* releaseLabel;
	GtkWidget* stepsizeLabel;
	GtkWidget* stepshapeDisplayFrame;
	GtkWidget* stepshapeDisplayBox;
	GtkWidget* stepControlLabel;
	GtkWidget* nrStepsLabel;
	GtkWidget* onLabel;
	GtkWidget* offLabel;
	GtkWidget* monitorLabel;
	GtkWidget* monitorOnOffControl;
	GtkWidget* scaleControl;
	GtkWidget* monitorDisplayFrame;
	GtkWidget* monitorDisplayBox;
	GtkWidget* nrStepsControl;
	GtkWidget* attackControl;
	GtkWidget* releaseControl;
	GtkWidget* stepsizeControl;
	GtkWidget* stepControl[MAXSTEPS];

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
	float step[MAXSTEPS];

	LV2_Atom_Forge forge;
	BSlicerURIs uris;
	LV2_URID_Map* map;

};

GdkColor monitor_bg_color = GDK_BG_COLOR;
GdkColor monitor_frame_color = GDK_FG_COLOR;
gchar stepshape_label[] = "Step shape";
gchar monitor_label[] = "Monitor";
gchar on_text[] = "on";
gchar off_text[] = "off";
gchar attackLabel_text[] = "Attack";
gchar releaseLabel_text[] = "Release";
gchar stepControlLabel_text[] = "Step control";
gchar stepsizeLabel_text[] = "\u2669 = 1/x";
gchar nrStepsLabel_text[] = "Number of steps";



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
			gtk_box_pack_start (GTK_BOX(container5), stepControl[i], TRUE, FALSE, 0);
		}
	}
	else if (nrSteps_new < nrSteps_old)
	{
		for (int i = nrSteps_new; i < nrSteps_old; i++)
		{
			g_object_ref (stepControl[i]);
			gtk_container_remove (GTK_CONTAINER(container5), stepControl[i]);
		}
	}
}

void BSlicer_GUI::value_changed_cb(GtkWidget* widget, void* data) {
	BSlicer_GUI *ui = (BSlicer_GUI*)data;

	// monitor on/off changed
	if (widget == ui->monitorOnOffControl)
	{
		int value = INT (gtk_range_get_value ((GtkRange*) widget));
		gtk_range_set_value ((GtkRange*) widget, value);
		if (value == 1)
		{
			ui->mainMonitor.record_on = true;
			ui->send_record_on ();
		}
		else
		{
			ui->mainMonitor.record_on = false;
			ui->send_record_off ();
		}
		return;
	}

	// Scale changed
	if (widget == ui->scaleControl)
	{
		float value = (float) gtk_range_get_value ((GtkRange*) widget);
		ui->scale = DB_CO (value);
		if (ui->scale < 0.0001f) ui->scale = 0.0001f;
		gtk_widget_queue_draw(ui->monitorDisplayBox);
	}

	// Attack changed
	if (widget == ui->attackControl)
	{
		ui->attack = (float)gtk_range_get_value((GtkRange*)widget);
		ui->write_function(ui->controller, Attack, sizeof(ui->attack), 0, &ui->attack);
		gtk_widget_queue_draw(ui->stepshapeDisplayBox);
		return;
	}

	// Release changed
	if (widget == ui->releaseControl)
	{
		ui->release = (float)gtk_range_get_value((GtkRange*)widget);
		ui->write_function(ui->controller, Release, sizeof(ui->release), 0, &ui->release);
		gtk_widget_queue_draw(ui->stepshapeDisplayBox);
		return;
	}

	// Step size changed
	if (widget == ui->stepsizeControl)
		{
			ui->stepsize = (float)gtk_range_get_value((GtkRange*)widget);
			ui->write_function(ui->controller, Stepsize, sizeof(ui->stepsize), 0, &ui->stepsize);
			return;
		}

	// nrSteps changed
	if (widget == ui->nrStepsControl)
	{
		float nrSteps_new = (float)gtk_range_get_value((GtkRange*)widget);
		if (nrSteps_new != ui->nrSteps) ui->rearrange_step_controllers (nrSteps_new);
		ui->nrSteps = nrSteps_new;
		ui->write_function(ui->controller, NrSteps, sizeof(ui->nrSteps), 0, &ui->nrSteps);
		gtk_widget_queue_draw(ui->monitorDisplayBox);
		return;
	}

	// Step controllers changed
	for (int i = 0; i < MAXSTEPS; i++)
	{
		if (widget == ui->stepControl[i])
		{
			if (i < INT (ui->nrSteps))
			{
				ui->step[i] = (float)gtk_range_get_value((GtkRange*)widget);
				ui->write_function(ui->controller, Step_+i , sizeof(ui->step[i]), 0, &ui->step[i]);
			}
			return;
		}
	}
}

gboolean BSlicer_GUI::stepshape_monitor_expose_event (GtkWidget* widget, GdkEventExpose* ev, gpointer data)
{
	BSlicer_GUI* ui = (BSlicer_GUI*) data;
	const uint32_t width = ev->area.width;
	const uint32_t height = ev->area.height;

	cairo_t* cr;
	cairo_pattern_t* pat;
	cr = gdk_cairo_create ((GdkDrawable*) ui->stepshapeDisplayBox->window); // @suppress("Field cannot be resolved")

	// Draw background
	cairo_set_source_rgba (cr, CAIRO_BG_COLOR);
	cairo_rectangle (cr, 0.0, 0.0, width, height);
	cairo_fill (cr);
	cairo_set_source_rgba (cr, CAIRO_BG_COLOR2);
	cairo_set_line_width (cr, 1);
	cairo_move_to (cr, 0, 0.1 * height);
	cairo_line_to (cr, width, 0.1 * height);
	cairo_move_to (cr, 0, 0.5 * height);
	cairo_line_to (cr, width, 0.5 * height);
	cairo_move_to (cr, 0, 0.9 * height);
	cairo_line_to (cr, width, 0.9 * height);
	cairo_move_to (cr, 0.25 * width, 0);
	cairo_line_to (cr, 0.25 * width, height);
	cairo_move_to (cr, 0.5 * width, 0);
	cairo_line_to (cr, 0.5 * width, height);
	cairo_move_to (cr, 0.75 * width, 0);
	cairo_line_to (cr, 0.75 * width, height);
	cairo_stroke (cr);

	// Draw step shape
	cairo_set_source_rgba (cr, CAIRO_INK1, 1.0);
	cairo_set_line_width (cr, 3);

	cairo_move_to (cr, 0, 0.9 * height);
	cairo_line_to (cr, width * 0.25, 0.9 * height);
	if ((ui->attack + ui->release) > 1)
	{
		float crosspointX = ui->attack / (ui->attack + ui->release);
		float crosspointY = crosspointX / ui->attack - (crosspointX - (1 - ui->release)) / ui->release;
		cairo_line_to (cr, width* 0.25 + crosspointX * width * 0.5, 0.9 * height - 0.8 * height * crosspointY);
	}
	else
	{
		cairo_line_to (cr, width * 0.25 + ui->attack * width * 0.5 , 0.1 * height);
		cairo_line_to (cr, width * 0.75  - ui->release * width * 0.5, 0.1 * height);

	}
	cairo_line_to (cr, width * 0.75, 0.9 * height);
	cairo_line_to (cr, width, 0.9 * height);

	cairo_stroke_preserve (cr);

	pat = cairo_pattern_create_linear (0, 0, 0, height);
	cairo_pattern_add_color_stop_rgba (pat, 0.1, CAIRO_INK1, 1);
	cairo_pattern_add_color_stop_rgba (pat, 0.9, CAIRO_INK1, 0);
	cairo_set_source (cr, pat);
	cairo_line_to(cr, 0, 0.9 * height);
	cairo_set_line_width (cr, 0);
	cairo_fill (cr);

	cairo_destroy (cr);

	return TRUE;
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

gboolean BSlicer_GUI::main_monitor_expose_event (GtkWidget* widget, GdkEventExpose* ev, gpointer data)
{
	BSlicer_GUI* ui = (BSlicer_GUI*) data;

	const uint32_t width = ev->area.width;
	const uint32_t height = ev->area.height;
	uint32_t i;
	bool lineBreak;
	float pos, nextpos, linebreakpos;

	cairo_t* cr;
	cairo_t* cr2;
	cairo_pattern_t* pat;
	cairo_pattern_t* pat2;
	cr = gdk_cairo_create ((GdkDrawable*) ui->monitorDisplayBox->window); // @suppress("Field cannot be resolved")
	cr2 = gdk_cairo_create ((GdkDrawable*) ui->monitorDisplayBox->window); // @suppress("Field cannot be resolved")
	pat = cairo_pattern_create_linear (0, 0, 0, height);
	cairo_pattern_add_color_stop_rgba (pat, 0.1, CAIRO_INK1, 1);
	cairo_pattern_add_color_stop_rgba (pat, 0.9, CAIRO_INK1, 0);
	pat2 = cairo_pattern_create_linear (0, 0, 0, height);
	cairo_pattern_add_color_stop_rgba (pat2, 0.1, CAIRO_INK2, 1);
	cairo_pattern_add_color_stop_rgba (pat2, 0.9, CAIRO_INK2, 0);

	// Draw background
	cairo_set_source_rgba (cr, CAIRO_BG_COLOR);
	cairo_rectangle (cr, 0, 0, width, height);
	cairo_fill (cr);

	cairo_set_source_rgba (cr, CAIRO_BG_COLOR2);
	cairo_set_line_width (cr, 1);
	cairo_move_to (cr, 0, 0.1 * height);
	cairo_line_to (cr, width, 0.1 * height);
	cairo_move_to (cr, 0, 0.9 * height);
	cairo_line_to (cr, width, 0.9 * height);

	i = 1;
	uint32_t steps = (uint32_t) ui->nrSteps;
	while (i < steps)
	{
		uint32_t x = (uint32_t) (i * width / steps);
		cairo_move_to (cr, x, 0);
		cairo_line_to (cr, x, height);
		i++;
	}
	cairo_stroke (cr);

	if (ui->mainMonitor.record_on)
	{
		uint32_t horizoni = ui->mainMonitor.horizonPos;
		float horizonpos = ui->mainMonitor.data[horizoni].position;
		if (horizonpos >= 0.0f)
		{
			// Draw horizon line
			cairo_set_source_rgba (cr, CAIRO_FG_COLOR);
			cairo_set_line_width (cr, 1);
			cairo_move_to (cr, (uint32_t) (horizonpos * width), 0);
			cairo_line_to (cr, (uint32_t) (horizonpos * width), height);
			cairo_stroke (cr);

			// Draw data: input@cr, output@cr2
			cairo_move_to (cr, (uint32_t) (horizonpos * width), (uint32_t) (height * (0.9  - (0.8 * LIMIT ((ui->mainMonitor.data[horizoni].input / ui->scale), 1.0f)))));
			cairo_move_to (cr2, (uint32_t) (horizonpos * width), (uint32_t) (height * (0.9  - (0.8 * LIMIT ((ui->mainMonitor.data[horizoni].output / ui->scale), 1.0f)))));
			lineBreak = false;
			i = horizoni;
			nextpos = horizonpos;
			linebreakpos = horizonpos;

			do
			{
				pos = nextpos;
	//			fprintf (stdout, "%i %f %f %f\n", i, pos, ui->mainMonitor.data[i].input, ui->mainMonitor.data[i].output);
				cairo_line_to (cr, (uint32_t) (pos * width), (uint32_t) (height * (0.9  - (0.8 * LIMIT ((ui->mainMonitor.data[i].input / ui->scale), 1.0f)))));
				cairo_line_to (cr2, (uint32_t) (pos * width), (uint32_t) (height * (0.9  - (0.8 * LIMIT ((ui->mainMonitor.data[i].output / ui->scale), 1.0f)))));
				if (i == 0) i = MONITORBUFFERSIZE - 1;
				else i--;
				nextpos = ui->mainMonitor.data[i].position;

				// Line break in raw data?
				if (nextpos > pos)
				{
	//				fprintf (stdout, "lb: %i %f %f %f %f\n", i, pos, nextpos, ui->mainMonitor.data[i].input, ui->mainMonitor.data[i].output);
					// Visualize input (cr) and output (cr2) curves between horizon and breakpoint
					cairo_set_source_rgba (cr, CAIRO_INK1, 1.0);
					cairo_set_line_width (cr, 3);
					cairo_set_source_rgba (cr2, CAIRO_INK2, 1.0);
					cairo_set_line_width (cr2, 3);
					cairo_stroke_preserve (cr);
					cairo_stroke_preserve (cr2);

					// Visualize input (cr) and output (cr2) areas under the curves between horizon and breakpoint
					cairo_line_to (cr, (uint32_t) (pos * width), (uint32_t) (height * 0.9));
					cairo_line_to (cr, (uint32_t) (horizonpos * width), (uint32_t) (height * 0.9));
					cairo_close_path (cr);
					cairo_line_to (cr2, (uint32_t) (pos * width), (uint32_t) (height * 0.9));
					cairo_line_to (cr2, (uint32_t) (horizonpos * width), (uint32_t) (height * 0.9));
					cairo_close_path (cr2);
					cairo_set_source (cr, pat);
					cairo_set_line_width (cr, 0);
					cairo_set_source (cr2, pat2);
					cairo_set_line_width (cr2, 0);
					cairo_fill (cr);
					cairo_fill (cr2);

					lineBreak = true;
					linebreakpos = nextpos;
					cairo_move_to (cr, (uint32_t) (nextpos * width), (uint32_t) (height * (0.9  - (0.8 * LIMIT ((ui->mainMonitor.data[i].input / ui->scale), 1.0f)))));
					cairo_move_to (cr2, (uint32_t) (nextpos * width), (uint32_t) (height * (0.9  - (0.8 * LIMIT ((ui->mainMonitor.data[i].output / ui->scale), 1.0f)))));
				}
			} while ((nextpos >= 0.0f) && (((nextpos < horizonpos) && !lineBreak) || ((nextpos > horizonpos) && lineBreak)));

			// Visualize (remaining parts of) input (cr) and output (cr2) curves
			cairo_set_source_rgba (cr, CAIRO_INK1, 1.0);
			cairo_set_line_width (cr, 3);
			cairo_set_source_rgba (cr2, CAIRO_INK2, 1.0);
			cairo_set_line_width (cr2, 3);


			if (nextpos > pos)
			{
				cairo_stroke (cr);
				cairo_stroke (cr2);
				// Do not visualize areas under the curves if do while loop ended exactly on line break
			}
			else
			{
				cairo_stroke_preserve (cr);
				cairo_stroke_preserve (cr2);

				// Visualize (remaining parts of) input (cr) and output (cr2) areas under the curves
				cairo_line_to (cr, (uint32_t) (pos * width), (uint32_t) (height * 0.9));
				cairo_line_to (cr, (uint32_t) (linebreakpos * width), (uint32_t) (height * 0.9));
				cairo_close_path (cr);
				cairo_line_to (cr2, (uint32_t) (pos * width), (uint32_t) (height * 0.9));
				cairo_line_to (cr2, (uint32_t) (linebreakpos * width), (uint32_t) (height * 0.9));
				cairo_close_path (cr2);
				cairo_set_source (cr, pat);
				cairo_set_line_width (cr, 0);
				cairo_set_source (cr2, pat2);
				cairo_set_line_width (cr2, 0);
				cairo_fill (cr);
				cairo_fill (cr2);
			}
		}
	}
	cairo_destroy (cr);
	cairo_destroy (cr2);

	return TRUE;
}

void b_widget_set_colors  (GtkWidget* widget, GdkColor* fg, GdkColor* bg1, GdkColor* bg2, GdkColor* bg3)
{
	if (fg) gtk_widget_modify_fg (widget, GTK_STATE_NORMAL, fg);
	if (bg1) gtk_widget_modify_bg (widget, GTK_STATE_NORMAL, bg1);
	if (bg2) gtk_widget_modify_bg (widget, GTK_STATE_ACTIVE, bg2);
	if (bg3) gtk_widget_modify_bg (widget, GTK_STATE_SELECTED, bg3);
}

GtkWidget* BSlicer_GUI::make_gui()
{
	GtkStyle* style;
	GdkColor white = GDK_WHITE;
	GdkColor fgColor2 = GDK_FG_COLOR;
	int fontsize;

	// Main container
	mContainer = gtk_fixed_new();
	gtk_widget_set_size_request (mContainer, 800, 560);

	// Background image
	bgImage = gtk_image_new_from_file ((pluginPath + BG_FILE).c_str());
	gtk_widget_set_size_request(bgImage, 800,560);
	gtk_fixed_put (GTK_FIXED (mContainer), bgImage, 0, 0);

	// Monitor on/off switch
	offLabel = gtk_label_new(&off_text[0]);
	gtk_widget_set_size_request(offLabel, 40,20);
	b_widget_set_colors (offLabel, &white, NULL, NULL, NULL);
	gtk_fixed_put (GTK_FIXED (mContainer), offLabel, 640, 30);

	monitorOnOffControl = gtk_hscale_new_with_range(0, 1, 1);
	gtk_range_set_value (GTK_RANGE(monitorOnOffControl), 1);
	gtk_scale_set_draw_value(GTK_SCALE(monitorOnOffControl), false);
	g_signal_connect(G_OBJECT(monitorOnOffControl), "value-changed", G_CALLBACK(value_changed_cb), this);
	gtk_widget_set_size_request(monitorOnOffControl, 60,20);
	b_widget_set_colors (monitorOnOffControl, &white, NULL, NULL, &fgColor2);
	gtk_fixed_put (GTK_FIXED (mContainer), monitorOnOffControl, 680, 30);

	onLabel = gtk_label_new(&on_text[0]);
	gtk_widget_set_size_request(onLabel, 40,20);
	b_widget_set_colors (onLabel, &white, NULL, NULL, NULL);
	gtk_fixed_put (GTK_FIXED (mContainer), onLabel, 740, 30);

	monitorLabel = gtk_label_new(&monitor_label[0]);
	gtk_widget_set_size_request(monitorLabel, 60,20);
	b_widget_set_colors (monitorLabel, &white, NULL, NULL, NULL);
	gtk_fixed_put (GTK_FIXED (mContainer), monitorLabel, 680, 50);

	// Scale control
	scaleControl = gtk_vscale_new_with_range(SCALEMIN, SCALEMAX, 0.1);
	gtk_range_set_inverted ((GtkRange*) scaleControl, TRUE);
	gtk_range_set_value (GTK_RANGE(scaleControl), CO_DB (scale));
	gtk_scale_set_draw_value(GTK_SCALE(scaleControl), false);
	g_signal_connect(G_OBJECT(scaleControl), "value-changed", G_CALLBACK(value_changed_cb), this);
	gtk_widget_set_size_request(scaleControl, 20, 230);
	b_widget_set_colors (scaleControl, &white, NULL, NULL, &fgColor2);
	gtk_fixed_put (GTK_FIXED (mContainer), scaleControl, 760, 80);

	// Step shape monitor
	stepshapeDisplayFrame = gtk_frame_new(&stepshape_label[0]);						// Frame
	gtk_container_set_border_width (GTK_CONTAINER (stepshapeDisplayFrame), 2);
	b_widget_set_colors (stepshapeDisplayFrame, NULL, &monitor_frame_color, NULL, NULL);
	gtk_widget_set_size_request(stepshapeDisplayFrame, 180,140);
	gtk_fixed_put (GTK_FIXED (mContainer), stepshapeDisplayFrame, 30, 320);
	stepshapeDisplayBox = gtk_drawing_area_new();									// Box
	gtk_widget_set_size_request(stepshapeDisplayBox, 180,120);
	g_signal_connect (G_OBJECT (stepshapeDisplayBox), "expose-event", G_CALLBACK (stepshape_monitor_expose_event), this);
	gtk_container_add (GTK_CONTAINER (stepshapeDisplayFrame), stepshapeDisplayBox);

	// Attack
	attackControl = gtk_hscale_new_with_range(0.0, 1.0, 0.01);
	g_signal_connect(G_OBJECT(attackControl), "value-changed", G_CALLBACK(value_changed_cb), this);
	gtk_widget_set_size_request(attackControl, 90,40);
	b_widget_set_colors (attackControl, &white, NULL, NULL, &fgColor2);
	gtk_fixed_put (GTK_FIXED (mContainer), attackControl, 20, 480);

	// Attack label
	attackLabel = gtk_label_new(&attackLabel_text[0]);
	gtk_widget_set_size_request(attackLabel, 90,20);
	b_widget_set_colors (attackLabel, &white, NULL, NULL, NULL);
	gtk_fixed_put (GTK_FIXED (mContainer), attackLabel, 20, 520);

	// Release
	releaseControl = gtk_hscale_new_with_range(0.0, 1.0, 0.01);
	g_signal_connect(G_OBJECT(releaseControl), "value-changed", G_CALLBACK(value_changed_cb), this);
	gtk_widget_set_size_request(releaseControl, 90,40);
	b_widget_set_colors (releaseControl, &white, NULL, NULL, &fgColor2);
	gtk_fixed_put (GTK_FIXED (mContainer), releaseControl, 130, 480);

	// Release label
	releaseLabel = gtk_label_new(&releaseLabel_text[0]);
	gtk_widget_set_size_request(releaseLabel, 90,20);
	b_widget_set_colors (releaseLabel, &white, NULL, NULL, NULL);
	gtk_fixed_put (GTK_FIXED (mContainer), releaseLabel, 130, 520);

	// Monitor
	monitorDisplayFrame = gtk_frame_new(&monitor_label[0]);						// Frame
	gtk_container_set_border_width (GTK_CONTAINER (monitorDisplayFrame), 2);
	b_widget_set_colors (monitorDisplayFrame, NULL, &monitor_frame_color, NULL, NULL);
	gtk_widget_set_size_request(monitorDisplayFrame, 480,240);
	gtk_fixed_put (GTK_FIXED (mContainer), monitorDisplayFrame, 260, 70);
	monitorDisplayBox = gtk_drawing_area_new();									// Box
	gtk_widget_set_size_request(monitorDisplayBox, 480,220);
	g_signal_connect (G_OBJECT (monitorDisplayBox), "expose-event", G_CALLBACK (main_monitor_expose_event), this);
	gtk_container_add (GTK_CONTAINER (monitorDisplayFrame), monitorDisplayBox);

	// Steps
	container5 = gtk_hbox_new(FALSE, 2);										// Container for step controllers
	gtk_widget_set_size_request(container5, 480,130);
	gtk_fixed_put (GTK_FIXED (mContainer), container5, 260, 330);
	for (int i = 0; i < MAXSTEPS; i++)
	{
		stepControl[i] = gtk_vscale_new_with_range(0.0, 1.0, 0.01);
		g_signal_connect(G_OBJECT(stepControl[i]), "value-changed", G_CALLBACK(value_changed_cb), this);
		gtk_range_set_inverted((GtkRange*)stepControl[i], TRUE);
		style = gtk_widget_get_style(stepControl[i]);
		pango_font_description_set_size(style->font_desc, FONTSIZE*0.75);
	    gtk_widget_modify_font(stepControl[i], style->font_desc);
	    b_widget_set_colors (stepControl[i], &white, NULL, NULL, &fgColor2);
		gtk_widget_set_size_request(stepControl[i], 28,130);
		gtk_box_pack_start (GTK_BOX(container5), stepControl[i], TRUE, FALSE, 0);
	}

	// Step controller label
	stepControlLabel = gtk_label_new(&stepControlLabel_text[0]);
	gtk_label_set_angle (GTK_LABEL (stepControlLabel), 90);
	gtk_widget_set_size_request(stepControlLabel, 20,130);
	b_widget_set_colors (stepControlLabel, &white, NULL, NULL, NULL);
	gtk_fixed_put (GTK_FIXED (mContainer), stepControlLabel, 760, 330);

	// Step size
	stepsizeControl = gtk_hscale_new_with_range(1, 8, 1);
	gtk_range_set_inverted((GtkRange*)stepsizeControl, TRUE);
	g_signal_connect(G_OBJECT(stepsizeControl), "value-changed", G_CALLBACK(value_changed_cb), this);
	gtk_widget_set_size_request(stepsizeControl, 120,40);
	b_widget_set_colors (stepsizeControl, &white, NULL, NULL, &fgColor2);
	gtk_fixed_put (GTK_FIXED (mContainer), stepsizeControl, 260, 480);

	// Step size label
	stepsizeLabel = gtk_label_new(&stepsizeLabel_text[0]);
	gtk_widget_set_size_request(stepsizeLabel, 120,20);
	b_widget_set_colors (stepsizeLabel, &white, NULL, NULL, NULL);
	gtk_fixed_put (GTK_FIXED (mContainer), stepsizeLabel, 260, 520);

	// nrSteps
	nrStepsControl = gtk_hscale_new_with_range(1, MAXSTEPS, 1);
	g_signal_connect(G_OBJECT(nrStepsControl), "value-changed", G_CALLBACK(value_changed_cb), this);
	gtk_widget_set_size_request(nrStepsControl, 380,40);
	b_widget_set_colors (nrStepsControl, &white, NULL, NULL, &fgColor2);
	gtk_fixed_put (GTK_FIXED (mContainer), nrStepsControl, 400, 480);

	// Number of steps label
	nrStepsLabel = gtk_label_new(&nrStepsLabel_text[0]);
	gtk_widget_set_size_request(nrStepsLabel, 380,20);
	b_widget_set_colors (nrStepsLabel, &white, NULL, NULL, NULL);
	gtk_fixed_put (GTK_FIXED (mContainer), nrStepsLabel, 400, 520);

	return mContainer;
}

BSlicer_GUI::BSlicer_GUI (const char *bundle_path, const LV2_Feature *const *features) :
	pluginPath (bundle_path ? std::string (bundle_path) : std::string ("")), controller (NULL), write_function (NULL),
	onLabel (NULL), offLabel (NULL), monitorLabel (NULL),  attackLabel (NULL), releaseLabel (NULL), stepControlLabel (NULL),
	stepsizeLabel (NULL), nrStepsLabel (NULL), stepshapeDisplayFrame (NULL), stepshapeDisplayBox (NULL), monitorDisplayFrame (NULL),
	monitorDisplayBox (NULL), monitorOnOffControl (NULL), scaleControl (NULL), nrStepsControl (NULL), attackControl (NULL), releaseControl (NULL),
	stepsizeControl (NULL),
	scale (1.0f), attack (0.2f), release (0.2f), nrSteps (MAXSTEPS), stepsize (1),
	map (NULL)

{
	//Initialialize stepControllers
	for (int i = 0; i < MAXSTEPS; i++) stepControl[i] = NULL;

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
		fprintf(stderr, "BSlicer.lv2#GUI: Host does not support urid:map.\n");
		return;
	}

	//Map URIS
	map = m;
	getURIs (map, &uris);

	// Initialize forge
	lv2_atom_forge_init (&forge,map);
}

BSlicer_GUI::~BSlicer_GUI()
{
	send_record_off ();
	delete mainMonitor.data;
}



void BSlicer_GUI::portEvent(uint32_t port_index, uint32_t buffer_size, uint32_t format, const void* buffer)
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
							gtk_widget_queue_draw(monitorDisplayBox);
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
			gtk_range_set_value((GtkRange*)attackControl, *pval);
			gtk_widget_queue_draw(stepshapeDisplayBox);
			break;
		case Release:
			gtk_range_set_value((GtkRange*)releaseControl, *pval);
			gtk_widget_queue_draw(stepshapeDisplayBox);
			break;
		case Stepsize:
			gtk_range_set_value((GtkRange*)stepsizeControl, *pval);
		break;
		case NrSteps:
			if (nrSteps != *pval)
			{
				rearrange_step_controllers (*pval);
				nrSteps = *pval;
			}
			gtk_widget_queue_draw(monitorDisplayBox);
			gtk_range_set_value((GtkRange*)nrStepsControl, *pval);
			break;
		default:
			if ((port_index >= Step_) and (port_index < Step_ + MAXSTEPS))
			{
				gtk_range_set_value((GtkRange*)stepControl[port_index-Step_], *pval);
			}
		}
	}

}

LV2UI_Handle instantiate (const LV2UI_Descriptor *descriptor, const char *plugin_uri, const char *bundle_path,
						  LV2UI_Write_Function write_function, LV2UI_Controller controller, LV2UI_Widget *widget,
						  const LV2_Feature *const *features)
{
	if (strcmp(plugin_uri, BSLICER_URI) != 0)
	{
		fprintf(stderr, "BSlicer.lv2#GUI: GUI does not support plugin with URI %s\n", plugin_uri);
		return NULL;
	}

	// New instance
	BSlicer_GUI* ui = new BSlicer_GUI (bundle_path, features);

	if (!ui)
	{
		fprintf(stderr, "BSlicer.lv2#GUI: Instantiation failed.\n");
		return NULL;
	}

	ui->controller = controller;
	ui->write_function = write_function;

	*widget = (LV2UI_Widget) ui->make_gui();
	ui->send_record_on();
	return (LV2UI_Handle) ui;
}

void cleanup(LV2UI_Handle ui)
{
	BSlicer_GUI* pluginGui = (BSlicer_GUI*) ui;
	delete pluginGui;
}

void portEvent(LV2UI_Handle ui, uint32_t port_index, uint32_t buffer_size,
	uint32_t format, const void* buffer)
{
	BSlicer_GUI* pluginGui = (BSlicer_GUI*) ui;
	pluginGui->portEvent(port_index, buffer_size, format, buffer);
}


const LV2UI_Descriptor guiDescriptor = {
		BSLICER_GUI_URI,
		instantiate,
		cleanup,
		portEvent,
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
