
#ifdef KINC_A2

#include <kinc/audio2/audio.h>
#include <alsa/asoundlib.h>
#include <errno.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

// apt-get install libasound2-dev

kinc_a2_buffer_t a2_buffer;

pthread_t threadid;
bool audioRunning = false;
snd_pcm_t *playback_handle;
short buf[4096 * 4];

static unsigned int samples_per_second = 44100;

uint32_t kinc_a2_samples_per_second(void) {
	return samples_per_second;
}

void copySample(void *buffer) {
	float left_value = *(float *)&a2_buffer.channels[0][a2_buffer.read_location];
	float right_value = *(float *)&a2_buffer.channels[1][a2_buffer.read_location];
	a2_buffer.read_location += 1;
	if (a2_buffer.read_location >= a2_buffer.data_size) {
		a2_buffer.read_location = 0;
	}
	((int16_t *)buffer)[0] = (int16_t)(left_value * 32767);
	((int16_t *)buffer)[1] = (int16_t)(right_value * 32767);
}

int playback_callback(snd_pcm_sframes_t nframes) {
	int err = 0;
	if (kinc_a2_internal_callback(&a2_buffer, nframes)) {
		int ni = 0;
		while (ni < nframes) {
			int i = 0;
			for (; ni < nframes && i < 4096 * 2; ++i, ++ni) {
				copySample(&buf[i * 2]);
			}
			int err2;
			if ((err2 = snd_pcm_writei(playback_handle, buf, i)) < 0) {
				fprintf(stderr, "write failed (%s)\n", snd_strerror(err2));
			}
			err += err2;
		}
	}
	return err;
}

bool tryToRecover(snd_pcm_t *handle, int errorCode) {
	switch (-errorCode) {
	case EINTR:
	case EPIPE:
	case ESPIPE:
	case ESTRPIPE:
	{
		int recovered = snd_pcm_recover(playback_handle, errorCode, 1);

		if (recovered != 0) {
			fprintf(stderr, "unable to recover from ALSA error code=%i\n", errorCode);
			return false;
		}
		else {
			fprintf(stdout, "recovered from ALSA error code=%i\n", errorCode);
			return true;
		}
	}
	default:
		fprintf(stderr, "unhandled ALSA error code=%i\n", errorCode);
		return false;
	}
}

void *doAudio(void *arg) {
	snd_pcm_hw_params_t *hw_params;
	snd_pcm_sw_params_t *sw_params;
	snd_pcm_sframes_t frames_to_deliver;
	int err;

	if ((err = snd_pcm_open(&playback_handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		fprintf(stderr, "cannot open audio device default (%s)\n", snd_strerror(err));
		return NULL;
	}

	if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
		fprintf(stderr, "cannot allocate hardware parameter structure (%s)\n", snd_strerror(err));
		return NULL;
	}

	if ((err = snd_pcm_hw_params_any(playback_handle, hw_params)) < 0) {
		fprintf(stderr, "cannot initialize hardware parameter structure (%s)\n", snd_strerror(err));
		return NULL;
	}

	if ((err = snd_pcm_hw_params_set_access(playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		fprintf(stderr, "cannot set access type (%s)\n", snd_strerror(err));
		return NULL;
	}

	if ((err = snd_pcm_hw_params_set_format(playback_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
		fprintf(stderr, "cannot set sample format (%s)\n", snd_strerror(err));
		return NULL;
	}

	int dir = 0;
	if ((err = snd_pcm_hw_params_set_rate_near(playback_handle, hw_params, &samples_per_second, &dir)) < 0) {
		fprintf(stderr, "cannot set sample rate (%s)\n", snd_strerror(err));
		return NULL;
	}

	if ((err = snd_pcm_hw_params_set_channels(playback_handle, hw_params, 2)) < 0) {
		fprintf(stderr, "cannot set channel count (%s)\n", snd_strerror(err));
		return NULL;
	}

	snd_pcm_uframes_t bufferSize = samples_per_second / 8;
	if (((err = snd_pcm_hw_params_set_buffer_size(playback_handle, hw_params, bufferSize)) < 0 &&
	     (snd_pcm_hw_params_set_buffer_size_near(playback_handle, hw_params, &bufferSize)) < 0)) {
		fprintf(stderr, "cannot set buffer size (%s)\n", snd_strerror(err));
		return NULL;
	}

	if ((err = snd_pcm_hw_params(playback_handle, hw_params)) < 0) {
		fprintf(stderr, "cannot set parameters (%s)\n", snd_strerror(err));
		return NULL;
	}

	snd_pcm_hw_params_free(hw_params);

	/* tell ALSA to wake us up whenever 4096 or more frames
	    of playback data can be delivered. Also, tell
	    ALSA that we'll start the device ourselves.
	*/

	if ((err = snd_pcm_sw_params_malloc(&sw_params)) < 0) {
		fprintf(stderr, "cannot allocate software parameters structure (%s)\n", snd_strerror(err));
		return NULL;
	}
	if ((err = snd_pcm_sw_params_current(playback_handle, sw_params)) < 0) {
		fprintf(stderr, "cannot initialize software parameters structure (%s)\n", snd_strerror(err));
		return NULL;
	}
	if ((err = snd_pcm_sw_params_set_avail_min(playback_handle, sw_params, 4096)) < 0) {
		fprintf(stderr, "cannot set minimum available count (%s)\n", snd_strerror(err));
		return NULL;
	}
	if ((err = snd_pcm_sw_params_set_start_threshold(playback_handle, sw_params, 0U)) < 0) {
		fprintf(stderr, "cannot set start mode (%s)\n", snd_strerror(err));
		return NULL;
	}
	if ((err = snd_pcm_sw_params(playback_handle, sw_params)) < 0) {
		fprintf(stderr, "cannot set software parameters (%s)\n", snd_strerror(err));
		return NULL;
	}

	/* the interface will interrupt the kernel every 4096 frames, and ALSA
	    will wake up this program very soon after that.
	*/

	if ((err = snd_pcm_prepare(playback_handle)) < 0) {
		fprintf(stderr, "cannot prepare audio interface for use (%s)\n", snd_strerror(err));
		return NULL;
	}

	while (audioRunning) {

		/* wait till the interface is ready for data, or 1 second
		    has elapsed.
		*/

		if ((err = snd_pcm_wait(playback_handle, 1000)) < 0) {
			fprintf(stderr, "poll failed (%s)\n", strerror(errno));
			break;
		}

		/* find out how much space is available for playback data */

		if ((frames_to_deliver = snd_pcm_avail_update(playback_handle)) < 0) {
			if (!tryToRecover(playback_handle, frames_to_deliver)) {
				// break;
			}
		}
		else {
			// frames_to_deliver = frames_to_deliver > 4096 ? 4096 : frames_to_deliver;

			/* deliver the data */

			int delivered = playback_callback(frames_to_deliver);

			if (delivered != frames_to_deliver) {
				fprintf(stderr, "playback callback failed (delivered %i / %ld frames)\n", delivered, frames_to_deliver);
				// break;
			}
		}
	}

	snd_pcm_close(playback_handle);
	return NULL;
}

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

	audioRunning = true;
	pthread_create(&threadid, NULL, &doAudio, NULL);
}

void kinc_a2_update() {}

void kinc_a2_shutdown() {
	audioRunning = false;
}

#endif
