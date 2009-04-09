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
#include "widgets/meter.h"
#include "widgets/display-FrequencyGain.h"
#include "../plugin/inv_filter.h"
#include "inv_filter_gui.h"


static LV2UI_Descriptor *IFilterGuiDescriptor = NULL;


typedef struct {
	GtkWidget	*windowContainer;
	GtkWidget	*heading;
	GtkWidget	*meterIn;
	GtkWidget	*meterOut;
	GtkWidget	*display;
	GtkWidget	*knobFreq;
	GtkWidget	*knobGain;

	gint		InChannels;
	gint		OutChannels;

} IFilterGui;


static LV2UI_Handle instantiateIFilterGui(const struct _LV2UI_Descriptor* descriptor, const char* plugin_uri, const char* bundle_path, LV2UI_Write_Function write_function, LV2UI_Controller controller, LV2UI_Widget* widget, const LV2_Feature* const* features)
{
	IFilterGui *pluginGui = (IFilterGui *)malloc(sizeof(IFilterGui));
	if(pluginGui==NULL)
		return NULL;

	GtkBuilder      *builder; 
	GtkWidget       *window;
	GtkWidget	*tempObject;

	GError *err = NULL;

	gtk_init (NULL,NULL);

	builder = gtk_builder_new ();
// TODO change this to use the supplied bundle path
	gtk_builder_add_from_file (builder, "/usr/local/lib/lv2/invada.lv2/gtk/inv_filter_gui.xml", &err);
	window = GTK_WIDGET (gtk_builder_get_object (builder, "filter_window"));

	/* get pointers to some useful widgets from the design */
	pluginGui->windowContainer = GTK_WIDGET (gtk_builder_get_object (builder, "filter_container"));
	pluginGui->heading = GTK_WIDGET (gtk_builder_get_object (builder, "label_heading"));

	/* add custom widgets */
	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "Fixed_meter_in_display"));
	pluginGui->meterIn = inv_meter_new ();
	gtk_fixed_put (GTK_FIXED (tempObject), pluginGui->meterIn,0,0);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "Fixed_meter_out_display"));
	pluginGui->meterOut = inv_meter_new ();
	gtk_fixed_put (GTK_FIXED (tempObject), pluginGui->meterOut,0,0);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "Filter_display"));
	pluginGui->display = inv_display_fg_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->display);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_frequency_knob"));
	pluginGui->knobFreq = inv_knob_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->knobFreq);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "alignment_gain_knob"));
	pluginGui->knobGain = inv_knob_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->knobGain);

	/* customise for the plugin */
	if(!strcmp(plugin_uri,IFILTER_MONO_LPF_URI)) 
	{
		pluginGui->InChannels=1;
		pluginGui->OutChannels=1;
		gtk_label_set_markup (GTK_LABEL (pluginGui->heading), "<b>Invada Low Pass Filter (mono)</b>");
		inv_display_fg_set_mode(INV_DISPLAY_FG (pluginGui->display), INV_DISPLAYFG_MODE_LPF);
		inv_knob_set_highlight(INV_KNOB (pluginGui->knobFreq), INV_KNOB_HIGHLIGHT_R);
	}
	if(!strcmp(plugin_uri,IFILTER_MONO_HPF_URI)) 
	{
		pluginGui->InChannels=1;
		pluginGui->OutChannels=1;
		gtk_label_set_markup (GTK_LABEL (pluginGui->heading), "<b>Invada High Pass Filter (mono)</b>");
		inv_display_fg_set_mode(INV_DISPLAY_FG (pluginGui->display), INV_DISPLAYFG_MODE_HPF);
		inv_knob_set_highlight(INV_KNOB (pluginGui->knobFreq), INV_KNOB_HIGHLIGHT_L);
	}
	if(!strcmp(plugin_uri,IFILTER_STEREO_LPF_URI)) 
	{
		pluginGui->InChannels=2;
		pluginGui->OutChannels=2;
		gtk_label_set_markup (GTK_LABEL (pluginGui->heading), "<b>Invada Low Pass Filter (stereo)</b>");
		inv_display_fg_set_mode(INV_DISPLAY_FG (pluginGui->display), INV_DISPLAYFG_MODE_LPF);
		inv_knob_set_highlight(INV_KNOB (pluginGui->knobFreq), INV_KNOB_HIGHLIGHT_R);
	}
	if(!strcmp(plugin_uri,IFILTER_STEREO_HPF_URI)) 
	{
		pluginGui->InChannels=2;
		pluginGui->OutChannels=2;
		gtk_label_set_markup (GTK_LABEL (pluginGui->heading), "<b>Invada High Pass Filter (stereo)</b>");
		inv_display_fg_set_mode(INV_DISPLAY_FG (pluginGui->display), INV_DISPLAYFG_MODE_HPF);
		inv_knob_set_highlight(INV_KNOB (pluginGui->knobFreq), INV_KNOB_HIGHLIGHT_L);
	}
	inv_meter_set_channels(INV_METER (pluginGui->meterIn), pluginGui->InChannels);
	inv_meter_set_channels(INV_METER (pluginGui->meterOut), pluginGui->OutChannels);

	inv_knob_set_size(INV_KNOB (pluginGui->knobFreq), INV_KNOB_SIZE_LARGE);
	inv_knob_set_curve(INV_KNOB (pluginGui->knobFreq), INV_KNOB_CURVE_LOG);
	inv_knob_set_markings(INV_KNOB (pluginGui->knobFreq), INV_KNOB_MARKINGS_4); 
	inv_knob_set_min(INV_KNOB (pluginGui->knobFreq), 20.0);
	inv_knob_set_max(INV_KNOB (pluginGui->knobFreq), 20000.0);

	inv_knob_set_size(INV_KNOB (pluginGui->knobGain), INV_KNOB_SIZE_LARGE);
	inv_knob_set_curve(INV_KNOB (pluginGui->knobGain), INV_KNOB_CURVE_LINEAR);
	inv_knob_set_markings(INV_KNOB (pluginGui->knobGain), INV_KNOB_MARKINGS_5);
	inv_knob_set_highlight(INV_KNOB (pluginGui->knobGain), INV_KNOB_HIGHLIGHT_L);
	inv_knob_set_min(INV_KNOB (pluginGui->knobGain), 0.0);
	inv_knob_set_max(INV_KNOB (pluginGui->knobGain), 12.0);

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
			case IFILTER_FREQ:
				inv_knob_set_value(INV_KNOB (pluginGui->knobFreq), value);
				inv_display_fg_set_freq(INV_DISPLAY_FG (pluginGui->display), value);
				break;
			case IFILTER_GAIN:
				inv_knob_set_value(INV_KNOB (pluginGui->knobGain), value);
				inv_display_fg_set_gain(INV_DISPLAY_FG (pluginGui->display), value);
				break;
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
		}
	}
}


static void init()
{
	IFilterGuiDescriptor =
	 (LV2UI_Descriptor *)malloc(sizeof(LV2UI_Descriptor));

	IFilterGuiDescriptor->URI 		= IFILTER_GUI_URI;
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


G_MODULE_EXPORT void on_filter_window_destroy(GtkObject *object, gpointer user_data)
{
	gtk_main_quit ();
	return;
}



