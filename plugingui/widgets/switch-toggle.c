#include "stdlib.h"
#include "math.h"
#include "string.h"
#include "switch-toggle.h"

static void 	inv_switch_toggle_class_init(InvSwitchToggleClass *klass);
static void 	inv_switch_toggle_init(InvSwitchToggle *switch_toggle);
static void 	inv_switch_toggle_size_request(GtkWidget *widget, GtkRequisition *requisition);
static void 	inv_switch_toggle_size_allocate(GtkWidget *widget, GtkAllocation *allocation);
static void 	inv_switch_toggle_realize(GtkWidget *widget);
static gboolean inv_switch_toggle_expose(GtkWidget *widget,GdkEventExpose *event);
static void 	inv_switch_toggle_paint(GtkWidget *widget);
static gboolean	inv_switch_toggle_button_press_event (GtkWidget *widget, GdkEventButton *event);
static gboolean inv_switch_toggle_button_release_event (GtkWidget *widget, GdkEventButton *event);
static void	inv_switch_toggle_destroy(GtkObject *object);

static gint	inv_switch_choose_light_dark(GdkColor *bg,GdkColor *light,GdkColor *dark);


GtkType
inv_switch_toggle_get_type(void)
{
	static GType inv_switch_toggle_type = 0;
	char *name;
	int i;


	if (!inv_switch_toggle_type) 
	{
		static const GTypeInfo type_info = {
			sizeof(InvSwitchToggleClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc)inv_switch_toggle_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof(InvSwitchToggle),
			0,    /* n_preallocs */
			(GInstanceInitFunc)inv_switch_toggle_init
		};
		for (i = 0; ; i++) {
			name = g_strdup_printf("InvSwitchToggle-%p-%d",inv_switch_toggle_class_init, i);
			if (g_type_from_name(name)) {
				free(name);
				continue;
			}
			inv_switch_toggle_type = g_type_register_static(GTK_TYPE_WIDGET,name,&type_info,(GTypeFlags)0);
			free(name);
			break;
		}
	}
	return inv_switch_toggle_type;
}

void
inv_switch_toggle_toggle(InvSwitchToggle *switch_toggle)
{
	if(switch_toggle->state == INV_SWITCH_TOGGLE_ON) {
		switch_toggle->state = INV_SWITCH_TOGGLE_OFF;
		switch_toggle->value = switch_toggle->off_value;
	} else {
		switch_toggle->state = INV_SWITCH_TOGGLE_ON;
		switch_toggle->value = switch_toggle->on_value;
	}
	if(GTK_WIDGET_REALIZED(switch_toggle))
		inv_switch_toggle_paint(GTK_WIDGET(switch_toggle));
}

float
inv_switch_toggle_get_value(InvSwitchToggle *switch_toggle)
{
	return switch_toggle->value;
}

void
inv_switch_toggle_set_state(InvSwitchToggle *switch_toggle, gint state)
{
	switch_toggle->state = state;
	switch(state) {
		case INV_SWITCH_TOGGLE_ON:
			switch_toggle->value = switch_toggle->on_value;
			break;
		case INV_SWITCH_TOGGLE_OFF:
			switch_toggle->value = switch_toggle->off_value;
			break;
	}
	if(GTK_WIDGET_REALIZED(switch_toggle))
		inv_switch_toggle_paint(GTK_WIDGET(switch_toggle));
}

void inv_switch_toggle_set_value(InvSwitchToggle *switch_toggle, gint state, float value)
{
	switch(state) {
		case INV_SWITCH_TOGGLE_ON:
			switch_toggle->on_value=value;
			break;
		case INV_SWITCH_TOGGLE_OFF:
			switch_toggle->off_value=value;
			break;
	}
}

void inv_switch_toggle_set_colour(InvSwitchToggle *switch_toggle, gint state, float R, float G, float B)
{
	switch(state) {
		case INV_SWITCH_TOGGLE_ON:
			switch_toggle->on_colour[0]=R;
			switch_toggle->on_colour[1]=G;
			switch_toggle->on_colour[2]=B;
			break;
		case INV_SWITCH_TOGGLE_OFF:
			switch_toggle->off_colour[0]=R;
			switch_toggle->off_colour[1]=G;
			switch_toggle->off_colour[2]=B;
			break;
	}
}

void inv_switch_toggle_set_text(InvSwitchToggle *switch_toggle, gint state, const char *text)
{
	switch(state) {
		case INV_SWITCH_TOGGLE_ON:
			strncpy(switch_toggle->on_text,text,14);
			break;
		case INV_SWITCH_TOGGLE_OFF:
			strncpy(switch_toggle->off_text,text,14);
			break;
	}
}

void inv_switch_toggle_set_label(InvSwitchToggle *switch_toggle, const char *text)
{
		strncpy(switch_toggle->label,text,14);
}

GtkWidget * inv_switch_toggle_new()
{
	return GTK_WIDGET(gtk_type_new(inv_switch_toggle_get_type()));
}


static void
inv_switch_toggle_class_init(InvSwitchToggleClass *klass)
{
	GtkWidgetClass *widget_class;
	GtkObjectClass *object_class;


	widget_class = (GtkWidgetClass *) klass;
	object_class = (GtkObjectClass *) klass;

	widget_class->realize = inv_switch_toggle_realize;
	widget_class->size_request = inv_switch_toggle_size_request;
	widget_class->size_allocate = inv_switch_toggle_size_allocate;
	widget_class->expose_event = inv_switch_toggle_expose;

    	widget_class->button_press_event = inv_switch_toggle_button_press_event;
    	widget_class->button_release_event = inv_switch_toggle_button_release_event;

	object_class->destroy = inv_switch_toggle_destroy;
}


static void
inv_switch_toggle_init(InvSwitchToggle *switch_toggle)
{

	switch_toggle->state = INV_SWITCH_TOGGLE_OFF;
	switch_toggle->laststate = INV_SWITCH_TOGGLE_OFF;
	switch_toggle->value=0;

	switch_toggle->on_value=1;
	switch_toggle->off_value=0;

	switch_toggle->on_colour[0] =0.0;	switch_toggle->on_colour[1] =1.0;	switch_toggle->on_colour[2] =0.0;
	switch_toggle->off_colour[0] =1.0;	switch_toggle->off_colour[1] =0.0;	switch_toggle->off_colour[2] =0.0;

	strcpy(switch_toggle->on_text,"");
	strcpy(switch_toggle->off_text,"");
	strcpy(switch_toggle->label,"");

    	GTK_WIDGET_SET_FLAGS (GTK_WIDGET(switch_toggle), GTK_CAN_FOCUS);
}


static void
inv_switch_toggle_size_request(GtkWidget *widget,
    GtkRequisition *requisition)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(INV_IS_SWITCH_TOGGLE(widget));
	g_return_if_fail(requisition != NULL);

	requisition->width = 64;
	requisition->height = 64;
}


static void
inv_switch_toggle_size_allocate(GtkWidget *widget,
    GtkAllocation *allocation)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(INV_IS_SWITCH_TOGGLE(widget));
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
inv_switch_toggle_realize(GtkWidget *widget)
{
	GdkWindowAttr attributes;
	guint attributes_mask;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(INV_IS_SWITCH_TOGGLE(widget));

	GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
	attributes.width = 64;
	attributes.height = 64;

	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.event_mask = gtk_widget_get_events(widget) | 
				GDK_EXPOSURE_MASK | 
				GDK_BUTTON_PRESS_MASK | 
				GDK_BUTTON_RELEASE_MASK;

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
inv_switch_toggle_expose(GtkWidget *widget, GdkEventExpose *event)
{
	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(INV_IS_SWITCH_TOGGLE(widget), FALSE);
	g_return_val_if_fail(event != NULL, FALSE);

	inv_switch_toggle_paint(widget);

	return FALSE;
}


static void
inv_switch_toggle_paint(GtkWidget *widget)
{
	gint 		state;
	gint 		laststate;
	float		onR,onG,onB,offR,offG,offB;
	char		*on_text;
	char		*off_text;
	char		*label;

	float 			max,grey;
	cairo_t 		*cr;
	GtkStyle		*style;
	cairo_text_extents_t 	extents;
	cairo_pattern_t 	*pat;

	style = gtk_widget_get_style(widget);

	state = INV_SWITCH_TOGGLE(widget)->state;
	laststate = INV_SWITCH_TOGGLE(widget)->laststate;
	onR = INV_SWITCH_TOGGLE(widget)->on_colour[0];
	onG = INV_SWITCH_TOGGLE(widget)->on_colour[1];
	onB = INV_SWITCH_TOGGLE(widget)->on_colour[2];
	offR = INV_SWITCH_TOGGLE(widget)->off_colour[0];
	offG = INV_SWITCH_TOGGLE(widget)->off_colour[1];
	offB = INV_SWITCH_TOGGLE(widget)->off_colour[2];
	on_text = INV_SWITCH_TOGGLE(widget)->on_text;
	off_text = INV_SWITCH_TOGGLE(widget)->off_text;
	label = INV_SWITCH_TOGGLE(widget)->label;

	cr = gdk_cairo_create(widget->window);
/*
	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_paint(cr);
*/

	cairo_select_font_face(cr,"monospace",CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cr,9);

	cairo_set_line_join (cr, CAIRO_LINE_JOIN_MITER);
	cairo_set_antialias (cr,CAIRO_ANTIALIAS_NONE);
	cairo_set_line_width(cr,1);

	gdk_cairo_set_source_color(cr,&style->dark[GTK_STATE_NORMAL]);
	cairo_move_to(cr, 0, 13);
	cairo_line_to(cr, 63, 13);
	cairo_line_to(cr, 63, 0);
	cairo_move_to(cr, 0, 63);
	cairo_line_to(cr, 63, 63);
	cairo_line_to(cr, 63, 50);
	cairo_stroke(cr);

	gdk_cairo_set_source_color(cr,&style->light[GTK_STATE_NORMAL]);
	cairo_move_to(cr, 0, 13);
	cairo_line_to(cr, 0, 0);
	cairo_line_to(cr, 63, 0);
	cairo_move_to(cr, 0, 63);
	cairo_line_to(cr, 0, 50);
	cairo_line_to(cr, 63, 50);
	cairo_stroke(cr);

	cairo_set_antialias (cr,CAIRO_ANTIALIAS_DEFAULT);
	cairo_new_path(cr);

	if(inv_switch_choose_light_dark(&style->bg[GTK_STATE_NORMAL],&style->light[GTK_STATE_NORMAL],&style->dark[GTK_STATE_NORMAL])==1) {
		gdk_cairo_set_source_color(cr,&style->light[GTK_STATE_NORMAL]);
	} else {
		gdk_cairo_set_source_color(cr,&style->dark[GTK_STATE_NORMAL]);
	}
	cairo_move_to(cr, 31, 14);
	cairo_line_to(cr, 31, 49);
	cairo_stroke(cr);

	switch(state) {
		case INV_SWITCH_TOGGLE_ON:

			max = offR > offG ? offR : offG;
			max = offB > max ? offB : max;
			grey=max/3;

			cairo_set_source_rgb(cr, grey/3, grey/3, grey/3);
			cairo_rectangle(cr, 1, 1, 62, 12);
			cairo_fill(cr);

			cairo_set_source_rgb(cr, grey, grey, grey);
			cairo_text_extents (cr,off_text,&extents);
			cairo_move_to(cr,31-(extents.width/2), extents.height + 3);
			cairo_show_text(cr,off_text);

			pat = cairo_pattern_create_linear (0.0, 0.0,  64.0, 0.0);
			cairo_pattern_add_color_stop_rgba (pat, 0.0, onR/6, onG/6, onB/6, 1);
			cairo_pattern_add_color_stop_rgba (pat, 0.2, onR/4, onG/4, onB/4, 1);
			cairo_pattern_add_color_stop_rgba (pat, 0.5, onR/2, onG/2, onB/2, 1);
			cairo_pattern_add_color_stop_rgba (pat, 0.8, onR/4, onG/4, onB/4, 1);
			cairo_pattern_add_color_stop_rgba (pat, 1.0, onR/6, onG/6, onB/6, 1);
			cairo_set_source (cr, pat);
			cairo_rectangle(cr, 1, 51, 62, 12);
			cairo_fill(cr);

			cairo_set_source_rgb(cr, onR, onG, onB);
			cairo_text_extents (cr,on_text,&extents);
			cairo_move_to(cr,31-(extents.width/2), 61);
			cairo_show_text(cr,on_text);
			break;

		case INV_SWITCH_TOGGLE_OFF:

			pat = cairo_pattern_create_linear (0.0, 0.0,  64.0, 0.0);
			cairo_pattern_add_color_stop_rgba (pat, 0.0, offR/6, offG/6, offB/6, 1);
			cairo_pattern_add_color_stop_rgba (pat, 0.2, offR/4, offG/4, offB/4, 1);
			cairo_pattern_add_color_stop_rgba (pat, 0.5, offR/2, offG/2, offB/2, 1);
			cairo_pattern_add_color_stop_rgba (pat, 0.8, offR/4, offG/4, offB/4, 1);
			cairo_pattern_add_color_stop_rgba (pat, 1.0, offR/6, offG/6, offB/6, 1);
			cairo_set_source (cr, pat);
			cairo_rectangle(cr, 1, 1, 62, 12);
			cairo_fill(cr);

			cairo_set_source_rgb(cr, offR, offG, offB);
			cairo_text_extents (cr,off_text,&extents);
			cairo_move_to(cr,31-(extents.width/2), extents.height + 3);
			cairo_show_text(cr,off_text);

			max = onR > onG ? onR : onG;
			max = onB > max ? onB : max;
			grey=max/3;

			cairo_set_source_rgb(cr, grey/3, grey/3, grey/3);
			cairo_rectangle(cr, 1, 51, 62, 12);
			cairo_fill(cr);

			cairo_set_source_rgb(cr, grey, grey, grey);
			cairo_text_extents (cr,on_text,&extents);
			cairo_move_to(cr,31-(extents.width)/2, 61);
			cairo_show_text(cr,on_text);
			break;
	}


  	cairo_destroy(cr);
}

static gboolean 
inv_switch_toggle_button_press_event (GtkWidget *widget, GdkEventButton *event)
{
	g_assert(INV_IS_SWITCH_TOGGLE(widget));
	gtk_widget_set_state(widget,GTK_STATE_ACTIVE);
    	gtk_widget_grab_focus(widget);

	inv_switch_toggle_paint(widget);

	return TRUE;
}


static gboolean 
inv_switch_toggle_button_release_event (GtkWidget *widget, GdkEventButton *event)
{
	g_assert(INV_IS_SWITCH_TOGGLE(widget));
	gtk_widget_set_state(widget,GTK_STATE_NORMAL);

	inv_switch_toggle_toggle(INV_SWITCH_TOGGLE (widget)); //this will also paint the widget

	return FALSE;  //let the signal in the gui run now
}

static void
inv_switch_toggle_destroy(GtkObject *object)
{
	InvSwitchToggle *switch_toggle;
	InvSwitchToggleClass *klass;

	g_return_if_fail(object != NULL);
	g_return_if_fail(INV_IS_SWITCH_TOGGLE(object));

	switch_toggle = INV_SWITCH_TOGGLE(object);

	klass = gtk_type_class(gtk_widget_get_type());

	if (GTK_OBJECT_CLASS(klass)->destroy) {
		(* GTK_OBJECT_CLASS(klass)->destroy) (object);
	}
}


static gint
inv_switch_choose_light_dark(GdkColor *bg,GdkColor *light,GdkColor *dark)
{

	float ld,dd;

	ld=pow(bg->red-light->red,2) + pow(bg->green-light->green,2) + pow(bg->blue-light->blue,2);
	dd=pow(bg->red-dark->red,2) + pow(bg->green-dark->green,2) + pow(bg->blue-dark->blue,2);

	return ld > dd ? 1 : 0;
}

