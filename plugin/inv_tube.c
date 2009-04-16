/* 

    This LV2 plugin provides tube distortion

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
#include "library/common.h"
#include "inv_tube.h"




static LV2_Descriptor *ITubeMonoDescriptor = NULL;
static LV2_Descriptor *ITubeStereoDescriptor = NULL;


typedef struct {
	float * ControlDrive;         
	float * ControlDcoffset;
	float * ControlPhase;  
	float * ControlMix; 
	float * AudioInputBufferL;
	float * AudioOutputBufferL;
	float * AudioInputBufferR; 
	float * AudioOutputBufferR;
	float * MeterInputL;
	float * MeterOutputL;
	float * MeterInputR; 
	float * MeterOutputR;

	double SampleRate; 

	/* params get remembered */
	float LastDrive;         
	float LastDcoffset;
	float LastPhase;         
	float LastMix;

	float ConvertedDrive;  
	float ConvertedDcoffset;
	float ConvertedPhase;  
	float ConvertedMix;

	/* stuff we need to remember between calls */
	float EnvInLLast; 
	float EnvOutLLast; 
	float EnvInRLast; 
	float EnvOutRLast; 
 
} ITube;



static LV2_Handle instantiateITube(const LV2_Descriptor *descriptor, double s_rate, const char *path, const LV2_Feature * const* features)
{
	ITube *plugin = (ITube *)malloc(sizeof(ITube));
	if(plugin==NULL)
		return NULL;

	/* set some initial params */
	plugin->SampleRate=s_rate;

	return (LV2_Handle)plugin;
}



static void connectPortITube(LV2_Handle instance, uint32_t port, void *data)
{
	ITube *plugin = (ITube *)instance;

	switch (port) {
		case ITUBE_DRIVE:
			plugin->ControlDrive = data;
			break;
		case ITUBE_DCOFFSET:
			plugin->ControlDcoffset  = data;
			break;
		case ITUBE_PHASE:
			plugin->ControlPhase = data;
			break;
		case ITUBE_MIX:
			plugin->ControlMix = data;
			break;
		case ITUBE_AUDIO_INL:
			plugin->AudioInputBufferL = data;
			break;
		case ITUBE_AUDIO_OUTL:
			plugin->AudioOutputBufferL = data;
			break;
		case ITUBE_AUDIO_INR:
			plugin->AudioInputBufferR = data;
			break;
		case ITUBE_AUDIO_OUTR:
			plugin->AudioOutputBufferR = data;
			break;
		case ITUBE_METER_INL:
			plugin->MeterInputL = data;
			break;
		case ITUBE_METER_INR:
			plugin->MeterInputR = data;
			break;
		case ITUBE_METER_OUTL:
			plugin->MeterOutputL = data;
			break;
		case ITUBE_METER_OUTR:
			plugin->MeterOutputR = data;
			break;
	}
}



static void activateITube(LV2_Handle instance) 
{

	ITube *plugin = (ITube *)instance;

	/* set some defaults */
	plugin->LastDrive=0;         
	plugin->LastDcoffset=0;
	plugin->LastPhase=0;         
	plugin->LastMix=75;

	plugin->EnvInLLast = 0; 
	plugin->EnvOutLLast = 0; 
	plugin->EnvInRLast = 0; 
	plugin->EnvOutRLast = 0; 

	plugin->ConvertedDrive    = convertParam(ITUBE_DRIVE,    plugin->LastDrive,     plugin->SampleRate);
	plugin->ConvertedDcoffset = convertParam(ITUBE_DCOFFSET, plugin->LastDcoffset,  plugin->SampleRate);
	plugin->ConvertedPhase    = convertParam(ITUBE_PHASE,    plugin->LastPhase,     plugin->SampleRate);
	plugin->ConvertedMix      = convertParam(ITUBE_MIX,      plugin->LastMix,       plugin->SampleRate);
}


static void runMonoITube(LV2_Handle instance, uint32_t SampleCount) 
{
	float (*pParamFunc)(unsigned long, float, double) = NULL;
	float * pfAudioInputL;
	float * pfAudioOutputL;
	float OutL,EnvInL,EnvOutL;
	float fAudioL, fDrive, fDCOffset, fPhase, fMix;
	uint32_t lSampleIndex;
			   
	ITube *plugin = (ITube *)instance;
	pParamFunc = &convertParam;

	/* check for any params changes */
	checkParamChange(ITUBE_DRIVE,    plugin->ControlDrive,    &(plugin->LastDrive),    &(plugin->ConvertedDrive),    plugin->SampleRate, pParamFunc);
	checkParamChange(ITUBE_DCOFFSET, plugin->ControlDcoffset, &(plugin->LastDcoffset), &(plugin->ConvertedDcoffset), plugin->SampleRate, pParamFunc);
	checkParamChange(ITUBE_PHASE,    plugin->ControlPhase,    &(plugin->LastPhase),    &(plugin->ConvertedPhase),    plugin->SampleRate, pParamFunc);
	checkParamChange(ITUBE_MIX,      plugin->ControlMix,      &(plugin->LastMix),      &(plugin->ConvertedMix),      plugin->SampleRate, pParamFunc);

	fDrive    = plugin->ConvertedDrive;
	fDCOffset = plugin->ConvertedDcoffset;
	fPhase    = plugin->ConvertedPhase;
	fMix      = plugin->ConvertedMix;	
			   
	pfAudioInputL = plugin->AudioInputBufferL;
	pfAudioOutputL = plugin->AudioOutputBufferL;
	EnvInL     = plugin->EnvInLLast;
	EnvOutL    = plugin->EnvOutLLast;
  
	for (lSampleIndex = 0; lSampleIndex < SampleCount; lSampleIndex++) {
		fAudioL=*(pfAudioInputL++);
		OutL= fPhase <= 0 ? 
				(fAudioL*(1-fMix)) + ITube_do(fAudioL + fDCOffset,fDrive)*fMix :
				(fAudioL*(1-fMix)) - ITube_do(fAudioL + fDCOffset,fDrive)*fMix ;
		*(pfAudioOutputL++) = OutL;

		//evelope on in and out for meters
		EnvInL  += IEnvelope(fAudioL, EnvInL, INVADA_METER_PEAK,plugin->SampleRate);
		EnvOutL += IEnvelope(OutL,EnvOutL,INVADA_METER_PEAK,plugin->SampleRate);
	}
	// remember for next time round  
	plugin->EnvInLLast = (fabs(EnvInL)<1.0e-10)  ? 0.f : EnvInL; 
	plugin->EnvOutLLast = (fabs(EnvOutL)<1.0e-10)  ? 0.f : EnvOutL; 

	// update the meters
	*(plugin->MeterInputL) =(EnvInL  > 0.001) ? 20*log10(EnvInL)  : -90.0;
	*(plugin->MeterOutputL)=(EnvOutL > 0.001) ? 20*log10(EnvOutL) : -90.0;
}

static void runStereoITube(LV2_Handle instance, uint32_t SampleCount) 
{
	float (*pParamFunc)(unsigned long, float, double) = NULL;
	float * pfAudioInputL;
	float * pfAudioInputR;
	float * pfAudioOutputL;
	float * pfAudioOutputR;
	float OutL,EnvInL,EnvOutL;
	float OutR,EnvInR,EnvOutR;
	float fAudioL, fAudioR, fDrive, fDCOffset, fPhase, fMix;
	uint32_t lSampleIndex;
			   
	ITube *plugin = (ITube *)instance;
	pParamFunc = &convertParam;
			   
	/* check for any params changes */
	checkParamChange(ITUBE_DRIVE,    plugin->ControlDrive,    &(plugin->LastDrive),    &(plugin->ConvertedDrive),    plugin->SampleRate, pParamFunc);
	checkParamChange(ITUBE_DCOFFSET, plugin->ControlDcoffset, &(plugin->LastDcoffset), &(plugin->ConvertedDcoffset), plugin->SampleRate, pParamFunc);
	checkParamChange(ITUBE_PHASE,    plugin->ControlPhase,    &(plugin->LastPhase),    &(plugin->ConvertedPhase),    plugin->SampleRate, pParamFunc);
	checkParamChange(ITUBE_MIX,      plugin->ControlMix,      &(plugin->LastMix),      &(plugin->ConvertedMix),      plugin->SampleRate, pParamFunc);

	fDrive    = plugin->ConvertedDrive;
	fDCOffset = plugin->ConvertedDcoffset;
	fPhase    = plugin->ConvertedPhase;
	fMix      = plugin->ConvertedMix;
  
	pfAudioInputL = plugin->AudioInputBufferL;
	pfAudioInputR = plugin->AudioInputBufferR;
	pfAudioOutputL = plugin->AudioOutputBufferL;
	pfAudioOutputR = plugin->AudioOutputBufferR;

	EnvInL     = plugin->EnvInLLast;
	EnvInR     = plugin->EnvInRLast;
	EnvOutL    = plugin->EnvOutLLast;
	EnvOutR    = plugin->EnvOutRLast;
  
	for (lSampleIndex = 0; lSampleIndex < SampleCount; lSampleIndex++) {
		fAudioL=*(pfAudioInputL++);
		OutL = fPhase <= 0 ? 
				(fAudioL*(1-fMix)) + (ITube_do(fAudioL + fDCOffset,fDrive)-(fDCOffset/2))*fMix :
				(fAudioL*(1-fMix)) - (ITube_do(fAudioL + fDCOffset,fDrive)-(fDCOffset/2))*fMix ;
		*(pfAudioOutputL++) = OutL;
		  
		fAudioR=*(pfAudioInputR++);
		OutR = fPhase <= 0 ? 
				(fAudioR*(1-fMix)) + (ITube_do(fAudioR + fDCOffset,fDrive)-(fDCOffset/2))*fMix :
				(fAudioR*(1-fMix)) - (ITube_do(fAudioR + fDCOffset,fDrive)-(fDCOffset/2))*fMix ;
		*(pfAudioOutputR++) = OutR;

		//evelope on in and out for meters
		EnvInL  += IEnvelope(fAudioL, EnvInL, INVADA_METER_PEAK,plugin->SampleRate);
		EnvInR  += IEnvelope(fAudioR, EnvInR, INVADA_METER_PEAK,plugin->SampleRate);
		EnvOutL += IEnvelope(OutL,EnvOutL,INVADA_METER_PEAK,plugin->SampleRate);
		EnvOutR += IEnvelope(OutR,EnvOutR,INVADA_METER_PEAK,plugin->SampleRate);

	}
	// store values for next loop
	plugin->EnvInLLast = (fabs(EnvInL)<1.0e-10)  ? 0.f : EnvInL; 
	plugin->EnvInRLast = (fabs(EnvInR)<1.0e-10)  ? 0.f : EnvInR; 
	plugin->EnvOutLLast = (fabs(EnvOutL)<1.0e-10)  ? 0.f : EnvOutL; 
	plugin->EnvOutRLast = (fabs(EnvOutR)<1.0e-10)  ? 0.f : EnvOutR; 

	// update the meters
	*(plugin->MeterInputL) =(EnvInL  > 0.001) ? 20*log10(EnvInL)  : -90.0;
	*(plugin->MeterInputR) =(EnvInR  > 0.001) ? 20*log10(EnvInR)  : -90.0;
	*(plugin->MeterOutputL)=(EnvOutL > 0.001) ? 20*log10(EnvOutL) : -90.0;
	*(plugin->MeterOutputR)=(EnvOutR > 0.001) ? 20*log10(EnvOutR) : -90.0;
}



static void cleanupITube(LV2_Handle instance)
{
	free(instance);
}


static void init()
{
	ITubeMonoDescriptor =
	 (LV2_Descriptor *)malloc(sizeof(LV2_Descriptor));

	ITubeMonoDescriptor->URI 		= ITUBE_MONO_URI;
	ITubeMonoDescriptor->activate 		= activateITube;
	ITubeMonoDescriptor->cleanup 		= cleanupITube;
	ITubeMonoDescriptor->connect_port 	= connectPortITube;
	ITubeMonoDescriptor->deactivate 	= NULL;
	ITubeMonoDescriptor->instantiate 	= instantiateITube;
	ITubeMonoDescriptor->run 		= runMonoITube;
	ITubeMonoDescriptor->extension_data 	= NULL;

	ITubeStereoDescriptor =
	 (LV2_Descriptor *)malloc(sizeof(LV2_Descriptor));
	ITubeStereoDescriptor->URI 		= ITUBE_STEREO_URI;
	ITubeStereoDescriptor->activate 	= activateITube;
	ITubeStereoDescriptor->cleanup 		= cleanupITube;
	ITubeStereoDescriptor->connect_port 	= connectPortITube;
	ITubeStereoDescriptor->deactivate 	= NULL;
	ITubeStereoDescriptor->instantiate 	= instantiateITube;
	ITubeStereoDescriptor->run 		= runStereoITube;
	ITubeStereoDescriptor->extension_data 	= NULL;
}


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
	if (!ITubeMonoDescriptor) init();

	switch (index) {
	case 0:
		return ITubeMonoDescriptor;
	case 1:
		return ITubeStereoDescriptor;
	default:
		return NULL;
	}
}


/*****************************************************************************/

float convertParam(unsigned long param, float value, double sr) {

	float result;

	switch(param)
	{
		case ITUBE_DRIVE:
			if(value<0)
				result= 1;
			else if (value < 18)
				result = pow(10,value/20.0);
			else
				result= pow(10,0.9);
			break;
		case ITUBE_DCOFFSET:
			if(value<-1)
				result= -1;
			else if (value < 0)
				result = -pow(value,2);
			else if (value < 1)
				result = pow(value,2);
			else
				result= 1;
			break;
		case ITUBE_PHASE:
			if(value<=0.0)
				result= 0; 
			else
				result= 1;
			break;
		case ITUBE_MIX:
			if(value<0)
				result= 0;
			else if (value < 100)
				result = value/100;
			else
				result= 1;
			break;
		default:
			result=0;
			break;
	}
	return result;
}

