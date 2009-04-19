/* 

    Common functions

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

#include <math.h>
#include <lv2.h>
#include "common.h"


/* a function that checks to see if a control has been changed and calls the provided conversion fuction */
void checkParamChange(
	unsigned long param, 
	float * control, 
	float * last, 
	float * converted, 
	double sr,
	float (*ConvertFunction)(unsigned long, float, double)
	) 
{
	if(*control != *last) {
		*last=*control;
		*converted=(*ConvertFunction)(param, *control, sr);
	}
}


/* a function which maps an envelope to a sound */
float IEnvelope(float value, float envelope, int mode, double sr)
{
	float valueA;
	float EnvelopeDelta;

	valueA=fabs(value);

	switch(mode) 
	{
		case INVADA_METER_VU:
			/* 300ms attack, 300ms decay */
			EnvelopeDelta=(valueA > envelope)   ? 0.00005 * (valueA - envelope)   : 0.00005 * (valueA - envelope);
			break;
		case INVADA_METER_PEAK:  
			/* 50us attack, 100ms decay */
			EnvelopeDelta=(valueA > envelope)   ? 0.3 * (valueA - envelope)   : 0.00015 * (valueA - envelope);
			break;

		case INVADA_METER_PHASE:  
			/* 10ms attack, 10ms decay */
			EnvelopeDelta=(valueA > envelope)   ? 0.0015 * (valueA - envelope)   : 0.0015 * (valueA - envelope);
			break;
		case INVADA_METER_LAMP:  
			/* 10ms attack, 100ms decay */
			EnvelopeDelta=(valueA > envelope)   ? 0.0015 * (valueA - envelope)   : 0.00005 * (valueA - envelope);
			break;
		default:
			EnvelopeDelta=0;
	}
	return EnvelopeDelta;
}


/* this function is linear between -0.7 & 0.7 (approx -3db) and returns a value bewteen 0.7 and 1 for an input from 0.7 to infinity */
float InoClip(float in, float * drive)
{
	float out; 
	if ( fabs(in) < 0.7 ) {
	  	out = in;
		*drive=0;
	} else { 
	  out = (in>0) ? 
	            (  0.7 + 0.3 * (1-pow(2.718281828, 3.33333333*(0.7-in)))):
	            ( -0.7 - 0.3 * (1-pow(2.718281828, 3.33333333*(0.7+in))));
		*drive=fabs(in) - fabs(out);
	}
	return out;
}

/* distortion function based on sin() */
float ITube_do(float in, float Drive)
{
	float out;
	out = (in>0) ? 
		 pow( fabs(sin( in*Drive*PI_ON_2)),ITUBE_MAGIC ) : 
		-pow( fabs(sin(-in*Drive*PI_ON_2)),ITUBE_MAGIC ) ;
	return out;
}


