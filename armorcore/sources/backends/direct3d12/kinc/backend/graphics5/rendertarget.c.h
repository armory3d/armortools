#include "rendertarget.h"

#include <kinc/graphics5/rendertarget.h>
#include <kinc/graphics5/texture.h>
#include <kinc/backend/SystemMicrosoft.h>

#ifdef KINC_WINDOWS
#include <dxgi1_4.h>
#endif

#ifdef KINC_DIRECT3D_HAS_NO_SWAPCHAIN
extern ID3D12Resource *swapChainRenderTargets[QUEUE_SLOT_COUNT];
#endif

static void WaitForFence(ID3D12Fence *fence, UINT64 completionValue, HANDLE waitEvent) {
	if (fence->GetCompletedValue() < completionValue) {
		fence->SetEventOnCompletion(completionValue, waitEvent);
		WaitForSingleObject(waitEvent, INFINITE);
	}
}

static void createRenderTargetView(ID3D12Resource *renderTarget, ID3D12DescriptorHeap *renderTargetDescriptorHeap, DXGI_FORMAT format) {
	// const D3D12_RESOURCE_DESC resourceDesc = renderTarget->lpVtbl->GetDesc(renderTarget);

	D3D12_RENDER_TARGET_VIEW_DESC viewDesc;
	viewDesc.Format = format;
	viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	viewDesc.Texture2D.MipSlice = 0;
	viewDesc.Texture2D.PlaneSlice = 0;

	device->CreateRenderTargetView(renderTarget, &viewDesc, renderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

static DXGI_FORMAT convertFormat(kinc_g5_render_target_format_t format) {
	switch (format) {
	case KINC_G5_RENDER_TARGET_FORMAT_128BIT_FLOAT:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case KINC_G5_RENDER_TARGET_FORMAT_64BIT_FLOAT:
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case KINC_G5_RENDER_TARGET_FORMAT_32BIT_RED_FLOAT:
		return DXGI_FORMAT_R32_FLOAT;
	case KINC_G5_RENDER_TARGET_FORMAT_16BIT_RED_FLOAT:
		return DXGI_FORMAT_R16_FLOAT;
	case KINC_G5_RENDER_TARGET_FORMAT_8BIT_RED:
		return DXGI_FORMAT_R8_UNORM;
	case KINC_G5_RENDER_TARGET_FORMAT_32BIT:
	default:
#ifdef KINC_WINDOWS
		return DXGI_FORMAT_R8G8B8A8_UNORM;
#else
		return DXGI_FORMAT_B8G8R8A8_UNORM;
#endif
	}
}

extern "C" void kinc_memory_emergency();

static void render_target_init(kinc_g5_render_target_t *render_target, int width, int height, kinc_g5_render_target_format_t format, int depthBufferBits,
                               int stencilBufferBits, int samples_per_pixel, int framebuffer_index) {
	render_target->texWidth = render_target->width = width;
	render_target->texHeight = render_target->height = height;
	render_target->impl.stage = 0;
	render_target->impl.stage_depth = -1;
	render_target->impl.renderTargetReadback = NULL;
	render_target->impl.framebuffer_index = framebuffer_index;

	DXGI_FORMAT dxgiFormat = convertFormat(format);

	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = dxgiFormat;
	clearValue.Color[0] = 0.0f;
	clearValue.Color[1] = 0.0f;
	clearValue.Color[2] = 0.0f;
	clearValue.Color[3] = 1.0f;

	D3D12_HEAP_PROPERTIES heapProperties;
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 1;
	heapProperties.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC texResourceDesc;
	texResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texResourceDesc.Alignment = 0;
	texResourceDesc.Width = render_target->texWidth;
	texResourceDesc.Height = render_target->texHeight;
	texResourceDesc.DepthOrArraySize = 1;
	texResourceDesc.MipLevels = 1;
	texResourceDesc.Format = dxgiFormat;
	texResourceDesc.SampleDesc.Count = 1;
	texResourceDesc.SampleDesc.Quality = 0;
	texResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	kinc_microsoft_affirm(device->CreateDescriptorHeap(&heapDesc, IID_GRAPHICS_PPV_ARGS(&render_target->impl.renderTargetDescriptorHeap)));

	if (framebuffer_index >= 0) {
#ifdef KINC_DIRECT3D_HAS_NO_SWAPCHAIN
		render_target->impl.renderTarget = swapChainRenderTargets[framebuffer_index];
#else
		IDXGISwapChain *swapChain = kinc_dx_current_window()->swapChain;
		kinc_microsoft_affirm(swapChain->GetBuffer(framebuffer_index, IID_PPV_ARGS(&render_target->impl.renderTarget)));
		wchar_t buffer[128];
		wsprintf(buffer, L"Backbuffer (index %i)", framebuffer_index);
		render_target->impl.renderTarget->SetName(buffer);
#endif
		createRenderTargetView(render_target->impl.renderTarget, render_target->impl.renderTargetDescriptorHeap, dxgiFormat);
	}
	else {
		HRESULT result = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &texResourceDesc, D3D12_RESOURCE_STATE_RENDER_TARGET,
		                                                 &clearValue, IID_GRAPHICS_PPV_ARGS(&render_target->impl.renderTarget));
		if (result != S_OK) {
			for (int i = 0; i < 10; ++i) {
				kinc_memory_emergency();
				result = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &texResourceDesc, D3D12_RESOURCE_STATE_RENDER_TARGET,
				                                         &clearValue, IID_GRAPHICS_PPV_ARGS(&render_target->impl.renderTarget));
				if (result == S_OK) {
					break;
				}
			}
		}

		D3D12_RENDER_TARGET_VIEW_DESC view;
		// const D3D12_RESOURCE_DESC resourceDesc = render_target->impl.renderTarget->lpVtbl->GetDesc(render_target->impl.renderTarget);
		view.Format = dxgiFormat;
		view.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		view.Texture2D.MipSlice = 0;
		view.Texture2D.PlaneSlice = 0;

		device->CreateRenderTargetView(render_target->impl.renderTarget, &view,
		                               render_target->impl.renderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	}

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
	descriptorHeapDesc.NumDescriptors = 1;

	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorHeapDesc.NodeMask = 0;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	kinc_microsoft_affirm(device->CreateDescriptorHeap(&descriptorHeapDesc, IID_GRAPHICS_PPV_ARGS(&render_target->impl.srvDescriptorHeap)));

	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shaderResourceViewDesc.Format = dxgiFormat;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	device->CreateShaderResourceView(render_target->impl.renderTarget, &shaderResourceViewDesc,
	                                 render_target->impl.srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	if (depthBufferBits > 0) {
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		kinc_microsoft_affirm(device->CreateDescriptorHeap(&dsvHeapDesc, IID_GRAPHICS_PPV_ARGS(&render_target->impl.depthStencilDescriptorHeap)));

		D3D12_RESOURCE_DESC depthTexture;
		depthTexture.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depthTexture.Alignment = 0;
		depthTexture.Width = width;
		depthTexture.Height = height;
		depthTexture.DepthOrArraySize = 1;
		depthTexture.MipLevels = 1;
		depthTexture.Format = DXGI_FORMAT_D32_FLOAT;
		depthTexture.SampleDesc.Count = 1;
		depthTexture.SampleDesc.Quality = 0;
		depthTexture.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		depthTexture.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE clearValue;
		clearValue.Format = DXGI_FORMAT_D32_FLOAT;
		clearValue.DepthStencil.Depth = 1.0f;
		clearValue.DepthStencil.Stencil = 0;

		D3D12_HEAP_PROPERTIES heapProperties;
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask = 1;
		heapProperties.VisibleNodeMask = 1;

		HRESULT result = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &depthTexture, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue,
		                                                 IID_GRAPHICS_PPV_ARGS(&render_target->impl.depthStencilTexture));
		if (result != S_OK) {
			for (int i = 0; i < 10; ++i) {
				kinc_memory_emergency();
				result = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &depthTexture, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue,
				                                         IID_GRAPHICS_PPV_ARGS(&render_target->impl.depthStencilTexture));
				if (result == S_OK) {
					break;
				}
			}
		}

		device->CreateDepthStencilView(render_target->impl.depthStencilTexture, NULL,
		                               render_target->impl.depthStencilDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

		// Reading depth texture as a shader resource
		D3D12_DESCRIPTOR_HEAP_DESC srvDepthHeapDesc = {};
		srvDepthHeapDesc.NumDescriptors = 1;
		srvDepthHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvDepthHeapDesc.NodeMask = 0;
		srvDepthHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		kinc_microsoft_affirm(device->CreateDescriptorHeap(&srvDepthHeapDesc, IID_GRAPHICS_PPV_ARGS(&render_target->impl.srvDepthDescriptorHeap)));

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDepthViewDesc;
		ZeroMemory(&srvDepthViewDesc, sizeof(srvDepthViewDesc));
		srvDepthViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDepthViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDepthViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDepthViewDesc.Texture2D.MipLevels = 1;
		srvDepthViewDesc.Texture2D.MostDetailedMip = 0;
		srvDepthViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		device->CreateShaderResourceView(render_target->impl.depthStencilTexture, &srvDepthViewDesc,
		                                 render_target->impl.srvDepthDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	}
	else {
		render_target->impl.depthStencilDescriptorHeap = NULL;
		render_target->impl.depthStencilTexture = NULL;
		render_target->impl.srvDepthDescriptorHeap = NULL;
	}

	render_target->impl.scissor.top = 0;
	render_target->impl.scissor.left = 0;
	render_target->impl.scissor.right = width;
	render_target->impl.scissor.bottom = height;

	render_target->impl.viewport.TopLeftX = 0.0f;
	render_target->impl.viewport.TopLeftY = 0.0f;
	render_target->impl.viewport.Width = (float)width;
	render_target->impl.viewport.Height = (float)height;
	render_target->impl.viewport.MinDepth = 0.0f;
	render_target->impl.viewport.MaxDepth = 1.0f;
}

void kinc_g5_render_target_init_with_multisampling(kinc_g5_render_target_t *target, int width, int height, kinc_g5_render_target_format_t format,
                                                   int depthBufferBits, int stencilBufferBits, int samples_per_pixel) {
	render_target_init(target, width, height, format, depthBufferBits, stencilBufferBits, samples_per_pixel, -1);
}

static int framebuffer_count = 0;

void kinc_g5_render_target_init_framebuffer_with_multisampling(kinc_g5_render_target_t *target, int width, int height, kinc_g5_render_target_format_t format,
                                                               int depthBufferBits, int stencilBufferBits, int samples_per_pixel) {
	render_target_init(target, width, height, format, depthBufferBits, stencilBufferBits, samples_per_pixel, framebuffer_count);
	framebuffer_count += 1;
}

void kinc_g5_render_target_init_cube_with_multisampling(kinc_g5_render_target_t *render_target, int cubeMapSize, kinc_g5_render_target_format_t format,
                                                        int depthBufferBits, int stencilBufferBits, int samples_per_pixel) {
	render_target->impl.stage = 0;
	render_target->impl.stage_depth = -1;
}

void kinc_g5_render_target_destroy(kinc_g5_render_target_t *render_target) {
	if (render_target->impl.framebuffer_index >= 0) {
		framebuffer_count -= 1;
	}

	render_target->impl.renderTarget->Release();
	render_target->impl.renderTargetDescriptorHeap->Release();
	render_target->impl.srvDescriptorHeap->Release();
	if (render_target->impl.depthStencilTexture != NULL) {
		render_target->impl.depthStencilTexture->Release();
		render_target->impl.depthStencilDescriptorHeap->Release();
		render_target->impl.srvDepthDescriptorHeap->Release();
	}
	if (render_target->impl.renderTargetReadback != NULL) {
		render_target->impl.renderTargetReadback->Release();
	}
}

void kinc_g5_render_target_set_depth_stencil_from(kinc_g5_render_target_t *render_target, kinc_g5_render_target_t *source) {
	render_target->impl.depthStencilDescriptorHeap = source->impl.depthStencilDescriptorHeap;
	render_target->impl.srvDepthDescriptorHeap = source->impl.srvDepthDescriptorHeap;
	render_target->impl.depthStencilTexture = source->impl.depthStencilTexture;
}
