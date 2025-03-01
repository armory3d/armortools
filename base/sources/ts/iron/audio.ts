
///if arm_audio

function audio_play(sound: sound_t, loop: bool = false, pitch: f32 = 1.0, unique: bool = false): audio_channel_t {
	return kinc_a1_play_sound(sound.sound_, loop, pitch, unique);
}

function audio_stop(sound: sound_t) {
	kinc_a1_stop_sound(sound.sound_);
}

///end
