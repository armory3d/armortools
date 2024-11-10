#include <kinc/graphics4/texture.h>
#include <kinc/graphics4/textureunit.h>

#include <assert.h>

static kinc_g4_texture_t *setTextures[16] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

static DXGI_FORMAT convertFormat(kinc_image_format_t format) {
	switch (format) {
	case KINC_IMAGE_FORMAT_RGBA128:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case KINC_IMAGE_FORMAT_RGBA64:
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case KINC_IMAGE_FORMAT_RGB24:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	case KINC_IMAGE_FORMAT_A32:
		return DXGI_FORMAT_R32_FLOAT;
	case KINC_IMAGE_FORMAT_A16:
		return DXGI_FORMAT_R16_FLOAT;
	case KINC_IMAGE_FORMAT_GREY8:
		return DXGI_FORMAT_R8_UNORM;
	case KINC_IMAGE_FORMAT_BGRA32:
		return DXGI_FORMAT_B8G8R8A8_UNORM;
	case KINC_IMAGE_FORMAT_RGBA32:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	default:
		assert(false);
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	}
}

static int formatByteSize(kinc_image_format_t format) {
	switch (format) {
	case KINC_IMAGE_FORMAT_RGBA128:
		return 16;
	case KINC_IMAGE_FORMAT_RGBA64:
		return 8;
	case KINC_IMAGE_FORMAT_RGB24:
		return 4;
	case KINC_IMAGE_FORMAT_A32:
		return 4;
	case KINC_IMAGE_FORMAT_A16:
		return 2;
	case KINC_IMAGE_FORMAT_GREY8:
		return 1;
	case KINC_IMAGE_FORMAT_BGRA32:
	case KINC_IMAGE_FORMAT_RGBA32:
		return 4;
	default:
		assert(false);
		return 4;
	}
}

static bool isHdr(kinc_image_format_t format) {
	return format == KINC_IMAGE_FORMAT_RGBA128 || format == KINC_IMAGE_FORMAT_RGBA64 || format == KINC_IMAGE_FORMAT_A32 || format == KINC_IMAGE_FORMAT_A16;
}

void kinc_g4_texture_init_from_image(kinc_g4_texture_t *texture, kinc_image_t *image) {
	memset(&texture->impl, 0, sizeof(texture->impl));
	texture->impl.stage = 0;
	texture->tex_width = image->width;
	texture->tex_height = image->height;
	texture->tex_depth = 1;
	texture->format = image->format;
	texture->impl.rowPitch = 0;

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = image->width;
	desc.Height = image->height;
	desc.MipLevels = desc.ArraySize = 1;
	desc.Format = convertFormat(image->format);
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0; // D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = image->data;
	data.SysMemPitch = image->width * formatByteSize(image->format);
	data.SysMemSlicePitch = 0;

	kinc_microsoft_affirm(dx_ctx.device->lpVtbl->CreateTexture2D(dx_ctx.device, &desc, &data, &texture->impl.texture));
	kinc_microsoft_affirm(dx_ctx.device->lpVtbl->CreateShaderResourceView(dx_ctx.device, (ID3D11Resource *)texture->impl.texture, NULL, &texture->impl.view));
}

void kinc_g4_texture_init_from_image3d(kinc_g4_texture_t *texture, kinc_image_t *image) {
	memset(&texture->impl, 0, sizeof(texture->impl));
	texture->impl.stage = 0;
	texture->tex_width = image->width;
	texture->tex_height = image->height;
	texture->tex_depth = image->depth;
	texture->format = image->format;
	texture->impl.rowPitch = 0;

	D3D11_TEXTURE3D_DESC desc;
	desc.Width = image->width;
	desc.Height = image->height;
	desc.Depth = image->depth;
	desc.MipLevels = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.MiscFlags = 0;
	desc.Format = convertFormat(image->format);
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = image->data;
	data.SysMemPitch = image->width * formatByteSize(image->format);
	data.SysMemSlicePitch = image->width * image->height * formatByteSize(image->format);

	kinc_microsoft_affirm(dx_ctx.device->lpVtbl->CreateTexture3D(dx_ctx.device, &desc, &data, &texture->impl.texture3D));
	kinc_microsoft_affirm(dx_ctx.device->lpVtbl->CreateShaderResourceView(dx_ctx.device, (ID3D11Resource *)texture->impl.texture3D, NULL, &texture->impl.view));
}

void kinc_g4_texture_init(kinc_g4_texture_t *texture, int width, int height, kinc_image_format_t format) {
	memset(&texture->impl, 0, sizeof(texture->impl));
	texture->impl.stage = 0;
	texture->tex_width = width;
	texture->tex_height = height;
	texture->tex_depth = 1;
	texture->format = format;

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.MiscFlags = 0;

	if (format == KINC_IMAGE_FORMAT_RGBA128) { // for compute
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
	}
	else {
		desc.Format = convertFormat(format);
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}

	kinc_microsoft_affirm(dx_ctx.device->lpVtbl->CreateTexture2D(dx_ctx.device, &desc, NULL, &texture->impl.texture));
	kinc_microsoft_affirm(dx_ctx.device->lpVtbl->CreateShaderResourceView(dx_ctx.device, (ID3D11Resource *)texture->impl.texture, NULL, &texture->impl.view));

	if (format == KINC_IMAGE_FORMAT_RGBA128) {
		D3D11_UNORDERED_ACCESS_VIEW_DESC du;
		du.Format = desc.Format;
		du.Texture2D.MipSlice = 0;
		du.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		kinc_microsoft_affirm(
		    dx_ctx.device->lpVtbl->CreateUnorderedAccessView(dx_ctx.device, (ID3D11Resource *)texture->impl.texture, &du, &texture->impl.computeView));
	}
}

void kinc_g4_texture_init3d(kinc_g4_texture_t *texture, int width, int height, int depth, kinc_image_format_t format) {
	memset(&texture->impl, 0, sizeof(texture->impl));
	texture->impl.stage = 0;
	texture->tex_width = width;
	texture->tex_height = height;
	texture->tex_depth = depth;
	texture->format = format;
	texture->impl.hasMipmaps = true;

	D3D11_TEXTURE3D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.Depth = depth;
	desc.MipLevels = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	desc.Format = format == KINC_IMAGE_FORMAT_RGBA32 ? DXGI_FORMAT_R8G8B8A8_UNORM : DXGI_FORMAT_R8_UNORM;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;

	kinc_microsoft_affirm(dx_ctx.device->lpVtbl->CreateTexture3D(dx_ctx.device, &desc, NULL, &texture->impl.texture3D));
	kinc_microsoft_affirm(dx_ctx.device->lpVtbl->CreateShaderResourceView(dx_ctx.device, (ID3D11Resource *)texture->impl.texture3D, NULL, &texture->impl.view));
}

// TextureImpl::TextureImpl() : hasMipmaps(false), renderView(nullptr), computeView(nullptr) {}

void kinc_internal_texture_unset(kinc_g4_texture_t *texture);

void kinc_g4_texture_destroy(kinc_g4_texture_t *texture) {
	kinc_internal_texture_unset(texture);
	if (texture->impl.view != NULL) {
		texture->impl.view->lpVtbl->Release(texture->impl.view);
	}
	if (texture->impl.texture != NULL) {
		texture->impl.texture->lpVtbl->Release(texture->impl.texture);
	}
	if (texture->impl.texture3D != NULL) {
		texture->impl.texture3D->lpVtbl->Release(texture->impl.texture3D);
	}
	if (texture->impl.computeView != NULL) {
		texture->impl.computeView->lpVtbl->Release(texture->impl.computeView);
	}
}

void kinc_internal_texture_unmipmap(kinc_g4_texture_t *texture) {
	texture->impl.hasMipmaps = false;
}

void kinc_internal_texture_set(kinc_g4_texture_t *texture, kinc_g4_texture_unit_t unit) {
	if (unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT] < 0 && unit.stages[KINC_G4_SHADER_TYPE_VERTEX] < 0 && unit.stages[KINC_G4_SHADER_TYPE_COMPUTE] < 0)
		return;

	if (unit.stages[KINC_G4_SHADER_TYPE_VERTEX] >= 0) {
		dx_ctx.context->lpVtbl->VSSetShaderResources(dx_ctx.context, unit.stages[KINC_G4_SHADER_TYPE_VERTEX], 1, &texture->impl.view);
	}

	if (unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT] >= 0) {
		dx_ctx.context->lpVtbl->PSSetShaderResources(dx_ctx.context, unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT], 1, &texture->impl.view);
	}

	if (unit.stages[KINC_G4_SHADER_TYPE_COMPUTE] >= 0) {
		dx_ctx.context->lpVtbl->PSSetShaderResources(dx_ctx.context, unit.stages[KINC_G4_SHADER_TYPE_COMPUTE], 1, &texture->impl.view);
	}

	if (unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT] >= 0 || unit.stages[KINC_G4_SHADER_TYPE_VERTEX] >= 0) {
		texture->impl.stage =
		    unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT] >= 0 ? unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT] : unit.stages[KINC_G4_SHADER_TYPE_VERTEX];
		setTextures[texture->impl.stage] = texture;
	}
}

void kinc_internal_texture_set_image(kinc_g4_texture_t *texture, kinc_g4_texture_unit_t unit) {
	if (unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT] < 0 && unit.stages[KINC_G4_SHADER_TYPE_VERTEX] < 0 && unit.stages[KINC_G4_SHADER_TYPE_COMPUTE] < 0)
		return;

	if (texture->impl.computeView == NULL) {
		D3D11_UNORDERED_ACCESS_VIEW_DESC du;
		du.Format = texture->format == KINC_IMAGE_FORMAT_RGBA32 ? DXGI_FORMAT_R8G8B8A8_UNORM : DXGI_FORMAT_R8_UNORM;
		du.Texture3D.MipSlice = 0;
		du.Texture3D.FirstWSlice = 0;
		du.Texture3D.WSize = -1;
		du.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
		kinc_microsoft_affirm(
		    dx_ctx.device->lpVtbl->CreateUnorderedAccessView(dx_ctx.device, (ID3D11Resource *)texture->impl.texture3D, &du, &texture->impl.computeView));
	}

	dx_ctx.context->lpVtbl->OMSetRenderTargetsAndUnorderedAccessViews(dx_ctx.context, 0, NULL, NULL, unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT], 1,
	                                                                  &texture->impl.computeView, NULL);

	/*if (unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT] >= 0) {
		ID3D11ShaderResourceView *nullView = NULL;
		dx_ctx.context->lpVtbl->PSSetShaderResources(dx_ctx.context, 0, 1, &nullView);

		dx_ctx.context->lpVtbl->CSSetUnorderedAccessViews(dx_ctx.context, unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT], 1, &texture->impl.computeView, NULL);
	}
	if (unit.stages[KINC_G4_SHADER_TYPE_VERTEX] >= 0) {
		ID3D11ShaderResourceView *nullView = NULL;
		dx_ctx.context->lpVtbl->PSSetShaderResources(dx_ctx.context, 0, 1, &nullView);

		dx_ctx.context->lpVtbl->CSSetUnorderedAccessViews(dx_ctx.context, unit.stages[KINC_G4_SHADER_TYPE_VERTEX], 1, &texture->impl.computeView, NULL);
	}
	if (unit.stages[KINC_G4_SHADER_TYPE_COMPUTE] >= 0) {
		ID3D11ShaderResourceView *nullView = NULL;
		dx_ctx.context->lpVtbl->PSSetShaderResources(dx_ctx.context, 0, 1, &nullView);

		dx_ctx.context->lpVtbl->CSSetUnorderedAccessViews(dx_ctx.context, unit.stages[KINC_G4_SHADER_TYPE_COMPUTE], 1, &texture->impl.computeView, NULL);
	}*/
}

void kinc_internal_texture_unset(kinc_g4_texture_t *texture) {
	if (setTextures[texture->impl.stage] == texture) {

		setTextures[texture->impl.stage] = NULL;
	}
}

uint8_t *kinc_g4_texture_lock(kinc_g4_texture_t *texture) {
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	kinc_microsoft_affirm(dx_ctx.context->lpVtbl->Map(dx_ctx.context, (ID3D11Resource *)texture->impl.texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	texture->impl.rowPitch = mappedResource.RowPitch;
	return (uint8_t *)mappedResource.pData;
}

void kinc_g4_texture_unlock(kinc_g4_texture_t *texture) {
	dx_ctx.context->lpVtbl->Unmap(dx_ctx.context, (ID3D11Resource *)texture->impl.texture, 0);
}

void kinc_g4_texture_clear(kinc_g4_texture_t *texture, int x, int y, int z, int width, int height, int depth, unsigned color) {
	if (texture->impl.renderView == NULL) {
		texture->tex_depth > 1 ? kinc_microsoft_affirm(dx_ctx.device->lpVtbl->CreateRenderTargetView(dx_ctx.device, (ID3D11Resource *)texture->impl.texture3D,
		                                                                                             0, &texture->impl.renderView))
		                       : kinc_microsoft_affirm(dx_ctx.device->lpVtbl->CreateRenderTargetView(dx_ctx.device, (ID3D11Resource *)texture->impl.texture, 0,
		                                                                                             &texture->impl.renderView));
	}
	static float clearColor[4];
	clearColor[0] = ((color & 0x00ff0000) >> 16) / 255.0f;
	clearColor[1] = ((color & 0x0000ff00) >> 8) / 255.0f;
	clearColor[2] = (color & 0x000000ff) / 255.0f;
	clearColor[3] = ((color & 0xff000000) >> 24) / 255.0f;
	dx_ctx.context->lpVtbl->ClearRenderTargetView(dx_ctx.context, texture->impl.renderView, clearColor);
}

int kinc_g4_texture_stride(kinc_g4_texture_t *texture) {
	assert(texture->impl.rowPitch != 0); // stride is not yet set, lock and unlock the texture first (or find a good fix for this and send a PR)
	return texture->impl.rowPitch;
}

static void enableMipmaps(kinc_g4_texture_t *texture, int texWidth, int texHeight, int format) {
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = texWidth;
	desc.Height = texHeight;
	desc.MipLevels = 0;
	desc.ArraySize = 1;
	desc.Format = convertFormat((kinc_image_format_t)format);
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	ID3D11Texture2D *mipMappedTexture;
	ID3D11ShaderResourceView *mipMappedView;
	kinc_microsoft_affirm(dx_ctx.device->lpVtbl->CreateTexture2D(dx_ctx.device, &desc, NULL, &mipMappedTexture));
	kinc_microsoft_affirm(dx_ctx.device->lpVtbl->CreateShaderResourceView(dx_ctx.device, (ID3D11Resource *)mipMappedTexture, NULL, &mipMappedView));

	D3D11_BOX sourceRegion;
	sourceRegion.left = 0;
	sourceRegion.right = texWidth;
	sourceRegion.top = 0;
	sourceRegion.bottom = texHeight;
	sourceRegion.front = 0;
	sourceRegion.back = 1;
	dx_ctx.context->lpVtbl->CopySubresourceRegion(dx_ctx.context, (ID3D11Resource *)mipMappedTexture, 0, 0, 0, 0, (ID3D11Resource *)texture->impl.texture, 0,
	                                              &sourceRegion);

	if (texture->impl.texture != NULL) {
		texture->impl.texture->lpVtbl->Release(texture->impl.texture);
	}
	texture->impl.texture = mipMappedTexture;

	if (texture->impl.view != NULL) {
		texture->impl.view->lpVtbl->Release(texture->impl.view);
	}
	texture->impl.view = mipMappedView;

	texture->impl.hasMipmaps = true;
}

void kinc_g4_texture_generate_mipmaps(kinc_g4_texture_t *texture, int levels) {
	if (!texture->impl.hasMipmaps) {
		enableMipmaps(texture, texture->tex_width, texture->tex_height, texture->format);
	}
	dx_ctx.context->lpVtbl->GenerateMips(dx_ctx.context, texture->impl.view);
}

void kinc_g4_texture_set_mipmap(kinc_g4_texture_t *texture, kinc_image_t *mipmap, int level) {
	if (!texture->impl.hasMipmaps) {
		enableMipmaps(texture, texture->tex_width, texture->tex_height, texture->format);
	}
	D3D11_BOX dstRegion;
	dstRegion.left = 0;
	dstRegion.right = mipmap->width;
	dstRegion.top = 0;
	dstRegion.bottom = mipmap->height;
	dstRegion.front = 0;
	dstRegion.back = 1;
	dx_ctx.context->lpVtbl->UpdateSubresource(dx_ctx.context, (ID3D11Resource *)texture->impl.texture, level, &dstRegion, mipmap->data,
	                                          mipmap->width * formatByteSize(mipmap->format), 0);
}
