/* 

    This LV2 extension provides input module gui's

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
#include "widgets/widgets.h"
#include "widgets/meter-peak.h"
#include "widgets/meter-phase.h"
#include "widgets/switch-toggle.h"
#include "../plugin/inv_meter.h"
#include "inv_meter_gui.h"


static LV2UI_Descriptor *IMeterGuiDescriptor = NULL;

typedef struct {
	GtkWidget	*windowContainer;
	GtkWidget	*heading;
	GtkWidget	*toggleBypass;
	GtkWidget	*meterPeak;
	GtkWidget	*meterVU;
	GtkWidget	*meterPhase;
	GtkWidget	*specDisplay;
	GtkWidget	*specDisplay1;
	GtkWidget	*specDisplay2;
	GtkWidget	*specDisplay3;
	GtkWidget	*specDisplay4;
	GtkWidget	*specDisplay5;
	GtkWidget	*specDisplay6;
	GtkWidget	*specDisplay7;
	GtkWidget	*specDisplay8;
	GtkWidget	*specDisplay9;

	gint		InChannels;
	gint		OutChannels;
	float 		bypass;

	LV2UI_Write_Function 	write_function;
	LV2UI_Controller 	controller;

} IMeterGui;



static LV2UI_Handle 
instantiateIMeterGui(const struct _LV2UI_Descriptor* descriptor, const char* plugin_uri, const char* bundle_path, LV2UI_Write_Function write_function, LV2UI_Controller controller, LV2UI_Widget* widget, const LV2_Feature* const* features)
{

	IMeterGui *pluginGui = (IMeterGui *)malloc(sizeof(IMeterGui));
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
	file = g_strdup_printf("%s/gtk/inv_meter_gui.xml",bundle_path);
	gtk_builder_add_from_file (builder, file, &err);
	free(file);

	window = GTK_WIDGET (gtk_builder_get_object (builder, "meter_window"));

	/* get pointers to some useful widgets from the design */
	pluginGui->windowContainer = GTK_WIDGET (gtk_builder_get_object (builder, "meter_container"));
	pluginGui->heading = GTK_WIDGET (gtk_builder_get_object (builder, "label_heading"));

	/* add custom widgets */
	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_bypass_toggle"));
	pluginGui->toggleBypass = inv_switch_toggle_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->toggleBypass);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_meter_peak"));
	pluginGui->meterPeak = inv_meter_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->meterPeak);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_meter_vu"));
	pluginGui->meterVU = inv_meter_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->meterVU);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_meter_phase"));
	pluginGui->meterPhase = inv_phase_meter_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->meterPhase);

// temp
	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_spec_display"));
	pluginGui->specDisplay = inv_meter_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->specDisplay);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_spec_display1"));
	pluginGui->specDisplay1 = inv_meter_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->specDisplay1);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_spec_display2"));
	pluginGui->specDisplay2 = inv_meter_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->specDisplay2);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_spec_display3"));
	pluginGui->specDisplay3 = inv_meter_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->specDisplay3);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_spec_display4"));
	pluginGui->specDisplay4 = inv_meter_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->specDisplay4);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_spec_display5"));
	pluginGui->specDisplay5 = inv_meter_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->specDisplay5);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_spec_display6"));
	pluginGui->specDisplay6 = inv_meter_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->specDisplay6);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_spec_display7"));
	pluginGui->specDisplay7 = inv_meter_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->specDisplay7);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_spec_display8"));
	pluginGui->specDisplay8 = inv_meter_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->specDisplay8);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_spec_display9"));
	pluginGui->specDisplay9 = inv_meter_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->specDisplay9);
// end temp

	pluginGui->InChannels	= 2;
	pluginGui->OutChannels	= 2;
	pluginGui->bypass	= 0.0;

	inv_switch_toggle_set_bypass( INV_SWITCH_TOGGLE (pluginGui->toggleBypass), INV_PLUGIN_ACTIVE);
	inv_switch_toggle_set_value( INV_SWITCH_TOGGLE (pluginGui->toggleBypass), INV_SWITCH_TOGGLE_OFF, 0.0);
	inv_switch_toggle_set_colour(INV_SWITCH_TOGGLE (pluginGui->toggleBypass), INV_SWITCH_TOGGLE_OFF, 0.0, 1.0, 0.0);
	inv_switch_toggle_set_text(  INV_SWITCH_TOGGLE (pluginGui->toggleBypass), INV_SWITCH_TOGGLE_OFF, "Active");
	inv_switch_toggle_set_value( INV_SWITCH_TOGGLE (pluginGui->toggleBypass), INV_SWITCH_TOGGLE_ON,  1.0);
	inv_switch_toggle_set_colour(INV_SWITCH_TOGGLE (pluginGui->toggleBypass), INV_SWITCH_TOGGLE_ON,  1.0, 0.0, 0.0);
	inv_switch_toggle_set_text(  INV_SWITCH_TOGGLE (pluginGui->toggleBypass), INV_SWITCH_TOGGLE_ON,  "Bypassed");
	inv_switch_toggle_set_state( INV_SWITCH_TOGGLE (pluginGui->toggleBypass), INV_SWITCH_TOGGLE_OFF);
	g_signal_connect_after(G_OBJECT(pluginGui->toggleBypass),"button-release-event",G_CALLBACK(on_inv_meter_bypass_toggle_button_release),pluginGui);

	inv_meter_set_bypass(INV_METER (pluginGui->meterPeak),INV_PLUGIN_ACTIVE);
	inv_meter_set_mode(INV_METER (pluginGui->meterPeak), INV_METER_DRAW_MODE_TOZERO);
	inv_meter_set_channels(INV_METER (pluginGui->meterPeak), pluginGui->InChannels);
	inv_meter_set_LdB(INV_METER (pluginGui->meterPeak),-90);
	inv_meter_set_RdB(INV_METER (pluginGui->meterPeak),-90);

	inv_meter_set_bypass(INV_METER (pluginGui->meterVU),INV_PLUGIN_ACTIVE);
	inv_meter_set_mode(INV_METER (pluginGui->meterVU), INV_METER_DRAW_MODE_TOZERO);
	inv_meter_set_channels(INV_METER (pluginGui->meterVU), pluginGui->InChannels);
	inv_meter_set_LdB(INV_METER (pluginGui->meterVU),-90);
	inv_meter_set_RdB(INV_METER (pluginGui->meterVU),-90);

// temp
	inv_meter_set_bypass(INV_METER (pluginGui->specDisplay),INV_PLUGIN_ACTIVE);
	inv_meter_set_mode(INV_METER (pluginGui->specDisplay), INV_METER_DRAW_MODE_TOZERO);
	inv_meter_set_channels(INV_METER (pluginGui->specDisplay), 2);
	inv_meter_set_LdB(INV_METER (pluginGui->specDisplay),-90);
	inv_meter_set_RdB(INV_METER (pluginGui->specDisplay),-90);

	inv_meter_set_bypass(INV_METER (pluginGui->specDisplay1),INV_PLUGIN_ACTIVE);
	inv_meter_set_mode(INV_METER (pluginGui->specDisplay1), INV_METER_DRAW_MODE_TOZERO);
	inv_meter_set_channels(INV_METER (pluginGui->specDisplay1), 2);
	inv_meter_set_LdB(INV_METER (pluginGui->specDisplay1),-90);
	inv_meter_set_RdB(INV_METER (pluginGui->specDisplay1),-90);

	inv_meter_set_bypass(INV_METER (pluginGui->specDisplay2),INV_PLUGIN_ACTIVE);
	inv_meter_set_mode(INV_METER (pluginGui->specDisplay2), INV_METER_DRAW_MODE_TOZERO);
	inv_meter_set_channels(INV_METER (pluginGui->specDisplay2), 2);
	inv_meter_set_LdB(INV_METER (pluginGui->specDisplay2),-90);
	inv_meter_set_RdB(INV_METER (pluginGui->specDisplay2),-90);

	inv_meter_set_bypass(INV_METER (pluginGui->specDisplay3),INV_PLUGIN_ACTIVE);
	inv_meter_set_mode(INV_METER (pluginGui->specDisplay3), INV_METER_DRAW_MODE_TOZERO);
	inv_meter_set_channels(INV_METER (pluginGui->specDisplay3), 2);
	inv_meter_set_LdB(INV_METER (pluginGui->specDisplay3),-90);
	inv_meter_set_RdB(INV_METER (pluginGui->specDisplay3),-90);

	inv_meter_set_bypass(INV_METER (pluginGui->specDisplay4),INV_PLUGIN_ACTIVE);
	inv_meter_set_mode(INV_METER (pluginGui->specDisplay4), INV_METER_DRAW_MODE_TOZERO);
	inv_meter_set_channels(INV_METER (pluginGui->specDisplay4), 2);
	inv_meter_set_LdB(INV_METER (pluginGui->specDisplay4),-90);
	inv_meter_set_RdB(INV_METER (pluginGui->specDisplay4),-90);

	inv_meter_set_bypass(INV_METER (pluginGui->specDisplay5),INV_PLUGIN_ACTIVE);
	inv_meter_set_mode(INV_METER (pluginGui->specDisplay5), INV_METER_DRAW_MODE_TOZERO);
	inv_meter_set_channels(INV_METER (pluginGui->specDisplay5), 2);
	inv_meter_set_LdB(INV_METER (pluginGui->specDisplay5),-90);
	inv_meter_set_RdB(INV_METER (pluginGui->specDisplay5),-90);

	inv_meter_set_bypass(INV_METER (pluginGui->specDisplay6),INV_PLUGIN_ACTIVE);
	inv_meter_set_mode(INV_METER (pluginGui->specDisplay6), INV_METER_DRAW_MODE_TOZERO);
	inv_meter_set_channels(INV_METER (pluginGui->specDisplay6), 2);
	inv_meter_set_LdB(INV_METER (pluginGui->specDisplay6),-90);
	inv_meter_set_RdB(INV_METER (pluginGui->specDisplay6),-90);

	inv_meter_set_bypass(INV_METER (pluginGui->specDisplay7),INV_PLUGIN_ACTIVE);
	inv_meter_set_mode(INV_METER (pluginGui->specDisplay7), INV_METER_DRAW_MODE_TOZERO);
	inv_meter_set_channels(INV_METER (pluginGui->specDisplay7), 2);
	inv_meter_set_LdB(INV_METER (pluginGui->specDisplay7),-90);
	inv_meter_set_RdB(INV_METER (pluginGui->specDisplay7),-90);

	inv_meter_set_bypass(INV_METER (pluginGui->specDisplay8),INV_PLUGIN_ACTIVE);
	inv_meter_set_mode(INV_METER (pluginGui->specDisplay8), INV_METER_DRAW_MODE_TOZERO);
	inv_meter_set_channels(INV_METER (pluginGui->specDisplay8), 2);
	inv_meter_set_LdB(INV_METER (pluginGui->specDisplay8),-90);
	inv_meter_set_RdB(INV_METER (pluginGui->specDisplay8),-90);

	inv_meter_set_bypass(INV_METER (pluginGui->specDisplay9),INV_PLUGIN_ACTIVE);
	inv_meter_set_mode(INV_METER (pluginGui->specDisplay9), INV_METER_DRAW_MODE_TOZERO);
	inv_meter_set_channels(INV_METER (pluginGui->specDisplay9), 2);
	inv_meter_set_LdB(INV_METER (pluginGui->specDisplay9),-90);
	inv_meter_set_RdB(INV_METER (pluginGui->specDisplay9),-90);
// end temp

	inv_phase_meter_set_bypass(INV_PHASE_METER (pluginGui->meterPhase),INV_PLUGIN_ACTIVE);
	inv_phase_meter_set_phase(INV_PHASE_METER (pluginGui->meterPhase),0);


	/* strip the parent window from the design so the host can attach its own */
	gtk_widget_ref(pluginGui->windowContainer);
	gtk_container_remove(GTK_CONTAINER(window), pluginGui->windowContainer);

	*widget = (LV2UI_Widget) pluginGui->windowContainer;

	g_object_unref (G_OBJECT (builder));
 
	/* return the instance */
	return pluginGui;
}


static void 
cleanupIMeterGui(LV2UI_Handle ui)
{
	return;
}


static void 
port_eventIMeterGui(LV2UI_Handle ui, uint32_t port, uint32_t buffer_size, uint32_t format, const void*  buffer)
{
	IMeterGui *pluginGui = (IMeterGui *)ui;

	float value;

	if(format==0) 
	{
		value=* (float *) buffer;
		switch(port)
		{
			case IMETER_BYPASS:
				pluginGui->bypass=value;
				if(value <= 0.0) {
					inv_switch_toggle_set_state(INV_SWITCH_TOGGLE (pluginGui->toggleBypass),      INV_SWITCH_TOGGLE_OFF);
					inv_meter_set_bypass(         INV_METER         (pluginGui->meterPeak),       INV_PLUGIN_ACTIVE);
					inv_meter_set_bypass(         INV_METER         (pluginGui->meterVU),         INV_PLUGIN_ACTIVE);
					inv_phase_meter_set_bypass(   INV_PHASE_METER   (pluginGui->meterPhase),      INV_PLUGIN_ACTIVE);
				} else {
					inv_switch_toggle_set_state(INV_SWITCH_TOGGLE (pluginGui->toggleBypass),      INV_SWITCH_TOGGLE_ON);
					inv_meter_set_bypass(         INV_METER         (pluginGui->meterPeak),       INV_PLUGIN_BYPASS);
					inv_meter_set_bypass(         INV_METER         (pluginGui->meterVU),         INV_PLUGIN_BYPASS);
					inv_phase_meter_set_bypass(   INV_PHASE_METER   (pluginGui->meterPhase),      INV_PLUGIN_BYPASS);
				}
				gtk_widget_queue_draw (pluginGui->windowContainer);
				break;
			case IMETER_METER_L:
				inv_meter_set_LdB(INV_METER (pluginGui->meterPeak),value);
				break;
			case IMETER_METER_R:
				inv_meter_set_RdB(INV_METER (pluginGui->meterPeak),value);
				break;
			case IMETER_VU_L:
				inv_meter_set_LdB(INV_METER (pluginGui->meterVU),value);
				break;
			case IMETER_VU_R:
				inv_meter_set_RdB(INV_METER (pluginGui->meterVU),value);
				break;
			case IMETER_METER_PHASE:
				inv_phase_meter_set_phase(INV_PHASE_METER (pluginGui->meterPhase),value);
				break;
			case IMETER_SPEC_20: 
				inv_meter_set_LdB(INV_METER (pluginGui->specDisplay),value);
				break;
			case IMETER_SPEC_25: 
				inv_meter_set_RdB(INV_METER (pluginGui->specDisplay),value);
				break;
			case IMETER_SPEC_31: 
				inv_meter_set_LdB(INV_METER (pluginGui->specDisplay1),value);
				break;
			case IMETER_SPEC_40: 
				inv_meter_set_RdB(INV_METER (pluginGui->specDisplay1),value);
				break;
			case IMETER_SPEC_50:
				inv_meter_set_LdB(INV_METER (pluginGui->specDisplay2),value);
				break;
			case IMETER_SPEC_63: 
				inv_meter_set_RdB(INV_METER (pluginGui->specDisplay2),value);
				break;
			case IMETER_SPEC_80: 
				inv_meter_set_LdB(INV_METER (pluginGui->specDisplay3),value);
				break;
			case IMETER_SPEC_100: 
				inv_meter_set_RdB(INV_METER (pluginGui->specDisplay3),value);
				break;
			case IMETER_SPEC_125: 
				inv_meter_set_LdB(INV_METER (pluginGui->specDisplay4),value);
				break;
			case IMETER_SPEC_160: 
				inv_meter_set_RdB(INV_METER (pluginGui->specDisplay4),value);
				break;
			case IMETER_SPEC_200: 
				inv_meter_set_LdB(INV_METER (pluginGui->specDisplay5),value);
				break;
			case IMETER_SPEC_250: 
				inv_meter_set_RdB(INV_METER (pluginGui->specDisplay5),value);
				break;
			case IMETER_SPEC_315: 
				inv_meter_set_LdB(INV_METER (pluginGui->specDisplay6),value);
				break;
			case IMETER_SPEC_400: 
				inv_meter_set_RdB(INV_METER (pluginGui->specDisplay6),value);
				break;
			case IMETER_SPEC_500: 
				inv_meter_set_LdB(INV_METER (pluginGui->specDisplay7),value);
				break;
			case IMETER_SPEC_630: 
				inv_meter_set_RdB(INV_METER (pluginGui->specDisplay7),value);
				break;
			case IMETER_SPEC_800: 
				inv_meter_set_LdB(INV_METER (pluginGui->specDisplay8),value);
				break;
			case IMETER_SPEC_1000: 
				inv_meter_set_RdB(INV_METER (pluginGui->specDisplay8),value);
				break;
			case IMETER_SPEC_1250: 
				inv_meter_set_LdB(INV_METER (pluginGui->specDisplay9),value);
				break;
			case IMETER_SPEC_1600:
				inv_meter_set_RdB(INV_METER (pluginGui->specDisplay9),value);
				break;
			case IMETER_SPEC_2000:
			case IMETER_SPEC_2500: 
			case IMETER_SPEC_3150: 
			case IMETER_SPEC_4000: 
			case IMETER_SPEC_5000:
			case IMETER_SPEC_6300:	
			case IMETER_SPEC_8000:
			case IMETER_SPEC_10000:  
			case IMETER_SPEC_12500:  
 			case IMETER_SPEC_16000: 
			case IMETER_SPEC_20000:  
				break;
		}
	}
}


static void 
init()
{
	IMeterGuiDescriptor =
	 (LV2UI_Descriptor *)malloc(sizeof(LV2UI_Descriptor));

	IMeterGuiDescriptor->URI 		= IMETER_GUI_URI;
	IMeterGuiDescriptor->instantiate 	= instantiateIMeterGui;
	IMeterGuiDescriptor->cleanup		= cleanupIMeterGui;
	IMeterGuiDescriptor->port_event		= port_eventIMeterGui;
	IMeterGuiDescriptor->extension_data 	= NULL;

}


const LV2UI_Descriptor* lv2ui_descriptor(uint32_t index)
{
	if (!IMeterGuiDescriptor) init();

	switch (index) {
		case 0:
			return IMeterGuiDescriptor;
	default:
		return NULL;
	}
}


/*****************************************************************************/

static void 
on_inv_meter_bypass_toggle_button_release(GtkWidget *widget, GdkEvent *event, gpointer data)
{

	IMeterGui *pluginGui = (IMeterGui *) data;

	pluginGui->bypass=inv_switch_toggle_get_value(INV_SWITCH_TOGGLE (widget));
	(*pluginGui->write_function)(pluginGui->controller, IMETER_BYPASS, 4, 0, &pluginGui->bypass);
	return;
}


