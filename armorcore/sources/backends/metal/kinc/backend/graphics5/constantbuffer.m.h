#include <kinc/graphics5/constantbuffer.h>
#import <Metal/Metal.h>

id getMetalDevice(void);

bool kinc_g5_transposeMat = true;

void kinc_g5_constant_buffer_init(kinc_g5_constant_buffer_t *buffer, int size) {
	buffer->impl.mySize = size;
	buffer->data = NULL;
	buffer->impl._buffer = (__bridge_retained void *)[getMetalDevice() newBufferWithLength:size options:MTLResourceOptionCPUCacheModeDefault];
}

void kinc_g5_constant_buffer_destroy(kinc_g5_constant_buffer_t *buffer) {
	id<MTLBuffer> buf = (__bridge_transfer id<MTLBuffer>)buffer->impl._buffer;
	buf = nil;
	buffer->impl._buffer = NULL;
}

void kinc_g5_constant_buffer_lock_all(kinc_g5_constant_buffer_t *buffer) {
	kinc_g5_constant_buffer_lock(buffer, 0, kinc_g5_constant_buffer_size(buffer));
}

void kinc_g5_constant_buffer_lock(kinc_g5_constant_buffer_t *buffer, int start, int count) {
	buffer->impl.lastStart = start;
	buffer->impl.lastCount = count;
	id<MTLBuffer> buf = (__bridge id<MTLBuffer>)buffer->impl._buffer;
	uint8_t *data = (uint8_t *)[buf contents];
	buffer->data = &data[start];
}

void kinc_g5_constant_buffer_unlock(kinc_g5_constant_buffer_t *buffer) {
	buffer->data = NULL;
}

int kinc_g5_constant_buffer_size(kinc_g5_constant_buffer_t *buffer) {
	return buffer->impl.mySize;
}
