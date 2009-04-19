#include "stdlib.h"
#include "string.h"
#include "switch-toggle.h"

static void 	inv_switch_toggle_class_init(InvSwitchToggleClass *klass);
static void 	inv_switch_toggle_init(InvSwitchToggle *switch_toggle);
static void 	inv_switch_toggle_size_request(GtkWidget *widget, GtkRequisition *requisition);
static void 	inv_switch_toggle_size_allocate(GtkWidget *widget, GtkAllocation *allocation);
static void 	inv_switch_toggle_realize(GtkWidget *widget);
static gboolean inv_switch_toggle_expose(GtkWidget *widget,GdkEventExpose *event);
static void 	inv_switch_toggle_paint(GtkWidget *widget);
static void	inv_switch_toggle_destroy(GtkObject *object);



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

void
inv_switch_toggle_set_state(InvSwitchToggle *switch_toggle, gint state)
{
	switch_toggle->state = state;
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
}


static void
inv_switch_toggle_size_request(GtkWidget *widget,
    GtkRequisition *requisition)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(INV_IS_SWITCH_TOGGLE(widget));
	g_return_if_fail(requisition != NULL);

	requisition->width = 48;
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
	attributes.width = 48;
	attributes.height = 64;

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
	cairo_t 	*cr;
	GtkStyle	*style;

	style = gtk_widget_get_style(widget);

	cr = gdk_cairo_create(widget->window);

	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_paint(cr);

	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_select_font_face(cr,"monospace",CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cr,8);

  	cairo_destroy(cr);
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




