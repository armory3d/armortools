// #include <iron_video.h>

// #import <AVFoundation/AVFoundation.h>
// #include <iron_audio.h>
// #include <iron_gpu.h>
// #include <iron_file.h>
// #include <iron_system.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

// extern const char *iphonegetresourcepath(void);
// extern const char *macgetresourcepath(void);

// void iron_internal_video_sound_stream_init(iron_internal_video_sound_stream_t *stream, int channel_count, int frequency) {
// 	stream->bufferSize = 1024 * 100;
// 	stream->bufferReadPosition = 0;
// 	stream->bufferWritePosition = 0;
// 	stream->read = 0;
// 	stream->written = 0;
// 	stream->buffer = (float *)malloc(stream->bufferSize * sizeof(float));
// }

// void iron_internal_video_sound_stream_destroy(iron_internal_video_sound_stream_t *stream) {
// 	free(stream->buffer);
// }

// void iron_internal_video_sound_stream_insert_data(iron_internal_video_sound_stream_t *stream, float *data, int sample_count) {
// 	for (int i = 0; i < sample_count; ++i) {
// 		float value = data[i]; // / 32767.0;
// 		stream->buffer[stream->bufferWritePosition++] = value;
// 		++stream->written;
// 		if (stream->bufferWritePosition >= stream->bufferSize) {
// 			stream->bufferWritePosition = 0;
// 		}
// 	}
// }

// static float samples[2] = {0};

// float *iron_internal_video_sound_stream_next_frame(iron_internal_video_sound_stream_t *stream) {
// 	++stream->read;
// 	if (stream->written <= stream->read) {
// 		iron_log("Out of audio\n");
// 		return 0;
// 	}

// 	if (stream->bufferReadPosition >= stream->bufferSize) {
// 		stream->bufferReadPosition = 0;
// 		iron_log("buffer read back - %i\n", (int)(stream->written - stream->read));
// 	}
// 	samples[0] = stream->buffer[stream->bufferReadPosition++];

// 	if (stream->bufferReadPosition >= stream->bufferSize) {
// 		stream->bufferReadPosition = 0;
// 		iron_log("buffer read back - %i\n", (int)(stream->written - stream->read));
// 	}
// 	samples[1] = stream->buffer[stream->bufferReadPosition++];

// 	return samples;
// }

// bool iron_internal_video_sound_stream_ended(iron_internal_video_sound_stream_t *stream) {
// 	return false;
// }

// static void load(iron_video_t *video, double startTime) {
// 	video->impl.videoStart = startTime;
// 	AVURLAsset *asset = [[AVURLAsset alloc] initWithURL:video->impl.url options:nil];
// 	video->impl.videoAsset = asset;

// 	video->impl.duration = [asset duration].value / [asset duration].timescale;

// 	AVAssetTrack *videoTrack = [[asset tracksWithMediaType:AVMediaTypeVideo] objectAtIndex:0];
// 	NSDictionary *videoOutputSettings =
// 	    [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithInt:kCVPixelFormatType_32BGRA], kCVPixelBufferPixelFormatTypeKey, nil];
// 	AVAssetReaderTrackOutput *videoOutput = [AVAssetReaderTrackOutput assetReaderTrackOutputWithTrack:videoTrack outputSettings:videoOutputSettings];
// 	[videoOutput setSupportsRandomAccess:YES];

// 	bool hasAudio = [[asset tracksWithMediaType:AVMediaTypeAudio] count] > 0;
// 	AVAssetReaderAudioMixOutput *audioOutput = NULL;
// 	if (hasAudio) {
// 		AVAssetTrack *audioTrack = [[asset tracksWithMediaType:AVMediaTypeAudio] objectAtIndex:0];
// 		NSDictionary *audioOutputSettings = [NSDictionary
// 		    dictionaryWithObjectsAndKeys:[NSNumber numberWithInt:kAudioFormatLinearPCM], AVFormatIDKey, [NSNumber numberWithFloat:44100.0], AVSampleRateKey,
// 		                                 [NSNumber numberWithInt:32], AVLinearPCMBitDepthKey, [NSNumber numberWithBool:NO], AVLinearPCMIsNonInterleaved,
// 		                                 [NSNumber numberWithBool:YES], AVLinearPCMIsFloatKey, [NSNumber numberWithBool:NO], AVLinearPCMIsBigEndianKey, nil];
// 		audioOutput = [AVAssetReaderAudioMixOutput assetReaderAudioMixOutputWithAudioTracks:@[ audioTrack ] audioSettings:audioOutputSettings];
// 		[audioOutput setSupportsRandomAccess:YES];
// 	}

// 	AVAssetReader *reader = [AVAssetReader assetReaderWithAsset:asset error:nil];

// 	if (startTime > 0) {
// 		CMTimeRange timeRange = CMTimeRangeMake(CMTimeMake(startTime * 1000, 1000), kCMTimePositiveInfinity);
// 		reader.timeRange = timeRange;
// 	}

// 	[reader addOutput:videoOutput];
// 	if (hasAudio) {
// 		[reader addOutput:audioOutput];
// 	}

// 	video->impl.assetReader = reader;
// 	video->impl.videoTrackOutput = videoOutput;
// 	if (hasAudio) {
// 		video->impl.audioTrackOutput = audioOutput;
// 	}
// 	else {
// 		video->impl.audioTrackOutput = NULL;
// 	}

// 	if (video->impl.myWidth < 0)
// 		video->impl.myWidth = [videoTrack naturalSize].width;
// 	if (video->impl.myHeight < 0)
// 		video->impl.myHeight = [videoTrack naturalSize].height;
// 	int framerate = [videoTrack nominalFrameRate];
// 	iron_log("Framerate: %i\n", framerate);
// 	video->impl.next = video->impl.videoStart;
// 	video->impl.audioTime = video->impl.videoStart * 44100;
// }

// void iron_video_init(iron_video_t *video, const char *filename) {
// 	video->impl.playing = false;
// 	video->impl.sound = NULL;
// 	video->impl.image_initialized = false;
// 	char name[2048];
// #ifdef IRON_IOS
// 	strcpy(name, iphonegetresourcepath());
// #else
// 	strcpy(name, macgetresourcepath());
// #endif
// 	strcat(name, "/");
// 	strcat(name, IRON_OUTDIR);
// 	strcat(name, "/");
// 	strcat(name, filename);
// 	video->impl.url = [NSURL fileURLWithPath:[NSString stringWithUTF8String:name]];
// 	video->impl.myWidth = -1;
// 	video->impl.myHeight = -1;
// 	video->impl.finished = false;
// 	video->impl.duration = 0;
// 	load(video, 0);
// }

// void iron_video_destroy(iron_video_t *video) {
// 	iron_video_stop(video);
// }

// #ifdef IRON_IOS
// void iosPlayVideoSoundStream(iron_internal_video_sound_stream_t *video);
// void iosStopVideoSoundStream(void);
// #else
// void macPlayVideoSoundStream(iron_internal_video_sound_stream_t *video);
// void macStopVideoSoundStream(void);
// #endif

// void iron_video_play(iron_video_t *video, bool loop) {
// 	AVAssetReader *reader = video->impl.assetReader;
// 	[reader startReading];

// 	iron_internal_video_sound_stream_t *stream = (iron_internal_video_sound_stream_t *)malloc(sizeof(iron_internal_video_sound_stream_t));
// 	iron_internal_video_sound_stream_init(stream, 2, 44100);
// 	video->impl.sound = stream;
// #ifdef IRON_IOS
// 	iosPlayVideoSoundStream((iron_internal_video_sound_stream_t *)video->impl.sound);
// #else
// 	macPlayVideoSoundStream((iron_internal_video_sound_stream_t *)video->impl.sound);
// #endif

// 	video->impl.playing = true;
// 	video->impl.start = iron_time() - video->impl.videoStart;
// 	video->impl.loop = loop;
// }

// void iron_video_pause(iron_video_t *video) {
// 	video->impl.playing = false;
// 	if (video->impl.sound != NULL) {
// // Mixer::stop(sound);
// #ifdef IRON_IOS
// 		iosStopVideoSoundStream();
// #else
// 		macStopVideoSoundStream();
// #endif
// 		iron_internal_video_sound_stream_destroy((iron_internal_video_sound_stream_t *)video->impl.sound);
// 		free(video->impl.sound);
// 		video->impl.sound = NULL;
// 	}
// }

// void iron_video_stop(iron_video_t *video) {
// 	iron_video_pause(video);
// 	video->impl.finished = true;
// }

// static void updateImage(iron_video_t *video) {
// 	if (!video->impl.playing)
// 		return;

// 	{
// 		AVAssetReaderTrackOutput *videoOutput = video->impl.videoTrackOutput;
// 		CMSampleBufferRef buffer = [videoOutput copyNextSampleBuffer];
// 		if (!buffer) {
// 			if (video->impl.loop) {
// 				CMTimeRange timeRange = CMTimeRangeMake(CMTimeMake(0, 1000), kCMTimePositiveInfinity);
// 				[videoOutput resetForReadingTimeRanges:[NSArray arrayWithObject:[NSValue valueWithCMTimeRange:timeRange]]];

// 				AVAssetReaderAudioMixOutput *audioOutput = video->impl.audioTrackOutput;
// 				CMSampleBufferRef audio_buffer = [audioOutput copyNextSampleBuffer];
// 				while (audio_buffer) {
// 					audio_buffer = [audioOutput copyNextSampleBuffer];
// 				}
// 				[audioOutput resetForReadingTimeRanges:[NSArray arrayWithObject:[NSValue valueWithCMTimeRange:timeRange]]];

// 				buffer = [videoOutput copyNextSampleBuffer];

// 				video->impl.start = iron_time() - video->impl.videoStart;
// 			}
// 			else {
// 				iron_video_stop(video);
// 				return;
// 			}
// 		}
// 		video->impl.next = CMTimeGetSeconds(CMSampleBufferGetOutputPresentationTimeStamp(buffer));

// 		CVImageBufferRef pixelBuffer = CMSampleBufferGetImageBuffer(buffer);

// 		if (!video->impl.image_initialized) {
// 			CGSize size = CVImageBufferGetDisplaySize(pixelBuffer);
// 			video->impl.myWidth = size.width;
// 			video->impl.myHeight = size.height;
// 			iron_g5_texture_init(&video->impl.image, iron_video_width(video), iron_video_height(video), IRON_IMAGE_FORMAT_BGRA32);
// 			video->impl.image_initialized = true;
// 		}

// 		if (pixelBuffer != NULL) {
// 			CVPixelBufferLockBaseAddress(pixelBuffer, 0);
// 			// iron_g4_texture_upload(&video->impl.image, (uint8_t *)CVPixelBufferGetBaseAddress(pixelBuffer), (int)(CVPixelBufferGetBytesPerRow(pixelBuffer)));
// 			CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
// 		}
// 		CFRelease(buffer);
// 	}

// 	if (video->impl.audioTrackOutput != NULL) {
// 		AVAssetReaderAudioMixOutput *audioOutput = video->impl.audioTrackOutput;
// 		while (video->impl.audioTime / 44100.0 < video->impl.next + 0.1) {
// 			CMSampleBufferRef buffer = [audioOutput copyNextSampleBuffer];
// 			if (!buffer)
// 				return;
// 			CMItemCount numSamplesInBuffer = CMSampleBufferGetNumSamples(buffer);
// 			AudioBufferList audioBufferList;
// 			CMBlockBufferRef blockBufferOut = nil;
// 			CMSampleBufferGetAudioBufferListWithRetainedBlockBuffer(buffer, NULL, &audioBufferList, sizeof(audioBufferList), NULL, NULL,
// 			                                                        kCMSampleBufferFlag_AudioBufferList_Assure16ByteAlignment, &blockBufferOut);
// 			for (int bufferCount = 0; bufferCount < audioBufferList.mNumberBuffers; ++bufferCount) {
// 				float *samples = (float *)audioBufferList.mBuffers[bufferCount].mData;
// 				iron_internal_video_sound_stream_t *sound = (iron_internal_video_sound_stream_t *)video->impl.sound;
// 				if (video->impl.audioTime / 44100.0 > video->impl.next - 0.1) {
// 					iron_internal_video_sound_stream_insert_data(sound, samples, (int)numSamplesInBuffer * 2);
// 				}
// 				else {
// 					// Send some data anyway because the buffers are huge
// 					iron_internal_video_sound_stream_insert_data(sound, samples, (int)numSamplesInBuffer);
// 				}
// 				video->impl.audioTime += numSamplesInBuffer;
// 			}
// 			CFRelease(blockBufferOut);
// 			CFRelease(buffer);
// 		}
// 	}
// }

// void iron_video_update(iron_video_t *video, double time) {
// 	if (video->impl.playing && time >= video->impl.start + video->impl.next) {
// 		updateImage(video);
// 	}
// }

// int iron_video_width(iron_video_t *video) {
// 	return video->impl.myWidth;
// }

// int iron_video_height(iron_video_t *video) {
// 	return video->impl.myHeight;
// }

// iron_g5_texture_t *iron_video_current_image(iron_video_t *video) {
// 	iron_video_update(video, iron_time());
// 	return &video->impl.image;
// }

// double iron_video_duration(iron_video_t *video) {
// 	return video->impl.duration;
// }

// bool iron_video_finished(iron_video_t *video) {
// 	return video->impl.finished;
// }

// bool iron_video_paused(iron_video_t *video) {
// 	return !video->impl.playing;
// }

// double iron_video_position(iron_video_t *video) {
// 	return video->impl.next - video->impl.start;
// }
