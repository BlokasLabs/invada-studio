/* 

    This LV2 extension provides delay gui's

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
#include "widgets/knob.h"
#include "widgets/lamp.h"
#include "widgets/meter-peak.h"
#include "widgets/switch-toggle.h"
#include "../plugin/inv_delay.h"
#include "inv_delay_gui.h"


static LV2UI_Descriptor *IDelayGuiDescriptor = NULL;

typedef struct {
	GtkWidget	*windowContainer;
	GtkWidget	*heading;
	GtkWidget	*toggleBypass;
	GtkWidget	*meterIn;
	GtkWidget	*meterOut;
	GtkWidget	*toggleMode;
	GtkWidget	*toggleMungeMode;
	GtkWidget	*knobCycle;
	GtkWidget	*knobWidth;
	GtkWidget	*lampLFO;
	GtkWidget	*knobMunge;
	GtkWidget	*knobDelay1;
	GtkWidget	*knobFB1;
	GtkWidget	*knobPan1;
	GtkWidget	*knobVol1;
	GtkWidget	*knobDelay2;
	GtkWidget	*knobFB2;
	GtkWidget	*knobPan2;
	GtkWidget	*knobVol2;



	gint		InChannels;
	gint		OutChannels;
	float 		bypass;
	float 		mode;
	float 		mungemode;
	float 		munge;
	float 		cycle;
	float 		width;
	float 		delay1;
	float		fb1;
	float		pan1;
	float		vol1;
	float 		delay2;
	float		fb2;
	float		pan2;
	float		vol2;

	LV2UI_Write_Function 	write_function;
	LV2UI_Controller 	controller;

} IDelayGui;



static LV2UI_Handle 
instantiateIDelayGui(const struct _LV2UI_Descriptor* descriptor, const char* plugin_uri, const char* bundle_path, LV2UI_Write_Function write_function, LV2UI_Controller controller, LV2UI_Widget* widget, const LV2_Feature* const* features)
{
	IDelayGui *pluginGui = (IDelayGui *)malloc(sizeof(IDelayGui));
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
	file = g_strdup_printf("%s/gtk/inv_delay_gui.xml",bundle_path);
	gtk_builder_add_from_file (builder, file, &err);
	free(file);

	window = GTK_WIDGET (gtk_builder_get_object (builder, "delay_window"));

	/* get pointers to some useful widgets from the design */
	pluginGui->windowContainer = GTK_WIDGET (gtk_builder_get_object (builder, "delay_container"));
	pluginGui->heading = GTK_WIDGET (gtk_builder_get_object (builder, "label_heading"));

	/* add custom widgets */
	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_bypass_toggle"));
	pluginGui->toggleBypass = inv_switch_toggle_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->toggleBypass);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_meter_in"));
	pluginGui->meterIn = inv_meter_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->meterIn);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_meter_out"));
	pluginGui->meterOut = inv_meter_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->meterOut);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_mode_toggle"));
	pluginGui->toggleMode = inv_switch_toggle_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->toggleMode);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_mungemode_toggle"));
	pluginGui->toggleMungeMode = inv_switch_toggle_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->toggleMungeMode);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_munge_knob"));
	pluginGui->knobMunge = inv_knob_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->knobMunge);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_cycle_knob"));
	pluginGui->knobCycle = inv_knob_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->knobCycle);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_width_knob"));
	pluginGui->knobWidth = inv_knob_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->knobWidth);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_cycle_lamp"));
	pluginGui->lampLFO = inv_lamp_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->lampLFO);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_delay1_knob"));
	pluginGui->knobDelay1 = inv_knob_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->knobDelay1);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_fb1_knob"));
	pluginGui->knobFB1 = inv_knob_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->knobFB1);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_pan1_knob"));
	pluginGui->knobPan1 = inv_knob_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->knobPan1);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_vol1_knob"));
	pluginGui->knobVol1 = inv_knob_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->knobVol1);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_delay2_knob"));
	pluginGui->knobDelay2 = inv_knob_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->knobDelay2);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_fb2_knob"));
	pluginGui->knobFB2 = inv_knob_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->knobFB2);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_pan2_knob"));
	pluginGui->knobPan2 = inv_knob_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->knobPan2);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_vol2_knob"));
	pluginGui->knobVol2 = inv_knob_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->knobVol2);

	/* customise for the plugin */
	if(!strcmp(plugin_uri,IDELAY_MONO_URI)) 
	{
		gtk_label_set_markup (GTK_LABEL (pluginGui->heading), "<b>Delay Munge (mono in)</b>");
	}
	if(!strcmp(plugin_uri,IDELAY_SUM_URI)) 
	{
		gtk_label_set_markup (GTK_LABEL (pluginGui->heading), "<b>Delay Munge (sum L+R in)</b>");
	}

	pluginGui->InChannels	= 1;
	pluginGui->OutChannels	= 2;
	pluginGui->bypass	= 0.0;
	pluginGui->mode		= 0.0;
	pluginGui->mungemode	= 0.0;
	pluginGui->munge	= 50.0;
	pluginGui->cycle	= 20.0;
	pluginGui->width	= 0.0;
	pluginGui->delay1	= 300.0;
	pluginGui->fb1		= 50.0;
	pluginGui->pan1		= -0.7;
	pluginGui->vol1		= 100.0;
	pluginGui->delay2	= 200.0;
	pluginGui->fb2		= 50.0;
	pluginGui->pan2		= 0.7;
	pluginGui->vol2		= 100.0;

	inv_switch_toggle_set_bypass( INV_SWITCH_TOGGLE (pluginGui->toggleBypass), INV_PLUGIN_ACTIVE);
	inv_switch_toggle_set_value(  INV_SWITCH_TOGGLE (pluginGui->toggleBypass), INV_SWITCH_TOGGLE_OFF, 0.0);
	inv_switch_toggle_set_colour( INV_SWITCH_TOGGLE (pluginGui->toggleBypass), INV_SWITCH_TOGGLE_OFF, 0.0, 1.0, 0.0);
	inv_switch_toggle_set_text(   INV_SWITCH_TOGGLE (pluginGui->toggleBypass), INV_SWITCH_TOGGLE_OFF, "Active");
	inv_switch_toggle_set_value(  INV_SWITCH_TOGGLE (pluginGui->toggleBypass), INV_SWITCH_TOGGLE_ON,  1.0);
	inv_switch_toggle_set_colour( INV_SWITCH_TOGGLE (pluginGui->toggleBypass), INV_SWITCH_TOGGLE_ON,  1.0, 0.0, 0.0);
	inv_switch_toggle_set_text(   INV_SWITCH_TOGGLE (pluginGui->toggleBypass), INV_SWITCH_TOGGLE_ON,  "Bypassed");
	inv_switch_toggle_set_state(  INV_SWITCH_TOGGLE (pluginGui->toggleBypass), INV_SWITCH_TOGGLE_OFF);
	inv_switch_toggle_set_tooltip(INV_SWITCH_TOGGLE (pluginGui->toggleBypass), "<small><b>Description:</b> This switch bypasses the plugin.\n<b>Usage:</b> Click to toggle between values.</small>");
	g_signal_connect_after(G_OBJECT(pluginGui->toggleBypass),"button-release-event",G_CALLBACK(on_inv_delay_bypass_toggle_button_release),pluginGui);

	inv_meter_set_bypass(INV_METER (pluginGui->meterIn),INV_PLUGIN_ACTIVE);
	inv_meter_set_mode(INV_METER (pluginGui->meterIn), INV_METER_DRAW_MODE_TOZERO);
	inv_meter_set_channels(INV_METER (pluginGui->meterIn), pluginGui->InChannels);
	inv_meter_set_LdB(INV_METER (pluginGui->meterIn),-90);

	inv_meter_set_bypass(INV_METER (pluginGui->meterOut),INV_PLUGIN_ACTIVE);
	inv_meter_set_mode(INV_METER (pluginGui->meterOut), INV_METER_DRAW_MODE_TOZERO);
	inv_meter_set_channels(INV_METER (pluginGui->meterOut), pluginGui->OutChannels);
	inv_meter_set_LdB(INV_METER (pluginGui->meterOut),-90);
	inv_meter_set_RdB(INV_METER (pluginGui->meterOut),-90);

	inv_switch_toggle_set_bypass( INV_SWITCH_TOGGLE (pluginGui->toggleMode), INV_PLUGIN_ACTIVE);
	inv_switch_toggle_set_value(  INV_SWITCH_TOGGLE (pluginGui->toggleMode), INV_SWITCH_TOGGLE_OFF, 0.0);
	inv_switch_toggle_set_colour (INV_SWITCH_TOGGLE (pluginGui->toggleMode), INV_SWITCH_TOGGLE_OFF, 0.0, 1.0, 0.0);
	inv_switch_toggle_set_text(   INV_SWITCH_TOGGLE (pluginGui->toggleMode), INV_SWITCH_TOGGLE_OFF, "Discrete");
	inv_switch_toggle_set_value(  INV_SWITCH_TOGGLE (pluginGui->toggleMode), INV_SWITCH_TOGGLE_ON,  1.0);
	inv_switch_toggle_set_colour( INV_SWITCH_TOGGLE (pluginGui->toggleMode), INV_SWITCH_TOGGLE_ON,  0.0, 1.0, 0.0);
	inv_switch_toggle_set_text(   INV_SWITCH_TOGGLE (pluginGui->toggleMode), INV_SWITCH_TOGGLE_ON,  "Ping-Pong");
	inv_switch_toggle_set_state(  INV_SWITCH_TOGGLE (pluginGui->toggleMode), INV_SWITCH_TOGGLE_OFF);
	inv_switch_toggle_set_tooltip(INV_SWITCH_TOGGLE (pluginGui->toggleMode), "<small><b>Description:</b> This switch changes the mode of the delay between discrete channels and ping-ping.\n<b>Usage:</b> Click to toggle between values.</small>");
	g_signal_connect_after(G_OBJECT(pluginGui->toggleMode),"button-release-event",G_CALLBACK(on_inv_delay_mode_toggle_button_release),pluginGui);

	inv_switch_toggle_set_bypass( INV_SWITCH_TOGGLE (pluginGui->toggleMungeMode), INV_PLUGIN_ACTIVE);
	inv_switch_toggle_set_value(  INV_SWITCH_TOGGLE (pluginGui->toggleMungeMode), INV_SWITCH_TOGGLE_OFF, 0.0);
	inv_switch_toggle_set_colour (INV_SWITCH_TOGGLE (pluginGui->toggleMungeMode), INV_SWITCH_TOGGLE_OFF, 0.0, 1.0, 0.0);
	inv_switch_toggle_set_text(   INV_SWITCH_TOGGLE (pluginGui->toggleMungeMode), INV_SWITCH_TOGGLE_OFF, "Type 1");
	inv_switch_toggle_set_value(  INV_SWITCH_TOGGLE (pluginGui->toggleMungeMode), INV_SWITCH_TOGGLE_ON,  1.0);
	inv_switch_toggle_set_colour( INV_SWITCH_TOGGLE (pluginGui->toggleMungeMode), INV_SWITCH_TOGGLE_ON,  0.0, 1.0, 0.0);
	inv_switch_toggle_set_text(   INV_SWITCH_TOGGLE (pluginGui->toggleMungeMode), INV_SWITCH_TOGGLE_ON,  "Type 2");
	inv_switch_toggle_set_state(  INV_SWITCH_TOGGLE (pluginGui->toggleMungeMode), INV_SWITCH_TOGGLE_OFF);
	inv_switch_toggle_set_tooltip(INV_SWITCH_TOGGLE (pluginGui->toggleMungeMode), "<small><b>Description:</b> This switch changes the munge type used on the signal in the feedback loop.\n<b>Usage:</b> Click to toggle between values.</small>");
	g_signal_connect_after(G_OBJECT(pluginGui->toggleMungeMode),"button-release-event",G_CALLBACK(on_inv_delay_mungemode_toggle_button_release),pluginGui);

	inv_knob_set_bypass(  INV_KNOB (pluginGui->knobMunge), INV_PLUGIN_ACTIVE);
	inv_knob_set_size(    INV_KNOB (pluginGui->knobMunge), INV_KNOB_SIZE_MEDIUM);
	inv_knob_set_curve(   INV_KNOB (pluginGui->knobMunge), INV_KNOB_CURVE_LINEAR);
	inv_knob_set_markings(INV_KNOB (pluginGui->knobMunge), INV_KNOB_MARKINGS_5); 
	inv_knob_set_units(   INV_KNOB (pluginGui->knobMunge), "%");
	inv_knob_set_min(     INV_KNOB (pluginGui->knobMunge), 0.0);
	inv_knob_set_max(     INV_KNOB (pluginGui->knobMunge), 100.0);
	inv_knob_set_value(   INV_KNOB (pluginGui->knobMunge), pluginGui->munge);
	inv_knob_set_tooltip( INV_KNOB (pluginGui->knobMunge), "<small><b>Description:</b> This knob sets the amount of munge.\n<b>Usage:</b> Click and drag vertically to change value, hortizontally to change the sensitvity.</small>");
	g_signal_connect_after(G_OBJECT(pluginGui->knobMunge),"motion-notify-event",G_CALLBACK(on_inv_delay_munge_knob_motion),pluginGui);

	inv_knob_set_bypass(  INV_KNOB (pluginGui->knobCycle), INV_PLUGIN_ACTIVE);
	inv_knob_set_size(    INV_KNOB (pluginGui->knobCycle), INV_KNOB_SIZE_MEDIUM);
	inv_knob_set_curve(   INV_KNOB (pluginGui->knobCycle), INV_KNOB_CURVE_LOG);
	inv_knob_set_markings(INV_KNOB (pluginGui->knobCycle), INV_KNOB_MARKINGS_3); 
	inv_knob_set_human(   INV_KNOB (pluginGui->knobCycle)); 
	inv_knob_set_units(   INV_KNOB (pluginGui->knobCycle), "s");
	inv_knob_set_min(     INV_KNOB (pluginGui->knobCycle), 2.0);
	inv_knob_set_max(     INV_KNOB (pluginGui->knobCycle), 200.0);
	inv_knob_set_value(   INV_KNOB (pluginGui->knobCycle), pluginGui->cycle);
	inv_knob_set_tooltip( INV_KNOB (pluginGui->knobCycle), "<small><b>Description:</b> This knob sets the period of the LFO.\n<b>Usage:</b> Click and drag vertically to change value, hortizontally to change the sensitvity.</small>");
	g_signal_connect_after(G_OBJECT(pluginGui->knobCycle), "motion-notify-event",G_CALLBACK(on_inv_delay_cycle_knob_motion),pluginGui);

	inv_knob_set_bypass(  INV_KNOB (pluginGui->knobWidth), INV_PLUGIN_ACTIVE);
	inv_knob_set_size(    INV_KNOB (pluginGui->knobWidth), INV_KNOB_SIZE_MEDIUM);
	inv_knob_set_curve(   INV_KNOB (pluginGui->knobWidth), INV_KNOB_CURVE_LINEAR);
	inv_knob_set_markings(INV_KNOB (pluginGui->knobWidth), INV_KNOB_MARKINGS_5); 
	inv_knob_set_units(   INV_KNOB (pluginGui->knobWidth), "%");
	inv_knob_set_min(     INV_KNOB (pluginGui->knobWidth), 0.0);
	inv_knob_set_max(     INV_KNOB (pluginGui->knobWidth), 100.0);
	inv_knob_set_value(   INV_KNOB (pluginGui->knobWidth), pluginGui->width);
	inv_knob_set_tooltip( INV_KNOB (pluginGui->knobWidth), "<small><b>Description:</b> This knob sets the width of the LFO.\n<b>Usage:</b> Click and drag vertically to change value, hortizontally to change the sensitvity.</small>");
	g_signal_connect_after(G_OBJECT(pluginGui->knobWidth),"motion-notify-event",G_CALLBACK(on_inv_delay_width_knob_motion),pluginGui);

	inv_lamp_set_value(INV_LAMP (pluginGui->lampLFO),0.0);
	inv_lamp_set_scale(INV_LAMP (pluginGui->lampLFO),1.0);
	inv_lamp_set_tooltip(INV_LAMP (pluginGui->lampLFO), "<small>This shows the LFO cycle.</small>");


	inv_knob_set_bypass(  INV_KNOB (pluginGui->knobDelay1), INV_PLUGIN_ACTIVE);
	inv_knob_set_size(    INV_KNOB (pluginGui->knobDelay1), INV_KNOB_SIZE_SMALL);
	inv_knob_set_curve(   INV_KNOB (pluginGui->knobDelay1), INV_KNOB_CURVE_LOG);
	inv_knob_set_markings(INV_KNOB (pluginGui->knobDelay1), INV_KNOB_MARKINGS_3); 
	inv_knob_set_human(   INV_KNOB (pluginGui->knobDelay1));
	inv_knob_set_units(   INV_KNOB (pluginGui->knobDelay1), "s");
	inv_knob_set_min(     INV_KNOB (pluginGui->knobDelay1), 0.02);
	inv_knob_set_max(     INV_KNOB (pluginGui->knobDelay1), 2.0);
	inv_knob_set_value(   INV_KNOB (pluginGui->knobDelay1), pluginGui->delay1);
	inv_knob_set_tooltip( INV_KNOB (pluginGui->knobDelay1), "<small><b>Description:</b> This knob sets the delay time for the first delay.\n<b>Usage:</b> Click and drag vertically to change value, hortizontally to change the sensitvity.</small>");
	g_signal_connect_after(G_OBJECT(pluginGui->knobDelay1),"motion-notify-event",G_CALLBACK(on_inv_delay_delay1_knob_motion),pluginGui);

	inv_knob_set_bypass(  INV_KNOB (pluginGui->knobFB1), INV_PLUGIN_ACTIVE);
	inv_knob_set_size(    INV_KNOB (pluginGui->knobFB1), INV_KNOB_SIZE_SMALL);
	inv_knob_set_curve(   INV_KNOB (pluginGui->knobFB1), INV_KNOB_CURVE_LINEAR);
	inv_knob_set_markings(INV_KNOB (pluginGui->knobFB1), INV_KNOB_MARKINGS_5); 
	inv_knob_set_units(   INV_KNOB (pluginGui->knobFB1), "%");
	inv_knob_set_min(     INV_KNOB (pluginGui->knobFB1), 0.0);
	inv_knob_set_max(     INV_KNOB (pluginGui->knobFB1), 133.3333);
	inv_knob_set_value(   INV_KNOB (pluginGui->knobFB1), pluginGui->fb1);
	inv_knob_set_tooltip( INV_KNOB (pluginGui->knobFB1), "<small><b>Description:</b> This knob sets the amount of feedback for the first delay.\n<b>Usage:</b> Click and drag vertically to change value, hortizontally to change the sensitvity.</small>");
	g_signal_connect_after(G_OBJECT(pluginGui->knobFB1),"motion-notify-event",G_CALLBACK(on_inv_delay_fb1_knob_motion),pluginGui);

	inv_knob_set_bypass(  INV_KNOB (pluginGui->knobPan1), INV_PLUGIN_ACTIVE);
	inv_knob_set_size(    INV_KNOB (pluginGui->knobPan1), INV_KNOB_SIZE_SMALL);
	inv_knob_set_curve(   INV_KNOB (pluginGui->knobPan1), INV_KNOB_CURVE_LINEAR);
	inv_knob_set_markings(INV_KNOB (pluginGui->knobPan1), INV_KNOB_MARKINGS_PAN); 
	inv_knob_set_min(     INV_KNOB (pluginGui->knobPan1), -1.0);
	inv_knob_set_max(     INV_KNOB (pluginGui->knobPan1), 1.0);
	inv_knob_set_value(   INV_KNOB (pluginGui->knobPan1), pluginGui->pan1);
	inv_knob_set_tooltip( INV_KNOB (pluginGui->knobPan1), "<small><b>Description:</b> This knob sets the position within the output mix for the first delay.\n<b>Usage:</b> Click and drag vertically to change value, hortizontally to change the sensitvity.</small>");
	g_signal_connect_after(G_OBJECT(pluginGui->knobPan1),"motion-notify-event",G_CALLBACK(on_inv_delay_pan1_knob_motion),pluginGui);

	inv_knob_set_bypass(  INV_KNOB (pluginGui->knobVol1), INV_PLUGIN_ACTIVE);
	inv_knob_set_size(    INV_KNOB (pluginGui->knobVol1), INV_KNOB_SIZE_SMALL);
	inv_knob_set_curve(   INV_KNOB (pluginGui->knobVol1), INV_KNOB_CURVE_LINEAR);
	inv_knob_set_markings(INV_KNOB (pluginGui->knobVol1), INV_KNOB_MARKINGS_4); 
	inv_knob_set_units(   INV_KNOB (pluginGui->knobVol1), "%");
	inv_knob_set_min(     INV_KNOB (pluginGui->knobVol1), 0.0);
	inv_knob_set_max(     INV_KNOB (pluginGui->knobVol1), 100.0);
	inv_knob_set_value(   INV_KNOB (pluginGui->knobVol1), pluginGui->vol1);
	inv_knob_set_tooltip( INV_KNOB (pluginGui->knobVol1), "<small><b>Description:</b> This knob sets the volume in the output mix for the first delay.\n<b>Usage:</b> Click and drag vertically to change value, hortizontally to change the sensitvity.</small>");
	g_signal_connect_after(G_OBJECT(pluginGui->knobVol1),"motion-notify-event",G_CALLBACK(on_inv_delay_vol1_knob_motion),pluginGui);

	inv_knob_set_bypass(  INV_KNOB (pluginGui->knobDelay2), INV_PLUGIN_ACTIVE);
	inv_knob_set_size(    INV_KNOB (pluginGui->knobDelay2), INV_KNOB_SIZE_SMALL);
	inv_knob_set_curve(   INV_KNOB (pluginGui->knobDelay2), INV_KNOB_CURVE_LOG);
	inv_knob_set_markings(INV_KNOB (pluginGui->knobDelay2), INV_KNOB_MARKINGS_3); 
	inv_knob_set_human(   INV_KNOB (pluginGui->knobDelay2));
	inv_knob_set_units(   INV_KNOB (pluginGui->knobDelay2), "s");
	inv_knob_set_min(     INV_KNOB (pluginGui->knobDelay2), 0.02);
	inv_knob_set_max(     INV_KNOB (pluginGui->knobDelay2), 2.0);
	inv_knob_set_value(   INV_KNOB (pluginGui->knobDelay2), pluginGui->delay2);
	inv_knob_set_tooltip( INV_KNOB (pluginGui->knobDelay2), "<small><b>Description:</b> This knob sets the delay time for the second delay.\n<b>Usage:</b> Click and drag vertically to change value, hortizontally to change the sensitvity.</small>");
	g_signal_connect_after(G_OBJECT(pluginGui->knobDelay2),"motion-notify-event",G_CALLBACK(on_inv_delay_delay2_knob_motion),pluginGui);

	inv_knob_set_bypass(  INV_KNOB (pluginGui->knobFB2), INV_PLUGIN_ACTIVE);
	inv_knob_set_size(    INV_KNOB (pluginGui->knobFB2), INV_KNOB_SIZE_SMALL);
	inv_knob_set_curve(   INV_KNOB (pluginGui->knobFB2), INV_KNOB_CURVE_LINEAR);
	inv_knob_set_markings(INV_KNOB (pluginGui->knobFB2), INV_KNOB_MARKINGS_5); 
	inv_knob_set_units(   INV_KNOB (pluginGui->knobFB2), "%");
	inv_knob_set_min(     INV_KNOB (pluginGui->knobFB2), 0.0);
	inv_knob_set_max(     INV_KNOB (pluginGui->knobFB2), 133.3333);
	inv_knob_set_value(   INV_KNOB (pluginGui->knobFB2), pluginGui->fb2);
	inv_knob_set_tooltip( INV_KNOB (pluginGui->knobFB2), "<small><b>Description:</b> This knob sets the amount of feedback for the second delay.\n<b>Usage:</b> Click and drag vertically to change value, hortizontally to change the sensitvity.</small>");
	g_signal_connect_after(G_OBJECT(pluginGui->knobFB2),"motion-notify-event",G_CALLBACK(on_inv_delay_fb2_knob_motion),pluginGui);

	inv_knob_set_bypass(  INV_KNOB (pluginGui->knobPan2), INV_PLUGIN_ACTIVE);
	inv_knob_set_size(    INV_KNOB (pluginGui->knobPan2), INV_KNOB_SIZE_SMALL);
	inv_knob_set_curve(   INV_KNOB (pluginGui->knobPan2), INV_KNOB_CURVE_LINEAR);
	inv_knob_set_markings(INV_KNOB (pluginGui->knobPan2), INV_KNOB_MARKINGS_PAN); 
	inv_knob_set_min(     INV_KNOB (pluginGui->knobPan2), -1.0);
	inv_knob_set_max(     INV_KNOB (pluginGui->knobPan2), 1.0);
	inv_knob_set_value(   INV_KNOB (pluginGui->knobPan2), pluginGui->pan2);
	inv_knob_set_tooltip( INV_KNOB (pluginGui->knobPan2), "<small><b>Description:</b> This knob sets the position within the output mix for the second delay.\n<b>Usage:</b> Click and drag vertically to change value, hortizontally to change the sensitvity.</small>");
	g_signal_connect_after(G_OBJECT(pluginGui->knobPan2),"motion-notify-event",G_CALLBACK(on_inv_delay_pan2_knob_motion),pluginGui);

	inv_knob_set_bypass(  INV_KNOB (pluginGui->knobVol2), INV_PLUGIN_ACTIVE);
	inv_knob_set_size(    INV_KNOB (pluginGui->knobVol2), INV_KNOB_SIZE_SMALL);
	inv_knob_set_curve(   INV_KNOB (pluginGui->knobVol2), INV_KNOB_CURVE_LINEAR);
	inv_knob_set_markings(INV_KNOB (pluginGui->knobVol2), INV_KNOB_MARKINGS_4); 
	inv_knob_set_units(   INV_KNOB (pluginGui->knobVol2), "%");
	inv_knob_set_min(     INV_KNOB (pluginGui->knobVol2), 0.0);
	inv_knob_set_max(     INV_KNOB (pluginGui->knobVol2), 100.0);
	inv_knob_set_value(   INV_KNOB (pluginGui->knobVol2), pluginGui->vol2);
	inv_knob_set_tooltip( INV_KNOB (pluginGui->knobVol2), "<small><b>Description:</b> This knob sets the volume in the output mix for the second delay.\n<b>Usage:</b> Click and drag vertically to change value, hortizontally to change the sensitvity.</small>");
	g_signal_connect_after(G_OBJECT(pluginGui->knobVol2),"motion-notify-event",G_CALLBACK(on_inv_delay_vol2_knob_motion),pluginGui);


	/* strip the parent window from the design so the host can attach its own */
	gtk_widget_ref(pluginGui->windowContainer);
	gtk_container_remove(GTK_CONTAINER(window), pluginGui->windowContainer);

	*widget = (LV2UI_Widget) pluginGui->windowContainer;

	g_object_unref (G_OBJECT (builder));
             
	/* return the instance */
	return pluginGui;
}


static void 
cleanupIDelayGui(LV2UI_Handle ui)
{
	return;
}


static void 
port_eventIDelayGui(LV2UI_Handle ui, uint32_t port, uint32_t buffer_size, uint32_t format, const void*  buffer)
{
	IDelayGui *pluginGui = (IDelayGui *)ui;

	float value;

	if(format==0) 
	{
		value=* (float *) buffer;
		switch(port)
		{
			case IDELAY_BYPASS:
				pluginGui->bypass=value;
				if(value <= 0.5) {
					inv_switch_toggle_set_state(INV_SWITCH_TOGGLE (pluginGui->toggleBypass),      INV_SWITCH_TOGGLE_OFF);
					inv_meter_set_bypass(         INV_METER         (pluginGui->meterIn),         INV_PLUGIN_ACTIVE);
					inv_meter_set_bypass(         INV_METER         (pluginGui->meterOut),        INV_PLUGIN_ACTIVE);
					inv_switch_toggle_set_bypass( INV_SWITCH_TOGGLE (pluginGui->toggleMode),      INV_PLUGIN_ACTIVE);
					inv_switch_toggle_set_bypass( INV_SWITCH_TOGGLE (pluginGui->toggleMungeMode), INV_PLUGIN_ACTIVE);
					inv_knob_set_bypass(          INV_KNOB          (pluginGui->knobMunge),       INV_PLUGIN_ACTIVE);
					inv_knob_set_bypass(          INV_KNOB          (pluginGui->knobDelay1),      INV_PLUGIN_ACTIVE);
					inv_knob_set_bypass(          INV_KNOB          (pluginGui->knobFB1),         INV_PLUGIN_ACTIVE);
					inv_knob_set_bypass(          INV_KNOB          (pluginGui->knobPan1),        INV_PLUGIN_ACTIVE);
					inv_knob_set_bypass(          INV_KNOB          (pluginGui->knobVol1),        INV_PLUGIN_ACTIVE);
					inv_knob_set_bypass(          INV_KNOB          (pluginGui->knobDelay2),      INV_PLUGIN_ACTIVE);
					inv_knob_set_bypass(          INV_KNOB          (pluginGui->knobFB2),         INV_PLUGIN_ACTIVE);
					inv_knob_set_bypass(          INV_KNOB          (pluginGui->knobPan2),        INV_PLUGIN_ACTIVE);
					inv_knob_set_bypass(          INV_KNOB          (pluginGui->knobVol2),        INV_PLUGIN_ACTIVE);
				} else {
					inv_switch_toggle_set_state(INV_SWITCH_TOGGLE (pluginGui->toggleBypass),      INV_SWITCH_TOGGLE_ON);
					inv_meter_set_bypass(         INV_METER         (pluginGui->meterIn),         INV_PLUGIN_BYPASS);
					inv_meter_set_bypass(         INV_METER         (pluginGui->meterOut),        INV_PLUGIN_BYPASS);
					inv_switch_toggle_set_bypass( INV_SWITCH_TOGGLE (pluginGui->toggleMode),      INV_PLUGIN_BYPASS);
					inv_switch_toggle_set_bypass( INV_SWITCH_TOGGLE (pluginGui->toggleMungeMode), INV_PLUGIN_BYPASS);
					inv_knob_set_bypass(          INV_KNOB          (pluginGui->knobMunge),       INV_PLUGIN_BYPASS);
					inv_knob_set_bypass(          INV_KNOB          (pluginGui->knobDelay1),      INV_PLUGIN_BYPASS);
					inv_knob_set_bypass(          INV_KNOB          (pluginGui->knobFB1),         INV_PLUGIN_BYPASS);
					inv_knob_set_bypass(          INV_KNOB          (pluginGui->knobPan1),        INV_PLUGIN_BYPASS);
					inv_knob_set_bypass(          INV_KNOB          (pluginGui->knobVol1),        INV_PLUGIN_BYPASS);
					inv_knob_set_bypass(          INV_KNOB          (pluginGui->knobDelay2),      INV_PLUGIN_BYPASS);
					inv_knob_set_bypass(          INV_KNOB          (pluginGui->knobFB2),         INV_PLUGIN_BYPASS);
					inv_knob_set_bypass(          INV_KNOB          (pluginGui->knobPan2),        INV_PLUGIN_BYPASS);
					inv_knob_set_bypass(          INV_KNOB          (pluginGui->knobVol2),        INV_PLUGIN_BYPASS);
				}
				break;
			case IDELAY_MODE:
				pluginGui->mode=value;
				if(value <= 0.5) {
					inv_switch_toggle_set_state(INV_SWITCH_TOGGLE (pluginGui->toggleMode), INV_SWITCH_TOGGLE_OFF);
				} else {
					inv_switch_toggle_set_state(INV_SWITCH_TOGGLE (pluginGui->toggleMode), INV_SWITCH_TOGGLE_ON);
				}
				break;
			case IDELAY_MUNGEMODE:
				pluginGui->mungemode=value;
				if(value <= 0.5) {
					inv_switch_toggle_set_state(INV_SWITCH_TOGGLE (pluginGui->toggleMungeMode), INV_SWITCH_TOGGLE_OFF);
				} else {
					inv_switch_toggle_set_state(INV_SWITCH_TOGGLE (pluginGui->toggleMungeMode), INV_SWITCH_TOGGLE_ON);
				}
				break;
			case IDELAY_MUNGE:
				pluginGui->munge=value;
				inv_knob_set_value(INV_KNOB (pluginGui->knobMunge), pluginGui->munge);
				break;
			case IDELAY_LFO_CYCLE:
				pluginGui->cycle=value;
				inv_knob_set_value(INV_KNOB (pluginGui->knobCycle), pluginGui->cycle);
				break;
			case IDELAY_LFO_WIDTH:
				pluginGui->width=value;
				inv_knob_set_value(INV_KNOB (pluginGui->knobWidth), pluginGui->width);
				break;
			case IDELAY_LAMP_LFO:
				inv_lamp_set_value(INV_LAMP (pluginGui->lampLFO),value);
				break;
			case IDELAY_1_DELAY:
				pluginGui->delay1=value;
				inv_knob_set_value(INV_KNOB (pluginGui->knobDelay1), pluginGui->delay1);
				break;
			case IDELAY_1_FB:
				pluginGui->fb1=value;
				inv_knob_set_value(INV_KNOB (pluginGui->knobFB1), pluginGui->fb1);
				break;
			case IDELAY_1_PAN:
				pluginGui->pan1=value;
				inv_knob_set_value(INV_KNOB (pluginGui->knobPan1), pluginGui->pan1);
				break;
			case IDELAY_1_VOL:
				pluginGui->vol1=value;
				inv_knob_set_value(INV_KNOB (pluginGui->knobVol1), pluginGui->vol1);
				break;
			case IDELAY_2_DELAY:
				pluginGui->delay2=value;
				inv_knob_set_value(INV_KNOB (pluginGui->knobDelay2), pluginGui->delay2);
				break;
			case IDELAY_2_FB:
				pluginGui->fb2=value;
				inv_knob_set_value(INV_KNOB (pluginGui->knobFB2), pluginGui->fb2);
				break;
			case IDELAY_2_PAN:
				pluginGui->pan2=value;
				inv_knob_set_value(INV_KNOB (pluginGui->knobPan2), pluginGui->pan2);
				break;
			case IDELAY_2_VOL:
				pluginGui->vol2=value;
				inv_knob_set_value(INV_KNOB (pluginGui->knobVol2), pluginGui->vol2);
				break;
			case IDELAY_METER_INL:
				inv_meter_set_LdB(INV_METER (pluginGui->meterIn),value);
				break;
			case IDELAY_METER_OUTL:
				inv_meter_set_LdB(INV_METER (pluginGui->meterOut),value);
				break;
			case IDELAY_METER_OUTR:
				inv_meter_set_RdB(INV_METER (pluginGui->meterOut),value);
				break;
		}
	}
}


static void 
init()
{
	IDelayGuiDescriptor =
	 (LV2UI_Descriptor *)malloc(sizeof(LV2UI_Descriptor));

	IDelayGuiDescriptor->URI 		= IDELAY_GUI_URI;
	IDelayGuiDescriptor->instantiate 	= instantiateIDelayGui;
	IDelayGuiDescriptor->cleanup		= cleanupIDelayGui;
	IDelayGuiDescriptor->port_event	= port_eventIDelayGui;
	IDelayGuiDescriptor->extension_data 	= NULL;

}


const LV2UI_Descriptor* lv2ui_descriptor(uint32_t index)
{
	if (!IDelayGuiDescriptor) init();

	switch (index) {
		case 0:
			return IDelayGuiDescriptor;
	default:
		return NULL;
	}
}


/*****************************************************************************/

static void 
on_inv_delay_bypass_toggle_button_release(GtkWidget *widget, GdkEvent *event, gpointer data)
{

	IDelayGui *pluginGui = (IDelayGui *) data;

	pluginGui->bypass=inv_switch_toggle_get_value(INV_SWITCH_TOGGLE (widget));
	(*pluginGui->write_function)(pluginGui->controller, IDELAY_BYPASS, 4, 0, &pluginGui->bypass);
	return;
}

static void 
on_inv_delay_mode_toggle_button_release(GtkWidget *widget, GdkEvent *event, gpointer data)
{

	IDelayGui *pluginGui = (IDelayGui *) data;

	pluginGui->mode=inv_switch_toggle_get_value(INV_SWITCH_TOGGLE (widget));
	(*pluginGui->write_function)(pluginGui->controller, IDELAY_MODE, 4, 0, &pluginGui->mode);
	return;
}

static void 
on_inv_delay_mungemode_toggle_button_release(GtkWidget *widget, GdkEvent *event, gpointer data)
{

	IDelayGui *pluginGui = (IDelayGui *) data;

	pluginGui->mungemode=inv_switch_toggle_get_value(INV_SWITCH_TOGGLE (widget));
	(*pluginGui->write_function)(pluginGui->controller, IDELAY_MUNGEMODE, 4, 0, &pluginGui->mungemode);
	return;
}

static void 
on_inv_delay_munge_knob_motion(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	IDelayGui *pluginGui = (IDelayGui *) data;

	pluginGui->munge=inv_knob_get_value(INV_KNOB (widget));
	(*pluginGui->write_function)(pluginGui->controller, IDELAY_MUNGE, 4, 0, &pluginGui->munge);
	return;
}

static void 
on_inv_delay_cycle_knob_motion(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	IDelayGui *pluginGui = (IDelayGui *) data;

	pluginGui->cycle=inv_knob_get_value(INV_KNOB (widget));
	(*pluginGui->write_function)(pluginGui->controller, IDELAY_LFO_CYCLE, 4, 0, &pluginGui->cycle);
	return;
}

static void 
on_inv_delay_width_knob_motion(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	IDelayGui *pluginGui = (IDelayGui *) data;

	pluginGui->width=inv_knob_get_value(INV_KNOB (widget));
	(*pluginGui->write_function)(pluginGui->controller, IDELAY_LFO_WIDTH, 4, 0, &pluginGui->width);
	return;
}

static void 
on_inv_delay_delay1_knob_motion(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	IDelayGui *pluginGui = (IDelayGui *) data;

	pluginGui->delay1=inv_knob_get_value(INV_KNOB (widget));
	(*pluginGui->write_function)(pluginGui->controller, IDELAY_1_DELAY, 4, 0, &pluginGui->delay1);
	return;
}

static void 
on_inv_delay_fb1_knob_motion(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	IDelayGui *pluginGui = (IDelayGui *) data;

	pluginGui->fb1=inv_knob_get_value(INV_KNOB (widget));
	(*pluginGui->write_function)(pluginGui->controller, IDELAY_1_FB, 4, 0, &pluginGui->fb1);
	return;
}

static void 
on_inv_delay_pan1_knob_motion(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	IDelayGui *pluginGui = (IDelayGui *) data;

	pluginGui->pan1=inv_knob_get_value(INV_KNOB (widget));
	(*pluginGui->write_function)(pluginGui->controller, IDELAY_1_PAN, 4, 0, &pluginGui->pan1);
	return;
}

static void 
on_inv_delay_vol1_knob_motion(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	IDelayGui *pluginGui = (IDelayGui *) data;

	pluginGui->vol1=inv_knob_get_value(INV_KNOB (widget));
	(*pluginGui->write_function)(pluginGui->controller, IDELAY_1_VOL, 4, 0, &pluginGui->vol1);
	return;
}

static void 
on_inv_delay_delay2_knob_motion(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	IDelayGui *pluginGui = (IDelayGui *) data;

	pluginGui->delay2=inv_knob_get_value(INV_KNOB (widget));
	(*pluginGui->write_function)(pluginGui->controller, IDELAY_2_DELAY, 4, 0, &pluginGui->delay2);
	return;
}

static void 
on_inv_delay_fb2_knob_motion(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	IDelayGui *pluginGui = (IDelayGui *) data;

	pluginGui->fb2=inv_knob_get_value(INV_KNOB (widget));
	(*pluginGui->write_function)(pluginGui->controller, IDELAY_2_FB, 4, 0, &pluginGui->fb2);
	return;
}

static void 
on_inv_delay_pan2_knob_motion(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	IDelayGui *pluginGui = (IDelayGui *) data;

	pluginGui->pan2=inv_knob_get_value(INV_KNOB (widget));
	(*pluginGui->write_function)(pluginGui->controller, IDELAY_2_PAN, 4, 0, &pluginGui->pan2);
	return;
}

static void 
on_inv_delay_vol2_knob_motion(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	IDelayGui *pluginGui = (IDelayGui *) data;

	pluginGui->vol2=inv_knob_get_value(INV_KNOB (widget));
	(*pluginGui->write_function)(pluginGui->controller, IDELAY_2_VOL, 4, 0, &pluginGui->vol2);
	return;
}

