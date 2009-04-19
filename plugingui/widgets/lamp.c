#include "stdlib.h"
#include "string.h"
#include "lamp.h"

static void 	inv_lamp_class_init(InvLampClass *klass);
static void 	inv_lamp_init(InvLamp *lamp);
static void 	inv_lamp_size_request(GtkWidget *widget, GtkRequisition *requisition);
static void 	inv_lamp_size_allocate(GtkWidget *widget, GtkAllocation *allocation);
static void 	inv_lamp_realize(GtkWidget *widget);
static gboolean inv_lamp_expose(GtkWidget *widget,GdkEventExpose *event);
static void 	inv_lamp_paint(GtkWidget *widget, gint mode);
static void	inv_lamp_destroy(GtkObject *object);
static void	inv_lamp_colour(GtkWidget *widget, float value, float *Rr, float *Gr, float *Br, float *Rc, float *Gc, float *Bc);


GtkType
inv_lamp_get_type(void)
{
	static GType inv_lamp_type = 0;
	char *name;
	int i;


	if (!inv_lamp_type) 
	{
		static const GTypeInfo type_info = {
			sizeof(InvLampClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc)inv_lamp_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof(InvLamp),
			0,    /* n_preallocs */
			(GInstanceInitFunc)inv_lamp_init
		};
		for (i = 0; ; i++) {
			name = g_strdup_printf("InvLamp-%p-%d",inv_lamp_class_init, i);
			if (g_type_from_name(name)) {
				free(name);
				continue;
			}
			inv_lamp_type = g_type_register_static(GTK_TYPE_WIDGET,name,&type_info,(GTypeFlags)0);
			free(name);
			break;
		}
	}
	return inv_lamp_type;
}

void
inv_lamp_set_scale(InvLamp *lamp, float num)
{
	lamp->scale = num;
}

void
inv_lamp_set_value(InvLamp *lamp, float num)
{
	lamp->value = num;
	if(GTK_WIDGET_REALIZED(lamp))
		inv_lamp_paint(GTK_WIDGET(lamp),INV_LAMP_DRAW_DATA);
}


GtkWidget * inv_lamp_new()
{
	return GTK_WIDGET(gtk_type_new(inv_lamp_get_type()));
}


static void
inv_lamp_class_init(InvLampClass *klass)
{
	GtkWidgetClass *widget_class;
	GtkObjectClass *object_class;


	widget_class = (GtkWidgetClass *) klass;
	object_class = (GtkObjectClass *) klass;

	widget_class->realize = inv_lamp_realize;
	widget_class->size_request = inv_lamp_size_request;
	widget_class->size_allocate = inv_lamp_size_allocate;
	widget_class->expose_event = inv_lamp_expose;

	object_class->destroy = inv_lamp_destroy;
}


static void
inv_lamp_init(InvLamp *lamp)
{
	lamp->scale = 1;
	lamp->value = 0;
	lamp->lastValue=0;

	lamp->l0_r[0] =0.1;	lamp->l0_r[1] =0.0;	lamp->l0_r[2] =0.0;
	lamp->l0_c[0] =0.2;	lamp->l0_c[1] =0.0;	lamp->l0_c[2] =0.0;

	lamp->l1_r[0] =0.2;	lamp->l1_r[1] =0.0;	lamp->l1_r[2] =0.0;
	lamp->l1_c[0] =1.0;	lamp->l1_c[1] =0.0;	lamp->l1_c[2] =0.0;

	lamp->l2_r[0] =0.3;	lamp->l2_r[1] =0.0;	lamp->l2_r[2] =0.0;
	lamp->l2_c[0] =1.0;	lamp->l2_c[1] =0.5;	lamp->l2_c[2] =0.0;

	lamp->l3_r[0] =0.4;	lamp->l3_r[1] =0.0;	lamp->l3_r[2] =0.0;
	lamp->l3_c[0] =1.0;	lamp->l3_c[1] =1.0;	lamp->l3_c[2] =0.0;

	lamp->l4_r[0] =0.5;	lamp->l4_r[1] =0.0;	lamp->l4_r[2] =0.0;
	lamp->l4_c[0] =1.0;	lamp->l4_c[1] =1.0;	lamp->l4_c[2] =0.5;   
}


static void
inv_lamp_size_request(GtkWidget *widget,
    GtkRequisition *requisition)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(INV_IS_LAMP(widget));
	g_return_if_fail(requisition != NULL);

	requisition->width = 32;
	requisition->height = 32;
}


static void
inv_lamp_size_allocate(GtkWidget *widget,
    GtkAllocation *allocation)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(INV_IS_LAMP(widget));
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
inv_lamp_realize(GtkWidget *widget)
{
	GdkWindowAttr attributes;
	guint attributes_mask;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(INV_IS_LAMP(widget));

	GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
	attributes.width = 32;
	attributes.height = 32;

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
inv_lamp_expose(GtkWidget *widget, GdkEventExpose *event)
{
	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(INV_IS_LAMP(widget), FALSE);
	g_return_val_if_fail(event != NULL, FALSE);

	inv_lamp_paint(widget,INV_LAMP_DRAW_ALL);

	return FALSE;
}


static void
inv_lamp_paint(GtkWidget *widget, gint mode)
{
	cairo_t 	*cr;
	float 		Rr,Gr,Br,Rc,Gc,Bc,xc,yc,r;
	GtkStyle	*style;
	cairo_pattern_t *pat;

	style = gtk_widget_get_style(widget);

	float scale = INV_LAMP(widget)->scale;
	float value = INV_LAMP(widget)->value;

	cr = gdk_cairo_create(widget->window);

	xc=16.0;
	yc=16.0;
	r=9.5;

	switch(mode) {
		case INV_LAMP_DRAW_ALL:

			cairo_arc(cr,xc,yc,13,0,2*INV_PI);
			cairo_set_source_rgb(cr, 0, 0, 0);
			cairo_fill_preserve(cr);

			pat = cairo_pattern_create_linear (0.0, 0.0,  32.0, 32.0);
			cairo_pattern_add_color_stop_rgba (pat, 0.0, 1.00, 1.00, 1.00, 1);
			cairo_pattern_add_color_stop_rgba (pat, 0.2, 0.91, 0.89, 0.83, 1);
			cairo_pattern_add_color_stop_rgba (pat, 0.5, 0.43, 0.32, 0.26, 1);
			cairo_pattern_add_color_stop_rgba (pat, 0.8, 0.10, 0.05, 0.04, 1);
			cairo_pattern_add_color_stop_rgba (pat, 1.0, 0.00, 0.00, 0.00, 1);
			cairo_set_source (cr, pat);
			cairo_set_line_width(cr,5);

			cairo_stroke(cr);


		case INV_LAMP_DRAW_DATA:

			pat = cairo_pattern_create_radial (xc-1, yc-1, 1.5,
						           xc,  yc, r);
			inv_lamp_colour(widget, value*scale, &Rr, &Gr, &Br, &Rc, &Gc, &Bc);
			cairo_pattern_add_color_stop_rgba (pat, 0.0, Rc,  Gc,  Bc,  1);
			cairo_pattern_add_color_stop_rgba (pat, 0.7, Rr,  Gr,  Br,  1);
			cairo_pattern_add_color_stop_rgba (pat, 0.9, 0.1, 0.0, 0.0, 1);
			cairo_pattern_add_color_stop_rgba (pat, 1.0, 0.1, 0.0, 0.0, 0);
			cairo_set_source (cr, pat);
			cairo_arc(cr,xc,yc,r,0,2*INV_PI);
			cairo_fill(cr);

			INV_LAMP(widget)->lastValue = value;
			break;


	}
  	cairo_destroy(cr);
}


static void
inv_lamp_destroy(GtkObject *object)
{
	InvLamp *lamp;
	InvLampClass *klass;

	g_return_if_fail(object != NULL);
	g_return_if_fail(INV_IS_LAMP(object));

	lamp = INV_LAMP(object);

	klass = gtk_type_class(gtk_widget_get_type());

	if (GTK_OBJECT_CLASS(klass)->destroy) {
		(* GTK_OBJECT_CLASS(klass)->destroy) (object);
	}
}

static void	
inv_lamp_colour(GtkWidget *widget, float value, float *Rr, float *Gr, float *Br, float *Rc, float *Gc, float *Bc)
{
	float f1,f2;
	float *r0;
	float *c0;
	float *r1;
	float *c1;

	if(value <= 0) 
	{
		*Rr=INV_LAMP(widget)->l0_r[0];
		*Gr=INV_LAMP(widget)->l0_r[1];
		*Br=INV_LAMP(widget)->l0_r[2];

		*Rc=INV_LAMP(widget)->l0_c[0];
		*Gc=INV_LAMP(widget)->l0_c[1];
		*Bc=INV_LAMP(widget)->l0_c[2];	
	} 

	else if (value < 1)
	{
		r0=INV_LAMP(widget)->l0_r;
		c0=INV_LAMP(widget)->l0_c;

		r1=INV_LAMP(widget)->l1_r;
		c1=INV_LAMP(widget)->l1_c;

		f1=1-value;
		f2=value;

		*Rr=(f1 * r0[0]) + (f2 * r1[0]);
		*Gr=(f1 * r0[1]) + (f2 * r1[1]);
		*Br=(f1 * r0[2]) + (f2 * r1[2]);

		*Rc=(f1 * c0[0]) + (f2 * c1[0]);
		*Gc=(f1 * c0[1]) + (f2 * c1[1]);
		*Bc=(f1 * c0[2]) + (f2 * c1[2]);
	}

	else if (value < 2)
	{
		r0=INV_LAMP(widget)->l1_r;
		c0=INV_LAMP(widget)->l1_c;

		r1=INV_LAMP(widget)->l2_r;
		c1=INV_LAMP(widget)->l2_c;

		f1=2-value;
		f2=value-1;

		*Rr=(f1 * r0[0]) + (f2 * r1[0]);
		*Gr=(f1 * r0[1]) + (f2 * r1[1]);
		*Br=(f1 * r0[2]) + (f2 * r1[2]);

		*Rc=(f1 * c0[0]) + (f2 * c1[0]);
		*Gc=(f1 * c0[1]) + (f2 * c1[1]);
		*Bc=(f1 * c0[2]) + (f2 * c1[2]);
	}
	else if (value < 3)
	{
		r0=INV_LAMP(widget)->l2_r;
		c0=INV_LAMP(widget)->l2_c;

		r1=INV_LAMP(widget)->l3_r;
		c1=INV_LAMP(widget)->l3_c;

		f1=3-value;
		f2=value-2;

		*Rr=(f1 * r0[0]) + (f2 * r1[0]);
		*Gr=(f1 * r0[1]) + (f2 * r1[1]);
		*Br=(f1 * r0[2]) + (f2 * r1[2]);

		*Rc=(f1 * c0[0]) + (f2 * c1[0]);
		*Gc=(f1 * c0[1]) + (f2 * c1[1]);
		*Bc=(f1 * c0[2]) + (f2 * c1[2]);
	}
	else if (value < 4)
	{
		r0=INV_LAMP(widget)->l3_r;
		c0=INV_LAMP(widget)->l3_c;

		r1=INV_LAMP(widget)->l4_r;
		c1=INV_LAMP(widget)->l4_c;

		f1=4-value;
		f2=value-3;

		*Rr=(f1 * r0[0]) + (f2 * r1[0]);
		*Gr=(f1 * r0[1]) + (f2 * r1[1]);
		*Br=(f1 * r0[2]) + (f2 * r1[2]);

		*Rc=(f1 * c0[0]) + (f2 * c1[0]);
		*Gc=(f1 * c0[1]) + (f2 * c1[1]);
		*Bc=(f1 * c0[2]) + (f2 * c1[2]);
	}
	else
	{
		*Rr=INV_LAMP(widget)->l4_r[0];
		*Gr=INV_LAMP(widget)->l4_r[1];
		*Br=INV_LAMP(widget)->l4_r[2];

		*Rc=INV_LAMP(widget)->l4_c[0];
		*Gc=INV_LAMP(widget)->l4_c[1];
		*Bc=INV_LAMP(widget)->l4_c[2];	
	}

}



