#ifndef __SWITCH_TOGGLE_H
#define __SWITCH_TOGGLE_H

#include <gtk/gtk.h>
#include <cairo.h>

G_BEGIN_DECLS

#define INV_PI 3.1415926535

#define INV_SWITCH_TOGGLE_ON  0
#define INV_SWITCH_TOGGLE_OFF 1

#define INV_SWITCH_TOGGLE(obj) GTK_CHECK_CAST(obj, inv_switch_toggle_get_type (), InvSwitchToggle)
#define INV_SWITCH_TOGGLE_CLASS(klass) GTK_CHECK_CLASS_CAST(klass, inv_switch_toggle_get_type(), InvSwitchToggleClass)
#define INV_IS_SWITCH_TOGGLE(obj) GTK_CHECK_TYPE(obj, inv_switch_toggle_get_type())


typedef struct _InvSwitchToggle InvSwitchToggle;
typedef struct _InvSwitchToggleClass InvSwitchToggleClass;


struct _InvSwitchToggle {
	GtkWidget widget;

	gint state;
	gint laststate;
	float value;

	float on_value;
	float off_value;

	float on_colour[3];
	float off_colour[3];

	char on_text[15];
	char off_text[15];

	char label[15];

};

struct _InvSwitchToggleClass {
	GtkWidgetClass parent_class;
};


GtkType inv_switch_toggle_get_type(void);
GtkWidget * inv_switch_toggle_new();

void inv_switch_toggle_toggle(InvSwitchToggle *switch_toggle);
void inv_switch_toggle_set_state(InvSwitchToggle *switch_toggle, gint state);
void inv_switch_toggle_set_value(InvSwitchToggle *switch_toggle, gint state, float value);
void inv_switch_toggle_set_colour(InvSwitchToggle *switch_toggle, gint state, float R, float G, float B);
void inv_switch_toggle_set_text(InvSwitchToggle *switch_toggle, gint state, const char *text);
void inv_switch_toggle_set_label(InvSwitchToggle *switch_toggle, const char *text);

G_END_DECLS

#endif /* __SWITCH_TOGGLE_H */
