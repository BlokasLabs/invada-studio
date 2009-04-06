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


static LV2UI_Descriptor *IFilterLPFGuiDescriptor = NULL;
static LV2UI_Descriptor *IFilterHPFGuiDescriptor = NULL;


typedef struct {
 
	GtkWidget       *window;
	GtkWidget	*container;

} IFilterGui;


static LV2UI_Handle instantiateIFilterGui(const struct _LV2UI_Descriptor* descriptor, const char* plugin_uri, const char* bundle_path, LV2UI_Write_Function write_function, LV2UI_Controller controller, LV2UI_Widget* widget, const LV2_Feature* const* features)
{
	IFilterGui *pluginGui = (IFilterGui *)malloc(sizeof(IFilterGui));
	if(pluginGui==NULL)
		return NULL;

	GtkBuilder      *builder; 
	GtkWidget	*object;
	GtkWidget	*meterIn;
	GtkWidget	*meterOut;
	GtkWidget	*display;

	GError *err = NULL;

	gtk_init (NULL,NULL);

	builder = gtk_builder_new ();
	gtk_builder_add_from_file (builder, "/usr/local/lib/lv2/invada.lv2/gtk/inv_filter_gui.xml", &err);
	pluginGui->window = GTK_WIDGET (gtk_builder_get_object (builder, "filter_window"));
	pluginGui->container = GTK_WIDGET (gtk_builder_get_object (builder, "vbox1"));


	object=GTK_WIDGET (gtk_builder_get_object (builder, "Fixed_meter_in_display"));
	meterIn = gtk_meter_new ();
	gtk_container_add (GTK_CONTAINER (object), meterIn);

	object=GTK_WIDGET (gtk_builder_get_object (builder, "Fixed_meter_out_display"));
	meterOut = gtk_meter_new ();
	gtk_container_add (GTK_CONTAINER (object), meterOut);


	gtk_widget_ref(pluginGui->container);
	gtk_container_remove(GTK_CONTAINER(pluginGui->window), pluginGui->container);

	*widget = (LV2UI_Widget) pluginGui->container;

//	g_signal_connect (object, "destroy", G_CALLBACK(on_filter_window_destroy),NULL); 

	g_object_unref (G_OBJECT (builder));
             

	return pluginGui;
}


static void cleanupIFilterGui(LV2UI_Handle ui)
{
//	gtk_main_quit ();
	return;
}


static void port_eventIFilterGui(LV2UI_Handle ui, uint32_t port_index, uint32_t buffer_size, uint32_t format, const void*  buffer)
{
	return;
}


static void init()
{
	IFilterLPFGuiDescriptor =
	 (LV2UI_Descriptor *)malloc(sizeof(LV2UI_Descriptor));

	IFilterLPFGuiDescriptor->URI 		= IFILTER_LPF_GUI_URI;
	IFilterLPFGuiDescriptor->instantiate 	= instantiateIFilterGui;
	IFilterLPFGuiDescriptor->cleanup 	= cleanupIFilterGui;
	IFilterLPFGuiDescriptor->port_event	= port_eventIFilterGui;
	IFilterLPFGuiDescriptor->extension_data = NULL;

	IFilterHPFGuiDescriptor =
	 (LV2UI_Descriptor *)malloc(sizeof(LV2UI_Descriptor));

	IFilterHPFGuiDescriptor->URI 		= IFILTER_HPF_GUI_URI;
	IFilterHPFGuiDescriptor->instantiate 	= instantiateIFilterGui;
	IFilterHPFGuiDescriptor->cleanup 	= cleanupIFilterGui;
	IFilterHPFGuiDescriptor->port_event	= port_eventIFilterGui;
	IFilterHPFGuiDescriptor->extension_data	= NULL;
}


const LV2UI_Descriptor* lv2ui_descriptor(uint32_t index)
{
	if (!IFilterLPFGuiDescriptor) init();

	switch (index) {
		case 0:
			return IFilterLPFGuiDescriptor;
		case 1:
			return IFilterHPFGuiDescriptor;
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



