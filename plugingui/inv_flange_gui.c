/* 

    This LV2 extension provides flange gui's

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
#include "../plugin/inv_flange.h"
#include "inv_flange_gui.h"


static LV2UI_Descriptor *IFlangeGuiDescriptor = NULL;

typedef struct {
	GtkWidget	*windowContainer;
	GtkWidget	*heading;
	GtkWidget	*toggleBypass;
	GtkWidget	*meterIn;
	GtkWidget	*meterOut;
	GtkWidget	*knobCycle;
	GtkWidget	*knobPhase;
	GtkWidget	*lampL;
	GtkWidget	*lampR;
	GtkWidget	*knobWidth;
	GtkWidget	*knobDepth;
	GtkWidget	*toggleNoClip;
	GtkWidget	*lampNoClip;

	gint		InChannels;
	gint		OutChannels;
	float 		bypass;
	float		cycle;
	float		phase;
	float		width;
	float		depth;
	float		noclip;

	LV2UI_Write_Function 	write_function;
	LV2UI_Controller 	controller;

} IFlangeGui;



static LV2UI_Handle 
instantiateIFlangeGui(const struct _LV2UI_Descriptor* descriptor, const char* plugin_uri, const char* bundle_path, LV2UI_Write_Function write_function, LV2UI_Controller controller, LV2UI_Widget* widget, const LV2_Feature* const* features)
{
	IFlangeGui *pluginGui = (IFlangeGui *)malloc(sizeof(IFlangeGui));
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
	file = g_strdup_printf("%s/gtk/inv_flange_gui.xml",bundle_path);
	gtk_builder_add_from_file (builder, file, &err);
	free(file);

	window = GTK_WIDGET (gtk_builder_get_object (builder, "flange_window"));

	/* get pointers to some useful widgets from the design */
	pluginGui->windowContainer = GTK_WIDGET (gtk_builder_get_object (builder, "flange_container"));
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

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_cycle_knob"));
	pluginGui->knobCycle = inv_knob_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->knobCycle);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_phase_knob"));
	pluginGui->knobPhase = inv_knob_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->knobPhase);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_l_lamp"));
	pluginGui->lampL = inv_lamp_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->lampL);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_r_lamp"));
	pluginGui->lampR = inv_lamp_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->lampR);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_width_knob"));
	pluginGui->knobWidth = inv_knob_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->knobWidth);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_depth_knob"));
	pluginGui->knobDepth = inv_knob_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->knobDepth);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_noclip_toggle"));
	pluginGui->toggleNoClip = inv_switch_toggle_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->toggleNoClip);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_noclip_lamp"));
	pluginGui->lampNoClip = inv_lamp_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->lampNoClip);


	/* customise for the plugin */
	if(!strcmp(plugin_uri,IFLANGE_MONO_URI)) 
	{
		gtk_label_set_markup (GTK_LABEL (pluginGui->heading), "<b>Stereo Flange (mono in)</b>");
		pluginGui->InChannels	= 1;
	}
	if(!strcmp(plugin_uri,IFLANGE_STEREO_URI)) 
	{
		gtk_label_set_markup (GTK_LABEL (pluginGui->heading), "<b>Stereo Flange (stereo in)</b>");
		pluginGui->InChannels	= 2;
	}
	if(!strcmp(plugin_uri,IFLANGE_SUM_URI)) 
	{
		gtk_label_set_markup (GTK_LABEL (pluginGui->heading), "<b>Stereo Flange (sum L+R in)</b>");
		pluginGui->InChannels	= 1;
	}


	pluginGui->OutChannels	= 2;
	pluginGui->bypass	= 0.0;
	pluginGui->cycle	= 25.0;
	pluginGui->phase	= 45.0;
	pluginGui->width	= 10.5;
	pluginGui->depth	= 75.0;
	pluginGui->noclip	= 1.0;

	inv_switch_toggle_set_bypass( INV_SWITCH_TOGGLE (pluginGui->toggleBypass), INV_PLUGIN_ACTIVE);
	inv_switch_toggle_set_value( INV_SWITCH_TOGGLE (pluginGui->toggleBypass), INV_SWITCH_TOGGLE_OFF, 0.0);
	inv_switch_toggle_set_colour(INV_SWITCH_TOGGLE (pluginGui->toggleBypass), INV_SWITCH_TOGGLE_OFF, 0.0, 1.0, 0.0);
	inv_switch_toggle_set_text(  INV_SWITCH_TOGGLE (pluginGui->toggleBypass), INV_SWITCH_TOGGLE_OFF, "Active");
	inv_switch_toggle_set_value( INV_SWITCH_TOGGLE (pluginGui->toggleBypass), INV_SWITCH_TOGGLE_ON,  1.0);
	inv_switch_toggle_set_colour(INV_SWITCH_TOGGLE (pluginGui->toggleBypass), INV_SWITCH_TOGGLE_ON,  1.0, 0.0, 0.0);
	inv_switch_toggle_set_text(  INV_SWITCH_TOGGLE (pluginGui->toggleBypass), INV_SWITCH_TOGGLE_ON,  "Bypassed");
	inv_switch_toggle_set_state( INV_SWITCH_TOGGLE (pluginGui->toggleBypass), INV_SWITCH_TOGGLE_OFF);
	g_signal_connect_after(G_OBJECT(pluginGui->toggleBypass),"button-release-event",G_CALLBACK(on_inv_flange_bypass_toggle_button_release),pluginGui);

	inv_meter_set_bypass(INV_METER (pluginGui->meterIn),INV_PLUGIN_ACTIVE);
	inv_meter_set_mode(INV_METER (pluginGui->meterIn), INV_METER_DRAW_MODE_TOZERO);
	inv_meter_set_channels(INV_METER (pluginGui->meterIn), pluginGui->InChannels);
	inv_meter_set_LdB(INV_METER (pluginGui->meterIn),-90);
	inv_meter_set_RdB(INV_METER (pluginGui->meterIn),-90);

	inv_meter_set_bypass(INV_METER (pluginGui->meterOut),INV_PLUGIN_ACTIVE);
	inv_meter_set_mode(INV_METER (pluginGui->meterOut), INV_METER_DRAW_MODE_TOZERO);
	inv_meter_set_channels(INV_METER (pluginGui->meterOut), pluginGui->OutChannels);
	inv_meter_set_LdB(INV_METER (pluginGui->meterOut),-90);
	inv_meter_set_RdB(INV_METER (pluginGui->meterOut),-90);

	inv_knob_set_bypass(INV_KNOB (pluginGui->knobCycle),   INV_PLUGIN_ACTIVE);
	inv_knob_set_size(INV_KNOB (pluginGui->knobCycle),     INV_KNOB_SIZE_MEDIUM);
	inv_knob_set_curve(INV_KNOB (pluginGui->knobCycle),    INV_KNOB_CURVE_LOG);
	inv_knob_set_markings(INV_KNOB (pluginGui->knobCycle), INV_KNOB_MARKINGS_4); 
	inv_knob_set_units(INV_KNOB (pluginGui->knobCycle),    "s");
	inv_knob_set_min(INV_KNOB (pluginGui->knobCycle),      1.0);
	inv_knob_set_max(INV_KNOB (pluginGui->knobCycle),      1000.0);
	inv_knob_set_value(INV_KNOB (pluginGui->knobCycle),    pluginGui->cycle);
	g_signal_connect_after(G_OBJECT(pluginGui->knobCycle),"motion-notify-event",G_CALLBACK(on_inv_flange_cycle_knob_motion),pluginGui);

	inv_knob_set_bypass(INV_KNOB (pluginGui->knobPhase), INV_PLUGIN_ACTIVE);
	inv_knob_set_size(INV_KNOB (pluginGui->knobPhase), INV_KNOB_SIZE_MEDIUM);
	inv_knob_set_curve(INV_KNOB (pluginGui->knobPhase), INV_KNOB_CURVE_LINEAR);
	inv_knob_set_markings(INV_KNOB (pluginGui->knobPhase), INV_KNOB_MARKINGS_5); 
	inv_knob_set_units(INV_KNOB (pluginGui->knobPhase), "ᵒ");
	inv_knob_set_min(INV_KNOB (pluginGui->knobPhase), -180);
	inv_knob_set_max(INV_KNOB (pluginGui->knobPhase), 180);
	inv_knob_set_value(INV_KNOB (pluginGui->knobPhase), pluginGui->phase);
	g_signal_connect_after(G_OBJECT(pluginGui->knobPhase),"motion-notify-event",G_CALLBACK(on_inv_flange_phase_knob_motion),pluginGui);

	inv_lamp_set_value(INV_LAMP (pluginGui->lampL),0.0);
	inv_lamp_set_scale(INV_LAMP (pluginGui->lampL),1.0);

	inv_lamp_set_value(INV_LAMP (pluginGui->lampR),0.0);
	inv_lamp_set_scale(INV_LAMP (pluginGui->lampR),1.0);

	inv_knob_set_bypass(INV_KNOB (pluginGui->knobWidth), INV_PLUGIN_ACTIVE);
	inv_knob_set_size(INV_KNOB (pluginGui->knobWidth), INV_KNOB_SIZE_MEDIUM);
	inv_knob_set_curve(INV_KNOB (pluginGui->knobWidth), INV_KNOB_CURVE_LINEAR);
	inv_knob_set_markings(INV_KNOB (pluginGui->knobWidth), INV_KNOB_MARKINGS_3); 
	inv_knob_set_units(INV_KNOB (pluginGui->knobWidth), "ms");
	inv_knob_set_min(INV_KNOB (pluginGui->knobWidth), 1.0);
	inv_knob_set_max(INV_KNOB (pluginGui->knobWidth), 15.0);
	inv_knob_set_value(INV_KNOB (pluginGui->knobWidth), pluginGui->width);
	g_signal_connect_after(G_OBJECT(pluginGui->knobWidth),"motion-notify-event",G_CALLBACK(on_inv_flange_width_knob_motion),pluginGui);

	inv_knob_set_bypass(INV_KNOB (pluginGui->knobDepth), INV_PLUGIN_ACTIVE);
	inv_knob_set_size(INV_KNOB (pluginGui->knobDepth), INV_KNOB_SIZE_MEDIUM);
	inv_knob_set_curve(INV_KNOB (pluginGui->knobDepth), INV_KNOB_CURVE_LINEAR);
	inv_knob_set_markings(INV_KNOB (pluginGui->knobDepth), INV_KNOB_MARKINGS_5);
	inv_knob_set_units(INV_KNOB (pluginGui->knobDepth), "%");
	inv_knob_set_min(INV_KNOB (pluginGui->knobDepth), 0.0);
	inv_knob_set_max(INV_KNOB (pluginGui->knobDepth), 100.0);
	inv_knob_set_value(INV_KNOB (pluginGui->knobDepth), pluginGui->depth);
	g_signal_connect_after(G_OBJECT(pluginGui->knobDepth),"motion-notify-event",G_CALLBACK(on_inv_flange_depth_knob_motion),pluginGui);

	inv_switch_toggle_set_bypass( INV_SWITCH_TOGGLE (pluginGui->toggleNoClip), INV_PLUGIN_ACTIVE);
	inv_switch_toggle_set_value( INV_SWITCH_TOGGLE (pluginGui->toggleNoClip), INV_SWITCH_TOGGLE_OFF, 0.0);
	inv_switch_toggle_set_colour(INV_SWITCH_TOGGLE (pluginGui->toggleNoClip), INV_SWITCH_TOGGLE_OFF, 0.0, 1.0, 0.0);
	inv_switch_toggle_set_text(  INV_SWITCH_TOGGLE (pluginGui->toggleNoClip), INV_SWITCH_TOGGLE_OFF, "Off");
	inv_switch_toggle_set_value( INV_SWITCH_TOGGLE (pluginGui->toggleNoClip), INV_SWITCH_TOGGLE_ON,  1.0);
	inv_switch_toggle_set_colour(INV_SWITCH_TOGGLE (pluginGui->toggleNoClip), INV_SWITCH_TOGGLE_ON,  0.0, 1.0, 0.0);
	inv_switch_toggle_set_text(  INV_SWITCH_TOGGLE (pluginGui->toggleNoClip), INV_SWITCH_TOGGLE_ON,  "Active");
	inv_switch_toggle_set_state( INV_SWITCH_TOGGLE (pluginGui->toggleNoClip), INV_SWITCH_TOGGLE_ON);
	g_signal_connect_after(G_OBJECT(pluginGui->toggleNoClip),"button-release-event",G_CALLBACK(on_inv_flange_noclip_toggle_button_release),pluginGui);

	inv_lamp_set_value(INV_LAMP (pluginGui->lampNoClip),0.0);
	inv_lamp_set_scale(INV_LAMP (pluginGui->lampNoClip),3.0);

	/* strip the parent window from the design so the host can attach its own */
	gtk_widget_ref(pluginGui->windowContainer);
	gtk_container_remove(GTK_CONTAINER(window), pluginGui->windowContainer);

	*widget = (LV2UI_Widget) pluginGui->windowContainer;

	g_object_unref (G_OBJECT (builder));
             
	/* return the instance */
	return pluginGui;
}


static void 
cleanupIFlangeGui(LV2UI_Handle ui)
{
	return;
}


static void 
port_eventIFlangeGui(LV2UI_Handle ui, uint32_t port, uint32_t buffer_size, uint32_t format, const void*  buffer)
{
	IFlangeGui *pluginGui = (IFlangeGui *)ui;

	float value;

	if(format==0) 
	{
		value=* (float *) buffer;
		switch(port)
		{
			case IFLANGE_BYPASS:
				pluginGui->bypass=value;
				if(value <= 0.0) {
					inv_switch_toggle_set_state(INV_SWITCH_TOGGLE (pluginGui->toggleBypass), INV_SWITCH_TOGGLE_OFF);
					inv_meter_set_bypass(         INV_METER         (pluginGui->meterIn),      INV_PLUGIN_ACTIVE);
					inv_meter_set_bypass(         INV_METER         (pluginGui->meterOut),     INV_PLUGIN_ACTIVE);
					inv_knob_set_bypass(          INV_KNOB          (pluginGui->knobCycle),    INV_PLUGIN_ACTIVE);
					inv_knob_set_bypass(          INV_KNOB          (pluginGui->knobPhase),    INV_PLUGIN_ACTIVE);
					inv_knob_set_bypass(          INV_KNOB          (pluginGui->knobWidth),    INV_PLUGIN_ACTIVE);
					inv_knob_set_bypass(          INV_KNOB          (pluginGui->knobDepth),    INV_PLUGIN_ACTIVE);
					inv_switch_toggle_set_bypass( INV_SWITCH_TOGGLE (pluginGui->toggleNoClip), INV_PLUGIN_ACTIVE);
				} else {
					inv_switch_toggle_set_state(INV_SWITCH_TOGGLE (pluginGui->toggleBypass), INV_SWITCH_TOGGLE_ON);
					inv_meter_set_bypass(         INV_METER         (pluginGui->meterIn),      INV_PLUGIN_BYPASS);
					inv_meter_set_bypass(         INV_METER         (pluginGui->meterOut),     INV_PLUGIN_BYPASS);
					inv_knob_set_bypass(          INV_KNOB          (pluginGui->knobCycle),    INV_PLUGIN_BYPASS);
					inv_knob_set_bypass(          INV_KNOB          (pluginGui->knobPhase),    INV_PLUGIN_BYPASS);
					inv_knob_set_bypass(          INV_KNOB          (pluginGui->knobWidth),    INV_PLUGIN_BYPASS);
					inv_knob_set_bypass(          INV_KNOB          (pluginGui->knobDepth),    INV_PLUGIN_BYPASS);
					inv_switch_toggle_set_bypass( INV_SWITCH_TOGGLE (pluginGui->toggleNoClip), INV_PLUGIN_BYPASS);
				}
				break;
			case IFLANGE_CYCLE:
				pluginGui->cycle=value;
				inv_knob_set_value(INV_KNOB (pluginGui->knobCycle), pluginGui->cycle);
				break;
			case IFLANGE_PHASE:
				pluginGui->phase=value;
				inv_knob_set_value(INV_KNOB (pluginGui->knobPhase), pluginGui->phase);
				break;
			case IFLANGE_WIDTH:
				pluginGui->width=value;
				inv_knob_set_value(INV_KNOB (pluginGui->knobWidth), pluginGui->width);
				break;
			case IFLANGE_DEPTH:
				pluginGui->depth=value;
				inv_knob_set_value(INV_KNOB (pluginGui->knobDepth), pluginGui->depth);
				break;
			case IFLANGE_METER_INL:
				inv_meter_set_LdB(INV_METER (pluginGui->meterIn),value);
				break;
			case IFLANGE_METER_INR:
				if(pluginGui->InChannels==2) inv_meter_set_RdB(INV_METER (pluginGui->meterIn),value);
				break;
			case IFLANGE_METER_OUTL:
				inv_meter_set_LdB(INV_METER (pluginGui->meterOut),value);
				break;
			case IFLANGE_METER_OUTR:
				inv_meter_set_RdB(INV_METER (pluginGui->meterOut),value);
				break;
			case IFLANGE_LAMP_L:
				inv_lamp_set_value(INV_LAMP (pluginGui->lampL),value);
				break;
			case IFLANGE_LAMP_R:
				inv_lamp_set_value(INV_LAMP (pluginGui->lampR),value);
				break;
			case IFLANGE_LAMP_NOCLIP:
				inv_lamp_set_value(INV_LAMP (pluginGui->lampNoClip),value);
				break;
		}
	}
}


static void 
init()
{
	IFlangeGuiDescriptor =
	 (LV2UI_Descriptor *)malloc(sizeof(LV2UI_Descriptor));

	IFlangeGuiDescriptor->URI 		= IFLANGE_GUI_URI;
	IFlangeGuiDescriptor->instantiate 	= instantiateIFlangeGui;
	IFlangeGuiDescriptor->cleanup		= cleanupIFlangeGui;
	IFlangeGuiDescriptor->port_event	= port_eventIFlangeGui;
	IFlangeGuiDescriptor->extension_data 	= NULL;

}


const LV2UI_Descriptor* lv2ui_descriptor(uint32_t index)
{
	if (!IFlangeGuiDescriptor) init();

	switch (index) {
		case 0:
			return IFlangeGuiDescriptor;
	default:
		return NULL;
	}
}


/*****************************************************************************/

static void 
on_inv_flange_bypass_toggle_button_release(GtkWidget *widget, GdkEvent *event, gpointer data)
{

	IFlangeGui *pluginGui = (IFlangeGui *) data;

	pluginGui->bypass=inv_switch_toggle_get_value(INV_SWITCH_TOGGLE (widget));
	(*pluginGui->write_function)(pluginGui->controller, IFLANGE_BYPASS, 4, 0, &pluginGui->bypass);
	return;
}

static void 
on_inv_flange_cycle_knob_motion(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	IFlangeGui *pluginGui = (IFlangeGui *) data;

	pluginGui->cycle=inv_knob_get_value(INV_KNOB (widget));
	(*pluginGui->write_function)(pluginGui->controller, IFLANGE_CYCLE, 4, 0, &pluginGui->cycle);
	return;
}

static void 
on_inv_flange_phase_knob_motion(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	IFlangeGui *pluginGui = (IFlangeGui *) data;

	pluginGui->phase=inv_knob_get_value(INV_KNOB (widget));
	(*pluginGui->write_function)(pluginGui->controller, IFLANGE_PHASE, 4, 0, &pluginGui->phase);
	return;
}

static void 
on_inv_flange_width_knob_motion(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	IFlangeGui *pluginGui = (IFlangeGui *) data;

	pluginGui->width=inv_knob_get_value(INV_KNOB (widget));
	(*pluginGui->write_function)(pluginGui->controller, IFLANGE_WIDTH, 4, 0, &pluginGui->width);
	return;
}

static void 
on_inv_flange_depth_knob_motion(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	IFlangeGui *pluginGui = (IFlangeGui *) data;

	pluginGui->depth=inv_knob_get_value(INV_KNOB (widget));
	(*pluginGui->write_function)(pluginGui->controller, IFLANGE_DEPTH, 4, 0, &pluginGui->depth);
	return;
}

static void 
on_inv_flange_noclip_toggle_button_release(GtkWidget *widget, GdkEvent *event, gpointer data)
{

	IFlangeGui *pluginGui = (IFlangeGui *) data;

	pluginGui->noclip=inv_switch_toggle_get_value(INV_SWITCH_TOGGLE (widget));
	(*pluginGui->write_function)(pluginGui->controller, IFLANGE_NOCLIP, 4, 0, &pluginGui->noclip);
	return;
}
