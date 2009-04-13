#include "math.h"
#include "string.h"
#include "knob.h"


static void 	inv_knob_class_init(InvKnobClass *klass);
static void 	inv_knob_init(InvKnob *knob);
static void 	inv_knob_size_request(GtkWidget *widget, GtkRequisition *requisition);
static void 	inv_knob_size_allocate(GtkWidget *widget, GtkAllocation *allocation);
static void 	inv_knob_realize(GtkWidget *widget);
static gboolean inv_knob_expose(GtkWidget *widget,GdkEventExpose *event);
static void 	inv_knob_paint(GtkWidget *widget, gint mode);
static void	inv_knob_destroy(GtkObject *object);
static gboolean	inv_knob_button_press_event (GtkWidget *widget, GdkEventButton *event);
static gboolean	inv_knob_motion_notify_event(GtkWidget *widget, GdkEventMotion *event);
static gboolean inv_knob_button_release_event (GtkWidget *widget, GdkEventButton *event);

static void	inv_knob_label(gint mode, char *label, char *units, gint human, float value);
static float    inv_marking_to_value(float mark, gint curve, float min, float max);
static float	inv_value_to_angle(float value, gint curve, float min, float max);
static gint	inv_choose_light_dark(GdkColor *bg,GdkColor *light,GdkColor *dark);
static float	inv_value_from_motion(float x_delta, float y_delta, float current, gint curve, float min, float max);


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
inv_knob_set_human(InvKnob *knob)
{
	knob->human=1;
}


void
inv_knob_set_units(InvKnob *knob, char *units)
{
	strncpy(knob->units, units, 4);
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
	if(num < knob->min) 
		knob->value = knob->min;
	else if(num > knob->max)
		knob->value = knob->min;
	else
		knob->value = num;

	if(GTK_WIDGET_REALIZED(knob))
		inv_knob_paint(GTK_WIDGET(knob),INV_KNOB_DRAW_DATA);
}

float
inv_knob_get_value(InvKnob *knob)
{
	return knob->value;
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

    	widget_class->button_press_event = inv_knob_button_press_event;
    	widget_class->motion_notify_event = inv_knob_motion_notify_event;
    	widget_class->button_release_event = inv_knob_button_release_event;

	object_class->destroy = inv_knob_destroy;
}


static void
inv_knob_init(InvKnob *knob)
{
	knob->size      = INV_KNOB_SIZE_MEDIUM;
	knob->curve     = INV_KNOB_CURVE_LINEAR;
	knob->markings  = INV_KNOB_MARKINGS_5;
	knob->highlight = INV_KNOB_HIGHLIGHT_L;
	strcpy(knob->units,"");
	knob->human 	= 0;
	knob->min       = 0.0;
	knob->max       = 1.0;
	knob->value     = 0.5;
	knob->click_x	=0;
	knob->click_y	=0;

    	GTK_WIDGET_SET_FLAGS (GTK_WIDGET(knob), GTK_CAN_FOCUS);
}

static void
inv_knob_size_request(GtkWidget *widget,
    GtkRequisition *requisition)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(INV_IS_KNOB(widget));
	g_return_if_fail(requisition != NULL);

	requisition->width  = INV_KNOB (widget)->size+4;
	switch(INV_KNOB (widget)->size) {
		case INV_KNOB_SIZE_SMALL:
			requisition->height = INV_KNOB (widget)->size+50;
			break;
		case INV_KNOB_SIZE_MEDIUM:
			requisition->height = INV_KNOB (widget)->size+56;
			break;
		case INV_KNOB_SIZE_LARGE:
		default:
			requisition->height = INV_KNOB (widget)->size+62;
			break;
	}
	
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
	attributes.width = INV_KNOB (widget)->size + 4;
	switch(INV_KNOB (widget)->size) {
		case INV_KNOB_SIZE_SMALL:
			attributes.height = INV_KNOB (widget)->size + 50;
			break;
		case INV_KNOB_SIZE_MEDIUM:
			attributes.height = INV_KNOB (widget)->size + 56;
			break;
		case INV_KNOB_SIZE_LARGE:
		default:
			attributes.height = INV_KNOB (widget)->size + 62;
			break;
	}
	

	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.event_mask = gtk_widget_get_events(widget) |
				GDK_EXPOSURE_MASK | 
				GDK_BUTTON_PRESS_MASK | 
				GDK_BUTTON_RELEASE_MASK | 
				GDK_BUTTON_MOTION_MASK;

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
	cairo_t		*cr;
	gint  		size;
	gint  		curve;
	gint  		markings;
	gint  		highlight;
	gint  		human;
	char		*units;
	float		min;
	float		max;
	float 		value;
	GtkStateType	state;
	GtkStyle	*style;

	gint i;
	float xc,yc,r,ll,ls,tb,angle;
	gint fontsize;
	char label[20];
	cairo_text_extents_t extents;

	cr = gdk_cairo_create(widget->window);

	state = GTK_WIDGET_STATE(widget);
	style = gtk_widget_get_style(widget);

	size = INV_KNOB(widget)->size;
	curve = INV_KNOB(widget)->curve;
	markings = INV_KNOB(widget)->markings;
	highlight = INV_KNOB(widget)->highlight;
	human = INV_KNOB(widget)->human;
	units = INV_KNOB(widget)->units;
	min = INV_KNOB(widget)->min;
	max = INV_KNOB(widget)->max;
	value = INV_KNOB(widget)->value;


	xc=(size/2)+2;
	r=size/2;
	switch(size) {
		case INV_KNOB_SIZE_SMALL:
			yc=(size/2)+19;
			fontsize=8;
			ls=3;
			ll=7;
			tb=11;
			break;
		case INV_KNOB_SIZE_MEDIUM:
			yc=(size/2)+22;
			fontsize=9;
			ls=5;
			ll=9;
			tb=12;
			break;
		case INV_KNOB_SIZE_LARGE:
		default:
			yc=(size/2)+25;
			fontsize=10;
			ls=7;
			ll=11;
			tb=13;
			break;
	}

	/* sanity check - ardour 2.7 doesn't initialise control values properly */

	if(value < min)
		value=min;
	else if (value > max)
		value=max;
		
/*
	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_paint(cr);
*/

	if(mode==INV_KNOB_DRAW_ALL) {

		gdk_cairo_set_source_color(cr,&style->bg[GTK_STATE_NORMAL]);
		cairo_paint(cr);
		cairo_new_path(cr);

		cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);

		if(inv_choose_light_dark(&style->bg[GTK_STATE_NORMAL],&style->light[GTK_STATE_NORMAL],&style->dark[GTK_STATE_NORMAL])==1) {
			gdk_cairo_set_source_color(cr,&style->light[GTK_STATE_NORMAL]);
		} else {
			gdk_cairo_set_source_color(cr,&style->dark[GTK_STATE_NORMAL]);
		}

		if(markings==INV_KNOB_MARKINGS_3 || markings==INV_KNOB_MARKINGS_4 || markings==INV_KNOB_MARKINGS_5) {
			for(i=0;i<=12;i++)
			{ 
				cairo_move_to(cr,xc+(r-6)*sin((INV_PI/3)+(i*INV_PI/9)),yc+(r-6)*cos((INV_PI/3)+(i*INV_PI/9)));
				if(i==0 || i==12) {
					/* bottom L & R */
					cairo_line_to(cr,xc+r*sin((INV_PI/3)+(i*INV_PI/9)),yc+r*cos((INV_PI/3)+(i*INV_PI/9)));
					cairo_line_to(cr,xc+r*sin((INV_PI/3)+(i*INV_PI/9)),yc+r-2);
					cairo_set_line_width(cr,2); 
					cairo_stroke(cr);
				} else if ((i==3 || i==9) && markings==INV_KNOB_MARKINGS_5) {
					/*top L & R for M5 */
					cairo_line_to(cr,xc+r*sin((INV_PI/3)+(i*INV_PI/9)),yc+r*cos((INV_PI/3)+(i*INV_PI/9)));
					cairo_line_to(cr,xc+r*sin((INV_PI/3)+(i*INV_PI/9)),yc-r+2);
					cairo_set_line_width(cr,2); 
					cairo_stroke(cr);
				} else if ((i==4 || i==8) && markings==INV_KNOB_MARKINGS_4) {
					/*top L & R for M4 */
					cairo_line_to(cr,xc+(r+ls)*sin((INV_PI/3)+(i*INV_PI/9)),yc+(r+ls)*cos((INV_PI/3)+(i*INV_PI/9)));
					cairo_line_to(cr,xc+(r+ls)*sin((INV_PI/3)+(i*INV_PI/9)),yc+(r+ls)*cos((INV_PI/3)+(i*INV_PI/9))-(ls+1));
					cairo_set_line_width(cr,2);
					cairo_stroke(cr); 
				} else if (i==6 && markings==INV_KNOB_MARKINGS_5) {
					/*top M5 */
					cairo_line_to(cr,xc+(r+ll)*sin((INV_PI/3)+(i*INV_PI/9)),yc+(r+ll)*cos((INV_PI/3)+(i*INV_PI/9)));
					cairo_set_line_width(cr,2); 
					cairo_stroke(cr);
				} else if (i==6 && markings==INV_KNOB_MARKINGS_3){
					/*top M3 */
					cairo_line_to(cr,xc+(r+ls)*sin((INV_PI/3)+(i*INV_PI/9)),yc+(r+ls)*cos((INV_PI/3)+(i*INV_PI/9)));
					cairo_set_line_width(cr,2);
					cairo_stroke(cr); 
				} else {
					/* all others */
					cairo_line_to(cr,xc+(r-2)*sin((INV_PI/3)+(i*INV_PI/9)),yc+(r-2)*cos((INV_PI/3)+(i*INV_PI/9)));
					cairo_set_line_width(cr,1.3); 
					cairo_stroke(cr);
				} 
			}
		} else { /* pan, cust & 10 */
			for(i=0;i<=10;i++)
			{
				cairo_move_to(cr,xc+(r-6)*sin((INV_PI/3)+(4*i*INV_PI/30)),yc+(r-6)*cos((INV_PI/3)+(4*i*INV_PI/30)));
				if(i==0 || i==10) {
					/* bottom L & R */
					cairo_line_to(cr,xc+r*sin((INV_PI/3)+(4*i*INV_PI/30)),yc+r*cos((INV_PI/3)+(4*i*INV_PI/30)));
					cairo_line_to(cr,xc+r*sin((INV_PI/3)+(4*i*INV_PI/30)),yc+r-2);
					cairo_set_line_width(cr,2); 
					cairo_stroke(cr);
				} else if (i==5){
					/*top */
					cairo_line_to(cr,xc+(r+ls)*sin((INV_PI/3)+(4*i*INV_PI/30)),yc+(r+ls)*cos((INV_PI/3)+(4*i*INV_PI/30)));
					cairo_set_line_width(cr,2); 
					cairo_stroke(cr);
				} else {
					/* all others */
					cairo_line_to(cr,xc+(r-2)*sin((INV_PI/3)+(4*i*INV_PI/30)),yc+(r-2)*cos((INV_PI/3)+(4*i*INV_PI/30)));
					cairo_set_line_width(cr,1.3); 
					cairo_stroke(cr);
				}
			}
		}

		cairo_select_font_face(cr,"monospace",CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size(cr,fontsize);
		gdk_cairo_set_source_color(cr,&style->fg[state]);

		/* bottom left */
		if(markings==INV_KNOB_MARKINGS_PAN) {
			switch(size)
			{
				case INV_KNOB_SIZE_MEDIUM:
				case INV_KNOB_SIZE_LARGE:
					strcpy(label,"Left");
					break;
				case INV_KNOB_SIZE_SMALL:
				default:
					strcpy(label,"L");
					break;
			}
		} else {
			inv_knob_label(0,label, units,human, min);
		}
		cairo_move_to(cr,1,yc+r+8);
		cairo_show_text(cr,label);

		/* bottom right */
		if(markings==INV_KNOB_MARKINGS_PAN) {
			switch(size)
			{
				case INV_KNOB_SIZE_MEDIUM:
				case INV_KNOB_SIZE_LARGE:
					strcpy(label,"Right");
					break;
				case INV_KNOB_SIZE_SMALL:
				default:
					strcpy(label,"R");
					break;
			}
		} else {
			inv_knob_label(0,label, units, human, max);
		}
		cairo_text_extents (cr,label,&extents);
		cairo_move_to(cr,size+1-extents.width,yc+r+8);
		cairo_show_text(cr,label);

		/* top */
		if(markings != INV_KNOB_MARKINGS_4) {
			if(markings==INV_KNOB_MARKINGS_PAN) {
				strcpy(label,"Centre");
			} else {
				inv_knob_label(0,label, units, human, inv_marking_to_value(1.0/2.0, curve, min, max));
			}
			cairo_text_extents (cr,label,&extents);
			switch(markings)
			{
				case INV_KNOB_MARKINGS_PAN:
				case INV_KNOB_MARKINGS_3:
				case INV_KNOB_MARKINGS_10:
					cairo_move_to(cr,xc-(extents.width/2)-1,(1.5*fontsize)+1);
					break;
				case INV_KNOB_MARKINGS_5:
					cairo_move_to(cr,xc-(extents.width/2)-1,fontsize+1);
					break;
			}
			cairo_show_text(cr,label);
		}

		/* M5 top left */
		if(markings==INV_KNOB_MARKINGS_5) {
			inv_knob_label(0,label, units,human, inv_marking_to_value(1.0/4.0, curve, min, max));
			cairo_move_to(cr,1,yc-r-1);
			cairo_show_text(cr,label);
		}

		/* M5 top right */
		if(markings==INV_KNOB_MARKINGS_5) {
			inv_knob_label(0,label, units,human, inv_marking_to_value(3.0/4.0, curve, min, max));
			cairo_text_extents (cr,label,&extents);
			cairo_move_to(cr,size+1-extents.width,yc-r-1);
			cairo_show_text(cr,label);
		}

		/* M4 top left */
		if(markings==INV_KNOB_MARKINGS_4) {
			inv_knob_label(0,label, units,human, inv_marking_to_value(1.0/3.0, curve, min, max));
			cairo_move_to(cr,1,yc-r-(ls+1));
			cairo_show_text(cr,label);
		}

		/* M4 top right */
		if(markings==INV_KNOB_MARKINGS_4) {
			inv_knob_label(0,label, units,human, inv_marking_to_value(2.0/3.0, curve, min, max));
			cairo_text_extents (cr,label,&extents);
			cairo_move_to(cr,size+1-extents.width,yc-r-(ls+1));
			cairo_show_text(cr,label);
		}
		cairo_new_path(cr);

		cairo_set_line_join (cr, CAIRO_LINE_JOIN_MITER);
		cairo_set_antialias (cr,CAIRO_ANTIALIAS_NONE);
		cairo_set_line_width(cr,1);

		gdk_cairo_set_source_color(cr,&style->dark[GTK_STATE_NORMAL]);
		cairo_move_to(cr, 3, yc+r+ll+tb+10);
		cairo_line_to(cr, 3, yc+r+ll+8);
		cairo_line_to(cr, 2*r+1, yc+r+ll+8);
		cairo_stroke(cr);

		gdk_cairo_set_source_color(cr,&style->light[GTK_STATE_NORMAL]);
		cairo_move_to(cr, 3, yc+r+ll+tb+10);
		cairo_line_to(cr, 2*r+1, yc+r+ll+tb+10);
		cairo_line_to(cr, 2*r+1, yc+r+ll+8);
		cairo_stroke(cr);

		cairo_set_antialias (cr,CAIRO_ANTIALIAS_DEFAULT);
		cairo_new_path(cr);


	}

	gdk_cairo_set_source_color(cr,&style->base[state]);
	cairo_rectangle(cr, 4, yc+r+ll+9, 2*r-4, tb);
	cairo_fill(cr);

	cairo_set_font_size(cr,fontsize);
	gdk_cairo_set_source_color(cr,&style->text[state]);
	inv_knob_label(1,label, units, human, value);
	cairo_text_extents (cr,label,&extents);
	cairo_move_to(cr,xc-(extents.width/2)-1,yc+r+ll+11+extents.height);
	cairo_show_text(cr,label);

	cairo_new_path(cr);

	/* knob -8 */
	angle=inv_value_to_angle(value,curve,min,max);
	cairo_set_line_width(cr,1.0);

	gdk_cairo_set_source_color(cr,&style->bg[state]);
	cairo_arc(cr,xc,yc,r-7,0,2*INV_PI);
	cairo_fill(cr);

	gdk_cairo_set_source_color(cr,&style->fg[state]);
	cairo_arc(cr,xc,yc,r-8,0,2*INV_PI);
	cairo_stroke(cr);

	cairo_set_line_width(cr,2.0);
	cairo_set_source_rgb(cr, 0, 0.1, 1);
	cairo_move_to(cr,xc-(r-9)*sin(angle+(INV_PI/3)), yc + (r-9)*cos(angle+(INV_PI/3)));
	cairo_line_to(cr,xc-(r-19)*sin(angle+(INV_PI/3)), yc + (r-19)*cos(angle+(INV_PI/3)));
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

static gboolean 
inv_knob_button_press_event (GtkWidget *widget, GdkEventButton *event)
{
	g_assert(INV_IS_KNOB(widget));
	gtk_widget_set_state(widget,GTK_STATE_ACTIVE);
    	gtk_widget_grab_focus(widget);

	INV_KNOB(widget)->click_x=event->x;
	INV_KNOB(widget)->click_y=event->y;

	inv_knob_paint(widget,INV_KNOB_DRAW_ALL);

	return TRUE;
}

static gboolean	
inv_knob_motion_notify_event(GtkWidget *widget, GdkEventMotion *event)
{
	g_assert(INV_IS_KNOB(widget));

	INV_KNOB(widget)->value = inv_value_from_motion(INV_KNOB(widget)->click_x-event->x, 
							INV_KNOB(widget)->click_y-event->y, 
							INV_KNOB(widget)->value, 
							INV_KNOB(widget)->curve, 
							INV_KNOB(widget)->min, 
							INV_KNOB(widget)->max );
	INV_KNOB(widget)->click_y = event->y; 
	inv_knob_paint(widget,INV_KNOB_DRAW_DATA);
	return FALSE; //let the after signal run
}

static gboolean 
inv_knob_button_release_event (GtkWidget *widget, GdkEventButton *event)
{
	g_assert(INV_IS_KNOB(widget));
	gtk_widget_set_state(widget,GTK_STATE_NORMAL);
	inv_knob_paint(widget,INV_KNOB_DRAW_ALL);

	return TRUE;
}

static void
inv_knob_label(gint mode, char *label, char *units, gint human, float value)
{
	if(mode==0) {
		if(human==1) {
			if(fabs(value)<0.001) {
				sprintf(label,"%0.0fµ%s",value*1000000,units);
			} else if(fabs(value)<1) {
				sprintf(label,"%0.0fm%s",value*1000,units);
			} else if(fabs(value<1000)) {
				sprintf(label,"%0.0f%s",value,units);
			} else if(fabs(value<1000000)) {
				sprintf(label,"%0.0fk%s",value/1000,units);
			} else {
				sprintf(label,"%0.0fM%s",value/1000000,units);
			}
		} else {
			sprintf(label,"%0.0f%s",value,units);
		}
	} else {
		if(human==1) {
			if(fabs(value)<0.001) {
				sprintf(label,"%0.3g µ%s",value*1000000,units);
			} else if(fabs(value)<1) {
				sprintf(label,"%0.3g m%s",value*1000,units);
			} else if(fabs(value<1000)) {
				sprintf(label,"%0.3g %s",value,units);
			} else if(fabs(value<1000000)) {
				sprintf(label,"%0.3g k%s",value/1000,units);
			} else {
				sprintf(label,"%0.3g M%s",value/1000000,units);
			}
		} else {
			sprintf(label,"%0.3g %s",value,units);
		}
	}
} 

static float
inv_marking_to_value(float mark, gint curve, float min, float max)
{
	float value;
	
	switch(curve)
	{
		case INV_KNOB_CURVE_LOG:
			value=pow(10,log10(min)+(mark*(log10(max)-log10(min))));
			break;
		case INV_KNOB_CURVE_QUAD:
			value=0;
			break;
		case INV_KNOB_CURVE_LINEAR:
		default:
			value = (max-min) * mark;
			break;
	}
	return value;
}

static float
inv_value_to_angle(float value, gint curve, float min, float max)
{
	float angle;

	switch(curve)
	{
		case INV_KNOB_CURVE_LOG:
			angle=(4.0*INV_PI*(log10(value)-log10(min)))/(3*(log10(max)-log10(min))) ;
			break;
		case INV_KNOB_CURVE_QUAD:
			angle=0;
			break;
		case INV_KNOB_CURVE_LINEAR:
		default:
			angle = (4.0*INV_PI*(value-min))/(3*(max-min)) ;
			break;
	}
	return angle;
}

static gint
inv_choose_light_dark(GdkColor *bg,GdkColor *light,GdkColor *dark)
{

	float ld,dd;

	ld=pow(bg->red-light->red,2) + pow(bg->green-light->green,2) + pow(bg->blue-light->blue,2);
	dd=pow(bg->red-dark->red,2) + pow(bg->green-dark->green,2) + pow(bg->blue-dark->blue,2);

	return ld > dd ? 1 : 0;
}

static float
inv_value_from_motion(float x_delta, float y_delta, float current, gint curve, float min, float max)
{
	float sens,value;

	sens=1/(75*(1+fabs(x_delta/10)));

	switch(curve)
	{
		case INV_KNOB_CURVE_LOG:
			value = pow(10,log10(current) + (y_delta * sens * (log10(max)-log10(min))));
			break;
		case INV_KNOB_CURVE_QUAD:
			value=0;
			break;
		case INV_KNOB_CURVE_LINEAR:
		default:
			value = current + (y_delta * sens * (max-min));
			break;
	}	

	if(value < min) value = min;
	if(value > max) value = max;

	return value;
}

