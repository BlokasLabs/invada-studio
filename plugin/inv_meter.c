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
	float *ControlMeterMode;
	float *ControlSpecMode;
	float *AudioInputBufferL;
	float *AudioInputBufferR; 
	float *AudioOutputBufferL;
	float *AudioOutputBufferR;
	float *MeterL;
	float *MeterR; 
	float *MeterPhase;


	/* stuff we need to remember to reduce cpu */ 
	double SampleRate; 

	/* stuff we need to remember to reduce cpu */ 
	float LastBypass;
	float LastMeterMode;
	float LastSpecMode;
	float ConvertedBypass;
	float ConvertedMeterMode;
	float ConvertedSpecMode;

	float EnvMeterLLast; 
	float EnvMeterRLast; 
	float EnvSpecLast[31];
	float EnvPhaseLast; 

	/* filters */
	struct FilterP * filters;

} IMeter;

static LV2_Handle 
instantiateIMeter(const LV2_Descriptor *descriptor, double s_rate, const char *path, const LV2_Feature * const* features)
{
	IMeter *plugin = (IMeter *)malloc(sizeof(IMeter));
	if(plugin==NULL)
		return NULL;

	/* set some initial params */
	plugin->SampleRate=s_rate;

	/* the delays */
	if((plugin->filters  = (struct FilterP *)malloc(sizeof(struct FilterP) * FILTER_COUNT))==NULL)
    		return NULL;	

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
		case IMETER_METER_MODE:
			plugin->ControlMeterMode = data;
			break;
		case IMETER_SPEC_MODE:
			plugin->ControlSpecMode = data;
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
		case IMETER_METER_L:
			plugin->MeterL = data;
			break;
		case IMETER_METER_R:
			plugin->MeterR = data;
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
	int i;

	// these values force the conversion to take place      

	plugin->LastBypass = 0;
	plugin->LastMeterMode = 0;
	plugin->LastSpecMode = 0;
	plugin->EnvMeterLLast = 0; 
	plugin->EnvMeterRLast = 0; 
	plugin->EnvPhaseLast = 0; 

	for(i=0;i<31;i++) {
		plugin->EnvSpecLast[i] = 0;
	}

	plugin->ConvertedBypass    = convertParam(IMETER_BYPASS,     plugin->LastBypass,     plugin->SampleRate);
	plugin->ConvertedMeterMode = convertParam(IMETER_METER_MODE, plugin->LastMeterMode,  plugin->SampleRate);
	plugin->ConvertedSpecMode  = convertParam(IMETER_SPEC_MODE,  plugin->LastSpecMode,   plugin->SampleRate);

	/* initialise filters */
	initBandpassFilter(&plugin->filters[0],  plugin->SampleRate,   20.0, 0.3);
	initBandpassFilter(&plugin->filters[31], plugin->SampleRate,   20.0, 0.3);

	initBandpassFilter(&plugin->filters[1],  plugin->SampleRate,   25.0, 0.3);
	initBandpassFilter(&plugin->filters[32], plugin->SampleRate,   25.0, 0.3);

	initBandpassFilter(&plugin->filters[2],  plugin->SampleRate,   31.5, 0.3);
	initBandpassFilter(&plugin->filters[33], plugin->SampleRate,   31.5, 0.3);

	initBandpassFilter(&plugin->filters[3],  plugin->SampleRate,   40.0, 0.3);
	initBandpassFilter(&plugin->filters[34], plugin->SampleRate,   40.0, 0.3);

	initBandpassFilter(&plugin->filters[4],  plugin->SampleRate,   50.0, 0.3);
	initBandpassFilter(&plugin->filters[35], plugin->SampleRate,   50.0, 0.3);

	initBandpassFilter(&plugin->filters[5],  plugin->SampleRate,   63.0, 0.3);
	initBandpassFilter(&plugin->filters[36], plugin->SampleRate,   63.0, 0.3);

	initBandpassFilter(&plugin->filters[6],  plugin->SampleRate,   80.0, 0.3);
	initBandpassFilter(&plugin->filters[37], plugin->SampleRate,   80.0, 0.3);

	initBandpassFilter(&plugin->filters[7],  plugin->SampleRate,  100.0, 0.3);
	initBandpassFilter(&plugin->filters[38], plugin->SampleRate,  100.0, 0.3);

	initBandpassFilter(&plugin->filters[8],  plugin->SampleRate,  125.0, 0.3);
	initBandpassFilter(&plugin->filters[39], plugin->SampleRate,  125.0, 0.3);

	initBandpassFilter(&plugin->filters[9],  plugin->SampleRate,  160.0, 0.3);
	initBandpassFilter(&plugin->filters[40], plugin->SampleRate,  160.0, 0.3);

	initBandpassFilter(&plugin->filters[10], plugin->SampleRate,  200.0, 0.3);
	initBandpassFilter(&plugin->filters[41], plugin->SampleRate,  200.0, 0.3);

	initBandpassFilter(&plugin->filters[11], plugin->SampleRate,  250.0, 0.3);
	initBandpassFilter(&plugin->filters[42], plugin->SampleRate,  250.0, 0.3);

	initBandpassFilter(&plugin->filters[12], plugin->SampleRate,  315.0, 0.3);
	initBandpassFilter(&plugin->filters[43], plugin->SampleRate,  315.0, 0.3);

	initBandpassFilter(&plugin->filters[13], plugin->SampleRate,  400.0, 0.3);
	initBandpassFilter(&plugin->filters[44], plugin->SampleRate,  400.0, 0.3);

	initBandpassFilter(&plugin->filters[14], plugin->SampleRate,  500.0, 0.3);
	initBandpassFilter(&plugin->filters[45], plugin->SampleRate,  500.0, 0.3);

	initBandpassFilter(&plugin->filters[15], plugin->SampleRate,  630.0, 0.3);
	initBandpassFilter(&plugin->filters[46], plugin->SampleRate,  630.0, 0.3);

	initBandpassFilter(&plugin->filters[16], plugin->SampleRate,  800.0, 0.3);
	initBandpassFilter(&plugin->filters[47], plugin->SampleRate,  800.0, 0.3);

	initBandpassFilter(&plugin->filters[17], plugin->SampleRate, 1000.0, 0.3);
	initBandpassFilter(&plugin->filters[48], plugin->SampleRate, 1000.0, 0.3);

	initBandpassFilter(&plugin->filters[18], plugin->SampleRate, 1250.0, 0.3);
	initBandpassFilter(&plugin->filters[49], plugin->SampleRate, 1250.0, 0.3);

	initBandpassFilter(&plugin->filters[19], plugin->SampleRate, 1600.0, 0.3);
	initBandpassFilter(&plugin->filters[50], plugin->SampleRate, 1600.0, 0.3);

	initBandpassFilter(&plugin->filters[20], plugin->SampleRate, 2000.0, 0.3);
	initBandpassFilter(&plugin->filters[51], plugin->SampleRate, 2000.0, 0.3);

	initBandpassFilter(&plugin->filters[21], plugin->SampleRate, 2500.0, 0.3);
	initBandpassFilter(&plugin->filters[52], plugin->SampleRate, 2500.0, 0.3);

	initBandpassFilter(&plugin->filters[22], plugin->SampleRate, 3150.0, 0.3);
	initBandpassFilter(&plugin->filters[53], plugin->SampleRate, 3150.0, 0.3);

	initBandpassFilter(&plugin->filters[23], plugin->SampleRate, 4000.0, 0.3);
	initBandpassFilter(&plugin->filters[54], plugin->SampleRate, 4000.0, 0.3);

	initBandpassFilter(&plugin->filters[24], plugin->SampleRate, 5000.0, 0.3);
	initBandpassFilter(&plugin->filters[55], plugin->SampleRate, 5000.0, 0.3);

	initBandpassFilter(&plugin->filters[25], plugin->SampleRate, 6300.0, 0.3);
	initBandpassFilter(&plugin->filters[56], plugin->SampleRate, 6300.0, 0.3);

	initBandpassFilter(&plugin->filters[26], plugin->SampleRate, 8000.0, 0.3);
	initBandpassFilter(&plugin->filters[57], plugin->SampleRate, 8000.0, 0.3);

	initBandpassFilter(&plugin->filters[27], plugin->SampleRate, 10000.0, 0.3);
	initBandpassFilter(&plugin->filters[58], plugin->SampleRate, 10000.0, 0.3);

	initBandpassFilter(&plugin->filters[28], plugin->SampleRate, 12500.0, 0.3);
	initBandpassFilter(&plugin->filters[59], plugin->SampleRate, 12500.0, 0.3);

	initBandpassFilter(&plugin->filters[29], plugin->SampleRate, 16000.0, 0.3);
	initBandpassFilter(&plugin->filters[60], plugin->SampleRate, 16000.0, 0.3);

	initBandpassFilter(&plugin->filters[30], plugin->SampleRate, 20000.0, 0.3);
	initBandpassFilter(&plugin->filters[61], plugin->SampleRate, 20000.0, 0.3);

}


static void 
runIMeter(LV2_Handle instance, uint32_t SampleCount) 
{
	float (*pParamFunc)(unsigned long, float, double) = NULL;
	float * pfAudioInputL;
	float * pfAudioInputR;
	float * pfAudioOutputL;
	float * pfAudioOutputR;
	float fBypass,fMeterMode,fSpecMode;
	float In;
	float InL,EnvMeterL;
	float InR,EnvMeterR;
	float EnvPhase,CurrentPhase;
	float EnvSpec[31];
	int i;
	struct FilterP *filter;
	uint32_t lSampleIndex;

	IMeter *plugin = (IMeter *)instance;

	pParamFunc = &convertParam;

	checkParamChange(IMETER_BYPASS,     plugin->ControlBypass,    &(plugin->LastBypass),    &(plugin->ConvertedBypass),    plugin->SampleRate, pParamFunc);
	checkParamChange(IMETER_METER_MODE, plugin->ControlMeterMode, &(plugin->LastMeterMode), &(plugin->ConvertedMeterMode), plugin->SampleRate, pParamFunc);
	checkParamChange(IMETER_SPEC_MODE,  plugin->ControlSpecMode,  &(plugin->LastSpecMode),  &(plugin->ConvertedSpecMode),  plugin->SampleRate, pParamFunc);


	fBypass    	= plugin->ConvertedBypass;
	fMeterMode    	= plugin->ConvertedMeterMode;
	fSpecMode    	= plugin->ConvertedSpecMode;

	pfAudioInputL 	= plugin->AudioInputBufferL;
	pfAudioInputR 	= plugin->AudioInputBufferR;
	pfAudioOutputL 	= plugin->AudioOutputBufferL;
	pfAudioOutputR 	= plugin->AudioOutputBufferR;

	EnvMeterL     	= plugin->EnvMeterLLast;
	EnvMeterR     	= plugin->EnvMeterRLast;
	EnvPhase  	= plugin->EnvPhaseLast;
	
	for(i=0;i<32;i++) {
		EnvSpec[i]=plugin->EnvSpecLast[i];
	}
 
	if(fBypass==0) { 
		for (lSampleIndex = 0; lSampleIndex < SampleCount; lSampleIndex++) {

			InL=*(pfAudioInputL++);
			InR=*(pfAudioInputR++);
			
			In=(InL+InR)/2;

			*(pfAudioOutputL++) = InL;
			*(pfAudioOutputR++) = InR;

			//evelope on in and out for meters
			if(fMeterMode<0.5) {
				EnvMeterL  	+= IEnvelope(InL, EnvMeterL, INVADA_METER_PEAK,  plugin->SampleRate);
				EnvMeterR  	+= IEnvelope(InR, EnvMeterR, INVADA_METER_PEAK,  plugin->SampleRate);
			} else {
				EnvMeterL  	+= IEnvelope(InL, EnvMeterL, INVADA_METER_VU,  plugin->SampleRate);
				EnvMeterR  	+= IEnvelope(InR, EnvMeterR, INVADA_METER_VU,  plugin->SampleRate);
			}

			//envelope for phase
			if(fabs(InL) > 0.001 || fabs(InR) > 0.001) {  // -60 db
				CurrentPhase = fabs(InL+InR) > 0.000001 ? atan(fabs((InL-InR)/(InL+InR))) : PI_ON_2;
			} else {
				CurrentPhase =0;
			}
			EnvPhase += IEnvelope(CurrentPhase,EnvPhase,INVADA_METER_PHASE,plugin->SampleRate);

			//envelop for spectrum
			filter=plugin->filters;
			for(i=0;i<32;i++) {
				if(fSpecMode< 0.5) {
					EnvSpec[i]+=IEnvelope(applyBandpassFilter(&filter[i],In), EnvSpec[i], INVADA_METER_PEAK,  plugin->SampleRate);
				} else {
					EnvSpec[i]+=IEnvelope(applyBandpassFilter(&filter[31+i],applyBandpassFilter(&filter[i],In)), EnvSpec[i], INVADA_METER_PEAK,  plugin->SampleRate);
				}
			}

		}
	} else {
		for (lSampleIndex = 0; lSampleIndex < SampleCount; lSampleIndex++) {

			*(pfAudioOutputL++) = *(pfAudioInputL++);
			*(pfAudioOutputR++) = *(pfAudioInputR++);
		}
		//evelope on in and out for meters
		EnvMeterL  	=0;
		EnvMeterR  	=0;
		EnvPhase 	=0;
		for(i=0;i<32;i++) {
			EnvSpec[i]=0;
		}
	}

	// store values for next loop
	plugin->EnvMeterLLast 	= (fabs(EnvMeterL)<1.0e-10)  ? 0.f : EnvMeterL; 
	plugin->EnvMeterRLast 	= (fabs(EnvMeterR)<1.0e-10)  ? 0.f : EnvMeterR; 
	plugin->EnvPhaseLast 	= (fabs(EnvPhase)<1.0e-10)  ? 0.f : EnvPhase; 
	for(i=0;i<32;i++) {
		plugin->EnvSpecLast[i] = (fabs(EnvSpec[i])<1.0e-10)  ? 0.f : EnvSpec[i];
	}

	// update the meters
	*(plugin->MeterL) 	= (EnvMeterL  > 0.001) ? 20*log10(EnvMeterL)  : -90.0;
	*(plugin->MeterR) 	= (EnvMeterR  > 0.001) ? 20*log10(EnvMeterR)  : -90.0;
	*(plugin->MeterPhase)	= EnvPhase;
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
		case IMETER_METER_MODE:
		case IMETER_SPEC_MODE:
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


