
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "display-FrequencyGain.h"



static void 	inv_display_fg_class_init(InvDisplayFGClass *klass);
static void 	inv_display_fg_init(InvDisplayFG *displayFG);
static void 	inv_display_fg_size_request(GtkWidget *widget, GtkRequisition *requisition);
static void 	inv_display_fg_size_allocate(GtkWidget *widget, GtkAllocation *allocation);
static void 	inv_display_fg_realize(GtkWidget *widget);
static gboolean inv_display_fg_expose(GtkWidget *widget,GdkEventExpose *event);
static void 	inv_display_fg_paint(GtkWidget *widget, gint mode);
static void	inv_display_fg_destroy(GtkObject *object);
float 		get_point(float min, float max, float num, float range);


GtkType
inv_display_fg_get_type(void)
{
	static GtkType inv_display_fg_type = 0;
	char *name;
	int i;


	if (!inv_display_fg_type) 
	{
		static const GTypeInfo type_info = {
			sizeof(InvDisplayFGClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc)inv_display_fg_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof(InvDisplayFG),
			0,    /* n_preallocs */
			(GInstanceInitFunc)inv_display_fg_init
		};
		for (i = 0; ; i++) {
			name = g_strdup_printf("InvDisplayFG-%p-%d",inv_display_fg_class_init, i);
			if (g_type_from_name(name)) {
				free(name);
				continue;
			}
			inv_display_fg_type = g_type_register_static(GTK_TYPE_WIDGET,name,&type_info,(GTypeFlags)0);
			free(name);
			break;
		}
	}
	return inv_display_fg_type;
}

void
inv_display_fg_set_mode(InvDisplayFG *displayFG, gint num)
{
	displayFG->mode = num;
}

void
inv_display_fg_set_freq(InvDisplayFG *displayFG, float num)
{
	if(num<20)
		displayFG->freq = 20;
	else if (num <= 20000)
		displayFG->freq = num;
	else displayFG->freq = 20000;
	if(GTK_WIDGET_REALIZED(displayFG))
		inv_display_fg_paint(GTK_WIDGET(displayFG),INV_DISPLAYFG_DRAW_DATA);
}

void
inv_display_fg_set_gain(InvDisplayFG *displayFG, float num)
{
	if(num<0)
		displayFG->gain = 0;
	else if (num <= 12)
		displayFG->gain = num;
	else displayFG->gain = 12;
	if(GTK_WIDGET_REALIZED(displayFG))
		inv_display_fg_paint(GTK_WIDGET(displayFG),INV_DISPLAYFG_DRAW_DATA);
}


GtkWidget * inv_display_fg_new()
{
	return GTK_WIDGET(gtk_type_new(inv_display_fg_get_type()));
}


static void
inv_display_fg_class_init(InvDisplayFGClass *klass)
{
	GtkWidgetClass *widget_class;
	GtkObjectClass *object_class;


	widget_class = (GtkWidgetClass *) klass;
	object_class = (GtkObjectClass *) klass;

	widget_class->realize = inv_display_fg_realize;
	widget_class->size_request = inv_display_fg_size_request;
	widget_class->size_allocate = inv_display_fg_size_allocate;
	widget_class->expose_event = inv_display_fg_expose;

	object_class->destroy = inv_display_fg_destroy;
}


static void
inv_display_fg_init(InvDisplayFG *displayFG)
{
	displayFG->mode = INV_DISPLAYFG_MODE_LPF;
	displayFG->freq = 1000.0;
	displayFG->gain = 0.0;
}


static void
inv_display_fg_size_request(GtkWidget *widget,
    GtkRequisition *requisition)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(INV_IS_DISPLAY_FG(widget));
	g_return_if_fail(requisition != NULL);

	requisition->width = 408;
	requisition->height = 108;
}


static void
inv_display_fg_size_allocate(GtkWidget *widget,
    GtkAllocation *allocation)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(INV_IS_DISPLAY_FG(widget));
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
inv_display_fg_realize(GtkWidget *widget)
{
	GdkWindowAttr attributes;
	guint attributes_mask;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(INV_IS_DISPLAY_FG(widget));

	GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
	attributes.width = 408;
	attributes.height = 108;

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
inv_display_fg_expose(GtkWidget *widget, GdkEventExpose *event)
{
	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(INV_IS_DISPLAY_FG(widget), FALSE);
	g_return_val_if_fail(event != NULL, FALSE);

	inv_display_fg_paint(widget,INV_DISPLAYFG_DRAW_ALL);

	return FALSE;
}


static void
inv_display_fg_paint(GtkWidget *widget, gint mode)
{
	cairo_t *cr;
	gint i,j,k;
	float p,freq,gain;
	float x,y;
	gint type;
	char string[10];
	GtkStyle	*style;

	style = gtk_widget_get_style(widget);
	type=INV_DISPLAY_FG(widget)->mode;
	freq=INV_DISPLAY_FG(widget)->freq;
	gain=INV_DISPLAY_FG(widget)->gain;

	cr = gdk_cairo_create(widget->window);

	if(mode==INV_DISPLAYFG_DRAW_ALL) {

		cairo_set_line_join (cr, CAIRO_LINE_JOIN_MITER);
		cairo_set_antialias (cr,CAIRO_ANTIALIAS_NONE);
		cairo_set_line_width(cr,1);

		gdk_cairo_set_source_color(cr,&style->dark[GTK_STATE_NORMAL]);
		cairo_move_to(cr, 0, 107);
		cairo_line_to(cr, 0, 0);
		cairo_line_to(cr, 407, 0);
		cairo_stroke(cr);

		gdk_cairo_set_source_color(cr,&style->light[GTK_STATE_NORMAL]);
		cairo_move_to(cr, 0, 107);
		cairo_line_to(cr, 407, 107);
		cairo_line_to(cr, 407, 0);
		cairo_stroke(cr);

		cairo_set_antialias (cr,CAIRO_ANTIALIAS_DEFAULT);
		cairo_new_path(cr);

		cairo_set_source_rgb(cr, 0.05, 0.05, 0.2);
		cairo_rectangle(cr, 1, 1, 406, 106 );
		cairo_fill(cr);

		/* horizontal axis */
		cairo_set_source_rgb(cr, 1, 1, 1);
		cairo_rectangle(cr, 4, 94, 374, 1);
		cairo_fill(cr);

		cairo_select_font_face(cr,"monospace",CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size(cr,8);
		cairo_set_source_rgb(cr, 0.7, 0.7, 0.7);

		for(i=1; i<5;i++) 
		{
			for(j=1;j<6;j++) 
			{
				if( j==1 || j==2 || j==5)
				{
					p=(float)j*pow(10,(float)i);
					if(p>=20 && p <= 20000) 
					{
						switch(i)
						{
							case 1:
								sprintf(string,"%i0Hz",j);
								break;
							case 2:
								sprintf(string,"%i00Hz",j);
								break;
							case 3:
								sprintf(string,"%ikHz",j);
								break;
							case 4:
								sprintf(string,"%i0kHz",j);
								break;
						}
						k= (gint) get_point(20.0, 20000.0, p, 358);
						cairo_move_to(cr,6+k,104);
						cairo_show_text(cr,string);
					}
				}
			}
		}

		/* vertical axis */
		cairo_set_source_rgb(cr, 1, 1, 1);
		cairo_rectangle(cr, 377, 4, 1, 91);
		cairo_fill(cr);

		cairo_select_font_face(cr,"monospace",CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size(cr,8);
		cairo_set_source_rgb(cr, 0.7, 0.7, 0.7);
		for(i=0;i<11;i+=2)
		{
			j=12-(i*6);
			sprintf(string,"%3idB",j);
			cairo_move_to(cr,379,11+(i*8));
			cairo_show_text(cr,string);
		}

	}

	/*graph area */
	cairo_set_source_rgb(cr, 0.05, 0.05, 0.2);
	cairo_rectangle(cr, 4, 4, 373, 90 );
	cairo_fill(cr);

	/* horizontal axis except for labeled lines */
	cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
	for(i=1; i<5;i++) 
	{
		for(j=1; j<10; j++)
		{
			if( !(j==1 || j==2 || j==5))
			{
				p=(float)j*pow(10,(float)i);
				if(p>=20 && p <= 20000) 
				{
					k= (gint) get_point(20.0, 20000.0, p, 358);
					cairo_rectangle(cr, 10+k, 4, 1, 90);
					cairo_fill(cr);
				}
			}
		}
	}

	/* vertical axis */
	for(i=0;i<11;i++)
	{
		switch(i) 
		{
			case 0:
			case 2:
			case 4:
			case 6:
			case 8:
			case 10:
				cairo_set_source_rgb(cr, 0.35, 0.35, 0.35);
				break;
			case 1: 
			case 3: 
			case 5: 
			case 7: 
			case 9: 
				cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
				break;
		}	
		cairo_rectangle(cr, 4, 8+(i*8), 373, 1);
		cairo_fill(cr);
	}

	/* horizontal axis labeled lines */
	cairo_set_source_rgb(cr, 0.35, 0.35, 0.35);
	for(i=1; i<5;i++) 
	{
		for(j=1; j<6; j++)
		{
			if( j==1 || j==2 || j==5)
			{
				p=(float)j*pow(10,(float)i);
				if(p>=20 && p <= 20000) 

				{
					k= (gint) get_point(20.0, 20000.0, p, 358);
					cairo_rectangle(cr, 10+k, 4, 1, 90);
					cairo_fill(cr);
				}
			}
		}
	}

	/* filter */



	cairo_rectangle(cr, 4, 4, 373, 90 );
	cairo_clip(cr);

	cairo_set_line_width(cr,3);
	cairo_set_source_rgb(cr, 0.0, 0.1, 1);

	switch(type)
	{
		case INV_DISPLAYFG_MODE_LPF:
			x=get_point(20.0, 20000.0, freq/10, 358);
			y=(12-gain)/6;
			if(10+x > 4) 
			{
				cairo_move_to(cr, 4, 9+(y*8));
				cairo_line_to(cr, 10+x, 9+(y*8));
			} else {
				cairo_move_to(cr, 10+x, 9+(y*8));
			}
			gain=gain -2;
			x=get_point(20.0, 20000.0, freq*0.3, 358);
			y=(12-gain)/6;
			cairo_line_to(cr, 10+x, 9+(y*8));

			gain=gain -2;
			x=get_point(20.0, 20000.0, freq*0.7, 358);
			y=(12-gain)/6;
			cairo_line_to(cr, 10+x, 9+(y*8));

			gain=gain -2;
			x=get_point(20.0, 20000.0, freq, 358);
			y=(12-gain)/6;
			cairo_line_to(cr, 10+x, 9+(y*8));

			i=0;
			while(freq < 30000)
			{
				freq=freq*1.58;
				gain=gain-3-i;
				x=get_point(20.0, 20000.0, freq, 358);
				y=(12-gain)/6;
				cairo_line_to(cr, 10+x, 9+(y*8));
				i++;
			}
			break;
		case INV_DISPLAYFG_MODE_HPF:

			x=get_point(20.0, 20000.0, freq*10, 358);
			y=(12-gain)/6;
			if(10+x < 376) 
			{
				cairo_move_to(cr, 376, 9+(y*8));
				cairo_line_to(cr, 10+x, 9+(y*8));
			} else {
				cairo_move_to(cr, 10+x, 9+(y*8));
			}
			gain=gain -2;
			x=get_point(20.0, 20000.0, freq/0.3, 358);
			y=(12-gain)/6;
			cairo_line_to(cr, 10+x, 9+(y*8));

			gain=gain -2;
			x=get_point(20.0, 20000.0, freq/0.7, 358);
			y=(12-gain)/6;
			cairo_line_to(cr, 10+x, 9+(y*8));

			gain=gain -2;
			x=get_point(20.0, 20000.0, freq, 358);
			y=(12-gain)/6;
			cairo_line_to(cr, 10+x, 9+(y*8));

			i=0;
			while(freq > 14)
			{
				freq=freq/1.58;
				gain=gain-3-i;
				x=get_point(20.0, 20000.0, freq, 358);
				y=(12-gain)/6;
				cairo_line_to(cr, 10+x, 9+(y*8));
				i++;
			}
			break;
	}

	cairo_stroke(cr);
  	cairo_destroy(cr);
}


static void
inv_display_fg_destroy(GtkObject *object)
{
	InvDisplayFG *displayFG;
	InvDisplayFGClass *klass;

	g_return_if_fail(object != NULL);
	g_return_if_fail(INV_IS_DISPLAY_FG(object));

	displayFG = INV_DISPLAY_FG(object);

	klass = gtk_type_class(gtk_widget_get_type());

	if (GTK_OBJECT_CLASS(klass)->destroy) {
		(* GTK_OBJECT_CLASS(klass)->destroy) (object);
	}
}

float get_point(float min, float max, float num, float range)
{
  return range * (log(num) - log(min)) / (log(max) - log(min));
}

