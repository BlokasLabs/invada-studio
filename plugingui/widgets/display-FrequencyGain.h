#ifndef __DISPLAYFG_H
#define __DISPLAYFG_H

#include <gtk/gtk.h>
#include <cairo.h>

G_BEGIN_DECLS

#define INV_DISPLAYFG_MODE_LPF 0
#define INV_DISPLAYFG_MODE_HPF 1

#define INV_DISPLAYFG_DRAW_ALL 0
#define INV_DISPLAYFG_DRAW_DATA 1

#define INV_DISPLAY_FG(obj) GTK_CHECK_CAST(obj, inv_display_fg_get_type (), InvDisplayFG)
#define INV_DISPLAY_FG_CLASS(klass) GTK_CHECK_CLASS_CAST(klass, inv_display_fg_get_type(), InvDisplayFGClass)
#define INV_IS_DISPLAY_FG(obj) GTK_CHECK_TYPE(obj, inv_display_fg_get_type())


typedef struct _InvDisplayFG InvDisplayFG;
typedef struct _InvDisplayFGClass InvDisplayFGClass;


struct _InvDisplayFG {
	GtkWidget widget;

	gint mode;
	float freq;
	float gain;
};

struct _InvDisplayFGClass {
	GtkWidgetClass parent_class;
};


GtkType inv_display_fg_get_type(void);
GtkWidget * inv_display_fg_new();

void inv_display_fg_set_mode(InvDisplayFG *displayFG, gint num);
void inv_display_fg_set_freq(InvDisplayFG *displayFG, float num);
void inv_display_fg_set_gain(InvDisplayFG *displayFG, float num);


G_END_DECLS

#endif /* __DISPLAYFG_H */

