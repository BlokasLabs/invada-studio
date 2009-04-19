#ifndef __LAMP_H
#define __LAMP_H

#include <gtk/gtk.h>
#include <cairo.h>

G_BEGIN_DECLS

#define INV_PI 3.1415926535

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

	float l0_r[3];
	float l0_c[3];
	float l1_r[3];
	float l1_c[3];
	float l2_r[3];
	float l2_c[3];
	float l3_r[3];
	float l3_c[3];
	float l4_r[3];
	float l4_c[3];
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

