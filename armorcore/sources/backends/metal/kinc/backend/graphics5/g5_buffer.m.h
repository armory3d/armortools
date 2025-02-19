#include <kinc/g5_pipeline.h>
#include <kinc/g5.h>
#include <kinc/g5_buffer.h>
#import <Metal/Metal.h>

id getMetalDevice(void);
id getMetalEncoder(void);

kinc_g5_vertex_buffer_t *currentVertexBuffer = NULL;

static void vertex_buffer_unset(kinc_g5_vertex_buffer_t *buffer) {
	if (currentVertexBuffer == buffer)
		currentVertexBuffer = NULL;
}

void kinc_g5_vertex_buffer_init(kinc_g5_vertex_buffer_t *buffer, int count, kinc_g5_vertex_structure_t *structure, bool gpuMemory) {
	memset(&buffer->impl, 0, sizeof(buffer->impl));
	buffer->impl.myCount = count;
	buffer->impl.gpuMemory = gpuMemory;
	for (int i = 0; i < structure->size; ++i) {
		kinc_g5_vertex_element_t element = structure->elements[i];
		buffer->impl.myStride += kinc_g5_vertex_data_size(element.data);
	}

	id<MTLDevice> device = getMetalDevice();
	MTLResourceOptions options = MTLResourceCPUCacheModeWriteCombined;
	options |= MTLResourceStorageModeShared;

	id<MTLBuffer> buf = [device newBufferWithLength:count * buffer->impl.myStride options:options];
	buffer->impl.mtlBuffer = (__bridge_retained void *)buf;

	buffer->impl.lastStart = 0;
	buffer->impl.lastCount = 0;
}

void kinc_g5_vertex_buffer_destroy(kinc_g5_vertex_buffer_t *buf) {
	id<MTLBuffer> buffer = (__bridge_transfer id<MTLBuffer>)buf->impl.mtlBuffer;
	buffer = nil;
	buf->impl.mtlBuffer = NULL;
	vertex_buffer_unset(buf);
}

float *kinc_g5_vertex_buffer_lock_all(kinc_g5_vertex_buffer_t *buf) {
	buf->impl.lastStart = 0;
	buf->impl.lastCount = kinc_g5_vertex_buffer_count(buf);
	id<MTLBuffer> buffer = (__bridge id<MTLBuffer>)buf->impl.mtlBuffer;
	float *floats = (float *)[buffer contents];
	return floats;
}

float *kinc_g5_vertex_buffer_lock(kinc_g5_vertex_buffer_t *buf, int start, int count) {
	buf->impl.lastStart = start;
	buf->impl.lastCount = count;
	id<MTLBuffer> buffer = (__bridge id<MTLBuffer>)buf->impl.mtlBuffer;
	float *floats = (float *)[buffer contents];
	return &floats[start * buf->impl.myStride / sizeof(float)];
}

void kinc_g5_vertex_buffer_unlock_all(kinc_g5_vertex_buffer_t *buf) {
}

void kinc_g5_vertex_buffer_unlock(kinc_g5_vertex_buffer_t *buf, int count) {
}

int kinc_g5_internal_vertex_buffer_set(kinc_g5_vertex_buffer_t *buf) {
	currentVertexBuffer = buf;

	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	id<MTLBuffer> buffer = (__bridge id<MTLBuffer>)buf->impl.mtlBuffer;
	[encoder setVertexBuffer:buffer offset:0 atIndex:0];

	return 0;
}

int kinc_g5_vertex_buffer_count(kinc_g5_vertex_buffer_t *buffer) {
	return buffer->impl.myCount;
}

int kinc_g5_vertex_buffer_stride(kinc_g5_vertex_buffer_t *buffer) {
	return buffer->impl.myStride;
}

bool kinc_g5_transpose_mat = true;

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
