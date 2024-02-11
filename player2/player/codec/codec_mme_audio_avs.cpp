/************************************************************************
Copyright (C) 2007 STMicroelectronics. All Rights Reserved.

This file is part of the Player2 Library.

Player2 is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License version 2 as published by the
Free Software Foundation.

Player2 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with player2; see the file COPYING. If not, write to the Free Software
Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

The Player2 Library may alternatively be licensed under a proprietary
license from ST.

Source file name : codec_mme_audio_avs.cpp
Author : Daniel

Implementation of the avs2 audio codec class for player 2.

Date Modification Name
---- ------------ --------
26-Apr-07 Created (from codec_mme_video_avs2.cpp) Daniel

************************************************************************/

////////////////////////////////////////////////////////////////////////////
/// \class Codec_MmeAudioAvs_c
///
/// The AVS audio codec proxy.
///

// /////////////////////////////////////////////////////////////////////
//
// Include any component headers

#include "codec_mme_audio_avs.h"
#include "avs_audio.h"

// /////////////////////////////////////////////////////////////////////////
//
// Locally defined constants
//

// /////////////////////////////////////////////////////////////////////////
//
// Locally defined structures
//

typedef struct AvsAudioCodecStreamParameterContext_s
{
	CodecBaseStreamParameterContext_t BaseContext;

	MME_LxAudioDecoderGlobalParams_t StreamParameters;
} AvsAudioCodecStreamParameterContext_t;

//#if __KERNEL__
#if 0
#define BUFFER_AVS_AUDIO_CODEC_STREAM_PARAMETER_CONTEXT "AvsAudioCodecStreamParameterContext"
#define BUFFER_AVS_AUDIO_CODEC_STREAM_PARAMETER_CONTEXT_TYPE {BUFFER_AVS_AUDIO_CODEC_STREAM_PARAMETER_CONTEXT, BufferDataTypeBase, AllocateFromDeviceMemory, 32, 0, true, true, sizeof(AvsAudioCodecStreamParameterContext_t)}
#else
#define BUFFER_AVS_AUDIO_CODEC_STREAM_PARAMETER_CONTEXT "AvsAudioCodecStreamParameterContext"
#define BUFFER_AVS_AUDIO_CODEC_STREAM_PARAMETER_CONTEXT_TYPE {BUFFER_AVS_AUDIO_CODEC_STREAM_PARAMETER_CONTEXT, BufferDataTypeBase, AllocateFromOSMemory, 32, 0, true, true, sizeof(AvsAudioCodecStreamParameterContext_t)}
#endif

static BufferDataDescriptor_t AvsAudioCodecStreamParameterContextDescriptor = BUFFER_AVS_AUDIO_CODEC_STREAM_PARAMETER_CONTEXT_TYPE;

// --------

typedef struct AvsAudioCodecDecodeContext_s
{
	CodecBaseDecodeContext_t BaseContext;

	MME_LxAudioDecoderFrameParams_t DecodeParameters;
	MME_LxAudioDecoderFrameStatus_t DecodeStatus;
} AvsAudioCodecDecodeContext_t;

//#if __KERNEL__
#if 0
#define BUFFER_AVS_AUDIO_CODEC_DECODE_CONTEXT "AvsAudioCodecDecodeContext"
#define BUFFER_AVS_AUDIO_CODEC_DECODE_CONTEXT_TYPE {BUFFER_AVS_AUDIO_CODEC_DECODE_CONTEXT, BufferDataTypeBase, AllocateFromDeviceMemory, 32, 0, true, true, sizeof(AvsAudioCodecDecodeContext_t)}
#else
#define BUFFER_AVS_AUDIO_CODEC_DECODE_CONTEXT "AvsAudioCodecDecodeContext"
#define BUFFER_AVS_AUDIO_CODEC_DECODE_CONTEXT_TYPE {BUFFER_AVS_AUDIO_CODEC_DECODE_CONTEXT, BufferDataTypeBase, AllocateFromOSMemory, 32, 0, true, true, sizeof(AvsAudioCodecDecodeContext_t)}
#endif

static BufferDataDescriptor_t AvsAudioCodecDecodeContextDescriptor = BUFFER_AVS_AUDIO_CODEC_DECODE_CONTEXT_TYPE;

////////////////////////////////////////////////////////////////////////////
///
/// Fill in the configuration parameters used by the super-class and reset everything.
///
Codec_MmeAudioAvs_c::Codec_MmeAudioAvs_c(void)
{
	Configuration.CodecName = "AVS audio";
	Configuration.StreamParameterContextCount = 1;
	Configuration.StreamParameterContextDescriptor = &AvsAudioCodecStreamParameterContextDescriptor;
	Configuration.DecodeContextCount = 4;
	Configuration.DecodeContextDescriptor = &AvsAudioCodecDecodeContextDescriptor;
//
	AudioDecoderTransformCapabilityMask.DecoderCapabilityFlags = (1 << ACC_MP2a) /*| (1 << ACC_MP3)*/;
	DecoderId = ACC_MP2A_ID;
	Reset();
}

////////////////////////////////////////////////////////////////////////////
///
/// Destructor function, ensures a full halt and reset
/// are executed for all levels of the class.
///
Codec_MmeAudioAvs_c::~Codec_MmeAudioAvs_c(void)
{
	Halt();
	Reset();
}

////////////////////////////////////////////////////////////////////////////
///
/// Populate the supplied structure with parameters for AVS audio.
///
/// \todo This method explicitly disables multi-channel AVS decoding in order
/// to hide some bugs in the audio firmware. The problem was originally
/// observed in BL012_5 and may have long since been fixed...
///
CodecStatus_t Codec_MmeAudioAvs_c::FillOutTransformerGlobalParameters(MME_LxAudioDecoderGlobalParams_t *GlobalParams_p)
{
//
	CODEC_TRACE("Initializing AVS layer %s audio decoder\n",
		    (DecoderId == ACC_MP2A_ID ? "I/II" :
		     DecoderId == ACC_MP3_ID ? "III" :
		     "unknown"));
	// check for firmware decoder existence in case of SET_GLOBAL only
	// (we don't know the frame type at init time)
	if (ParsedFrameParameters)
	{
		AvsAudioStreamParameters_t *Parsed = (AvsAudioStreamParameters_t *)ParsedFrameParameters->StreamParameterStructure;
		if (Parsed && Parsed->Layer)
		{
			int mask = (DecoderId == ACC_MP2A_ID) ? ACC_MP2a : ACC_MP3;
			if (!(AudioDecoderTransformCapability.DecoderCapabilityFlags & (1 << mask)))
			{
				CODEC_ERROR("This type of AVS Layer is not supported by the firmware!\n");
				Player->MarkStreamUnPlayable(Stream);
				return (CodecError);
			}
		}
	}
//
	MME_LxAudioDecoderGlobalParams_t &GlobalParams = *GlobalParams_p;
	GlobalParams.StructSize = sizeof(MME_LxAudioDecoderGlobalParams_t);
//
	MME_LxMp2aConfig_t &Config = *((MME_LxMp2aConfig_t *) GlobalParams.DecConfig);
	Config.DecoderId = DecoderId;
	Config.StructSize = sizeof(Config);
	Config.Config[MP2a_CRC_ENABLE] = ACC_MME_TRUE;
	Config.Config[MP2a_LFE_ENABLE] = ACC_MME_FALSE; // low frequency effects
	Config.Config[MP2a_DRC_ENABLE] = ACC_MME_FALSE; // dynamic range compression
#if 0
	// TODO: enabling multi-channel decode can cause a firmware crash when decoding
	// AVS-1 audio (without multi-channel).
	Config.Config[MP2a_MC_ENABLE] = ACC_MME_TRUE; // multi-channel
#else
	Config.Config[MP2a_MC_ENABLE] = ACC_MME_FALSE;
#endif
	Config.Config[MP2a_NIBBLE_ENABLE] = ACC_MME_FALSE; // ?part of frame analysis?
	Config.Config[MP3_FREE_FORMAT] = ACC_MME_TRUE; // "free bit rate streams"
	Config.Config[MP3_MUTE_ON_ERROR] = ACC_MME_TRUE;
	Config.Config[MP3_DECODE_LAST_FRAME] = ACC_MME_TRUE;
//
	return Codec_MmeAudio_c::FillOutTransformerGlobalParameters(GlobalParams_p);
}

////////////////////////////////////////////////////////////////////////////
///
/// Populate the AUDIO_DECODER's initialization parameters for AVS audio.
///
/// When this method completes Codec_MmeAudio_c::AudioDecoderInitializationParameters
/// will have been filled out with valid values sufficient to initialize an
/// AVS audio decoder (defaults to AVS Layer II but can be updated by new
/// stream parameters).
///
CodecStatus_t Codec_MmeAudioAvs_c::FillOutTransformerInitializationParameters(void)
{
	CodecStatus_t Status;
	MME_LxAudioDecoderInitParams_t &Params = AudioDecoderInitializationParameters;
//
	MMEInitializationParameters.TransformerInitParamsSize = sizeof(Params);
	MMEInitializationParameters.TransformerInitParams_p = &Params;
//
	Status = Codec_MmeAudio_c::FillOutTransformerInitializationParameters();
	if (Status != CodecNoError)
		return Status;
//
	return FillOutTransformerGlobalParameters(&Params.GlobalParams);
}

////////////////////////////////////////////////////////////////////////////
///
/// Populate the AUDIO_DECODER's MME_SET_GLOBAL_TRANSFORMER_PARAMS parameters for AVS audio.
///
CodecStatus_t Codec_MmeAudioAvs_c::FillOutSetStreamParametersCommand(void)
{
	CodecStatus_t Status;
	AvsAudioStreamParameters_t *Parsed = (AvsAudioStreamParameters_t *)ParsedFrameParameters->StreamParameterStructure;
	AvsAudioCodecStreamParameterContext_t *Context = (AvsAudioCodecStreamParameterContext_t *)StreamParameterContext;
//Parsed may be NULL if call to this function results from an ALSA parameter update.
	if (Parsed)
	{
		//
		// Examine the parsed stream parameters and determine what type of codec to instanciate
		//
		DecoderId = Parsed->Layer == 3 ? ACC_MP3_ID : ACC_MP2A_ID;
	}
	//
	// Now fill out the actual structure
	//
	memset(&(Context->StreamParameters), 0, sizeof(Context->StreamParameters));
	Status = FillOutTransformerGlobalParameters(&(Context->StreamParameters));
	if (Status != CodecNoError)
		return Status;
	//
	// Fill out the actual command
	//
	Context->BaseContext.MMECommand.CmdStatus.AdditionalInfoSize = 0;
	Context->BaseContext.MMECommand.CmdStatus.AdditionalInfo_p = NULL;
	Context->BaseContext.MMECommand.ParamSize = sizeof(Context->StreamParameters);
	Context->BaseContext.MMECommand.Param_p = (MME_GenericParams_t)(&Context->StreamParameters);
//
	return CodecNoError;
}

////////////////////////////////////////////////////////////////////////////
///
/// Populate the AUDIO_DECODER's MME_TRANSFORM parameters for AVS audio.
///
CodecStatus_t Codec_MmeAudioAvs_c::FillOutDecodeCommand(void)
{
	AvsAudioCodecDecodeContext_t *Context = (AvsAudioCodecDecodeContext_t *)DecodeContext;
//AvsAudioFrameParameters_t *Parsed = (AvsAudioFrameParameters_t *)ParsedFrameParameters->FrameParameterStructure;
	//
	// Initialize the frame parameters (we don't actually have much to say here)
	//
	memset(&Context->DecodeParameters, 0, sizeof(Context->DecodeParameters));
	//
	// Zero the reply structure
	//
	memset(&Context->DecodeStatus, 0, sizeof(Context->DecodeStatus));
	//
	// Fill out the actual command
	//
	Context->BaseContext.MMECommand.CmdStatus.AdditionalInfoSize = sizeof(Context->DecodeStatus);
	Context->BaseContext.MMECommand.CmdStatus.AdditionalInfo_p = (MME_GenericParams_t)(&Context->DecodeStatus);
	Context->BaseContext.MMECommand.ParamSize = sizeof(Context->DecodeParameters);
	Context->BaseContext.MMECommand.Param_p = (MME_GenericParams_t)(&Context->DecodeParameters);
	return CodecNoError;
}

////////////////////////////////////////////////////////////////////////////
///
/// Validate the ACC status structure and squawk loudly if problems are found.
///
/// Dispite the squawking this method unconditionally returns success. This is
/// because the firmware will already have concealed the decode problems by
/// performing a soft mute.
///
/// \return CodecSuccess
///
CodecStatus_t Codec_MmeAudioAvs_c::ValidateDecodeContext(CodecBaseDecodeContext_t *Context)
{
	AvsAudioCodecDecodeContext_t *DecodeContext = (AvsAudioCodecDecodeContext_t *) Context;
	MME_LxAudioDecoderFrameStatus_t &Status = DecodeContext->DecodeStatus;
	ParsedAudioParameters_t *AudioParameters;
	CODEC_DEBUG(">><<\n");
	if (ENABLE_CODEC_DEBUG)
	{
		//DumpCommand(bufferIndex);
	}
	if (Status.DecStatus != ACC_AVS2_OK && Status.DecStatus != ACC_AVS_MC_CRC_ERROR)
	{
		CODEC_ERROR("AVS audio decode error (muted frame): %d\n", Status.DecStatus);
		//DumpCommand(bufferIndex);
		// don't report an error to the higher levels (because the frame is muted)
	}
	if (Status.NbOutSamples != 1152 && Status.NbOutSamples != 576) // TODO: manifest constant
	{
		// 288/144 aren't valid values (but testing showed that it was emited for some
		// of the AVS2 streams for example WrenTesting/AVS2_LIII/AVS2_LIII_80kbps_24khz_s.mp3)
		// so we continue to output a message if this is observed but only if diagnostics are
		// enabled.
		if (Status.NbOutSamples == 288 || Status.NbOutSamples == 144)
			CODEC_DEBUG("Unexpected number of output samples (%d)\n", Status.NbOutSamples);
		else
			CODEC_ERROR("Unexpected number of output samples (%d)\n", Status.NbOutSamples);
		//DumpCommand(bufferIndex);
	}
	// SYSFS
	AudioDecoderStatus = Status;
	//
	// Attach any codec derived metadata to the output buffer (or verify the
	// frame analysis if the frame analyser already filled everything in for
	// us).
	//
	AudioParameters = BufferState[DecodeContext->BaseContext.BufferIndex].ParsedAudioParameters;
	AudioParameters->Source.BitsPerSample = AudioOutputSurface->BitsPerSample;
	AudioParameters->Source.ChannelCount = AudioOutputSurface->ChannelCount;
	AudioParameters->Organisation = Status.AudioMode;
	AudioParameters->SampleCount = Status.NbOutSamples;
	int SamplingFreqCode = Status.SamplingFreq;
	if (SamplingFreqCode < ACC_FS_reserved)
	{
		AudioParameters->Source.SampleRateHz = ACC_SamplingFreqLUT[SamplingFreqCode];
	}
	else
	{
		AudioParameters->Source.SampleRateHz = 0;
		CODEC_ERROR("AVS audio decode bad sampling freq returned: 0x%x\n", SamplingFreqCode);
	}
	return CodecNoError;
}

// /////////////////////////////////////////////////////////////////////////
//
// Function to dump out the set stream
// parameters from an mme command.
//

CodecStatus_t Codec_MmeAudioAvs_c::DumpSetStreamParameters(void *Parameters)
{
	CODEC_ERROR("Not implemented\n");
	return CodecNoError;
}

// /////////////////////////////////////////////////////////////////////////
//
// Function to dump out the decode
// parameters from an mme command.
//

CodecStatus_t Codec_MmeAudioAvs_c::DumpDecodeParameters(void *Parameters)
{
	CODEC_ERROR("Not implemented\n");
	return CodecNoError;
}
