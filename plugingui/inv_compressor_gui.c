/* 

    This LV2 extension provides compressor gui's

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
#include "../plugin/inv_compressor.h"
#include "inv_compressor_gui.h"


static LV2UI_Descriptor *IFilterGuiDescriptor = NULL;

typedef struct {
	GtkWidget	*windowContainer;
	GtkWidget	*heading;
	GtkWidget	*meterIn;
	GtkWidget	*meterOut;
	GtkWidget	*display;
	GtkWidget	*knobRms;
	GtkWidget	*knobAttack;
	GtkWidget	*knobRelease;
	GtkWidget	*knobThreshold;
	GtkWidget	*knobRatio;
	GtkWidget	*knobGain;
	GtkWidget	*lampNoClip;

	gint		InChannels;
	gint		OutChannels;
	float		rms;
	float		attack;
	float		release;
	float		threshold;
	float		ratio;
	float		gain;

	LV2UI_Write_Function 	write_function;
	LV2UI_Controller 	controller;

} IFilterGui;



static LV2UI_Handle instantiateIFilterGui(const struct _LV2UI_Descriptor* descriptor, const char* plugin_uri, const char* bundle_path, LV2UI_Write_Function write_function, LV2UI_Controller controller, LV2UI_Widget* widget, const LV2_Feature* const* features)
{
	IFilterGui *pluginGui = (IFilterGui *)malloc(sizeof(IFilterGui));
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
	file = g_strdup_printf("%s/gtk/inv_compressor_gui.xml",bundle_path);
	gtk_builder_add_from_file (builder, file, &err);
	free(file);

	window = GTK_WIDGET (gtk_builder_get_object (builder, "comp_window"));

	/* get pointers to some useful widgets from the design */
	pluginGui->windowContainer = GTK_WIDGET (gtk_builder_get_object (builder, "comp_container"));
	pluginGui->heading = GTK_WIDGET (gtk_builder_get_object (builder, "label_heading"));

	/* add custom widgets */
	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_meter_in"));
	pluginGui->meterIn = inv_meter_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->meterIn);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_meter_out"));
	pluginGui->meterOut = inv_meter_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->meterOut);


	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_rms_knob"));
	pluginGui->knobRms = inv_knob_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->knobRms);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_attack_knob"));
	pluginGui->knobAttack = inv_knob_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->knobAttack);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_release_knob"));
	pluginGui->knobRelease = inv_knob_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->knobRelease);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_threshold_knob"));
	pluginGui->knobThreshold = inv_knob_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->knobThreshold);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_ratio_knob"));
	pluginGui->knobRatio = inv_knob_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->knobRatio);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_makeupgain_knob"));
	pluginGui->knobGain = inv_knob_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->knobGain);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_noclip_lamp"));
	pluginGui->lampNoClip = inv_lamp_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->lampNoClip);

	/* customise for the plugin */
	if(!strcmp(plugin_uri,ICOMP_MONO_URI)) 
	{
		pluginGui->InChannels=1;
		pluginGui->OutChannels=1;
		gtk_label_set_markup (GTK_LABEL (pluginGui->heading), "<b>Invada Compressor (mono)</b>");
	}
	if(!strcmp(plugin_uri,ICOMP_STEREO_URI)) 
	{
		pluginGui->InChannels=2;
		pluginGui->OutChannels=2;
		gtk_label_set_markup (GTK_LABEL (pluginGui->heading), "<b>Invada Compressor (stereo)</b>");
	}

	pluginGui->rms=0.5;
	pluginGui->attack=0.00001;
	pluginGui->release=0.001;
	pluginGui->threshold=0.0;
	pluginGui->ratio=1.0;
	pluginGui->gain=0.0;

	inv_meter_set_channels(INV_METER (pluginGui->meterIn), pluginGui->InChannels);
	inv_meter_set_LdB(INV_METER (pluginGui->meterIn),-90);
	inv_meter_set_RdB(INV_METER (pluginGui->meterIn),-90);

	inv_meter_set_channels(INV_METER (pluginGui->meterOut), pluginGui->OutChannels);
	inv_meter_set_LdB(INV_METER (pluginGui->meterOut),-90);
	inv_meter_set_RdB(INV_METER (pluginGui->meterOut),-90);

	inv_knob_set_size(INV_KNOB (pluginGui->knobRms), INV_KNOB_SIZE_MEDIUM);
	inv_knob_set_curve(INV_KNOB (pluginGui->knobRms), INV_KNOB_CURVE_LINEAR);
	inv_knob_set_markings(INV_KNOB (pluginGui->knobRms), INV_KNOB_MARKINGS_CUST10); 
	inv_knob_set_units(INV_KNOB (pluginGui->knobRms), "");
	inv_knob_set_custom(INV_KNOB (pluginGui->knobRms), 0, "Peak");
	inv_knob_set_custom(INV_KNOB (pluginGui->knobRms), 1, "Fast RMS");
	inv_knob_set_custom(INV_KNOB (pluginGui->knobRms), 2, "RMS");
	inv_knob_set_min(INV_KNOB (pluginGui->knobRms), 0.0);
	inv_knob_set_max(INV_KNOB (pluginGui->knobRms), 1.0);
	inv_knob_set_value(INV_KNOB (pluginGui->knobRms), pluginGui->rms);
	g_signal_connect_after(G_OBJECT(pluginGui->knobRms),"motion-notify-event",G_CALLBACK(on_inv_comp_rms_knob_motion),pluginGui);

	inv_knob_set_size(INV_KNOB (pluginGui->knobAttack), INV_KNOB_SIZE_MEDIUM);
	inv_knob_set_curve(INV_KNOB (pluginGui->knobAttack), INV_KNOB_CURVE_LOG);
	inv_knob_set_markings(INV_KNOB (pluginGui->knobAttack), INV_KNOB_MARKINGS_5);
	inv_knob_set_human(INV_KNOB (pluginGui->knobAttack)); 
	inv_knob_set_units(INV_KNOB (pluginGui->knobAttack), "s");
	inv_knob_set_min(INV_KNOB (pluginGui->knobAttack), 0.00001);
	inv_knob_set_max(INV_KNOB (pluginGui->knobAttack), 0.750);
	inv_knob_set_value(INV_KNOB (pluginGui->knobAttack), pluginGui->attack);
	g_signal_connect_after(G_OBJECT(pluginGui->knobAttack),"motion-notify-event",G_CALLBACK(on_inv_comp_attack_knob_motion),pluginGui);

	inv_knob_set_size(INV_KNOB (pluginGui->knobRelease), INV_KNOB_SIZE_MEDIUM);
	inv_knob_set_curve(INV_KNOB (pluginGui->knobRelease), INV_KNOB_CURVE_LOG);
	inv_knob_set_markings(INV_KNOB (pluginGui->knobRelease), INV_KNOB_MARKINGS_5);
	inv_knob_set_human(INV_KNOB (pluginGui->knobRelease)); 
	inv_knob_set_units(INV_KNOB (pluginGui->knobRelease), "s");
	inv_knob_set_min(INV_KNOB (pluginGui->knobRelease), 0.001);
	inv_knob_set_max(INV_KNOB (pluginGui->knobRelease), 5.0);
	inv_knob_set_value(INV_KNOB (pluginGui->knobRelease), pluginGui->release);
	g_signal_connect_after(G_OBJECT(pluginGui->knobRelease),"motion-notify-event",G_CALLBACK(on_inv_comp_release_knob_motion),pluginGui);

	inv_knob_set_size(INV_KNOB (pluginGui->knobThreshold), INV_KNOB_SIZE_MEDIUM);
	inv_knob_set_curve(INV_KNOB (pluginGui->knobThreshold), INV_KNOB_CURVE_LINEAR);
	inv_knob_set_markings(INV_KNOB (pluginGui->knobThreshold), INV_KNOB_MARKINGS_5);
	inv_knob_set_highlight(INV_KNOB (pluginGui->knobThreshold), INV_KNOB_HIGHLIGHT_L);
	inv_knob_set_units(INV_KNOB (pluginGui->knobThreshold), "dB");
	inv_knob_set_min(INV_KNOB (pluginGui->knobThreshold), -36.0);
	inv_knob_set_max(INV_KNOB (pluginGui->knobThreshold), 0.0);
	inv_knob_set_value(INV_KNOB (pluginGui->knobThreshold), pluginGui->threshold);
	g_signal_connect_after(G_OBJECT(pluginGui->knobThreshold),"motion-notify-event",G_CALLBACK(on_inv_comp_threshold_knob_motion),pluginGui);

	inv_knob_set_size(INV_KNOB (pluginGui->knobRatio), INV_KNOB_SIZE_MEDIUM);
	inv_knob_set_curve(INV_KNOB (pluginGui->knobRatio), INV_KNOB_CURVE_LINEAR);
	inv_knob_set_markings(INV_KNOB (pluginGui->knobRatio), INV_KNOB_MARKINGS_5);
	inv_knob_set_highlight(INV_KNOB (pluginGui->knobRatio), INV_KNOB_HIGHLIGHT_L);
	inv_knob_set_units(INV_KNOB (pluginGui->knobRatio), ":1");
	inv_knob_set_min(INV_KNOB (pluginGui->knobRatio), 1.0);
	inv_knob_set_max(INV_KNOB (pluginGui->knobRatio), 20.0);
	inv_knob_set_value(INV_KNOB (pluginGui->knobRatio), pluginGui->ratio);
	g_signal_connect_after(G_OBJECT(pluginGui->knobRatio),"motion-notify-event",G_CALLBACK(on_inv_comp_ratio_knob_motion),pluginGui);

	inv_knob_set_size(INV_KNOB (pluginGui->knobGain), INV_KNOB_SIZE_MEDIUM);
	inv_knob_set_curve(INV_KNOB (pluginGui->knobGain), INV_KNOB_CURVE_LINEAR);
	inv_knob_set_markings(INV_KNOB (pluginGui->knobGain), INV_KNOB_MARKINGS_5);
	inv_knob_set_highlight(INV_KNOB (pluginGui->knobGain), INV_KNOB_HIGHLIGHT_L);
	inv_knob_set_units(INV_KNOB (pluginGui->knobGain), "dB");
	inv_knob_set_min(INV_KNOB (pluginGui->knobGain), -6.0);
	inv_knob_set_max(INV_KNOB (pluginGui->knobGain), 36.0);
	inv_knob_set_value(INV_KNOB (pluginGui->knobGain), pluginGui->gain);
	g_signal_connect_after(G_OBJECT(pluginGui->knobGain),"motion-notify-event",G_CALLBACK(on_inv_comp_gain_knob_motion),pluginGui);

	inv_lamp_set_value(INV_LAMP (pluginGui->lampNoClip),0.0);
	inv_lamp_set_scale(INV_LAMP (pluginGui->lampNoClip),2.0);

	/* strip the parent window from the design so the host can attach its own */
	gtk_widget_ref(pluginGui->windowContainer);
	gtk_container_remove(GTK_CONTAINER(window), pluginGui->windowContainer);

	*widget = (LV2UI_Widget) pluginGui->windowContainer;

	g_object_unref (G_OBJECT (builder));
             
	/* return the instance */
	return pluginGui;
}


static void cleanupIFilterGui(LV2UI_Handle ui)
{
	return;
}


static void port_eventIFilterGui(LV2UI_Handle ui, uint32_t port, uint32_t buffer_size, uint32_t format, const void*  buffer)
{
	IFilterGui *pluginGui = (IFilterGui *)ui;

	float value;

	if(format==0) 
	{
		value=* (float *) buffer;
		switch(port)
		{
			case ICOMP_RMS:
				pluginGui->rms=value;
				inv_knob_set_value(INV_KNOB (pluginGui->knobRms), pluginGui->rms);
				break;
			case ICOMP_ATTACK:
				pluginGui->attack=value;
				inv_knob_set_value(INV_KNOB (pluginGui->knobAttack), pluginGui->attack);
				break;
			case ICOMP_RELEASE:
				pluginGui->release=value;
				inv_knob_set_value(INV_KNOB (pluginGui->knobRelease), pluginGui->release);
				break;
			case ICOMP_THRESH:
				pluginGui->threshold=value;
				inv_knob_set_value(INV_KNOB (pluginGui->knobThreshold), pluginGui->threshold);
				break;
			case ICOMP_RATIO:
				pluginGui->ratio=value;
				inv_knob_set_value(INV_KNOB (pluginGui->knobRatio), pluginGui->ratio);
				break;
			case ICOMP_GAIN:
				pluginGui->gain=value;
				inv_knob_set_value(INV_KNOB (pluginGui->knobGain), pluginGui->gain);
				break;
/*
			case IFILTER_METER_INL:
				inv_meter_set_LdB(INV_METER (pluginGui->meterIn),value);
				break;
			case IFILTER_METER_INR:
				if(pluginGui->InChannels==2) inv_meter_set_RdB(INV_METER (pluginGui->meterIn),value);
				break;
			case IFILTER_METER_OUTL:
				inv_meter_set_LdB(INV_METER (pluginGui->meterOut),value);
				break;
			case IFILTER_METER_OUTR:
				if(pluginGui->OutChannels==2) inv_meter_set_RdB(INV_METER (pluginGui->meterOut),value);
				break;
			case IFILTER_METER_DRIVE:
				inv_lamp_set_value(INV_LAMP (pluginGui->lampNoClip),value);
				break;
*/
		}
	}
}


static void init()
{
	IFilterGuiDescriptor =
	 (LV2UI_Descriptor *)malloc(sizeof(LV2UI_Descriptor));

	IFilterGuiDescriptor->URI 		= ICOMP_GUI_URI;
	IFilterGuiDescriptor->instantiate 	= instantiateIFilterGui;
	IFilterGuiDescriptor->cleanup		= cleanupIFilterGui;
	IFilterGuiDescriptor->port_event	= port_eventIFilterGui;
	IFilterGuiDescriptor->extension_data 	= NULL;

}


const LV2UI_Descriptor* lv2ui_descriptor(uint32_t index)
{
	if (!IFilterGuiDescriptor) init();

	switch (index) {
		case 0:
			return IFilterGuiDescriptor;
	default:
		return NULL;
	}
}


/*****************************************************************************/


static void on_inv_comp_rms_knob_motion(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	IFilterGui *pluginGui = (IFilterGui *) data;

	pluginGui->rms=inv_knob_get_value(INV_KNOB (widget));
	(*pluginGui->write_function)(pluginGui->controller, ICOMP_RMS, 4, 0, &pluginGui->rms);
	return;
}

static void on_inv_comp_attack_knob_motion(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	IFilterGui *pluginGui = (IFilterGui *) data;

	pluginGui->attack=inv_knob_get_value(INV_KNOB (widget));
	(*pluginGui->write_function)(pluginGui->controller, ICOMP_ATTACK, 4, 0, &pluginGui->attack);
	return;
}

static void on_inv_comp_release_knob_motion(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	IFilterGui *pluginGui = (IFilterGui *) data;

	pluginGui->release=inv_knob_get_value(INV_KNOB (widget));
	(*pluginGui->write_function)(pluginGui->controller, ICOMP_RELEASE, 4, 0, &pluginGui->release);
	return;
}

static void on_inv_comp_threshold_knob_motion(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	IFilterGui *pluginGui = (IFilterGui *) data;

	pluginGui->threshold=inv_knob_get_value(INV_KNOB (widget));
	(*pluginGui->write_function)(pluginGui->controller, ICOMP_THRESH, 4, 0, &pluginGui->threshold);
	return;
}

static void on_inv_comp_ratio_knob_motion(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	IFilterGui *pluginGui = (IFilterGui *) data;

	pluginGui->ratio=inv_knob_get_value(INV_KNOB (widget));
	(*pluginGui->write_function)(pluginGui->controller, ICOMP_RATIO, 4, 0, &pluginGui->ratio);
	return;
}

static void on_inv_comp_gain_knob_motion(GtkWidget *widget, GdkEvent *event, gpointer data)
{

	IFilterGui *pluginGui = (IFilterGui *) data;

	pluginGui->gain=inv_knob_get_value(INV_KNOB (widget));
	(*pluginGui->write_function)(pluginGui->controller, ICOMP_GAIN, 4, 0, &pluginGui->gain);
	return;
}


