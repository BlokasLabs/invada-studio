#ifndef __METER_H
#define __METER_H

#include <gtk/gtk.h>
#include <cairo.h>

G_BEGIN_DECLS

#define INV_METER_DRAW_ALL 0
#define INV_METER_DRAW_DATA 1

#define INV_METER(obj) GTK_CHECK_CAST(obj, inv_meter_get_type (), InvMeter)
#define INV_METER_CLASS(klass) GTK_CHECK_CLASS_CAST(klass, inv_meter_get_type(), InvMeterClass)
#define INV_IS_METER(obj) GTK_CHECK_TYPE(obj, inv_meter_get_type())


typedef struct _InvMeter InvMeter;
typedef struct _InvMeterClass InvMeterClass;


struct _InvMeter {
	GtkWidget widget;

	gint  channels;
	float LdB;
	float RdB;

	float mOff60[3];
	float mOn60[3];  /* delta */

	float mOff12[3];
	float mOn12[3];  /* delta */

	float mOff6[3];
	float mOn6[3];  /* delta */

	float mOff0[3];
	float mOn0[3];   /* delta */

	float overOff[3];
	float overOn[3]; /* delta */
};

struct _InvMeterClass {
	GtkWidgetClass parent_class;
};


GtkType inv_meter_get_type(void);
GtkWidget * inv_meter_new();

void inv_meter_set_channels(InvMeter *meter, gint num);
void inv_meter_set_LdB(InvMeter *meter, float num);
void inv_meter_set_RdB(InvMeter *meter, float num);


G_END_DECLS

#endif /* __METER_H */

