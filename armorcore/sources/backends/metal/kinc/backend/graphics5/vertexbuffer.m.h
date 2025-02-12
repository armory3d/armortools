#include <kinc/graphics5/shader.h>
#include <kinc/graphics5/vertexbuffer.h>
#include <kinc/graphics5/graphics.h>
#include <kinc/graphics5/indexbuffer.h>
#include <kinc/graphics5/vertexbuffer.h>
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
		buffer->impl.myStride += kinc_g4_vertex_data_size(element.data);
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
