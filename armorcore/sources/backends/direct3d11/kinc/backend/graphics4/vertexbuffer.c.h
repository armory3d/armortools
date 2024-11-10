#include <kinc/graphics4/vertexbuffer.h>

void kinc_g4_vertex_buffer_init(kinc_g4_vertex_buffer_t *buffer, int count, kinc_g4_vertex_structure_t *structure, kinc_g4_usage_t usage,
                                int instance_data_step_rate) {
	buffer->impl.count = count;
	buffer->impl.stride = 0;
	for (int i = 0; i < structure->size; ++i) {
		buffer->impl.stride += kinc_g4_vertex_data_size(structure->elements[i].data);
	}

	if (usage == KINC_G4_USAGE_DYNAMIC) {
		buffer->impl.vertices = NULL;
	}
	else {
		buffer->impl.vertices = (float *)malloc(buffer->impl.stride * count);
	}

	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.CPUAccessFlags = 0;

	buffer->impl.usage = usage;
	switch (usage) {
	case KINC_G4_USAGE_STATIC:
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		break;
	case KINC_G4_USAGE_DYNAMIC:
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		break;
	case KINC_G4_USAGE_READABLE:
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		break;
	}

	bufferDesc.ByteWidth = buffer->impl.stride * count;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	kinc_microsoft_affirm(dx_ctx.device->lpVtbl->CreateBuffer(dx_ctx.device, &bufferDesc, NULL, &buffer->impl.vb));
}

void kinc_g4_vertex_buffer_destroy(kinc_g4_vertex_buffer_t *buffer) {
	buffer->impl.vb->lpVtbl->Release(buffer->impl.vb);
	free(buffer->impl.vertices);
	buffer->impl.vertices = NULL;
}

float *kinc_g4_vertex_buffer_lock_all(kinc_g4_vertex_buffer_t *buffer) {
	return kinc_g4_vertex_buffer_lock(buffer, 0, buffer->impl.count);
}

float *kinc_g4_vertex_buffer_lock(kinc_g4_vertex_buffer_t *buffer, int start, int count) {
	buffer->impl.lockStart = start;
	buffer->impl.lockCount = count;

	if (buffer->impl.usage == KINC_G4_USAGE_DYNAMIC) {
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		memset(&mappedResource, 0, sizeof(D3D11_MAPPED_SUBRESOURCE));
		dx_ctx.context->lpVtbl->Map(dx_ctx.context, (ID3D11Resource *)buffer->impl.vb, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		float *data = (float *)mappedResource.pData;
		return &data[start * buffer->impl.stride / 4];
	}
	else {
		return &buffer->impl.vertices[start * buffer->impl.stride / 4];
	}
}

void kinc_g4_vertex_buffer_unlock_all(kinc_g4_vertex_buffer_t *buffer) {
	kinc_g4_vertex_buffer_unlock(buffer, buffer->impl.lockCount);
}

void kinc_g4_vertex_buffer_unlock(kinc_g4_vertex_buffer_t *buffer, int count) {
	if (buffer->impl.usage == KINC_G4_USAGE_DYNAMIC) {
		dx_ctx.context->lpVtbl->Unmap(dx_ctx.context, (ID3D11Resource *)buffer->impl.vb, 0);
	}
	else {
		dx_ctx.context->lpVtbl->UpdateSubresource(dx_ctx.context, (ID3D11Resource *)buffer->impl.vb, 0, NULL, buffer->impl.vertices, 0, 0);
	}
}

int kinc_internal_g4_vertex_buffer_set(kinc_g4_vertex_buffer_t *buffer, int offset) {
	// UINT stride = myStride;
	// UINT internaloffset = 0;
	// dx_ctx.context->IASetVertexBuffers(0, 1, &vb, &stride, &internaloffset);
	return 0;
}

int kinc_g4_vertex_buffer_count(kinc_g4_vertex_buffer_t *buffer) {
	return buffer->impl.count;
}

int kinc_g4_vertex_buffer_stride(kinc_g4_vertex_buffer_t *buffer) {
	return buffer->impl.stride;
}
