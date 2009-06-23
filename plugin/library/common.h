/* 

    (c) Fraser Stuart 2009

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifndef __INVADA_COMMON_H
#define __INVADA_COMMON_H

/* 2.0 * atan(1.0) */
#define PI_ON_2 1.570796327

/* PI */
#define PI 3.1415926535877932

/* 2*PI */
#define PI_2 6.2831853071795864

/* -1.0 / log10(sin(0.2 * atan(1.0))) */
#define ITUBE_MAGIC 1.241206735

/*2^31-2 */
#define TWO31_MINUS2 2147483646

/* maximun number of early reflections to calculate */
#define MAX_ER            100 

/* speed of sound in air in meters/second */
#define SPEED_OF_SOUND    330 

#define INVADA_METER_VU 0
#define INVADA_METER_PEAK 1
#define INVADA_METER_PHASE 2
#define INVADA_METER_LAMP 3

struct ERunit {
	int Active;
	float rand;
	float DelayActual;
	float DelayOffset;
	unsigned long Delay;
	unsigned int Reflections;
	float AbsGain;
	float GainL;
	float GainR;
};

/* param change detect functions, second one calculates deltas for interpolation in run loops */
void checkParamChange(unsigned long param, float * control, float * last, float * converted, double sr, float (*ConvertFunction)(unsigned long, float, double));
float  getParamChange(unsigned long param, float * control, float * last, float * converted, double sr, float (*ConvertFunction)(unsigned long, float, double));

/* audio envelope */
float IEnvelope(float value, float envelope, int mode, double sr);

/* soft clipping function */
float InoClip(float in, float *drive);

/* distortion function */
float ITube_do(float in, float Drive);

/* works out a single er reflection */
void calculateSingleIReverbER(struct ERunit * er, float Width, float Length, float Height, int Phase, unsigned int reflections, float DDist, double sr);

/* works out all er reflections */
int calculateIReverbER(struct ERunit *erarray, int erMax, 
			float width, float length, float height, 
			float sourceLR, float sourceFB, 
			float destLR, float destFB, float objectHeight, 
			float diffusion,
			double sr);

#endif /*__INVADA_COMMON_H */
