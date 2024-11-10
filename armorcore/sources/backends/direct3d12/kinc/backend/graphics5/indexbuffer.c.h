#include "indexbuffer.h"

#include <kinc/backend/SystemMicrosoft.h>
#include <kinc/graphics5/indexbuffer.h>

void kinc_g5_index_buffer_init(kinc_g5_index_buffer_t *buffer, int count, kinc_g5_index_buffer_format_t format, bool gpuMemory) {
	buffer->impl.count = count;
	buffer->impl.gpu_memory = gpuMemory;
	buffer->impl.format = format;

	// static_assert(sizeof(D3D12IindexBufferView) == sizeof(D3D12_INDEX_BUFFER_VIEW), "Something is wrong with D3D12IindexBufferView");
	int uploadBufferSize = format == KINC_G5_INDEX_BUFFER_FORMAT_16BIT ? sizeof(uint16_t) * count : sizeof(uint32_t) * count;

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

	kinc_microsoft_affirm(device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
	                                                      IID_GRAPHICS_PPV_ARGS(&buffer->impl.upload_buffer)));

	if (gpuMemory) {
		D3D12_HEAP_PROPERTIES heapProperties;
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask = 1;
		heapProperties.VisibleNodeMask = 1;

		kinc_microsoft_affirm(device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, NULL,
		                                                      IID_GRAPHICS_PPV_ARGS(&buffer->impl.index_buffer)));

		buffer->impl.index_buffer_view.BufferLocation = buffer->impl.index_buffer->GetGPUVirtualAddress();
	}
	else {
		buffer->impl.index_buffer_view.BufferLocation = buffer->impl.upload_buffer->GetGPUVirtualAddress();
	}
	buffer->impl.index_buffer_view.SizeInBytes = uploadBufferSize;
	buffer->impl.index_buffer_view.Format = format == KINC_G5_INDEX_BUFFER_FORMAT_16BIT ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

	buffer->impl.last_start = 0;
	buffer->impl.last_count = kinc_g5_index_buffer_count(buffer);
}

void kinc_g5_index_buffer_destroy(kinc_g5_index_buffer_t *buffer) {
	if (buffer->impl.index_buffer != NULL) {
		buffer->impl.index_buffer->Release();
		buffer->impl.index_buffer = NULL;
	}

	buffer->impl.upload_buffer->Release();
	buffer->impl.upload_buffer = NULL;
}

static int kinc_g5_internal_index_buffer_stride(kinc_g5_index_buffer_t *buffer) {
	return buffer->impl.format == KINC_G5_INDEX_BUFFER_FORMAT_32BIT ? 4 : 2;
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
	buffer->impl.upload_buffer->Map(0, &range, &p);
	byte *bytes = (byte *)p;
	bytes += start * kinc_g5_internal_index_buffer_stride(buffer);
	return bytes;
}

void kinc_g5_index_buffer_unlock_all(kinc_g5_index_buffer_t *buffer) {
	D3D12_RANGE range;
	range.Begin = buffer->impl.last_start * kinc_g5_internal_index_buffer_stride(buffer);
	range.End = (buffer->impl.last_start + buffer->impl.last_count) * kinc_g5_internal_index_buffer_stride(buffer);

	buffer->impl.upload_buffer->Unmap(0, &range);
}

void kinc_g5_index_buffer_unlock(kinc_g5_index_buffer_t *buffer, int count) {
	D3D12_RANGE range;
	range.Begin = buffer->impl.last_start * kinc_g5_internal_index_buffer_stride(buffer);
	range.End = (buffer->impl.last_start + count) * kinc_g5_internal_index_buffer_stride(buffer);

	buffer->impl.upload_buffer->Unmap(0, &range);
}

void kinc_g5_internal_index_buffer_upload(kinc_g5_index_buffer_t *buffer, ID3D12GraphicsCommandList *commandList) {
	if (!buffer->impl.gpu_memory)
		return;

	commandList->CopyBufferRegion(buffer->impl.index_buffer, 0, buffer->impl.upload_buffer, 0,
	                              buffer->impl.format == KINC_G5_INDEX_BUFFER_FORMAT_16BIT ? sizeof(uint16_t) * buffer->impl.count
	                                                                                       : sizeof(uint32_t) * buffer->impl.count);

	D3D12_RESOURCE_BARRIER barriers[1] = {};
	barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barriers[0].Transition.pResource = buffer->impl.index_buffer;
	barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_INDEX_BUFFER;
	barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	commandList->ResourceBarrier(1, barriers);
}

int kinc_g5_index_buffer_count(kinc_g5_index_buffer_t *buffer) {
	return buffer->impl.count;
}
