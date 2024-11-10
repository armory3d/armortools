#include "Direct3D11.h"

#include <kinc/graphics4/compute.h>
#include <kinc/graphics4/texture.h>
#include <kinc/log.h>
#include <kinc/math/core.h>

#include <kinc/backend/SystemMicrosoft.h>

#include <assert.h>

static int getMultipleOf16(int value) {
	int ret = 16;
	while (ret < value)
		ret += 16;
	return ret;
}

void kinc_g4_compute_shader_init(kinc_g4_compute_shader *shader, void *_data, int length) {
	unsigned index = 0;
	uint8_t *data = (uint8_t *)_data;

	memset(&shader->impl.attributes, 0, sizeof(shader->impl.attributes));
	int attributesCount = data[index++];
	for (int i = 0; i < attributesCount; ++i) {
		unsigned char name[256];
		for (unsigned i2 = 0; i2 < 255; ++i2) {
			name[i2] = data[index++];
			if (name[i2] == 0)
				break;
		}
		shader->impl.attributes[i].hash = kinc_internal_hash_name(name);
		shader->impl.attributes[i].index = data[index++];
	}

	memset(&shader->impl.textures, 0, sizeof(shader->impl.textures));
	uint8_t texCount = data[index++];
	for (unsigned i = 0; i < texCount; ++i) {
		unsigned char name[256];
		for (unsigned i2 = 0; i2 < 255; ++i2) {
			name[i2] = data[index++];
			if (name[i2] == 0)
				break;
		}
		shader->impl.textures[i].hash = kinc_internal_hash_name(name);
		shader->impl.textures[i].index = data[index++];
	}

	memset(&shader->impl.constants, 0, sizeof(shader->impl.constants));
	uint8_t constantCount = data[index++];
	shader->impl.constantsSize = 0;
	for (unsigned i = 0; i < constantCount; ++i) {
		unsigned char name[256];
		for (unsigned i2 = 0; i2 < 255; ++i2) {
			name[i2] = data[index++];
			if (name[i2] == 0)
				break;
		}
		kinc_g4_compute_internal_shader_constant constant;
		constant.hash = kinc_internal_hash_name(name);
		constant.offset = *(uint32_t *)&data[index];
		index += 4;
		constant.size = *(uint32_t *)&data[index];
		index += 4;
		constant.columns = data[index];
		index += 1;
		constant.rows = data[index];
		index += 1;

		shader->impl.constants[i] = constant;
		shader->impl.constantsSize = constant.offset + constant.size;
	}

	shader->impl.length = (int)(length - index);
	shader->impl.data = (uint8_t *)malloc(shader->impl.length);
	assert(shader->impl.data != NULL);
	memcpy(shader->impl.data, &data[index], shader->impl.length);

	HRESULT hr =
	    dx_ctx.device->lpVtbl->CreateComputeShader(dx_ctx.device, shader->impl.data, shader->impl.length, NULL, (ID3D11ComputeShader **)&shader->impl.shader);

	if (hr != S_OK) {
		kinc_log(KINC_LOG_LEVEL_WARNING, "Could not initialize compute shader.");
		return;
	}

	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = getMultipleOf16(shader->impl.constantsSize);
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	kinc_microsoft_affirm(dx_ctx.device->lpVtbl->CreateBuffer(dx_ctx.device, &desc, NULL, &shader->impl.constantBuffer));
}

void kinc_g4_compute_shader_destroy(kinc_g4_compute_shader *shader) {}

static kinc_g4_compute_internal_shader_constant *compute_findConstant(kinc_g4_compute_internal_shader_constant *constants, uint32_t hash) {
	for (int i = 0; i < 64; ++i) {
		if (constants[i].hash == hash) {
			return &constants[i];
		}
	}
	return NULL;
}

static kinc_internal_hash_index_t *compute_findTextureUnit(kinc_internal_hash_index_t *units, uint32_t hash) {
	for (int i = 0; i < 64; ++i) {
		if (units[i].hash == hash) {
			return &units[i];
		}
	}
	return NULL;
}

kinc_g4_constant_location_t kinc_g4_compute_shader_get_constant_location(kinc_g4_compute_shader *shader, const char *name) {
	kinc_g4_constant_location_t location = {0};

	uint32_t hash = kinc_internal_hash_name((unsigned char *)name);

	kinc_g4_compute_internal_shader_constant *constant = compute_findConstant(shader->impl.constants, hash);
	if (constant == NULL) {
		location.impl.computeOffset = 0;
		location.impl.computeSize = 0;
		location.impl.computeColumns = 0;
		location.impl.computeRows = 0;
	}
	else {
		location.impl.computeOffset = constant->offset;
		location.impl.computeSize = constant->size;
		location.impl.computeColumns = constant->columns;
		location.impl.computeRows = constant->rows;
	}

	if (location.impl.computeSize == 0) {
		kinc_log(KINC_LOG_LEVEL_WARNING, "Uniform %s not found.", name);
	}

	return location;
}

kinc_g4_texture_unit_t kinc_g4_compute_shader_get_texture_unit(kinc_g4_compute_shader *shader, const char *name) {
	char unitName[64];
	int unitOffset = 0;
	size_t len = strlen(name);
	if (len > 63) {
		len = 63;
	}
	strncpy(unitName, name, len + 1);
	if (unitName[len - 1] == ']') {                  // Check for array - mySampler[2]
		unitOffset = (int)(unitName[len - 2] - '0'); // Array index is unit offset
		unitName[len - 3] = 0;                       // Strip array from name
	}

	uint32_t hash = kinc_internal_hash_name((unsigned char *)unitName);

	kinc_g4_texture_unit_t unit;
	for (int i = 0; i < KINC_G4_SHADER_TYPE_COUNT; ++i) {
		unit.stages[i] = -1;
	}

	kinc_internal_hash_index_t *compute_unit = compute_findTextureUnit(shader->impl.textures, hash);
	if (compute_unit == NULL) {
		unit.stages[KINC_G4_SHADER_TYPE_COMPUTE] = -1;
#ifndef NDEBUG
		static int notFoundCount = 0;
		if (notFoundCount < 10) {
			kinc_log(KINC_LOG_LEVEL_WARNING, "Sampler %s not found.", unitName);
			++notFoundCount;
		}
		else if (notFoundCount == 10) {
			kinc_log(KINC_LOG_LEVEL_WARNING, "Giving up on sampler not found messages.", unitName);
			++notFoundCount;
		}
#endif
	}
	else {
		unit.stages[KINC_G4_SHADER_TYPE_COMPUTE] = compute_unit->index + unitOffset;
	}
	return unit;
}

void kinc_g4_set_compute_shader(kinc_g4_compute_shader *shader) {
	dx_ctx.context->lpVtbl->CSSetShader(dx_ctx.context, (ID3D11ComputeShader *)shader->impl.shader, NULL, 0);
	dx_ctx.context->lpVtbl->UpdateSubresource(dx_ctx.context, (ID3D11Resource *)shader->impl.constantBuffer, 0, NULL, computeConstants, 0, 0);
	dx_ctx.context->lpVtbl->CSSetConstantBuffers(dx_ctx.context, 0, 1, &shader->impl.constantBuffer);
}

void kinc_g4_compute(int x, int y, int z) {
	dx_ctx.context->lpVtbl->Dispatch(dx_ctx.context, x, y, z);

	ID3D11UnorderedAccessView *nullView = NULL;
	dx_ctx.context->lpVtbl->CSSetUnorderedAccessViews(dx_ctx.context, 0, 1, &nullView, NULL);
}
