#include "meter.h"


static void 	inv_meter_class_init(GtkMeterClass *klass);
static void 	inv_meter_init(GtkMeter *meter);
static void 	inv_meter_size_request(GtkWidget *widget, GtkRequisition *requisition);
static void 	inv_meter_size_allocate(GtkWidget *widget, GtkAllocation *allocation);
static void 	inv_meter_realize(GtkWidget *widget);
static gboolean inv_meter_expose(GtkWidget *widget,GdkEventExpose *event);
static void 	inv_meter_paint(GtkWidget *widget, gint mode);
static void	inv_meter_destroy(GtkObject *object);


GtkType
inv_meter_get_type(void)
{
	static GtkType inv_meter_type = 0;

	if (!inv_meter_type) {
		static const GtkTypeInfo inv_meter_info = {
		  "GtkMeter",
		  sizeof(GtkMeter),
		  sizeof(GtkMeterClass),
		  (GtkClassInitFunc) inv_meter_class_init,
		  (GtkObjectInitFunc) inv_meter_init,
		  NULL,
		  NULL,
		  (GtkClassInitFunc) NULL
		};
	inv_meter_type = gtk_type_unique(GTK_TYPE_WIDGET, &inv_meter_info);
	}
  return inv_meter_type;
}

void
inv_meter_set_channels(GtkMeter *meter, gint num)
{
	meter->channels = num;
}

void
inv_meter_set_LdB(GtkMeter *meter, float num)
{
	meter->LdB = num;
	inv_meter_paint(GTK_WIDGET(meter),1);
}

void
inv_meter_set_RdB(GtkMeter *meter, float num)
{
	meter->RdB = num;
	inv_meter_paint(GTK_WIDGET(meter),1);
}


GtkWidget * inv_meter_new()
{
	return GTK_WIDGET(gtk_type_new(inv_meter_get_type()));
}


static void
inv_meter_class_init(GtkMeterClass *klass)
{
	GtkWidgetClass *widget_class;
	GtkObjectClass *object_class;


	widget_class = (GtkWidgetClass *) klass;
	object_class = (GtkObjectClass *) klass;

	widget_class->realize = inv_meter_realize;
	widget_class->size_request = inv_meter_size_request;
	widget_class->size_allocate = inv_meter_size_allocate;
	widget_class->expose_event = inv_meter_expose;

	object_class->destroy = inv_meter_destroy;
}


static void
inv_meter_init(GtkMeter *meter)
{
	meter->channels = 1;
	meter->LdB = -90;
	meter->RdB = -90;
}


static void
inv_meter_size_request(GtkWidget *widget,
    GtkRequisition *requisition)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(INV_IS_METER(widget));
	g_return_if_fail(requisition != NULL);

	requisition->width = 107;
	requisition->height = 22;
}


static void
inv_meter_size_allocate(GtkWidget *widget,
    GtkAllocation *allocation)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(INV_IS_METER(widget));
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
inv_meter_realize(GtkWidget *widget)
{
	GdkWindowAttr attributes;
	guint attributes_mask;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(INV_IS_METER(widget));

	GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
	attributes.width = 107;
	attributes.height = 22;

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
inv_meter_expose(GtkWidget *widget, GdkEventExpose *event)
{
	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(INV_IS_METER(widget), FALSE);
	g_return_val_if_fail(event != NULL, FALSE);

	inv_meter_paint(widget,0);

	return FALSE;
}


static void
inv_meter_paint(GtkWidget *widget, gint mode)
{
	cairo_t *cr;
	float Lon,Ron;

	cr = gdk_cairo_create(widget->window);

	if(mode==0) {
		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_paint(cr);
	}

	gint channels = INV_METER(widget)->channels;
	gint Lpos = (gint)((INV_METER(widget)->LdB/2)+31);
	gint Rpos = (gint)((INV_METER(widget)->RdB/2)+31);

	cairo_set_source_rgb(cr, 0.2, 0.4, 0);

	gint i;
	for ( i = 1; i <= 34; i++) 
	{
		switch(channels)
		{
			case 1:
				Lon = i <= Lpos ? 1 : 0;
				if(i< 31 ) cairo_set_source_rgb(cr, 0.1+((float)i * 0.01)+(Lon*(0.3+((float)i * 0.01))), 0.4+(Lon*0.6), 0);
				else cairo_set_source_rgb(cr, 0.4+(Lon*0.6), 0.2, 0);
				cairo_rectangle(cr, i*3, 2, 2, 18 );
				cairo_fill(cr);
				break;
			case 2:
				Lon = i <= Lpos ? 1 : 0;
				Ron = i <= Rpos ? 1 : 0;
				if(i< 31 ) cairo_set_source_rgb(cr, 0.1+((float)i * 0.01)+(Lon*(0.3+((float)i * 0.01))), 0.4+(Lon*0.6), 0);
				else cairo_set_source_rgb(cr, 0.4+(Lon*0.6), 0.2, 0);
				cairo_rectangle(cr, i*3, 2, 2, 8 );
				cairo_fill(cr);
				if(i< 31 ) cairo_set_source_rgb(cr, 0.1+((float)i * 0.01)+(Ron*(0.3+((float)i * 0.01))), 0.4+(Ron*0.6), 0);
				else cairo_set_source_rgb(cr, 0.4+(Ron*0.6), 0.2, 0);
				cairo_rectangle(cr, i*3, 12, 2, 8);
				cairo_fill(cr);
				break; 
		}
	}
  	cairo_destroy(cr);
}


static void
inv_meter_destroy(GtkObject *object)
{
	GtkMeter *meter;
	GtkMeterClass *klass;

	g_return_if_fail(object != NULL);
	g_return_if_fail(INV_IS_METER(object));

	meter = INV_METER(object);

	klass = gtk_type_class(gtk_widget_get_type());

	if (GTK_OBJECT_CLASS(klass)->destroy) {
		(* GTK_OBJECT_CLASS(klass)->destroy) (object);
	}
}


