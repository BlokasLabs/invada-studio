/* 

    This LV2 plugin provides meters

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
#include "inv_meter.h"


static LV2_Descriptor *IMeterDescriptor = NULL;


typedef struct {

	/* Ports */
	float *ControlBypass;
	float *AudioInputBufferL;
	float *AudioInputBufferR; 
	float *AudioOutputBufferL;
	float *AudioOutputBufferR;
	float *MeterPeakL;
	float *MeterPeakR; 
	float *MeterVuL;
	float *MeterVuR;
	float *MeterPhase;


	/* stuff we need to remember to reduce cpu */ 
	double SampleRate; 

	/* stuff we need to remember to reduce cpu */ 
	float LastBypass;
	float ConvertedBypass;

	float EnvPeakLLast; 
	float EnvPeakRLast; 
	float EnvVuLLast; 
	float EnvVuRLast; 
	float EnvPhaseLast; 

} IMeter;

static LV2_Handle 
instantiateIMeter(const LV2_Descriptor *descriptor, double s_rate, const char *path, const LV2_Feature * const* features)
{
	IMeter *plugin = (IMeter *)malloc(sizeof(IMeter));
	if(plugin==NULL)
		return NULL;

	/* set some initial params */
	plugin->SampleRate=s_rate;

	return (LV2_Handle)plugin;
}



static void 
connectPortIMeter(LV2_Handle instance, uint32_t port, void *data)
{
	IMeter *plugin = (IMeter *)instance;

	switch (port) {
		case IMETER_BYPASS:
			plugin->ControlBypass = data;
			break;
		case IMETER_AUDIO_INL:
			plugin->AudioInputBufferL = data;
			break;
		case IMETER_AUDIO_INR:
			plugin->AudioInputBufferR = data;
			break;
		case IMETER_AUDIO_OUTL:
			plugin->AudioOutputBufferL = data;
			break;
		case IMETER_AUDIO_OUTR:
			plugin->AudioOutputBufferR = data;
			break;
		case IMETER_METER_PEAK_L:
			plugin->MeterPeakL = data;
			break;
		case IMETER_METER_PEAK_R:
			plugin->MeterPeakR = data;
			break;
		case IMETER_METER_VU_L:
			plugin->MeterVuL = data;
			break;
		case IMETER_METER_VU_R:
			plugin->MeterVuR = data;
			break;
		case IMETER_METER_PHASE:
			plugin->MeterPhase = data;
			break;
	}
}


static void 
activateIMeter(LV2_Handle instance) 
{

	IMeter *plugin = (IMeter *)instance;

	// these values force the conversion to take place      

	plugin->LastBypass = 0;
	plugin->EnvPeakLLast = 0; 
	plugin->EnvPeakRLast = 0; 
	plugin->EnvVuLLast = 0; 
	plugin->EnvVuRLast = 0; 
	plugin->EnvPhaseLast = 0; 

	plugin->ConvertedBypass = convertParam(IMETER_BYPASS, plugin->LastBypass,  plugin->SampleRate);
}


static void 
runIMeter(LV2_Handle instance, uint32_t SampleCount) 
{
	float (*pParamFunc)(unsigned long, float, double) = NULL;
	float * pfAudioInputL;
	float * pfAudioInputR;
	float * pfAudioOutputL;
	float * pfAudioOutputR;
	float fBypass;
	float InL,EnvPeakL,EnvVuL,InR,EnvPeakR,EnvVuR,EnvPhase,CurrentPhase;
	uint32_t lSampleIndex;

	IMeter *plugin = (IMeter *)instance;

	pParamFunc = &convertParam;

	checkParamChange(IMETER_BYPASS, plugin->ControlBypass, &(plugin->LastBypass), &(plugin->ConvertedBypass), plugin->SampleRate, pParamFunc);


	fBypass    	= plugin->ConvertedBypass;

	pfAudioInputL 	= plugin->AudioInputBufferL;
	pfAudioInputR 	= plugin->AudioInputBufferR;
	pfAudioOutputL 	= plugin->AudioOutputBufferL;
	pfAudioOutputR 	= plugin->AudioOutputBufferR;

	EnvPeakL     	= plugin->EnvPeakLLast;
	EnvPeakR     	= plugin->EnvPeakRLast;
	EnvVuL    	= plugin->EnvVuLLast;
	EnvVuR    	= plugin->EnvVuRLast;
	EnvPhase  	= plugin->EnvPhaseLast;

 
	if(fBypass==0) { 
		for (lSampleIndex = 0; lSampleIndex < SampleCount; lSampleIndex++) {

			InL=*(pfAudioInputL++);
			InR=*(pfAudioInputR++);

			*(pfAudioOutputL++) = InL;
			*(pfAudioOutputR++) = InR;

			//evelope on in and out for meters
			EnvPeakL  	+= IEnvelope(InL, EnvPeakL, INVADA_METER_PEAK,  plugin->SampleRate);
			EnvPeakR  	+= IEnvelope(InR, EnvPeakR, INVADA_METER_PEAK,  plugin->SampleRate);
			EnvVuL 		+= IEnvelope(InL, EnvVuL,    INVADA_METER_VU,   plugin->SampleRate);
			EnvVuR 		+= IEnvelope(InR, EnvVuR,    INVADA_METER_VU,   plugin->SampleRate);

			if(fabs(InL) > 0.001 || fabs(InR) > 0.001) {  // -60 db
				CurrentPhase = fabs(InL+InR) > 0.000001 ? atan(fabs((InL-InR)/(InL+InR))) : PI_ON_2;
			} else {
				CurrentPhase =0;
			}
			EnvPhase += IEnvelope(CurrentPhase,EnvPhase,INVADA_METER_PHASE,plugin->SampleRate);

		}
	} else {
		for (lSampleIndex = 0; lSampleIndex < SampleCount; lSampleIndex++) {

			*(pfAudioOutputL++) = *(pfAudioInputL++);
			*(pfAudioOutputR++) = *(pfAudioInputR++);
		}
		//evelope on in and out for meters
		EnvPeakL  	=0;
		EnvPeakR  	=0;
		EnvVuL 		=0;
		EnvVuR 		=0;
		EnvPhase 	=0;
	}

	// store values for next loop
	plugin->EnvPeakLLast 	= (fabs(EnvPeakL)<1.0e-10)  ? 0.f : EnvPeakL; 
	plugin->EnvPeakRLast 	= (fabs(EnvPeakR)<1.0e-10)  ? 0.f : EnvPeakR; 
	plugin->EnvVuLLast 	= (fabs(EnvVuL)<1.0e-10)  ? 0.f : EnvVuL; 
	plugin->EnvVuRLast 	= (fabs(EnvVuR)<1.0e-10)  ? 0.f : EnvVuR; 
	plugin->EnvPhaseLast 	= (fabs(EnvPhase)<1.0e-10)  ? 0.f : EnvPhase; 

	// update the meters
	*(plugin->MeterPeakL) 	=(EnvPeakL  > 0.001) ? 20*log10(EnvPeakL)  : -90.0;
	*(plugin->MeterPeakR) 	=(EnvPeakR  > 0.001) ? 20*log10(EnvPeakR)  : -90.0;
	*(plugin->MeterVuL)	=(EnvVuL > 0.001) ? 20*log10(EnvVuL) : -90.0;
	*(plugin->MeterVuR)	=(EnvVuR > 0.001) ? 20*log10(EnvVuR) : -90.0;
	*(plugin->MeterPhase)	=EnvPhase;
}


static void 
cleanupIMeter(LV2_Handle instance)
{
	free(instance);
}


static void 
init()
{
	IMeterDescriptor =
	 (LV2_Descriptor *)malloc(sizeof(LV2_Descriptor));

	IMeterDescriptor->URI 		= IMETER_URI;
	IMeterDescriptor->activate 	= activateIMeter;
	IMeterDescriptor->cleanup 	= cleanupIMeter;
	IMeterDescriptor->connect_port 	= connectPortIMeter;
	IMeterDescriptor->deactivate 	= NULL;
	IMeterDescriptor->instantiate 	= instantiateIMeter;
	IMeterDescriptor->run 		= runIMeter;
	IMeterDescriptor->extension_data = NULL;
}


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
	if (!IMeterDescriptor) init();

	switch (index) {
	case 0:
		return IMeterDescriptor;
	default:
		return NULL;
	}
}


/*****************************************************************************/


float 
convertParam(unsigned long param, float value, double sr) {
	float result;
	switch(param)  {
		case IMETER_BYPASS:
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


