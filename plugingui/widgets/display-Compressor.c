
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "display-Compressor.h"



static void 	inv_display_comp_class_init(InvDisplayCompClass *klass);
static void 	inv_display_comp_init(InvDisplayComp *displayComp);
static void 	inv_display_comp_size_request(GtkWidget *widget, GtkRequisition *requisition);
static void 	inv_display_comp_size_allocate(GtkWidget *widget, GtkAllocation *allocation);
static void 	inv_display_comp_realize(GtkWidget *widget);
static gboolean inv_display_comp_expose(GtkWidget *widget,GdkEventExpose *event);
static void 	inv_display_comp_paint(GtkWidget *widget, gint mode);
static void	inv_display_comp_destroy(GtkObject *object);
float 		inv_display_comp_rms_waveform(float pos, float width, float height);


GtkType
inv_display_comp_get_type(void)
{
	static GtkType inv_display_comp_type = 0;
	char *name;
	int i;


	if (!inv_display_comp_type) 
	{
		static const GTypeInfo type_info = {
			sizeof(InvDisplayCompClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc)inv_display_comp_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof(InvDisplayComp),
			0,    /* n_preallocs */
			(GInstanceInitFunc)inv_display_comp_init
		};
		for (i = 0; ; i++) {
			name = g_strdup_printf("InvDisplayComp-%p-%d",inv_display_comp_class_init, i);
			if (g_type_from_name(name)) {
				free(name);
				continue;
			}
			inv_display_comp_type = g_type_register_static(GTK_TYPE_WIDGET,name,&type_info,(GTypeFlags)0);
			free(name);
			break;
		}
	}
	return inv_display_comp_type;
}

void
inv_display_comp_set_rms(InvDisplayComp *displayComp, float num)
{
	if(num<0)
		displayComp->rms = 0;
	else if (num <= 1)
		displayComp->rms = num;
	else displayComp->rms = 1;
	if(GTK_WIDGET_REALIZED(displayComp))
		inv_display_comp_paint(GTK_WIDGET(displayComp),INV_DISPLAYCOMP_DRAW_DATA);
}

void
inv_display_comp_set_attack(InvDisplayComp *displayComp, float num)
{
	if(num<0.00001)
		displayComp->attack = 0.00001;
	else if (num <= 0.750)
		displayComp->attack = num;
	else displayComp->attack = 0.750;
	if(GTK_WIDGET_REALIZED(displayComp))
		inv_display_comp_paint(GTK_WIDGET(displayComp),INV_DISPLAYCOMP_DRAW_DATA);
}

void
inv_display_comp_set_release(InvDisplayComp *displayComp, float num)
{
	if(num<0.001)
		displayComp->release = 0.001;
	else if (num <= 5.0)
		displayComp->release = num;
	else displayComp->release = 5.0;
	if(GTK_WIDGET_REALIZED(displayComp))
		inv_display_comp_paint(GTK_WIDGET(displayComp),INV_DISPLAYCOMP_DRAW_DATA);
}

void
inv_display_comp_set_threshold(InvDisplayComp *displayComp, float num)
{
	if(num<-36.0)
		displayComp->threshold = -36.0;
	else if (num <= 0.0)
		displayComp->threshold = num;
	else displayComp->threshold = 0.0;
	if(GTK_WIDGET_REALIZED(displayComp))
		inv_display_comp_paint(GTK_WIDGET(displayComp),INV_DISPLAYCOMP_DRAW_DATA);
}

void
inv_display_comp_set_ratio(InvDisplayComp *displayComp, float num)
{
	if(num<1.0)
		displayComp->ratio = 1.0;
	else if (num <= 20.0)
		displayComp->ratio = num;
	else displayComp->ratio = 20.0;
	if(GTK_WIDGET_REALIZED(displayComp))
		inv_display_comp_paint(GTK_WIDGET(displayComp),INV_DISPLAYCOMP_DRAW_DATA);
}

void
inv_display_comp_set_gain(InvDisplayComp *displayComp, float num)
{
	if(num<-6.0)
		displayComp->gain = -6.0;
	else if (num <= 36.0)
		displayComp->gain = num;
	else displayComp->gain = 36.0;
	if(GTK_WIDGET_REALIZED(displayComp))
		inv_display_comp_paint(GTK_WIDGET(displayComp),INV_DISPLAYCOMP_DRAW_DATA);
}


GtkWidget * inv_display_comp_new()
{
	return GTK_WIDGET(gtk_type_new(inv_display_comp_get_type()));
}


static void
inv_display_comp_class_init(InvDisplayCompClass *klass)
{
	GtkWidgetClass *widget_class;
	GtkObjectClass *object_class;


	widget_class = (GtkWidgetClass *) klass;
	object_class = (GtkObjectClass *) klass;

	widget_class->realize = inv_display_comp_realize;
	widget_class->size_request = inv_display_comp_size_request;
	widget_class->size_allocate = inv_display_comp_size_allocate;
	widget_class->expose_event = inv_display_comp_expose;

	object_class->destroy = inv_display_comp_destroy;
}


static void
inv_display_comp_init(InvDisplayComp *displayComp)
{
	displayComp->rms=0.5;
	displayComp->attack=0.00001;
	displayComp->release=0.001;
	displayComp->threshold=0.0;
	displayComp->ratio=1.0;
	displayComp->gain=0.0;

	displayComp->Lastrms=displayComp->rms;
	displayComp->Lastattack=displayComp->attack;
	displayComp->Lastrelease=displayComp->release;
	displayComp->Lastthreshold=displayComp->threshold;
	displayComp->Lastratio=displayComp->ratio;
	displayComp->Lastgain=displayComp->gain;
}


static void
inv_display_comp_size_request(GtkWidget *widget,
    GtkRequisition *requisition)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(INV_IS_DISPLAY_COMP(widget));
	g_return_if_fail(requisition != NULL);

	requisition->width = 600;
	requisition->height = 156;
}


static void
inv_display_comp_size_allocate(GtkWidget *widget,
    GtkAllocation *allocation)
{
	g_return_if_fail(widget != NULL);
	g_return_if_fail(INV_IS_DISPLAY_COMP(widget));
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
inv_display_comp_realize(GtkWidget *widget)
{
	GdkWindowAttr attributes;
	guint attributes_mask;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(INV_IS_DISPLAY_COMP(widget));

	GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
	attributes.width = 600;
	attributes.height = 156;

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
inv_display_comp_expose(GtkWidget *widget, GdkEventExpose *event)
{
	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(INV_IS_DISPLAY_COMP(widget), FALSE);
	g_return_val_if_fail(event != NULL, FALSE);

	inv_display_comp_paint(widget,INV_DISPLAYCOMP_DRAW_ALL);

	return FALSE;
}


static void
inv_display_comp_paint(GtkWidget *widget, gint mode)
{
	float 		rms,rmsC,rmsV;
	float 		attack;
	float 		release;
	float 		threshold;
	float 		ratio;
	float 		gain;	

	gint		i;
	float		y,threshsig;
	cairo_t 	*cr;
	GtkStyle	*style;
	char		label[50];

	style = gtk_widget_get_style(widget);
	rms=INV_DISPLAY_COMP(widget)->rms;
	attack=INV_DISPLAY_COMP(widget)->attack;
	release=INV_DISPLAY_COMP(widget)->release;
	threshold=INV_DISPLAY_COMP(widget)->threshold;
	ratio=INV_DISPLAY_COMP(widget)->ratio;
	gain=INV_DISPLAY_COMP(widget)->gain;

	cr = gdk_cairo_create(widget->window);

	if(mode==INV_DISPLAYCOMP_DRAW_ALL) {

		cairo_set_line_join (cr, CAIRO_LINE_JOIN_MITER);
		cairo_set_antialias (cr,CAIRO_ANTIALIAS_NONE);
		cairo_set_line_width(cr,1);

		for(i=0;i<3;i++) {
			gdk_cairo_set_source_color(cr,&style->dark[GTK_STATE_NORMAL]);
			cairo_move_to(cr, (i*200), 155);
			cairo_line_to(cr, (i*200), 0);
			cairo_line_to(cr, (i*200)+199, 0);
			cairo_stroke(cr);

			gdk_cairo_set_source_color(cr,&style->light[GTK_STATE_NORMAL]);
			cairo_move_to(cr, (i*200), 155);
			cairo_line_to(cr, (i*200)+199, 155);
			cairo_line_to(cr, (i*200)+199, 0);
			cairo_stroke(cr);

			cairo_set_antialias (cr,CAIRO_ANTIALIAS_DEFAULT);
			cairo_new_path(cr);

			cairo_set_source_rgb(cr, 0.05, 0.05, 0.2);
			cairo_rectangle(cr, (i*200)+1, 1, 198, 154 );
			cairo_fill(cr);
		}

		/* detector labels */
		cairo_select_font_face(cr,"monospace",CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_source_rgb(cr, 0.7, 0.7, 0.7);
		
		cairo_set_font_size(cr,10);
		sprintf(label,"Detector");
		cairo_move_to(cr,75,12);
		cairo_show_text(cr,label);

		cairo_set_font_size(cr,9);
		sprintf(label,"Audio");
		cairo_move_to(cr,35,152);
		cairo_show_text(cr,label);

		sprintf(label,"Detected Signal");
		cairo_move_to(cr,95,152);
		cairo_show_text(cr,label);

		cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.4);
		cairo_set_line_width(cr,1.0);
		cairo_move_to(cr, 25, 152);
		cairo_line_to(cr, 31, 146);
		cairo_stroke(cr);

		cairo_set_source_rgb(cr, 0.0, 0.1, 1.0);
		cairo_rectangle(cr, 85, 146, 6, 6 );
		cairo_fill(cr);

		/* envelope labels */
		cairo_select_font_face(cr,"monospace",CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_source_rgb(cr, 0.7, 0.7, 0.7);
		
		cairo_set_font_size(cr,10);
		sprintf(label,"Envelope");
		cairo_move_to(cr,275,12);
		cairo_show_text(cr,label);

		/* compressor labels */
		cairo_select_font_face(cr,"monospace",CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_source_rgb(cr, 0.7, 0.7, 0.7);
		
		cairo_set_font_size(cr,10);
		sprintf(label,"Compressor");
		cairo_move_to(cr,470,12);
		cairo_show_text(cr,label);


		cairo_set_source_rgb(cr, 1, 1, 1);
		cairo_rectangle(cr, 566, 17, 1, 113);
		cairo_fill(cr);

		cairo_rectangle(cr, 406, 129, 160, 1);
		cairo_fill(cr);

		cairo_select_font_face(cr,"monospace",CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size(cr,8);
		cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
		for(i=0;i<8;i+=2)
		{
			sprintf(label,"%3idB",-(i*6));
			cairo_move_to(cr,569,34+(i*14));
			cairo_show_text(cr,label);

			sprintf(label,"%idB",-(i*6));
			switch(i) {
				case 0:
					cairo_move_to(cr,540-(i*20),141);
					break;
				case 2:
				case 4:
				case 6:
					cairo_move_to(cr,535-(i*20),141);
					break;
			}
			cairo_show_text(cr,label);
		}

		cairo_set_source_rgb(cr, 0.7, 0.7, 0.7);
		cairo_set_font_size(cr,9);
		sprintf(label,"Original");
		cairo_move_to(cr,435,152);
		cairo_show_text(cr,label);

		sprintf(label,"Compressed");
		cairo_move_to(cr,515,152);
		cairo_show_text(cr,label);

		cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.4);
		cairo_set_line_width(cr,1.0);
		cairo_move_to(cr, 425, 152);
		cairo_line_to(cr, 431, 146);
		cairo_stroke(cr);

		cairo_set_source_rgb(cr, 0.0, 0.1, 1.0);
		cairo_set_line_width(cr,2.0);
		cairo_move_to(cr, 505, 152);
		cairo_line_to(cr, 511, 146);
		cairo_stroke(cr);



	}

	/* rms display */
	if(mode == INV_DISPLAYCOMP_DRAW_ALL 
	|| rms  != INV_DISPLAY_COMP(widget)->Lastrms ) {
		cairo_set_source_rgb(cr, 0.05, 0.05, 0.2);
		cairo_rectangle(cr, 3, 13, 194, 129 );
		cairo_fill(cr);

		rmsC= (pow(rms,3) * 400)+1; 
		rmsV=0;

		cairo_set_source_rgb(cr, 0.0, 0.1, 1.0);
		cairo_set_line_width(cr,1.0);
		cairo_move_to(cr, 4, 77);
			
		for (i=0;i<386;i++) {
			y=inv_display_comp_rms_waveform((float) i, 386, 64);
			rmsV = sqrt(( (rmsC-1)*rmsV*rmsV + y*y ) / rmsC); 
			cairo_line_to(cr, 4+((float)i/2), 77-rmsV);
		}
		cairo_line_to(cr, 197, 77);
		cairo_line_to(cr, 4, 77);
		cairo_fill(cr);

		cairo_set_source_rgb(cr, 0.35, 0.35, 35);
		cairo_rectangle(cr, 4, 77, 192, 1 );
		cairo_fill(cr);

		cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.25);
		cairo_set_line_width(cr,1.0);
		cairo_move_to(cr, 4, 77);
			
		for (i=0;i<386;i++) {
			y=inv_display_comp_rms_waveform((float) i, 386, 64);
			cairo_line_to(cr, 4+((float)i/2), 77-y);
		}
		cairo_stroke(cr);
	}
	
	/* compressor display */
	if(mode      == INV_DISPLAYCOMP_DRAW_ALL 
	|| threshold != INV_DISPLAY_COMP(widget)->Lastthreshold 
	|| ratio     != INV_DISPLAY_COMP(widget)->Lastratio 
	|| gain      != INV_DISPLAY_COMP(widget)->Lastgain ) {

		cairo_set_source_rgb(cr, 0.05, 0.05, 0.2);
		cairo_rectangle(cr, 406, 17, 160, 112 );
		cairo_fill(cr);
		for(i=1;i<8;i+=2) {
			cairo_set_source_rgb(cr, 0.20, 0.20, 0.20);
			cairo_rectangle(cr, 386+(i*20), 17, 1, 112);
			cairo_fill(cr);
			cairo_rectangle(cr, 406, 3+(i*14), 160, 1);
			cairo_fill(cr);
		}
		for(i=0;i<7;i+=2) {
			cairo_set_source_rgb(cr, 0.35, 0.35, 0.35);
			cairo_rectangle(cr, 426+(i*20), 17, 1, 112);
			cairo_fill(cr);
			cairo_rectangle(cr, 406, 31+(i*14), 160, 1);
			cairo_fill(cr);
		}

		cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
		cairo_rectangle(cr, 546, 17, 1, 112);
		cairo_fill(cr);
		cairo_rectangle(cr, 406, 31, 160, 1);
		cairo_fill(cr);

		cairo_rectangle(cr, 406, 17, 160, 112 );
		cairo_clip(cr);

		/* compressed signal */

		// gain change at +6 db
		threshsig=pow(10,threshold/20);
		y = 20*log10(threshsig+((2-threshsig)/ratio));

		cairo_set_source_rgb(cr, 0.0, 0.1, 1.0);
		cairo_set_line_width(cr,2);
		cairo_move_to(cr, 406                 , 129-(14*gain/6));
		cairo_line_to(cr, 546+(20*threshold/6), 31-(14*gain/6)-(14*threshold/6));
		cairo_line_to(cr, 566                 , 31-(14*gain/6)-(14*y/6)); 
		cairo_stroke(cr);

		/* original signal */
		cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.25);
		cairo_set_line_width(cr,1.0);
		cairo_move_to(cr, 406, 129);
		cairo_line_to(cr, 566, 17);
		cairo_stroke(cr);



	}
  	cairo_destroy(cr);

	INV_DISPLAY_COMP(widget)->Lastrms=rms;
	INV_DISPLAY_COMP(widget)->Lastattack=attack;
	INV_DISPLAY_COMP(widget)->Lastrelease=release;
	INV_DISPLAY_COMP(widget)->Lastthreshold=threshold;
	INV_DISPLAY_COMP(widget)->Lastratio=ratio;
	INV_DISPLAY_COMP(widget)->Lastgain=gain;

}


static void
inv_display_comp_destroy(GtkObject *object)
{
	InvDisplayComp *displayComp;
	InvDisplayCompClass *klass;

	g_return_if_fail(object != NULL);
	g_return_if_fail(INV_IS_DISPLAY_COMP(object));

	displayComp = INV_DISPLAY_COMP(object);

	klass = gtk_type_class(gtk_widget_get_type());

	if (GTK_OBJECT_CLASS(klass)->destroy) {
		(* GTK_OBJECT_CLASS(klass)->destroy) (object);
	}
}

float 
inv_display_comp_rms_waveform(float pos, float width, float height)
{
  	float theta,theta2;
	float heightr,heightr2;
	float value;
	
	value=0;

	if(pos < width/2) {
		theta=2*INV_PI*(13.5*pow(2*pos/width,0.5));
		heightr=1-(pow(2*pos/width,0.1));
		value+=3*height*heightr*sin(theta);
	}

	if(pos > width/2) {
		theta=2*INV_PI*(5* 2* (pos-width/2) / width );
		theta2=2*INV_PI*(20* 2* (pos-width/2) / width );
		heightr=1-(pow(2*(pos-width/2)/width,20));
		heightr2=1-(pow(2*(pos-width/2)/width,0.5));
		value+=0.6*height*heightr*sin(theta) + 0.2*height*heightr2*sin(theta2);
	}
	return value;
}

