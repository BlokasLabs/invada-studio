#ifndef __DISPLAY_COMP_H
#define __DISPLAY_COMP_H

#include <gtk/gtk.h>
#include <cairo.h>

G_BEGIN_DECLS

#define INV_PI 3.1415926535

#define INV_DISPLAYCOMP_DRAW_ALL 0
#define INV_DISPLAYCOMP_DRAW_DATA 1

#define INV_DISPLAY_COMP(obj) GTK_CHECK_CAST(obj, inv_display_comp_get_type (), InvDisplayComp)
#define INV_DISPLAY_COMP_CLASS(klass) GTK_CHECK_CLASS_CAST(klass, inv_display_comp_get_type(), InvDisplayCompClass)
#define INV_IS_DISPLAY_COMP(obj) GTK_CHECK_TYPE(obj, inv_display_comp_get_type())


typedef struct _InvDisplayComp InvDisplayComp;
typedef struct _InvDisplayCompClass InvDisplayCompClass;


struct _InvDisplayComp {
	GtkWidget widget;

	float rms;
	float attack;
	float release;
	float threshold;
	float ratio;
	float gain;

	float Lastrms;
	float Lastattack;
	float Lastrelease;
	float Lastthreshold;
	float Lastratio;
	float Lastgain;
};

struct _InvDisplayCompClass {
	GtkWidgetClass parent_class;
};


GtkType inv_display_comp_get_type(void);
GtkWidget * inv_display_comp_new();

void inv_display_comp_set_rms(InvDisplayComp *displayComp, float num);
void inv_display_comp_set_attack(InvDisplayComp *displayComp, float num);
void inv_display_comp_set_release(InvDisplayComp *displayComp, float num);
void inv_display_comp_set_threshold(InvDisplayComp *displayComp, float num);
void inv_display_comp_set_ratio(InvDisplayComp *displayComp, float num);
void inv_display_comp_set_gain(InvDisplayComp *displayComp, float num);


G_END_DECLS

#endif /* __DISPLAY_COMP_H */

