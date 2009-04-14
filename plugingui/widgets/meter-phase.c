#include "stdlib.h"
#include "string.h"
#include "meter-phase.h"


static void 	inv_phase_meter_class_init(InvPhaseMeterClass *klass);
static void 	inv_phase_meter_init(InvPhaseMeter *meter);
static void 	inv_phase_meter_size_request(GtkWidget *widget, GtkRequisition *requisition);
static void 	inv_phase_meter_size_allocate(GtkWidget *widget, GtkAllocation *allocation);
static void 	inv_phase_meter_realize(GtkWidget *widget);
static gboolean inv_phase_meter_expose(GtkWidget *widget,GdkEventExpose *event);
static void 	inv_phase_meter_paint(GtkWidget *widget, gint mode);
static void	inv_phase_meter_destroy(GtkObject *object);
static void	inv_phase_meter_colour(GtkWidget *widget, gint pos, gint on, float *R, float *G, float *B);


GtkType
inv_phase_meter_get_type(void)
{
	static GType inv_phase_meter_type = 0;
	char *name;
	int i;


	if (!inv_phase_meter_type) 
	{
		static const GTypeInfo type_info = {
			sizeof(InvPhaseMeterClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc)inv_phase_meter_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof(InvPhaseMeter),
			0,    /* n_preallocs */
			(GInstanceInitFunc)inv_phase_meter_init
		};
		for (i = 0; ; i++) {
			name = g_strdup_printf("InvPhaseMeter-%p-%d",inv_phase_meter_class_init, i);
			if (g_type_from_name(name)) {
				free(name);
				continue;
			}
			inv_phase_meter_type = g_type_register_static(GTK_TYPE_WIDGET,name,&type_info,(GTypeFlags)0);
			free(name);
			break;
		}
	}
	return inv_phase_meter_type;
}

void
inv_phase_meter_set_phase(InvPhaseMeter *meter, float num)
{
	meter->phase = num;
	if(GTK_WIDGET_REALIZED(meter))
		inv_phase_meter_paint(GTK_WIDGET(meter),INV_PHASE_METER_DRAW_DATA);
}

GtkWidget * inv_phase_meter_new()
{
	return GTK_WIDGET(gtk_type_new(inv_phase_meter_get_type()));
}


static void
inv_phase_meter_class_init(InvPhaseMeterClass *klass)
{
	GtkWidgetClass *widget_class;
	GtkObjectClass *object_class;


	widget_class = (GtkWidgetClass *) klass;
	object_class = (GtkObjectClass *) klass;

	widget_class->realize = inv_phase_meter_realize;
	widget_class->size_request = inv_phase_meter_size_request;
	widget_class->size_allocate = inv_phase_meter_size_allocate;
	widget_class->expose_event = inv_phase_meter_expose;

	object_class->destroy = inv_phase_meter_destroy;
}


static void
inv_phase_meter_init(InvPhaseMeter *meter)
{
	meter->phase = 0;

	meter->mOff0[0] =0.1;	meter->mOff0[1] =0.1;	meter->mOff0[2] =0.4;
	meter->mOn0[0]  =-0.1;	meter->mOn0[1]  =-0.1;	meter->mOn0[2]  =0.6;

	meter->mOff30[0] =0.2; 	meter->mOff30[1] =0.3;	meter->mOff30[2] =0.4;
	meter->mOn30[0]  =-0.1;	meter->mOn30[1]  =0.3;	meter->mOn30[2]  =0.6;

	meter->mOff45[0] =0.2; 	meter->mOff45[1] =0.4;	meter->mOff45[2] =0.2;
	meter->mOn45[0]  =0.1;	meter->mOn45[1]  =0.6;	meter->mOn45[2]  =-0.1;

	meter->mOff60[0]  =0.5;	meter->mOff60[1]  =0.5;	meter->mOff60[2]  =0.0;
	meter->mOn60[0]   =0.5;	meter->mOn60[1]   =0.5;	meter->mOn60[2]   =0.0;

	meter->mOff90[0]=0.4;	meter->mOff90[1]=0.2;	meter->mOff90[2]=0.0;
	meter->mOn90[0] =0.6;	meter->mOn90[1] =0.0;	meter->mOn90[2] =0.0;

}


static void
inv_phase_meter_size_request(GtkWidget *widget,
    GtkRequisition *requisition)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(INV_IS_PHASE_METER(widget));
	g_return_if_fail(requisition != NULL);

	requisition->width = 268;
	requisition->height = 36;
}


static void
inv_phase_meter_size_allocate(GtkWidget *widget,
    GtkAllocation *allocation)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(INV_IS_PHASE_METER(widget));
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
inv_phase_meter_realize(GtkWidget *widget)
{
	GdkWindowAttr attributes;
	guint attributes_mask;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(INV_IS_PHASE_METER(widget));

	GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
	attributes.width = 268;
	attributes.height = 36;

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
inv_phase_meter_expose(GtkWidget *widget, GdkEventExpose *event)
{
	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(INV_IS_PHASE_METER(widget), FALSE);
	g_return_val_if_fail(event != NULL, FALSE);

	inv_phase_meter_paint(widget,INV_PHASE_METER_DRAW_ALL);

	return FALSE;
}


static void
inv_phase_meter_paint(GtkWidget *widget, gint mode)
{
	cairo_t 	*cr;
	float 		Pon;
	float 		R,G,B;
	GtkStyle	*style;

	char label[10];
	cairo_text_extents_t extents;

	style = gtk_widget_get_style(widget);
	gint phase = (gint)((INV_PHASE_METER(widget)->phase*38.197186337)+0.2);

	cr = gdk_cairo_create(widget->window);

	if(mode==INV_PHASE_METER_DRAW_ALL) {
		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_paint(cr);

		cairo_new_path(cr);

		cairo_set_line_join (cr, CAIRO_LINE_JOIN_MITER);
		cairo_set_antialias (cr,CAIRO_ANTIALIAS_NONE);
		cairo_set_line_width(cr,1);

		gdk_cairo_set_source_color(cr,&style->dark[GTK_STATE_NORMAL]);
		cairo_move_to(cr, 0, 35);
		cairo_line_to(cr, 0, 0);
		cairo_line_to(cr, 267, 0);
		cairo_stroke(cr);

		gdk_cairo_set_source_color(cr,&style->light[GTK_STATE_NORMAL]);
		cairo_move_to(cr, 0, 35);
		cairo_line_to(cr, 267, 35);
		cairo_line_to(cr, 267, 0);
		cairo_stroke(cr);

		cairo_set_antialias (cr,CAIRO_ANTIALIAS_DEFAULT);
		cairo_new_path(cr);

		cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);

		cairo_rectangle(cr, 14, 21, 1, 2);
		cairo_fill(cr);

		cairo_rectangle(cr, 74, 21, 1, 2);
		cairo_fill(cr);

		cairo_rectangle(cr, 134, 21, 1, 2);
		cairo_fill(cr);

		cairo_rectangle(cr, 194, 21, 1, 2);
		cairo_fill(cr);

		cairo_rectangle(cr, 254, 21, 1, 2);
		cairo_fill(cr);


		cairo_set_source_rgb(cr, 1, 1, 1);
		cairo_select_font_face(cr,"monospace",CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size(cr,8);

		strcpy(label,"-90");
		cairo_text_extents (cr,label,&extents);
		cairo_move_to(cr,14-(2*extents.width/3),31);
		cairo_show_text(cr,label);

		strcpy(label,"-45");
		cairo_text_extents (cr,label,&extents);
		cairo_move_to(cr,74-(2*extents.width/3),31);
		cairo_show_text(cr,label);

		strcpy(label,"0");
		cairo_text_extents (cr,label,&extents);
		cairo_move_to(cr,134-(extents.width/2),31);
		cairo_show_text(cr,label);

		strcpy(label,"45");
		cairo_text_extents (cr,label,&extents);
		cairo_move_to(cr,194-(extents.width/2),31);
		cairo_show_text(cr,label);

		strcpy(label,"90");
		cairo_text_extents (cr,label,&extents);
		cairo_move_to(cr,254-(extents.width/2),31);
		cairo_show_text(cr,label);


	}

	gint i;

	inv_phase_meter_colour(widget, 0, 1, &R, &G,&B);
	cairo_set_source_rgb(cr, R, G, B);
	cairo_rectangle(cr, 134, 5, 1, 14);

	for ( i = 1; i <= 60; i++) 
	{
		Pon = i <= phase ? 1 : 0;

		inv_phase_meter_colour(widget, i, Pon, &R, &G,&B);
		cairo_set_source_rgb(cr, R, G, B);
		cairo_rectangle(cr, 134+(i*2), 5, 1, 14);
		cairo_fill(cr);
		cairo_rectangle(cr, 134-(i*2), 5, 1, 14);
		cairo_fill(cr);
	}
  	cairo_destroy(cr);
}


static void
inv_phase_meter_destroy(GtkObject *object)
{
	InvPhaseMeter *meter;
	InvPhaseMeterClass *klass;

	g_return_if_fail(object != NULL);
	g_return_if_fail(INV_IS_PHASE_METER(object));

	meter = INV_PHASE_METER(object);

	klass = gtk_type_class(gtk_widget_get_type());

	if (GTK_OBJECT_CLASS(klass)->destroy) {
		(* GTK_OBJECT_CLASS(klass)->destroy) (object);
	}
}

static void	
inv_phase_meter_colour(GtkWidget *widget, gint pos, gint on, float *R, float *G, float *B)
{

/* 
20 =  30
30 =  45
40 =  60
60 =  90
*/

	float r1,r2;

	float mOff0R = INV_PHASE_METER(widget)->mOff0[0];
	float mOff0G = INV_PHASE_METER(widget)->mOff0[1];
	float mOff0B = INV_PHASE_METER(widget)->mOff0[2];

	float mOn0R = INV_PHASE_METER(widget)->mOn0[0];
	float mOn0G = INV_PHASE_METER(widget)->mOn0[1];
	float mOn0B = INV_PHASE_METER(widget)->mOn0[2];

	float mOff30R = INV_PHASE_METER(widget)->mOff30[0];
	float mOff30G = INV_PHASE_METER(widget)->mOff30[1];
	float mOff30B = INV_PHASE_METER(widget)->mOff30[2];

	float mOn30R = INV_PHASE_METER(widget)->mOn30[0];
	float mOn30G = INV_PHASE_METER(widget)->mOn30[1];
	float mOn30B = INV_PHASE_METER(widget)->mOn30[2];

	float mOff45R = INV_PHASE_METER(widget)->mOff45[0];
	float mOff45G = INV_PHASE_METER(widget)->mOff45[1];
	float mOff45B = INV_PHASE_METER(widget)->mOff45[2];

	float mOn45R = INV_PHASE_METER(widget)->mOn45[0];
	float mOn45G = INV_PHASE_METER(widget)->mOn45[1];
	float mOn45B = INV_PHASE_METER(widget)->mOn45[2];

	float mOff60R = INV_PHASE_METER(widget)->mOff60[0];
	float mOff60G = INV_PHASE_METER(widget)->mOff60[1];
	float mOff60B = INV_PHASE_METER(widget)->mOff60[2];

	float mOn60R = INV_PHASE_METER(widget)->mOn60[0];
	float mOn60G = INV_PHASE_METER(widget)->mOn60[1];
	float mOn60B = INV_PHASE_METER(widget)->mOn60[2];

	float mOff90R = INV_PHASE_METER(widget)->mOff90[0];
	float mOff90G = INV_PHASE_METER(widget)->mOff90[1];
	float mOff90B = INV_PHASE_METER(widget)->mOff90[2];

	float mOn90R = INV_PHASE_METER(widget)->mOn90[0];
	float mOn90G = INV_PHASE_METER(widget)->mOn90[1];
	float mOn90B = INV_PHASE_METER(widget)->mOn90[2];

	if(pos < 20) 
	{
		r1=(20.0-(float)pos)/20.0;
		r2=(float)pos/20.0;
		*R=(r1 * mOff0R + (r2 * mOff30R))  + (on * ((r1 * mOn0R) + (r2 * mOn30R))) ;
		*G=(r1 * mOff0G + (r2 * mOff30G))  + (on * ((r1 * mOn0G) + (r2 * mOn30G))) ;
		*B=(r1 * mOff0B + (r2 * mOff30B))  + (on * ((r1 * mOn0B) + (r2 * mOn30B))) ;
	} 

	else if (pos < 30)
	{
		r1=(30.0-(float)pos)/10.0;
		r2=((float)pos-20.0)/10.0;
		*R=(r1 * mOff30R + (r2 * mOff45R))  + (on * ((r1 * mOn30R) + (r2 * mOn45R))) ;
		*G=(r1 * mOff30G + (r2 * mOff45G))  + (on * ((r1 * mOn30G) + (r2 * mOn45G))) ;
		*B=(r1 * mOff30B + (r2 * mOff45B))  + (on * ((r1 * mOn30B) + (r2 * mOn45B))) ;
	}

	else if (pos < 40)
	{
		r1=(40.0-(float)pos)/10.0;
		r2=((float)pos-30.0)/10.0;
		*R=(r1 * mOff45R + (r2 * mOff60R))  + (on * ((r1 * mOn45R) + (r2 * mOn60R))) ;
		*G=(r1 * mOff45G + (r2 * mOff60G))  + (on * ((r1 * mOn45G) + (r2 * mOn60G))) ;
		*B=(r1 * mOff45B + (r2 * mOff60B))  + (on * ((r1 * mOn45B) + (r2 * mOn60B))) ;
	}
	else
	{
		r1=(60.0-(float)pos)/20.0;
		r2=((float)pos-40.0)/20.0;
		*R=(r1 * mOff60R + (r2 * mOff90R))  + (on * ((r1 * mOn60R) + (r2 * mOn90R))) ;
		*G=(r1 * mOff60G + (r2 * mOff90G))  + (on * ((r1 * mOn60G) + (r2 * mOn90G))) ;
		*B=(r1 * mOff60B + (r2 * mOff90B))  + (on * ((r1 * mOn60B) + (r2 * mOn90B))) ;

	}	
}

