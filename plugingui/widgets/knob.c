#include "knob.h"


static void 	inv_knob_class_init(InvKnobClass *klass);
static void 	inv_knob_init(InvKnob *knob);
static void 	inv_knob_size_request(GtkWidget *widget, GtkRequisition *requisition);
static void 	inv_knob_size_allocate(GtkWidget *widget, GtkAllocation *allocation);
static void 	inv_knob_realize(GtkWidget *widget);
static gboolean inv_knob_expose(GtkWidget *widget,GdkEventExpose *event);
static void 	inv_knob_paint(GtkWidget *widget, gint mode);
static void	inv_knob_destroy(GtkObject *object);


GtkType
inv_knob_get_type(void)
{
	static GtkType inv_knob_type = 0;

	if (!inv_knob_type) {
		static const GtkTypeInfo inv_knob_info = {
		  "InvKnob",
		  sizeof(InvKnob),
		  sizeof(InvKnobClass),
		  (GtkClassInitFunc) inv_knob_class_init,
		  (GtkObjectInitFunc) inv_knob_init,
		  NULL,
		  NULL,
		  (GtkClassInitFunc) NULL
		};
	inv_knob_type = gtk_type_unique(GTK_TYPE_WIDGET, &inv_knob_info);
	}
  return inv_knob_type;
}

void
inv_knob_set_size(InvKnob *knob, gint num)
{
	knob->size = num;
}

void
inv_knob_set_curve(InvKnob *knob, gint num)
{
	knob->curve = num;
}

void
inv_knob_set_markings(InvKnob *knob, gint num)
{
	knob->markings = num;
}

void
inv_knob_set_highlight(InvKnob *knob, gint num)
{
	knob->highlight = num;
}

void
inv_knob_set_min(InvKnob *knob, float num)
{
	knob->min = num;
}

void
inv_knob_set_max(InvKnob *knob, float num)
{
	knob->max = num;
}

void
inv_knob_set_value(InvKnob *knob, float num)
{
	knob->value = num;
	if(GTK_WIDGET_REALIZED(knob))
		inv_knob_paint(GTK_WIDGET(knob),INV_KNOB_DRAW_DATA);
}


GtkWidget * inv_knob_new()
{
	return GTK_WIDGET(gtk_type_new(inv_knob_get_type()));
}


static void
inv_knob_class_init(InvKnobClass *klass)
{
	GtkWidgetClass *widget_class;
	GtkObjectClass *object_class;


	widget_class = (GtkWidgetClass *) klass;
	object_class = (GtkObjectClass *) klass;

	widget_class->realize = inv_knob_realize;
	widget_class->size_request = inv_knob_size_request;
	widget_class->size_allocate = inv_knob_size_allocate;
	widget_class->expose_event = inv_knob_expose;

	object_class->destroy = inv_knob_destroy;
}


static void
inv_knob_init(InvKnob *knob)
{
	knob->size      = INV_KNOB_SIZE_MEDIUM;
	knob->curve     = INV_KNOB_CURVE_LINEAR;
	knob->markings  = INV_KNOB_MARKINGS_5;
	knob->highlight = INV_KNOB_HIGHLIGHT_L;
	knob->min       = 0.0;
	knob->max       = 1.0;
	knob->value     = 0.5;
}

static void
inv_knob_size_request(GtkWidget *widget,
    GtkRequisition *requisition)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(INV_IS_KNOB(widget));
	g_return_if_fail(requisition != NULL);

	requisition->width  = INV_KNOB (widget)->size+2;
	requisition->height = INV_KNOB (widget)->size+14;
}


static void
inv_knob_size_allocate(GtkWidget *widget,
    GtkAllocation *allocation)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(INV_IS_KNOB(widget));
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
inv_knob_realize(GtkWidget *widget)
{
	GdkWindowAttr attributes;
	guint attributes_mask;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(INV_IS_KNOB(widget));

	GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
	attributes.width = INV_KNOB (widget)->size + 2;
	attributes.height = INV_KNOB (widget)->size + 14;

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
inv_knob_expose(GtkWidget *widget, GdkEventExpose *event)
{
	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(INV_IS_KNOB(widget), FALSE);
	g_return_val_if_fail(event != NULL, FALSE);

	inv_knob_paint(widget,INV_KNOB_DRAW_ALL);

	return FALSE;
}


static void
inv_knob_paint(GtkWidget *widget, gint mode)
{
	cairo_t *cr;
	gint size;

	float xc,yc,x,y,a,b,r;

	cr = gdk_cairo_create(widget->window);

	size = INV_KNOB(widget)->size;

	xc=(size/2)+1;
	yc=(size/2)+1;
	r=size/2;

	if(mode==INV_KNOB_DRAW_ALL) {
		cairo_set_source_rgb(cr, 0.05, 0.05, 0.2);
		cairo_arc(cr,xc,yc,r,0,2*INV_PI);
		cairo_fill(cr);
	/*	
		cairo_move_to(cr,r-13,0);
		cairo_line_to(cr,r-10,0);

		cairo_rotate(cr,INV_PI/4);
		cairo_translate(cr,xc,yc);

		cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
		cairo_stroke(cr);
	*/
		
	}


	cairo_arc(cr,xc,yc,r-15,0,2*INV_PI);
	cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
	cairo_fill_preserve(cr);
	cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
	cairo_stroke(cr);

  	cairo_destroy(cr);
}


static void
inv_knob_destroy(GtkObject *object)
{
	InvKnob *knob;
	InvKnobClass *klass;

	g_return_if_fail(object != NULL);
	g_return_if_fail(INV_IS_KNOB(object));

	knob = INV_KNOB(object);

	klass = gtk_type_class(gtk_widget_get_type());

	if (GTK_OBJECT_CLASS(klass)->destroy) {
		(* GTK_OBJECT_CLASS(klass)->destroy) (object);
	}
}


