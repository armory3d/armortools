
#include <kinc/backend/system_microsoft.h>

bool kinc_g5_transposeMat = false;

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
