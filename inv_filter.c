/* 

    This LV2 plugin provides mono and stereo filters

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
#include "inv_filter.h"


#define IFILTER_MONO_LPF_URI	"http://invadarecords.com/plugins/lv2/filter/lpf/mono";
#define IFILTER_STEREO_LPF_URI	"http://invadarecords.com/plugins/lv2/filter/lpf/stereo";
#define IFILTER_MONO_HPF_URI	"http://invadarecords.com/plugins/lv2/filter/hpf/mono";
#define IFILTER_STEREO_HPF_URI	"http://invadarecords.com/plugins/lv2/filter/hpf/stereo";
#define IFILTER_FREQ 		0
#define IFILTER_GAIN 		1
#define IFILTER_NOCLIP 		2
#define IFILTER_AUDIO_INL  	3
#define IFILTER_AUDIO_OUTL 	4
#define IFILTER_AUDIO_INR  	5 /* not used in mono mode */
#define IFILTER_AUDIO_OUTR 	6 /* not used in mono mode */


static LV2_Descriptor *IFilterMonoLPFDescriptor = NULL;
static LV2_Descriptor *IFilterStereoLPFDescriptor = NULL;
static LV2_Descriptor *IFilterMonoHPFDescriptor = NULL;
static LV2_Descriptor *IFilterStereoHPFDescriptor = NULL;


typedef struct {
 
	/* Ports */
	float * ControlFreq;         
	float * ControlGain;
	float * ControlNoClip;  
	float * AudioInputBufferL;
	float * AudioOutputBufferL;
	float * AudioInputBufferR; 
	float * AudioOutputBufferR;

	double SampleRate;

	/* stuff we need to remember */
	float LastFreq;         
	float LastGain;
	float LastNoClip;

	float ConvertedFreq;         
	float ConvertedGain;
	float ConvertedNoClip;

	/* stuff we need to remember between calls */
	float AudioLLast; 
	float AudioRLast;

} IFilter;


static LV2_Handle instantiateIFilter(const LV2_Descriptor *descriptor, double s_rate, const char *path, const LV2_Feature * const* features)
{
	IFilter *plugin = (IFilter *)malloc(sizeof(IFilter));
	if(plugin==NULL)
		return NULL;

	/* set some initial params */
	plugin->SampleRate=s_rate;

	return (LV2_Handle)plugin;
}


static void connectPortIFilter(LV2_Handle instance, uint32_t port, void *data)
{
	IFilter *plugin = (IFilter *)instance;
	switch (port) {
		case IFILTER_FREQ:
			plugin->ControlFreq = data;
			break;
		case IFILTER_GAIN:
			plugin->ControlGain  = data;
			break;
		case IFILTER_NOCLIP:
			plugin->ControlNoClip = data;
			break;
		case IFILTER_AUDIO_INL:
			plugin->AudioInputBufferL = data;
			break;
		case IFILTER_AUDIO_OUTL:
			plugin->AudioOutputBufferL = data;
			break;
		case IFILTER_AUDIO_INR:
			plugin->AudioInputBufferR = data;
			break;
		case IFILTER_AUDIO_OUTR:
			plugin->AudioOutputBufferR = data;
			break;
	}
}


static void activateIFilter(LV2_Handle instance) 
{

	IFilter *plugin = (IFilter *)instance;

	plugin->AudioLLast = 0;
	plugin->AudioRLast = 0;

	/* defaults */
	plugin->LastFreq = 0.015811388;   // middle on a logarithmic scale      
	plugin->LastGain = 0;
	plugin->LastNoClip = 0;

	plugin->ConvertedFreq   = convertParam(IFILTER_FREQ,   plugin->LastFreq,    plugin->SampleRate);
	plugin->ConvertedGain   = convertParam(IFILTER_GAIN,   plugin->LastGain,    plugin->SampleRate);
	plugin->ConvertedNoClip = convertParam(IFILTER_NOCLIP, plugin->LastNoClip,  plugin->SampleRate);
}


static void runMonoLPFIFilter(LV2_Handle instance, uint32_t SampleCount) 
{
	float (*pParamFunc)(unsigned long, float, double) = NULL;
	float * pfAudioInputL;
	float * pfAudioOutputL;
	float fSamples,fGain,fNoClip;
	float fAudioL,fAudioLSum;
	unsigned long lSampleIndex;

	IFilter *plugin = (IFilter *)instance;
	pParamFunc = &convertParam;

	checkParamChange(IFILTER_FREQ,   plugin->ControlFreq,   &(plugin->LastFreq),   &(plugin->ConvertedFreq),   plugin->SampleRate, pParamFunc);
	checkParamChange(IFILTER_GAIN,   plugin->ControlGain,   &(plugin->LastGain),   &(plugin->ConvertedGain),   plugin->SampleRate, pParamFunc);
	checkParamChange(IFILTER_NOCLIP, plugin->ControlNoClip, &(plugin->LastNoClip), &(plugin->ConvertedNoClip), plugin->SampleRate, pParamFunc);

	fSamples = plugin->ConvertedFreq;
	fGain    = plugin->ConvertedGain;
	fNoClip  = plugin->ConvertedNoClip;

	pfAudioInputL = plugin->AudioInputBufferL;
	pfAudioOutputL = plugin->AudioOutputBufferL;

	fAudioLSum = plugin->AudioLLast;
  
	for (lSampleIndex = 0; lSampleIndex < SampleCount; lSampleIndex++) 
	{
		fAudioLSum = ((fSamples-1) * fAudioLSum + *(pfAudioInputL++)) / fSamples;  
		fAudioL = fAudioLSum*fGain; 
		*(pfAudioOutputL++)=fNoClip > 0 ? InoClip(fAudioL) : fAudioL;  
	}
  
	plugin->AudioLLast = (fabs(fAudioLSum)<1.0e-10)  ? 0.f : fAudioLSum;  // and store values for next loop
}


static void runStereoLPFIFilter(LV2_Handle instance, uint32_t SampleCount) 
{
	float (*pParamFunc)(unsigned long, float, double) = NULL;
	float * pfAudioInputL;
	float * pfAudioInputR;
	float * pfAudioOutputL;
	float * pfAudioOutputR;
	float fSamples,fGain,fNoClip;
	float fAudioL,fAudioR,fAudioLSum,fAudioRSum;
	unsigned long lSampleIndex;

	IFilter *plugin = (IFilter *)instance;
	pParamFunc = &convertParam;

	checkParamChange(IFILTER_FREQ,   plugin->ControlFreq,   &(plugin->LastFreq),   &(plugin->ConvertedFreq),   plugin->SampleRate, pParamFunc);
	checkParamChange(IFILTER_GAIN,   plugin->ControlGain,   &(plugin->LastGain),   &(plugin->ConvertedGain),   plugin->SampleRate, pParamFunc);
	checkParamChange(IFILTER_NOCLIP, plugin->ControlNoClip, &(plugin->LastNoClip), &(plugin->ConvertedNoClip), plugin->SampleRate, pParamFunc);

	fSamples = plugin->ConvertedFreq;
	fGain    = plugin->ConvertedGain;
	fNoClip  = plugin->ConvertedNoClip;

	pfAudioInputL = plugin->AudioInputBufferL;
	pfAudioInputR = plugin->AudioInputBufferR;
	pfAudioOutputL = plugin->AudioOutputBufferL;
	pfAudioOutputR = plugin->AudioOutputBufferR;

	fAudioLSum = plugin->AudioLLast;
	fAudioRSum = plugin->AudioRLast;

	for (lSampleIndex = 0; lSampleIndex < SampleCount; lSampleIndex++) {
		fAudioLSum = ((fSamples-1) * fAudioLSum + *(pfAudioInputL++)) / fSamples;  
		fAudioRSum = ((fSamples-1) * fAudioRSum + *(pfAudioInputR++)) / fSamples;
		  
		fAudioL = fAudioLSum*fGain; 
		fAudioR = fAudioRSum*fGain; 
		  
		*(pfAudioOutputL++)=fNoClip > 0 ? InoClip(fAudioL) : fAudioL;  
		*(pfAudioOutputR++)=fNoClip > 0 ? InoClip(fAudioR) : fAudioR;	
	}
  
	plugin->AudioLLast = (fabs(fAudioLSum)<1.0e-10)  ? 0.f : fAudioLSum;  // and store values for next loop
	plugin->AudioRLast = (fabs(fAudioRSum)<1.0e-10)  ? 0.f : fAudioRSum;  // and store values for next loop
}



static void runMonoHPFIFilter(LV2_Handle instance, uint32_t SampleCount) 
{
	float (*pParamFunc)(unsigned long, float, double) = NULL;
	float * pfAudioInputL;
	float * pfAudioOutputL;
	float fSamples,fGain,fNoClip;
	float fAudioL,fAudioLSum;
	unsigned long lSampleIndex;

	IFilter *plugin = (IFilter *)instance;
	pParamFunc = &convertParam;

	checkParamChange(IFILTER_FREQ,   plugin->ControlFreq,   &(plugin->LastFreq),   &(plugin->ConvertedFreq),   plugin->SampleRate, pParamFunc);
	checkParamChange(IFILTER_GAIN,   plugin->ControlGain,   &(plugin->LastGain),   &(plugin->ConvertedGain),   plugin->SampleRate, pParamFunc);
	checkParamChange(IFILTER_NOCLIP, plugin->ControlNoClip, &(plugin->LastNoClip), &(plugin->ConvertedNoClip), plugin->SampleRate, pParamFunc);

	fSamples = plugin->ConvertedFreq;
	fGain    = plugin->ConvertedGain;
	fNoClip  = plugin->ConvertedNoClip;

	pfAudioInputL = plugin->AudioInputBufferL;
	pfAudioOutputL = plugin->AudioOutputBufferL;

	fAudioLSum = plugin->AudioLLast;

	for (lSampleIndex = 0; lSampleIndex < SampleCount; lSampleIndex++) 
	{
		fAudioL = *(pfAudioInputL++);
		fAudioLSum = ((fSamples-1) * fAudioLSum + fAudioL) / fSamples;  
		fAudioL = (fAudioL - fAudioLSum)*fGain;
		*(pfAudioOutputL++)=fNoClip > 0 ? InoClip(fAudioL) : fAudioL;  
	}

	plugin->AudioLLast = (fabs(fAudioLSum)<1.0e-10)  ? 0.f : fAudioLSum;  // and store values for next loop
}


static void runStereoHPFIFilter(LV2_Handle instance, uint32_t SampleCount) 
{
	float (*pParamFunc)(unsigned long, float, double) = NULL;
	float * pfAudioInputL;
	float * pfAudioInputR;
	float * pfAudioOutputL;
	float * pfAudioOutputR;
	float fSamples,fGain,fNoClip;
	float fAudioL,fAudioR,fAudioLSum,fAudioRSum;
	unsigned long lSampleIndex;

	IFilter *plugin = (IFilter *)instance;
	pParamFunc = &convertParam;

	checkParamChange(IFILTER_FREQ,   plugin->ControlFreq,   &(plugin->LastFreq),   &(plugin->ConvertedFreq),   plugin->SampleRate, pParamFunc);
	checkParamChange(IFILTER_GAIN,   plugin->ControlGain,   &(plugin->LastGain),   &(plugin->ConvertedGain),   plugin->SampleRate, pParamFunc);
	checkParamChange(IFILTER_NOCLIP, plugin->ControlNoClip, &(plugin->LastNoClip), &(plugin->ConvertedNoClip), plugin->SampleRate, pParamFunc);

	fSamples = plugin->ConvertedFreq;
	fGain    = plugin->ConvertedGain;
	fNoClip  = plugin->ConvertedNoClip;

	pfAudioInputL = plugin->AudioInputBufferL;
	pfAudioInputR = plugin->AudioInputBufferR;
	pfAudioOutputL = plugin->AudioOutputBufferL;
	pfAudioOutputR = plugin->AudioOutputBufferR;

	fAudioLSum = plugin->AudioLLast;
	fAudioRSum = plugin->AudioRLast;
  
	for (lSampleIndex = 0; lSampleIndex < SampleCount; lSampleIndex++) {
		fAudioL = *(pfAudioInputL++);
		fAudioR = *(pfAudioInputR++);
		  
		fAudioLSum = ((fSamples-1) * fAudioLSum + fAudioL) / fSamples;  
		fAudioRSum = ((fSamples-1) * fAudioRSum + fAudioR) / fSamples;
		  
		fAudioL = (fAudioL - fAudioLSum)*fGain;
		fAudioR = (fAudioR - fAudioRSum)*fGain;
		
		*(pfAudioOutputL++)=fNoClip > 0 ? InoClip(fAudioL) : fAudioL;
		*(pfAudioOutputR++)=fNoClip > 0 ? InoClip(fAudioR) : fAudioR;	  
	}

	plugin->AudioLLast = (fabs(fAudioLSum)<1.0e-10)  ? 0.f : fAudioLSum;  // and store values for next loop
	plugin->AudioRLast = (fabs(fAudioRSum)<1.0e-10)  ? 0.f : fAudioRSum;  // and store values for next loop
}


static void cleanupIFilter(LV2_Handle instance)
{
	free(instance);
}


static void init()
{
	IFilterMonoLPFDescriptor =
	 (LV2_Descriptor *)malloc(sizeof(LV2_Descriptor));

	IFilterMonoLPFDescriptor->URI 			= IFILTER_MONO_LPF_URI;
	IFilterMonoLPFDescriptor->activate 		= activateIFilter;
	IFilterMonoLPFDescriptor->cleanup 		= cleanupIFilter;
	IFilterMonoLPFDescriptor->connect_port 		= connectPortIFilter;
	IFilterMonoLPFDescriptor->deactivate 		= NULL;
	IFilterMonoLPFDescriptor->instantiate 		= instantiateIFilter;
	IFilterMonoLPFDescriptor->run 			= runMonoLPFIFilter;
	IFilterMonoLPFDescriptor->extension_data 	= NULL;

	IFilterStereoLPFDescriptor =
	 (LV2_Descriptor *)malloc(sizeof(LV2_Descriptor));

	IFilterStereoLPFDescriptor->URI 		= IFILTER_STEREO_LPF_URI;
	IFilterStereoLPFDescriptor->activate 		= activateIFilter;
	IFilterStereoLPFDescriptor->cleanup 		= cleanupIFilter;
	IFilterStereoLPFDescriptor->connect_port 	= connectPortIFilter;
	IFilterStereoLPFDescriptor->deactivate 		= NULL;
	IFilterStereoLPFDescriptor->instantiate 	= instantiateIFilter;
	IFilterStereoLPFDescriptor->run 		= runStereoLPFIFilter;
	IFilterStereoLPFDescriptor->extension_data	= NULL;

	IFilterMonoHPFDescriptor =
	 (LV2_Descriptor *)malloc(sizeof(LV2_Descriptor));

	IFilterMonoHPFDescriptor->URI 			= IFILTER_MONO_HPF_URI;
	IFilterMonoHPFDescriptor->activate 		= activateIFilter;
	IFilterMonoHPFDescriptor->cleanup 		= cleanupIFilter;
	IFilterMonoHPFDescriptor->connect_port 		= connectPortIFilter;
	IFilterMonoHPFDescriptor->deactivate 		= NULL;
	IFilterMonoHPFDescriptor->instantiate 		= instantiateIFilter;
	IFilterMonoHPFDescriptor->run 			= runMonoHPFIFilter;
	IFilterMonoHPFDescriptor->extension_data 	= NULL;

	IFilterStereoHPFDescriptor =
	 (LV2_Descriptor *)malloc(sizeof(LV2_Descriptor));

	IFilterStereoHPFDescriptor->URI 		= IFILTER_STEREO_HPF_URI;
	IFilterStereoHPFDescriptor->activate 		= activateIFilter;
	IFilterStereoHPFDescriptor->cleanup 		= cleanupIFilter;
	IFilterStereoHPFDescriptor->connect_port 	= connectPortIFilter;
	IFilterStereoHPFDescriptor->deactivate 		= NULL;
	IFilterStereoHPFDescriptor->instantiate 	= instantiateIFilter;
	IFilterStereoHPFDescriptor->run 		= runStereoHPFIFilter;
	IFilterStereoHPFDescriptor->extension_data	= NULL;
}


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
	if (!IFilterMonoLPFDescriptor) init();

	switch (index) {
		case 0:
			return IFilterMonoLPFDescriptor;
		case 1:
			return IFilterStereoLPFDescriptor;
		case 2:
			return IFilterMonoHPFDescriptor;
		case 3:
			return IFilterStereoHPFDescriptor;
	default:
		return NULL;
	}
}

/*****************************************************************************/


float convertParam(unsigned long param, float value, double sr) {

	float temp;
	float result;

	switch(param)
	{
		case IFILTER_FREQ:
			temp = value / (float)sr;
			if (temp <  0.0005)
				result = 1000;
			else if (temp <= 0.5)
				result = 1/(2*temp);
			else
				result=1;
			break;
		case IFILTER_GAIN:
			if(value<0)
				result= 1;
			else if (value < 12)
				result = pow(10,value/20);
			else
				result= pow(10,0.6);
			break;
		case IFILTER_NOCLIP:
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

