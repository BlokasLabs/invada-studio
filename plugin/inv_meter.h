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

#define IMETER_URI		"http://invadarecords.com/plugins/lv2/meter";
#define IMETER_GUI_URI		"http://invadarecords.com/plugins/lv2/meter/gui";
#define IMETER_BYPASS		0
#define IMETER_AUDIO_INL	1
#define IMETER_AUDIO_INR	2
#define IMETER_AUDIO_OUTL	3
#define IMETER_AUDIO_OUTR	4
#define IMETER_METER_PEAK_L  	5
#define IMETER_METER_PEAK_R  	6  
#define IMETER_METER_VU_L 	7
#define IMETER_METER_VU_R 	8 
#define IMETER_METER_PHASE 	9 


/* control conversion function */
float convertParam(unsigned long param, float value, double sr);


