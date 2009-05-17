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
#include "library/common.h"
#include "inv_flange.h"

static LV2_Descriptor *IFlangeMonoDescriptor = NULL;
static LV2_Descriptor *IFlangeStereoDescriptor = NULL;
static LV2_Descriptor *IFlangeSumDescriptor = NULL;



typedef struct {

	/* Ports */
	float *ControlBypass;
	float *ControlCycle;
	float *ControlPhase;
	float *ControlWidth; 
	float *ControlDepth;
	float *ControlNoClip;

	float *AudioOutputBufferL;
	float *AudioOutputBufferR;
	float *AudioInputBufferL;
	float *AudioInputBufferR; 

	float *MeterInputL;
	float *MeterInputR;
	float *MeterOutputL;
	float *MeterOutputR;
	float *LampLfoL;
	float *LampLfoR;
	float *LampNoClip;

	double SampleRate;
	float Offset;

	/* Stuff to remember to avoid recalculating the delays every run */
	float LastBypass;
	float LastCycle;
	float LastPhase;
	float LastWidth; 
	float LastDepth;
	float LastNoClip;

	float AngleLast; 
	float EnvInLLast; 
	float EnvInRLast; 
	float EnvOutLLast; 
	float EnvOutRLast;
	float EnvDriveLast; 

	float ConvertedBypass; 
	float ConvertedCycle;
	float ConvertedPhase;
	float ConvertedWidth; 
	float ConvertedDepth;
	float ConvertedNoClip;

	/* Delay Space Data */
	unsigned long SpaceSize;
	float * SpaceL;
	float * SpaceR;
	float * SpaceLCur;
	float * SpaceRCur;
	float * SpaceLEnd;
	float * SpaceREnd;

} IFlange;


static LV2_Handle 
instantiateIFlange(const LV2_Descriptor *descriptor, double s_rate, const char *path, const LV2_Feature * const* features)
{
	IFlange *plugin = (IFlange *)malloc(sizeof(IFlange));
	if(plugin==NULL)
		return NULL;

	/* set some initial params */
	plugin->SampleRate=s_rate;
	plugin->Offset =  s_rate / 10000;  /* 10khz wavelength */
	plugin->SpaceSize = FLANGE_SPACE_SIZE * s_rate / 1000;

	/* the delay space */
	if((plugin->SpaceL  = (float *)malloc(sizeof(float) * plugin->SpaceSize))==NULL)
    		return NULL;
	
	if((plugin->SpaceR  = (float *)malloc(sizeof(float) * plugin->SpaceSize))==NULL)
    		return NULL;

	return (LV2_Handle)plugin;
}


static void 
connectPortIFlange(LV2_Handle instance, uint32_t port, void *data)
{
	IFlange *plugin = (IFlange *)instance;
	switch (port) {
		case IFLANGE_BYPASS:
			plugin->ControlBypass = data;
			break;
		case IFLANGE_CYCLE:
			plugin->ControlCycle = data;
			break;
		case IFLANGE_PHASE:
			plugin->ControlPhase = data;
			break;
		case IFLANGE_WIDTH:
			plugin->ControlWidth = data;
			break;
		case IFLANGE_DEPTH:
			plugin->ControlDepth = data;
			break;
		case IFLANGE_NOCLIP:
			plugin->ControlNoClip = data;
			break;
		case IFLANGE_LAMP_L:
			plugin->LampLfoL = data;
			break;
		case IFLANGE_LAMP_R:
			plugin->LampLfoR = data;
			break;
		case IFLANGE_LAMP_NOCLIP:
			plugin->LampNoClip = data;
			break;
		case IFLANGE_AUDIO_INL:
			plugin->AudioInputBufferL = data;
			break;
		case IFLANGE_AUDIO_INR:
			plugin->AudioInputBufferR = data;
			break;
		case IFLANGE_AUDIO_OUTL:
			plugin->AudioOutputBufferL = data;
			break;
		case IFLANGE_AUDIO_OUTR:
			plugin->AudioOutputBufferR = data;
			break;
		case IFLANGE_METER_INL:
			plugin->MeterInputL = data;
			break;
		case IFLANGE_METER_INR:
			plugin->MeterInputR = data;
			break;
		case IFLANGE_METER_OUTL:
			plugin->MeterOutputL = data;
			break;
		case IFLANGE_METER_OUTR:
			plugin->MeterOutputR = data;
			break;
	}
}


static void 
activateIFlange(LV2_Handle instance) 
{
	IFlange *plugin = (IFlange *)instance;

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
	plugin->LastCycle 	= 25.0;
	plugin->LastPhase  	= 45.0; 
	plugin->LastWidth 	= 10.5;
	plugin->LastDepth   	= 75.0;
	plugin->LastNoClip   	= 1.0;

	plugin->EnvInLLast 	= 0; 
	plugin->EnvInRLast 	= 0; 
	plugin->EnvOutLLast 	= 0; 
	plugin->EnvOutRLast 	= 0; 
	plugin->EnvDriveLast 	= 0; 

	plugin->AngleLast 	= 0; 

	plugin->ConvertedBypass = convertParam(IFLANGE_BYPASS, plugin->LastBypass, plugin->SampleRate);  
	plugin->ConvertedCycle  = convertParam(IFLANGE_CYCLE,  plugin->LastCycle,  plugin->SampleRate);  
	plugin->ConvertedPhase  = convertParam(IFLANGE_PHASE,  plugin->LastPhase,  plugin->SampleRate);  
	plugin->ConvertedWidth  = convertParam(IFLANGE_WIDTH,  plugin->LastWidth,  plugin->SampleRate);  
	plugin->ConvertedDepth  = convertParam(IFLANGE_DEPTH,  plugin->LastDepth,  plugin->SampleRate);  
	plugin->ConvertedNoClip = convertParam(IFLANGE_NOCLIP, plugin->LastNoClip, plugin->SampleRate);  
}


static void 
runMonoIFlange(LV2_Handle instance, uint32_t SampleCount) 
{
	float (*pParamFunc)(unsigned long, float, double) = NULL;
	float * pfAudioInputL;
	float * pfAudioOutputL;
	float * pfAudioOutputR;	
	float fOffset;
	float fAngle;
	float In,OutL,OutR;
	float EnvIn,EnvOutL,EnvOutR,EnvDrive;
	float drive;
	float driveL=0;
	float driveR=0;
	float fBypass,fAngleDelta,fRPhase,fMaxDelay2,fDepth,fNoClip;
	float fDelayL,fDelayOffsetL;
	float fDelayR,fDelayOffsetR;
	unsigned long lDelaySampleL;
	unsigned long lDelaySampleR;
	unsigned long lSampleIndex;
	unsigned long SpaceSize;
	float * SpaceLStr;
	float * SpaceRStr;
	float * SpaceLCur;
	float * SpaceRCur;
	float * SpaceLEnd;
	float * SpaceREnd;

	IFlange *plugin = (IFlange *)instance;
	pParamFunc = &convertParam;

	/* check if any other params have changed */
	checkParamChange(IFLANGE_BYPASS, plugin->ControlBypass, &(plugin->LastBypass), &(plugin->ConvertedBypass), plugin->SampleRate, pParamFunc);
	checkParamChange(IFLANGE_CYCLE,  plugin->ControlCycle,  &(plugin->LastCycle),  &(plugin->ConvertedCycle),  plugin->SampleRate, pParamFunc);
	checkParamChange(IFLANGE_PHASE,  plugin->ControlPhase,  &(plugin->LastPhase),  &(plugin->ConvertedPhase),  plugin->SampleRate, pParamFunc);
	checkParamChange(IFLANGE_WIDTH,  plugin->ControlWidth,  &(plugin->LastWidth),  &(plugin->ConvertedWidth),  plugin->SampleRate, pParamFunc);
	checkParamChange(IFLANGE_DEPTH,  plugin->ControlDepth,  &(plugin->LastDepth),  &(plugin->ConvertedDepth),  plugin->SampleRate, pParamFunc);
	checkParamChange(IFLANGE_NOCLIP, plugin->ControlNoClip, &(plugin->LastNoClip), &(plugin->ConvertedNoClip), plugin->SampleRate, pParamFunc);


	fBypass         = plugin->ConvertedBypass;
	fAngleDelta	= plugin->ConvertedCycle;
	fRPhase   	= plugin->ConvertedPhase;
	fMaxDelay2	= plugin->ConvertedWidth;
	fDepth		= plugin->ConvertedDepth;
	fNoClip		= plugin->ConvertedNoClip;
	
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

	fOffset		= plugin->Offset;
	fAngle		= plugin->AngleLast;
	EnvIn     	= plugin->EnvInLLast;
	EnvOutL    	= plugin->EnvOutLLast;
	EnvOutR   	= plugin->EnvOutRLast;
	EnvDrive   	= plugin->EnvDriveLast;

	if(fBypass==0) { 
		for (lSampleIndex = 0; lSampleIndex < SampleCount; lSampleIndex++) {

			In=*(pfAudioInputL++);

			fDelayL=fOffset + (fMaxDelay2 * ( 1 + cos(fAngle)));
			fDelayR=fOffset + (fMaxDelay2 * ( 1 + cos(fAngle+fRPhase)));

			fAngle += fAngleDelta;

			lDelaySampleL=(unsigned long)fDelayL;
			lDelaySampleR=(unsigned long)fDelayR;

			fDelayOffsetL=fDelayL-(float)lDelaySampleL;
			fDelayOffsetR=fDelayR-(float)lDelaySampleR;

			// add to the delay space
			if(SpaceLCur+lDelaySampleL > SpaceLEnd)
				*(SpaceLCur+lDelaySampleL-SpaceSize)-=fDepth*In*(1-fDelayOffsetL);
			else
				*(SpaceLCur+lDelaySampleL)-=fDepth*In*(1-fDelayOffsetL);

			if(SpaceLCur+lDelaySampleL+1 > SpaceLEnd)
				*(SpaceLCur+lDelaySampleL-SpaceSize+1)-=fDepth*In*fDelayOffsetL;
			else
				*(SpaceLCur+lDelaySampleL+1)-=fDepth*In*fDelayOffsetL;

			if(SpaceRCur+lDelaySampleR > SpaceREnd)
				*(SpaceRCur+lDelaySampleR-SpaceSize)-=fDepth*In*(1-fDelayOffsetR);
			else
				*(SpaceRCur+lDelaySampleR)-=fDepth*In*(1-fDelayOffsetR);

			if(SpaceRCur+lDelaySampleR+1 > SpaceREnd)
				*(SpaceRCur+lDelaySampleR-SpaceSize+1)-=fDepth*In*fDelayOffsetR;
			else
				*(SpaceRCur+lDelaySampleR+1)-=fDepth*In*fDelayOffsetR;


			// read the audio out of the delay space
			OutL = In + *(SpaceLCur);
			OutR = In + *(SpaceRCur);
			OutL = fNoClip > 0 ? InoClip(OutL,&driveL) : OutL;
			OutR = fNoClip > 0 ? InoClip(OutR,&driveR) : OutR;
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
			drive = driveL > driveR ? driveL : driveR;
			EnvDrive += IEnvelope(drive,EnvDrive,INVADA_METER_LAMP,plugin->SampleRate);
		}
	} else {
		for (lSampleIndex = 0; lSampleIndex < SampleCount; lSampleIndex++) {

			*(pfAudioOutputL++) = *pfAudioInputL;
			*(pfAudioOutputR++) = *(pfAudioInputL++);
			fAngle += fAngleDelta;
			// zero the spot we just read
			*(SpaceLCur)=0;
			*(SpaceRCur)=0;
			// advance the pointer to the next spot
			SpaceLCur = SpaceLCur < SpaceLEnd ? SpaceLCur + 1 : SpaceLStr;
			SpaceRCur = SpaceRCur < SpaceREnd ? SpaceRCur + 1 : SpaceRStr;
		}
		//zero envelope on in and out for meters
		EnvIn   =0;
		EnvOutL =0;
		EnvOutR =0;
		EnvDrive=0;
	}

	while(fAngle > PI_2) {
		fAngle -= PI_2;
	}
	// remember for next run
	plugin->SpaceLCur=SpaceLCur;
	plugin->SpaceRCur=SpaceRCur;

	plugin->AngleLast   = fAngle; 
	plugin->EnvInLLast  = (fabs(EnvIn)<1.0e-10)   ? 0.f  : EnvIn; 
	plugin->EnvOutLLast = (fabs(EnvOutL)<1.0e-10)  ? 0.f : EnvOutL; 
	plugin->EnvOutRLast = (fabs(EnvOutR)<1.0e-10)  ? 0.f : EnvOutR; 
	plugin->EnvDriveLast = (fabs(EnvDrive)<1.0e-10)  ? 0.f : EnvDrive; 

	// update the meters
	*(plugin->LampNoClip)=EnvDrive;
	*(plugin->LampLfoL) = 1.75*(1+(cos(fAngle)));
	*(plugin->LampLfoR) = 1.75*(1+(cos(fAngle+fRPhase)));
	*(plugin->MeterInputL) =(EnvIn   > 0.001) ? 20*log10(EnvIn)   : -90.0;
	*(plugin->MeterOutputL)=(EnvOutL > 0.001) ? 20*log10(EnvOutL) : -90.0;
	*(plugin->MeterOutputR)=(EnvOutR > 0.001) ? 20*log10(EnvOutR) : -90.0;
}


static void 
runStereoIFlange(LV2_Handle instance, uint32_t SampleCount) 
{
	float (*pParamFunc)(unsigned long, float, double) = NULL;
	float * pfAudioInputL;
	float * pfAudioInputR;
	float * pfAudioOutputL;
	float * pfAudioOutputR;
	float fOffset;
	float fAngle;
	float InL,InR,OutL,OutR;
	float EnvInL,EnvInR,EnvOutL,EnvOutR,EnvDrive;
	float drive;
	float driveL=0;
	float driveR=0;
	float fBypass,fAngleDelta,fRPhase,fMaxDelay2,fDepth,fNoClip;
	float fDelayL,fDelayOffsetL;
	float fDelayR,fDelayOffsetR;
	unsigned long lDelaySampleL;
	unsigned long lDelaySampleR;
	unsigned long lSampleIndex;
	unsigned long SpaceSize;
	float * SpaceLStr;
	float * SpaceRStr;
	float * SpaceLCur;
	float * SpaceRCur;
	float * SpaceLEnd;
	float * SpaceREnd;

	IFlange *plugin = (IFlange *)instance;
	pParamFunc = &convertParam;

	/* check if any other params have changed */
	checkParamChange(IFLANGE_BYPASS, plugin->ControlBypass, &(plugin->LastBypass), &(plugin->ConvertedBypass), plugin->SampleRate, pParamFunc);
	checkParamChange(IFLANGE_CYCLE,  plugin->ControlCycle,  &(plugin->LastCycle),  &(plugin->ConvertedCycle),  plugin->SampleRate, pParamFunc);
	checkParamChange(IFLANGE_PHASE,  plugin->ControlPhase,  &(plugin->LastPhase),  &(plugin->ConvertedPhase),  plugin->SampleRate, pParamFunc);
	checkParamChange(IFLANGE_WIDTH,  plugin->ControlWidth,  &(plugin->LastWidth),  &(plugin->ConvertedWidth),  plugin->SampleRate, pParamFunc);
	checkParamChange(IFLANGE_DEPTH,  plugin->ControlDepth,  &(plugin->LastDepth),  &(plugin->ConvertedDepth),  plugin->SampleRate, pParamFunc);
	checkParamChange(IFLANGE_NOCLIP, plugin->ControlNoClip, &(plugin->LastNoClip), &(plugin->ConvertedNoClip), plugin->SampleRate, pParamFunc);


	fBypass         = plugin->ConvertedBypass;
	fAngleDelta	= plugin->ConvertedCycle;
	fRPhase   	= plugin->ConvertedPhase;
	fMaxDelay2	= plugin->ConvertedWidth;
	fDepth		= plugin->ConvertedDepth;
	fNoClip		= plugin->ConvertedNoClip;
	
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

	fOffset		= plugin->Offset;
	fAngle		= plugin->AngleLast;
	EnvInL     	= plugin->EnvInLLast;
	EnvInR     	= plugin->EnvInRLast;
	EnvOutL    	= plugin->EnvOutLLast;
	EnvOutR   	= plugin->EnvOutRLast;
	EnvDrive   	= plugin->EnvDriveLast;

	if(fBypass==0) { 
		for (lSampleIndex = 0; lSampleIndex < SampleCount; lSampleIndex++) {

			InL=*(pfAudioInputL++);
			InR=*(pfAudioInputR++);

			fDelayL=fOffset + (fMaxDelay2 * ( 1 + cos(fAngle)));
			fDelayR=fOffset + (fMaxDelay2 * ( 1 + cos(fAngle+fRPhase)));

			fAngle += fAngleDelta;

			lDelaySampleL=(unsigned long)fDelayL;
			lDelaySampleR=(unsigned long)fDelayR;

			fDelayOffsetL=fDelayL-(float)lDelaySampleL;
			fDelayOffsetR=fDelayR-(float)lDelaySampleR;

			// add to the delay space
			if(SpaceLCur+lDelaySampleL > SpaceLEnd)
				*(SpaceLCur+lDelaySampleL-SpaceSize)-=fDepth*InL*(1-fDelayOffsetL);
			else
				*(SpaceLCur+lDelaySampleL)-=fDepth*InL*(1-fDelayOffsetL);

			if(SpaceLCur+lDelaySampleL+1 > SpaceLEnd)
				*(SpaceLCur+lDelaySampleL-SpaceSize+1)-=fDepth*InL*fDelayOffsetL;
			else
				*(SpaceLCur+lDelaySampleL+1)-=fDepth*InL*fDelayOffsetL;

			if(SpaceRCur+lDelaySampleR > SpaceREnd)
				*(SpaceRCur+lDelaySampleR-SpaceSize)-=fDepth*InR*(1-fDelayOffsetR);
			else
				*(SpaceRCur+lDelaySampleR)-=fDepth*InR*(1-fDelayOffsetR);

			if(SpaceRCur+lDelaySampleR+1 > SpaceREnd)
				*(SpaceRCur+lDelaySampleR-SpaceSize+1)-=fDepth*InR*fDelayOffsetR;
			else
				*(SpaceRCur+lDelaySampleR+1)-=fDepth*InR*fDelayOffsetR;


			// read the audio out of the delay space
			OutL = InL + *(SpaceLCur);
			OutR = InR + *(SpaceRCur);
			OutL = fNoClip > 0 ? InoClip(OutL,&driveL) : OutL;
			OutR = fNoClip > 0 ? InoClip(OutR,&driveR) : OutR;
			*(pfAudioOutputL++) = OutL;
			*(pfAudioOutputR++) = OutR;
			// zero the spot we just read
			*(SpaceLCur)=0;
			*(SpaceRCur)=0;
			// advance the pointer to the next spot
			SpaceLCur = SpaceLCur < SpaceLEnd ? SpaceLCur + 1 : SpaceLStr;
			SpaceRCur = SpaceRCur < SpaceREnd ? SpaceRCur + 1 : SpaceRStr;

			//evelope on in and out for meters
			EnvInL  += IEnvelope(InL, EnvInL, INVADA_METER_PEAK,plugin->SampleRate);
			EnvInR  += IEnvelope(InR, EnvInR, INVADA_METER_PEAK,plugin->SampleRate);
			EnvOutL += IEnvelope(OutL,EnvOutL,INVADA_METER_PEAK,plugin->SampleRate);
			EnvOutR += IEnvelope(OutR,EnvOutR,INVADA_METER_PEAK,plugin->SampleRate);

			drive = driveL > driveR ? driveL : driveR;
			EnvDrive += IEnvelope(drive,EnvDrive,INVADA_METER_LAMP,plugin->SampleRate);
		}
	} else {
		for (lSampleIndex = 0; lSampleIndex < SampleCount; lSampleIndex++) {

			*(pfAudioOutputL++) = *(pfAudioInputL++);
			*(pfAudioOutputR++) = *(pfAudioInputR++);
			fAngle += fAngleDelta;
			// zero the spot we just read
			*(SpaceLCur)=0;
			*(SpaceRCur)=0;
			// advance the pointer to the next spot
			SpaceLCur = SpaceLCur < SpaceLEnd ? SpaceLCur + 1 : SpaceLStr;
			SpaceRCur = SpaceRCur < SpaceREnd ? SpaceRCur + 1 : SpaceRStr;
		}
		//zero envelope on in and out for meters
		EnvInL  =0;
		EnvInR  =0;
		EnvOutL =0;
		EnvOutR =0;
		EnvDrive=0;
	}

	while(fAngle > PI_2) {
		fAngle -= PI_2;
	}
	// remember for next run
	plugin->SpaceLCur=SpaceLCur;
	plugin->SpaceRCur=SpaceRCur;

	plugin->AngleLast    = fAngle; 
	plugin->EnvInLLast   = (fabs(EnvInL)<1.0e-10)   ? 0.f : EnvInL; 
	plugin->EnvInRLast   = (fabs(EnvInR)<1.0e-10)   ? 0.f : EnvInR; 
	plugin->EnvOutLLast  = (fabs(EnvOutL)<1.0e-10)  ? 0.f : EnvOutL; 
	plugin->EnvOutRLast  = (fabs(EnvOutR)<1.0e-10)  ? 0.f : EnvOutR; 
	plugin->EnvDriveLast = (fabs(EnvDrive)<1.0e-10) ? 0.f : EnvDrive; 

	// update the meters
	*(plugin->LampNoClip)=EnvDrive;
	*(plugin->LampLfoL) = 1.75*(1+(cos(fAngle)));
	*(plugin->LampLfoR) = 1.75*(1+(cos(fAngle+fRPhase)));
	*(plugin->MeterInputL) =(EnvInL  > 0.001) ? 20*log10(EnvInL)  : -90.0;
	*(plugin->MeterInputR) =(EnvInR  > 0.001) ? 20*log10(EnvInR)  : -90.0;
	*(plugin->MeterOutputL)=(EnvOutL > 0.001) ? 20*log10(EnvOutL) : -90.0;
	*(plugin->MeterOutputR)=(EnvOutR > 0.001) ? 20*log10(EnvOutR) : -90.0;
}



static void 
runSumIFlange(LV2_Handle instance, uint32_t SampleCount) 
{
	float (*pParamFunc)(unsigned long, float, double) = NULL;
	float * pfAudioInputL;
	float * pfAudioInputR;
	float * pfAudioOutputL;
	float * pfAudioOutputR;
	float fOffset;
	float fAngle;
	float In,OutL,OutR;
	float EnvIn,EnvOutL,EnvOutR,EnvDrive;
	float drive;
	float driveL=0;
	float driveR=0;
	float fBypass,fAngleDelta,fRPhase,fMaxDelay2,fDepth,fNoClip;
	float fDelayL,fDelayOffsetL;
	float fDelayR,fDelayOffsetR;
	unsigned long lDelaySampleL;
	unsigned long lDelaySampleR;
	unsigned long lSampleIndex;
	unsigned long SpaceSize;
	float * SpaceLStr;
	float * SpaceRStr;
	float * SpaceLCur;
	float * SpaceRCur;
	float * SpaceLEnd;
	float * SpaceREnd;

	IFlange *plugin = (IFlange *)instance;
	pParamFunc = &convertParam;

	/* check if any other params have changed */
	checkParamChange(IFLANGE_BYPASS, plugin->ControlBypass, &(plugin->LastBypass), &(plugin->ConvertedBypass), plugin->SampleRate, pParamFunc);
	checkParamChange(IFLANGE_CYCLE,  plugin->ControlCycle,  &(plugin->LastCycle),  &(plugin->ConvertedCycle),  plugin->SampleRate, pParamFunc);
	checkParamChange(IFLANGE_PHASE,  plugin->ControlPhase,  &(plugin->LastPhase),  &(plugin->ConvertedPhase),  plugin->SampleRate, pParamFunc);
	checkParamChange(IFLANGE_WIDTH,  plugin->ControlWidth,  &(plugin->LastWidth),  &(plugin->ConvertedWidth),  plugin->SampleRate, pParamFunc);
	checkParamChange(IFLANGE_DEPTH,  plugin->ControlDepth,  &(plugin->LastDepth),  &(plugin->ConvertedDepth),  plugin->SampleRate, pParamFunc);
	checkParamChange(IFLANGE_NOCLIP, plugin->ControlNoClip, &(plugin->LastNoClip), &(plugin->ConvertedNoClip), plugin->SampleRate, pParamFunc);


	fBypass         = plugin->ConvertedBypass;
	fAngleDelta	= plugin->ConvertedCycle;
	fRPhase   	= plugin->ConvertedPhase;
	fMaxDelay2	= plugin->ConvertedWidth;
	fDepth		= plugin->ConvertedDepth;
	fNoClip		= plugin->ConvertedNoClip;
	
	SpaceSize 	= plugin->SpaceSize;

	SpaceLStr	= plugin->SpaceL;
	SpaceRStr 	= plugin->SpaceR;
	SpaceLCur 	= plugin->SpaceLCur;
	SpaceRCur 	= plugin->SpaceRCur;
	SpaceLEnd 	= plugin->SpaceLEnd;
	SpaceREnd 	= plugin->SpaceREnd;

	pfAudioInputL 	= plugin->AudioInputBufferL;
	pfAudioInputR 	= plugin->AudioInputBufferL;
	pfAudioOutputL 	= plugin->AudioOutputBufferL;
	pfAudioOutputR 	= plugin->AudioOutputBufferR;

	fOffset		= plugin->Offset;
	fAngle		= plugin->AngleLast;
	EnvIn     	= plugin->EnvInLLast;
	EnvOutL    	= plugin->EnvOutLLast;
	EnvOutR   	= plugin->EnvOutRLast;
	EnvDrive   	= plugin->EnvDriveLast;

	if(fBypass==0) { 
		for (lSampleIndex = 0; lSampleIndex < SampleCount; lSampleIndex++) {

			In=(*(pfAudioInputL++) + *(pfAudioInputR++))/2;

			fDelayL=fOffset + (fMaxDelay2 * ( 1 + cos(fAngle)));
			fDelayR=fOffset + (fMaxDelay2 * ( 1 + cos(fAngle+fRPhase)));

			fAngle += fAngleDelta;

			lDelaySampleL=(unsigned long)fDelayL;
			lDelaySampleR=(unsigned long)fDelayR;

			fDelayOffsetL=fDelayL-(float)lDelaySampleL;
			fDelayOffsetR=fDelayR-(float)lDelaySampleR;

			// add to the delay space
			if(SpaceLCur+lDelaySampleL > SpaceLEnd)
				*(SpaceLCur+lDelaySampleL-SpaceSize)-=fDepth*In*(1-fDelayOffsetL);
			else
				*(SpaceLCur+lDelaySampleL)-=fDepth*In*(1-fDelayOffsetL);

			if(SpaceLCur+lDelaySampleL+1 > SpaceLEnd)
				*(SpaceLCur+lDelaySampleL-SpaceSize+1)-=fDepth*In*fDelayOffsetL;
			else
				*(SpaceLCur+lDelaySampleL+1)-=fDepth*In*fDelayOffsetL;

			if(SpaceRCur+lDelaySampleR > SpaceREnd)
				*(SpaceRCur+lDelaySampleR-SpaceSize)-=fDepth*In*(1-fDelayOffsetR);
			else
				*(SpaceRCur+lDelaySampleR)-=fDepth*In*(1-fDelayOffsetR);

			if(SpaceRCur+lDelaySampleR+1 > SpaceREnd)
				*(SpaceRCur+lDelaySampleR-SpaceSize+1)-=fDepth*In*fDelayOffsetR;
			else
				*(SpaceRCur+lDelaySampleR+1)-=fDepth*In*fDelayOffsetR;


			// read the audio out of the delay space
			OutL = In + *(SpaceLCur);
			OutR = In + *(SpaceRCur);
			OutL = fNoClip > 0 ? InoClip(OutL,&driveL) : OutL;
			OutR = fNoClip > 0 ? InoClip(OutR,&driveR) : OutR;
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
			drive = driveL > driveR ? driveL : driveR;
			EnvDrive += IEnvelope(drive,EnvDrive,INVADA_METER_LAMP,plugin->SampleRate);
		}
	} else {
		for (lSampleIndex = 0; lSampleIndex < SampleCount; lSampleIndex++) {

			*(pfAudioOutputL++) = *pfAudioInputL;
			*(pfAudioOutputR++) = *(pfAudioInputL++);
			fAngle += fAngleDelta;
			// zero the spot we just read
			*(SpaceLCur)=0;
			*(SpaceRCur)=0;
			// advance the pointer to the next spot
			SpaceLCur = SpaceLCur < SpaceLEnd ? SpaceLCur + 1 : SpaceLStr;
			SpaceRCur = SpaceRCur < SpaceREnd ? SpaceRCur + 1 : SpaceRStr;
		}
		//zero envelope on in and out for meters
		EnvIn   =0;
		EnvOutL =0;
		EnvOutR =0;
		EnvDrive=0;
	}

	while(fAngle > PI_2) {
		fAngle -= PI_2;
	}
	// remember for next run
	plugin->SpaceLCur=SpaceLCur;
	plugin->SpaceRCur=SpaceRCur;

	plugin->AngleLast   = fAngle; 
	plugin->EnvInLLast  = (fabs(EnvIn)<1.0e-10)   ? 0.f  : EnvIn; 
	plugin->EnvOutLLast = (fabs(EnvOutL)<1.0e-10)  ? 0.f : EnvOutL; 
	plugin->EnvOutRLast = (fabs(EnvOutR)<1.0e-10)  ? 0.f : EnvOutR; 
	plugin->EnvDriveLast = (fabs(EnvDrive)<1.0e-10)  ? 0.f : EnvDrive; 

	// update the meters
	*(plugin->LampNoClip)=EnvDrive;
	*(plugin->LampLfoL) = 1.75*(1+(cos(fAngle)));
	*(plugin->LampLfoR) = 1.75*(1+(cos(fAngle+fRPhase)));
	*(plugin->MeterInputL) =(EnvIn   > 0.001) ? 20*log10(EnvIn)   : -90.0;
	*(plugin->MeterOutputL)=(EnvOutL > 0.001) ? 20*log10(EnvOutL) : -90.0;
	*(plugin->MeterOutputR)=(EnvOutR > 0.001) ? 20*log10(EnvOutR) : -90.0;
}



static void 
cleanupIFlange(LV2_Handle instance)
{
	IFlange *plugin = (IFlange *)instance;

	free(plugin->SpaceL);
	free(plugin->SpaceR);
	free(instance);
}


static void 
init()
{
	IFlangeMonoDescriptor =
	 (LV2_Descriptor *)malloc(sizeof(LV2_Descriptor));

	IFlangeMonoDescriptor->URI 		= IFLANGE_MONO_URI;
	IFlangeMonoDescriptor->activate 	= activateIFlange;
	IFlangeMonoDescriptor->cleanup 	= cleanupIFlange;
	IFlangeMonoDescriptor->connect_port 	= connectPortIFlange;
	IFlangeMonoDescriptor->deactivate 	= NULL;
	IFlangeMonoDescriptor->instantiate 	= instantiateIFlange;
	IFlangeMonoDescriptor->run 		= runMonoIFlange;
	IFlangeMonoDescriptor->extension_data	= NULL;

	IFlangeStereoDescriptor =
	 (LV2_Descriptor *)malloc(sizeof(LV2_Descriptor));

	IFlangeStereoDescriptor->URI 		= IFLANGE_STEREO_URI;
	IFlangeStereoDescriptor->activate 	= activateIFlange;
	IFlangeStereoDescriptor->cleanup 	= cleanupIFlange;
	IFlangeStereoDescriptor->connect_port 	= connectPortIFlange;
	IFlangeStereoDescriptor->deactivate 	= NULL;
	IFlangeStereoDescriptor->instantiate 	= instantiateIFlange;
	IFlangeStereoDescriptor->run 		= runStereoIFlange;
	IFlangeStereoDescriptor->extension_data	= NULL;

	IFlangeSumDescriptor =
	 (LV2_Descriptor *)malloc(sizeof(LV2_Descriptor));

	IFlangeSumDescriptor->URI 		= IFLANGE_SUM_URI;
	IFlangeSumDescriptor->activate 		= activateIFlange;
	IFlangeSumDescriptor->cleanup 		= cleanupIFlange;
	IFlangeSumDescriptor->connect_port 	= connectPortIFlange;
	IFlangeSumDescriptor->deactivate 	= NULL;
	IFlangeSumDescriptor->instantiate 	= instantiateIFlange;
	IFlangeSumDescriptor->run 		= runSumIFlange;
	IFlangeSumDescriptor->extension_data	= NULL;

}


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
	if (!IFlangeMonoDescriptor) init();

	switch (index) {
		case 0:
			return IFlangeMonoDescriptor;
		case 1:
			return IFlangeStereoDescriptor;
		case 2:
			return IFlangeSumDescriptor;

		default:
			return NULL;
	}
}


/*****************************************************************************/
#define IFLANGE_CYCLE		1
#define IFLANGE_PHASE 		2
#define IFLANGE_WIDTH 		3
#define IFLANGE_DEPTH 		4
#define IFLANGE_NOCLIP 		5


float 
convertParam(unsigned long param, float value, double sr) {
	float result;

	switch(param)
	{
		case IFLANGE_BYPASS:
		case IFLANGE_NOCLIP:
			if(value<=0.0)
				result= 0; 
			else
				result= 1;
			break;
		case IFLANGE_CYCLE:
			if (value < 0.5)
				result = PI_2/(0.5*sr);
			else if (value <= 500.0)
				result = PI_2/(value * sr);
			else
				result = PI_2/(500.0 * sr);
			break;
		case IFLANGE_PHASE:
			if(value<-180)
				result = -PI;
			else if (value < 180)
				result = value*PI/180.0;
			else
				result = PI;
			break;
		case IFLANGE_WIDTH:
			if(value<1)
				result = sr/2000.0;
			else if (value < 15)
				result = value*sr/2000.0;
			else
				result = 15.0*sr/2000.0;
			break;
		case IFLANGE_DEPTH:
			if(value<0)
				result = 0.0;
			else if (value < 100)
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

