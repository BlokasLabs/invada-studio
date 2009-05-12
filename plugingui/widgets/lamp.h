#ifndef __LAMP_H
#define __LAMP_H

#include <gtk/gtk.h>
#include <cairo.h>
#include "widgets.h"

G_BEGIN_DECLS

#define INV_LAMP_DRAW_ALL 0
#define INV_LAMP_DRAW_DATA 1

#define INV_LAMP(obj) GTK_CHECK_CAST(obj, inv_lamp_get_type (), InvLamp)
#define INV_LAMP_CLASS(klass) GTK_CHECK_CLASS_CAST(klass, inv_lamp_get_type(), InvLampClass)
#define INV_IS_LAMP(obj) GTK_CHECK_TYPE(obj, inv_lamp_get_type())


typedef struct _InvLamp InvLamp;
typedef struct _InvLampClass InvLampClass;


struct _InvLamp {
	GtkWidget widget;

	float scale;
	float value;

	float lastValue;

	struct colour l0_r, l1_r, l2_r, l3_r, l4_r;
	struct colour l0_c, l1_c, l2_c, l3_c, l4_c;
};

struct _InvLampClass {
	GtkWidgetClass parent_class;
};


GtkType inv_lamp_get_type(void);
GtkWidget * inv_lamp_new();

void inv_lamp_set_scale(InvLamp *lamp, float num);
void inv_lamp_set_value(InvLamp *lamp, float num);

G_END_DECLS

#endif /* __LAMP_H */

