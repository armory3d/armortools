#ifndef OPENGL_1_X

#include "graphics.h"

static int samples = 1;

int kinc_g5_antialiasing_samples(void) {
	return samples;
}

void kinc_g5_set_antialiasing_samples(int samples_) {
	samples = samples_;
}

bool kinc_g5_fullscreen = false;

// void Graphics5::setVertexBuffer(VertexBuffer& vertexBuffer) {
//	VertexBuffer* vertexBuffers[1] = {&vertexBuffer};
//	setVertexBuffers(vertexBuffers, 1);
//}

#endif
