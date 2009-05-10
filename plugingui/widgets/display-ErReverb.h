#ifndef __DISPLAY_ERR_H
#define __DISPLAY_ERR_H

#include <gtk/gtk.h>
#include <cairo.h>
#include "../../plugin/library/common.h"

G_BEGIN_DECLS

#define INV_PI 3.1415926535

#define INV_DISPLAY_ERR_DRAW_ALL 0
#define INV_DISPLAY_ERR_DRAW_DATA 1

#define INV_DISPLAY_ERR_ROOM_LENGTH 0
#define INV_DISPLAY_ERR_ROOM_WIDTH  1
#define INV_DISPLAY_ERR_ROOM_HEIGHT 2

#define INV_DISPLAY_ERR_LR 0
#define INV_DISPLAY_ERR_FB 1

#define INV_DISPLAY_ERR_ACTIVE_NONE 0
#define INV_DISPLAY_ERR_ACTIVE_SOURCE 1
#define INV_DISPLAY_ERR_ACTIVE_DEST 2

#define INV_DISPLAY_ERR(obj) GTK_CHECK_CAST(obj, inv_display_err_get_type (), InvDisplayErr)
#define INV_DISPLAY_ERR_CLASS(klass) GTK_CHECK_CLASS_CAST(klass, inv_display_err_get_type(), InvDisplayErrClass)
#define INV_IS_DISPLAY_ERR(obj) GTK_CHECK_TYPE(obj, inv_display_err_get_type())


typedef struct _InvDisplayErr InvDisplayErr;
typedef struct _InvDisplayErrClass InvDisplayErrClass;


struct _InvDisplayErr {
	GtkWidget widget;
	
	gint active_dot;

	float room[3];
	float source[2];
	float dest[2];
	float diffusion;

	float Lastroom[3];
	float Lastsource[2];
	float Lastdest[2];
	float Lastdiffusion;

	struct ERunit * er;
	gint er_size;
};

struct _InvDisplayErrClass {
	GtkWidgetClass parent_class;
};

struct point2D {
	float x; 
	float y;
};

struct point3D {
	float x; 
	float y;
	float z;
};

GtkType inv_display_err_get_type(void);
GtkWidget * inv_display_err_new();

void inv_display_err_set_room(InvDisplayErr *displayErr, gint axis, float num);
void inv_display_err_set_source(InvDisplayErr *displayErr, gint axis, float num);
void inv_display_err_set_dest(InvDisplayErr *displayErr, gint axis, float num);
void inv_display_err_set_diffusion(InvDisplayErr *displayErr, float num);

G_END_DECLS

#endif /* __DISPLAY_ERR_H */

