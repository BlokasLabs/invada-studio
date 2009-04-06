#ifndef __METER_H
#define __METER_H

#include <gtk/gtk.h>
#include <cairo.h>

G_BEGIN_DECLS


#define GTK_METER(obj) GTK_CHECK_CAST(obj, gtk_meter_get_type (), GtkMeter)
#define GTK_METER_CLASS(klass) GTK_CHECK_CLASS_CAST(klass, gtk_meter_get_type(), GtkMeterClass)
#define GTK_IS_METER(obj) GTK_CHECK_TYPE(obj, gtk_meter_get_type())


typedef struct _GtkMeter GtkMeter;
typedef struct _GtkMeterClass GtkMeterClass;


struct _GtkMeter {
  GtkWidget widget;

  gint sel;
};

struct _GtkMeterClass {
  GtkWidgetClass parent_class;
};


GtkType gtk_meter_get_type(void);
void gtk_meter_set_sel(GtkMeter *meter, gint sel);
GtkWidget * gtk_meter_new();


G_END_DECLS

#endif /* __METER_H */

