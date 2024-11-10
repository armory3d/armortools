#import <AudioToolbox/AudioToolbox.h>
#import <Foundation/Foundation.h>

#include <kinc/audio2/audio.h>
#include <kinc/backend/video.h>
#include <kinc/math/core.h>

#include <stdio.h>

#define kOutputBus 0

static kinc_internal_video_sound_stream_t *video = NULL;

void iosPlayVideoSoundStream(kinc_internal_video_sound_stream_t *v) {
	video = v;
}

void iosStopVideoSoundStream(void) {
	video = NULL;
}

static void affirm(OSStatus err) {
	if (err) {
		fprintf(stderr, "Error: %i\n", (int)err);
	}
}

static bool initialized;
static bool soundPlaying;
static AudioStreamBasicDescription deviceFormat;
static AudioComponentInstance audioUnit;
static bool isFloat = false;
static bool isInterleaved = true;

static kinc_a2_buffer_t a2_buffer;

static void copySample(void *buffer, void *secondary_buffer) {
	float left_value = *(float *)&a2_buffer.channels[0][a2_buffer.read_location];
	float right_value = *(float *)&a2_buffer.channels[1][a2_buffer.read_location];
	a2_buffer.read_location += 1;
	if (a2_buffer.read_location >= a2_buffer.data_size) {
		a2_buffer.read_location = 0;
	}

	if (video != NULL) {
		float *frame = kinc_internal_video_sound_stream_next_frame(video);
		left_value += frame[0];
		left_value = kinc_max(kinc_min(left_value, 1.0f), -1.0f);
		right_value += frame[1];
		right_value = kinc_max(kinc_min(right_value, 1.0f), -1.0f);
		if (kinc_internal_video_sound_stream_ended(video)) {
			video = NULL;
		}
	}

	if (secondary_buffer == NULL) {
		if (isFloat) {
			((float *)buffer)[0] = left_value;
			((float *)buffer)[1] = right_value;
		}
		else {
			((int16_t *)buffer)[0] = (int16_t)(left_value * 32767);
			((int16_t *)buffer)[1] = (int16_t)(right_value * 32767);
		}
	}
	else {
		if (isFloat) {
			*(float *)buffer = left_value;
			*(float *)secondary_buffer = right_value;
		}
		else {
			*(int16_t *)buffer = (int16_t)(left_value * 32767);
			*(int16_t *)secondary_buffer = (int16_t)(right_value * 32767);
		}
	}
}

static OSStatus renderInput(void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber,
                            UInt32 inNumberFrames, AudioBufferList *outOutputData) {
	kinc_a2_internal_callback(&a2_buffer, inNumberFrames);
	if (isInterleaved) {
		if (isFloat) {
			float *output = (float *)outOutputData->mBuffers[0].mData;
			for (int i = 0; i < inNumberFrames; ++i) {
				copySample(output, NULL);
				output += 2;
			}
		}
		else {
			int16_t *output = (int16_t *)outOutputData->mBuffers[0].mData;
			for (int i = 0; i < inNumberFrames; ++i) {
				copySample(output, NULL);
				output += 2;
			}
		}
	}
	else {
		if (isFloat) {
			float *out1 = (float *)outOutputData->mBuffers[0].mData;
			float *out2 = (float *)outOutputData->mBuffers[1].mData;
			for (int i = 0; i < inNumberFrames; ++i) {
				copySample(out1++, out2++);
			}
		}
		else {
			int16_t *out1 = (int16_t *)outOutputData->mBuffers[0].mData;
			int16_t *out2 = (int16_t *)outOutputData->mBuffers[1].mData;
			for (int i = 0; i < inNumberFrames; ++i) {
				copySample(out1++, out2++);
			}
		}
	}
	return noErr;
}

static uint32_t samples_per_second = 44100;

static void sampleRateListener(void *inRefCon, AudioUnit inUnit, AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement) {
	Float64 sampleRate;
	UInt32 size = sizeof(sampleRate);
	affirm(AudioUnitGetProperty(inUnit, kAudioUnitProperty_SampleRate, kAudioUnitScope_Output, 0, &sampleRate, &size));

	if (samples_per_second != (uint32_t)sampleRate) {
		samples_per_second = (uint32_t)sampleRate;
		kinc_a2_internal_sample_rate_callback();
	}
}

static bool initialized = false;

void kinc_a2_init(void) {
	if (initialized) {
		return;
	}

	kinc_a2_internal_init();
	initialized = true;

	a2_buffer.read_location = 0;
	a2_buffer.write_location = 0;
	a2_buffer.data_size = 128 * 1024;
	a2_buffer.channel_count = 2;
	a2_buffer.channels[0] = (float *)malloc(a2_buffer.data_size * sizeof(float));
	a2_buffer.channels[1] = (float *)malloc(a2_buffer.data_size * sizeof(float));

	initialized = false;

	AudioComponentDescription desc;
	desc.componentType = kAudioUnitType_Output;
	desc.componentSubType = kAudioUnitSubType_RemoteIO;
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;

	AudioComponent comp = AudioComponentFindNext(NULL, &desc);

	// Get audio units
	affirm(AudioComponentInstanceNew(comp, &audioUnit));
	UInt32 flag = 1;
	affirm(AudioUnitSetProperty(audioUnit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Output, kOutputBus, &flag, sizeof(UInt32)));

	if (soundPlaying)
		return;

	affirm(AudioOutputUnitStart(audioUnit));

	UInt32 size = sizeof(AudioStreamBasicDescription);
	affirm(AudioUnitGetProperty(audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, &deviceFormat, &size));

	if (deviceFormat.mFormatID != kAudioFormatLinearPCM) {
		fprintf(stderr, "mFormatID !=  kAudioFormatLinearPCM\n");
		return;
	}

	if (deviceFormat.mFormatFlags & kLinearPCMFormatFlagIsFloat) {
		isFloat = true;
	}

	if (deviceFormat.mFormatFlags & kAudioFormatFlagIsNonInterleaved) {
		isInterleaved = false;
	}

	AudioUnitAddPropertyListener(audioUnit, kAudioUnitProperty_StreamFormat, sampleRateListener, nil);

	initialized = true;

	printf("mSampleRate = %g\n", deviceFormat.mSampleRate);
	printf("mFormatFlags = %08X\n", (unsigned int)deviceFormat.mFormatFlags);
	printf("mBytesPerPacket = %d\n", (unsigned int)deviceFormat.mBytesPerPacket);
	printf("mFramesPerPacket = %d\n", (unsigned int)deviceFormat.mFramesPerPacket);
	printf("mChannelsPerFrame = %d\n", (unsigned int)deviceFormat.mChannelsPerFrame);
	printf("mBytesPerFrame = %d\n", (unsigned int)deviceFormat.mBytesPerFrame);
	printf("mBitsPerChannel = %d\n", (unsigned int)deviceFormat.mBitsPerChannel);

	if (samples_per_second != (uint32_t)deviceFormat.mSampleRate) {
		samples_per_second = (uint32_t)deviceFormat.mSampleRate;
		kinc_a2_internal_sample_rate_callback();
	}

	AURenderCallbackStruct callbackStruct;
	callbackStruct.inputProc = renderInput;
	callbackStruct.inputProcRefCon = NULL;
	affirm(AudioUnitSetProperty(audioUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Global, kOutputBus, &callbackStruct, sizeof(callbackStruct)));

	soundPlaying = true;
}

void kinc_a2_update(void) {}

void kinc_a2_shutdown(void) {
	if (!initialized)
		return;
	if (!soundPlaying)
		return;

	affirm(AudioOutputUnitStop(audioUnit));

	soundPlaying = false;
}

uint32_t kinc_a2_samples_per_second(void) {
	return samples_per_second;
}
