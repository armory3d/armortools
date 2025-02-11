#include <stdlib.h>
#include <string.h>
#include <kinc/graphics5/vertexbuffer.h>
#include <kinc/log.h>

extern WGPUDevice device;

kinc_g5_vertex_buffer_t *kinc_g5_internal_current_vertex_buffer = NULL;

void kinc_g5_vertex_buffer_init(kinc_g5_vertex_buffer_t *buffer, int count, kinc_g5_vertex_structure_t *structure, bool gpuMemory) {
	buffer->impl.count = count;
	buffer->impl.stride = 0;
	for (int i = 0; i < structure->size; ++i) {
		buffer->impl.stride += kinc_g4_vertex_data_size(structure->elements[i].data);
	}
}

void kinc_g5_vertex_buffer_destroy(kinc_g5_vertex_buffer_t *buffer) {

}

float *kinc_g5_vertex_buffer_lock_all(kinc_g5_vertex_buffer_t *buffer) {
	WGPUBufferDescriptor bDesc;
	memset(&bDesc, 0, sizeof(bDesc));
	bDesc.size = buffer->impl.count * buffer->impl.stride * sizeof(float);
	bDesc.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
	bDesc.mappedAtCreation = true;
	buffer->impl.buffer = wgpuDeviceCreateBuffer(device, &bDesc);
	return wgpuBufferGetMappedRange(buffer->impl.buffer, 0, bDesc.size);
}

float *kinc_g5_vertex_buffer_lock(kinc_g5_vertex_buffer_t *buffer, int start, int count) {
	return NULL;
}

void kinc_g5_vertex_buffer_unlock_all(kinc_g5_vertex_buffer_t *buffer) {
	wgpuBufferUnmap(buffer->impl.buffer);
}

void kinc_g5_vertex_buffer_unlock(kinc_g5_vertex_buffer_t* buffer, int count) {

}

int kinc_g5_vertex_buffer_count(kinc_g5_vertex_buffer_t *buffer) {
	return buffer->impl.count;
}

int kinc_g5_vertex_buffer_stride(kinc_g5_vertex_buffer_t *buffer) {
	return buffer->impl.stride;
}
