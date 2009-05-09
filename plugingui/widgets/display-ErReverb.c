
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "../../plugin/library/common.h"
#include "display-ErReverb.h"



static void 	inv_display_err_class_init(InvDisplayErrClass *klass);
static void 	inv_display_err_init(InvDisplayErr *displayErr);
static void 	inv_display_err_size_request(GtkWidget *widget, GtkRequisition *requisition);
static void 	inv_display_err_size_allocate(GtkWidget *widget, GtkAllocation *allocation);
static void 	inv_display_err_realize(GtkWidget *widget);
static gboolean inv_display_err_expose(GtkWidget *widget,GdkEventExpose *event);
static void 	inv_display_err_paint(GtkWidget *widget, gint mode);
static void	inv_display_err_destroy(GtkObject *object);
static void	inv_display_err_screen(struct point3D *point, struct point2D * screen, struct point3D * camera, struct point3D *lookat);


GtkType
inv_display_err_get_type(void)
{
	static GtkType inv_display_err_type = 0;
	char *name;
	int i;


	if (!inv_display_err_type) 
	{
		static const GTypeInfo type_info = {
			sizeof(InvDisplayErrClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc)inv_display_err_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof(InvDisplayErr),
			0,    /* n_preallocs */
			(GInstanceInitFunc)inv_display_err_init
		};
		for (i = 0; ; i++) {
			name = g_strdup_printf("InvDisplayErr-%p-%d",inv_display_err_class_init, i);
			if (g_type_from_name(name)) {
				free(name);
				continue;
			}
			inv_display_err_type = g_type_register_static(GTK_TYPE_WIDGET,name,&type_info,(GTypeFlags)0);
			free(name);
			break;
		}
	}
	return inv_display_err_type;
}

void
inv_display_err_set_room(InvDisplayErr *displayErr, gint axis, float num)
{
	switch(axis) {
		case INV_DISPLAY_ERR_ROOM_LENGTH:
		case INV_DISPLAY_ERR_ROOM_WIDTH:
			if(num<3) num=3;
			if(num>100) num=100;
			displayErr->room[axis]=num;
			break;
		case INV_DISPLAY_ERR_ROOM_HEIGHT:
			if(num<3) num=3;
			if(num>30) num=30;
			displayErr->room[axis]=num;
			break;
	}
	if(GTK_WIDGET_REALIZED(displayErr))
		inv_display_err_paint(GTK_WIDGET(displayErr),INV_DISPLAY_ERR_DRAW_DATA);
}

void
inv_display_err_set_source(InvDisplayErr *displayErr, gint axis, float num)
{
	switch(axis) {
		case INV_DISPLAY_ERR_LR:
			if(num<-1) num=-1;
			if(num>1) num=1;
			displayErr->source[axis]=num;
			break;
		case INV_DISPLAY_ERR_FB:
			if(num<0.51) num=0.51;
			if(num>0.99) num=0.99;
			displayErr->source[axis]=num;
			break;
	}
	if(GTK_WIDGET_REALIZED(displayErr))
		inv_display_err_paint(GTK_WIDGET(displayErr),INV_DISPLAY_ERR_DRAW_DATA);
}

void
inv_display_err_set_dest(InvDisplayErr *displayErr, gint axis, float num)
{
	switch(axis) {
		case INV_DISPLAY_ERR_LR:
			if(num<-1) num=-1;
			if(num>1) num=1;
			displayErr->dest[axis]=num;
			break;
		case INV_DISPLAY_ERR_FB:
			if(num<0.01) num=0.01;
			if(num>0.49) num=0.49;
			displayErr->dest[axis]=num;
			break;
	}
	if(GTK_WIDGET_REALIZED(displayErr))
		inv_display_err_paint(GTK_WIDGET(displayErr),INV_DISPLAY_ERR_DRAW_DATA);
}

void
inv_display_err_set_diffusion(InvDisplayErr *displayErr, float num)
{
	if(num<0) num=0;
	if(num>100) num=1;
	displayErr->diffusion=num/100;

	if(GTK_WIDGET_REALIZED(displayErr))
		inv_display_err_paint(GTK_WIDGET(displayErr),INV_DISPLAY_ERR_DRAW_DATA);
}

GtkWidget * inv_display_err_new()
{
	return GTK_WIDGET(gtk_type_new(inv_display_err_get_type()));
}


static void
inv_display_err_class_init(InvDisplayErrClass *klass)
{
	GtkWidgetClass *widget_class;
	GtkObjectClass *object_class;


	widget_class = (GtkWidgetClass *) klass;
	object_class = (GtkObjectClass *) klass;

	widget_class->realize = inv_display_err_realize;
	widget_class->size_request = inv_display_err_size_request;
	widget_class->size_allocate = inv_display_err_size_allocate;
	widget_class->expose_event = inv_display_err_expose;

	object_class->destroy = inv_display_err_destroy;
}


static void
inv_display_err_init(InvDisplayErr *displayErr)
{
	displayErr->room[0]=25.0;
	displayErr->room[1]=30.0;
	displayErr->room[2]=10.0;
	displayErr->source[0]=-0.01;
	displayErr->source[1]=0.8;
	displayErr->dest[0]=0.01;
	displayErr->dest[1]=0.2;

	displayErr->Lastroom[0]=25.0;
	displayErr->Lastroom[1]=30.0;
	displayErr->Lastroom[2]=10.0;
	displayErr->Lastsource[0]=-0.01;
	displayErr->Lastsource[1]=0.8;
	displayErr->Lastdest[0]=0.01;
	displayErr->Lastdest[1]=0.2;

	displayErr->er=(struct ERunit *)malloc(sizeof(struct ERunit) * MAX_ER);
	displayErr->er_size=0;
}


static void
inv_display_err_size_request(GtkWidget *widget,
    GtkRequisition *requisition)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(INV_IS_DISPLAY_ERR(widget));
	g_return_if_fail(requisition != NULL);

	requisition->width = 510;
	requisition->height = 300;
}


static void
inv_display_err_size_allocate(GtkWidget *widget,
    GtkAllocation *allocation)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(INV_IS_DISPLAY_ERR(widget));
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
inv_display_err_realize(GtkWidget *widget)
{
	GdkWindowAttr attributes;
	guint attributes_mask;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(INV_IS_DISPLAY_ERR(widget));

	GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
	attributes.width = 510;
	attributes.height = 300;

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
inv_display_err_expose(GtkWidget *widget, GdkEventExpose *event)
{
	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(INV_IS_DISPLAY_ERR(widget), FALSE);
	g_return_val_if_fail(event != NULL, FALSE);

	inv_display_err_paint(widget,INV_DISPLAY_ERR_DRAW_ALL);

	return FALSE;
}


static void
inv_display_err_paint(GtkWidget *widget, gint mode)
{
	float 		w,l,h;
	float 		sLR,sFB;
	float 		dLR,dFB;
	float 		diffusion;
	struct ERunit   *er;
	gint		er_size;	

	gint		i;
	float		sw,sl,scale;
	float		x,y,xc,yc,r;
	struct point3D	camera,lookat,source,dest;
	struct point2D	sourceS,destS;
	struct point3D	room[8];
	struct point2D	roomS[8];
	float		sx,sy;
	char		label[15];
	float		max_delay,max_gain;

	cairo_t 	*cr;
	GtkStyle	*style;
	cairo_text_extents_t extents;

	l=INV_DISPLAY_ERR(widget)->room[INV_DISPLAY_ERR_ROOM_LENGTH];
	w=INV_DISPLAY_ERR(widget)->room[INV_DISPLAY_ERR_ROOM_WIDTH];
	h=INV_DISPLAY_ERR(widget)->room[INV_DISPLAY_ERR_ROOM_HEIGHT];
	sLR=INV_DISPLAY_ERR(widget)->source[INV_DISPLAY_ERR_LR];
	sFB=INV_DISPLAY_ERR(widget)->source[INV_DISPLAY_ERR_FB];
	dLR=INV_DISPLAY_ERR(widget)->dest[INV_DISPLAY_ERR_LR];
	dFB=INV_DISPLAY_ERR(widget)->dest[INV_DISPLAY_ERR_FB];
	diffusion=INV_DISPLAY_ERR(widget)->diffusion;

	er=INV_DISPLAY_ERR(widget)->er;
	er_size=INV_DISPLAY_ERR(widget)->er_size;

	style = gtk_widget_get_style(widget);
	cr = gdk_cairo_create(widget->window);

	if(mode==INV_DISPLAY_ERR_DRAW_ALL) {
		cairo_set_line_join (cr, CAIRO_LINE_JOIN_MITER);
		cairo_set_line_width(cr,1);

		for(i=0;i<2;i++) {

			cairo_set_antialias (cr,CAIRO_ANTIALIAS_NONE);
			gdk_cairo_set_source_color(cr,&style->dark[GTK_STATE_NORMAL]);
			cairo_move_to(cr, 0, (i*150)+149);
			cairo_line_to(cr, 0, (i*150));
			cairo_line_to(cr, 209, (i*150));
			cairo_stroke(cr);

			gdk_cairo_set_source_color(cr,&style->light[GTK_STATE_NORMAL]);
			cairo_move_to(cr, 0, (i*150)+149);
			cairo_line_to(cr, 209, (i*150)+149);
			cairo_line_to(cr, 209, (i*150));
			cairo_stroke(cr);

			cairo_set_antialias (cr,CAIRO_ANTIALIAS_DEFAULT);

			cairo_set_source_rgb(cr, 0.05, 0.05, 0.2);
			cairo_rectangle(cr, 1, (i*150)+1, 208, 148 );
			cairo_fill(cr);
		}

		cairo_set_antialias (cr,CAIRO_ANTIALIAS_NONE);
		gdk_cairo_set_source_color(cr,&style->dark[GTK_STATE_NORMAL]);
		cairo_move_to(cr, 210, 299);
		cairo_line_to(cr, 210, 0);
		cairo_line_to(cr, 509, 0);
		cairo_stroke(cr);

		gdk_cairo_set_source_color(cr,&style->light[GTK_STATE_NORMAL]);
		cairo_move_to(cr, 210, 299);
		cairo_line_to(cr, 509, 299);
		cairo_line_to(cr, 509, 0);
		cairo_stroke(cr);

		cairo_set_antialias (cr,CAIRO_ANTIALIAS_DEFAULT);

		cairo_set_source_rgb(cr, 0.05, 0.05, 0.2);
		cairo_rectangle(cr, 211, 1, 318, 298 );
		cairo_fill(cr);
	}


	/* room display */
	/* 200x140 centered at 105,75 */

	cairo_set_source_rgb(cr, 0.05, 0.05, 0.2);
	cairo_rectangle(cr, 3, 3, 204, 144 );
	cairo_fill(cr);

	camera.x=4*l;		camera.y=h;		camera.z=4*w;
	lookat.x=0;		lookat.y=0;		lookat.z=0;
	source.x=sLR*w/2;	source.y=1.5-h/2;	source.z=sFB*h-(h/2);
	dest.x=dLR*w/2;		dest.y=1.5-h/2;		dest.z=dFB*h-(h/2);

	room[0].x=-w/2;	room[0].y=h/2;	room[0].z=-l/2;
	room[1].x=w/2;	room[1].y=h/2;	room[1].z=-l/2;
	room[2].x=w/2;	room[2].y=h/2;	room[2].z=l/2;
	room[3].x=-w/2;	room[3].y=h/2;	room[3].z=l/2;
	room[4].x=-w/2;	room[4].y=-h/2;	room[4].z=-l/2;
	room[5].x=w/2;	room[5].y=-h/2;	room[5].z=-l/2;
	room[6].x=w/2;	room[6].y=-h/2;	room[6].z=l/2;
	room[7].x=-w/2;	room[7].y=-h/2;	room[7].z=l/2;

	inv_display_err_screen(&source,&sourceS,&camera,&lookat);
	inv_display_err_screen(&dest,&destS,&camera,&lookat);

	sx=0;
	sy=0;
	for(i=0;i<8;i++) {
		inv_display_err_screen(&room[i],&roomS[i],&camera,&lookat);
		sx = fabs(roomS[i].x) > sx ? fabs(roomS[i].x) : sx;
		sy = fabs(roomS[i].y) > sy ? fabs(roomS[i].y) : sy;
	}

	sx = 93/sx;
	sy = 65/sy;

	scale = sx > sy ? sy : sx;

	//plot with scale
	cairo_set_antialias (cr,CAIRO_ANTIALIAS_DEFAULT);
	cairo_set_line_width(cr,2);
	cairo_set_source_rgb(cr, 0.0, 0.1, 1.0);

	cairo_move_to(cr,105+roomS[0].x*scale,75-roomS[0].y*scale);
	cairo_line_to(cr,105+roomS[1].x*scale,75-roomS[1].y*scale);
	cairo_line_to(cr,105+roomS[2].x*scale,75-roomS[2].y*scale);
	cairo_line_to(cr,105+roomS[3].x*scale,75-roomS[3].y*scale);
	cairo_line_to(cr,105+roomS[0].x*scale,75-roomS[0].y*scale);

	cairo_move_to(cr,105+roomS[4].x*scale,75-roomS[4].y*scale);
	cairo_line_to(cr,105+roomS[5].x*scale,75-roomS[5].y*scale);
	cairo_line_to(cr,105+roomS[6].x*scale,75-roomS[6].y*scale);
	cairo_line_to(cr,105+roomS[7].x*scale,75-roomS[7].y*scale);
	cairo_line_to(cr,105+roomS[4].x*scale,75-roomS[4].y*scale);

	cairo_move_to(cr,105+roomS[0].x*scale,75-roomS[0].y*scale);
	cairo_line_to(cr,105+roomS[4].x*scale,75-roomS[4].y*scale);

	cairo_move_to(cr,105+roomS[1].x*scale,75-roomS[1].y*scale);
	cairo_line_to(cr,105+roomS[5].x*scale,75-roomS[5].y*scale);

	cairo_move_to(cr,105+roomS[2].x*scale,75-roomS[2].y*scale);
	cairo_line_to(cr,105+roomS[6].x*scale,75-roomS[6].y*scale);

	cairo_move_to(cr,105+roomS[3].x*scale,75-roomS[3].y*scale);
	cairo_line_to(cr,105+roomS[7].x*scale,75-roomS[7].y*scale);

	cairo_stroke(cr);

	r=2;

	cairo_set_source_rgba(cr, 0.8, 0.8, 0.0, 0.2);
	cairo_arc(cr,105+sourceS.x*scale,75-sourceS.y*scale,r,0,2*INV_PI);
	cairo_fill(cr);
	
	cairo_set_source_rgba(cr, 0.0, 0.1, 1.0, 0.2);
	cairo_arc(cr,105+destS.x*scale,75-destS.y*scale,r,0,2*INV_PI);
	cairo_fill(cr);


	/* er display */
	/* 200x140 centered at 105,225 */

	cairo_set_source_rgb(cr, 0.05, 0.05, 0.2);
	cairo_rectangle(cr, 3, 153, 204, 144 );
	cairo_fill(cr);


	max_delay=0;
	max_gain=0;
	er_size=calculateIReverbER(er, MAX_ER, w, l, h, sLR, sFB, dLR, dFB, 1.5, diffusion, SPEED_OF_SOUND);
	printf("er size: %i %f\n",er_size,diffusion);
	for(i=0;i<er_size;i++) {
		max_delay=er->Delay > max_delay ? er->Delay : max_delay;
		max_gain=fabs(er->GainL) > max_gain ? fabs(er->GainL) : max_gain;
		max_gain=fabs(er->GainR) > max_gain ? fabs(er->GainR) : max_gain;
		er++;
	}
	max_delay= ((gint)(max_delay/25 )+2) * 25;
	er=INV_DISPLAY_ERR(widget)->er;

	cairo_set_antialias (cr,CAIRO_ANTIALIAS_DEFAULT);
	cairo_set_line_width(cr,1);

	cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.5);
	cairo_rectangle(cr, 5, 230, 200, 1 );
	cairo_fill(cr);

	for(i=0;i<er_size;i++) {

		x=5+200*(er->Delay/max_delay);
		y=1+fabs(er->GainL/max_gain)*65;

		cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 0.5);
		cairo_rectangle(cr, x,229, 1, -y );
		cairo_fill(cr);

		y=1+fabs(er->GainR/max_gain)*65;

		cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 0.5);
		cairo_rectangle(cr, x,231, 1, y );
		cairo_fill(cr);
		er++;
	}

	/* source dest display */
	/* 310x280 centered at 350,155 */

	cairo_set_source_rgb(cr, 0.05, 0.05, 0.2);
	cairo_rectangle(cr, 213, 3, 292, 292 );
	cairo_fill(cr);

	sw=pow(w,0.5);
	sl=pow(l,0.5);
	scale=9999999999999;
	scale = 280/sw < scale ? 280/sw : scale;
	scale = 270/sl < scale ? 270/sl : scale;

	sw=sw*scale/2;
	sl=sl*scale/2;
	
	cairo_set_antialias (cr,CAIRO_ANTIALIAS_DEFAULT);
	cairo_set_source_rgba(cr, 1.0, 1.0, 0.5, 0.05);
	cairo_rectangle(cr, 360-(sw*0.99), 155-(sl*0.99), sw*1.98, sl*0.98 );
	cairo_fill(cr);

	cairo_set_source_rgba(cr, 0.5, 0.6, 1.0, 0.05);
	cairo_rectangle(cr, 360-(sw*0.99), 155+(sl*0.01), sw*1.98, sl*0.98 );
	cairo_fill(cr);

	cairo_set_antialias (cr,CAIRO_ANTIALIAS_NONE);
	cairo_set_line_width(cr,1);
	cairo_set_source_rgb(cr, 0.35, 0.35, 0.35);
	cairo_rectangle(cr, 360-sw, 155-sl, sw*2, sl*2 );
	cairo_stroke(cr);
	
	cairo_set_antialias (cr,CAIRO_ANTIALIAS_DEFAULT);

	xc=360.0 + sLR*sw;
	yc=155+sl - sFB*sl*2;
	r=3;

	cairo_set_source_rgb(cr, 0.8, 0.8, 0.0);
	cairo_arc(cr,xc,yc,r,0,2*INV_PI);
	cairo_fill(cr);

	cairo_set_source_rgb(cr, 0.5, 0.5, 0.3);
	cairo_select_font_face(cr,"monospace",CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cr,8);
	strcpy(label,"Source");
	cairo_text_extents (cr,label,&extents);
	if(l<w) {
		// horz
		if(sLR<0.0) {
			cairo_move_to(cr,xc-7-extents.width,yc+extents.height/2);
		} else {
			cairo_move_to(cr,xc+7,yc+extents.height/2);
		}
	} else {
		// vert
		if(sFB<0.75) {
			cairo_move_to(cr,xc-extents.width/2,yc-6);
		} else {
			cairo_move_to(cr,xc-extents.width/2,yc+5+extents.height);
		}
	}
	cairo_show_text(cr,label);

	xc=360.0 + dLR*sw;
	yc=155+sl - dFB*sl*2;
	r=3;

	cairo_set_source_rgb(cr, 0.0, 0.1, 1.0);
	cairo_arc(cr,xc,yc,r,0,2*INV_PI);
	cairo_fill(cr);

	cairo_set_source_rgb(cr, 0.3, 0.3, 0.6);
	cairo_select_font_face(cr,"monospace",CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cr,8);
	strcpy(label,"Listener");
	cairo_text_extents (cr,label,&extents);
	if(l<w) {
		// horz
		if(dLR<0.0) {
			cairo_move_to(cr,xc+7,yc-1+extents.height/2);
		} else {
			cairo_move_to(cr,xc-7-extents.width,yc-1+extents.height/2);
		}
	} else {
		// vert
		if(dFB<0.25) {
			cairo_move_to(cr,xc-extents.width/2,yc-6);
		} else {
			cairo_move_to(cr,xc-extents.width/2,yc+5+extents.height);
		}
	}
	cairo_show_text(cr,label);

  	cairo_destroy(cr);
}


static void
inv_display_err_destroy(GtkObject *object)
{
	InvDisplayErr *displayErr;
	InvDisplayErrClass *klass;

	g_return_if_fail(object != NULL);
	g_return_if_fail(INV_IS_DISPLAY_ERR(object));

	displayErr = INV_DISPLAY_ERR(object);
	free(displayErr->er);

	klass = gtk_type_class(gtk_widget_get_type());

	if (GTK_OBJECT_CLASS(klass)->destroy) {
		(* GTK_OBJECT_CLASS(klass)->destroy) (object);
	}
}

static void	
inv_display_err_screen(struct point3D *point, struct point2D *screen, struct point3D *camera, struct point3D *lookat)
{

	struct point3D 	CameraLookV;		// a vector describing where the camera is looking
	struct point3D 	ScreenXP;		// the x dimension of the plane the image is projected onto
	struct point3D 	ScreenYP;		// the y dimension of the plane the image is projected onto
	struct point3D 	CameraPointV;		// a vector describing where the point is in relation to the camera
	struct point3D 	ProjectedP;		// the location of the projected point on the x/y screen
	struct point3D 	ProjectedPCenter;	// the location of the projected point on the x/y screen relative to the lookat point
	float		CameraLookVLengthSqr,CameraLookVLength;
	float		ScreenXPLength;
	float		ScreenYPLength;
	float		d,t;

	//Establish the Vector Components of the Camera-to-Center Vector
	CameraLookV.x = (lookat->x - camera->x);
	CameraLookV.y = (lookat->y - camera->y);
	CameraLookV.z = (lookat->z - camera->z);
    
    	CameraLookVLengthSqr = CameraLookV.x*CameraLookV.x + CameraLookV.y*CameraLookV.y + CameraLookV.z*CameraLookV.z;
    	CameraLookVLength = pow(CameraLookVLengthSqr,0.5);
    
	// x plane of the projection
	ScreenXP.x = -CameraLookV.z;
	ScreenXP.y = 0;
	ScreenXP.z = CameraLookV.x;

    	ScreenXPLength = pow(ScreenXP.x*ScreenXP.x + ScreenXP.y*ScreenXP.y + ScreenXP.z*ScreenXP.z, 0.5);

	ScreenXP.x = ScreenXP.x / ScreenXPLength;
	ScreenXP.y = ScreenXP.y / ScreenXPLength;
	ScreenXP.z = ScreenXP.z / ScreenXPLength;

	// y plane of the projection
	ScreenYP.x = -(ScreenXP.z * CameraLookV.y);
	ScreenYP.y = (ScreenXP.z * CameraLookV.x) - (ScreenXP.x * CameraLookV.z);
	ScreenYP.z = (ScreenXP.x * CameraLookV.y);

	ScreenYPLength = pow(ScreenYP.x*ScreenYP.x + ScreenYP.y*ScreenYP.y + ScreenYP.z*ScreenYP.z, 0.5);

	ScreenYP.x = ScreenYP.x / ScreenYPLength;
	ScreenYP.y = ScreenYP.y / ScreenYPLength;
	ScreenYP.z = ScreenYP.z / ScreenYPLength;

	// scale by the camera distance
	ScreenXP.x = ScreenXP.x / CameraLookVLength;
	ScreenXP.y = ScreenXP.y / CameraLookVLength;
	ScreenXP.z = ScreenXP.z / CameraLookVLength;

	ScreenYP.x = ScreenYP.x / CameraLookVLength;
	ScreenYP.y = ScreenYP.y / CameraLookVLength;
	ScreenYP.z = ScreenYP.z / CameraLookVLength;

	// point -> camera vector
	CameraPointV.x = point->x - camera->x;
	CameraPointV.y = point->y - camera->y;
	CameraPointV.z = point->z - camera->z;

	// find where the vector intesects the projection plane
	d = (CameraPointV.x * CameraLookV.x) + (CameraPointV.y * CameraLookV.y) + (CameraPointV.z * CameraLookV.y);
	t = CameraLookVLengthSqr / d;

	ProjectedP.x = camera->x + CameraPointV.x*t;
	ProjectedP.y = camera->y + CameraPointV.y*t;
	ProjectedP.z = camera->z + CameraPointV.z*t ;

	//convert relative to lookat point
	ProjectedPCenter.x = (ProjectedP.x - lookat->x);
	ProjectedPCenter.y = (ProjectedP.y - lookat->y);
	ProjectedPCenter.z = (ProjectedP.z - lookat->z);

	// find screen coordinates
	screen->x = (ProjectedPCenter.x*ScreenXP.x) + (ProjectedPCenter.y*ScreenXP.y) + (ProjectedPCenter.z*ScreenXP.z);
	screen->y = (ProjectedPCenter.x*ScreenYP.x) + (ProjectedPCenter.y*ScreenYP.y) + (ProjectedPCenter.z*ScreenYP.z);

//	printf("point: x %f, y %f, z %f,   screen: x %f, y %f\n",point->x,point->y,point->z,screen->x,screen->y);


}

