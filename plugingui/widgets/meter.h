#ifndef __METER_H
#define __METER_H

#include <gtk/gtk.h>
#include <cairo.h>

G_BEGIN_DECLS


#define INV_METER(obj) GTK_CHECK_CAST(obj, inv_meter_get_type (), GtkMeter)
#define INV_METER_CLASS(klass) GTK_CHECK_CLASS_CAST(klass, inv_meter_get_type(), GtkMeterClass)
#define INV_IS_METER(obj) GTK_CHECK_TYPE(obj, inv_meter_get_type())


typedef struct _GtkMeter GtkMeter;
typedef struct _GtkMeterClass GtkMeterClass;


struct _GtkMeter {
	GtkWidget widget;

	gint  channels;
	float LdB;
	float RdB;
};

struct _GtkMeterClass {
	GtkWidgetClass parent_class;
};


GtkType inv_meter_get_type(void);
GtkWidget * inv_meter_new();

void inv_meter_set_channels(GtkMeter *meter, gint num);
void inv_meter_set_LdB(GtkMeter *meter, float num);
void inv_meter_set_RdB(GtkMeter *meter, float num);


G_END_DECLS

#endif /* __METER_H */

