#include "libinv_widget-meter.h"


static void gtk_meter_class_init(GtkMeterClass *klass);
static void gtk_meter_init(GtkMeter *meter);
static void gtk_meter_size_request(GtkWidget *widget,
    GtkRequisition *requisition);
static void gtk_meter_size_allocate(GtkWidget *widget,
    GtkAllocation *allocation);
static void gtk_meter_realize(GtkWidget *widget);
static gboolean gtk_meter_expose(GtkWidget *widget,
    GdkEventExpose *event);
static void gtk_meter_paint(GtkWidget *widget);
static void gtk_meter_destroy(GtkObject *object);


GtkType
gtk_meter_get_type(void)
{
  static GtkType gtk_meter_type = 0;


  if (!gtk_meter_type) {
      static const GtkTypeInfo gtk_meter_info = {
          "GtkMeter",
          sizeof(GtkMeter),
          sizeof(GtkMeterClass),
          (GtkClassInitFunc) gtk_meter_class_init,
          (GtkObjectInitFunc) gtk_meter_init,
          NULL,
          NULL,
          (GtkClassInitFunc) NULL
      };
      gtk_meter_type = gtk_type_unique(GTK_TYPE_WIDGET, &gtk_meter_info);
  }


  return gtk_meter_type;
}

void
gtk_meter_set_state(GtkMeter *meter, gint num)
{
   meter->sel = num;
   gtk_meter_paint(GTK_WIDGET(meter));
}


GtkWidget * gtk_meter_new()
{
   return GTK_WIDGET(gtk_type_new(gtk_meter_get_type()));
}


static void
gtk_meter_class_init(GtkMeterClass *klass)
{
  GtkWidgetClass *widget_class;
  GtkObjectClass *object_class;


  widget_class = (GtkWidgetClass *) klass;
  object_class = (GtkObjectClass *) klass;

  widget_class->realize = gtk_meter_realize;
  widget_class->size_request = gtk_meter_size_request;
  widget_class->size_allocate = gtk_meter_size_allocate;
  widget_class->expose_event = gtk_meter_expose;

  object_class->destroy = gtk_meter_destroy;
}


static void
gtk_meter_init(GtkMeter *meter)
{
   meter->sel = 0;
}


static void
gtk_meter_size_request(GtkWidget *widget,
    GtkRequisition *requisition)
{
  g_return_if_fail(widget != NULL);
  g_return_if_fail(GTK_IS_METER(widget));
  g_return_if_fail(requisition != NULL);

  requisition->width = 107;
  requisition->height = 22;
}


static void
gtk_meter_size_allocate(GtkWidget *widget,
    GtkAllocation *allocation)
{
  g_return_if_fail(widget != NULL);
  g_return_if_fail(GTK_IS_METER(widget));
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
gtk_meter_realize(GtkWidget *widget)
{
  GdkWindowAttr attributes;
  guint attributes_mask;

  g_return_if_fail(widget != NULL);
  g_return_if_fail(GTK_IS_METER(widget));

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
gtk_meter_expose(GtkWidget *widget,
    GdkEventExpose *event)
{
  g_return_val_if_fail(widget != NULL, FALSE);
  g_return_val_if_fail(GTK_IS_METER(widget), FALSE);
  g_return_val_if_fail(event != NULL, FALSE);

  gtk_meter_paint(widget);

  return FALSE;
}


static void
gtk_meter_paint(GtkWidget *widget)
{
	cairo_t *cr;
	float on;

	cr = gdk_cairo_create(widget->window);

	cairo_translate(cr, 0, 0);

	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_paint(cr);

	gint pos = GTK_METER(widget)->sel;

	cairo_set_source_rgb(cr, 0.2, 0.4, 0);

	gint i;
	for ( i = 1; i <= 34; i++) {
		on= i < 32 ? 1 : 0;
		if(i< 27 ) cairo_set_source_rgb(cr, 0.2+(on*0.4), 0.4+(on*0.6), 0);
		else if (i< 31) cairo_set_source_rgb(cr, 0.4+(on*0.6), 0.4+(on*0.6), 0);
		else cairo_set_source_rgb(cr, 0.4+(on*0.6), 0.2, 0);

		cairo_rectangle(cr, i*3, 2, 2, 8 );
		cairo_rectangle(cr, i*3, 12, 2, 8);
		cairo_fill(cr);

	}
  	cairo_destroy(cr);
}


static void
gtk_meter_destroy(GtkObject *object)
{
  GtkMeter *meter;
  GtkMeterClass *klass;

  g_return_if_fail(object != NULL);
  g_return_if_fail(GTK_IS_METER(object));

  meter = GTK_METER(object);

  klass = gtk_type_class(gtk_widget_get_type());

  if (GTK_OBJECT_CLASS(klass)->destroy) {
     (* GTK_OBJECT_CLASS(klass)->destroy) (object);
  }
}


