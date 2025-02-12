#include <kinc/backend/system_microsoft.h>
#include <kinc/graphics5/shader.h>
#include <kinc/math/core.h>

void kinc_g5_shader_init(kinc_g5_shader_t *shader, const void *_data, size_t length, kinc_g5_shader_type_t type) {
	memset(shader->impl.constants, 0, sizeof(shader->impl.constants));
	memset(shader->impl.attributes, 0, sizeof(shader->impl.attributes));
	memset(shader->impl.textures, 0, sizeof(shader->impl.textures));

	unsigned index = 0;
	uint8_t *data = (uint8_t *)_data;

	int attributesCount = data[index++];
	for (int i = 0; i < attributesCount; ++i) {
		char name[64];
		for (unsigned i2 = 0; i2 < 63; ++i2) {
			name[i2] = data[index++];
			if (name[i2] == 0)
				break;
		}
		strcpy(shader->impl.attributes[i].name, name);
		shader->impl.attributes[i].attribute = data[index++];
	}

	uint8_t texCount = data[index++];
	for (unsigned i = 0; i < texCount; ++i) {
		char name[64];
		for (unsigned i2 = 0; i2 < 63; ++i2) {
			name[i2] = data[index++];
			if (name[i2] == 0)
				break;
		}
		strcpy(shader->impl.textures[i].name, name);
		shader->impl.textures[i].texture = data[index++];
	}
	shader->impl.texturesCount = texCount;

	uint8_t constantCount = data[index++];
	shader->impl.constantsSize = 0;
	for (unsigned i = 0; i < constantCount; ++i) {
		char name[64];
		for (unsigned i2 = 0; i2 < 63; ++i2) {
			name[i2] = data[index++];
			if (name[i2] == 0)
				break;
		}
		ShaderConstant constant;
		memcpy(&constant.offset, &data[index], sizeof(constant.offset));
		index += 4;
		memcpy(&constant.size, &data[index], sizeof(constant.size));
		index += 4;
		index += 2; // columns and rows
		strcpy(constant.name, name);
		shader->impl.constants[i] = constant;
		shader->impl.constantsSize = constant.offset + constant.size;
	}

	shader->impl.length = (int)length - index;
	shader->impl.data = (uint8_t *)malloc(shader->impl.length);
	memcpy(shader->impl.data, &data[index], shader->impl.length);
}

void kinc_g5_shader_destroy(kinc_g5_shader_t *shader) {
	free(shader->impl.data);
}
