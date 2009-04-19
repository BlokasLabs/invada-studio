#include "stdlib.h"
#include "string.h"
#include "meter-peak.h"


static void 	inv_meter_class_init(InvMeterClass *klass);
static void 	inv_meter_init(InvMeter *meter);
static void 	inv_meter_size_request(GtkWidget *widget, GtkRequisition *requisition);
static void 	inv_meter_size_allocate(GtkWidget *widget, GtkAllocation *allocation);
static void 	inv_meter_realize(GtkWidget *widget);
static gboolean inv_meter_expose(GtkWidget *widget,GdkEventExpose *event);
static void 	inv_meter_paint(GtkWidget *widget, gint mode);
static void	inv_meter_destroy(GtkObject *object);
static void	inv_meter_colour(GtkWidget *widget, gint pos, gint on, float *R, float *G, float *B);


GtkType
inv_meter_get_type(void)
{
	static GType inv_meter_type = 0;
	char *name;
	int i;


	if (!inv_meter_type) 
	{
		static const GTypeInfo type_info = {
			sizeof(InvMeterClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc)inv_meter_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof(InvMeter),
			0,    /* n_preallocs */
			(GInstanceInitFunc)inv_meter_init
		};
		for (i = 0; ; i++) {
			name = g_strdup_printf("InvMeter-%p-%d",inv_meter_class_init, i);
			if (g_type_from_name(name)) {
				free(name);
				continue;
			}
			inv_meter_type = g_type_register_static(GTK_TYPE_WIDGET,name,&type_info,(GTypeFlags)0);
			free(name);
			break;
		}
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
		inv_meter_paint(GTK_WIDGET(meter),INV_METER_DRAW_L);
}

void
inv_meter_set_RdB(InvMeter *meter, float num)
{
	meter->RdB = num;
	if(GTK_WIDGET_REALIZED(meter))
		inv_meter_paint(GTK_WIDGET(meter),INV_METER_DRAW_R);
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
	meter->lastLpos = 1;
	meter->lastRpos = 1;


	meter->mOff60[0] =0.1;	meter->mOff60[1] =0.1;	meter->mOff60[2] =0.4;
	meter->mOn60[0]  =-0.1;	meter->mOn60[1]  =-0.1;	meter->mOn60[2]  =0.6;

	meter->mOff12[0] =0.2; 	meter->mOff12[1] =0.3;	meter->mOff12[2] =0.4;
	meter->mOn12[0]  =-0.1;	meter->mOn12[1]  =0.3;	meter->mOn12[2]  =0.6;

	meter->mOff6[0] =0.2; 	meter->mOff6[1] =0.4;	meter->mOff6[2] =0.2;
	meter->mOn6[0]  =0.1;	meter->mOn6[1]  =0.6;	meter->mOn6[2]  =-0.1;

	meter->mOff0[0]  =0.5;	meter->mOff0[1]  =0.5;	meter->mOff0[2]  =0.0;
	meter->mOn0[0]   =0.5;	meter->mOn0[1]   =0.5;	meter->mOn0[2]   =0.0;

	meter->overOff[0]=0.4;	meter->overOff[1]=0.2;	meter->overOff[2]=0.0;
	meter->overOn[0] =0.6;	meter->overOn[1] =0.0;	meter->overOn[2] =0.0;

}


static void
inv_meter_size_request(GtkWidget *widget,
    GtkRequisition *requisition)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(INV_IS_METER(widget));
	g_return_if_fail(requisition != NULL);

	requisition->width = 149;
	requisition->height = 24;
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
	attributes.width = 149;
	attributes.height = 24;

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
	cairo_t 	*cr;
	gint 		Lon,Ron,min,max,i;
	float 		R,G,B;
	GtkStyle	*style;

	style = gtk_widget_get_style(widget);
	gint channels = INV_METER(widget)->channels;
	gint Lpos = (gint)(INV_METER(widget)->LdB+60.25);
	gint Rpos = (gint)(INV_METER(widget)->RdB+60.25);

	gint lastLpos = INV_METER(widget)->lastLpos;
	gint lastRpos = INV_METER(widget)->lastRpos;

	cr = gdk_cairo_create(widget->window);

	switch(mode) {
		case INV_METER_DRAW_ALL:

			cairo_set_source_rgb(cr, 0, 0, 0);
			cairo_paint(cr);

			cairo_new_path(cr);

			cairo_set_line_join (cr, CAIRO_LINE_JOIN_MITER);
			cairo_set_antialias (cr,CAIRO_ANTIALIAS_NONE);
			cairo_set_line_width(cr,1);

			gdk_cairo_set_source_color(cr,&style->dark[GTK_STATE_NORMAL]);
			cairo_move_to(cr, 0, 23);
			cairo_line_to(cr, 0, 0);
			cairo_line_to(cr, 148, 0);
			cairo_stroke(cr);

			gdk_cairo_set_source_color(cr,&style->light[GTK_STATE_NORMAL]);
			cairo_move_to(cr, 0, 23);
			cairo_line_to(cr, 148, 23);
			cairo_line_to(cr, 148, 0);
			cairo_stroke(cr);

			cairo_set_antialias (cr,CAIRO_ANTIALIAS_DEFAULT);
			cairo_new_path(cr);

			cairo_set_source_rgb(cr, 1, 1, 1);
			cairo_select_font_face(cr,"monospace",CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_NORMAL);
			cairo_set_font_size(cr,8);

			switch(channels)
			{
				case 1:
					cairo_move_to(cr,3,14);
					cairo_show_text(cr,"M");
					break;
				case 2:
					cairo_move_to(cr,3,9);
					cairo_show_text(cr,"L");
					cairo_move_to(cr,3,19);
					cairo_show_text(cr,"R");
					break; 
			}

			for ( i = 1; i <= 67; i++) 
			{
				switch(channels)
				{
					case 1:
						Lon = i <= Lpos ? 1 : 0;

						inv_meter_colour(widget, i, Lon, &R, &G,&B);
						cairo_set_source_rgb(cr, R, G, B);
						cairo_rectangle(cr, 10+(i*2), 3, 1, 18);
						cairo_fill(cr);
						break;
					case 2:
						Lon = i <= Lpos ? 1 : 0;
						Ron = i <= Rpos ? 1 : 0;

						inv_meter_colour(widget, i, Lon, &R, &G,&B);
						cairo_set_source_rgb(cr, R, G, B);
						cairo_rectangle(cr, 10+(i*2), 3, 1, 8);
						cairo_fill(cr);

						inv_meter_colour(widget, i, Ron, &R, &G,&B);
						cairo_set_source_rgb(cr, R, G, B);
						cairo_rectangle(cr, 10+(i*2), 13, 1, 8);
						cairo_fill(cr);
						break; 
				}
			}
			INV_METER(widget)->lastLpos = Lpos;
			INV_METER(widget)->lastRpos = Rpos;
			break;

		case INV_METER_DRAW_L:
			min = lastLpos < Lpos ? lastLpos : Lpos;
			max = lastLpos > Lpos ? lastLpos : Lpos;
			if(min<1) min=1;
			if(max<1) max=1;
			if(min != max || max == 1 ) {
				for ( i = min ; i <= max; i++) 
				{
					Lon = i <= Lpos ? 1 : 0;
					inv_meter_colour(widget, i, Lon, &R, &G,&B);
					cairo_set_source_rgb(cr, R, G, B);
					switch(channels)
					{
						case 1:
							cairo_rectangle(cr, 10+(i*2), 3, 1, 18);
							break;
						case 2:
							cairo_rectangle(cr, 10+(i*2), 3, 1, 8);
							break; 
					}
					cairo_fill(cr);
				}
			}
			INV_METER(widget)->lastLpos = Lpos;
			break;

		case INV_METER_DRAW_R:
			min = lastRpos < Rpos ? lastRpos : Rpos;
			max = lastRpos > Rpos ? lastRpos : Rpos;
			if(min<1) min=1;
			if(max<1) max=1;
			if(min != max || max == 1 ) {
				for ( i = min ; i <= max; i++) 
				{
					Ron = i <= Rpos ? 1 : 0;
					inv_meter_colour(widget, i, Ron, &R, &G, &B);
					cairo_set_source_rgb(cr, R, G, B);
					cairo_rectangle(cr, 10+(i*2), 13, 1, 8);
					cairo_fill(cr);
				}
			}
			INV_METER(widget)->lastRpos = Rpos;
			break;

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

static void	
inv_meter_colour(GtkWidget *widget, gint pos, gint on, float *R, float *G, float *B)
{

/* 
66 =  +6dB
60 =   0dB
51 =  -9dB
42 = -18dB
*/

	float r1,r2;

	float mOff60R = INV_METER(widget)->mOff60[0];
	float mOff60G = INV_METER(widget)->mOff60[1];
	float mOff60B = INV_METER(widget)->mOff60[2];

	float mOn60R = INV_METER(widget)->mOn60[0];
	float mOn60G = INV_METER(widget)->mOn60[1];
	float mOn60B = INV_METER(widget)->mOn60[2];

	float mOff12R = INV_METER(widget)->mOff12[0];
	float mOff12G = INV_METER(widget)->mOff12[1];
	float mOff12B = INV_METER(widget)->mOff12[2];

	float mOn12R = INV_METER(widget)->mOn12[0];
	float mOn12G = INV_METER(widget)->mOn12[1];
	float mOn12B = INV_METER(widget)->mOn12[2];

	float mOff6R = INV_METER(widget)->mOff6[0];
	float mOff6G = INV_METER(widget)->mOff6[1];
	float mOff6B = INV_METER(widget)->mOff6[2];

	float mOn6R = INV_METER(widget)->mOn6[0];
	float mOn6G = INV_METER(widget)->mOn6[1];
	float mOn6B = INV_METER(widget)->mOn6[2];

	float mOff0R = INV_METER(widget)->mOff0[0];
	float mOff0G = INV_METER(widget)->mOff0[1];
	float mOff0B = INV_METER(widget)->mOff0[2];

	float mOn0R = INV_METER(widget)->mOn0[0];
	float mOn0G = INV_METER(widget)->mOn0[1];
	float mOn0B = INV_METER(widget)->mOn0[2];

	float overOffR = INV_METER(widget)->overOff[0];
	float overOffG = INV_METER(widget)->overOff[1];
	float overOffB = INV_METER(widget)->overOff[2];

	float overOnR = INV_METER(widget)->overOn[0];
	float overOnG = INV_METER(widget)->overOn[1];
	float overOnB = INV_METER(widget)->overOn[2];

	if(pos < 42) 
	{
		r1=(42.0-(float)pos)/42.0;
		r2=(float)pos/42.0;
		*R=(r1 * mOff60R + (r2 * mOff12R))  + (on * ((r1 * mOn60R) + (r2 * mOn12R))) ;
		*G=(r1 * mOff60G + (r2 * mOff12G))  + (on * ((r1 * mOn60G) + (r2 * mOn12G))) ;
		*B=(r1 * mOff60B + (r2 * mOff12B))  + (on * ((r1 * mOn60B) + (r2 * mOn12B))) ;
	} 

	else if (pos < 51)
	{
		r1=(51.0-(float)pos)/9.0;
		r2=((float)pos-42.0)/9.0;
		*R=(r1 * mOff12R + (r2 * mOff6R))  + (on * ((r1 * mOn12R) + (r2 * mOn6R))) ;
		*G=(r1 * mOff12G + (r2 * mOff6G))  + (on * ((r1 * mOn12G) + (r2 * mOn6G))) ;
		*B=(r1 * mOff12B + (r2 * mOff6B))  + (on * ((r1 * mOn12B) + (r2 * mOn6B))) ;
	}

	else if (pos < 60)
	{
		r1=(60.0-(float)pos)/9.0;
		r2=((float)pos-51.0)/9.0;
		*R=(r1 * mOff6R + (r2 * mOff0R))  + (on * ((r1 * mOn6R) + (r2 * mOn0R))) ;
		*G=(r1 * mOff6G + (r2 * mOff0G))  + (on * ((r1 * mOn6G) + (r2 * mOn0G))) ;
		*B=(r1 * mOff6B + (r2 * mOff0B))  + (on * ((r1 * mOn6B) + (r2 * mOn0B))) ;
	}
	else
	{
		*R=overOffR + (on * overOnR) ;
		*G=overOffG + (on * overOnG) ;
		*B=overOffB + (on * overOnB) ;
	}	
}
