#include "vertexbuffer.h"

#include <kinc/graphics5/vertexbuffer.h>

#include <kinc/backend/SystemMicrosoft.h>
#include <kinc/graphics4/graphics.h>

kinc_g5_vertex_buffer_t *_current_vertex_buffer = NULL;

void kinc_g5_vertex_buffer_init(kinc_g5_vertex_buffer_t *buffer, int count, kinc_g5_vertex_structure_t *structure, bool gpuMemory, int instanceDataStepRate) {
	buffer->impl.myCount = count;

	// static_assert(sizeof(D3D12VertexBufferView) == sizeof(D3D12_VERTEX_BUFFER_VIEW), "Something is wrong with D3D12IVertexBufferView");

	buffer->impl.myStride = 0;
	for (int i = 0; i < structure->size; ++i) {
		buffer->impl.myStride += kinc_g4_vertex_data_size(structure->elements[i].data);
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

	device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
	                                IID_GRAPHICS_PPV_ARGS(&buffer->impl.uploadBuffer));

	// device_->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES (D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
	// &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
	//	D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&vertexBuffer));

	buffer->impl.view.BufferLocation = buffer->impl.uploadBuffer->GetGPUVirtualAddress();
	buffer->impl.view.SizeInBytes = uploadBufferSize;
	buffer->impl.view.StrideInBytes = buffer->impl.myStride;

	buffer->impl.lastStart = 0;
	buffer->impl.lastCount = kinc_g5_vertex_buffer_count(buffer);
}

void kinc_g5_vertex_buffer_destroy(kinc_g5_vertex_buffer_t *buffer) {
	// buffer->impl.vertexBuffer->Release();
	buffer->impl.uploadBuffer->Release();
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
	buffer->impl.uploadBuffer->Map(0, &range, &p);
	byte *bytes = (byte *)p;
	bytes += start * buffer->impl.myStride;
	return (float *)bytes;
}

void kinc_g5_vertex_buffer_unlock_all(kinc_g5_vertex_buffer_t *buffer) {
	D3D12_RANGE range;
	range.Begin = buffer->impl.lastStart * buffer->impl.myStride;
	range.End = (buffer->impl.lastStart + buffer->impl.lastCount) * buffer->impl.myStride;
	buffer->impl.uploadBuffer->Unmap(0, &range);

	// view.BufferLocation = uploadBuffer->GetGPUVirtualAddress() + myStart * myStride;

	// commandList->CopyBufferRegion(vertexBuffer, 0, uploadBuffer, 0, count() * stride());
	// CD3DX12_RESOURCE_BARRIER barriers[1] = { CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST,
	// D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER) };
	// commandList->ResourceBarrier(1, barriers);
}

void kinc_g5_vertex_buffer_unlock(kinc_g5_vertex_buffer_t *buffer, int count) {
	D3D12_RANGE range;
	range.Begin = buffer->impl.lastStart * buffer->impl.myStride;
	range.End = (buffer->impl.lastStart + count) * buffer->impl.myStride;
	buffer->impl.uploadBuffer->Unmap(0, &range);

	// view.BufferLocation = uploadBuffer->GetGPUVirtualAddress() + myStart * myStride;

	// commandList->CopyBufferRegion(vertexBuffer, 0, uploadBuffer, 0, count() * stride());
	// CD3DX12_RESOURCE_BARRIER barriers[1] = { CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST,
	// D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER) };
	// commandList->ResourceBarrier(1, barriers);
}

int kinc_g5_internal_vertex_buffer_set(kinc_g5_vertex_buffer_t *buffer, int offset) {
	// UINT stride = myStride;
	// UINT offset = 0;
	// context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	_current_vertex_buffer = buffer;
	return 0;
}

int kinc_g5_vertex_buffer_count(kinc_g5_vertex_buffer_t *buffer) {
	return buffer->impl.myCount;
}

int kinc_g5_vertex_buffer_stride(kinc_g5_vertex_buffer_t *buffer) {
	return buffer->impl.myStride;
}
