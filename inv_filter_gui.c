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
#include "inv_filter.h"
#include "inv_filter_gui.h"


static LV2UI_Descriptor *IFilterLPFGuiDescriptor = NULL;
static LV2UI_Descriptor *IFilterHPFGuiDescriptor = NULL;


static LV2UI_Handle instantiateIFilterGui(const struct _LV2UI_Descriptor* descriptor, const char* plugin_uri, const char* bundle_path, LV2UI_Write_Function write_function, LV2UI_Controller controller, LV2UI_Widget* widget, const LV2_Feature* const* features)
{
	GtkBuilder      *builder; 
	GtkWidget       *window;

	gtk_init (NULL,NULL);

	builder = gtk_builder_new ();
	gtk_builder_add_from_file (builder, "gtk/inv_filter.glade", NULL);
	window = GTK_WIDGET (gtk_builder_get_object (builder, "window_filter"));
	gtk_builder_connect_signals (builder, NULL);

	g_object_unref (G_OBJECT (builder));

	gtk_widget_show (window);                
	gtk_main ();

	return NULL;
}


static void cleanupIFilterGui(LV2UI_Handle ui)
{
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


void on_window_filter_destroy(GtkObject *object, gpointer user_data)
{
    gtk_main_quit ();
}



