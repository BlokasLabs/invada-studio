/* 

    This widget provides vu meters

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
#include "widgets.h"
#include "meter-vu.h"


static void 	inv_vu_meter_class_init(InvVuMeterClass *klass);
static void 	inv_vu_meter_init(InvVuMeter *meter);
static void 	inv_vu_meter_size_request(GtkWidget *widget, GtkRequisition *requisition);
static void 	inv_vu_meter_size_allocate(GtkWidget *widget, GtkAllocation *allocation);
static void 	inv_vu_meter_realize(GtkWidget *widget);
static gboolean inv_vu_meter_expose(GtkWidget *widget,GdkEventExpose *event);
static void 	inv_vu_meter_paint(GtkWidget *widget, gint mode);
static void	inv_vu_meter_destroy(GtkObject *object);



GtkType
inv_vu_meter_get_type(void)
{
	static GType inv_vu_meter_type = 0;
	char *name;
	int i;


	if (!inv_vu_meter_type) 
	{
		static const GTypeInfo type_info = {
			sizeof(InvVuMeterClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc)inv_vu_meter_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof(InvVuMeter),
			0,    /* n_preallocs */
			(GInstanceInitFunc)inv_vu_meter_init
		};
		for (i = 0; ; i++) {
			name = g_strdup_printf("InvVuMeter-%p-%d",inv_vu_meter_class_init, i);
			if (g_type_from_name(name)) {
				free(name);
				continue;
			}
			inv_vu_meter_type = g_type_register_static(GTK_TYPE_WIDGET,name,&type_info,(GTypeFlags)0);
			free(name);
			break;
		}
	}
	return inv_vu_meter_type;
}

void
inv_vu_meter_set_bypass(InvVuMeter *meter, gint num)
{
	if(meter->bypass != num) {
		meter->bypass = num;
		meter->value=0;
	}
}

void
inv_vu_meter_set_value(InvVuMeter *meter, float num)
{
	meter->value = num;
	if(GTK_WIDGET_REALIZED(meter) && meter->value != meter->lastvalue)
		inv_vu_meter_paint(GTK_WIDGET(meter),INV_VU_METER_DRAW_DATA);
}

GtkWidget * inv_vu_meter_new()
{
	return GTK_WIDGET(gtk_type_new(inv_vu_meter_get_type()));
}


static void
inv_vu_meter_class_init(InvVuMeterClass *klass)
{
	GtkWidgetClass *widget_class;
	GtkObjectClass *object_class;


	widget_class = (GtkWidgetClass *) klass;
	object_class = (GtkObjectClass *) klass;

	widget_class->realize = inv_vu_meter_realize;
	widget_class->size_request = inv_vu_meter_size_request;
	widget_class->size_allocate = inv_vu_meter_size_allocate;
	widget_class->expose_event = inv_vu_meter_expose;

	object_class->destroy = inv_vu_meter_destroy;
}


static void
inv_vu_meter_init(InvVuMeter *meter)
{
	meter->bypass = INV_PLUGIN_ACTIVE;
	meter->value = 0;

	gtk_widget_set_tooltip_markup(GTK_WIDGET(meter),"<span size=\"8000\">VU Meter.</span>");
}


static void
inv_vu_meter_size_request(GtkWidget *widget,
    GtkRequisition *requisition)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(INV_IS_VU_METER(widget));
	g_return_if_fail(requisition != NULL);

	requisition->width = 183;
	requisition->height = 100;
}


static void
inv_vu_meter_size_allocate(GtkWidget *widget,
    GtkAllocation *allocation)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(INV_IS_VU_METER(widget));
	g_return_if_fail(allocation != NULL);

	widget->allocation = *allocation;

	if (GTK_WIDGET_REALIZED(widget)) {
		gdk_window_move_resize(
		   widget->window,
		   allocation->x, allocation->y,
		   allocation->width, allocation->height
		);
	}
}


static void
inv_vu_meter_realize(GtkWidget *widget)
{
	GdkWindowAttr attributes;
	guint attributes_mask;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(INV_IS_VU_METER(widget));

	GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
	attributes.width = 183;
	attributes.height = 100;

	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.event_mask = gtk_widget_get_events(widget) | GDK_EXPOSURE_MASK;

	attributes_mask = GDK_WA_X | GDK_WA_Y;

	widget->window = gdk_window_new(
	  gtk_widget_get_parent_window (widget),
	  & attributes, attributes_mask
	);

	gdk_window_set_user_data(widget->window, widget);

	widget->style = gtk_style_attach(widget->style, widget->window);
	gtk_style_set_background(widget->style, widget->window, GTK_STATE_NORMAL);
}


static gboolean
inv_vu_meter_expose(GtkWidget *widget, GdkEventExpose *event)
{
	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(INV_IS_VU_METER(widget), FALSE);
	g_return_val_if_fail(event != NULL, FALSE);

	inv_vu_meter_paint(widget,INV_VU_METER_DRAW_ALL);

	return FALSE;
}


static void
inv_vu_meter_paint(GtkWidget *widget, gint mode)
{
	float 			value;
	gint 			bypass;

	cairo_t 		*cr;
	GtkStyle		*style;

	char 			label[10];
	cairo_text_extents_t 	extents;


	bypass  = INV_VU_METER(widget)->bypass;
	value   = INV_VU_METER(widget)->value*4;  //add 12dB

	style   = gtk_widget_get_style(widget);


	cr = gdk_cairo_create(widget->window);

	if(mode==INV_VU_METER_DRAW_ALL) {
		cairo_set_source_rgb(cr, 1.0, 1.0, 0.9);
		cairo_paint(cr);

	}

	cairo_set_source_rgb(cr, 1.0, 1.0, 0.9);
	cairo_paint(cr);

	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_move_to(cr,91,100);
	cairo_line_to(cr,91+80*(sin(value-0.7943)),100-80*cos(value-0.7943));
  	cairo_stroke(cr);
	
  	cairo_destroy(cr);
}


static void
inv_vu_meter_destroy(GtkObject *object)
{
	InvVuMeter *meter;
	InvVuMeterClass *klass;

	g_return_if_fail(object != NULL);
	g_return_if_fail(INV_IS_VU_METER(object));

	meter = INV_VU_METER(object);

	klass = gtk_type_class(gtk_widget_get_type());

	if (GTK_OBJECT_CLASS(klass)->destroy) {
		(* GTK_OBJECT_CLASS(klass)->destroy) (object);
	}
}


