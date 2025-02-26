#include "g5_buffer.h"
#include <iron_gpu.h>
#include <kinc/backend/system_microsoft.h>

kinc_g5_vertex_buffer_t *_current_vertex_buffer = NULL;

void kinc_g5_vertex_buffer_init(kinc_g5_vertex_buffer_t *buffer, int count, kinc_g5_vertex_structure_t *structure, bool gpuMemory) {
	buffer->impl.myCount = count;

	buffer->impl.myStride = 0;
	for (int i = 0; i < structure->size; ++i) {
		buffer->impl.myStride += kinc_g5_vertex_data_size(structure->elements[i].data);
	}

	int uploadBufferSize = buffer->impl.myStride * buffer->impl.myCount;

	D3D12_HEAP_PROPERTIES heapProperties;
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 1;
	heapProperties.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC resourceDesc;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = uploadBufferSize;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	device->lpVtbl->CreateCommittedResource(device, &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
	                                &IID_ID3D12Resource, &buffer->impl.uploadBuffer);

	buffer->impl.view.BufferLocation = buffer->impl.uploadBuffer->lpVtbl->GetGPUVirtualAddress(buffer->impl.uploadBuffer);
	buffer->impl.view.SizeInBytes = uploadBufferSize;
	buffer->impl.view.StrideInBytes = buffer->impl.myStride;

	buffer->impl.lastStart = 0;
	buffer->impl.lastCount = kinc_g5_vertex_buffer_count(buffer);
}

void kinc_g5_vertex_buffer_destroy(kinc_g5_vertex_buffer_t *buffer) {
	// buffer->impl.vertexBuffer->lpVtbl->Release();
	buffer->impl.uploadBuffer->lpVtbl->Release(buffer->impl.uploadBuffer);
}

float *kinc_g5_vertex_buffer_lock_all(kinc_g5_vertex_buffer_t *buffer) {
	return kinc_g5_vertex_buffer_lock(buffer, 0, kinc_g5_vertex_buffer_count(buffer));
}

float *kinc_g5_vertex_buffer_lock(kinc_g5_vertex_buffer_t *buffer, int start, int count) {
	buffer->impl.lastStart = start;
	buffer->impl.lastCount = count;

	D3D12_RANGE range;
	range.Begin = start * buffer->impl.myStride;
	range.End = (start + count) * buffer->impl.myStride;

	void *p;
	buffer->impl.uploadBuffer->lpVtbl->Map(buffer->impl.uploadBuffer, 0, &range, &p);
	byte *bytes = (byte *)p;
	bytes += start * buffer->impl.myStride;
	return (float *)bytes;
}

void kinc_g5_vertex_buffer_unlock_all(kinc_g5_vertex_buffer_t *buffer) {
	D3D12_RANGE range;
	range.Begin = buffer->impl.lastStart * buffer->impl.myStride;
	range.End = (buffer->impl.lastStart + buffer->impl.lastCount) * buffer->impl.myStride;
	buffer->impl.uploadBuffer->lpVtbl->Unmap(buffer->impl.uploadBuffer, 0, &range);
}

void kinc_g5_vertex_buffer_unlock(kinc_g5_vertex_buffer_t *buffer, int count) {
	D3D12_RANGE range;
	range.Begin = buffer->impl.lastStart * buffer->impl.myStride;
	range.End = (buffer->impl.lastStart + count) * buffer->impl.myStride;
	buffer->impl.uploadBuffer->lpVtbl->Unmap(buffer->impl.uploadBuffer, 0, &range);
}

int kinc_g5_internal_vertex_buffer_set(kinc_g5_vertex_buffer_t *buffer) {
	_current_vertex_buffer = buffer;
	return 0;
}

int kinc_g5_vertex_buffer_count(kinc_g5_vertex_buffer_t *buffer) {
	return buffer->impl.myCount;
}

int kinc_g5_vertex_buffer_stride(kinc_g5_vertex_buffer_t *buffer) {
	return buffer->impl.myStride;
}

bool kinc_g5_transpose_mat = false;

void kinc_g5_constant_buffer_init(kinc_g5_constant_buffer_t *buffer, int size) {
	buffer->impl.mySize = size;
	buffer->data = NULL;

	D3D12_HEAP_PROPERTIES heapProperties;
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 1;
	heapProperties.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC resourceDesc;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = size;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	kinc_microsoft_affirm(device->lpVtbl->CreateCommittedResource(device , &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
	                                                              D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
	                                                      &IID_ID3D12Resource, &buffer->impl.constant_buffer));

	void *p;
	buffer->impl.constant_buffer->lpVtbl->Map(buffer->impl.constant_buffer, 0, NULL, &p);
	ZeroMemory(p, size);
	buffer->impl.constant_buffer->lpVtbl->Unmap(buffer->impl.constant_buffer, 0, NULL);
}

void kinc_g5_constant_buffer_destroy(kinc_g5_constant_buffer_t *buffer) {
	buffer->impl.constant_buffer->lpVtbl->Release(buffer->impl.constant_buffer);
}

void kinc_g5_constant_buffer_lock_all(kinc_g5_constant_buffer_t *buffer) {
	kinc_g5_constant_buffer_lock(buffer, 0, kinc_g5_constant_buffer_size(buffer));
}

void kinc_g5_constant_buffer_lock(kinc_g5_constant_buffer_t *buffer, int start, int count) {
	buffer->impl.lastStart = start;
	buffer->impl.lastCount = count;
	D3D12_RANGE range;
	range.Begin = start;
	range.End = range.Begin + count;
	uint8_t *p;
	buffer->impl.constant_buffer->lpVtbl->Map(buffer->impl.constant_buffer, 0, &range, (void **)&p);
	buffer->data = &p[start];
}

void kinc_g5_constant_buffer_unlock(kinc_g5_constant_buffer_t *buffer) {
	D3D12_RANGE range;
	range.Begin = buffer->impl.lastStart;
	range.End = range.Begin + buffer->impl.lastCount;
	buffer->impl.constant_buffer->lpVtbl->Unmap(buffer->impl.constant_buffer, 0, &range);
	buffer->data = NULL;
}

int kinc_g5_constant_buffer_size(kinc_g5_constant_buffer_t *buffer) {
	return buffer->impl.mySize;
}

void kinc_g5_index_buffer_init(kinc_g5_index_buffer_t *buffer, int count, bool gpuMemory) {
	buffer->impl.count = count;
	buffer->impl.gpu_memory = gpuMemory;

	int uploadBufferSize = sizeof(uint32_t) * count;

	D3D12_HEAP_PROPERTIES heapProperties;
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 1;
	heapProperties.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC resourceDesc;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = uploadBufferSize;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	kinc_microsoft_affirm(device->lpVtbl->CreateCommittedResource(device , &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
	                                                              D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
	                                                      &IID_ID3D12Resource, &buffer->impl.upload_buffer));

	if (gpuMemory) {
		D3D12_HEAP_PROPERTIES heapProperties;
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask = 1;
		heapProperties.VisibleNodeMask = 1;

		kinc_microsoft_affirm(device->lpVtbl->CreateCommittedResource(device , &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
		                                                              D3D12_RESOURCE_STATE_COPY_DEST, NULL,
		                                                      &IID_ID3D12Resource, &buffer->impl.index_buffer));

		buffer->impl.index_buffer_view.BufferLocation = buffer->impl.index_buffer->lpVtbl->GetGPUVirtualAddress(buffer->impl.index_buffer);
	}
	else {
		buffer->impl.index_buffer_view.BufferLocation = buffer->impl.upload_buffer->lpVtbl->GetGPUVirtualAddress(buffer->impl.upload_buffer);
	}
	buffer->impl.index_buffer_view.SizeInBytes = uploadBufferSize;
	buffer->impl.index_buffer_view.Format = DXGI_FORMAT_R32_UINT;

	buffer->impl.last_start = 0;
	buffer->impl.last_count = kinc_g5_index_buffer_count(buffer);
}

void kinc_g5_index_buffer_destroy(kinc_g5_index_buffer_t *buffer) {
	if (buffer->impl.index_buffer != NULL) {
		buffer->impl.index_buffer->lpVtbl->Release(buffer->impl.index_buffer);
		buffer->impl.index_buffer = NULL;
	}

	buffer->impl.upload_buffer->lpVtbl->Release(buffer->impl.upload_buffer);
	buffer->impl.upload_buffer = NULL;
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

	D3D12_RANGE range;
	range.Begin = start * kinc_g5_internal_index_buffer_stride(buffer);
	range.End = (start + count) * kinc_g5_internal_index_buffer_stride(buffer);

	void *p;
	buffer->impl.upload_buffer->lpVtbl->Map(buffer->impl.upload_buffer, 0, &range, &p);
	byte *bytes = (byte *)p;
	bytes += start * kinc_g5_internal_index_buffer_stride(buffer);
	return bytes;
}

void kinc_g5_index_buffer_unlock_all(kinc_g5_index_buffer_t *buffer) {
	D3D12_RANGE range;
	range.Begin = buffer->impl.last_start * kinc_g5_internal_index_buffer_stride(buffer);
	range.End = (buffer->impl.last_start + buffer->impl.last_count) * kinc_g5_internal_index_buffer_stride(buffer);

	buffer->impl.upload_buffer->lpVtbl->Unmap(buffer->impl.upload_buffer, 0, &range);
}

void kinc_g5_index_buffer_unlock(kinc_g5_index_buffer_t *buffer, int count) {
	D3D12_RANGE range;
	range.Begin = buffer->impl.last_start * kinc_g5_internal_index_buffer_stride(buffer);
	range.End = (buffer->impl.last_start + count) * kinc_g5_internal_index_buffer_stride(buffer);

	buffer->impl.upload_buffer->lpVtbl->Unmap(buffer->impl.upload_buffer, 0, &range);
}

void kinc_g5_internal_index_buffer_upload(kinc_g5_index_buffer_t *buffer, ID3D12GraphicsCommandList *commandList) {
	if (!buffer->impl.gpu_memory)
		return;

	commandList->lpVtbl->CopyBufferRegion(commandList, buffer->impl.index_buffer, 0, buffer->impl.upload_buffer, 0, sizeof(uint32_t) * buffer->impl.count);

	D3D12_RESOURCE_BARRIER barriers[1] = {};
	barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barriers[0].Transition.pResource = buffer->impl.index_buffer;
	barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_INDEX_BUFFER;
	barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	commandList->lpVtbl->ResourceBarrier(commandList, 1, barriers);
}

int kinc_g5_index_buffer_count(kinc_g5_index_buffer_t *buffer) {
	return buffer->impl.count;
}
