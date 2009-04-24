/* 

    This LV2 extension provides lpf and hpf gui's

    (c) Fraser Stuart 2009

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gtk/gtk.h>
#include <lv2.h>
#include "lv2_ui.h"
#include "widgets/knob.h"
#include "widgets/lamp.h"
#include "widgets/meter-peak.h"
#include "widgets/switch-toggle.h"
#include "../plugin/inv_tube.h"
#include "inv_tube_gui.h"


static LV2UI_Descriptor *ITubeGuiDescriptor = NULL;

typedef struct {
	GtkWidget	*windowContainer;
	GtkWidget	*heading;
	GtkWidget	*meterIn;
	GtkWidget	*meterOut;
	GtkWidget	*knobDrive;
	GtkWidget	*lampDrive;
	GtkWidget	*knobDC;
	GtkWidget	*togglePhase;
	GtkWidget	*knobBlend;

	gint		InChannels;
	gint		OutChannels;
	float		drive;
	float		dc;
	float		phase;
	float		blend;

	LV2UI_Write_Function 	write_function;
	LV2UI_Controller 	controller;

} ITubeGui;



static LV2UI_Handle instantiateITubeGui(const struct _LV2UI_Descriptor* descriptor, const char* plugin_uri, const char* bundle_path, LV2UI_Write_Function write_function, LV2UI_Controller controller, LV2UI_Widget* widget, const LV2_Feature* const* features)
{

	ITubeGui *pluginGui = (ITubeGui *)malloc(sizeof(ITubeGui));
	if(pluginGui==NULL)
		return NULL;

	pluginGui->write_function = write_function;
	pluginGui->controller = controller;

	GtkBuilder      *builder; 
	GtkWidget       *window;
	GtkWidget	*tempObject;

	char 		*file;

	GError *err = NULL;

	gtk_init (NULL,NULL);

	builder = gtk_builder_new ();
	file = g_strdup_printf("%s/gtk/inv_tube_gui.xml",bundle_path);
	gtk_builder_add_from_file (builder, file, &err);
	free(file);

	window = GTK_WIDGET (gtk_builder_get_object (builder, "tube_window"));

	/* get pointers to some useful widgets from the design */
	pluginGui->windowContainer = GTK_WIDGET (gtk_builder_get_object (builder, "tube_container"));
	pluginGui->heading = GTK_WIDGET (gtk_builder_get_object (builder, "label_heading"));

	/* add custom widgets */
	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_meter_in"));
	pluginGui->meterIn = inv_meter_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->meterIn);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_meter_out"));
	pluginGui->meterOut = inv_meter_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->meterOut);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_drive_knob"));
	pluginGui->knobDrive = inv_knob_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->knobDrive);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_drive_lamp"));
	pluginGui->lampDrive = inv_lamp_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->lampDrive);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_dc_knob"));
	pluginGui->knobDC = inv_knob_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->knobDC);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_phase_toggle"));
	pluginGui->togglePhase = inv_switch_toggle_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->togglePhase);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_blend_knob"));
	pluginGui->knobBlend = inv_knob_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->knobBlend);


	/* customise for the plugin */
	if(!strcmp(plugin_uri,ITUBE_MONO_URI)) 
	{
		pluginGui->InChannels=1;
		pluginGui->OutChannels=1;
		gtk_label_set_markup (GTK_LABEL (pluginGui->heading), "<b>Invada Tube (mono)</b>");
	}
	if(!strcmp(plugin_uri,ITUBE_STEREO_URI)) 
	{
		pluginGui->InChannels=2;
		pluginGui->OutChannels=2;
		gtk_label_set_markup (GTK_LABEL (pluginGui->heading), "<b>Invada Tube (stereo)</b>");
	}

	pluginGui->drive=0.0;
	pluginGui->dc=0.0;
	pluginGui->phase=0;
	pluginGui->blend=75;



	inv_meter_set_channels(INV_METER (pluginGui->meterIn), pluginGui->InChannels);
	inv_meter_set_LdB(INV_METER (pluginGui->meterIn),-90);
	inv_meter_set_RdB(INV_METER (pluginGui->meterIn),-90);

	inv_meter_set_channels(INV_METER (pluginGui->meterOut), pluginGui->OutChannels);
	inv_meter_set_LdB(INV_METER (pluginGui->meterOut),-90);
	inv_meter_set_RdB(INV_METER (pluginGui->meterOut),-90);

	inv_knob_set_size(INV_KNOB (pluginGui->knobDrive), INV_KNOB_SIZE_MEDIUM);
	inv_knob_set_curve(INV_KNOB (pluginGui->knobDrive), INV_KNOB_CURVE_LINEAR);
	inv_knob_set_markings(INV_KNOB (pluginGui->knobDrive), INV_KNOB_MARKINGS_4);
	inv_knob_set_highlight(INV_KNOB (pluginGui->knobDrive), INV_KNOB_HIGHLIGHT_L);
	inv_knob_set_units(INV_KNOB (pluginGui->knobDrive), "dB");
	inv_knob_set_min(INV_KNOB (pluginGui->knobDrive), 0.0);
	inv_knob_set_max(INV_KNOB (pluginGui->knobDrive), 18.0);
	inv_knob_set_value(INV_KNOB (pluginGui->knobDrive), pluginGui->drive);
	g_signal_connect_after(G_OBJECT(pluginGui->knobDrive),"motion-notify-event",G_CALLBACK(on_inv_tube_drive_knob_motion),pluginGui);

	inv_lamp_set_value(INV_LAMP (pluginGui->lampDrive),0.0);
	inv_lamp_set_scale(INV_LAMP (pluginGui->lampDrive),1.0);

	inv_knob_set_size(INV_KNOB (pluginGui->knobDC), INV_KNOB_SIZE_MEDIUM);
	inv_knob_set_curve(INV_KNOB (pluginGui->knobDC), INV_KNOB_CURVE_QUAD);
	inv_knob_set_markings(INV_KNOB (pluginGui->knobDC), INV_KNOB_MARKINGS_3); 
	inv_knob_set_highlight(INV_KNOB (pluginGui->knobDC), INV_KNOB_HIGHLIGHT_C);
	inv_knob_set_units(INV_KNOB (pluginGui->knobDC), "");
	inv_knob_set_min(INV_KNOB (pluginGui->knobDC), -1.0);
	inv_knob_set_max(INV_KNOB (pluginGui->knobDC), 1.0);
	inv_knob_set_value(INV_KNOB (pluginGui->knobDC), pluginGui->dc);
	g_signal_connect_after(G_OBJECT(pluginGui->knobDC),"motion-notify-event",G_CALLBACK(on_inv_tube_dc_knob_motion),pluginGui);

	inv_switch_toggle_set_value( INV_SWITCH_TOGGLE (pluginGui->togglePhase), INV_SWITCH_TOGGLE_OFF, 0.0);
	inv_switch_toggle_set_colour(INV_SWITCH_TOGGLE (pluginGui->togglePhase), INV_SWITCH_TOGGLE_OFF, 0.0, 1.0, 0.0);
	inv_switch_toggle_set_text(  INV_SWITCH_TOGGLE (pluginGui->togglePhase), INV_SWITCH_TOGGLE_OFF, "Normal");
	inv_switch_toggle_set_value( INV_SWITCH_TOGGLE (pluginGui->togglePhase), INV_SWITCH_TOGGLE_ON,  1.0);
	inv_switch_toggle_set_colour(INV_SWITCH_TOGGLE (pluginGui->togglePhase), INV_SWITCH_TOGGLE_ON,  0.0, 1.0, 0.0);
	inv_switch_toggle_set_text(  INV_SWITCH_TOGGLE (pluginGui->togglePhase), INV_SWITCH_TOGGLE_ON,  "Reversed");
	inv_switch_toggle_set_state( INV_SWITCH_TOGGLE (pluginGui->togglePhase), INV_SWITCH_TOGGLE_OFF);
	g_signal_connect_after(G_OBJECT(pluginGui->togglePhase),"button-release-event",G_CALLBACK(on_inv_tube_phase_toggle_button_release),pluginGui);

	inv_knob_set_size(INV_KNOB (pluginGui->knobBlend), INV_KNOB_SIZE_MEDIUM);
	inv_knob_set_curve(INV_KNOB (pluginGui->knobBlend), INV_KNOB_CURVE_LINEAR);
	inv_knob_set_markings(INV_KNOB (pluginGui->knobBlend), INV_KNOB_MARKINGS_5); 
	inv_knob_set_highlight(INV_KNOB (pluginGui->knobBlend), INV_KNOB_HIGHLIGHT_C);
	inv_knob_set_units(INV_KNOB (pluginGui->knobBlend), "%");
	inv_knob_set_min(INV_KNOB (pluginGui->knobBlend), 0.0);
	inv_knob_set_max(INV_KNOB (pluginGui->knobBlend), 100.0);
	inv_knob_set_value(INV_KNOB (pluginGui->knobBlend), pluginGui->blend);
	g_signal_connect_after(G_OBJECT(pluginGui->knobBlend),"motion-notify-event",G_CALLBACK(on_inv_tube_blend_knob_motion),pluginGui);

	/* strip the parent window from the design so the host can attach its own */
	gtk_widget_ref(pluginGui->windowContainer);
	gtk_container_remove(GTK_CONTAINER(window), pluginGui->windowContainer);

	*widget = (LV2UI_Widget) pluginGui->windowContainer;

	g_object_unref (G_OBJECT (builder));
 
	/* return the instance */
	return pluginGui;
}


static void cleanupITubeGui(LV2UI_Handle ui)
{
	return;
}


static void port_eventITubeGui(LV2UI_Handle ui, uint32_t port, uint32_t buffer_size, uint32_t format, const void*  buffer)
{
	ITubeGui *pluginGui = (ITubeGui *)ui;

	float value;

	if(format==0) 
	{
		value=* (float *) buffer;
		switch(port)
		{
			case ITUBE_DRIVE:
				pluginGui->drive=value;
				inv_knob_set_value(INV_KNOB (pluginGui->knobDrive), pluginGui->drive);
				break;
			case ITUBE_DCOFFSET:
				pluginGui->dc=value;
				inv_knob_set_value(INV_KNOB (pluginGui->knobDC), pluginGui->dc);
				break;
			case ITUBE_PHASE:
				pluginGui->phase=value;
				if(value <= 0.0) {
					inv_switch_toggle_set_state(INV_SWITCH_TOGGLE (pluginGui->togglePhase), INV_SWITCH_TOGGLE_OFF);
				} else {
					inv_switch_toggle_set_state(INV_SWITCH_TOGGLE (pluginGui->togglePhase), INV_SWITCH_TOGGLE_ON);
				}
				break;
			case ITUBE_MIX:
				pluginGui->blend=value;
				inv_knob_set_value(INV_KNOB (pluginGui->knobBlend), pluginGui->blend);
				break;
			case ITUBE_METER_INL:
				inv_meter_set_LdB(INV_METER (pluginGui->meterIn),value);
				break;
			case ITUBE_METER_INR:
				if(pluginGui->InChannels==2)  inv_meter_set_RdB(INV_METER (pluginGui->meterIn),value);
				break;
			case ITUBE_METER_OUTL:
				inv_meter_set_LdB(INV_METER (pluginGui->meterOut),value);
				break;
			case ITUBE_METER_OUTR:
				if(pluginGui->OutChannels==2)  inv_meter_set_RdB(INV_METER (pluginGui->meterOut),value);
				break;
		}
	}
}


static void init()
{
	ITubeGuiDescriptor =
	 (LV2UI_Descriptor *)malloc(sizeof(LV2UI_Descriptor));

	ITubeGuiDescriptor->URI 		= ITUBE_GUI_URI;
	ITubeGuiDescriptor->instantiate 	= instantiateITubeGui;
	ITubeGuiDescriptor->cleanup		= cleanupITubeGui;
	ITubeGuiDescriptor->port_event		= port_eventITubeGui;
	ITubeGuiDescriptor->extension_data 	= NULL;

}


const LV2UI_Descriptor* lv2ui_descriptor(uint32_t index)
{
	if (!ITubeGuiDescriptor) init();

	switch (index) {
		case 0:
			return ITubeGuiDescriptor;
	default:
		return NULL;
	}
}


/*****************************************************************************/


static void on_inv_tube_drive_knob_motion(GtkWidget *widget, GdkEvent *event, gpointer data)
{

	ITubeGui *pluginGui = (ITubeGui *) data;

	pluginGui->drive=inv_knob_get_value(INV_KNOB (widget));
	(*pluginGui->write_function)(pluginGui->controller, ITUBE_DRIVE, 4, 0, &pluginGui->drive);
	return;
}

static void on_inv_tube_dc_knob_motion(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	ITubeGui *pluginGui = (ITubeGui *) data;

	pluginGui->dc=inv_knob_get_value(INV_KNOB (widget));
	(*pluginGui->write_function)(pluginGui->controller, ITUBE_DCOFFSET, 4, 0, &pluginGui->dc);
	return;
}

static void on_inv_tube_phase_toggle_button_release(GtkWidget *widget, GdkEvent *event, gpointer data)
{

	ITubeGui *pluginGui = (ITubeGui *) data;

	pluginGui->phase=inv_switch_toggle_get_value(INV_SWITCH_TOGGLE (widget));
	(*pluginGui->write_function)(pluginGui->controller, ITUBE_PHASE, 4, 0, &pluginGui->phase);
	return;
}

static void on_inv_tube_blend_knob_motion(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	ITubeGui *pluginGui = (ITubeGui *) data;

	pluginGui->blend=inv_knob_get_value(INV_KNOB (widget));
	(*pluginGui->write_function)(pluginGui->controller, ITUBE_MIX, 4, 0, &pluginGui->blend);
	return;
}

