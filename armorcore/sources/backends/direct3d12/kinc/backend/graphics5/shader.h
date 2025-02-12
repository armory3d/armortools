#pragma once

typedef struct {
	char name[64];
	uint32_t offset;
	uint32_t size;
} ShaderConstant;

typedef struct {
	char name[64];
	int attribute;
} ShaderAttribute;

typedef struct {
	char name[64];
	int texture;
} ShaderTexture;

typedef struct {
	ShaderConstant constants[32];
	int constantsSize;
	ShaderAttribute attributes[32];
	ShaderTexture textures[32];
	int texturesCount;
	void *shader;
	uint8_t *data;
	int length;
} Shader5Impl;
