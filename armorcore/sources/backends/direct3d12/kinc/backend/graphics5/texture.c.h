#include "texture.h"

#include <kinc/graphics5/rendertarget.h>
#include <kinc/graphics5/texture.h>
#include <kinc/math/core.h>

#include <kinc/backend/system_microsoft.h>

#include <math.h>

static const int heapSize = 1024;

void kinc_g5_internal_reset_textures(struct kinc_g5_command_list *list) {
	for (int i = 0; i < KINC_INTERNAL_G5_TEXTURE_COUNT; ++i) {
		list->impl.currentRenderTargets[i] = NULL;
		list->impl.currentTextures[i] = NULL;
		list->impl.current_samplers[i] = NULL;
	}
}

static inline UINT64 GetRequiredIntermediateSize(ID3D12Resource *destinationResource, UINT FirstSubresource, UINT NumSubresources) {
	D3D12_RESOURCE_DESC desc;
	destinationResource->lpVtbl->GetDesc(destinationResource, &desc);
	UINT64 requiredSize = 0;
	device->lpVtbl->GetCopyableFootprints(device, &desc, FirstSubresource, NumSubresources, 0, NULL, NULL, NULL, &requiredSize);
	device->lpVtbl->Release(device);
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
		int srvStep = device->lpVtbl->GetDescriptorHandleIncrementSize(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		int samplerStep = device->lpVtbl->GetDescriptorHandleIncrementSize(device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

		if (list->impl.heapIndex + KINC_INTERNAL_G5_TEXTURE_COUNT >= heapSize) {
			list->impl.heapIndex = 0;
		}

		D3D12_GPU_DESCRIPTOR_HANDLE srvGpu;
		list->impl.srvHeap->lpVtbl->GetGPUDescriptorHandleForHeapStart(list->impl.srvHeap, &srvGpu);
		D3D12_GPU_DESCRIPTOR_HANDLE samplerGpu;
		list->impl.samplerHeap->lpVtbl->GetGPUDescriptorHandleForHeapStart(list->impl.samplerHeap, &samplerGpu);
		srvGpu.ptr += list->impl.heapIndex * srvStep;
		samplerGpu.ptr += list->impl.heapIndex * samplerStep;

		for (int i = 0; i < KINC_INTERNAL_G5_TEXTURE_COUNT; ++i) {
			if ((list->impl.currentRenderTargets[i] != NULL || list->impl.currentTextures[i] != NULL) && list->impl.current_samplers[i] != NULL) {
				ID3D12DescriptorHeap *samplerDescriptorHeap = list->impl.current_samplers[i]->impl.sampler_heap;

				D3D12_CPU_DESCRIPTOR_HANDLE srvCpu;
				list->impl.srvHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(list->impl.srvHeap, &srvCpu);
				D3D12_CPU_DESCRIPTOR_HANDLE samplerCpu;
				list->impl.samplerHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(list->impl.samplerHeap, &samplerCpu);
				srvCpu.ptr += list->impl.heapIndex * srvStep;
				samplerCpu.ptr += list->impl.heapIndex * samplerStep;
				++list->impl.heapIndex;

				if (list->impl.currentRenderTargets[i] != NULL) {
					bool is_depth = list->impl.currentRenderTargets[i]->impl.stage_depth == i;
					D3D12_CPU_DESCRIPTOR_HANDLE sourceCpu;

					if (is_depth) {
						list->impl.currentRenderTargets[i]->impl.srvDepthDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(
						    list->impl.currentRenderTargets[i]->impl.srvDepthDescriptorHeap, &sourceCpu);
					}
					else {
						list->impl.currentRenderTargets[i]->impl.srvDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(
						    list->impl.currentRenderTargets[i]->impl.srvDescriptorHeap, &sourceCpu);
					}

					D3D12_CPU_DESCRIPTOR_HANDLE handle;
					samplerDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(samplerDescriptorHeap, &handle);

					device->lpVtbl->CopyDescriptorsSimple(device, 1, srvCpu, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
					device->lpVtbl->CopyDescriptorsSimple(device, 1, samplerCpu, handle, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
				}
				else {
					D3D12_CPU_DESCRIPTOR_HANDLE sourceCpu;
					list->impl.currentTextures[i]->impl.srvDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(
					    list->impl.currentTextures[i]->impl.srvDescriptorHeap, &sourceCpu);
					device->lpVtbl->CopyDescriptorsSimple(device, 1, srvCpu, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

					D3D12_CPU_DESCRIPTOR_HANDLE handle;
					samplerDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(samplerDescriptorHeap, &handle);
					device->lpVtbl->CopyDescriptorsSimple(device, 1, samplerCpu, handle, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
				}
			}
		}

		ID3D12DescriptorHeap *heaps[2] = {list->impl.srvHeap, list->impl.samplerHeap};
		list->impl._commandList->lpVtbl->SetDescriptorHeaps(list->impl._commandList, 2, heaps);
		if (compute_pipeline_set) {
			list->impl._commandList->lpVtbl->SetComputeRootDescriptorTable(list->impl._commandList, 0, srvGpu);
			list->impl._commandList->lpVtbl->SetComputeRootDescriptorTable(list->impl._commandList, 1, srvGpu);
			list->impl._commandList->lpVtbl->SetComputeRootDescriptorTable(list->impl._commandList, 2, samplerGpu);
		}
		else {
			list->impl._commandList->lpVtbl->SetGraphicsRootDescriptorTable(list->impl._commandList, 0, srvGpu);
			list->impl._commandList->lpVtbl->SetGraphicsRootDescriptorTable(list->impl._commandList, 1, samplerGpu);
		}
	}
}

void createHeaps(kinc_g5_command_list_t *list) {
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = heapSize;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	device->lpVtbl->CreateDescriptorHeap(device, &heapDesc, &IID_ID3D12DescriptorHeap, &list->impl.srvHeap);

	D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
	samplerHeapDesc.NumDescriptors = heapSize;
	samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	device->lpVtbl->CreateDescriptorHeap(device, &samplerHeapDesc, &IID_ID3D12DescriptorHeap, &list->impl.samplerHeap);
}

void kinc_memory_emergency();

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

	HRESULT result = device->lpVtbl->CreateCommittedResource(device , &heapPropertiesDefault, D3D12_HEAP_FLAG_NONE, &resourceDescTex,
	                                                         D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
	                                                 NULL, &IID_ID3D12Resource, &texture->impl.image);
	if (result != S_OK) {
		for (int i = 0; i < 10; ++i) {
			kinc_memory_emergency();
			result = device->lpVtbl->CreateCommittedResource(device, &heapPropertiesDefault, D3D12_HEAP_FLAG_NONE, &resourceDescTex,
			                                                 D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			                                         NULL, &IID_ID3D12Resource, &texture->impl.image);
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

	result = device->lpVtbl->CreateCommittedResource(device, &heapPropertiesUpload, D3D12_HEAP_FLAG_NONE, &resourceDescBuffer,
	                                                 D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
	                                         &IID_ID3D12Resource, &texture->impl.uploadImage);
	if (result != S_OK) {
		for (int i = 0; i < 10; ++i) {
			kinc_memory_emergency();
			result = device->lpVtbl->CreateCommittedResource(device, &heapPropertiesUpload, D3D12_HEAP_FLAG_NONE, &resourceDescBuffer,
			                                                 D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
			                                         &IID_ID3D12Resource, &texture->impl.uploadImage);
			if (result == S_OK) {
				break;
			}
		}
	}

	texture->impl.stride = (int)ceilf(uploadBufferSize / (float)(image->height * d3d12_textureAlignment())) * d3d12_textureAlignment();

	BYTE *pixel;
	texture->impl.uploadImage->lpVtbl->Map(texture->impl.uploadImage, 0, NULL, (void **)&pixel);
	int pitch = kinc_g5_texture_stride(texture);
	for (int y = 0; y < texture->texHeight; ++y) {
		memcpy(&pixel[y * pitch], &((uint8_t *)image->data)[y * texture->texWidth * formatSize], texture->texWidth * formatSize);
	}
	texture->impl.uploadImage->lpVtbl->Unmap(texture->impl.uploadImage, 0, NULL);

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
	descriptorHeapDesc.NumDescriptors = 1;

	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorHeapDesc.NodeMask = 0;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	device->lpVtbl->CreateDescriptorHeap(device, &descriptorHeapDesc, &IID_ID3D12DescriptorHeap, &texture->impl.srvDescriptorHeap);

	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shaderResourceViewDesc.Format = d3dformat;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	D3D12_CPU_DESCRIPTOR_HANDLE handle;
	texture->impl.srvDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(texture->impl.srvDescriptorHeap, &handle);
	device->lpVtbl->CreateShaderResourceView(device, texture->impl.image, &shaderResourceViewDesc, handle);
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

	HRESULT result = device->lpVtbl->CreateCommittedResource(device, &heapPropertiesDefault, D3D12_HEAP_FLAG_NONE, &resourceDescTex,
	                                                         D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
	                                                 NULL, &IID_ID3D12Resource, &texture->impl.image);
	if (result != S_OK) {
		for (int i = 0; i < 10; ++i) {
			kinc_memory_emergency();
			result = device->lpVtbl->CreateCommittedResource(device, &heapPropertiesDefault, D3D12_HEAP_FLAG_NONE, &resourceDescTex,
			                                                 D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			                                         NULL, &IID_ID3D12Resource, &texture->impl.image);
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

	result = device->lpVtbl->CreateCommittedResource(device, &heapPropertiesUpload, D3D12_HEAP_FLAG_NONE, &resourceDescBuffer,
	                                                 D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
	                                         &IID_ID3D12Resource, &texture->impl.uploadImage);
	if (result != S_OK) {
		for (int i = 0; i < 10; ++i) {
			kinc_memory_emergency();
			result = device->lpVtbl->CreateCommittedResource(device, &heapPropertiesUpload, D3D12_HEAP_FLAG_NONE, &resourceDescBuffer,
			                                                 D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
			                                         &IID_ID3D12Resource, &texture->impl.uploadImage);
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

	device->lpVtbl->CreateDescriptorHeap(device, &descriptorHeapDesc, &IID_ID3D12DescriptorHeap, &texture->impl.srvDescriptorHeap);

	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
	shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shaderResourceViewDesc.Format = d3dformat;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	D3D12_CPU_DESCRIPTOR_HANDLE handle;
	texture->impl.srvDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(texture->impl.srvDescriptorHeap, &handle);

	device->lpVtbl->CreateShaderResourceView(device, texture->impl.image, &shaderResourceViewDesc, handle);
}

void kinc_g5_texture_init(struct kinc_g5_texture *texture, int width, int height, kinc_image_format_t format) {
	create_texture(texture, width, height, format, D3D12_RESOURCE_FLAG_NONE);
}

void kinc_g5_texture_init_non_sampled_access(struct kinc_g5_texture *texture, int width, int height, kinc_image_format_t format) {
	create_texture(texture, width, height, format, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
}

void kinc_g5_texture_destroy(struct kinc_g5_texture *texture) {
	texture->impl.image->lpVtbl->Release(texture->impl.image);
	texture->impl.uploadImage->lpVtbl->Release(texture->impl.uploadImage);
	texture->impl.srvDescriptorHeap->lpVtbl->Release(texture->impl.srvDescriptorHeap);
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
	texture->impl.uploadImage->lpVtbl->Map(texture->impl.uploadImage, 0, NULL, (void **)&pixel);
	return pixel;
}

void kinc_g5_texture_unlock(struct kinc_g5_texture *texture) {
	texture->impl.uploadImage->lpVtbl->Unmap(texture->impl.uploadImage, 0, NULL);
}

int kinc_g5_texture_stride(struct kinc_g5_texture *texture) {
	return texture->impl.stride;
}

void kinc_g5_texture_generate_mipmaps(struct kinc_g5_texture *texture, int levels) {}

void kinc_g5_texture_set_mipmap(struct kinc_g5_texture *texture, kinc_image_t *mipmap, int level) {}
