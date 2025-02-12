#pragma once

struct ID3D12Resource;
struct ID3D12DescriptorHeap;
struct ID3D12GraphicsCommandList;

typedef struct {
	int unit;
} TextureUnit5Impl;

typedef struct {
	bool mipmap;
	int stage;
	int stride;
	struct ID3D12Resource *image;
	struct ID3D12Resource *uploadImage;
	struct ID3D12DescriptorHeap *srvDescriptorHeap;
} Texture5Impl;

struct kinc_g5_texture;
struct kinc_g5_command_list;

void kinc_g5_internal_set_textures(struct kinc_g5_command_list *commandList);
void kinc_g5_internal_texture_set(struct kinc_g5_command_list *commandList, struct kinc_g5_texture *texture, int unit);
