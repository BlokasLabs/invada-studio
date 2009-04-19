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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <lv2.h>
#include "library/common.h"
#include "inv_input.h"


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
	float *MeterInputL;
	float *MeterInputR; 
	float *MeterOutputL;
	float *MeterOutputR;
	float *MeterPhase;
	float *MeterDrive;

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
	float EnvInLLast; 
	float EnvInRLast; 
	float EnvOutLLast; 
	float EnvOutRLast; 
	float EnvPhaseLast; 
	float EnvDriveLast; 


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
		case IINPUT_METER_INL:
			plugin->MeterInputL = data;
			break;
		case IINPUT_METER_INR:
			plugin->MeterInputR = data;
			break;
		case IINPUT_METER_OUTL:
			plugin->MeterOutputL = data;
			break;
		case IINPUT_METER_OUTR:
			plugin->MeterOutputR = data;
			break;
		case IINPUT_METER_PHASE:
			plugin->MeterPhase = data;
			break;
		case IINPUT_METER_DRIVE:
			plugin->MeterDrive = data;
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
	plugin->LastNoClip = 1; 
	plugin->EnvInLLast = 0; 
	plugin->EnvOutLLast = 0; 
	plugin->EnvInRLast = 0; 
	plugin->EnvOutRLast = 0; 
	plugin->EnvPhaseLast = 0; 
	plugin->EnvDriveLast = 0; 

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
	float drive;
	float driveL=0;
	float driveR=0;
	float InL,EnvInL,EnvOutL;
	float InR,EnvInR,EnvOutR;
	float CurrentPhase,EnvPhase,EnvDrive;
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

	EnvInL     = plugin->EnvInLLast;
	EnvInR     = plugin->EnvInRLast;
	EnvOutL    = plugin->EnvOutLLast;
	EnvOutR    = plugin->EnvOutRLast;
	EnvPhase   = plugin->EnvPhaseLast;
	EnvDrive   = plugin->EnvDriveLast;
  
	for (lSampleIndex = 0; lSampleIndex < SampleCount; lSampleIndex++) {

		InL=*(pfAudioInputL++);
		InR=*(pfAudioInputR++);
  
		fAudioL =  fPhaseL > 0 ? -InL : InL ;
		fAudioR =  fPhaseR > 0 ? -InR : InR ;
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

		fAudioL = fNoClip > 0 ? InoClip(fAudioL,&driveL) : fAudioL;
		fAudioR = fNoClip > 0 ? InoClip(fAudioR,&driveR) : fAudioR;
		*(pfAudioOutputL++) = fAudioL;
		*(pfAudioOutputR++) = fAudioR;

		//evelope on in and out for meters
		EnvInL  += IEnvelope(InL, EnvInL, INVADA_METER_PEAK,plugin->SampleRate);
		EnvInR  += IEnvelope(InR, EnvInR, INVADA_METER_PEAK,plugin->SampleRate);
		EnvOutL += IEnvelope(fAudioL,EnvOutL,INVADA_METER_PEAK,plugin->SampleRate);
		EnvOutR += IEnvelope(fAudioR,EnvOutR,INVADA_METER_PEAK,plugin->SampleRate);

		if(fabs(fAudioL) > 0.001 || fabs(fAudioR) > 0.001) {  // -60 db
			CurrentPhase = fabs(fAudioL+fAudioR) > 0.000001 ? atan(fabs((fAudioL-fAudioR)/(fAudioL+fAudioR))) : PI_ON_2;
		} else {
			CurrentPhase =0;
		}
		EnvPhase += IEnvelope(CurrentPhase,EnvPhase,INVADA_METER_PHASE,plugin->SampleRate);

		drive = driveL > driveR ? driveL : driveR;
		EnvDrive += IEnvelope(drive,EnvDrive,INVADA_METER_LAMP,plugin->SampleRate);
	}

	// store values for next loop
	plugin->EnvInLLast = (fabs(EnvInL)<1.0e-10)  ? 0.f : EnvInL; 
	plugin->EnvInRLast = (fabs(EnvInR)<1.0e-10)  ? 0.f : EnvInR; 
	plugin->EnvOutLLast = (fabs(EnvOutL)<1.0e-10)  ? 0.f : EnvOutL; 
	plugin->EnvOutRLast = (fabs(EnvOutR)<1.0e-10)  ? 0.f : EnvOutR; 
	plugin->EnvPhaseLast = (fabs(EnvPhase)<1.0e-10)  ? 0.f : EnvPhase; 
	plugin->EnvDriveLast = (fabs(EnvDrive)<1.0e-10)  ? 0.f : EnvDrive; 

	// update the meters
	*(plugin->MeterInputL) =(EnvInL  > 0.001) ? 20*log10(EnvInL)  : -90.0;
	*(plugin->MeterInputR) =(EnvInR  > 0.001) ? 20*log10(EnvInR)  : -90.0;
	*(plugin->MeterOutputL)=(EnvOutL > 0.001) ? 20*log10(EnvOutL) : -90.0;
	*(plugin->MeterOutputR)=(EnvOutR > 0.001) ? 20*log10(EnvOutR) : -90.0;
	*(plugin->MeterPhase)=EnvPhase;
	*(plugin->MeterDrive)=EnvDrive;
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
			if(value<=0.0)
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
			if(value<=0.0)
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


