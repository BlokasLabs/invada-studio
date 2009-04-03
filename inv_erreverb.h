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

#define REVERB_SPACE_SIZE 2   /* size in seconds */
#define MAX_ER            100 /* maximun number of early reflections to calculate */
#define SPEED_OF_SOUND    330 /* speed of sound in air in meters/second */
#define OBJECT_HEIGHT     1.5 /* the height of the sound source and the listener */

struct ERunit {
	int Active;
	unsigned long Delay;
	unsigned int Reflections;
	float AbsGain;
	float GainL;
	float GainR;
};

/* works out the reflection details */
void calculateIReverbER(LV2_Handle instance);

/* works out a single reflection */
void calculateSingleIReverbER(struct ERunit * er, float Width, float Length, float Height, int Phase, unsigned int reflections, float DDist, double sr);

/* control conversion function */
float convertParam(unsigned long param, float value, double sr);

