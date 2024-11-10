#pragma once

struct ID3D11Texture2D;
struct ID3D11Texture3D;
struct ID3D11ShaderResourceView;
struct ID3D11UnorderedAccessView;
struct ID3D11RenderTargetView;

// TextureImpl();
//~TextureImpl();
// void enableMipmaps(int texWidth, int texHeight, int format);
// void unmipmap();
// void unset();

typedef struct {
	bool hasMipmaps;
	int stage;
	struct ID3D11Texture2D *texture;
	struct ID3D11Texture3D *texture3D;
	struct ID3D11ShaderResourceView *view;
	struct ID3D11UnorderedAccessView *computeView;
	struct ID3D11RenderTargetView *renderView;
	int rowPitch;
} kinc_g4_texture_impl_t;
