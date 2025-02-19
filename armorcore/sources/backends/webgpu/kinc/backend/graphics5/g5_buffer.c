#include <stdlib.h>
#include <string.h>
#include <kinc/g5_buffer.h>
#include <kinc/log.h>

extern WGPUDevice device;

kinc_g5_vertex_buffer_t *kinc_g5_internal_current_vertex_buffer = NULL;

void kinc_g5_vertex_buffer_init(kinc_g5_vertex_buffer_t *buffer, int count, kinc_g5_vertex_structure_t *structure, bool gpuMemory) {
	buffer->impl.count = count;
	buffer->impl.stride = 0;
	for (int i = 0; i < structure->size; ++i) {
		buffer->impl.stride += kinc_g5_vertex_data_size(structure->elements[i].data);
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

bool kinc_g5_transpose_mat = false;

void kinc_g5_constant_buffer_init(kinc_g5_constant_buffer_t *buffer, int size) {

}

void kinc_g5_constant_buffer_destroy(kinc_g5_constant_buffer_t *buffer) {}

void kinc_g5_constant_buffer_lock_all(kinc_g5_constant_buffer_t *buffer) {
	kinc_g5_constant_buffer_lock(buffer, 0, kinc_g5_constant_buffer_size(buffer));
}

void kinc_g5_constant_buffer_lock(kinc_g5_constant_buffer_t *buffer, int start, int count) {}

void kinc_g5_constant_buffer_unlock(kinc_g5_constant_buffer_t *buffer) {

}

int kinc_g5_constant_buffer_size(kinc_g5_constant_buffer_t *buffer) {
	return 0;
}

kinc_g5_index_buffer_t *kinc_g5_internal_current_index_buffer = NULL;

void kinc_g5_index_buffer_init(kinc_g5_index_buffer_t *buffer, int count, bool gpuMemory) {
	buffer->impl.count = count;
}

void kinc_g5_index_buffer_destroy(kinc_g5_index_buffer_t *buffer) {

}

static int kinc_g5_internal_index_buffer_stride(kinc_g5_index_buffer_t *buffer) {
	return 4;
}

void *kinc_g5_index_buffer_lock_all(kinc_g5_index_buffer_t *buffer) {
	kinc_g5_index_buffer_lock(buffer, 0, kinc_g5_index_buffer_count(buffer));
}

void *kinc_g5_index_buffer_lock(kinc_g5_index_buffer_t *buffer, int start, int count) {
	WGPUBufferDescriptor bDesc;
	memset(&bDesc, 0, sizeof(bDesc));
	bDesc.size = count * kinc_g5_internal_index_buffer_stride(buffer);
	bDesc.usage = WGPUBufferUsage_Index | WGPUBufferUsage_CopyDst;
	bDesc.mappedAtCreation = true;
	buffer->impl.buffer = wgpuDeviceCreateBuffer(device, &bDesc);
	return wgpuBufferGetMappedRange(buffer->impl.buffer, start * kinc_g5_internal_index_buffer_stride(buffer), bDesc.size);
}

void kinc_g5_index_buffer_unlock_all(kinc_g5_index_buffer_t *buffer) {
	wgpuBufferUnmap(buffer->impl.buffer);
}

void kinc_g5_index_buffer_unlock(kinc_g5_index_buffer_t *buffer, int count) {
	kinc_g5_index_buffer_unlock_all(buffer);
}

void kinc_g5_internal_index_buffer_set(kinc_g5_index_buffer_t *buffer) {

}

int kinc_g5_index_buffer_count(kinc_g5_index_buffer_t *buffer) {
	return buffer->impl.count;
}
