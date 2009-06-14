/* 

    This LV2 plugin provides munged delay plugins

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
#include "inv_delay.h"

static LV2_Descriptor *IDelayMonoDescriptor = NULL;
static LV2_Descriptor *IDelaySumDescriptor = NULL;



typedef struct {

	/* Ports */
	float *ControlBypass;
	float *ControlMode;
	float *ControlMungeMode;
	float *ControlMunge;
	float *Control1Delay;
	float *Control1FB; 
	float *Control1Pan;
	float *Control1Vol;
	float *Control2Delay;
	float *Control2FB; 
	float *Control2Pan;
	float *Control2Vol;

	float *AudioOutputBufferL;
	float *AudioOutputBufferR;
	float *AudioInputBufferL;
	float *AudioInputBufferR; 

	float *MeterInputL;
	float *MeterOutputL;
	float *MeterOutputR;

	double SampleRate;

	/* Stuff to remember to avoid recalculating the delays every run */
	float LastBypass;
	float LastMode;
	float LastMungeMode;
	float LastMunge;
	float Last1Delay;
	float Last1FB; 
	float Last1Pan;
	float Last1Vol;
	float Last2Delay;
	float Last2FB; 
	float Last2Pan;
	float Last2Vol;


	float AudioLPF1; 
	float AudioLPF2; 
	float AudioHPF1; 
	float AudioHPF2; 
	float AudioDegrain1; 
	float AudioDegrain2; 
	float EnvInLLast; 
	float EnvOutLLast; 
	float EnvOutRLast;

	float ConvertedBypass; 
	float ConvertedMode; 
	float ConvertedMungeMode;
	float ConvertedMunge;
	float ConvertedLPFsamples;
	float ConvertedHPFsamples;
	float Converted1Delay;
	float Converted1FB; 
	float Converted1Pan;
	float Converted1Vol;
	float Converted2Delay;
	float Converted2FB; 
	float Converted2Pan;
	float Converted2Vol;


	/* Delay Space Data */
	unsigned long SpaceSize;
	float * SpaceL;
	float * SpaceR;
	float * SpaceLCur;
	float * SpaceRCur;
	float * SpaceLEnd;
	float * SpaceREnd;

} IDelay;


static LV2_Handle 
instantiateIDelay(const LV2_Descriptor *descriptor, double s_rate, const char *path, const LV2_Feature * const* features)
{
	IDelay *plugin = (IDelay *)malloc(sizeof(IDelay));
	if(plugin==NULL)
		return NULL;

	/* set some initial params */
	plugin->SampleRate=s_rate;
	plugin->SpaceSize = IDELAY_SPACE_SIZE * s_rate / 1000;

	/* the delay space */
	if((plugin->SpaceL  = (float *)malloc(sizeof(float) * plugin->SpaceSize))==NULL)
    		return NULL;
	
	if((plugin->SpaceR  = (float *)malloc(sizeof(float) * plugin->SpaceSize))==NULL)
    		return NULL;

	return (LV2_Handle)plugin;
}

static void 
connectPortIDelay(LV2_Handle instance, uint32_t port, void *data)
{
	IDelay *plugin = (IDelay *)instance;
	switch (port) {
		case IDELAY_BYPASS:
			plugin->ControlBypass = data;
			break;
		case IDELAY_MODE:
			plugin->ControlMode = data;
			break;
		case IDELAY_MUNGEMODE:
			plugin->ControlMungeMode = data;
			break;
		case IDELAY_MUNGE:
			plugin->ControlMunge = data;
			break;
		case IDELAY_1_DELAY:
			plugin->Control1Delay = data;
			break;
		case IDELAY_1_FB:
			plugin->Control1FB = data;
			break;
		case IDELAY_1_PAN:
			plugin->Control1Pan = data;
			break;
		case IDELAY_1_VOL:
			plugin->Control1Vol = data;
			break;
		case IDELAY_2_DELAY:
			plugin->Control2Delay = data;
			break;
		case IDELAY_2_FB:
			plugin->Control2FB = data;
			break;
		case IDELAY_2_PAN:
			plugin->Control2Pan = data;
			break;
		case IDELAY_2_VOL:
			plugin->Control2Vol = data;
			break;
		case IDELAY_AUDIO_INL:
			plugin->AudioInputBufferL = data;
			break;
		case IDELAY_AUDIO_INR:
			plugin->AudioInputBufferR = data;
			break;
		case IDELAY_AUDIO_OUTL:
			plugin->AudioOutputBufferL = data;
			break;
		case IDELAY_AUDIO_OUTR:
			plugin->AudioOutputBufferR = data;
			break;
		case IDELAY_METER_INL:
			plugin->MeterInputL = data;
			break;
		case IDELAY_METER_OUTL:
			plugin->MeterOutputL = data;
			break;
		case IDELAY_METER_OUTR:
			plugin->MeterOutputR = data;
			break;
	}
}


static void 
activateIDelay(LV2_Handle instance) 
{
	IDelay *plugin = (IDelay *)instance;

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
	plugin->LastBypass    	= 0.0;
	plugin->LastMode	= 0.0;
	plugin->LastMungeMode	= 0.0;
	plugin->LastMunge	= 50.0;
	plugin->Last1Delay	= 300.0;
	plugin->Last1FB		= 50.0; 
	plugin->Last1Pan	= -0.7;
	plugin->Last1Vol	= 100.0;
	plugin->Last2Delay	= 200.0;
	plugin->Last2FB		= 50.0; 
	plugin->Last2Pan	= 0.7;
	plugin->Last2Vol	= 100.0;

	plugin->AudioLPF1 	= 0; 
	plugin->AudioLPF2 	= 0;  
	plugin->AudioHPF1 	= 0; 
	plugin->AudioHPF2 	= 0; 
	plugin->AudioDegrain1 	= 0; 
	plugin->AudioDegrain2 	= 0; 
	plugin->EnvInLLast 	= 0; 
	plugin->EnvOutLLast 	= 0; 
	plugin->EnvOutRLast 	= 0; 

	plugin->ConvertedBypass     = convertParam(IDELAY_BYPASS,    plugin->LastBypass,    plugin->SampleRate);  
	plugin->ConvertedMungeMode  = convertParam(IDELAY_MUNGEMODE, plugin->LastMungeMode, plugin->SampleRate);  
	plugin->ConvertedMunge      = convertParam(IDELAY_MUNGE,     plugin->LastMunge,     plugin->SampleRate); 
	plugin->ConvertedLPFsamples = convertMunge(0,                plugin->LastMunge,     plugin->SampleRate); 
	plugin->ConvertedHPFsamples = convertMunge(1,                plugin->LastMunge,     plugin->SampleRate);  
	plugin->Converted1Delay     = convertParam(IDELAY_1_DELAY,   plugin->Last1Delay,    plugin->SampleRate);  
	plugin->Converted1FB        = convertParam(IDELAY_1_FB,      plugin->Last1FB,       plugin->SampleRate);  
	plugin->Converted1Pan       = convertParam(IDELAY_1_PAN,     plugin->Last1Pan,      plugin->SampleRate);  
	plugin->Converted1Vol       = convertParam(IDELAY_1_VOL,     plugin->Last1Vol,      plugin->SampleRate);  
	plugin->Converted2Delay     = convertParam(IDELAY_2_DELAY,   plugin->Last2Delay,    plugin->SampleRate);  
	plugin->Converted2FB        = convertParam(IDELAY_2_FB,      plugin->Last2FB,       plugin->SampleRate); 
	plugin->Converted2Pan       = convertParam(IDELAY_2_PAN,     plugin->Last2Pan,      plugin->SampleRate); 
	plugin->Converted2Vol       = convertParam(IDELAY_2_VOL,     plugin->Last2Vol,      plugin->SampleRate); 
}


static void 
runMonoIDelay(LV2_Handle instance, uint32_t SampleCount) 
{
	float (*pParamFunc)(unsigned long, float, double) = NULL;
	float * pfAudioInputL;
	float * pfAudioOutputL;
	float * pfAudioOutputR;	
	float In,Out1,Out2,OutL,OutR;
	float LPF1,LPF2,HPF1,HPF2,Degrain1,Degrain2;
	float EnvIn,EnvOutL,EnvOutR;
	float fBypass,fMode,fMunge,fMungeMode,fHPFsamples,fLPFsamples;
	float f1Delay,f1DelayOffset,f1PanLGain,f1PanRGain,f1FB,In1FB,In1FBmix,In1Munged;
	float f2Delay,f2DelayOffset,f2PanLGain,f2PanRGain,f2FB,In2FB,In2FBmix,In2Munged;
	unsigned long l1DelaySample;
	unsigned long l2DelaySample;
	unsigned long lSampleIndex;
	unsigned long SpaceSize;
	float * SpaceLStr;
	float * SpaceRStr;
	float * SpaceLCur;
	float * SpaceRCur;
	float * SpaceLEnd;
	float * SpaceREnd;

	IDelay *plugin = (IDelay *)instance;
	pParamFunc = &convertParam;

	/* check if any other params have changed */
	checkParamChange(IDELAY_BYPASS,    plugin->ControlBypass,    &(plugin->LastBypass),    &(plugin->ConvertedBypass),    plugin->SampleRate, pParamFunc);
	checkParamChange(IDELAY_MODE,      plugin->ControlMode,      &(plugin->LastMode),      &(plugin->ConvertedMode),      plugin->SampleRate, pParamFunc);
	checkParamChange(IDELAY_MUNGEMODE, plugin->ControlMungeMode, &(plugin->LastMungeMode), &(plugin->ConvertedMungeMode), plugin->SampleRate, pParamFunc);
	checkParamChange(IDELAY_1_DELAY,   plugin->Control1Delay,    &(plugin->Last1Delay),    &(plugin->Converted1Delay),    plugin->SampleRate, pParamFunc);
	checkParamChange(IDELAY_1_FB,      plugin->Control1FB,       &(plugin->Last1FB),       &(plugin->Converted1FB),       plugin->SampleRate, pParamFunc);
	checkParamChange(IDELAY_1_PAN,     plugin->Control1Pan,      &(plugin->Last1Pan),      &(plugin->Converted1Pan),      plugin->SampleRate, pParamFunc);
	checkParamChange(IDELAY_1_VOL,     plugin->Control1Vol,      &(plugin->Last1Vol),      &(plugin->Converted1Vol),      plugin->SampleRate, pParamFunc);
	checkParamChange(IDELAY_2_DELAY,   plugin->Control2Delay,    &(plugin->Last2Delay),    &(plugin->Converted2Delay),    plugin->SampleRate, pParamFunc);
	checkParamChange(IDELAY_2_FB,      plugin->Control2FB,       &(plugin->Last2FB),       &(plugin->Converted2FB),       plugin->SampleRate, pParamFunc);
	checkParamChange(IDELAY_2_PAN,     plugin->Control2Pan,      &(plugin->Last2Pan),      &(plugin->Converted2Pan),      plugin->SampleRate, pParamFunc);
	checkParamChange(IDELAY_2_VOL,     plugin->Control2Vol,      &(plugin->Last2Vol),      &(plugin->Converted2Vol),      plugin->SampleRate, pParamFunc);

	if(*(plugin->ControlMunge) != plugin->LastMunge) {
		plugin->LastMunge 		= *(plugin->ControlMunge);
		plugin->ConvertedMunge		= convertParam(IDELAY_MUNGE, plugin->LastMunge, plugin->SampleRate); 
		plugin->ConvertedLPFsamples	= convertMunge(0,            plugin->LastMunge, plugin->SampleRate); 
		plugin->ConvertedHPFsamples	= convertMunge(1,            plugin->LastMunge, plugin->SampleRate); 
	}

	fBypass         = plugin->ConvertedBypass;
	fMode           = plugin->ConvertedMode;
	fMungeMode	= plugin->ConvertedMungeMode;
	fMunge		= plugin->ConvertedMunge;
	fLPFsamples	= plugin->ConvertedLPFsamples;
	fHPFsamples	= plugin->ConvertedHPFsamples;

	f1Delay		= plugin->Converted1Delay;
	l1DelaySample	= (unsigned long)f1Delay;
	f1DelayOffset	= f1Delay-(float)l1DelaySample;
	f1PanLGain	= plugin->Converted1Vol * (1-plugin->Converted1Pan)/2;
	f1PanRGain	= plugin->Converted1Vol * (1+plugin->Converted1Pan)/2;


	f2Delay		= plugin->Converted2Delay;
	l2DelaySample	= (unsigned long)f2Delay;
	f2DelayOffset	= f2Delay-(float)l2DelaySample;
	f2PanLGain	= plugin->Converted2Vol * (1-plugin->Converted2Pan)/2;
	f2PanRGain	= plugin->Converted2Vol * (1+plugin->Converted2Pan)/2;

	if(fMungeMode < 0.5) {
		f1FB	= plugin->Converted1FB / (1+fMunge);
		f2FB	= plugin->Converted2FB / (1+fMunge);
	} else {
		f1FB	= plugin->Converted1FB * pow(2.0,-2.0*fMunge);
		f2FB	= plugin->Converted2FB * pow(2.0,-2.0*fMunge);
	}

	
	SpaceSize 	= plugin->SpaceSize;

	SpaceLStr	= plugin->SpaceL;
	SpaceRStr 	= plugin->SpaceR;
	SpaceLCur 	= plugin->SpaceLCur;
	SpaceRCur 	= plugin->SpaceRCur;
	SpaceLEnd 	= plugin->SpaceLEnd;
	SpaceREnd 	= plugin->SpaceREnd;

	pfAudioInputL 	= plugin->AudioInputBufferL;
	pfAudioOutputL 	= plugin->AudioOutputBufferL;
	pfAudioOutputR 	= plugin->AudioOutputBufferR;

	LPF1     	= plugin->AudioLPF1;
	LPF2     	= plugin->AudioLPF2;
	HPF1     	= plugin->AudioHPF1;
	HPF2     	= plugin->AudioHPF2;
	Degrain1     	= plugin->AudioDegrain1;
	Degrain2     	= plugin->AudioDegrain2;
	EnvIn     	= plugin->EnvInLLast;
	EnvOutL    	= plugin->EnvOutLLast;
	EnvOutR   	= plugin->EnvOutRLast;

	if(fBypass==0) { 
		for (lSampleIndex = 0; lSampleIndex < SampleCount; lSampleIndex++) {


			// read the audio out of the delay space
			Out1 = *(SpaceLCur);
			Out2 = *(SpaceRCur);

			// read the input
			In = *(pfAudioInputL++);

			// and mix the feedback in
			if(fMode<0.5 ) {
				// normal
				In1FBmix= In+(f1FB*Out1);
				In2FBmix= In+(f2FB*Out2);
			} else {
 				//ping pong
				In1FBmix= In+(f2FB*Out2);
				In2FBmix= In+(f1FB*Out1);
			}

			// munge it
			In1FB	= (1-fMunge)*In1FBmix + fMunge*ITube_do(In1FBmix,1+fMunge);
			In2FB	= (1-fMunge)*In2FBmix + fMunge*ITube_do(In2FBmix,1+fMunge);
			HPF1 	= ((fHPFsamples-1) * HPF1 + In1FB) / fHPFsamples;  
			HPF2 	= ((fHPFsamples-1) * HPF2 + In2FB) / fHPFsamples; 
			LPF1 	= ((fLPFsamples-1) * LPF1 + (In1FB-HPF1)) / fLPFsamples; 
			LPF2 	= ((fLPFsamples-1) * LPF2 + (In2FB-HPF2)) / fLPFsamples;  
			if(fMungeMode<0.5) {
				In1Munged = LPF1;
				In2Munged = LPF2;
			} else {
				In1Munged = 2 * (In1FB-HPF1) - LPF1;
				In2Munged = 2 * (In2FB-HPF2) - LPF2;
			} 
			Degrain1     	= (In1Munged+Degrain1)/2;
			Degrain2     	= (In2Munged+Degrain2)/2;

			// add to the delay space
			if(SpaceLCur+l1DelaySample > SpaceLEnd)
				*(SpaceLCur+l1DelaySample-SpaceSize)+=Degrain1*(1-f1DelayOffset);
			else
				*(SpaceLCur+l1DelaySample)+=Degrain1*(1-f1DelayOffset);

			if(SpaceLCur+l1DelaySample+1 > SpaceLEnd)
				*(SpaceLCur+l1DelaySample-SpaceSize+1)+=Degrain1*f1DelayOffset;
			else
				*(SpaceLCur+l1DelaySample+1)+=Degrain1*f1DelayOffset;

			if(SpaceRCur+l2DelaySample > SpaceREnd)
				*(SpaceRCur+l2DelaySample-SpaceSize)+=Degrain2*(1-f2DelayOffset);
			else
				*(SpaceRCur+l2DelaySample)+=Degrain2*(1-f2DelayOffset);

			if(SpaceRCur+l2DelaySample+1 > SpaceREnd)
				*(SpaceRCur+l2DelaySample-SpaceSize+1)+=Degrain2*f2DelayOffset;
			else
				*(SpaceRCur+l2DelaySample+1)+=Degrain2*f2DelayOffset;

			// mix the two delays in
			OutL = f1PanLGain*Out1 + f2PanLGain*Out2;
			OutR = f1PanRGain*Out1 + f2PanRGain*Out2;
			// write the output
			*(pfAudioOutputL++) = OutL;
			*(pfAudioOutputR++) = OutR;
			// zero the spot we just read
			*(SpaceLCur)=0;
			*(SpaceRCur)=0;
			// advance the pointer to the next spot
			SpaceLCur = SpaceLCur < SpaceLEnd ? SpaceLCur + 1 : SpaceLStr;
			SpaceRCur = SpaceRCur < SpaceREnd ? SpaceRCur + 1 : SpaceRStr;

			//evelope on in and out for meters
			EnvIn   += IEnvelope(In,  EnvIn,  INVADA_METER_PEAK,plugin->SampleRate);
			EnvOutL += IEnvelope(OutL,EnvOutL,INVADA_METER_PEAK,plugin->SampleRate);
			EnvOutR += IEnvelope(OutR,EnvOutR,INVADA_METER_PEAK,plugin->SampleRate);

		}
	} else {
		for (lSampleIndex = 0; lSampleIndex < SampleCount; lSampleIndex++) {

			*(pfAudioOutputL++) = *pfAudioInputL;
			*(pfAudioOutputR++) = *(pfAudioInputL++);
			// zero the spot we just read
			*(SpaceLCur)=0;
			*(SpaceRCur)=0;
			// advance the pointer to the next spot
			SpaceLCur = SpaceLCur < SpaceLEnd ? SpaceLCur + 1 : SpaceLStr;
			SpaceRCur = SpaceRCur < SpaceREnd ? SpaceRCur + 1 : SpaceRStr;
		}
		//zero filters
		LPF1    =0;
		LPF2    =0;
		HPF1    =0;
		HPF2    =0;
		Degrain1    =0;
		Degrain2    =0;
		//zero envelope on in and out for meters
		EnvIn   =0;
		EnvOutL =0;
		EnvOutR =0;
	}

	// remember for next run
	plugin->SpaceLCur=SpaceLCur;
	plugin->SpaceRCur=SpaceRCur;

	plugin->AudioLPF1   = (fabs(LPF1)<1.0e-10)     ? 0.f : LPF1; 
	plugin->AudioLPF2   = (fabs(LPF2)<1.0e-10)     ? 0.f : LPF2; 
	plugin->AudioHPF1   = (fabs(HPF1)<1.0e-10)     ? 0.f : HPF1; 
	plugin->AudioHPF2   = (fabs(HPF2)<1.0e-10)     ? 0.f : HPF2; 
	plugin->AudioDegrain1   = (fabs(Degrain1)<1.0e-10)     ? 0.f : Degrain1; 
	plugin->AudioDegrain2   = (fabs(Degrain2)<1.0e-10)     ? 0.f : Degrain2; 
	plugin->EnvInLLast  = (fabs(EnvIn)<1.0e-10)    ? 0.f : EnvIn; 
	plugin->EnvOutLLast = (fabs(EnvOutL)<1.0e-10)  ? 0.f : EnvOutL; 
	plugin->EnvOutRLast = (fabs(EnvOutR)<1.0e-10)  ? 0.f : EnvOutR; 

	// update the meters
	*(plugin->MeterInputL)  = (EnvIn   > 0.001) ? 20*log10(EnvIn)   : -90.0;
	*(plugin->MeterOutputL) = (EnvOutL > 0.001) ? 20*log10(EnvOutL) : -90.0;
	*(plugin->MeterOutputR) = (EnvOutR > 0.001) ? 20*log10(EnvOutR) : -90.0;
}




static void 
runSumIDelay(LV2_Handle instance, uint32_t SampleCount) 
{
	float (*pParamFunc)(unsigned long, float, double) = NULL;
	float * pfAudioInputL;
	float * pfAudioInputR;
	float * pfAudioOutputL;
	float * pfAudioOutputR;	
	float In,Out1,Out2,OutL,OutR;
	float LPF1,LPF2,HPF1,HPF2,Degrain1,Degrain2;
	float EnvIn,EnvOutL,EnvOutR;
	float fBypass,fMode,fMunge,fMungeMode,fHPFsamples,fLPFsamples;
	float f1Delay,f1DelayOffset,f1PanLGain,f1PanRGain,f1FB,In1FB,In1FBmix,In1Munged;
	float f2Delay,f2DelayOffset,f2PanLGain,f2PanRGain,f2FB,In2FB,In2FBmix,In2Munged;
	unsigned long l1DelaySample;
	unsigned long l2DelaySample;
	unsigned long lSampleIndex;
	unsigned long SpaceSize;
	float * SpaceLStr;
	float * SpaceRStr;
	float * SpaceLCur;
	float * SpaceRCur;
	float * SpaceLEnd;
	float * SpaceREnd;

	IDelay *plugin = (IDelay *)instance;
	pParamFunc = &convertParam;

	/* check if any other params have changed */
	checkParamChange(IDELAY_BYPASS,    plugin->ControlBypass,    &(plugin->LastBypass),    &(plugin->ConvertedBypass),    plugin->SampleRate, pParamFunc);
	checkParamChange(IDELAY_MODE,      plugin->ControlMode,      &(plugin->LastMode),      &(plugin->ConvertedMode),      plugin->SampleRate, pParamFunc);
	checkParamChange(IDELAY_MUNGEMODE, plugin->ControlMungeMode, &(plugin->LastMungeMode), &(plugin->ConvertedMungeMode), plugin->SampleRate, pParamFunc);
	checkParamChange(IDELAY_1_DELAY,   plugin->Control1Delay,    &(plugin->Last1Delay),    &(plugin->Converted1Delay),    plugin->SampleRate, pParamFunc);
	checkParamChange(IDELAY_1_FB,      plugin->Control1FB,       &(plugin->Last1FB),       &(plugin->Converted1FB),       plugin->SampleRate, pParamFunc);
	checkParamChange(IDELAY_1_PAN,     plugin->Control1Pan,      &(plugin->Last1Pan),      &(plugin->Converted1Pan),      plugin->SampleRate, pParamFunc);
	checkParamChange(IDELAY_1_VOL,     plugin->Control1Vol,      &(plugin->Last1Vol),      &(plugin->Converted1Vol),      plugin->SampleRate, pParamFunc);
	checkParamChange(IDELAY_2_DELAY,   plugin->Control2Delay,    &(plugin->Last2Delay),    &(plugin->Converted2Delay),    plugin->SampleRate, pParamFunc);
	checkParamChange(IDELAY_2_FB,      plugin->Control2FB,       &(plugin->Last2FB),       &(plugin->Converted2FB),       plugin->SampleRate, pParamFunc);
	checkParamChange(IDELAY_2_PAN,     plugin->Control2Pan,      &(plugin->Last2Pan),      &(plugin->Converted2Pan),      plugin->SampleRate, pParamFunc);
	checkParamChange(IDELAY_2_VOL,     plugin->Control2Vol,      &(plugin->Last2Vol),      &(plugin->Converted2Vol),      plugin->SampleRate, pParamFunc);

	if(*(plugin->ControlMunge) != plugin->LastMunge) {
		plugin->LastMunge 		= *(plugin->ControlMunge);
		plugin->ConvertedMunge		= convertParam(IDELAY_MUNGE, plugin->LastMunge, plugin->SampleRate); 
		plugin->ConvertedLPFsamples	= convertMunge(0,            plugin->LastMunge, plugin->SampleRate); 
		plugin->ConvertedHPFsamples	= convertMunge(1,            plugin->LastMunge, plugin->SampleRate); 
	}

	fBypass         = plugin->ConvertedBypass;
	fMode           = plugin->ConvertedMode;
	fMungeMode	= plugin->ConvertedMungeMode;
	fMunge		= plugin->ConvertedMunge;
	fLPFsamples	= plugin->ConvertedLPFsamples;
	fHPFsamples	= plugin->ConvertedHPFsamples;

	f1Delay		= plugin->Converted1Delay;
	l1DelaySample	= (unsigned long)f1Delay;
	f1DelayOffset	= f1Delay-(float)l1DelaySample;
	f1PanLGain	= plugin->Converted1Vol * (1-plugin->Converted1Pan)/2;
	f1PanRGain	= plugin->Converted1Vol * (1+plugin->Converted1Pan)/2;

	f2Delay		= plugin->Converted2Delay;
	l2DelaySample	= (unsigned long)f2Delay;
	f2DelayOffset	= f2Delay-(float)l2DelaySample;
	f2PanLGain	= plugin->Converted2Vol * (1-plugin->Converted2Pan)/2;
	f2PanRGain	= plugin->Converted2Vol * (1+plugin->Converted2Pan)/2;

	if(fMungeMode < 0.5) {
		f1FB	= plugin->Converted1FB / (1+fMunge);
		f2FB	= plugin->Converted2FB / (1+fMunge);
	} else {
		f1FB	= plugin->Converted1FB * pow(2.0,-2.0*fMunge);
		f2FB	= plugin->Converted2FB * pow(2.0,-2.0*fMunge);
	}
	
	SpaceSize 	= plugin->SpaceSize;

	SpaceLStr	= plugin->SpaceL;
	SpaceRStr 	= plugin->SpaceR;
	SpaceLCur 	= plugin->SpaceLCur;
	SpaceRCur 	= plugin->SpaceRCur;
	SpaceLEnd 	= plugin->SpaceLEnd;
	SpaceREnd 	= plugin->SpaceREnd;

	pfAudioInputL 	= plugin->AudioInputBufferL;
	pfAudioInputR 	= plugin->AudioInputBufferR;
	pfAudioOutputL 	= plugin->AudioOutputBufferL;
	pfAudioOutputR 	= plugin->AudioOutputBufferR;

	LPF1     	= plugin->AudioLPF1;
	LPF2     	= plugin->AudioLPF2;
	HPF1     	= plugin->AudioHPF1;
	HPF2     	= plugin->AudioHPF2;
	Degrain1     	= plugin->AudioDegrain1;
	Degrain2     	= plugin->AudioDegrain2;
	EnvIn     	= plugin->EnvInLLast;
	EnvOutL    	= plugin->EnvOutLLast;
	EnvOutR   	= plugin->EnvOutRLast;

	if(fBypass==0) { 
		for (lSampleIndex = 0; lSampleIndex < SampleCount; lSampleIndex++) {

			// read the audio out of the delay space
			Out1 = *(SpaceLCur);
			Out2 = *(SpaceRCur);

			// now read the input
			In=( *(pfAudioInputL++) + *(pfAudioInputR++) )/2;

			// and mix the feedback in
			if(fMode<0.5 ) {
				// normal
				In1FBmix= In+(f1FB*Out1);
				In2FBmix= In+(f2FB*Out2);
			} else {
 				//ping pong
				In1FBmix= In+(f2FB*Out2);
				In2FBmix= In+(f1FB*Out1);
			}

			// munge it
			In1FB	= (1-fMunge)*In1FBmix + fMunge*ITube_do(In1FBmix,1+fMunge);
			In2FB	= (1-fMunge)*In2FBmix + fMunge*ITube_do(In2FBmix,1+fMunge);
			HPF1 = ((fHPFsamples-1) * HPF1 + In1FB) / fHPFsamples;  
			HPF2 = ((fHPFsamples-1) * HPF2 + In2FB) / fHPFsamples; 
			LPF1 = ((fLPFsamples-1) * LPF1 + (In1FB-HPF1)) / fLPFsamples; 
			LPF2 = ((fLPFsamples-1) * LPF2 + (In2FB-HPF2)) / fLPFsamples;  
			if(fMungeMode<0.5) {
				In1Munged = LPF1;
				In2Munged = LPF2;
			} else {
				In1Munged = 2 * (In1FB-HPF1) - LPF1;
				In2Munged = 2 * (In2FB-HPF2) - LPF2;
			} 
			Degrain1     	= (In1Munged+Degrain1)/2;
			Degrain2     	= (In2Munged+Degrain2)/2;

			// add to the delay space
			if(SpaceLCur+l1DelaySample > SpaceLEnd)
				*(SpaceLCur+l1DelaySample-SpaceSize)+=Degrain1*(1-f1DelayOffset);
			else
				*(SpaceLCur+l1DelaySample)+=Degrain1*(1-f1DelayOffset);

			if(SpaceLCur+l1DelaySample+1 > SpaceLEnd)
				*(SpaceLCur+l1DelaySample-SpaceSize+1)+=Degrain1*f1DelayOffset;
			else
				*(SpaceLCur+l1DelaySample+1)+=Degrain1*f1DelayOffset;

			if(SpaceRCur+l2DelaySample > SpaceREnd)
				*(SpaceRCur+l2DelaySample-SpaceSize)+=Degrain2*(1-f2DelayOffset);
			else
				*(SpaceRCur+l2DelaySample)+=Degrain2*(1-f2DelayOffset);

			if(SpaceRCur+l2DelaySample+1 > SpaceREnd)
				*(SpaceRCur+l2DelaySample-SpaceSize+1)+=Degrain2*f2DelayOffset;
			else
				*(SpaceRCur+l2DelaySample+1)+=Degrain2*f2DelayOffset;


			// mix the two delays in
			OutL = f1PanLGain*Out1 + f2PanLGain*Out2;
			OutR = f1PanRGain*Out1 + f2PanRGain*Out2;
			// write the output
			*(pfAudioOutputL++) = OutL;
			*(pfAudioOutputR++) = OutR;
			// zero the spot we just read
			*(SpaceLCur)=0;
			*(SpaceRCur)=0;
			// advance the pointer to the next spot
			SpaceLCur = SpaceLCur < SpaceLEnd ? SpaceLCur + 1 : SpaceLStr;
			SpaceRCur = SpaceRCur < SpaceREnd ? SpaceRCur + 1 : SpaceRStr;

			//evelope on in and out for meters
			EnvIn   += IEnvelope(In,  EnvIn,  INVADA_METER_PEAK,plugin->SampleRate);
			EnvOutL += IEnvelope(OutL,EnvOutL,INVADA_METER_PEAK,plugin->SampleRate);
			EnvOutR += IEnvelope(OutR,EnvOutR,INVADA_METER_PEAK,plugin->SampleRate);

		}
	} else {
		for (lSampleIndex = 0; lSampleIndex < SampleCount; lSampleIndex++) {

			*(pfAudioOutputL++) = *pfAudioInputL;
			*(pfAudioOutputR++) = *(pfAudioInputL++);
			// zero the spot we just read
			*(SpaceLCur)=0;
			*(SpaceRCur)=0;
			// advance the pointer to the next spot
			SpaceLCur = SpaceLCur < SpaceLEnd ? SpaceLCur + 1 : SpaceLStr;
			SpaceRCur = SpaceRCur < SpaceREnd ? SpaceRCur + 1 : SpaceRStr;
		}
		//zero filters
		LPF1    =0;
		LPF2    =0;
		HPF1    =0;
		HPF2    =0;
		Degrain1    =0;
		Degrain2    =0;
		//zero envelope on in and out for meters
		EnvIn   =0;
		EnvOutL =0;
		EnvOutR =0;
	}

	// remember for next run
	plugin->SpaceLCur=SpaceLCur;
	plugin->SpaceRCur=SpaceRCur;

	plugin->AudioLPF1   = (fabs(LPF1)<1.0e-10)     ? 0.f : LPF1; 
	plugin->AudioLPF2   = (fabs(LPF2)<1.0e-10)     ? 0.f : LPF2; 
	plugin->AudioHPF1   = (fabs(HPF1)<1.0e-10)     ? 0.f : HPF1; 
	plugin->AudioHPF2   = (fabs(HPF2)<1.0e-10)     ? 0.f : HPF2; 
	plugin->AudioDegrain1   = (fabs(Degrain1)<1.0e-10)     ? 0.f : Degrain1; 
	plugin->AudioDegrain2   = (fabs(Degrain2)<1.0e-10)     ? 0.f : Degrain2; 
	plugin->EnvInLLast  = (fabs(EnvIn)<1.0e-10)    ? 0.f : EnvIn; 
	plugin->EnvOutLLast = (fabs(EnvOutL)<1.0e-10)  ? 0.f : EnvOutL; 
	plugin->EnvOutRLast = (fabs(EnvOutR)<1.0e-10)  ? 0.f : EnvOutR; 

	// update the meters
	*(plugin->MeterInputL) =(EnvIn   > 0.001) ? 20*log10(EnvIn)   : -90.0;
	*(plugin->MeterOutputL)=(EnvOutL > 0.001) ? 20*log10(EnvOutL) : -90.0;
	*(plugin->MeterOutputR)=(EnvOutR > 0.001) ? 20*log10(EnvOutR) : -90.0;
}


static void 
cleanupIDelay(LV2_Handle instance)
{
	IDelay *plugin = (IDelay *)instance;

	free(plugin->SpaceL);
	free(plugin->SpaceR);
	free(instance);
}


static void 
init()
{
	IDelayMonoDescriptor =
	 (LV2_Descriptor *)malloc(sizeof(LV2_Descriptor));

	IDelayMonoDescriptor->URI 		= IDELAY_MONO_URI;
	IDelayMonoDescriptor->activate 	= activateIDelay;
	IDelayMonoDescriptor->cleanup 		= cleanupIDelay;
	IDelayMonoDescriptor->connect_port 	= connectPortIDelay;
	IDelayMonoDescriptor->deactivate 	= NULL;
	IDelayMonoDescriptor->instantiate 	= instantiateIDelay;
	IDelayMonoDescriptor->run 		= runMonoIDelay;
	IDelayMonoDescriptor->extension_data	= NULL;

	IDelaySumDescriptor =
	 (LV2_Descriptor *)malloc(sizeof(LV2_Descriptor));

	IDelaySumDescriptor->URI 		= IDELAY_SUM_URI;
	IDelaySumDescriptor->activate 		= activateIDelay;
	IDelaySumDescriptor->cleanup 		= cleanupIDelay;
	IDelaySumDescriptor->connect_port 	= connectPortIDelay;
	IDelaySumDescriptor->deactivate 	= NULL;
	IDelaySumDescriptor->instantiate 	= instantiateIDelay;
	IDelaySumDescriptor->run 		= runSumIDelay;
	IDelaySumDescriptor->extension_data	= NULL;

}


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
	if (!IDelayMonoDescriptor) init();

	switch (index) {
		case 0:
			return IDelayMonoDescriptor;
		case 1:
			return IDelaySumDescriptor;

		default:
			return NULL;
	}
}


/*****************************************************************************/

float 
convertParam(unsigned long param, float value, double sr) {
	float result;

	switch(param)
	{
		case IDELAY_BYPASS:
		case IDELAY_MODE:
		case IDELAY_MUNGEMODE:
			if(value<=0.0)
				result= 0; 
			else
				result= 1;
			break;
		case IDELAY_MUNGE:
			if(value<0)
				result = 0.0;
			else if (value < 100)
				result = value/100.0;
			else
				result = 1.0;
			break;
		case IDELAY_1_DELAY:
		case IDELAY_2_DELAY:
			if (value < 0.02)
				result = 0.02 * sr;
			else if (value <= 2.0)
				result = value * sr;
			else
				result = 2.0 * sr;
			break;
		case IDELAY_1_FB:
		case IDELAY_2_FB:
			if(value<0)
				result = 0.0;
			else if (value < 133.333333)
				result = value/100.0;
			else
				result = 1.3333333;
			break;
		case IDELAY_1_PAN:
		case IDELAY_2_PAN:
			if(value<-1.0)
				result = -1.0;
			else if (value < 1.0)
				result = value;
			else
				result = 1.0;
			break;
		case IDELAY_1_VOL:
		case IDELAY_2_VOL:
			if(value<0.0)
				result = 0.0;
			else if (value < 100.0)
				result = value/100.0;
			else
				result = 1.0;
			break;
		default:
			result=0;
			break;
	}
	return result;
}

float
convertMunge (unsigned int mode, float munge, double sr)
{
	float result;
	switch(mode) {
		case 0: //LPF
			if (munge < 0)
				result = sr/(2*pow(10,(4.34)));  //22kHz
			else if (munge <= 100.0)
				result = sr/(2*pow(10,(4.34-(munge*0.0074))));
			else
				result = sr/(2*pow(10,(3.60)));  //4kHz
			break;
		case 1: //HPF
			if (munge < 0)
				result = sr/(2*pow(10,(1.30)));  //20Hz
			else if (munge <= 100.0)
				result = sr/(2*pow(10,(1.30+(munge*0.0160))));
			else
				result = sr/(2*pow(10,(2.90)));  //800Hz
			break;
		default:
			result=1;
			break;
	}
	return result;
}


