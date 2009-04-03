/* 

    This LADSPA plugin provides an early reflection reverb from a mono source

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


    Some Notes:
    ===========
    This plugin is not your classic sort of reverb that makes long washy spaces.
    It simulates a room by calculating the early reflections that occur off the walls/roof etc.

    It's useful for:
    a) putting dry signals in a 'natural' space so they sit with sounds recorded with 'room' anbience in them.
    b) thickening strings, vocals etc without softening or washing out.
    c) as a singal preprocess for reverb that does not have early reflections.
    d) accurate stereo placement by setting the 'source' LR the same as any panning the dry signle has. This is 
    because the early reflections from the off-center single reinforce the location of the sound rather than 
    contradicting it as a normal reverb would do. 

    Parameter description:
    RoomLength,RoomWidth,RoomHeight - the dimensions (in meters) of the room
    SourceLR, SourceFB (FB=Front/Back) - where the sound source is in the room (always the back half)
    DestLR, DestFB (FB=Front/Back) - where the destination (or listener) is in the room (always the front half)
    HPF - roll off some bottom end
    Warmth - roll off top end (amount depends on reflection count)
    Diffusion - makes the relections less perfect to simulate objects in the room.

*/

#include <stdlib.h> 
#include <string.h>
#include <math.h>
#include <lv2.h>
#include "libinv_common.h"
#include "inv_erreverb.h"


#define IERR_MONO_URI	"http://invadarecords.com/plugins/lv2/erreverb/mono";
#define IERR_SUM_URI	"http://invadarecords.com/plugins/lv2/erreverb/sum";
#define IERR_ROOMLENGTH	0
#define IERR_ROOMWIDTH 	1
#define IERR_ROOMHEIGHT	2
#define IERR_SOURCELR 	3
#define IERR_SOURCEFB 	4
#define IERR_DESTLR 	5
#define IERR_DESTFB 	6
#define IERR_HPF 	7
#define IERR_WARMTH 	8
#define IERR_DIFFUSION 	9
#define IERR_AUDIO_OUTL 10
#define IERR_AUDIO_OUTR 11 
#define IERR_AUDIO_INL  12
#define IERR_AUDIO_INR  13   /* not used in mono in mode */ 


static LV2_Descriptor *IReverbERMonoDescriptor = NULL;
static LV2_Descriptor *IReverbERSumDescriptor = NULL;


typedef struct {

	/* Ports */
	float * ControlRoomLength;
	float * ControlRoomWidth; 
	float * ControlRoomHeight;
	float * ControlSourceLR;
	float * ControlSourceFB;
	float * ControlDestLR; 
	float * ControlDestFB;
	float * ControlHPF;
	float * ControlWarmth;
	float * ControlDiffusion;

	float * AudioOutputBufferL;
	float * AudioOutputBufferR;
	float * AudioInputBufferL;
	float * AudioInputBufferR; 

	double SampleRate;

	/* Stuff to remember to avoid recalculating the delays every run */
	float LastRoomLength;
	float LastRoomWidth; 
	float LastRoomHeight;
	float LastSourceLR;
	float LastSourceFB;
	float LastDestLR; 
	float LastDestFB;
	float LastHPF;
	float LastWarmth; 
	float LastDiffusion;

	float ConvertedHPF; 
	float ConvertedWarmth; 

	/* Delay and Reverb Space Data */
	unsigned int er_size;
	struct ERunit * er;
	unsigned long SpaceSize;
	float * SpaceL;
	float * SpaceR;
	float * SpaceLCur;
	float * SpaceRCur;
	float * SpaceLEnd;
	float * SpaceREnd;

	float AudioHPFLast;
	float AudioIn1Last;
	float AudioIn2Last;
	float AudioIn3Last; 
	float AudioIn4Last;

} IReverbER;


static LV2_Handle instantiateIReverbER(const LV2_Descriptor *descriptor, double s_rate, const char *path, const LV2_Feature * const* features)
{
	IReverbER *plugin = (IReverbER *)malloc(sizeof(IReverbER));
	if(plugin==NULL)
		return NULL;

	/* set some initial params */
	plugin->SampleRate=s_rate;
	plugin->SpaceSize = REVERB_SPACE_SIZE * s_rate;

	/* the delay space */
	if((plugin->SpaceL  = (float *)malloc(sizeof(float) * plugin->SpaceSize))==NULL)
    		return NULL;
	
	if((plugin->SpaceR  = (float *)malloc(sizeof(float) * plugin->SpaceSize))==NULL)
    		return NULL;
	
	/* the delays */
	if((plugin->er  = (struct ERunit *)malloc(sizeof(struct ERunit) * MAX_ER))==NULL)
    		return NULL;	

	return (LV2_Handle)plugin;
}



static void connectPortIReverbER(LV2_Handle instance, uint32_t port, void *data)
{
	IReverbER *plugin = (IReverbER *)instance;
	switch (port) {
		case IERR_ROOMLENGTH:
			plugin->ControlRoomLength = data;
			break;
		case IERR_ROOMWIDTH:
			plugin->ControlRoomWidth = data;
			break;
		case IERR_ROOMHEIGHT:
			plugin->ControlRoomHeight = data;
			break;
		case IERR_SOURCELR:
			plugin->ControlSourceLR = data;
			break;
		case IERR_SOURCEFB:
			plugin->ControlSourceFB = data;
			break;
		case IERR_DESTLR:
			plugin->ControlDestLR = data;
			break;
		case IERR_DESTFB:
			plugin->ControlDestFB = data;
			break;
		case IERR_HPF:
			plugin->ControlHPF = data;
			break;
		case IERR_WARMTH:
			plugin->ControlWarmth = data;
			break;
		case IERR_DIFFUSION:
			plugin->ControlDiffusion = data;
			break;
		case IERR_AUDIO_OUTL:
			plugin->AudioOutputBufferL = data;
			break;
		case IERR_AUDIO_OUTR:
			plugin->AudioOutputBufferR = data;
			break;
		case IERR_AUDIO_INL:
			plugin->AudioInputBufferL = data;
			break;
		case IERR_AUDIO_INR:
			plugin->AudioInputBufferR = data;
			break;
	}
}


static void activateIReverbER(LV2_Handle instance) 
{
	IReverbER *plugin = (IReverbER *)instance;

	unsigned long i;
	float * p;
	float * q;
	
	//set ourselves at the beginning of space
	plugin->SpaceLCur=plugin->SpaceL;
	plugin->SpaceRCur=plugin->SpaceR;

	// clear space	
	p=plugin->SpaceL;
	q=plugin->SpaceR;
	for(i=0; i < plugin->SpaceSize; i++) {
		*(p++)=0;
		*(q++)=0;
	}
	plugin->SpaceLEnd=--p;
	plugin->SpaceREnd=--q;
  
	//set defaults
	plugin->LastRoomLength = 25.0;
	plugin->LastRoomWidth  = 30.0; 
	plugin->LastRoomHeight = 10;
	plugin->LastSourceLR   = -0.01;
	plugin->LastSourceFB   = 0.8;
	plugin->LastDestLR     = 0.01; 
	plugin->LastDestFB     = 0.2;
	plugin->LastHPF        = 0.001;
	plugin->LastWarmth     = 0.5;
	plugin->LastDiffusion  = 0.5;

	plugin->AudioHPFLast=0;
	plugin->AudioIn1Last=0;
	plugin->AudioIn2Last=0;
	plugin->AudioIn3Last=0; 
	plugin->AudioIn4Last=0;

	plugin->ConvertedHPF    = convertParam(IERR_HPF,    plugin->LastHPF,    plugin->SampleRate);  
	plugin->ConvertedWarmth = convertParam(IERR_WARMTH, plugin->LastWarmth, plugin->SampleRate);  
	calculateIReverbER(instance);
}


static void runMonoIReverbER(LV2_Handle instance, uint32_t SampleCount) 
{
	float (*pParamFunc)(unsigned long, float, double) = NULL;
	float * pfAudioInputL;
	float * pfAudioOutputL;
	float * pfAudioOutputR;
	float AudioIn,AudioHPF,AudioIn1,AudioIn2,AudioIn3,AudioIn4,AudioProc;
	float HPFsamples,WarmthSamples;
	struct ERunit * er;
	unsigned long lSampleIndex;
	unsigned int i;
	unsigned int er_size;
	unsigned long SpaceSize;
	float * SpaceLStr;
	float * SpaceRStr;
	float * SpaceLCur;
	float * SpaceRCur;
	float * SpaceLEnd;
	float * SpaceREnd;

	IReverbER *plugin = (IReverbER *)instance;
	pParamFunc = &convertParam;

	/* see if the room has changed and recalculate the reflection details if needed */
	if(*(plugin->ControlRoomLength) != plugin->LastRoomLength || 
	   *(plugin->ControlRoomWidth)  != plugin->LastRoomWidth  ||
	   *(plugin->ControlRoomHeight) != plugin->LastRoomHeight ||
	   *(plugin->ControlSourceLR)   != plugin->LastSourceLR   ||
	   *(plugin->ControlSourceFB)   != plugin->LastSourceFB   ||
	   *(plugin->ControlDestLR)     != plugin->LastDestLR     ||
	   *(plugin->ControlDestFB)     != plugin->LastDestFB     ||
	   *(plugin->ControlDiffusion)  != plugin->LastDiffusion  ) {
		  plugin->LastRoomLength = *(plugin->ControlRoomLength);
		  plugin->LastRoomWidth  = *(plugin->ControlRoomWidth);
		  plugin->LastRoomHeight = *(plugin->ControlRoomHeight);
		  plugin->LastSourceLR   = *(plugin->ControlSourceLR);
		  plugin->LastSourceFB   = *(plugin->ControlSourceFB);
		  plugin->LastDestLR     = *(plugin->ControlDestLR);
		  plugin->LastDestFB     = *(plugin->ControlDestFB);
		  plugin->LastDiffusion  = *(plugin->ControlDiffusion);
		  
		  calculateIReverbER(instance);
	}

	/* check if any other params have changed */
	checkParamChange(IERR_WARMTH, plugin->ControlWarmth, &(plugin->LastWarmth), &(plugin->ConvertedWarmth), plugin->SampleRate, pParamFunc);
	checkParamChange(IERR_HPF,    plugin->ControlHPF,    &(plugin->LastHPF),    &(plugin->ConvertedHPF),    plugin->SampleRate, pParamFunc);

	WarmthSamples   = plugin->ConvertedWarmth;
	HPFsamples   	= plugin->ConvertedHPF;
	
	er_size   	= plugin->er_size;
	SpaceSize 	= plugin->SpaceSize;

	SpaceLStr	= plugin->SpaceL;
	SpaceRStr 	= plugin->SpaceR;
	SpaceLCur 	= plugin->SpaceLCur;
	SpaceRCur 	= plugin->SpaceRCur;
	SpaceLEnd 	= plugin->SpaceLEnd;
	SpaceREnd 	= plugin->SpaceREnd;

	AudioHPF	= plugin->AudioHPFLast;
	AudioIn1	= plugin->AudioIn1Last;
	AudioIn2	= plugin->AudioIn2Last;
	AudioIn3	= plugin->AudioIn3Last;
	AudioIn4	= plugin->AudioIn4Last;

	pfAudioInputL 	= plugin->AudioInputBufferL;
	pfAudioOutputL 	= plugin->AudioOutputBufferL;
	pfAudioOutputR 	= plugin->AudioOutputBufferR;

	for (lSampleIndex = 0; lSampleIndex < SampleCount; lSampleIndex++) {

		AudioIn=*(pfAudioInputL++);
		// apply HPF as bottom end in reverbs sounds crap
		AudioHPF = ((HPFsamples-1) * AudioHPF + AudioIn) / HPFsamples;  
		AudioIn = AudioIn - AudioHPF;

		// apply simple LPF filter repeatedly to audio to simluate frequency loss with each reflection
		AudioIn1=((WarmthSamples-1) * AudioIn1 + AudioIn) / WarmthSamples; 
		AudioIn2=((WarmthSamples-1) * AudioIn2 + AudioIn1) / WarmthSamples; 
		AudioIn3=((WarmthSamples-1) * AudioIn3 + AudioIn2) / WarmthSamples; 
		AudioIn4=((WarmthSamples-1) * AudioIn4 + AudioIn3) / WarmthSamples; 
		  
		er = plugin->er;
		  
		// now calculate the reflections
		for(i=0;i<er_size;i++) {
			// pick the right version of the audio as per reflection count
			switch(er->Reflections) {
				case 0:
					AudioProc=AudioIn;
					break;
				case 1:
					AudioProc=AudioIn1;
					break;
				case 2:
					AudioProc=AudioIn2;
					break;
				case 3:
					AudioProc=AudioIn3;
					break;
				case 4:
					AudioProc=AudioIn4;
					break;
				default:
					AudioProc=0;
					break;
			}
			// add the reflection into the delay space
			if(SpaceLCur+er->Delay > SpaceLEnd)
				*(SpaceLCur+er->Delay-SpaceSize)+=AudioProc*er->GainL;
			else
				*(SpaceLCur+er->Delay)+=AudioProc*er->GainL;

			if(SpaceRCur+er->Delay > SpaceREnd)
				*(SpaceRCur+er->Delay-SpaceSize)+=AudioProc*er->GainR;
			else
				*(SpaceRCur+er->Delay)+=AudioProc*er->GainR;
			  
			er++;
		}
		// read the audio out of the delay space
		*(pfAudioOutputL++) = *(SpaceLCur);
		*(pfAudioOutputR++) = *(SpaceRCur);
		// zero the spot we just read
		*(SpaceLCur)=0;
		*(SpaceRCur)=0;
		// advance the pointer to the next spot
		SpaceLCur = SpaceLCur < SpaceLEnd ? SpaceLCur + 1 : SpaceLStr;
		SpaceRCur = SpaceRCur < SpaceREnd ? SpaceRCur + 1 : SpaceRStr;
	
	}
	// remember for next run
	plugin->SpaceLCur=SpaceLCur;
	plugin->SpaceRCur=SpaceRCur;
	plugin->AudioHPFLast=(fabs(AudioHPF)<1.0e-10)  ? 0.f : AudioHPF; 
	plugin->AudioIn1Last=(fabs(AudioIn1)<1.0e-10)  ? 0.f : AudioIn1; 
	plugin->AudioIn2Last=(fabs(AudioIn2)<1.0e-10)  ? 0.f : AudioIn2; 
	plugin->AudioIn3Last=(fabs(AudioIn3)<1.0e-10)  ? 0.f : AudioIn3; 
	plugin->AudioIn4Last=(fabs(AudioIn4)<1.0e-10)  ? 0.f : AudioIn4; 
}


static void runSumIReverbER(LV2_Handle instance, uint32_t SampleCount) 
{
	float (*pParamFunc)(unsigned long, float, double) = NULL;
	float * pfAudioInputL;
	float * pfAudioInputR;
	float * pfAudioOutputL;
	float * pfAudioOutputR;
	float AudioIn,AudioHPF,AudioIn1,AudioIn2,AudioIn3,AudioIn4,AudioProc;
	float HPFsamples,WarmthSamples;
	struct ERunit * er;
	unsigned long lSampleIndex;
	unsigned int i;
	unsigned int er_size;
	unsigned long SpaceSize;
	float * SpaceLStr;
	float * SpaceRStr;
	float * SpaceLCur;
	float * SpaceRCur;
	float * SpaceLEnd;
	float * SpaceREnd;

	IReverbER *plugin = (IReverbER *)instance;
	pParamFunc = &convertParam;

	/* see if the room has changed and recalculate the reflection details if needed */
	if(*(plugin->ControlRoomLength) != plugin->LastRoomLength || 
	   *(plugin->ControlRoomWidth)  != plugin->LastRoomWidth  ||
	   *(plugin->ControlRoomHeight) != plugin->LastRoomHeight ||
	   *(plugin->ControlSourceLR)   != plugin->LastSourceLR   ||
	   *(plugin->ControlSourceFB)   != plugin->LastSourceFB   ||
	   *(plugin->ControlDestLR)     != plugin->LastDestLR     ||
	   *(plugin->ControlDestFB)     != plugin->LastDestFB     ||
	   *(plugin->ControlDiffusion)  != plugin->LastDiffusion  ) {
		  plugin->LastRoomLength = *(plugin->ControlRoomLength);
		  plugin->LastRoomWidth  = *(plugin->ControlRoomWidth);
		  plugin->LastRoomHeight = *(plugin->ControlRoomHeight);
		  plugin->LastSourceLR   = *(plugin->ControlSourceLR);
		  plugin->LastSourceFB   = *(plugin->ControlSourceFB);
		  plugin->LastDestLR     = *(plugin->ControlDestLR);
		  plugin->LastDestFB     = *(plugin->ControlDestFB);
		  plugin->LastDiffusion  = *(plugin->ControlDiffusion);
		  
		  calculateIReverbER(instance);
	}

	/* check if any other params have changed */
	checkParamChange(IERR_WARMTH, plugin->ControlWarmth, &(plugin->LastWarmth), &(plugin->ConvertedWarmth), plugin->SampleRate, pParamFunc);
	checkParamChange(IERR_HPF,    plugin->ControlHPF,    &(plugin->LastHPF),    &(plugin->ConvertedHPF),    plugin->SampleRate, pParamFunc);

	WarmthSamples   = plugin->ConvertedWarmth;
	HPFsamples   	= plugin->ConvertedHPF;
	
	er_size   	= plugin->er_size;
	SpaceSize 	= plugin->SpaceSize;

	SpaceLStr	= plugin->SpaceL;
	SpaceRStr 	= plugin->SpaceR;
	SpaceLCur 	= plugin->SpaceLCur;
	SpaceRCur 	= plugin->SpaceRCur;
	SpaceLEnd 	= plugin->SpaceLEnd;
	SpaceREnd 	= plugin->SpaceREnd;

	AudioHPF	= plugin->AudioHPFLast;
	AudioIn1	= plugin->AudioIn1Last;
	AudioIn2	= plugin->AudioIn2Last;
	AudioIn3	= plugin->AudioIn3Last;
	AudioIn4	= plugin->AudioIn4Last;

	pfAudioInputL 	= plugin->AudioInputBufferL;
	pfAudioInputR 	= plugin->AudioInputBufferR;
	pfAudioOutputL 	= plugin->AudioOutputBufferL;
	pfAudioOutputR 	= plugin->AudioOutputBufferR;

	for (lSampleIndex = 0; lSampleIndex < SampleCount; lSampleIndex++) {

		AudioIn=( *(pfAudioInputL++) + *(pfAudioInputR++) )/2;

		// apply HPF as bottom end in reverbs sounds crap
		AudioHPF = ((HPFsamples-1) * AudioHPF + AudioIn) / HPFsamples;  
		AudioIn = AudioIn - AudioHPF;

		// apply simple filter repeatedly to audio to simluate frequency loss with each reflection
		AudioIn1=((WarmthSamples-1) * AudioIn1 + AudioIn) / WarmthSamples; 
		AudioIn2=((WarmthSamples-1) * AudioIn2 + AudioIn1) / WarmthSamples; 
		AudioIn3=((WarmthSamples-1) * AudioIn3 + AudioIn2) / WarmthSamples; 
		AudioIn4=((WarmthSamples-1) * AudioIn4 + AudioIn3) / WarmthSamples; 
		  
		er = plugin->er;
		  
		// now calculate the reflections
		for(i=0;i<er_size;i++) {
			// pick the right version of the audio as per reflection count
			switch(er->Reflections) {
				case 0:
					AudioProc=AudioIn;
					break;
				case 1:
					AudioProc=AudioIn1;
					break;
				case 2:
					AudioProc=AudioIn2;
					break;
				case 3:
					AudioProc=AudioIn3;
					break;
				case 4:
					AudioProc=AudioIn4;
					break;
				default:
					AudioProc=0;
					break;
			}
			// add the reflection into the delay space
			if(SpaceLCur+er->Delay > SpaceLEnd)
				*(SpaceLCur+er->Delay-SpaceSize)+=AudioProc*er->GainL;
			else
				*(SpaceLCur+er->Delay)+=AudioProc*er->GainL;

			if(SpaceRCur+er->Delay > SpaceREnd)
				*(SpaceRCur+er->Delay-SpaceSize)+=AudioProc*er->GainR;
			else
				*(SpaceRCur+er->Delay)+=AudioProc*er->GainR;
			  
			er++;
		}
		// read the audio out of the delay space
		*(pfAudioOutputL++) = *(SpaceLCur);
		*(pfAudioOutputR++) = *(SpaceRCur);
		// zero the spot we just read
		*(SpaceLCur)=0;
		*(SpaceRCur)=0;
		// advance the pointer to the next spot
		SpaceLCur = SpaceLCur < SpaceLEnd ? SpaceLCur + 1 : SpaceLStr;
		SpaceRCur = SpaceRCur < SpaceREnd ? SpaceRCur + 1 : SpaceRStr;
	
	}
	// remember for next run
	plugin->SpaceLCur=SpaceLCur;
	plugin->SpaceRCur=SpaceRCur;
	plugin->AudioHPFLast=(fabs(AudioHPF)<1.0e-10)  ? 0.f : AudioHPF; 
	plugin->AudioIn1Last=(fabs(AudioIn1)<1.0e-10)  ? 0.f : AudioIn1; 
	plugin->AudioIn2Last=(fabs(AudioIn2)<1.0e-10)  ? 0.f : AudioIn2; 
	plugin->AudioIn3Last=(fabs(AudioIn3)<1.0e-10)  ? 0.f : AudioIn3; 
	plugin->AudioIn4Last=(fabs(AudioIn4)<1.0e-10)  ? 0.f : AudioIn4; 
}


static void cleanupIReverbER(LV2_Handle instance)
{
	IReverbER *plugin = (IReverbER *)instance;

	free(plugin->er);
	free(plugin->SpaceL);
	free(plugin->SpaceR);
	free(instance);
}


static void init()
{
	IReverbERMonoDescriptor =
	 (LV2_Descriptor *)malloc(sizeof(LV2_Descriptor));

	IReverbERMonoDescriptor->URI 		= IERR_MONO_URI;
	IReverbERMonoDescriptor->activate 	= activateIReverbER;
	IReverbERMonoDescriptor->cleanup 	= cleanupIReverbER;
	IReverbERMonoDescriptor->connect_port 	= connectPortIReverbER;
	IReverbERMonoDescriptor->deactivate 	= NULL;
	IReverbERMonoDescriptor->instantiate 	= instantiateIReverbER;
	IReverbERMonoDescriptor->run 		= runMonoIReverbER;
	IReverbERMonoDescriptor->extension_data	= NULL;

	IReverbERSumDescriptor =
	 (LV2_Descriptor *)malloc(sizeof(LV2_Descriptor));

	IReverbERSumDescriptor->URI 		= IERR_SUM_URI;
	IReverbERSumDescriptor->activate 	= activateIReverbER;
	IReverbERSumDescriptor->cleanup 	= cleanupIReverbER;
	IReverbERSumDescriptor->connect_port 	= connectPortIReverbER;
	IReverbERSumDescriptor->deactivate 	= NULL;
	IReverbERSumDescriptor->instantiate 	= instantiateIReverbER;
	IReverbERSumDescriptor->run 		= runSumIReverbER;
	IReverbERSumDescriptor->extension_data	= NULL;
}


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
	if (!IReverbERMonoDescriptor) init();

	switch (index) {
		case 0:
			return IReverbERMonoDescriptor;
		case 1:
			return IReverbERSumDescriptor;
		default:
			return NULL;
	}
}


/*****************************************************************************/


void calculateIReverbER(LV2_Handle instance)
{
	IReverbER *plugin = (IReverbER *)instance;

	float convertedWidth,convertedLength,convertedHeight,convertedSourceLR,convertedSourceFB,convertedDestLR,convertedDestFB,convertedDiffusion;
	float SourceToLeft,SourceToRight,SourceToRear,SourceToFront;
	float DestToLeft,DestToRight,DestToRear,DestToFront;
	float RoofHeight,FloorDepth;
	float DirectLength,DirectWidth,DirectHeight,DirectDistanceSQRD,DirectDistance;
	float ERLength,ERWidth,ERHeight,MaxGain;

	struct ERunit * er;
	unsigned int Num,i;

	if (plugin->LastRoomWidth < 3.0)
		convertedWidth = 3.0;
	else if (plugin->LastRoomWidth <= 100.0)
		convertedWidth = plugin->LastRoomWidth;
	else
		convertedWidth = 100.0;

	if (plugin->LastRoomLength < 3.0)
		convertedLength = 3.0;
	else if (plugin->LastRoomLength <= 100.0)
		convertedLength = plugin->LastRoomLength;
	else
		convertedLength = 100.0;

	if (plugin->LastRoomHeight < 3.0)
		convertedHeight = 3.0;
	else if (plugin->LastRoomHeight <= 30.0)
		convertedHeight = plugin->LastRoomHeight;
	else
		convertedHeight = 30.0;

	if (plugin->LastSourceLR < -0.99)
		convertedSourceLR = -0.99;
	else if (plugin->LastSourceLR <= 0.99)
		convertedSourceLR = plugin->LastSourceLR;
	else
		convertedSourceLR = 0.99;

	if (plugin->LastSourceFB < 0.51)
		convertedSourceFB = 0.51;
	else if (plugin->LastSourceFB <= 0.99)
		convertedSourceFB = plugin->LastSourceFB;
	else
		convertedSourceFB = 0.99;

	if (plugin->LastDestLR < -0.99)
		convertedDestLR = -0.99;
	else if (plugin->LastDestLR <= 0.99)
		convertedDestLR = plugin->LastDestLR;
	else
		convertedDestLR = 0.99;

	if (plugin->LastDestFB < 0.01)
		convertedDestFB = 0.01;
	else if (plugin->LastDestFB <= 0.49)
		convertedDestFB = plugin->LastDestFB;
	else
		convertedDestFB = 0.49;

	if (plugin->LastDiffusion < 0.0)
		convertedDiffusion = 0.0;
	else if (plugin->LastDiffusion <= 1.0)
		convertedDiffusion = plugin->LastDiffusion;
	else
		convertedDiffusion = 1.0;

	SourceToLeft = (1+convertedSourceLR) /2 * convertedWidth;
	SourceToRight= (1-convertedSourceLR) /2 * convertedWidth;
	SourceToFront= convertedSourceFB        * convertedLength;
	SourceToRear = (1-convertedSourceFB)    * convertedLength;

	DestToLeft = (1+convertedDestLR) /2 * convertedWidth;
	DestToRight= (1-convertedDestLR) /2 * convertedWidth;
	DestToFront= convertedDestFB        * convertedLength;
	DestToRear = (1-convertedDestFB)    * convertedLength;

	RoofHeight = convertedHeight - OBJECT_HEIGHT;
	FloorDepth = OBJECT_HEIGHT;

	DirectLength = SourceToFront-DestToFront;
	DirectWidth = SourceToLeft-DestToLeft;
	DirectHeight =0; // both the source and the lisenter are at the same height
	DirectDistanceSQRD = pow(DirectLength,2)+pow(DirectWidth,2) < 1 ? 1 : pow(DirectLength,2)+pow(DirectWidth,2);
	DirectDistance = sqrt(DirectDistanceSQRD) < 1 ? 1 : sqrt(DirectDistanceSQRD);

	er=plugin->er;
	Num=0;
	MaxGain=0.000000000001; /* this is used to scale up the reflections so that the loudest one has a gain of 1 (0db) */

	/* seed the random sequence with a version of the diffusion */
	srand48(1+(long int)(convertedDiffusion*TWO31_MINUS2));
  
	// reflections from the left wall
	// 0: S->Left->D
	ERLength       = DirectLength;
	ERWidth        = -(SourceToLeft + DestToLeft);
	ERHeight       = DirectHeight;
	calculateSingleIReverbER(er, ERWidth, ERLength,  ERHeight, -1, 1, DirectDistance, plugin->SampleRate);
	if(er->AbsGain > MaxGain)
		MaxGain=er->AbsGain;
	er++;
	Num++;

	// 1: S->BackWall->Left->D
	ERLength       = SourceToRear + DestToRear;
	ERWidth        = -(SourceToLeft + DestToLeft);
	ERHeight       = DirectHeight;
	calculateSingleIReverbER(er, ERWidth, ERLength,  ERHeight, 1, 2, DirectDistance, plugin->SampleRate);
	if(er->AbsGain > MaxGain)
		MaxGain=er->AbsGain;
	er++;
	Num++;

	// 2: S->Right->Left->D
	ERLength       = DirectLength;
	ERWidth        = -(SourceToRight + convertedWidth + DestToLeft);
	ERHeight       = DirectHeight;
	calculateSingleIReverbER(er, ERWidth, ERLength,  ERHeight, 1, 2, DirectDistance, plugin->SampleRate);
	if(er->AbsGain > MaxGain)
		MaxGain=er->AbsGain;
	er++;
	Num++;

	// 3: S->BackWall->Right->Left->D
	ERLength       = SourceToRear + DestToRear;
	ERWidth        = -(SourceToRight + convertedWidth + DestToLeft);
	ERHeight       = DirectHeight;
	calculateSingleIReverbER(er, ERWidth, ERLength,  ERHeight, -1, 3, DirectDistance, plugin->SampleRate);
	if(er->AbsGain > MaxGain)
		MaxGain=er->AbsGain;
	er++;
	Num++;  

	// 4: S->Left->Rigtht->Left->D
	ERLength       = DirectLength;
	ERWidth        = -(SourceToLeft + (2 * convertedWidth) + DestToLeft);
	ERHeight       = DirectHeight;
	calculateSingleIReverbER(er, ERWidth, ERLength,  ERHeight, -1, 3, DirectDistance, plugin->SampleRate);
	if(er->AbsGain > MaxGain)
		MaxGain=er->AbsGain;
	er++;
	Num++;

	// 5: S->BackWall->Left->Right->Left->D
	ERLength       = SourceToRear + DestToRear;
	ERWidth        = -(SourceToLeft + (2 * convertedWidth) + DestToLeft);
	ERHeight       = DirectHeight;
	calculateSingleIReverbER(er, ERWidth, ERLength, ERHeight, 1, 4, DirectDistance,  plugin->SampleRate);
	if(er->AbsGain > MaxGain)
		MaxGain=er->AbsGain;
	er++;
	Num++;  

	// reflections from the right wall
	// 6: S->Right->D
	ERLength       = DirectLength;
	ERWidth        = SourceToRight + DestToRight;
	ERHeight       = DirectHeight;
	calculateSingleIReverbER(er, ERWidth, ERLength, ERHeight, -1, 1, DirectDistance, plugin->SampleRate);
	if(er->AbsGain > MaxGain)
		MaxGain=er->AbsGain;
	er++;
	Num++;

	// 7: S->BackWall->Right->D
	ERLength       = SourceToRear + DestToRear;
	ERWidth        = SourceToRight + DestToRight;
	ERHeight       = DirectHeight;
	calculateSingleIReverbER(er, ERWidth, ERLength, ERHeight, 1, 2, DirectDistance, plugin->SampleRate);
	if(er->AbsGain > MaxGain)
		MaxGain=er->AbsGain;
	er++;
	Num++;

	// 8: S->Left->Right->D
	ERLength       = DirectLength;
	ERWidth        = SourceToLeft + convertedWidth + DestToRight;
	ERHeight       = DirectHeight;
	calculateSingleIReverbER(er, ERWidth, ERLength, ERHeight, 1, 2, DirectDistance, plugin->SampleRate);
	if(er->AbsGain > MaxGain)
		MaxGain=er->AbsGain;
	er++;
	Num++;

	// 9: S->BackWall->Left->Right->D
	ERLength       = SourceToRear + DestToRear;
	ERWidth        = SourceToLeft + convertedWidth + DestToRight;
	ERHeight       = DirectHeight;
	calculateSingleIReverbER(er, ERWidth, ERLength, ERHeight, -1, 3, DirectDistance, plugin->SampleRate);
	if(er->AbsGain > MaxGain)
		MaxGain=er->AbsGain;
	er++;
	Num++;

	// 10: S->Right->Left->Right->D
	ERLength       = DirectLength;
	ERWidth        = SourceToRight + (2 * convertedWidth) + DestToRight;
	ERHeight       = DirectHeight;
	calculateSingleIReverbER(er, ERWidth, ERLength, ERHeight, -1, 3, DirectDistance, plugin->SampleRate);
	if(er->AbsGain > MaxGain)
		MaxGain=er->AbsGain;
	er++;
	Num++;

	// 11: S->BackWall->Right->Left->Right->D
	ERLength       = SourceToRear + DestToRear;
	ERWidth        = SourceToRight + (2 * convertedWidth) + DestToRight;
	ERHeight       = DirectHeight;
	calculateSingleIReverbER(er, ERWidth, ERLength, ERHeight, 1, 4, DirectDistance, plugin->SampleRate);
	if(er->AbsGain > MaxGain)
		MaxGain=er->AbsGain;
	er++;
	Num++;

	// reflections from the rear wall
	// 12: S->BackWall->D
	ERLength       = SourceToRear + DestToRear;
	ERWidth        = DirectWidth;
	ERHeight       = DirectHeight;
	calculateSingleIReverbER(er, ERWidth, ERLength, ERHeight, -1, 1, DirectDistance, plugin->SampleRate);
	if(er->AbsGain > MaxGain)
		MaxGain=er->AbsGain;
	er++;
	Num++;

	// 13: S->NearWall->BackWall->D
	ERLength       = SourceToFront + convertedLength + DestToRear;
	ERWidth        = DirectWidth;
	ERHeight       = DirectHeight;
	calculateSingleIReverbER(er, ERWidth, ERLength, ERHeight, 1, 2, DirectDistance, plugin->SampleRate);
	if(er->AbsGain > MaxGain)
		MaxGain=er->AbsGain;
	er++;
	Num++;

	// S->Left->NearWall->BackWall->D
	ERLength       = SourceToFront + convertedLength + DestToRear;
	ERWidth        = -(SourceToLeft + DestToLeft);
	ERHeight       = DirectHeight;
	calculateSingleIReverbER(er, ERWidth, ERLength, ERHeight, -1, 3, DirectDistance, plugin->SampleRate);
	if(er->AbsGain > MaxGain)
		MaxGain=er->AbsGain;
	er++;
	Num++;

	// S->Right->NearWall->BackWall->D
	ERLength       = SourceToFront + convertedLength + DestToRear;
	ERWidth        = SourceToRight + DestToRight;
	ERHeight       = DirectHeight;
	calculateSingleIReverbER(er, ERWidth, ERLength, ERHeight, -1, 3, DirectDistance, plugin->SampleRate);
	if(er->AbsGain > MaxGain)
		MaxGain=er->AbsGain;
	er++;
	Num++;

	// reflections from the roof
	// S->Roof->Left->D
	ERLength       = DirectLength;
	ERWidth        = -(SourceToLeft + DestToLeft);
	ERHeight       = 2*RoofHeight;
	calculateSingleIReverbER(er, ERWidth, ERLength,  ERHeight, 1, 2, DirectDistance, plugin->SampleRate);
	if(er->AbsGain > MaxGain)
		MaxGain=er->AbsGain;
	er++;
	Num++;

	// S->Roof->Right->D
	ERLength       = DirectLength;
	ERWidth        = SourceToRight + DestToRight;
	ERHeight       = 2*RoofHeight;
	calculateSingleIReverbER(er, ERWidth, ERLength, ERHeight, -1, 1, DirectDistance, plugin->SampleRate);
	if(er->AbsGain > MaxGain)
		MaxGain=er->AbsGain;
	er++;
	Num++;

	// S->BackWall->Roof->Left->D
	ERLength       = SourceToRear + DestToRear;
	ERWidth        = -(SourceToLeft + DestToLeft);
	ERHeight       = 2*RoofHeight;
	calculateSingleIReverbER(er, ERWidth, ERLength, ERHeight, -1, 3, DirectDistance, plugin->SampleRate);
	if(er->AbsGain > MaxGain)
		MaxGain=er->AbsGain;
	er++;
	Num++;

	// S->BackWall->Roof->Right->D
	ERLength       = SourceToRear + DestToRear;
	ERWidth        = SourceToRight + DestToRight;
	ERHeight       = 2*RoofHeight;
	calculateSingleIReverbER(er, ERWidth, ERLength, ERHeight, -1, 3, DirectDistance, plugin->SampleRate);
	if(er->AbsGain > MaxGain)
		MaxGain=er->AbsGain;
	er++;
	Num++;

	// reflections from the floor 
	// S->Floor->Left->D
	ERLength       = DirectLength;
	ERWidth        = -(SourceToLeft + DestToLeft);
	ERHeight       = 2*FloorDepth;
	calculateSingleIReverbER(er, ERWidth, ERLength,  ERHeight, 1, 2, DirectDistance, plugin->SampleRate);
	if(er->AbsGain > MaxGain)
		MaxGain=er->AbsGain;
	er++;
	Num++;

	// S->Floor->Right->D
	ERLength       = DirectLength;
	ERWidth        = SourceToRight + DestToRight;
	ERHeight       = 2*FloorDepth;
	calculateSingleIReverbER(er, ERWidth, ERLength, ERHeight, 1, 2, DirectDistance, plugin->SampleRate);
	if(er->AbsGain > MaxGain)
		MaxGain=er->AbsGain;
	er++;
	Num++;

	// reflections from roof and floor
	// S->Roof->Left->Floor->D
	ERLength       = DirectLength;
	ERWidth        = -(SourceToLeft + DestToLeft);
	ERHeight       = 2*RoofHeight + 2*FloorDepth;
	calculateSingleIReverbER(er, ERWidth, ERLength,  ERHeight, -1, 3, DirectDistance, plugin->SampleRate);
	if(er->AbsGain > MaxGain)
		MaxGain=er->AbsGain;
	er++;
	Num++;

	// S->Roof->Right->Floor->D
	ERLength       = DirectLength;
	ERWidth        = SourceToRight + DestToRight;
	ERHeight       = 2*RoofHeight + 2*FloorDepth;
	calculateSingleIReverbER(er, ERWidth, ERLength, ERHeight, -1, 3, DirectDistance, plugin->SampleRate);
	if(er->AbsGain > MaxGain)
		MaxGain=er->AbsGain;
	er++;
	Num++;

	plugin->er_size = Num; 	


	/* scale up and calculate l/r actual gains for run() */
	er=plugin->er;
	for(i=0;i<Num;i++) {
		er->Delay=er->Delay*(1.01+drand48()*convertedDiffusion/10);
		er->GainL=er->GainL/MaxGain;
		er->GainR=er->GainR/MaxGain;
		er++;
	}
}


void calculateSingleIReverbER(struct ERunit * er, float Width, float Length, float Height, int Phase, unsigned int Reflections, float DDist, double sr) {

	float ERAngle,ERDistanceSQRD,ERDistance,ERRelGain,ERRelGainL,ERRelGainR;
	unsigned long ERRelDelay;

	ERAngle        = atan(Width/Length);
	ERDistanceSQRD = pow(Length,2) + pow(Width,2)+ pow(Height,2);
	ERDistance     = sqrt(ERDistanceSQRD);
	ERRelDelay     = (unsigned long)((ERDistance-DDist) * (float)sr /SPEED_OF_SOUND);
	ERRelGain      = Phase / ERDistanceSQRD;
	ERRelGainL     = (ERRelGain * (1 - (ERAngle/PI_ON_2)))/2;
	ERRelGainR     = (ERRelGain * (1 + (ERAngle/PI_ON_2)))/2;

	er->Active=1;
	er->Delay=ERRelDelay;
	er->Reflections=Reflections;
	er->AbsGain=fabs(ERRelGain);
	er->GainL=ERRelGainL;
	er->GainR=ERRelGainR;
}

float convertParam(unsigned long param, float value, double sr) {

	float temp;
	float result;

	switch(param)
	{
		case IERR_HPF:
			temp = value / (float)sr;
			if (temp < 0.001)
				result = 500;
			else if (temp <= 0.05)
				result = 1/(2*temp);
			else
				result=10;
			break;
		case IERR_WARMTH:
			if(value<0)
				result= 1;
			else if (value < 1)
				result = pow(2,value*2);
			else
				result= 4;
			break;
			
			break;
		default:
			result=0;
			break;
	}
	return result;
}

