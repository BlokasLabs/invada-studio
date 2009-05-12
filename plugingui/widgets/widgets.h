#ifndef __INV_WIDGETS_H
#define __INV_WIDGETS_H

#define INV_PLUGIN_ACTIVE 0
#define INV_PLUGIN_BYPASS 1

#define INV_PI 3.1415926535

struct point2D {
	float x; 
	float y;
};

struct point3D {
	float x; 
	float y;
	float z;
};

struct colour {
	float R; 
	float G;
	float B;
};

#endif /* __INV_WIDGETS_H */
