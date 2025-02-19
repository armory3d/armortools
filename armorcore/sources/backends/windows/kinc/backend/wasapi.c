
#ifdef KINC_A2

#include <kinc/audio2/audio.h>
#include <kinc/backend/system_microsoft.h>
#include <kinc/error.h>
#include <kinc/log.h>

// Windows 7
#define WINVER 0x0601
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0601

#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCOMM
#define NOCTLMGR
#define NODEFERWINDOWPOS
#define NODRAWTEXT
// #define NOGDI
#define NOGDICAPMASKS
#define NOHELP
#define NOICONS
#define NOKANJI
#define NOKEYSTATES
#define NOMB
#define NOMCX
#define NOMEMMGR
#define NOMENUS
#define NOMETAFILE
#define NOMINMAX
// #define NOMSG
#define NONLS
#define NOOPENFILE
#define NOPROFILER
#define NORASTEROPS
#define NOSCROLL
#define NOSERVICE
#define NOSHOWWINDOW
#define NOSOUND
#define NOSYSCOMMANDS
#define NOSYSMETRICS
#define NOTEXTMETRIC
// #define NOUSER
#define NOVIRTUALKEYCODES
#define NOWH
#define NOWINMESSAGES
#define NOWINOFFSETS
#define NOWINSTYLES
#define WIN32_LEAN_AND_MEAN

#include <initguid.h>

#include <AudioClient.h>
#include <mmdeviceapi.h>

#ifndef __MINGW32__
// MIDL_INTERFACE("1CB9AD4C-DBFA-4c32-B178-C2F568A703B2")
DEFINE_GUID(IID_IAudioClient, 0x1CB9AD4C, 0xDBFA, 0x4c32, 0xB1, 0x78, 0xC2, 0xF5, 0x68, 0xA7, 0x03, 0xB2);
// MIDL_INTERFACE("F294ACFC-3146-4483-A7BF-ADDCA7C260E2")
DEFINE_GUID(IID_IAudioRenderClient, 0xF294ACFC, 0x3146, 0x4483, 0xA7, 0xBF, 0xAD, 0xDC, 0xA7, 0xC2, 0x60, 0xE2);
// MIDL_INTERFACE("A95664D2-9614-4F35-A746-DE8DB63617E6")
DEFINE_GUID(IID_IMMDeviceEnumerator, 0xA95664D2, 0x9614, 0x4F35, 0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6);
// DECLSPEC_UUID("BCDE0395-E52F-467C-8E3D-C4579291692E")
DEFINE_GUID(CLSID_MMDeviceEnumerator, 0xBCDE0395, 0xE52F, 0x467C, 0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E);
#endif

// based on the implementation in soloud and Microsoft sample code
static kinc_a2_buffer_t a2_buffer;

static IMMDeviceEnumerator *deviceEnumerator;
static IMMDevice *device;
static IAudioClient *audioClient = NULL;
static IAudioRenderClient *renderClient = NULL;
static HANDLE bufferEndEvent = 0;
static HANDLE audioProcessingDoneEvent;
static UINT32 bufferFrames;
static WAVEFORMATEX requestedFormat;
static WAVEFORMATEX *closestFormat;
static WAVEFORMATEX *format;
static uint32_t samples_per_second = 44100;

uint32_t kinc_a2_samples_per_second(void) {
	return samples_per_second;
}

static bool initDefaultDevice() {
	if (renderClient != NULL) {
		renderClient->lpVtbl->Release(renderClient);
		renderClient = NULL;
	}

	if (audioClient != NULL) {
		audioClient->lpVtbl->Release(audioClient);
		audioClient = NULL;
	}

	if (bufferEndEvent != 0) {
		CloseHandle(bufferEndEvent);
		bufferEndEvent = 0;
	}

	kinc_log(KINC_LOG_LEVEL_INFO, "Initializing a new default audio device.");

	HRESULT hr = deviceEnumerator->lpVtbl->GetDefaultAudioEndpoint(deviceEnumerator, eRender, eConsole, &device);
	if (hr == S_OK) {
		hr = device->lpVtbl->Activate(device, &IID_IAudioClient, CLSCTX_ALL, 0, (void **)&audioClient);
	}

	if (hr == S_OK) {
		const int sampleRate = 48000;

		format = &requestedFormat;
		memset(&requestedFormat, 0, sizeof(WAVEFORMATEX));
		requestedFormat.nChannels = 2;
		requestedFormat.nSamplesPerSec = sampleRate;
		requestedFormat.wFormatTag = WAVE_FORMAT_PCM;
		requestedFormat.wBitsPerSample = sizeof(short) * 8;
		requestedFormat.nBlockAlign = (requestedFormat.nChannels * requestedFormat.wBitsPerSample) / 8;
		requestedFormat.nAvgBytesPerSec = requestedFormat.nSamplesPerSec * requestedFormat.nBlockAlign;
		requestedFormat.cbSize = 0;

		HRESULT supported = audioClient->lpVtbl->IsFormatSupported(audioClient, AUDCLNT_SHAREMODE_SHARED, format, &closestFormat);
		if (supported == S_FALSE) {
			kinc_log(KINC_LOG_LEVEL_WARNING, "Falling back to the system's preferred WASAPI mix format.", supported);
			if (closestFormat != NULL) {
				format = closestFormat;
			}
			else {
				audioClient->lpVtbl->GetMixFormat(audioClient, &format);
			}
		}
		HRESULT result =
		    audioClient->lpVtbl->Initialize(audioClient, AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, 40 * 1000 * 10, 0, format, 0);
		if (result != S_OK) {
			kinc_log(KINC_LOG_LEVEL_WARNING, "Could not initialize WASAPI audio, going silent (error code 0x%x).", result);
			return false;
		}

		uint32_t old_samples_per_second = samples_per_second;
		samples_per_second = format->nSamplesPerSec;
		if (samples_per_second != old_samples_per_second) {
			kinc_a2_internal_sample_rate_callback();
		}
		a2_buffer.channel_count = 2;

		bufferFrames = 0;
		kinc_microsoft_affirm(audioClient->lpVtbl->GetBufferSize(audioClient, &bufferFrames));
		kinc_microsoft_affirm(audioClient->lpVtbl->GetService(audioClient, &IID_IAudioRenderClient, (void **)&renderClient));

		bufferEndEvent = CreateEvent(0, FALSE, FALSE, 0);
		kinc_affirm(bufferEndEvent != 0);

		kinc_microsoft_affirm(audioClient->lpVtbl->SetEventHandle(audioClient, bufferEndEvent));

		return true;
	}
	else {
		kinc_log(KINC_LOG_LEVEL_WARNING, "Could not initialize WASAPI audio.");
		return false;
	}
}

static void copyS16Sample(int16_t *left, int16_t *right) {
	float left_value = *(float *)&a2_buffer.channels[0][a2_buffer.read_location];
	float right_value = *(float *)&a2_buffer.channels[1][a2_buffer.read_location];
	a2_buffer.read_location += 1;
	if (a2_buffer.read_location >= a2_buffer.data_size) {
		a2_buffer.read_location = 0;
	}
	*left = (int16_t)(left_value * 32767);
	*right = (int16_t)(right_value * 32767);
}

static void copyFloatSample(float *left, float *right) {
	float left_value = *(float *)&a2_buffer.channels[0][a2_buffer.read_location];
	float right_value = *(float *)&a2_buffer.channels[1][a2_buffer.read_location];
	a2_buffer.read_location += 1;
	if (a2_buffer.read_location >= a2_buffer.data_size) {
		a2_buffer.read_location = 0;
	}
	*left = left_value;
	*right = right_value;
}

static void submitEmptyBuffer(unsigned frames) {
	BYTE *buffer = NULL;
	HRESULT result = renderClient->lpVtbl->GetBuffer(renderClient, frames, &buffer);
	if (FAILED(result)) {
		return;
	}

	memset(buffer, 0, frames * format->nBlockAlign);

	result = renderClient->lpVtbl->ReleaseBuffer(renderClient, frames, 0);
}

static void submitBuffer(unsigned frames) {
	BYTE *buffer = NULL;
	HRESULT result = renderClient->lpVtbl->GetBuffer(renderClient, frames, &buffer);
	if (FAILED(result)) {
		if (result == AUDCLNT_E_DEVICE_INVALIDATED) {
			initDefaultDevice();
			submitEmptyBuffer(bufferFrames);
			audioClient->lpVtbl->Start(audioClient);
		}
		return;
	}

	if (kinc_a2_internal_callback(&a2_buffer, frames)) {
		if (format->wFormatTag == WAVE_FORMAT_PCM) {
			for (UINT32 i = 0; i < frames; ++i) {
				copyS16Sample((int16_t *)&buffer[i * format->nBlockAlign], (int16_t *)&buffer[i * format->nBlockAlign + 2]);
			}
		}
		else {
			for (UINT32 i = 0; i < frames; ++i) {
				copyFloatSample((float *)&buffer[i * format->nBlockAlign], (float *)&buffer[i * format->nBlockAlign + 4]);
			}
		}
	}
	else {
		memset(buffer, 0, frames * format->nBlockAlign);
	}

	result = renderClient->lpVtbl->ReleaseBuffer(renderClient, frames, 0);
	if (FAILED(result)) {
		if (result == AUDCLNT_E_DEVICE_INVALIDATED) {
			initDefaultDevice();
			submitEmptyBuffer(bufferFrames);
			audioClient->lpVtbl->Start(audioClient);
		}
	}
}

static DWORD WINAPI audioThread(LPVOID ignored) {
	submitBuffer(bufferFrames);
	audioClient->lpVtbl->Start(audioClient);
	while (WAIT_OBJECT_0 != WaitForSingleObject(audioProcessingDoneEvent, 0)) {
		WaitForSingleObject(bufferEndEvent, INFINITE);
		UINT32 padding = 0;
		HRESULT result = audioClient->lpVtbl->GetCurrentPadding(audioClient, &padding);
		if (FAILED(result)) {
			if (result == AUDCLNT_E_DEVICE_INVALIDATED) {
				initDefaultDevice();
				submitEmptyBuffer(bufferFrames);
				audioClient->lpVtbl->Start(audioClient);
			}
			continue;
		}
		UINT32 frames = bufferFrames - padding;
		submitBuffer(frames);
	}
	return 0;
}

void kinc_windows_co_initialize(void);

static bool initialized = false;

void kinc_a2_init() {
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

	audioProcessingDoneEvent = CreateEvent(0, FALSE, FALSE, 0);
	kinc_affirm(audioProcessingDoneEvent != 0);

	kinc_windows_co_initialize();
	kinc_microsoft_affirm(CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, &IID_IMMDeviceEnumerator, (void **)&deviceEnumerator));

	if (initDefaultDevice()) {
		CreateThread(0, 65536, audioThread, NULL, 0, 0);
	}
}

void kinc_a2_update() {}

#define SAFE_RELEASE(punk)                                                                                                                                     \
	if ((punk) != NULL) {                                                                                                                                      \
		(punk)->Release();                                                                                                                                     \
		(punk) = NULL;                                                                                                                                         \
	}

void kinc_a2_shutdown() {
	// Wait for last data in buffer to play before stopping.
	// Sleep((DWORD)(hnsActualDuration/REFTIMES_PER_MILLISEC/2));

	//	affirm(pAudioClient->Stop());  // Stop playing.

	//	CoTaskMemFree(pwfx);
	//	SAFE_RELEASE(pEnumerator)
	//	SAFE_RELEASE(pDevice)
	//	SAFE_RELEASE(pAudioClient)
	//	SAFE_RELEASE(pRenderClient)
}

#endif
