/* 

    This LV2 plugin provides a stereo input module

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


#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <lv2.h>
#include "libinv_common.h"
#include "inv_input.h"

/* The port numbers for the plugin: */

#define IINPUT_URI		"http://invadarecords.com/plugins/inv_input";
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


static LV2_Descriptor *IInputDescriptor = NULL;


typedef struct {

	/* Ports */
	float *ControlPhaseL;
	float *ControlPhaseR;         
	float *ControlGain;
	float *ControlPan;  
	float *ControlWidth;  
	float *ControlNoClip;  
	float *AudioInputBufferL;
	float *AudioInputBufferR; 
	float *AudioOutputBufferL;
	float *AudioOutputBufferR;

	/* stuff we need to remember to reduce cpu */ 
	double SampleRate; 

	/* stuff we need to remember to reduce cpu */ 
	float LastPhaseL;
	float LastPhaseR;     
	float LastGain;
	float LastPan;  
	float LastWidth;  
	float LastNoClip; 

	float ConvertedPhaseL;
	float ConvertedPhaseR; 
	float ConvertedGain;
	float ConvertedPan;  
	float ConvertedWidth;  
	float ConvertedNoClip; 


} IInput;

static LV2_Handle instantiateIInput(const LV2_Descriptor *descriptor, double s_rate, const char *path, const LV2_Feature * const* features)
{
	IInput *plugin = (IInput *)malloc(sizeof(IInput));
	if(plugin==NULL)
		return NULL;

	/* set some initial params */
	plugin->SampleRate=s_rate;

	return (LV2_Handle)plugin;
}



static void connectPortIInput(LV2_Handle instance, uint32_t port, void *data)
{
	IInput *plugin = (IInput *)instance;

	switch (port) {
		case IINPUT_PHASEL:
			plugin->ControlPhaseL = data;
			break;
		case IINPUT_PHASER:
			plugin->ControlPhaseR = data;
			break;
		case IINPUT_GAIN:
			plugin->ControlGain  = data;
			break;
		case IINPUT_PAN:
			plugin->ControlPan = data;
			break;
		case IINPUT_WIDTH:
			plugin->ControlWidth = data;
			break;
		case IINPUT_NOCLIP:
			plugin->ControlNoClip = data;
			break;
		case IINPUT_AUDIO_INL:
			plugin->AudioInputBufferL = data;
			break;
		case IINPUT_AUDIO_INR:
			plugin->AudioInputBufferR = data;
			break;
		case IINPUT_AUDIO_OUTL:
			plugin->AudioOutputBufferL = data;
			break;
		case IINPUT_AUDIO_OUTR:
			plugin->AudioOutputBufferR = data;
			break;
	}
}


static void activateIInput(LV2_Handle instance) 
{

	IInput *plugin = (IInput *)instance;

	// these values force the conversion to take place      

	plugin->LastPhaseL = 0;
	plugin->LastPhaseR = 0;     
	plugin->LastGain   = 0;
	plugin->LastPan    = 0;  
	plugin->LastWidth  = 0;  
	plugin->LastNoClip = 0; 

	plugin->ConvertedPhaseL = convertParam(IINPUT_PHASEL, plugin->LastPhaseL,  plugin->SampleRate);
	plugin->ConvertedPhaseR = convertParam(IINPUT_PHASER, plugin->LastPhaseR,  plugin->SampleRate);
	plugin->ConvertedGain   = convertParam(IINPUT_GAIN,   plugin->LastGain,    plugin->SampleRate);
	plugin->ConvertedPan    = convertParam(IINPUT_PAN,    plugin->LastPan,     plugin->SampleRate);
	plugin->ConvertedWidth  = convertParam(IINPUT_WIDTH,  plugin->LastWidth,   plugin->SampleRate);
	plugin->ConvertedNoClip = convertParam(IINPUT_NOCLIP, plugin->LastNoClip,  plugin->SampleRate);
}


static void runIInput(LV2_Handle instance, uint32_t SampleCount) 
{
	float (*pParamFunc)(unsigned long, float, double) = NULL;
	float * pfAudioInputL;
	float * pfAudioInputR;
	float * pfAudioOutputL;
	float * pfAudioOutputR;
	float fPhaseL,fPhaseR,fGain,fPan,fLPan,fRPan,fWidth,fMono,fStereoL,fStereoR,fNoClip;
	float fAudioL,fAudioR;
	uint32_t lSampleIndex;

	IInput *plugin = (IInput *)instance;

	pParamFunc = &convertParam;

	checkParamChange(IINPUT_PHASEL, plugin->ControlPhaseL, &(plugin->LastPhaseL), &(plugin->ConvertedPhaseL), plugin->SampleRate, pParamFunc);
	checkParamChange(IINPUT_PHASER, plugin->ControlPhaseR, &(plugin->LastPhaseR), &(plugin->ConvertedPhaseR), plugin->SampleRate, pParamFunc);
	checkParamChange(IINPUT_GAIN,   plugin->ControlGain,   &(plugin->LastGain),   &(plugin->ConvertedGain),   plugin->SampleRate, pParamFunc);
	checkParamChange(IINPUT_PAN,    plugin->ControlPan,    &(plugin->LastPan),    &(plugin->ConvertedPan),    plugin->SampleRate, pParamFunc);
	checkParamChange(IINPUT_WIDTH,  plugin->ControlWidth,  &(plugin->LastWidth),  &(plugin->ConvertedWidth),  plugin->SampleRate, pParamFunc);
	checkParamChange(IINPUT_NOCLIP, plugin->ControlNoClip, &(plugin->LastNoClip), &(plugin->ConvertedNoClip), plugin->SampleRate, pParamFunc);

	fPhaseL    = plugin->ConvertedPhaseL;
	fPhaseR    = plugin->ConvertedPhaseR;
	fGain      = plugin->ConvertedGain;
	fPan       = plugin->ConvertedPan;
	fWidth     = plugin->ConvertedWidth;
	fNoClip    = plugin->ConvertedNoClip;

	fLPan=1-fPan;
	fRPan=1+fPan;

	pfAudioInputL = plugin->AudioInputBufferL;
	pfAudioInputR = plugin->AudioInputBufferR;
	pfAudioOutputL = plugin->AudioOutputBufferL;
	pfAudioOutputR = plugin->AudioOutputBufferR;
  
	for (lSampleIndex = 0; lSampleIndex < SampleCount; lSampleIndex++) {
  
		fAudioL =  fPhaseL > 0 ? -(*(pfAudioInputL++)) : *(pfAudioInputL++) ;
		fAudioR =  fPhaseR > 0 ? -(*(pfAudioInputR++)) : *(pfAudioInputR++) ;
		fAudioL *= fGain;
		fAudioR *= fGain;
		fAudioL *= fLPan;
		fAudioR *= fRPan;
	  
		if(fWidth<=0) {
			fMono = (fAudioL + fAudioR) / 2;
			fAudioL = (1+fWidth)*fAudioL - fWidth*fMono;
			fAudioR = (1+fWidth)*fAudioR - fWidth*fMono;
		} else {
			fStereoL = (fAudioL - fAudioR) / 2;
			fStereoR = (fAudioR - fAudioL) / 2;
			fAudioL = (1-fWidth)*fAudioL + fWidth*fStereoL;
			fAudioR = (1-fWidth)*fAudioR + fWidth*fStereoR;
		}

		fAudioL = fNoClip > 0 ? InoClip(fAudioL) : fAudioL;
		fAudioR = fNoClip > 0 ? InoClip(fAudioR) : fAudioR;
		*(pfAudioOutputL++) = fAudioL;
		*(pfAudioOutputR++) = fAudioR;
	}
}


static void cleanupIInput(LV2_Handle instance)
{
	free(instance);
}


static void init()
{
	IInputDescriptor =
	 (LV2_Descriptor *)malloc(sizeof(LV2_Descriptor));

	IInputDescriptor->URI 		= IINPUT_URI;
	IInputDescriptor->activate 	= activateIInput;
	IInputDescriptor->cleanup 	= cleanupIInput;
	IInputDescriptor->connect_port 	= connectPortIInput;
	IInputDescriptor->deactivate 	= NULL;
	IInputDescriptor->instantiate 	= instantiateIInput;
	IInputDescriptor->run 		= runIInput;
	IInputDescriptor->extension_data = NULL;
}


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
	if (!IInputDescriptor) init();

	switch (index) {
	case 0:
		return IInputDescriptor;
	default:
		return NULL;
	}
}


/*****************************************************************************/


float convertParam(unsigned long param, float value, double sr) {
	float result;
	switch(param)  {
		case IINPUT_PHASEL:
		case IINPUT_PHASER:
			if(value<0.5)
				result= 0; 
			else
				result= 1;
			break;
		case IINPUT_GAIN:
			if(value<-24)
				result= pow(10, -24/20);
			else if (value < 24)
				result= pow(10, value/20);
			else
				result= pow(10, 1.2);
			break;
		case IINPUT_PAN:
		case IINPUT_WIDTH:
			if(value < -1)
				result= -1;
			else if (value < 1)
				result= value;
			else
				result= 1;
			break;
		case IINPUT_NOCLIP:
			if(value<0.5)
				result= 0; 
			else
				result= 1;
			break;
		default:
			result=0;
			break;
	}
	return result;
}


