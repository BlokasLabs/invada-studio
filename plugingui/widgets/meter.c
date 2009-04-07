#include "meter.h"


static void 	inv_meter_class_init(InvMeterClass *klass);
static void 	inv_meter_init(InvMeter *meter);
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
		  "InvMeter",
		  sizeof(InvMeter),
		  sizeof(InvMeterClass),
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
inv_meter_set_channels(InvMeter *meter, gint num)
{
	meter->channels = num;
}

void
inv_meter_set_LdB(InvMeter *meter, float num)
{
	meter->LdB = num;
	if(GTK_WIDGET_REALIZED(meter))
		inv_meter_paint(GTK_WIDGET(meter),INV_METER_DRAW_DATA);
}

void
inv_meter_set_RdB(InvMeter *meter, float num)
{
	meter->RdB = num;
	if(GTK_WIDGET_REALIZED(meter))
		inv_meter_paint(GTK_WIDGET(meter),INV_METER_DRAW_DATA);
}


GtkWidget * inv_meter_new()
{
	return GTK_WIDGET(gtk_type_new(inv_meter_get_type()));
}


static void
inv_meter_class_init(InvMeterClass *klass)
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
inv_meter_init(InvMeter *meter)
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

	requisition->width = 113;
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
	attributes.width = 113;
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

	inv_meter_paint(widget,INV_METER_DRAW_ALL);

	return FALSE;
}


static void
inv_meter_paint(GtkWidget *widget, gint mode)
{
	cairo_t *cr;
	float Lon,Ron;

	cr = gdk_cairo_create(widget->window);

	gint channels = INV_METER(widget)->channels;
	gint Lpos = (gint)((INV_METER(widget)->LdB/2)+31);
	gint Rpos = (gint)((INV_METER(widget)->RdB/2)+31);

	if(mode==INV_METER_DRAW_ALL) {
		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_paint(cr);

		cairo_set_source_rgb(cr, 1, 1, 1);
		cairo_select_font_face(cr,"monospace",CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size(cr,6);
		if(channels==1) {
			cairo_move_to(cr,2,13);
			cairo_show_text(cr,"M");
		}
		if(channels==2) {
			cairo_move_to(cr,2,8);
			cairo_show_text(cr,"L");
			cairo_move_to(cr,2,18);
			cairo_show_text(cr,"R");
		}
	}

	gint i;
	for ( i = 1; i <= 34; i++) 
	{
		switch(channels)
		{
			case 1:
				Lon = i <= Lpos ? 1 : 0;
				if(i< 31 ) cairo_set_source_rgb(cr, 0.1+((float)i * 0.01)+(Lon*(0.3+((float)i * 0.01))), 0.4+(Lon*0.6), 0);
				else cairo_set_source_rgb(cr, 0.4+(Lon*0.6), 0.2, 0);
				cairo_rectangle(cr, 6+(i*3), 2, 2, 18 );
				cairo_fill(cr);
				break;
			case 2:
				Lon = i <= Lpos ? 1 : 0;
				Ron = i <= Rpos ? 1 : 0;
				if(i< 31 ) cairo_set_source_rgb(cr, 0.1+((float)i * 0.01)+(Lon*(0.3+((float)i * 0.01))), 0.4+(Lon*0.6), 0);
				else cairo_set_source_rgb(cr, 0.4+(Lon*0.6), 0.2, 0);
				cairo_rectangle(cr, 6+(i*3), 2, 2, 8 );
				cairo_fill(cr);
				if(i< 31 ) cairo_set_source_rgb(cr, 0.1+((float)i * 0.01)+(Ron*(0.3+((float)i * 0.01))), 0.4+(Ron*0.6), 0);
				else cairo_set_source_rgb(cr, 0.4+(Ron*0.6), 0.2, 0);
				cairo_rectangle(cr, 6+(i*3), 12, 2, 8);
				cairo_fill(cr);
				break; 
		}
	}
  	cairo_destroy(cr);
}


static void
inv_meter_destroy(GtkObject *object)
{
	InvMeter *meter;
	InvMeterClass *klass;

	g_return_if_fail(object != NULL);
	g_return_if_fail(INV_IS_METER(object));

	meter = INV_METER(object);

	klass = gtk_type_class(gtk_widget_get_type());

	if (GTK_OBJECT_CLASS(klass)->destroy) {
		(* GTK_OBJECT_CLASS(klass)->destroy) (object);
	}
}


