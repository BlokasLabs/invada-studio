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


#define IDELAY_GUI_URI		"http://invadarecords.com/plugins/lv2/delay/gui"
#define IDELAY_MONO_URI		"http://invadarecords.com/plugins/lv2/delay/mono"
#define IDELAY_SUM_URI		"http://invadarecords.com/plugins/lv2/delay/sum"
#define IDELAY_BYPASS		0
#define IDELAY_MUNGEMODE	1
#define IDELAY_MUNGE 		2
#define IDELAY_1_DELAY		3
#define IDELAY_1_PAN 		4
#define IDELAY_1_FB 		5
#define IDELAY_2_DELAY		6
#define IDELAY_2_PAN 		7
#define IDELAY_2_FB 		8
#define IDELAY_METER_INL   	9
#define IDELAY_METER_OUTL 	10
#define IDELAY_METER_OUTR 	11
#define IDELAY_AUDIO_INL  	12
#define IDELAY_AUDIO_OUTL 	13
#define IDELAY_AUDIO_OUTR 	14 
#define IDELAY_AUDIO_INR  	15   /* not used in mono in mode */ 

#define IDELAY_SPACE_SIZE 2500   /* size in milli seconds */

/* control conversion function */
float convertParam(unsigned long param, float value, double sr);

/* munge conversion */
float convertMunge(unsigned int mode, float munge, double sr);

