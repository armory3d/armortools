#include "texture.h"

#include <kinc/graphics5/rendertarget.h>
#include <kinc/graphics5/texture.h>
#include <kinc/math/core.h>

#include <kinc/backend/SystemMicrosoft.h>

#include <math.h>

static const int heapSize = 1024;

#if defined(KINC_WINDOWS)
/*static int d3d12_textureAlignment() {
    return D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
}*/
#else
int d3d12_textureAlignment();
#endif

void kinc_g5_internal_reset_textures(struct kinc_g5_command_list *list) {
	for (int i = 0; i < KINC_INTERNAL_G5_TEXTURE_COUNT; ++i) {
		list->impl.currentRenderTargets[i] = NULL;
		list->impl.currentTextures[i] = NULL;
		list->impl.current_samplers[i] = NULL;
	}
}

static inline UINT64 GetRequiredIntermediateSize(ID3D12Resource *destinationResource, UINT FirstSubresource, UINT NumSubresources) {
	D3D12_RESOURCE_DESC desc = destinationResource->GetDesc();
	UINT64 requiredSize = 0;
	device->GetCopyableFootprints(&desc, FirstSubresource, NumSubresources, 0, NULL, NULL, NULL, &requiredSize);
	device->Release();
	return requiredSize;
}

static DXGI_FORMAT convertImageFormat(kinc_image_format_t format) {
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
		return 4;
	}
}

void kinc_g5_internal_set_textures(kinc_g5_command_list_t *list) {
	if (list->impl.currentRenderTargets[0] != NULL || list->impl.currentTextures[0] != NULL) {
		int srvStep = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		int samplerStep = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

		if (list->impl.heapIndex + KINC_INTERNAL_G5_TEXTURE_COUNT >= heapSize) {
			list->impl.heapIndex = 0;
		}

		D3D12_GPU_DESCRIPTOR_HANDLE srvGpu = list->impl.srvHeap->GetGPUDescriptorHandleForHeapStart();
		D3D12_GPU_DESCRIPTOR_HANDLE samplerGpu = list->impl.samplerHeap->GetGPUDescriptorHandleForHeapStart();
		srvGpu.ptr += list->impl.heapIndex * srvStep;
		samplerGpu.ptr += list->impl.heapIndex * samplerStep;

		for (int i = 0; i < KINC_INTERNAL_G5_TEXTURE_COUNT; ++i) {
			if ((list->impl.currentRenderTargets[i] != NULL || list->impl.currentTextures[i] != NULL) && list->impl.current_samplers[i] != NULL) {
				ID3D12DescriptorHeap *samplerDescriptorHeap = list->impl.current_samplers[i]->impl.sampler_heap;

				D3D12_CPU_DESCRIPTOR_HANDLE srvCpu = list->impl.srvHeap->GetCPUDescriptorHandleForHeapStart();
				D3D12_CPU_DESCRIPTOR_HANDLE samplerCpu = list->impl.samplerHeap->GetCPUDescriptorHandleForHeapStart();
				srvCpu.ptr += list->impl.heapIndex * srvStep;
				samplerCpu.ptr += list->impl.heapIndex * samplerStep;
				++list->impl.heapIndex;

				if (list->impl.currentRenderTargets[i] != NULL) {
					bool is_depth = list->impl.currentRenderTargets[i]->impl.stage_depth == i;
					D3D12_CPU_DESCRIPTOR_HANDLE sourceCpu =
					    is_depth ? list->impl.currentRenderTargets[i]->impl.srvDepthDescriptorHeap->GetCPUDescriptorHandleForHeapStart()
					             : list->impl.currentRenderTargets[i]->impl.srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
					device->CopyDescriptorsSimple(1, srvCpu, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
					device->CopyDescriptorsSimple(1, samplerCpu, samplerDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
					                              D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
				}
				else {
					D3D12_CPU_DESCRIPTOR_HANDLE sourceCpu = list->impl.currentTextures[i]->impl.srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
					device->CopyDescriptorsSimple(1, srvCpu, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
					device->CopyDescriptorsSimple(1, samplerCpu, samplerDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
					                              D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
				}
			}
		}

		ID3D12DescriptorHeap *heaps[2] = {list->impl.srvHeap, list->impl.samplerHeap};
		list->impl._commandList->SetDescriptorHeaps(2, heaps);
		if (compute_pipeline_set) {
			list->impl._commandList->SetComputeRootDescriptorTable(0, srvGpu);
			list->impl._commandList->SetComputeRootDescriptorTable(1, srvGpu);
			list->impl._commandList->SetComputeRootDescriptorTable(2, samplerGpu);
		}
		else {
			list->impl._commandList->SetGraphicsRootDescriptorTable(0, srvGpu);
			list->impl._commandList->SetGraphicsRootDescriptorTable(1, samplerGpu);
		}
	}
}

void createHeaps(kinc_g5_command_list_t *list) {
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = heapSize;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	device->CreateDescriptorHeap(&heapDesc, IID_GRAPHICS_PPV_ARGS(&list->impl.srvHeap));

	D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
	samplerHeapDesc.NumDescriptors = heapSize;
	samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	device->CreateDescriptorHeap(&samplerHeapDesc, IID_GRAPHICS_PPV_ARGS(&list->impl.samplerHeap));
}

extern "C" void kinc_memory_emergency();

void kinc_g5_texture_init_from_image(kinc_g5_texture_t *texture, kinc_image_t *image) {
	memset(&texture->impl, 0, sizeof(texture->impl));
	texture->impl.stage = 0;
	texture->impl.mipmap = true;
	texture->texWidth = image->width;
	texture->texHeight = image->height;

	DXGI_FORMAT d3dformat = convertImageFormat(image->format);
	int formatSize = formatByteSize(image->format);

	D3D12_HEAP_PROPERTIES heapPropertiesDefault;
	heapPropertiesDefault.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapPropertiesDefault.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapPropertiesDefault.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapPropertiesDefault.CreationNodeMask = 1;
	heapPropertiesDefault.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC resourceDescTex;
	resourceDescTex.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDescTex.Alignment = 0;
	resourceDescTex.Width = texture->texWidth;
	resourceDescTex.Height = texture->texHeight;
	resourceDescTex.DepthOrArraySize = 1;
	resourceDescTex.MipLevels = 1;
	resourceDescTex.Format = d3dformat;
	resourceDescTex.SampleDesc.Count = 1;
	resourceDescTex.SampleDesc.Quality = 0;
	resourceDescTex.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDescTex.Flags = D3D12_RESOURCE_FLAG_NONE;

	HRESULT result = device->CreateCommittedResource(&heapPropertiesDefault, D3D12_HEAP_FLAG_NONE, &resourceDescTex, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
	                                                 NULL, IID_GRAPHICS_PPV_ARGS(&texture->impl.image));
	if (result != S_OK) {
		for (int i = 0; i < 10; ++i) {
			kinc_memory_emergency();
			result = device->CreateCommittedResource(&heapPropertiesDefault, D3D12_HEAP_FLAG_NONE, &resourceDescTex, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			                                         NULL, IID_GRAPHICS_PPV_ARGS(&texture->impl.image));
			if (result == S_OK) {
				break;
			}
		}
	}

	D3D12_HEAP_PROPERTIES heapPropertiesUpload;
	heapPropertiesUpload.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapPropertiesUpload.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapPropertiesUpload.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapPropertiesUpload.CreationNodeMask = 1;
	heapPropertiesUpload.VisibleNodeMask = 1;

	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture->impl.image, 0, 1);
	D3D12_RESOURCE_DESC resourceDescBuffer;
	resourceDescBuffer.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDescBuffer.Alignment = 0;
	resourceDescBuffer.Width = uploadBufferSize;
	resourceDescBuffer.Height = 1;
	resourceDescBuffer.DepthOrArraySize = 1;
	resourceDescBuffer.MipLevels = 1;
	resourceDescBuffer.Format = DXGI_FORMAT_UNKNOWN;
	resourceDescBuffer.SampleDesc.Count = 1;
	resourceDescBuffer.SampleDesc.Quality = 0;
	resourceDescBuffer.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDescBuffer.Flags = D3D12_RESOURCE_FLAG_NONE;

	result = device->CreateCommittedResource(&heapPropertiesUpload, D3D12_HEAP_FLAG_NONE, &resourceDescBuffer, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
	                                         IID_GRAPHICS_PPV_ARGS(&texture->impl.uploadImage));
	if (result != S_OK) {
		for (int i = 0; i < 10; ++i) {
			kinc_memory_emergency();
			result = device->CreateCommittedResource(&heapPropertiesUpload, D3D12_HEAP_FLAG_NONE, &resourceDescBuffer, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
			                                         IID_GRAPHICS_PPV_ARGS(&texture->impl.uploadImage));
			if (result == S_OK) {
				break;
			}
		}
	}

	texture->impl.stride = (int)ceilf(uploadBufferSize / (float)(image->height * d3d12_textureAlignment())) * d3d12_textureAlignment();

	BYTE *pixel;
	texture->impl.uploadImage->Map(0, NULL, (void **)&pixel);
	int pitch = kinc_g5_texture_stride(texture);
	for (int y = 0; y < texture->texHeight; ++y) {
		memcpy(&pixel[y * pitch], &((uint8_t *)image->data)[y * texture->texWidth * formatSize], texture->texWidth * formatSize);
	}
	texture->impl.uploadImage->Unmap(0, NULL);

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
	descriptorHeapDesc.NumDescriptors = 1;

	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorHeapDesc.NodeMask = 0;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	device->CreateDescriptorHeap(&descriptorHeapDesc, IID_GRAPHICS_PPV_ARGS(&texture->impl.srvDescriptorHeap));

	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shaderResourceViewDesc.Format = d3dformat;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	device->CreateShaderResourceView(texture->impl.image, &shaderResourceViewDesc, texture->impl.srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

void create_texture(struct kinc_g5_texture *texture, int width, int height, kinc_image_format_t format, D3D12_RESOURCE_FLAGS flags) {
	// kinc_image_init(&texture->image, width, height, format, readable);
	memset(&texture->impl, 0, sizeof(texture->impl));
	texture->impl.stage = 0;
	texture->impl.mipmap = true;
	texture->texWidth = width;
	texture->texHeight = height;

	DXGI_FORMAT d3dformat = convertImageFormat(format);

	D3D12_HEAP_PROPERTIES heapPropertiesDefault;
	heapPropertiesDefault.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapPropertiesDefault.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapPropertiesDefault.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapPropertiesDefault.CreationNodeMask = 1;
	heapPropertiesDefault.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC resourceDescTex;
	resourceDescTex.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDescTex.Alignment = 0;
	resourceDescTex.Width = texture->texWidth;
	resourceDescTex.Height = texture->texHeight;
	resourceDescTex.DepthOrArraySize = 1;
	resourceDescTex.MipLevels = 1;
	resourceDescTex.Format = d3dformat;
	resourceDescTex.SampleDesc.Count = 1;
	resourceDescTex.SampleDesc.Quality = 0;
	resourceDescTex.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDescTex.Flags = flags;

	HRESULT result = device->CreateCommittedResource(&heapPropertiesDefault, D3D12_HEAP_FLAG_NONE, &resourceDescTex, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
	                                                 NULL, IID_GRAPHICS_PPV_ARGS(&texture->impl.image));
	if (result != S_OK) {
		for (int i = 0; i < 10; ++i) {
			kinc_memory_emergency();
			result = device->CreateCommittedResource(&heapPropertiesDefault, D3D12_HEAP_FLAG_NONE, &resourceDescTex, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			                                         NULL, IID_GRAPHICS_PPV_ARGS(&texture->impl.image));
			if (result == S_OK) {
				break;
			}
		}
	}

	D3D12_HEAP_PROPERTIES heapPropertiesUpload;
	heapPropertiesUpload.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapPropertiesUpload.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapPropertiesUpload.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapPropertiesUpload.CreationNodeMask = 1;
	heapPropertiesUpload.VisibleNodeMask = 1;

	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture->impl.image, 0, 1);
	D3D12_RESOURCE_DESC resourceDescBuffer;
	resourceDescBuffer.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDescBuffer.Alignment = 0;
	resourceDescBuffer.Width = uploadBufferSize;
	resourceDescBuffer.Height = 1;
	resourceDescBuffer.DepthOrArraySize = 1;
	resourceDescBuffer.MipLevels = 1;
	resourceDescBuffer.Format = DXGI_FORMAT_UNKNOWN;
	resourceDescBuffer.SampleDesc.Count = 1;
	resourceDescBuffer.SampleDesc.Quality = 0;
	resourceDescBuffer.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDescBuffer.Flags = D3D12_RESOURCE_FLAG_NONE;

	result = device->CreateCommittedResource(&heapPropertiesUpload, D3D12_HEAP_FLAG_NONE, &resourceDescBuffer, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
	                                         IID_GRAPHICS_PPV_ARGS(&texture->impl.uploadImage));
	if (result != S_OK) {
		for (int i = 0; i < 10; ++i) {
			kinc_memory_emergency();
			result = device->CreateCommittedResource(&heapPropertiesUpload, D3D12_HEAP_FLAG_NONE, &resourceDescBuffer, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
			                                         IID_GRAPHICS_PPV_ARGS(&texture->impl.uploadImage));
			if (result == S_OK) {
				break;
			}
		}
	}

	texture->impl.stride = (int)ceilf(uploadBufferSize / (float)(height * d3d12_textureAlignment())) * d3d12_textureAlignment();

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc;
	ZeroMemory(&descriptorHeapDesc, sizeof(descriptorHeapDesc));
	descriptorHeapDesc.NumDescriptors = 1;

	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorHeapDesc.NodeMask = 0;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	device->CreateDescriptorHeap(&descriptorHeapDesc, IID_GRAPHICS_PPV_ARGS(&texture->impl.srvDescriptorHeap));

	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
	shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shaderResourceViewDesc.Format = d3dformat;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	device->CreateShaderResourceView(texture->impl.image, &shaderResourceViewDesc, texture->impl.srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

void kinc_g5_texture_init(struct kinc_g5_texture *texture, int width, int height, kinc_image_format_t format) {
	create_texture(texture, width, height, format, D3D12_RESOURCE_FLAG_NONE);
}

void kinc_g5_texture_init3d(kinc_g5_texture_t *texture, int width, int height, int depth, kinc_image_format_t format) {
	// kinc_image_init3d(&texture->image, width, height, depth, format, readable);
}

void kinc_g5_texture_init_non_sampled_access(struct kinc_g5_texture *texture, int width, int height, kinc_image_format_t format) {
	create_texture(texture, width, height, format, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
}

void kinc_g5_texture_destroy(struct kinc_g5_texture *texture) {
	texture->impl.image->Release();
	texture->impl.uploadImage->Release();
	texture->impl.srvDescriptorHeap->Release();
}

void kinc_g5_internal_texture_unmipmap(struct kinc_g5_texture *texture) {
	texture->impl.mipmap = false;
}

void kinc_g5_internal_texture_set(kinc_g5_command_list_t *list, struct kinc_g5_texture *texture, int unit) {
	if (unit < 0)
		return;
	// context->PSSetShaderResources(unit.unit, 1, &view);
	texture->impl.stage = unit;
	list->impl.currentTextures[texture->impl.stage] = texture;
	list->impl.currentRenderTargets[texture->impl.stage] = NULL;
}

void kinc_g5_internal_sampler_set(kinc_g5_command_list_t *list, kinc_g5_sampler_t *sampler, int unit) {
	if (unit < 0)
		return;

	list->impl.current_samplers[unit] = sampler;
}

uint8_t *kinc_g5_texture_lock(struct kinc_g5_texture *texture) {
	BYTE *pixel;
	texture->impl.uploadImage->Map(0, NULL, (void **)&pixel);
	return pixel;
}

void kinc_g5_texture_unlock(struct kinc_g5_texture *texture) {
	texture->impl.uploadImage->Unmap(0, NULL);
}

void kinc_g5_texture_clear(kinc_g5_texture_t *texture, int x, int y, int z, int width, int height, int depth, unsigned color) {}

int kinc_g5_texture_stride(struct kinc_g5_texture *texture) {
	/*int baseStride = texture->format == KINC_IMAGE_FORMAT_RGBA32 ? (texture->texWidth * 4) : texture->texWidth;
	if (texture->format == KINC_IMAGE_FORMAT_GREY8) return texture->texWidth; // please investigate further
	for (int i = 0;; ++i) {
	    if (d3d12_textureAlignment() * i >= baseStride) {
	        return d3d12_textureAlignment() * i;
	    }
	}*/
	return texture->impl.stride;
}

void kinc_g5_texture_generate_mipmaps(struct kinc_g5_texture *texture, int levels) {}

void kinc_g5_texture_set_mipmap(struct kinc_g5_texture *texture, kinc_image_t *mipmap, int level) {}
