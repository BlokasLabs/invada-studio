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

#define IINPUT_URI		"http://invadarecords.com/plugins/lv2/input";
#define IINPUT_GUI_URI		"http://invadarecords.com/plugins/lv2/input/gui";
#define IINPUT_PHASEL		0
#define IINPUT_PHASER		1
#define IINPUT_GAIN		2
#define IINPUT_PAN		3
#define IINPUT_WIDTH		4
#define IINPUT_NOCLIP		5
#define IINPUT_AUDIO_INL	6
#define IINPUT_AUDIO_INR	7
#define IINPUT_AUDIO_OUTL	8
#define IINPUT_AUDIO_OUTR	9
#define IINPUT_METER_INL  	10
#define IINPUT_METER_INR  	11  
#define IINPUT_METER_OUTL 	12
#define IINPUT_METER_OUTR 	13 
#define IINPUT_METER_PHASE 	14 
#define IINPUT_METER_DRIVE 	15 

/* control conversion function */
float convertParam(unsigned long param, float value, double sr);


