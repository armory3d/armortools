#include <kinc/graphics5/graphics.h>
#include <kinc/graphics5/indexbuffer.h>
#import <Metal/Metal.h>

id getMetalDevice(void);

void kinc_g5_index_buffer_init(kinc_g5_index_buffer_t *buffer, int indexCount, bool gpuMemory) {
	buffer->impl.count = indexCount;
	buffer->impl.gpu_memory = gpuMemory;
	buffer->impl.last_start = 0;
	buffer->impl.last_count = indexCount;

	id<MTLDevice> device = getMetalDevice();
	MTLResourceOptions options = MTLResourceCPUCacheModeWriteCombined;
	options |= MTLResourceStorageModeShared;

	buffer->impl.metal_buffer = (__bridge_retained void *)[device
	    newBufferWithLength:sizeof(uint32_t) * indexCount
	                options:options];
}

void kinc_g5_index_buffer_destroy(kinc_g5_index_buffer_t *buffer) {
	id<MTLBuffer> buf = (__bridge_transfer id<MTLBuffer>)buffer->impl.metal_buffer;
	buf = nil;
	buffer->impl.metal_buffer = NULL;
}

static int kinc_g5_internal_index_buffer_stride(kinc_g5_index_buffer_t *buffer) {
	return 4;
}

void *kinc_g5_index_buffer_lock_all(kinc_g5_index_buffer_t *buffer) {
	return kinc_g5_index_buffer_lock(buffer, 0, kinc_g5_index_buffer_count(buffer));
}

void *kinc_g5_index_buffer_lock(kinc_g5_index_buffer_t *buffer, int start, int count) {
	buffer->impl.last_start = start;
	buffer->impl.last_count = count;

	id<MTLBuffer> metal_buffer = (__bridge id<MTLBuffer>)buffer->impl.metal_buffer;
	uint8_t *data = (uint8_t *)[metal_buffer contents];
	return &data[start * kinc_g5_internal_index_buffer_stride(buffer)];
}

void kinc_g5_index_buffer_unlock_all(kinc_g5_index_buffer_t *buffer) {
	kinc_g5_index_buffer_unlock(buffer, buffer->impl.last_count);
}

void kinc_g5_index_buffer_unlock(kinc_g5_index_buffer_t *buffer, int count) {
}

int kinc_g5_index_buffer_count(kinc_g5_index_buffer_t *buffer) {
	return buffer->impl.count;
}
