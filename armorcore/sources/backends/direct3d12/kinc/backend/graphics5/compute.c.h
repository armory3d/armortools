#include <kinc/graphics5/compute.h>
#include <kinc/graphics4/texture.h>
#include <kinc/log.h>
#include <kinc/math/core.h>
#include <kinc/backend/system_microsoft.h>

void kinc_g5_compute_shader_init(kinc_g5_compute_shader *shader, void *_data, int length) {
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
		kinc_compute_internal_shader_constant_t constant;
		constant.hash = kinc_internal_hash_name(name);
		memcpy(&constant.offset, &data[index], sizeof(constant.offset));
		index += 4;
		memcpy(&constant.size, &data[index], sizeof(constant.size));
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

	D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {0};
	desc.CS.BytecodeLength = shader->impl.length;
	desc.CS.pShaderBytecode = shader->impl.data;
	desc.pRootSignature = globalComputeRootSignature;
	HRESULT hr = device->lpVtbl->CreateComputePipelineState(device , &desc, &IID_ID3D12PipelineState, &shader->impl.pso);

	if (hr != S_OK) {
		kinc_log(KINC_LOG_LEVEL_WARNING, "Could not initialize compute shader.");
		return;
	}

	// kinc_microsoft_affirm(device->CreateBuffer(&CD3D11_BUFFER_DESC(getMultipleOf16(shader->impl.constantsSize), D3D11_BIND_CONSTANT_BUFFER), nullptr,
	// &shader->impl.constantBuffer));
}

void kinc_g5_compute_shader_destroy(kinc_g5_compute_shader *shader) {
	if (shader->impl.pso != NULL) {
		shader->impl.pso->lpVtbl->Release(shader->impl.pso);
		shader->impl.pso = NULL;
	}
}

static kinc_compute_internal_shader_constant_t *findComputeConstant(kinc_compute_internal_shader_constant_t *constants, uint32_t hash) {
	for (int i = 0; i < 64; ++i) {
		if (constants[i].hash == hash) {
			return &constants[i];
		}
	}
	return NULL;
}

static kinc_internal_hash_index_t *findComputeTextureUnit(kinc_internal_hash_index_t *units, uint32_t hash) {
	for (int i = 0; i < 64; ++i) {
		if (units[i].hash == hash) {
			return &units[i];
		}
	}
	return NULL;
}

kinc_g5_constant_location_t kinc_g5_compute_shader_get_constant_location(kinc_g5_compute_shader *shader, const char *name) {
	kinc_g5_constant_location_t location = {0};

	uint32_t hash = kinc_internal_hash_name((unsigned char *)name);

	kinc_compute_internal_shader_constant_t *constant = findComputeConstant(shader->impl.constants, hash);
	if (constant == NULL) {
		location.impl.computeOffset = 0;
		location.impl.computeSize = 0;
	}
	else {
		location.impl.computeOffset = constant->offset;
		location.impl.computeSize = constant->size;
	}

	if (location.impl.computeSize == 0) {
		kinc_log(KINC_LOG_LEVEL_WARNING, "Uniform %s not found.", name);
	}

	return location;
}

kinc_g5_texture_unit_t kinc_g5_compute_shader_get_texture_unit(kinc_g5_compute_shader *shader, const char *name) {
	char unitName[64];
	int unitOffset = 0;
	size_t len = strlen(name);
	if (len > 63)
		len = 63;
	strncpy(unitName, name, len + 1);
	if (unitName[len - 1] == ']') {                  // Check for array - mySampler[2]
		unitOffset = (int)(unitName[len - 2] - '0'); // Array index is unit offset
		unitName[len - 3] = 0;                       // Strip array from name
	}

	uint32_t hash = kinc_internal_hash_name((unsigned char *)unitName);

	kinc_g5_texture_unit_t unit;
	for (int i = 0; i < KINC_G5_SHADER_TYPE_COUNT; ++i) {
		unit.stages[i] = -1;
	}
	kinc_internal_hash_index_t *computeUnit = findComputeTextureUnit(shader->impl.textures, hash);
	if (computeUnit == NULL) {
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
		unit.stages[KINC_G5_SHADER_TYPE_COMPUTE] = computeUnit->index + unitOffset;
	}
	return unit;
}
