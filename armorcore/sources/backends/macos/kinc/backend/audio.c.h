
#ifdef KINC_A2

#include <CoreAudio/AudioHardware.h>
#include <CoreServices/CoreServices.h>
#include <kinc/audio2/audio.h>
#include <kinc/backend/video.h>
#include <kinc/log.h>
#include <stdio.h>

static kinc_internal_video_sound_stream_t *video = NULL;

void macPlayVideoSoundStream(kinc_internal_video_sound_stream_t *v) {
	video = v;
}

void macStopVideoSoundStream(void) {
	video = NULL;
}

static void affirm(OSStatus err) {
	if (err != kAudioHardwareNoError) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Error: %i\n", err);
	}
}

static bool initialized;
static bool soundPlaying;
static AudioDeviceID device;
static UInt32 deviceBufferSize;
static UInt32 size;
static AudioStreamBasicDescription deviceFormat;
static AudioObjectPropertyAddress address;

static AudioDeviceIOProcID theIOProcID = NULL;

static kinc_a2_buffer_t a2_buffer;

static uint32_t samples_per_second = 44100;

uint32_t kinc_a2_samples_per_second(void) {
	return samples_per_second;
}

static void copySample(void *buffer) {
	float left_value = *(float *)&a2_buffer.channels[0][a2_buffer.read_location];
	float right_value = *(float *)&a2_buffer.channels[1][a2_buffer.read_location];
	a2_buffer.read_location += 1;
	if (a2_buffer.read_location >= a2_buffer.data_size) {
		a2_buffer.read_location = 0;
	}
	((float *)buffer)[0] = left_value;
	((float *)buffer)[1] = right_value;
}

static OSStatus appIOProc(AudioDeviceID inDevice, const AudioTimeStamp *inNow, const AudioBufferList *inInputData, const AudioTimeStamp *inInputTime,
                          AudioBufferList *outOutputData, const AudioTimeStamp *inOutputTime, void *userdata) {
	affirm(AudioObjectGetPropertyData(device, &address, 0, NULL, &size, &deviceFormat));
	if (samples_per_second != (int)deviceFormat.mSampleRate) {
		samples_per_second = (int)deviceFormat.mSampleRate;
		kinc_a2_internal_sample_rate_callback();
	}
	int num_frames = deviceBufferSize / deviceFormat.mBytesPerFrame;
	kinc_a2_internal_callback(&a2_buffer, num_frames);
	float *output = (float *)outOutputData->mBuffers[0].mData;
	for (int i = 0; i < num_frames; ++i) {
		copySample(output);
		output += 2;
	}
	return kAudioHardwareNoError;
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

	device = kAudioDeviceUnknown;

	initialized = false;

	size = sizeof(AudioDeviceID);
	address.mSelector = kAudioHardwarePropertyDefaultOutputDevice;
	address.mScope = kAudioObjectPropertyScopeGlobal;
	address.mElement = kAudioObjectPropertyElementMaster;
	affirm(AudioObjectGetPropertyData(kAudioObjectSystemObject, &address, 0, NULL, &size, &device));

	size = sizeof(UInt32);
	address.mSelector = kAudioDevicePropertyBufferSize;
	address.mScope = kAudioDevicePropertyScopeOutput;
	affirm(AudioObjectGetPropertyData(device, &address, 0, NULL, &size, &deviceBufferSize));

	kinc_log(KINC_LOG_LEVEL_INFO, "deviceBufferSize = %i\n", deviceBufferSize);

	size = sizeof(AudioStreamBasicDescription);
	address.mSelector = kAudioDevicePropertyStreamFormat;
	address.mScope = kAudioDevicePropertyScopeOutput;

	affirm(AudioObjectGetPropertyData(device, &address, 0, NULL, &size, &deviceFormat));

	if (deviceFormat.mFormatID != kAudioFormatLinearPCM) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "mFormatID !=  kAudioFormatLinearPCM\n");
		return;
	}

	if (!(deviceFormat.mFormatFlags & kLinearPCMFormatFlagIsFloat)) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Only works with float format.\n");
		return;
	}

	if (samples_per_second != (int)deviceFormat.mSampleRate) {
		samples_per_second = (int)deviceFormat.mSampleRate;
		kinc_a2_internal_sample_rate_callback();
	}

	initialized = true;

	kinc_log(KINC_LOG_LEVEL_INFO, "mSampleRate = %g\n", deviceFormat.mSampleRate);
	kinc_log(KINC_LOG_LEVEL_INFO, "mFormatFlags = %08X\n", (unsigned int)deviceFormat.mFormatFlags);
	kinc_log(KINC_LOG_LEVEL_INFO, "mBytesPerPacket = %d\n", (unsigned int)deviceFormat.mBytesPerPacket);
	kinc_log(KINC_LOG_LEVEL_INFO, "mFramesPerPacket = %d\n", (unsigned int)deviceFormat.mFramesPerPacket);
	kinc_log(KINC_LOG_LEVEL_INFO, "mChannelsPerFrame = %d\n", (unsigned int)deviceFormat.mChannelsPerFrame);
	kinc_log(KINC_LOG_LEVEL_INFO, "mBytesPerFrame = %d\n", (unsigned int)deviceFormat.mBytesPerFrame);
	kinc_log(KINC_LOG_LEVEL_INFO, "mBitsPerChannel = %d\n", (unsigned int)deviceFormat.mBitsPerChannel);

	if (soundPlaying)
		return;

	affirm(AudioDeviceCreateIOProcID(device, appIOProc, NULL, &theIOProcID));
	affirm(AudioDeviceStart(device, theIOProcID));

	soundPlaying = true;
}

void kinc_a2_update(void) {}

void kinc_a2_shutdown(void) {
	if (!initialized)
		return;
	if (!soundPlaying)
		return;

	affirm(AudioDeviceStop(device, theIOProcID));
	affirm(AudioDeviceDestroyIOProcID(device, theIOProcID));

	soundPlaying = false;
}

#endif
