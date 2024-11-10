#include <kinc/graphics4/indexbuffer.h>

void kinc_g4_index_buffer_init(kinc_g4_index_buffer_t *buffer, int count, kinc_g4_index_buffer_format_t format, kinc_g4_usage_t usage) {
	buffer->impl.count = count;
	buffer->impl.sixteen = format == KINC_G4_INDEX_BUFFER_FORMAT_16BIT;
	buffer->impl.last_start = 0;
	buffer->impl.last_count = count;

	uint32_t byte_size = buffer->impl.sixteen ? sizeof(uint16_t) * count : sizeof(uint32_t) * count;

	if (usage == KINC_G4_USAGE_DYNAMIC) {
		buffer->impl.indices = NULL;
	}
	else {
		buffer->impl.indices = malloc(byte_size);
	}

	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = byte_size;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

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

	kinc_microsoft_affirm(dx_ctx.device->lpVtbl->CreateBuffer(dx_ctx.device, &bufferDesc, NULL, &buffer->impl.ib));
}

void kinc_g4_index_buffer_destroy(kinc_g4_index_buffer_t *buffer) {
	buffer->impl.ib->lpVtbl->Release(buffer->impl.ib);
	free(buffer->impl.indices);
	buffer->impl.indices = NULL;
}

static int kinc_g4_internal_index_buffer_stride(kinc_g4_index_buffer_t *buffer) {
	return buffer->impl.sixteen ? 2 : 4;
}

void *kinc_g4_index_buffer_lock_all(kinc_g4_index_buffer_t *buffer) {
	return kinc_g4_index_buffer_lock(buffer, 0, kinc_g4_index_buffer_count(buffer));
}

void *kinc_g4_index_buffer_lock(kinc_g4_index_buffer_t *buffer, int start, int count) {
	buffer->impl.last_start = start;
	buffer->impl.last_count = count;

	if (buffer->impl.usage == KINC_G4_USAGE_DYNAMIC) {
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		memset(&mappedResource, 0, sizeof(D3D11_MAPPED_SUBRESOURCE));
		dx_ctx.context->lpVtbl->Map(dx_ctx.context, (ID3D11Resource *)buffer->impl.ib, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		uint8_t *data = (uint8_t *)mappedResource.pData;
		return &data[start * kinc_g4_internal_index_buffer_stride(buffer)];
	}
	else {
		uint8_t *data = (uint8_t *)buffer->impl.indices;
		return &data[start * kinc_g4_internal_index_buffer_stride(buffer)];
	}
}

void kinc_g4_index_buffer_unlock_all(kinc_g4_index_buffer_t *buffer) {
	kinc_g4_index_buffer_unlock(buffer, buffer->impl.last_count);
}

void kinc_g4_index_buffer_unlock(kinc_g4_index_buffer_t *buffer, int count) {
	if (buffer->impl.usage == KINC_G4_USAGE_DYNAMIC) {
		dx_ctx.context->lpVtbl->Unmap(dx_ctx.context, (ID3D11Resource *)buffer->impl.ib, 0);
	}
	else {
		dx_ctx.context->lpVtbl->UpdateSubresource(dx_ctx.context, (ID3D11Resource *)buffer->impl.ib, 0, NULL, buffer->impl.indices, 0, 0);
	}
}

int kinc_g4_index_buffer_count(kinc_g4_index_buffer_t *buffer) {
	return buffer->impl.count;
}
