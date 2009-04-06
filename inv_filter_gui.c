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
#include "libinv_widget-meter.h"
#include "lv2_ui.h"
#include "inv_filter.h"
#include "inv_filter_gui.h"


static LV2UI_Descriptor *IFilterGuiDescriptor = NULL;


typedef struct {
	GtkWidget	*windowContainer;
	GtkWidget	*meterIn;
	GtkWidget	*meterOut;
//	GtkWidget	*display;
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
	gtk_builder_add_from_file (builder, "/usr/local/lib/lv2/invada.lv2/gtk/inv_filter_gui.xml", &err);
	window = GTK_WIDGET (gtk_builder_get_object (builder, "filter_window"));
	pluginGui->windowContainer = GTK_WIDGET (gtk_builder_get_object (builder, "vbox1"));


	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "Fixed_meter_in_display"));
	pluginGui->meterIn = gtk_meter_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->meterIn);

	tempObject=GTK_WIDGET (gtk_builder_get_object (builder, "Fixed_meter_out_display"));
	pluginGui->meterOut = gtk_meter_new ();
	gtk_container_add (GTK_CONTAINER (tempObject), pluginGui->meterOut);

	gtk_meter_set_channels(GTK_METER (pluginGui->meterIn), 2);
	gtk_meter_set_channels(GTK_METER (pluginGui->meterOut), 2);


	gtk_widget_ref(pluginGui->windowContainer);
	gtk_container_remove(GTK_CONTAINER(window), pluginGui->windowContainer);

	*widget = (LV2UI_Widget) pluginGui->windowContainer;

	g_object_unref (G_OBJECT (builder));
             

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
			case IFILTER_METER_INL:
				gtk_meter_set_LdB(GTK_METER (pluginGui->meterIn),value);
				break;
			case IFILTER_METER_INR:
				gtk_meter_set_RdB(GTK_METER (pluginGui->meterIn),value);
				break;
			case IFILTER_METER_OUTL:
				gtk_meter_set_LdB(GTK_METER (pluginGui->meterOut),value);
				break;
			case IFILTER_METER_OUTR:
				gtk_meter_set_RdB(GTK_METER (pluginGui->meterOut),value);
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



