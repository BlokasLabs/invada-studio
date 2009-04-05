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

#define IFILTER_LPF_GUI_URI	"http://invadarecords.com/plugins/lv2/filter/lpf/gui";
#define IFILTER_MONO_LPF_URI	"http://invadarecords.com/plugins/lv2/filter/lpf/mono";
#define IFILTER_STEREO_LPF_URI	"http://invadarecords.com/plugins/lv2/filter/lpf/stereo";
#define IFILTER_HPF_GUI_URI	"http://invadarecords.com/plugins/lv2/filter/hpf/gui";
#define IFILTER_MONO_HPF_URI	"http://invadarecords.com/plugins/lv2/filter/hpf/mono";
#define IFILTER_STEREO_HPF_URI	"http://invadarecords.com/plugins/lv2/filter/hpf/stereo";
#define IFILTER_FREQ 		0
#define IFILTER_GAIN 		1
#define IFILTER_NOCLIP 		2
#define IFILTER_AUDIO_INL  	3
#define IFILTER_AUDIO_OUTL 	4
#define IFILTER_AUDIO_INR  	5 /* not used in mono mode */
#define IFILTER_AUDIO_OUTR 	6 /* not used in mono mode */


/* control conversion function */
float convertParam(unsigned long param, float value, double sr);

