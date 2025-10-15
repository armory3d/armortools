#define WIN32_LEAN_AND_MEAN
#include <assert.h>
#include <backends/windows_system.h>
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <iron_global.h>
#include <iron_gpu.h>
#include <iron_math.h>
#include <iron_system.h>
#include <malloc.h>
#include <math.h>
#include <stdbool.h>

bool                                gpu_transpose_mat = false;
static ID3D12Device                *device            = NULL;
static ID3D12CommandQueue          *queue;
static IDXGISwapChain              *window_swapchain;
static ID3D12RootSignature         *root_signature = NULL;
static ID3D12CommandAllocator      *command_allocator;
static ID3D12GraphicsCommandList   *command_list;
static D3D12_VIEWPORT               current_viewport;
static D3D12_RECT                   current_scissor;
static gpu_buffer_t                *current_vb;
static gpu_buffer_t                *current_ib;
static D3D12_CPU_DESCRIPTOR_HANDLE  target_descriptors[GPU_MAX_TEXTURES];
static D3D12_CPU_DESCRIPTOR_HANDLE  depth_handle;
static D3D12_CPU_DESCRIPTOR_HANDLE *current_depth_handle;
static bool                         window_vsync;
static ID3D12DescriptorHeap        *sampler_heap;
static ID3D12DescriptorHeap        *srv_heap;
static int                          srv_heap_index = 0;
static UINT64                       fence_value;
static ID3D12Fence                 *fence;
static HANDLE                       fence_event;
static UINT64                       frame_fence_values[GPU_FRAMEBUFFER_COUNT] = {0, 0};
static bool                         resized                                   = false;
static ID3D12Resource              *readback_buffer                           = NULL;
static int                          readback_buffer_size                      = 0;
static ID3D12Resource              *upload_buffer                             = NULL;
static int                          upload_buffer_size                        = 0;
static ID3D12Resource              *resources_to_destroy[512];
static int                          resources_to_destroy_count = 0;
static char                         device_name[256];

static D3D12_BLEND convert_blend_factor(gpu_blending_factor_t factor) {
	switch (factor) {
	case GPU_BLEND_ONE:
		return D3D12_BLEND_ONE;
	case GPU_BLEND_ZERO:
		return D3D12_BLEND_ZERO;
	case GPU_BLEND_SOURCE_ALPHA:
		return D3D12_BLEND_SRC_ALPHA;
	case GPU_BLEND_DEST_ALPHA:
		return D3D12_BLEND_DEST_ALPHA;
	case GPU_BLEND_INV_SOURCE_ALPHA:
		return D3D12_BLEND_INV_SRC_ALPHA;
	case GPU_BLEND_INV_DEST_ALPHA:
		return D3D12_BLEND_INV_DEST_ALPHA;
	}
}

static D3D12_CULL_MODE convert_cull_mode(gpu_cull_mode_t cull_mode) {
	switch (cull_mode) {
	case GPU_CULL_MODE_CLOCKWISE:
		return D3D12_CULL_MODE_FRONT;
	case GPU_CULL_MODE_COUNTERCLOCKWISE:
		return D3D12_CULL_MODE_BACK;
	default:
		return D3D12_CULL_MODE_NONE;
	}
}

static D3D12_COMPARISON_FUNC convert_compare_mode(gpu_compare_mode_t compare) {
	switch (compare) {
	default:
	case GPU_COMPARE_MODE_ALWAYS:
		return D3D12_COMPARISON_FUNC_ALWAYS;
	case GPU_COMPARE_MODE_NEVER:
		return D3D12_COMPARISON_FUNC_NEVER;
	case GPU_COMPARE_MODE_EQUAL:
		return D3D12_COMPARISON_FUNC_EQUAL;
	case GPU_COMPARE_MODE_LESS:
		return D3D12_COMPARISON_FUNC_LESS;
	}
}

static DXGI_FORMAT convert_format(gpu_texture_format_t format) {
	switch (format) {
	case GPU_TEXTURE_FORMAT_RGBA128:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case GPU_TEXTURE_FORMAT_RGBA64:
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case GPU_TEXTURE_FORMAT_R32:
		return DXGI_FORMAT_R32_FLOAT;
	case GPU_TEXTURE_FORMAT_R16:
		return DXGI_FORMAT_R16_FLOAT;
	case GPU_TEXTURE_FORMAT_R8:
		return DXGI_FORMAT_R8_UNORM;
	case GPU_TEXTURE_FORMAT_D32:
		return DXGI_FORMAT_D32_FLOAT;
	case GPU_TEXTURE_FORMAT_RGBA32:
	default:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	}
}

static D3D12_RESOURCE_STATES convert_texture_state(gpu_texture_state_t state) {
	switch (state) {
	case GPU_TEXTURE_STATE_SHADER_RESOURCE:
		return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	case GPU_TEXTURE_STATE_RENDER_TARGET:
		return D3D12_RESOURCE_STATE_RENDER_TARGET;
	case GPU_TEXTURE_STATE_RENDER_TARGET_DEPTH:
		return D3D12_RESOURCE_STATE_DEPTH_WRITE;
	case GPU_TEXTURE_STATE_PRESENT:
		return D3D12_RESOURCE_STATE_PRESENT;
	}
}

static void wait_for_fence(ID3D12Fence *fence, UINT64 completion_value, HANDLE wait_event) {
	if (fence->lpVtbl->GetCompletedValue(fence) < completion_value) {
		fence->lpVtbl->SetEventOnCompletion(fence, completion_value, wait_event);
		WaitForSingleObject(wait_event, INFINITE);
	}
}

static void _gpu_barrier(ID3D12Resource *r, D3D12_RESOURCE_STATES state_before, D3D12_RESOURCE_STATES state_after) {
	D3D12_RESOURCE_BARRIER barrier = {
	    .Transition.pResource   = r,
	    .Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
	    .Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE,
	    .Transition.StateBefore = state_before,
	    .Transition.StateAfter  = state_after,
	    .Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
	};
	command_list->lpVtbl->ResourceBarrier(command_list, 1, &barrier);
}

void gpu_barrier(gpu_texture_t *render_target, gpu_texture_state_t state_after) {
	if (render_target->state == state_after) {
		return;
	}
	_gpu_barrier(render_target->impl.image, convert_texture_state(render_target->state), convert_texture_state(state_after));
	render_target->state = state_after;
}

void gpu_destroy() {
	wait_for_fence(fence, fence_value, fence_event);
	for (int i = 0; i < GPU_FRAMEBUFFER_COUNT; ++i) {
		gpu_texture_destroy_internal(&framebuffers[i]);
	}
	if (framebuffer_depth.width > 0) {
		gpu_texture_destroy_internal(&framebuffer_depth);
	}
	if (readback_buffer != NULL) {
		readback_buffer->lpVtbl->Release(readback_buffer);
	}
	if (upload_buffer != NULL) {
		upload_buffer->lpVtbl->Release(upload_buffer);
	}
	command_list->lpVtbl->Release(command_list);
	command_allocator->lpVtbl->Release(command_allocator);
	window_swapchain->lpVtbl->Release(window_swapchain);
	queue->lpVtbl->Release(queue);
	root_signature->lpVtbl->Release(root_signature);
	srv_heap->lpVtbl->Release(srv_heap);
	fence->lpVtbl->Release(fence);
	CloseHandle(fence_event);
	device->lpVtbl->Release(device);
}

void gpu_render_target_init2(gpu_texture_t *render_target, int width, int height, gpu_texture_format_t format, int framebuffer_index) {
	render_target->width  = width;
	render_target->height = height;
	render_target->format = format;
	render_target->state  = (framebuffer_index >= 0) ? GPU_TEXTURE_STATE_PRESENT : GPU_TEXTURE_STATE_SHADER_RESOURCE;
	render_target->buffer = NULL;

	DXGI_FORMAT dxgi_format = convert_format(format);

	D3D12_CLEAR_VALUE clear_value;
	clear_value.Format             = dxgi_format;
	clear_value.Color[0]           = 0.0f;
	clear_value.Color[1]           = 0.0f;
	clear_value.Color[2]           = 0.0f;
	clear_value.Color[3]           = 0.0f;
	clear_value.DepthStencil.Depth = 1.0f;

	D3D12_HEAP_PROPERTIES heap_properties = {
	    .Type                 = D3D12_HEAP_TYPE_DEFAULT,
	    .CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
	    .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
	    .CreationNodeMask     = 1,
	    .VisibleNodeMask      = 1,
	};

	D3D12_RESOURCE_DESC resource_desc = {
	    .Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
	    .Alignment          = 0,
	    .Width              = width,
	    .Height             = height,
	    .DepthOrArraySize   = 1,
	    .MipLevels          = 1,
	    .Format             = dxgi_format,
	    .SampleDesc.Count   = 1,
	    .SampleDesc.Quality = 0,
	    .Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN,
	    .Flags              = format == GPU_TEXTURE_FORMAT_D32 ? D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL : D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
	};

	D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {
	    .NumDescriptors = 1,
	    .Type           = format == GPU_TEXTURE_FORMAT_D32 ? D3D12_DESCRIPTOR_HEAP_TYPE_DSV : D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
	    .Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
	};
	device->lpVtbl->CreateDescriptorHeap(device, &heap_desc, &IID_ID3D12DescriptorHeap, &render_target->impl.rtv_descriptor_heap);

	if (framebuffer_index >= 0) {
		window_swapchain->lpVtbl->GetBuffer(window_swapchain, framebuffer_index, &IID_ID3D12Resource, &render_target->impl.image);
	}
	else {
		device->lpVtbl->CreateCommittedResource(device, &heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		                                        &clear_value, &IID_ID3D12Resource, &render_target->impl.image);
	}

	D3D12_RENDER_TARGET_VIEW_DESC view_desc = {
	    .Format               = dxgi_format,
	    .ViewDimension        = D3D12_RTV_DIMENSION_TEXTURE2D,
	    .Texture2D.MipSlice   = 0,
	    .Texture2D.PlaneSlice = 0,
	};

	D3D12_CPU_DESCRIPTOR_HANDLE handle;
	render_target->impl.rtv_descriptor_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(render_target->impl.rtv_descriptor_heap, &handle);

	if (format == GPU_TEXTURE_FORMAT_D32) {
		device->lpVtbl->CreateDepthStencilView(device, render_target->impl.image, NULL, handle);
	}
	else {
		device->lpVtbl->CreateRenderTargetView(device, render_target->impl.image, &view_desc, handle);
	}

	D3D12_DESCRIPTOR_HEAP_DESC descriptor_heap_desc = {
	    .NumDescriptors = 1,
	    .Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
	    .NodeMask       = 0,
	    .Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
	};

	device->lpVtbl->CreateDescriptorHeap(device, &descriptor_heap_desc, &IID_ID3D12DescriptorHeap, &render_target->impl.srv_descriptor_heap);

	D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {
	    .ViewDimension                 = D3D12_SRV_DIMENSION_TEXTURE2D,
	    .Shader4ComponentMapping       = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
	    .Format                        = format == GPU_TEXTURE_FORMAT_D32 ? DXGI_FORMAT_R32_FLOAT : dxgi_format,
	    .Texture2D.MipLevels           = 1,
	    .Texture2D.MostDetailedMip     = 0,
	    .Texture2D.ResourceMinLODClamp = 0.0f,
	};

	render_target->impl.srv_descriptor_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(render_target->impl.srv_descriptor_heap, &handle);
	device->lpVtbl->CreateShaderResourceView(device, render_target->impl.image, &srv_desc, handle);
}

void create_root_signature(bool linear_sampling) {
	ID3DBlob              *root_blob;
	ID3DBlob              *error_blob;
	D3D12_ROOT_PARAMETER   parameters[3] = {0};
	D3D12_DESCRIPTOR_RANGE range         = {
	            .RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
	            .NumDescriptors                    = (UINT)GPU_MAX_TEXTURES,
	            .BaseShaderRegister                = 0,
	            .RegisterSpace                     = 0,
	            .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND,
    };
	parameters[0].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	parameters[0].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_ALL;
	parameters[0].DescriptorTable.NumDescriptorRanges = 1;
	parameters[0].DescriptorTable.pDescriptorRanges   = &range;
	parameters[1].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_CBV;
	parameters[1].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_ALL;
	parameters[1].Descriptor.ShaderRegister           = 0;
	parameters[1].Descriptor.RegisterSpace            = 0;
	D3D12_DESCRIPTOR_RANGE sampler_range              = {
	                 .RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER,
	                 .NumDescriptors                    = 1,
	                 .BaseShaderRegister                = 0,
	                 .RegisterSpace                     = 0,
	                 .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND,
    };
	parameters[2].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	parameters[2].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_ALL;
	parameters[2].DescriptorTable.NumDescriptorRanges = 1;
	parameters[2].DescriptorTable.pDescriptorRanges   = &sampler_range;

	D3D12_ROOT_SIGNATURE_DESC root_signature_desc = {
	    .NumParameters = 3,
	    .pParameters   = parameters,
	    .Flags         = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT,
	};
	D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &root_blob, &error_blob);
	device->lpVtbl->CreateRootSignature(device, 0, root_blob->lpVtbl->GetBufferPointer(root_blob), root_blob->lpVtbl->GetBufferSize(root_blob),
	                                    &IID_ID3D12RootSignature, &root_signature);
}

void gpu_init_internal(int depth_buffer_bits, bool vsync) {
	window_vsync = vsync;
#ifdef _DEBUG
	ID3D12Debug *debug_controller = NULL;
	if (D3D12GetDebugInterface(&IID_ID3D12Debug, &debug_controller) == S_OK) {
		debug_controller->lpVtbl->EnableDebugLayer(debug_controller);
	}
#endif

	D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_11_0, &IID_ID3D12Device, &device);
	create_root_signature(true);

	D3D12_COMMAND_QUEUE_DESC queue_desc = {
	    .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
	    .Type  = D3D12_COMMAND_LIST_TYPE_DIRECT,
	};
	device->lpVtbl->CreateCommandQueue(device, &queue_desc, &IID_ID3D12CommandQueue, &queue);

	HWND                 hwnd           = iron_windows_window_handle();
	DXGI_SWAP_CHAIN_DESC swapchain_desc = {
	    .BufferCount       = GPU_FRAMEBUFFER_COUNT,
	    .BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
	    .BufferUsage       = DXGI_USAGE_RENDER_TARGET_OUTPUT,
	    .BufferDesc.Width  = iron_window_width(),
	    .BufferDesc.Height = iron_window_height(),
	    .OutputWindow      = hwnd,
	    .SampleDesc.Count  = 1,
	    .SwapEffect        = DXGI_SWAP_EFFECT_FLIP_DISCARD,
	    .Windowed          = true,
	};

	IDXGIFactory4 *dxgi_factory = NULL;
	CreateDXGIFactory1(&IID_IDXGIFactory4, &dxgi_factory);
	dxgi_factory->lpVtbl->CreateSwapChain(dxgi_factory, (IUnknown *)queue, &swapchain_desc, &window_swapchain);

	fence_value = 0;
	fence_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	device->lpVtbl->CreateFence(device, 0, D3D12_FENCE_FLAG_NONE, &IID_ID3D12Fence, &fence);

	gpu_create_framebuffers(depth_buffer_bits);

	D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {
	    .NumDescriptors = GPU_CONSTANT_BUFFER_MULTIPLE,
	    .Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
	    .Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
	};
	device->lpVtbl->CreateDescriptorHeap(device, &heap_desc, &IID_ID3D12DescriptorHeap, &srv_heap);

	D3D12_DESCRIPTOR_HEAP_DESC sampler_heap_desc = {
	    .NumDescriptors = 1,
	    .Type           = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
	    .Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
	};
	device->lpVtbl->CreateDescriptorHeap(device, &sampler_heap_desc, &IID_ID3D12DescriptorHeap, &sampler_heap);
	gpu_use_linear_sampling(true);

	device->lpVtbl->CreateCommandAllocator(device, D3D12_COMMAND_LIST_TYPE_DIRECT, &IID_ID3D12CommandAllocator, &command_allocator);
	device->lpVtbl->CreateCommandList(device, 0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator, NULL, &IID_ID3D12CommandList, &command_list);
}

void gpu_begin_internal(gpu_clear_t flags, unsigned color, float depth) {
	for (int i = 0; i < current_render_targets_count; ++i) {
		current_render_targets[i]->impl.rtv_descriptor_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(current_render_targets[i]->impl.rtv_descriptor_heap,
		                                                                                                &target_descriptors[i]);
	}

	if (current_depth_buffer != NULL) {
		current_depth_buffer->impl.rtv_descriptor_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(current_depth_buffer->impl.rtv_descriptor_heap,
		                                                                                           &depth_handle);
		current_depth_handle = &depth_handle;
	}
	else {
		current_depth_handle = NULL;
	}

	command_list->lpVtbl->OMSetRenderTargets(command_list, current_render_targets_count, &target_descriptors[0], false, current_depth_handle);

	gpu_texture_t *target = current_render_targets[0];
	gpu_viewport(0, 0, target->width, target->height);
	gpu_scissor(0, 0, target->width, target->height);

	if (flags & GPU_CLEAR_COLOR) {
		float clear_color[] = {((color & 0x00ff0000) >> 16) / 255.0f, ((color & 0x0000ff00) >> 8) / 255.0f, (color & 0x000000ff) / 255.0f,
		                       ((color & 0xff000000) >> 24) / 255.0f};

		D3D12_CPU_DESCRIPTOR_HANDLE handle;
		target->impl.rtv_descriptor_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(target->impl.rtv_descriptor_heap, &handle);
		command_list->lpVtbl->ClearRenderTargetView(command_list, handle, clear_color, 0, NULL);
	}
	if (flags & GPU_CLEAR_DEPTH && current_depth_buffer != NULL) {
		D3D12_CPU_DESCRIPTOR_HANDLE handle;
		current_depth_buffer->impl.rtv_descriptor_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(current_depth_buffer->impl.rtv_descriptor_heap, &handle);
		command_list->lpVtbl->ClearDepthStencilView(command_list, handle, D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, NULL);
	}
}

void gpu_end_internal() {
	for (int i = 0; i < current_render_targets_count; ++i) {
		gpu_barrier(current_render_targets[i],
		            current_render_targets[i] == &framebuffers[framebuffer_index] ? GPU_TEXTURE_STATE_PRESENT : GPU_TEXTURE_STATE_SHADER_RESOURCE);
	}
	current_render_targets_count = 0;
}

void gpu_execute_and_wait() {
	command_list->lpVtbl->Close(command_list);
	ID3D12CommandList *command_lists[] = {(ID3D12CommandList *)command_list};
	queue->lpVtbl->ExecuteCommandLists(queue, 1, command_lists);
	queue->lpVtbl->Signal(queue, fence, ++fence_value);
	wait_for_fence(fence, fence_value, fence_event);
	command_allocator->lpVtbl->Reset(command_allocator);
	command_list->lpVtbl->Reset(command_list, command_allocator, NULL);

	if (gpu_in_use) {
		command_list->lpVtbl->OMSetRenderTargets(command_list, current_render_targets_count, &target_descriptors[0], false, current_depth_handle);
		command_list->lpVtbl->SetPipelineState(command_list, current_pipeline->impl.pipeline);
		command_list->lpVtbl->SetGraphicsRootSignature(command_list, root_signature);
		command_list->lpVtbl->IASetVertexBuffers(command_list, 0, 1, (D3D12_VERTEX_BUFFER_VIEW *)&current_vb->impl.vertex_buffer_view);
		command_list->lpVtbl->IASetIndexBuffer(command_list, (D3D12_INDEX_BUFFER_VIEW *)&current_ib->impl.index_buffer_view);
		command_list->lpVtbl->RSSetViewports(command_list, 1, &current_viewport);
		command_list->lpVtbl->RSSetScissorRects(command_list, 1, &current_scissor);
	}
}

void gpu_present_internal() {
	gpu_execute_and_wait();

	window_swapchain->lpVtbl->Present(window_swapchain, window_vsync, 0);
	queue->lpVtbl->Signal(queue, fence, ++fence_value);
	frame_fence_values[framebuffer_index] = fence_value;
	framebuffer_index                     = (framebuffer_index + 1) % GPU_FRAMEBUFFER_COUNT;
	wait_for_fence(fence, frame_fence_values[framebuffer_index], fence_event);

	if (resized) {
		framebuffer_index = 0;

		for (int i = 0; i < GPU_FRAMEBUFFER_COUNT; ++i) {
			gpu_texture_destroy_internal(&framebuffers[i]);
		}
		if (framebuffer_depth.width > 0) {
			gpu_texture_destroy_internal(&framebuffer_depth);
		}

		window_swapchain->lpVtbl->ResizeBuffers(window_swapchain, GPU_FRAMEBUFFER_COUNT, iron_window_width(), iron_window_height(), DXGI_FORMAT_R8G8B8A8_UNORM,
		                                        0);

		for (int i = 0; i < GPU_FRAMEBUFFER_COUNT; ++i) {
			gpu_render_target_init2(&framebuffers[i], iron_window_width(), iron_window_height(), GPU_TEXTURE_FORMAT_RGBA32, i);
		}
		if (framebuffer_depth.width > 0) {
			gpu_render_target_init2(&framebuffer_depth, iron_window_width(), iron_window_height(), GPU_TEXTURE_FORMAT_D32, -1);
		}

		resized = false;
	}

	while (resources_to_destroy_count > 0) {
		resources_to_destroy_count--;
		ID3D12Resource *r = resources_to_destroy[resources_to_destroy_count];
		r->lpVtbl->Release(r);
	}
}

void gpu_resize_internal(int width, int height) {
	if (fence_value == 0) {
		return;
	}
	resized = true;
}

bool gpu_raytrace_supported() {
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 options;
	if (device->lpVtbl->CheckFeatureSupport(device, D3D12_FEATURE_D3D12_OPTIONS5, &options, sizeof(options)) == S_OK) {
		return options.RaytracingTier >= D3D12_RAYTRACING_TIER_1_0;
	}
	return false;
}

void gpu_set_constant_buffer(gpu_buffer_t *buffer, int offset, size_t size) {
	command_list->lpVtbl->SetGraphicsRootConstantBufferView(command_list, 1, buffer->impl.buffer->lpVtbl->GetGPUVirtualAddress(buffer->impl.buffer) + offset);
}

void gpu_internal_set_textures() {
	UINT srv_step = device->lpVtbl->GetDescriptorHandleIncrementSize(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	if (srv_heap_index + GPU_MAX_TEXTURES > GPU_CONSTANT_BUFFER_MULTIPLE) {
		srv_heap_index = 0;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE cpu_base;
	D3D12_GPU_DESCRIPTOR_HANDLE gpu_base;
	srv_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(srv_heap, &cpu_base);
	srv_heap->lpVtbl->GetGPUDescriptorHandleForHeapStart(srv_heap, &gpu_base);
	cpu_base.ptr += srv_heap_index * srv_step;
	gpu_base.ptr += srv_heap_index * srv_step;

	for (int i = 0; i < GPU_MAX_TEXTURES; ++i) {
		if (current_textures[i] != NULL) {
			D3D12_CPU_DESCRIPTOR_HANDLE source_cpu;
			ID3D12DescriptorHeap       *source_heap = current_textures[i]->impl.srv_descriptor_heap;
			source_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(source_heap, &source_cpu);

			device->lpVtbl->CopyDescriptorsSimple(device, 1, cpu_base, source_cpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			cpu_base.ptr += srv_step;
			srv_heap_index++;
		}
	}

	ID3D12DescriptorHeap *heaps[] = {srv_heap, sampler_heap};
	command_list->lpVtbl->SetDescriptorHeaps(command_list, 2, heaps);
	command_list->lpVtbl->SetGraphicsRootDescriptorTable(command_list, 0, gpu_base);

	D3D12_GPU_DESCRIPTOR_HANDLE sampler_gpu_base;
	sampler_heap->lpVtbl->GetGPUDescriptorHandleForHeapStart(sampler_heap, &sampler_gpu_base);
	command_list->lpVtbl->SetGraphicsRootDescriptorTable(command_list, 2, sampler_gpu_base);
}

void gpu_draw_internal() {
	gpu_internal_set_textures();
	command_list->lpVtbl->IASetPrimitiveTopology(command_list, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	command_list->lpVtbl->DrawIndexedInstanced(command_list, current_ib->count, 1, 0, 0, 0);
}

void gpu_viewport(int x, int y, int width, int height) {
	current_viewport = (D3D12_VIEWPORT){
	    .TopLeftX = (float)x,
	    .TopLeftY = (float)y,
	    .Width    = (float)width,
	    .Height   = (float)height,
	    .MinDepth = 0.0f,
	    .MaxDepth = 1.0f,
	};
	command_list->lpVtbl->RSSetViewports(command_list, 1, &current_viewport);
}

void gpu_scissor(int x, int y, int width, int height) {
	current_scissor = (D3D12_RECT){
	    .left   = x,
	    .top    = y,
	    .right  = x + width,
	    .bottom = y + height,
	};
	command_list->lpVtbl->RSSetScissorRects(command_list, 1, &current_scissor);
}

void gpu_disable_scissor() {
	current_scissor = (D3D12_RECT){
	    .left   = 0,
	    .top    = 0,
	    .right  = current_render_targets[0]->width,
	    .bottom = current_render_targets[0]->height,
	};
	command_list->lpVtbl->RSSetScissorRects(command_list, 1, &current_scissor);
}

void gpu_set_pipeline_internal(gpu_pipeline_t *pipeline) {
	command_list->lpVtbl->SetPipelineState(command_list, pipeline->impl.pipeline);
	command_list->lpVtbl->SetGraphicsRootSignature(command_list, root_signature);
}

void gpu_set_vertex_buffer(gpu_buffer_t *buffer) {
	current_vb = buffer;
	command_list->lpVtbl->IASetVertexBuffers(command_list, 0, 1, (D3D12_VERTEX_BUFFER_VIEW *)&buffer->impl.vertex_buffer_view);
}

void gpu_set_index_buffer(gpu_buffer_t *buffer) {
	current_ib = buffer;
	command_list->lpVtbl->IASetIndexBuffer(command_list, (D3D12_INDEX_BUFFER_VIEW *)&buffer->impl.index_buffer_view);
}

void gpu_get_render_target_pixels(gpu_texture_t *render_target, uint8_t *data) {
	D3D12_RESOURCE_DESC desc;
	render_target->impl.image->lpVtbl->GetDesc(render_target->impl.image, &desc);
	DXGI_FORMAT dxgi_format     = desc.Format;
	int         format_size     = gpu_texture_format_size(render_target->format);
	int         packed_row_size = render_target->width * format_size;
	int         row_pitch       = packed_row_size;
	int         align           = row_pitch % D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
	if (align != 0) {
		row_pitch = row_pitch + (D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - align);
	}

	int new_readback_buffer_size = row_pitch * render_target->height;
	if (new_readback_buffer_size < (2048 * 2048 * 4)) {
		new_readback_buffer_size = (2048 * 2048 * 4);
	}
	if (readback_buffer_size < new_readback_buffer_size) {
		readback_buffer_size = new_readback_buffer_size;
		if (readback_buffer != NULL) {
			readback_buffer->lpVtbl->Release(readback_buffer);
		}
		D3D12_HEAP_PROPERTIES heap_properties = {
		    .Type                 = D3D12_HEAP_TYPE_READBACK,
		    .CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		    .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
		    .CreationNodeMask     = 1,
		    .VisibleNodeMask      = 1,
		};
		D3D12_RESOURCE_DESC resource_desc = {
		    .Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER,
		    .Alignment          = 0,
		    .Width              = readback_buffer_size,
		    .Height             = 1,
		    .DepthOrArraySize   = 1,
		    .MipLevels          = 1,
		    .Format             = DXGI_FORMAT_UNKNOWN,
		    .SampleDesc.Count   = 1,
		    .SampleDesc.Quality = 0,
		    .Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		    .Flags              = D3D12_RESOURCE_FLAG_NONE,
		};
		device->lpVtbl->CreateCommittedResource(device, &heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc, D3D12_RESOURCE_STATE_COPY_DEST, NULL,
		                                        &IID_ID3D12Resource, &readback_buffer);
	}

	// Copy render target to readback buffer
	D3D12_RESOURCE_BARRIER barrier = {
	    .Transition.pResource   = render_target->impl.image,
	    .Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
	    .Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE,
	    .Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
	    .Transition.StateAfter  = D3D12_RESOURCE_STATE_COPY_SOURCE,
	    .Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
	};
	command_list->lpVtbl->ResourceBarrier(command_list, 1, &barrier);

	D3D12_TEXTURE_COPY_LOCATION source = {
	    .pResource        = render_target->impl.image,
	    .Type             = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
	    .SubresourceIndex = 0,
	};
	D3D12_TEXTURE_COPY_LOCATION dest = {
	    .pResource                          = readback_buffer,
	    .Type                               = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
	    .PlacedFootprint.Offset             = 0,
	    .PlacedFootprint.Footprint.Format   = dxgi_format,
	    .PlacedFootprint.Footprint.Width    = render_target->width,
	    .PlacedFootprint.Footprint.Height   = render_target->height,
	    .PlacedFootprint.Footprint.Depth    = 1,
	    .PlacedFootprint.Footprint.RowPitch = row_pitch,
	};
	command_list->lpVtbl->CopyTextureRegion(command_list, &dest, 0, 0, 0, &source, NULL);

	barrier = (D3D12_RESOURCE_BARRIER){
	    .Transition.pResource   = render_target->impl.image,
	    .Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
	    .Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE,
	    .Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE,
	    .Transition.StateAfter  = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
	    .Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
	};
	command_list->lpVtbl->ResourceBarrier(command_list, 1, &barrier);

	gpu_execute_and_wait();

	void *p;
	readback_buffer->lpVtbl->Map(readback_buffer, 0, NULL, &p);
	if (packed_row_size == row_pitch) {
		memcpy(data, p, render_target->width * render_target->height * format_size);
	}
	else {
		uint8_t *src = (uint8_t *)p;
		uint8_t *dst = data;
		for (int y = 0; y < render_target->height; y++) {
			memcpy(dst, src, packed_row_size);
			src += row_pitch;
			dst += packed_row_size;
		}
	}
	readback_buffer->lpVtbl->Unmap(readback_buffer, 0, NULL);
}

void gpu_set_texture(int unit, gpu_texture_t *texture) {
	current_textures[unit] = texture;
}

void gpu_use_linear_sampling(bool b) {
	D3D12_SAMPLER_DESC sampler_desc = {
	    .Filter         = b ? D3D12_FILTER_MIN_MAG_MIP_LINEAR : D3D12_FILTER_MIN_MAG_MIP_POINT,
	    .AddressU       = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
	    .AddressV       = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
	    .AddressW       = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
	    .MipLODBias     = 0,
	    .MaxAnisotropy  = 16,
	    .ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
	    .BorderColor    = {0.0f, 0.0f, 0.0f, 0.0f},
	    .MinLOD         = 0.0f,
	    .MaxLOD         = D3D12_FLOAT32_MAX,
	};
	D3D12_CPU_DESCRIPTOR_HANDLE sampler_handle;
	sampler_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(sampler_heap, &sampler_handle);
	device->lpVtbl->CreateSampler(device, &sampler_desc, sampler_handle);
}

void gpu_pipeline_destroy_internal(gpu_pipeline_t *pipe) {
	if (pipe->impl.pipeline != NULL) {
		pipe->impl.pipeline->lpVtbl->Release(pipe->impl.pipeline);
		pipe->impl.pipeline = NULL;
	}
}

void gpu_pipeline_compile(gpu_pipeline_t *pipe) {
	int                       vertex_attribute_count = pipe->input_layout->size;
	D3D12_INPUT_ELEMENT_DESC *vertex_desc            = (D3D12_INPUT_ELEMENT_DESC *)alloca(sizeof(D3D12_INPUT_ELEMENT_DESC) * vertex_attribute_count);
	ZeroMemory(vertex_desc, sizeof(D3D12_INPUT_ELEMENT_DESC) * vertex_attribute_count);

	for (int i = 0; i < pipe->input_layout->size; ++i) {
		vertex_desc[i].SemanticName         = "TEXCOORD";
		vertex_desc[i].SemanticIndex        = i;
		vertex_desc[i].InputSlot            = 0;
		vertex_desc[i].AlignedByteOffset    = (i == 0) ? 0 : D3D12_APPEND_ALIGNED_ELEMENT;
		vertex_desc[i].InputSlotClass       = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		vertex_desc[i].InstanceDataStepRate = 0;
		switch (pipe->input_layout->elements[i].data) {
		case GPU_VERTEX_DATA_F32_1X:
			vertex_desc[i].Format = DXGI_FORMAT_R32_FLOAT;
			break;
		case GPU_VERTEX_DATA_F32_2X:
			vertex_desc[i].Format = DXGI_FORMAT_R32G32_FLOAT;
			break;
		case GPU_VERTEX_DATA_F32_3X:
			vertex_desc[i].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			break;
		case GPU_VERTEX_DATA_F32_4X:
			vertex_desc[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			break;
		case GPU_VERTEX_DATA_I16_2X_NORM:
			vertex_desc[i].Format = DXGI_FORMAT_R16G16_SNORM;
			break;
		case GPU_VERTEX_DATA_I16_4X_NORM:
			vertex_desc[i].Format = DXGI_FORMAT_R16G16B16A16_SNORM;
			break;
		default:
			break;
		}
	}

	const D3D12_DEPTH_STENCILOP_DESC default_stencil_op = {D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_NEVER};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {
	    .VS.BytecodeLength                     = pipe->vertex_shader->impl.length,
	    .VS.pShaderBytecode                    = pipe->vertex_shader->impl.data,
	    .PS.BytecodeLength                     = pipe->fragment_shader->impl.length,
	    .PS.pShaderBytecode                    = pipe->fragment_shader->impl.data,
	    .pRootSignature                        = root_signature,
	    .NumRenderTargets                      = pipe->color_attachment_count,
	    .DSVFormat                             = DXGI_FORMAT_UNKNOWN,
	    .InputLayout.NumElements               = vertex_attribute_count,
	    .InputLayout.pInputElementDescs        = vertex_desc,
	    .RasterizerState.FillMode              = D3D12_FILL_MODE_SOLID,
	    .RasterizerState.CullMode              = convert_cull_mode(pipe->cull_mode),
	    .RasterizerState.FrontCounterClockwise = FALSE,
	    .RasterizerState.DepthBias             = D3D12_DEFAULT_DEPTH_BIAS,
	    .RasterizerState.DepthBiasClamp        = D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
	    .RasterizerState.SlopeScaledDepthBias  = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
	    .RasterizerState.DepthClipEnable       = TRUE,
	    .RasterizerState.MultisampleEnable     = FALSE,
	    .RasterizerState.AntialiasedLineEnable = FALSE,
	    .RasterizerState.ForcedSampleCount     = 0,
	    .RasterizerState.ConservativeRaster    = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF,
	    .BlendState.AlphaToCoverageEnable      = FALSE,
	    .BlendState.IndependentBlendEnable     = FALSE,
	    .DepthStencilState.DepthEnable         = TRUE,
	    .DepthStencilState.DepthWriteMask      = D3D12_DEPTH_WRITE_MASK_ALL,
	    .DepthStencilState.DepthFunc           = D3D12_COMPARISON_FUNC_LESS,
	    .DepthStencilState.StencilEnable       = FALSE,
	    .DepthStencilState.StencilReadMask     = D3D12_DEFAULT_STENCIL_READ_MASK,
	    .DepthStencilState.StencilWriteMask    = D3D12_DEFAULT_STENCIL_WRITE_MASK,
	    .DepthStencilState.DepthEnable         = pipe->depth_mode != GPU_COMPARE_MODE_ALWAYS,
	    .DepthStencilState.DepthWriteMask      = pipe->depth_write ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO,
	    .DepthStencilState.DepthFunc           = convert_compare_mode(pipe->depth_mode),
	    .DepthStencilState.StencilEnable       = false,
	    .DSVFormat                             = DXGI_FORMAT_D32_FLOAT,
	    .SampleDesc.Count                      = 1,
	    .SampleMask                            = 0xFFFFFFFF,
	    .PrimitiveTopologyType                 = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
	    .DepthStencilState.FrontFace           = default_stencil_op,
	    .DepthStencilState.BackFace            = default_stencil_op,
	};

	for (int i = 0; i < pipe->color_attachment_count; ++i) {
		psoDesc.RTVFormats[i] = convert_format(pipe->color_attachment[i]);
	}

	psoDesc.BlendState.IndependentBlendEnable = true;
	for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i) {
		psoDesc.BlendState.RenderTarget[i].BlendEnable = pipe->blend_source != GPU_BLEND_ONE || pipe->blend_destination != GPU_BLEND_ZERO ||
		                                                 pipe->alpha_blend_source != GPU_BLEND_ONE || pipe->alpha_blend_destination != GPU_BLEND_ZERO;
		psoDesc.BlendState.RenderTarget[i].SrcBlend       = convert_blend_factor(pipe->blend_source);
		psoDesc.BlendState.RenderTarget[i].DestBlend      = convert_blend_factor(pipe->blend_destination);
		psoDesc.BlendState.RenderTarget[i].BlendOp        = D3D12_BLEND_OP_ADD;
		psoDesc.BlendState.RenderTarget[i].SrcBlendAlpha  = convert_blend_factor(pipe->alpha_blend_source);
		psoDesc.BlendState.RenderTarget[i].DestBlendAlpha = convert_blend_factor(pipe->alpha_blend_destination);
		psoDesc.BlendState.RenderTarget[i].BlendOpAlpha   = D3D12_BLEND_OP_ADD;
		psoDesc.BlendState.RenderTarget[i].RenderTargetWriteMask =
		    (((pipe->color_write_mask_red[i] ? D3D12_COLOR_WRITE_ENABLE_RED : 0) | (pipe->color_write_mask_green[i] ? D3D12_COLOR_WRITE_ENABLE_GREEN : 0)) |
		     (pipe->color_write_mask_blue[i] ? D3D12_COLOR_WRITE_ENABLE_BLUE : 0)) |
		    (pipe->color_write_mask_alpha[i] ? D3D12_COLOR_WRITE_ENABLE_ALPHA : 0);
	}

	device->lpVtbl->CreateGraphicsPipelineState(device, &psoDesc, &IID_ID3D12PipelineState, &pipe->impl.pipeline);
}

void gpu_shader_init(gpu_shader_t *shader, const void *_data, size_t length, gpu_shader_type_t type) {
	uint8_t *data       = (uint8_t *)_data;
	shader->impl.length = (int)length;
	shader->impl.data   = (uint8_t *)malloc(shader->impl.length);
	memcpy(shader->impl.data, data, shader->impl.length);
}

void gpu_shader_destroy(gpu_shader_t *shader) {
	free(shader->impl.data);
}

void gpu_texture_init_from_bytes(gpu_texture_t *texture, void *data, int width, int height, gpu_texture_format_t format) {
	texture->width                    = width;
	texture->height                   = height;
	texture->format                   = format;
	texture->state                    = GPU_TEXTURE_STATE_SHADER_RESOURCE;
	texture->buffer                   = NULL;
	texture->impl.rtv_descriptor_heap = NULL;
	DXGI_FORMAT dxgi_format           = convert_format(format);
	int         format_size           = gpu_texture_format_size(format);

	D3D12_HEAP_PROPERTIES heap_properties = {
	    .Type                 = D3D12_HEAP_TYPE_DEFAULT,
	    .CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
	    .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
	    .CreationNodeMask     = 1,
	    .VisibleNodeMask      = 1,
	};

	D3D12_RESOURCE_DESC resource_desc = {
	    .Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
	    .Alignment          = 0,
	    .Width              = texture->width,
	    .Height             = texture->height,
	    .DepthOrArraySize   = 1,
	    .MipLevels          = 1,
	    .Format             = dxgi_format,
	    .SampleDesc.Count   = 1,
	    .SampleDesc.Quality = 0,
	    .Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN,
	    .Flags              = D3D12_RESOURCE_FLAG_NONE,
	};

	device->lpVtbl->CreateCommittedResource(device, &heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, NULL,
	                                        &IID_ID3D12Resource, &texture->impl.image);

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
	UINT64                             upload_size;
	device->lpVtbl->GetCopyableFootprints(device, &resource_desc, 0, 1, 0, &footprint, NULL, NULL, &upload_size);

	int new_upload_buffer_size = upload_size;
	if (new_upload_buffer_size < (1024 * 1024 * 4)) {
		new_upload_buffer_size = (1024 * 1024 * 4);
	}
	if (upload_buffer_size < new_upload_buffer_size) {
		upload_buffer_size = new_upload_buffer_size;
		if (upload_buffer != NULL) {
			upload_buffer->lpVtbl->Release(upload_buffer);
		}

		D3D12_HEAP_PROPERTIES heap_properties_upload = {
		    .Type                 = D3D12_HEAP_TYPE_UPLOAD,
		    .CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		    .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
		    .CreationNodeMask     = 1,
		    .VisibleNodeMask      = 1,
		};

		D3D12_RESOURCE_DESC resource_desc_upload = {
		    .Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER,
		    .Alignment          = 0,
		    .Width              = upload_buffer_size,
		    .Height             = 1,
		    .DepthOrArraySize   = 1,
		    .MipLevels          = 1,
		    .Format             = DXGI_FORMAT_UNKNOWN,
		    .SampleDesc.Count   = 1,
		    .SampleDesc.Quality = 0,
		    .Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		    .Flags              = D3D12_RESOURCE_FLAG_NONE,
		};

		device->lpVtbl->CreateCommittedResource(device, &heap_properties_upload, D3D12_HEAP_FLAG_NONE, &resource_desc_upload, D3D12_RESOURCE_STATE_GENERIC_READ,
		                                        NULL, &IID_ID3D12Resource, &upload_buffer);
	}

	BYTE *pixel;
	upload_buffer->lpVtbl->Map(upload_buffer, 0, NULL, (void **)&pixel);
	UINT row_pitch = footprint.Footprint.RowPitch;
	for (int y = 0; y < texture->height; ++y) {
		memcpy(pixel + y * row_pitch, ((uint8_t *)data) + y * width * format_size, width * format_size);
	}
	upload_buffer->lpVtbl->Unmap(upload_buffer, 0, NULL);

	D3D12_DESCRIPTOR_HEAP_DESC descriptor_heap_desc = {
	    .NumDescriptors = 1,
	    .Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
	    .NodeMask       = 0,
	    .Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
	};
	device->lpVtbl->CreateDescriptorHeap(device, &descriptor_heap_desc, &IID_ID3D12DescriptorHeap, &texture->impl.srv_descriptor_heap);

	D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {
	    .ViewDimension                 = D3D12_SRV_DIMENSION_TEXTURE2D,
	    .Shader4ComponentMapping       = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
	    .Format                        = dxgi_format,
	    .Texture2D.MipLevels           = 1,
	    .Texture2D.MostDetailedMip     = 0,
	    .Texture2D.ResourceMinLODClamp = 0.0f,
	};

	D3D12_CPU_DESCRIPTOR_HANDLE handle;
	texture->impl.srv_descriptor_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(texture->impl.srv_descriptor_heap, &handle);
	device->lpVtbl->CreateShaderResourceView(device, texture->impl.image, &srv_desc, handle);

	D3D12_RESOURCE_BARRIER barrier = {
	    .Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
	    .Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE,
	    .Transition.pResource   = texture->impl.image,
	    .Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
	    .Transition.StateAfter  = D3D12_RESOURCE_STATE_COPY_DEST,
	    .Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
	};
	command_list->lpVtbl->ResourceBarrier(command_list, 1, &barrier);

	D3D12_TEXTURE_COPY_LOCATION source = {
	    .pResource       = upload_buffer,
	    .Type            = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
	    .PlacedFootprint = footprint,
	};

	D3D12_TEXTURE_COPY_LOCATION destination = {
	    .pResource        = texture->impl.image,
	    .Type             = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
	    .SubresourceIndex = 0,
	};

	command_list->lpVtbl->CopyTextureRegion(command_list, &destination, 0, 0, 0, &source, NULL);

	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST, barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
	command_list->lpVtbl->ResourceBarrier(command_list, 1, &barrier);

	gpu_execute_and_wait(); ////
}

void gpu_texture_destroy_internal(gpu_texture_t *render_target) {
	if (render_target->impl.image != NULL) {
		render_target->impl.image->lpVtbl->Release(render_target->impl.image);
	}
	if (render_target->impl.rtv_descriptor_heap != NULL) {
		render_target->impl.rtv_descriptor_heap->lpVtbl->Release(render_target->impl.rtv_descriptor_heap);
	}
	if (render_target->impl.srv_descriptor_heap != NULL) {
		render_target->impl.srv_descriptor_heap->lpVtbl->Release(render_target->impl.srv_descriptor_heap);
	}
}

void gpu_render_target_init(gpu_texture_t *target, int width, int height, gpu_texture_format_t format) {
	gpu_render_target_init2(target, width, height, format, -1);
}

void _gpu_buffer_init(ID3D12Resource **buffer, int size, D3D12_HEAP_TYPE heap_type) {
	if (*buffer != NULL) {
		assert(resources_to_destroy_count < 512);
		resources_to_destroy[resources_to_destroy_count] = *buffer;
		resources_to_destroy_count++;
	}
	D3D12_HEAP_PROPERTIES heap_properties = {
	    .Type                 = heap_type,
	    .CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
	    .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
	    .CreationNodeMask     = 1,
	    .VisibleNodeMask      = 1,
	};

	D3D12_RESOURCE_DESC resource_desc = {
	    .Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER,
	    .Alignment          = 0,
	    .Width              = size,
	    .Height             = 1,
	    .DepthOrArraySize   = 1,
	    .MipLevels          = 1,
	    .Format             = DXGI_FORMAT_UNKNOWN,
	    .SampleDesc.Count   = 1,
	    .SampleDesc.Quality = 0,
	    .Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
	    .Flags              = D3D12_RESOURCE_FLAG_NONE,
	};

	device->lpVtbl->CreateCommittedResource(device, &heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc,
	                                        heap_type == D3D12_HEAP_TYPE_UPLOAD ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_COMMON, NULL,
	                                        &IID_ID3D12Resource, buffer);
}

void gpu_vertex_buffer_init(gpu_buffer_t *buffer, int count, gpu_vertex_structure_t *structure) {
	buffer->count  = count;
	buffer->stride = 0;
	for (int i = 0; i < structure->size; ++i) {
		buffer->stride += gpu_vertex_data_size(structure->elements[i].data);
	}
	buffer->impl.vertex_buffer_view.SizeInBytes   = buffer->stride * buffer->count;
	buffer->impl.vertex_buffer_view.StrideInBytes = buffer->stride;
	buffer->impl.buffer                           = NULL;
}

void *gpu_vertex_buffer_lock(gpu_buffer_t *buffer) {
	_gpu_buffer_init(&buffer->impl.buffer, buffer->stride * buffer->count, D3D12_HEAP_TYPE_UPLOAD);

	D3D12_RANGE range = {
	    .Begin = 0,
	    .End   = buffer->count * buffer->stride,
	};
	void *p;
	buffer->impl.buffer->lpVtbl->Map(buffer->impl.buffer, 0, &range, &p);
	return p;
}

void gpu_vertex_buffer_unlock(gpu_buffer_t *buffer) {
	D3D12_RANGE range = {
	    .Begin = 0,
	    .End   = buffer->count * buffer->stride,
	};
	buffer->impl.buffer->lpVtbl->Unmap(buffer->impl.buffer, 0, &range);

	ID3D12Resource *upload_buffer = buffer->impl.buffer;
	_gpu_buffer_init(&buffer->impl.buffer, buffer->stride * buffer->count, D3D12_HEAP_TYPE_DEFAULT);
	_gpu_barrier(buffer->impl.buffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	command_list->lpVtbl->CopyBufferRegion(command_list, buffer->impl.buffer, 0, upload_buffer, 0, buffer->stride * buffer->count);
	_gpu_barrier(buffer->impl.buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	buffer->impl.vertex_buffer_view.BufferLocation = buffer->impl.buffer->lpVtbl->GetGPUVirtualAddress(buffer->impl.buffer);
}

void gpu_index_buffer_init(gpu_buffer_t *buffer, int count) {
	buffer->count                              = count;
	buffer->impl.index_buffer_view.SizeInBytes = count * 4;
	buffer->impl.index_buffer_view.Format      = DXGI_FORMAT_R32_UINT;
	buffer->impl.buffer                        = NULL;
}

void *gpu_index_buffer_lock(gpu_buffer_t *buffer) {
	_gpu_buffer_init(&buffer->impl.buffer, buffer->count * 4, D3D12_HEAP_TYPE_UPLOAD);

	D3D12_RANGE range = {
	    .Begin = 0,
	    .End   = buffer->count * 4,
	};
	void *p;
	buffer->impl.buffer->lpVtbl->Map(buffer->impl.buffer, 0, &range, &p);
	return p;
}

void gpu_index_buffer_unlock(gpu_buffer_t *buffer) {
	D3D12_RANGE range = {
	    .Begin = 0,
	    .End   = buffer->count * 4,
	};
	buffer->impl.buffer->lpVtbl->Unmap(buffer->impl.buffer, 0, &range);

	ID3D12Resource *upload_buffer = buffer->impl.buffer;
	_gpu_buffer_init(&buffer->impl.buffer, buffer->count * 4, D3D12_HEAP_TYPE_DEFAULT);
	_gpu_barrier(buffer->impl.buffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	command_list->lpVtbl->CopyBufferRegion(command_list, buffer->impl.buffer, 0, upload_buffer, 0, buffer->count * 4);
	_gpu_barrier(buffer->impl.buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	buffer->impl.index_buffer_view.BufferLocation = buffer->impl.buffer->lpVtbl->GetGPUVirtualAddress(buffer->impl.buffer);
}

void gpu_constant_buffer_init(gpu_buffer_t *buffer, int size) {
	buffer->count       = size;
	buffer->data        = NULL;
	buffer->impl.buffer = NULL;
	_gpu_buffer_init(&buffer->impl.buffer, size, D3D12_HEAP_TYPE_UPLOAD);
}

void gpu_constant_buffer_lock(gpu_buffer_t *buffer, int start, int count) {
	buffer->impl.last_start = start;
	buffer->impl.last_count = count;
	D3D12_RANGE range       = {
	          .Begin = start,
	          .End   = start + count,
    };
	uint8_t *p;
	buffer->impl.buffer->lpVtbl->Map(buffer->impl.buffer, 0, &range, (void **)&p);
	buffer->data = &p[start];
}

void gpu_constant_buffer_unlock(gpu_buffer_t *buffer) {
	D3D12_RANGE range = {
	    .Begin = buffer->impl.last_start,
	    .End   = buffer->impl.last_start + buffer->impl.last_count,
	};
	buffer->impl.buffer->lpVtbl->Unmap(buffer->impl.buffer, 0, &range);
	buffer->data = NULL;
}

void gpu_buffer_destroy_internal(gpu_buffer_t *buffer) {
	buffer->impl.buffer->lpVtbl->Release(buffer->impl.buffer);
	buffer->impl.buffer = NULL;
}

char *gpu_device_name() {
	IDXGIFactory *factory;
	CreateDXGIFactory(&IID_IDXGIFactory, (void **)&factory);
	IDXGIAdapter *adapter;
	factory->lpVtbl->EnumAdapters(factory, 0, &adapter);
	DXGI_ADAPTER_DESC desc;
	adapter->lpVtbl->GetDesc(adapter, &desc);
	WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1, device_name, sizeof(device_name), NULL, NULL);
	adapter->lpVtbl->Release(adapter);
	factory->lpVtbl->Release(factory);
	return device_name;
}

typedef struct inst {
	iron_matrix4x4_t m;
	int              i;
} inst_t;

static ID3D12Device5                         *dxr_device          = NULL;
static ID3D12GraphicsCommandList4            *dxr_command_list    = NULL;
static ID3D12RootSignature                   *dxr_root_signature  = NULL;
static ID3D12DescriptorHeap                  *dxr_descriptor_heap = NULL;
static gpu_raytrace_acceleration_structure_t *dxr_accel;
static gpu_raytrace_pipeline_t               *dxr_pipeline;
static gpu_texture_t                         *dxr_output = NULL;
static D3D12_CPU_DESCRIPTOR_HANDLE            dxr_output_cpu_descriptor;
static D3D12_GPU_DESCRIPTOR_HANDLE            dxr_output_descriptor_handle;
static D3D12_GPU_DESCRIPTOR_HANDLE            dxr_vbgpu_descriptor_handle;
static D3D12_GPU_DESCRIPTOR_HANDLE            dxr_ibgpu_descriptor_handle;
static D3D12_GPU_DESCRIPTOR_HANDLE            dxr_tex0gpu_descriptor_handle;
static D3D12_GPU_DESCRIPTOR_HANDLE            dxr_tex1gpu_descriptor_handle;
static D3D12_GPU_DESCRIPTOR_HANDLE            dxr_tex2gpu_descriptor_handle;
static D3D12_GPU_DESCRIPTOR_HANDLE            dxr_texenvgpu_descriptor_handle;
static D3D12_GPU_DESCRIPTOR_HANDLE            dxr_texsobolgpu_descriptor_handle;
static D3D12_GPU_DESCRIPTOR_HANDLE            dxr_texscramblegpu_descriptor_handle;
static D3D12_GPU_DESCRIPTOR_HANDLE            dxr_texrankgpu_descriptor_handle;
static int                                    dxr_descriptors_allocated = 0;
static UINT                                   dxr_descriptor_size;
static gpu_buffer_t                          *dxr_vb[16];
static gpu_buffer_t                          *dxr_vb_last[16];
static gpu_buffer_t                          *dxr_ib[16];
static int                                    dxr_vb_count      = 0;
static int                                    dxr_vb_count_last = 0;
static inst_t                                 dxr_instances[1024];
static int                                    dxr_instances_count = 0;

void gpu_raytrace_pipeline_init(gpu_raytrace_pipeline_t *pipeline, void *ray_shader, int ray_shader_size, gpu_buffer_t *constant_buffer) {
	dxr_output                = NULL;
	dxr_descriptors_allocated = 0;
	pipeline->constant_buffer = constant_buffer;

	D3D12_DESCRIPTOR_HEAP_DESC descriptor_heap_desc = {
	    .NumDescriptors = 12,
	    .Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
	    .Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
	    .NodeMask       = 0,
	};
	if (dxr_descriptor_heap != NULL) {
		dxr_descriptor_heap->lpVtbl->Release(dxr_descriptor_heap);
	}
	device->lpVtbl->CreateDescriptorHeap(device, &descriptor_heap_desc, &IID_ID3D12DescriptorHeap, &dxr_descriptor_heap);
	dxr_descriptor_size = device->lpVtbl->GetDescriptorHandleIncrementSize(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	if (dxr_device != NULL) {
		dxr_device->lpVtbl->Release(dxr_device);
	}
	if (dxr_command_list != NULL) {
		dxr_command_list->lpVtbl->Release(dxr_command_list);
	}
	device->lpVtbl->QueryInterface(device, &IID_ID3D12Device5, &dxr_device);
	command_list->lpVtbl->QueryInterface(command_list, &IID_ID3D12GraphicsCommandList4, &dxr_command_list);

	// Root signatures
	D3D12_DESCRIPTOR_RANGE ranges[] = {{D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND},
	                                   {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND},
	                                   {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND},
	                                   {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND},
	                                   {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND},
	                                   {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND},
	                                   {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND},
	                                   {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 7, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND},
	                                   {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 8, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND},
	                                   {D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 9, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND}};

	D3D12_ROOT_PARAMETER root_parameters[12] = {{D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, {1, &ranges[0]}, D3D12_SHADER_VISIBILITY_ALL},
	                                            {D3D12_ROOT_PARAMETER_TYPE_SRV, {0}, D3D12_SHADER_VISIBILITY_ALL},
	                                            {D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, {1, &ranges[1]}, D3D12_SHADER_VISIBILITY_ALL},
	                                            {D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, {1, &ranges[2]}, D3D12_SHADER_VISIBILITY_ALL},
	                                            {D3D12_ROOT_PARAMETER_TYPE_CBV, {0}, D3D12_SHADER_VISIBILITY_ALL},
	                                            {D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, {1, &ranges[3]}, D3D12_SHADER_VISIBILITY_ALL},
	                                            {D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, {1, &ranges[4]}, D3D12_SHADER_VISIBILITY_ALL},
	                                            {D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, {1, &ranges[5]}, D3D12_SHADER_VISIBILITY_ALL},
	                                            {D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, {1, &ranges[6]}, D3D12_SHADER_VISIBILITY_ALL},
	                                            {D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, {1, &ranges[7]}, D3D12_SHADER_VISIBILITY_ALL},
	                                            {D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, {1, &ranges[8]}, D3D12_SHADER_VISIBILITY_ALL},
	                                            {D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, {1, &ranges[9]}, D3D12_SHADER_VISIBILITY_ALL}};

	D3D12_ROOT_SIGNATURE_DESC root_signature_desc = {
	    .NumParameters = ARRAYSIZE(root_parameters),
	    .pParameters   = root_parameters,
	};
	ID3DBlob *blob  = NULL;
	ID3DBlob *error = NULL;
	D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error);
	if (dxr_root_signature != NULL) {
		dxr_root_signature->lpVtbl->Release(dxr_root_signature);
	}
	device->lpVtbl->CreateRootSignature(device, 1, blob->lpVtbl->GetBufferPointer(blob), blob->lpVtbl->GetBufferSize(blob), &IID_ID3D12RootSignature,
	                                    &dxr_root_signature);

	// Pipeline
	D3D12_STATE_OBJECT_DESC raytracingPipeline = {
	    .Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE,
	};
	D3D12_SHADER_BYTECODE shaderBytecode = {
	    .pShaderBytecode = ray_shader,
	    .BytecodeLength  = ray_shader_size,
	};
	D3D12_DXIL_LIBRARY_DESC dxilLibrary = {
	    .DXILLibrary = shaderBytecode,
	};
	D3D12_EXPORT_DESC exports[3] = {0};
	exports[0].Name              = L"raygeneration";
	exports[1].Name              = L"closesthit";
	exports[2].Name              = L"miss";
	dxilLibrary.pExports         = exports;
	dxilLibrary.NumExports       = 3;

	D3D12_HIT_GROUP_DESC hitGroup = {
	    .ClosestHitShaderImport = L"closesthit",
	    .HitGroupExport         = L"hitgroup",
	    .Type                   = D3D12_HIT_GROUP_TYPE_TRIANGLES,
	};
	D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {
	    .MaxPayloadSizeInBytes   = 10 * sizeof(float), // float4 color, float3 ray_origin, float3 ray_dir
	    .MaxAttributeSizeInBytes = 2 * sizeof(float),  // float2 barycentrics
	};
	D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {
	    .MaxTraceRecursionDepth = 1, // ~ primary rays only
	};

	D3D12_STATE_SUBOBJECT subobjects[5] = {{D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, &dxilLibrary},
	                                       {D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP, &hitGroup},
	                                       {D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG, &shaderConfig},
	                                       {D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE, &dxr_root_signature},
	                                       {D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG, &pipelineConfig}};
	raytracingPipeline.NumSubobjects    = 5;
	raytracingPipeline.pSubobjects      = subobjects;
	dxr_device->lpVtbl->CreateStateObject(dxr_device, &raytracingPipeline, &IID_ID3D12StateObject, &pipeline->impl.state);

	// Shader tables
	ID3D12StateObjectProperties *stateObjectProps = NULL;
	pipeline->impl.state->lpVtbl->QueryInterface(pipeline->impl.state, &IID_ID3D12StateObjectProperties, &stateObjectProps);
	const void *rayGenShaderId   = stateObjectProps->lpVtbl->GetShaderIdentifier(stateObjectProps, L"raygeneration");
	const void *missShaderId     = stateObjectProps->lpVtbl->GetShaderIdentifier(stateObjectProps, L"miss");
	const void *hitGroupShaderId = stateObjectProps->lpVtbl->GetShaderIdentifier(stateObjectProps, L"hitgroup");
	UINT        shaderIdSize     = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	int         align            = D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;

	// Ray gen shader table
	{
		UINT                size             = shaderIdSize + constant_buffer->count;
		UINT                shaderRecordSize = (size + (align - 1)) & ~(align - 1);
		D3D12_RESOURCE_DESC bufferDesc       = {
		          .Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER,
		          .Width              = shaderRecordSize,
		          .Height             = 1,
		          .DepthOrArraySize   = 1,
		          .MipLevels          = 1,
		          .Format             = DXGI_FORMAT_UNKNOWN,
		          .SampleDesc.Count   = 1,
		          .SampleDesc.Quality = 0,
		          .Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
        };
		D3D12_HEAP_PROPERTIES uploadHeapProperties = {
		    .Type             = D3D12_HEAP_TYPE_UPLOAD,
		    .CreationNodeMask = 1,
		    .VisibleNodeMask  = 1,
		};
		device->lpVtbl->CreateCommittedResource(device, &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
		                                        &IID_ID3D12Resource, &pipeline->impl.raygen_shader_table);

		D3D12_RANGE rstRange = {
		    .Begin = 0,
		    .End   = 0,
		};
		uint8_t *byteDest;
		pipeline->impl.raygen_shader_table->lpVtbl->Map(pipeline->impl.raygen_shader_table, 0, &rstRange, (void **)(&byteDest));

		D3D12_RANGE cbRange = {
		    .Begin = 0,
		    .End   = constant_buffer->count,
		};
		void *constantBufferData;
		constant_buffer->impl.buffer->lpVtbl->Map(constant_buffer->impl.buffer, 0, &cbRange, (void **)&constantBufferData);
		memcpy(byteDest, rayGenShaderId, size);
		memcpy(byteDest + size, constantBufferData, constant_buffer->count);
		pipeline->impl.raygen_shader_table->lpVtbl->Unmap(pipeline->impl.raygen_shader_table, 0, NULL);
	}

	// Miss shader table
	{
		UINT                size             = shaderIdSize;
		UINT                shaderRecordSize = (size + (align - 1)) & ~(align - 1);
		D3D12_RESOURCE_DESC bufferDesc       = {
		          .Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER,
		          .Width              = shaderRecordSize,
		          .Height             = 1,
		          .DepthOrArraySize   = 1,
		          .MipLevels          = 1,
		          .Format             = DXGI_FORMAT_UNKNOWN,
		          .SampleDesc.Count   = 1,
		          .SampleDesc.Quality = 0,
		          .Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
        };
		D3D12_HEAP_PROPERTIES uploadHeapProperties = {
		    .Type             = D3D12_HEAP_TYPE_UPLOAD,
		    .CreationNodeMask = 1,
		    .VisibleNodeMask  = 1,
		};

		device->lpVtbl->CreateCommittedResource(device, &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
		                                        &IID_ID3D12Resource, &pipeline->impl.miss_shader_table);

		D3D12_RANGE mstRange = {
		    .Begin = 0,
		    .End   = 0,
		};
		uint8_t *byteDest;
		pipeline->impl.miss_shader_table->lpVtbl->Map(pipeline->impl.miss_shader_table, 0, &mstRange, (void **)(&byteDest));
		memcpy(byteDest, missShaderId, size);
		pipeline->impl.miss_shader_table->lpVtbl->Unmap(pipeline->impl.miss_shader_table, 0, NULL);
	}

	// Hit group shader table
	{
		UINT                size             = shaderIdSize;
		UINT                shaderRecordSize = (size + (align - 1)) & ~(align - 1);
		D3D12_RESOURCE_DESC bufferDesc       = {
		          .Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER,
		          .Width              = shaderRecordSize,
		          .Height             = 1,
		          .DepthOrArraySize   = 1,
		          .MipLevels          = 1,
		          .Format             = DXGI_FORMAT_UNKNOWN,
		          .SampleDesc.Count   = 1,
		          .SampleDesc.Quality = 0,
		          .Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
        };
		D3D12_HEAP_PROPERTIES uploadHeapProperties = {
		    .Type             = D3D12_HEAP_TYPE_UPLOAD,
		    .CreationNodeMask = 1,
		    .VisibleNodeMask  = 1,
		};

		device->lpVtbl->CreateCommittedResource(device, &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
		                                        &IID_ID3D12Resource, &pipeline->impl.hitgroup_shader_table);

		D3D12_RANGE hstRange = {
		    .Begin = 0,
		    .End   = 0,
		};
		uint8_t *byteDest;
		pipeline->impl.hitgroup_shader_table->lpVtbl->Map(pipeline->impl.hitgroup_shader_table, 0, &hstRange, (void **)(&byteDest));
		memcpy(byteDest, hitGroupShaderId, size);
		pipeline->impl.hitgroup_shader_table->lpVtbl->Unmap(pipeline->impl.hitgroup_shader_table, 0, NULL);
	}

	// Output descriptor
	D3D12_CPU_DESCRIPTOR_HANDLE handle;
	dxr_descriptor_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(dxr_descriptor_heap, &handle);
	dxr_output_cpu_descriptor.ptr = handle.ptr + (INT64)(dxr_descriptors_allocated) * (UINT64)(dxr_descriptor_size);

	dxr_descriptor_heap->lpVtbl->GetGPUDescriptorHandleForHeapStart(dxr_descriptor_heap, &handle);
	int descriptorHeapIndex          = dxr_descriptors_allocated++;
	dxr_output_descriptor_handle.ptr = handle.ptr + (INT64)(descriptorHeapIndex) * (UINT64)(dxr_descriptor_size);
}

void gpu_raytrace_pipeline_destroy(gpu_raytrace_pipeline_t *pipeline) {
	pipeline->impl.state->lpVtbl->Release(pipeline->impl.state);
	pipeline->impl.raygen_shader_table->lpVtbl->Release(pipeline->impl.raygen_shader_table);
	pipeline->impl.miss_shader_table->lpVtbl->Release(pipeline->impl.miss_shader_table);
	pipeline->impl.hitgroup_shader_table->lpVtbl->Release(pipeline->impl.hitgroup_shader_table);
}

UINT create_srv_vb(gpu_buffer_t *vb, UINT numElements, UINT elementSize) {
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {
	    .ViewDimension           = D3D12_SRV_DIMENSION_BUFFER,
	    .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
	    .Buffer.NumElements      = numElements,
	};

	if (elementSize == 0) {
		srvDesc.Format                     = DXGI_FORMAT_R32_TYPELESS;
		srvDesc.Buffer.Flags               = D3D12_BUFFER_SRV_FLAG_RAW;
		srvDesc.Buffer.StructureByteStride = 0;
	}
	else {
		srvDesc.Format                     = DXGI_FORMAT_UNKNOWN;
		srvDesc.Buffer.Flags               = D3D12_BUFFER_SRV_FLAG_NONE;
		srvDesc.Buffer.StructureByteStride = elementSize;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE handle;
	dxr_descriptor_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(dxr_descriptor_heap, &handle);

	D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor = {
	    .ptr = handle.ptr + (INT64)(dxr_descriptors_allocated) * (UINT64)(dxr_descriptor_size),
	};
	UINT descriptorIndex = dxr_descriptors_allocated++;
	device->lpVtbl->CreateShaderResourceView(device, vb->impl.buffer, &srvDesc, cpuDescriptor);
	dxr_descriptor_heap->lpVtbl->GetGPUDescriptorHandleForHeapStart(dxr_descriptor_heap, &handle);
	dxr_vbgpu_descriptor_handle.ptr = handle.ptr + (INT64)(descriptorIndex) * (UINT64)(dxr_descriptor_size);
	return descriptorIndex;
}

UINT create_srv_ib(gpu_buffer_t *ib, UINT numElements, UINT elementSize) {
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {
	    .ViewDimension           = D3D12_SRV_DIMENSION_BUFFER,
	    .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
	    .Buffer.NumElements      = numElements,
	};

	if (elementSize == 0) {
		srvDesc.Format                     = DXGI_FORMAT_R32_TYPELESS;
		srvDesc.Buffer.Flags               = D3D12_BUFFER_SRV_FLAG_RAW;
		srvDesc.Buffer.StructureByteStride = 0;
	}
	else {
		srvDesc.Format                     = DXGI_FORMAT_UNKNOWN;
		srvDesc.Buffer.Flags               = D3D12_BUFFER_SRV_FLAG_NONE;
		srvDesc.Buffer.StructureByteStride = elementSize;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE handle;
	dxr_descriptor_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(dxr_descriptor_heap, &handle);

	D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor = {
	    .ptr = handle.ptr + (INT64)(dxr_descriptors_allocated) * (UINT64)(dxr_descriptor_size),
	};

	UINT descriptorIndex = dxr_descriptors_allocated++;
	dxr_descriptor_heap->lpVtbl->GetGPUDescriptorHandleForHeapStart(dxr_descriptor_heap, &handle);
	device->lpVtbl->CreateShaderResourceView(device, ib->impl.buffer, &srvDesc, cpuDescriptor);
	dxr_ibgpu_descriptor_handle.ptr = handle.ptr + (INT64)(descriptorIndex) * (UINT64)(dxr_descriptor_size);

	return descriptorIndex;
}

void gpu_raytrace_acceleration_structure_init(gpu_raytrace_acceleration_structure_t *accel) {
	dxr_vb_count        = 0;
	dxr_instances_count = 0;
}

void gpu_raytrace_acceleration_structure_add(gpu_raytrace_acceleration_structure_t *accel, gpu_buffer_t *vb, gpu_buffer_t *ib, iron_matrix4x4_t transform) {
	int vb_i = -1;
	for (int i = 0; i < dxr_vb_count; ++i) {
		if (vb == dxr_vb[i]) {
			vb_i = i;
			break;
		}
	}
	if (vb_i == -1) {
		vb_i                 = dxr_vb_count;
		dxr_vb[dxr_vb_count] = vb;
		dxr_ib[dxr_vb_count] = ib;
		dxr_vb_count++;
	}

	inst_t inst                        = {.i = vb_i, .m = transform};
	dxr_instances[dxr_instances_count] = inst;
	dxr_instances_count++;
}

void _gpu_raytrace_acceleration_structure_destroy_bottom(gpu_raytrace_acceleration_structure_t *accel) {
	for (int i = 0; i < dxr_vb_count_last; ++i) {
		accel->impl.bottom_level_accel[i]->lpVtbl->Release(accel->impl.bottom_level_accel[i]);
	}
}

void _gpu_raytrace_acceleration_structure_destroy_top(gpu_raytrace_acceleration_structure_t *accel) {
	accel->impl.top_level_accel->lpVtbl->Release(accel->impl.top_level_accel);
}

void gpu_raytrace_acceleration_structure_build(gpu_raytrace_acceleration_structure_t *accel, gpu_buffer_t *vb_full, gpu_buffer_t *ib_full) {
	bool build_bottom = false;
	for (int i = 0; i < 16; ++i) {
		if (dxr_vb_last[i] != dxr_vb[i]) {
			build_bottom = true;
		}
		dxr_vb_last[i] = dxr_vb[i];
	}

	if (dxr_vb_count_last > 0) {
		if (build_bottom) {
			_gpu_raytrace_acceleration_structure_destroy_bottom(accel);
		}
		_gpu_raytrace_acceleration_structure_destroy_top(accel);
	}

	dxr_vb_count_last = dxr_vb_count;

	if (dxr_vb_count == 0) {
		return;
	}

	dxr_descriptors_allocated = 1; // 1 descriptor already allocated in gpu_raytrace_pipeline_init

#ifdef is_forge
	create_srv_ib(ib_full, ib_full->count, 0);
	create_srv_vb(vb_full, vb_full->count, dxr_vb[0]->stride);
#else
	create_srv_ib(dxr_ib[0], dxr_ib[0]->count, 0);
	create_srv_vb(dxr_vb[0], dxr_vb[0]->count, dxr_vb[0]->stride);
#endif

	command_list->lpVtbl->Reset(command_list, command_allocator, NULL);

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS topLevelInputs = {
	    .DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
	    .Flags       = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
	    .NumDescs    = 1,
	    .Type        = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL,
	};

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {0};
	dxr_device->lpVtbl->GetRaytracingAccelerationStructurePrebuildInfo(dxr_device, &topLevelInputs, &topLevelPrebuildInfo);

	UINT64 scratch_size = topLevelPrebuildInfo.ScratchDataSizeInBytes;

	// Bottom AS
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS bottomLevelInputs[16];
	D3D12_RAYTRACING_GEOMETRY_DESC                       geometryDescs[16];
	if (build_bottom) {
		for (int i = 0; i < dxr_vb_count; ++i) {
			D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {
			    .Type                   = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES,
			    .Triangles.IndexBuffer  = dxr_ib[i]->impl.buffer->lpVtbl->GetGPUVirtualAddress(dxr_ib[i]->impl.buffer),
			    .Triangles.IndexCount   = dxr_ib[i]->count,
			    .Triangles.IndexFormat  = DXGI_FORMAT_R32_UINT,
			    .Triangles.Transform3x4 = 0,
			    .Triangles.VertexFormat = DXGI_FORMAT_R16G16B16A16_SNORM,
			    .Triangles.VertexCount  = dxr_vb[i]->count,
			};

			D3D12_RESOURCE_DESC desc;
			dxr_vb[i]->impl.buffer->lpVtbl->GetDesc(dxr_vb[i]->impl.buffer, &desc);

			geometryDesc.Triangles.VertexBuffer.StartAddress  = dxr_vb[i]->impl.buffer->lpVtbl->GetGPUVirtualAddress(dxr_vb[i]->impl.buffer);
			geometryDesc.Triangles.VertexBuffer.StrideInBytes = desc.Width / dxr_vb[i]->count;
			geometryDesc.Flags                                = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
			geometryDescs[i]                                  = geometryDesc;

			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {0};
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS  inputs                  = {
			                      .DescsLayout    = D3D12_ELEMENTS_LAYOUT_ARRAY,
			                      .NumDescs       = 1,
			                      .Type           = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL,
			                      .pGeometryDescs = &geometryDescs[i],
			                      .Flags          = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
            };
			dxr_device->lpVtbl->GetRaytracingAccelerationStructurePrebuildInfo(dxr_device, &inputs, &bottomLevelPrebuildInfo);
			bottomLevelInputs[i] = inputs;

			UINT64 blSize = bottomLevelPrebuildInfo.ScratchDataSizeInBytes;
			if (scratch_size < blSize) {
				scratch_size = blSize;
			}

			{
				D3D12_RESOURCE_DESC bufferDesc = {
				    .Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER,
				    .Width              = bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes,
				    .Height             = 1,
				    .DepthOrArraySize   = 1,
				    .MipLevels          = 1,
				    .Format             = DXGI_FORMAT_UNKNOWN,
				    .SampleDesc.Count   = 1,
				    .SampleDesc.Quality = 0,
				    .Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
				    .Flags              = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
				};
				D3D12_HEAP_PROPERTIES uploadHeapProperties = {
				    .Type             = D3D12_HEAP_TYPE_DEFAULT,
				    .CreationNodeMask = 1,
				    .VisibleNodeMask  = 1,
				};

				device->lpVtbl->CreateCommittedResource(dxr_device, &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc,
				                                        D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, NULL, &IID_ID3D12Resource,
				                                        &accel->impl.bottom_level_accel[i]);
			}
		}
	}

	// Create scratch memory
	ID3D12Resource *scratchResource;
	{
		D3D12_RESOURCE_DESC bufferDesc = {
		    .Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER,
		    .Width              = scratch_size,
		    .Height             = 1,
		    .DepthOrArraySize   = 1,
		    .MipLevels          = 1,
		    .Format             = DXGI_FORMAT_UNKNOWN,
		    .SampleDesc.Count   = 1,
		    .SampleDesc.Quality = 0,
		    .Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		    .Flags              = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		};
		D3D12_HEAP_PROPERTIES uploadHeapProperties = {
		    .Type             = D3D12_HEAP_TYPE_DEFAULT,
		    .CreationNodeMask = 1,
		    .VisibleNodeMask  = 1,
		};

		device->lpVtbl->CreateCommittedResource(dxr_device, &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		                                        NULL, &IID_ID3D12Resource, &scratchResource);
	}

	// Bottom AS
	if (build_bottom) {
		for (int i = 0; i < dxr_vb_count; ++i) {
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {
			    .Inputs                           = bottomLevelInputs[i],
			    .ScratchAccelerationStructureData = scratchResource->lpVtbl->GetGPUVirtualAddress(scratchResource),
			    .DestAccelerationStructureData    = accel->impl.bottom_level_accel[i]->lpVtbl->GetGPUVirtualAddress(accel->impl.bottom_level_accel[i]),
			};
			dxr_command_list->lpVtbl->BuildRaytracingAccelerationStructure(dxr_command_list, &bottomLevelBuildDesc, 0, NULL);
		}
	}

	// Top AS
	{
		D3D12_RESOURCE_DESC bufferDesc = {
		    .Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER,
		    .Width              = topLevelPrebuildInfo.ResultDataMaxSizeInBytes,
		    .Height             = 1,
		    .DepthOrArraySize   = 1,
		    .MipLevels          = 1,
		    .Format             = DXGI_FORMAT_UNKNOWN,
		    .SampleDesc.Count   = 1,
		    .SampleDesc.Quality = 0,
		    .Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		    .Flags              = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		};
		D3D12_HEAP_PROPERTIES uploadHeapProperties = {
		    .Type             = D3D12_HEAP_TYPE_DEFAULT,
		    .CreationNodeMask = 1,
		    .VisibleNodeMask  = 1,
		};

		device->lpVtbl->CreateCommittedResource(device, &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc,
		                                        D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, NULL, &IID_ID3D12Resource,
		                                        &accel->impl.top_level_accel);
	}

	D3D12_RESOURCE_DESC bufferDesc = {
	    .Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER,
	    .Width              = sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * dxr_instances_count,
	    .Height             = 1,
	    .DepthOrArraySize   = 1,
	    .MipLevels          = 1,
	    .Format             = DXGI_FORMAT_UNKNOWN,
	    .SampleDesc.Count   = 1,
	    .SampleDesc.Quality = 0,
	    .Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
	};
	D3D12_HEAP_PROPERTIES uploadHeapProperties = {
	    .Type             = D3D12_HEAP_TYPE_UPLOAD,
	    .CreationNodeMask = 1,
	    .VisibleNodeMask  = 1,
	};

	ID3D12Resource *instanceDescs;
	device->lpVtbl->CreateCommittedResource(device, &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
	                                        &IID_ID3D12Resource, &instanceDescs);
	void *mappedData;
	instanceDescs->lpVtbl->Map(instanceDescs, 0, NULL, &mappedData);

	for (int i = 0; i < dxr_instances_count; ++i) {
		D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {0};

		instanceDesc.Transform[0][0] = dxr_instances[i].m.m[0];
		instanceDesc.Transform[0][1] = dxr_instances[i].m.m[1];
		instanceDesc.Transform[0][2] = dxr_instances[i].m.m[2];
		instanceDesc.Transform[0][3] = dxr_instances[i].m.m[3];
		instanceDesc.Transform[1][0] = dxr_instances[i].m.m[4];
		instanceDesc.Transform[1][1] = dxr_instances[i].m.m[5];
		instanceDesc.Transform[1][2] = dxr_instances[i].m.m[6];
		instanceDesc.Transform[1][3] = dxr_instances[i].m.m[7];
		instanceDesc.Transform[2][0] = dxr_instances[i].m.m[8];
		instanceDesc.Transform[2][1] = dxr_instances[i].m.m[9];
		instanceDesc.Transform[2][2] = dxr_instances[i].m.m[10];
		instanceDesc.Transform[2][3] = dxr_instances[i].m.m[11];

		int ib_off = 0;
		for (int j = 0; j < dxr_instances[i].i; ++j) {
			ib_off += dxr_ib[j]->count * 4;
		}
		instanceDesc.InstanceID   = ib_off;
		instanceDesc.InstanceMask = 1;
		instanceDesc.AccelerationStructure =
		    accel->impl.bottom_level_accel[dxr_instances[i].i]->lpVtbl->GetGPUVirtualAddress(accel->impl.bottom_level_accel[dxr_instances[i].i]);
		memcpy((uint8_t *)mappedData + i * sizeof(D3D12_RAYTRACING_INSTANCE_DESC), &instanceDesc, sizeof(D3D12_RAYTRACING_INSTANCE_DESC));
	}

	instanceDescs->lpVtbl->Unmap(instanceDescs, 0, NULL);

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {0};
	topLevelInputs.InstanceDescs                                         = instanceDescs->lpVtbl->GetGPUVirtualAddress(instanceDescs);
	topLevelBuildDesc.Inputs                                             = topLevelInputs;
	topLevelBuildDesc.DestAccelerationStructureData    = accel->impl.top_level_accel->lpVtbl->GetGPUVirtualAddress(accel->impl.top_level_accel);
	topLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->lpVtbl->GetGPUVirtualAddress(scratchResource);

	D3D12_RESOURCE_BARRIER barrier = {
	    .Type          = D3D12_RESOURCE_BARRIER_TYPE_UAV,
	    .UAV.pResource = accel->impl.bottom_level_accel[0],
	};
	command_list->lpVtbl->ResourceBarrier(command_list, 1, &barrier);
	dxr_command_list->lpVtbl->BuildRaytracingAccelerationStructure(dxr_command_list, &topLevelBuildDesc, 0, NULL);

	gpu_execute_and_wait();

	scratchResource->lpVtbl->Release(scratchResource);
	instanceDescs->lpVtbl->Release(instanceDescs);
}

void gpu_raytrace_acceleration_structure_destroy(gpu_raytrace_acceleration_structure_t *accel) {
	// accel->impl.bottom_level_accel->Release();
	// accel->impl.top_level_accel->Release();
}

void gpu_raytrace_set_textures(gpu_texture_t *texpaint0, gpu_texture_t *texpaint1, gpu_texture_t *texpaint2, gpu_texture_t *texenv, gpu_texture_t *texsobol,
                               gpu_texture_t *texscramble, gpu_texture_t *texrank) {
	D3D12_CPU_DESCRIPTOR_HANDLE  handle, cpuDescriptor, sourceCpu;
	D3D12_GPU_DESCRIPTOR_HANDLE  ghandle;
	gpu_texture_t               *textures[]    = {texpaint0, texpaint1, texpaint2, texenv, texsobol, texscramble, texrank};
	D3D12_GPU_DESCRIPTOR_HANDLE *gpu_handles[] = {&dxr_tex0gpu_descriptor_handle,   &dxr_tex1gpu_descriptor_handle,     &dxr_tex2gpu_descriptor_handle,
	                                              &dxr_texenvgpu_descriptor_handle, &dxr_texsobolgpu_descriptor_handle, &dxr_texscramblegpu_descriptor_handle,
	                                              &dxr_texrankgpu_descriptor_handle};

	dxr_descriptor_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(dxr_descriptor_heap, &handle);
	for (int i = 0; i < 7; i++) {
		cpuDescriptor.ptr = handle.ptr + (5 + i) * (UINT64)dxr_descriptor_size;
		textures[i]->impl.srv_descriptor_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(textures[i]->impl.srv_descriptor_heap, &sourceCpu);
		device->lpVtbl->CopyDescriptorsSimple(device, 1, cpuDescriptor, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		dxr_descriptor_heap->lpVtbl->GetGPUDescriptorHandleForHeapStart(dxr_descriptor_heap, &ghandle);
		gpu_handles[i]->ptr = ghandle.ptr + (5 + i) * (UINT64)dxr_descriptor_size;
	}
}

void gpu_raytrace_set_acceleration_structure(gpu_raytrace_acceleration_structure_t *accel) {
	dxr_accel = accel;
}

void gpu_raytrace_set_pipeline(gpu_raytrace_pipeline_t *pipeline) {
	dxr_pipeline = pipeline;
}

void gpu_raytrace_set_target(gpu_texture_t *output) {
	if (output != dxr_output) {
		output->impl.image->lpVtbl->Release(output->impl.image);
		output->impl.rtv_descriptor_heap->lpVtbl->Release(output->impl.rtv_descriptor_heap);
		output->impl.srv_descriptor_heap->lpVtbl->Release(output->impl.srv_descriptor_heap);

		D3D12_HEAP_PROPERTIES heap_properties = {
		    .Type             = D3D12_HEAP_TYPE_DEFAULT,
		    .CreationNodeMask = 1,
		    .VisibleNodeMask  = 1,
		};
		D3D12_RESOURCE_DESC desc = {
		    .Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		    .Width              = output->width,
		    .Height             = output->height,
		    .DepthOrArraySize   = 1,
		    .MipLevels          = 1,
		    .Format             = DXGI_FORMAT_R16G16B16A16_FLOAT,
		    .SampleDesc.Count   = 1,
		    .SampleDesc.Quality = 0,
		    .Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN,
		    .Flags              = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		};
		D3D12_CLEAR_VALUE clear_value;
		clear_value.Format   = DXGI_FORMAT_R16G16B16A16_FLOAT;
		clear_value.Color[0] = 0.0f;
		clear_value.Color[1] = 0.0f;
		clear_value.Color[2] = 0.0f;
		clear_value.Color[3] = 0.0f;

		device->lpVtbl->CreateCommittedResource(device, &heap_properties, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, &clear_value,
		                                        &IID_ID3D12Resource, &output->impl.image);

		D3D12_RENDER_TARGET_VIEW_DESC view = {
		    .Format               = DXGI_FORMAT_R16G16B16A16_FLOAT,
		    .ViewDimension        = D3D12_RTV_DIMENSION_TEXTURE2D,
		    .Texture2D.MipSlice   = 0,
		    .Texture2D.PlaneSlice = 0,
		};
		D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {
		    .NumDescriptors = 1,
		    .Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		    .Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		};
		device->lpVtbl->CreateDescriptorHeap(device, &heap_desc, &IID_ID3D12DescriptorHeap, &output->impl.rtv_descriptor_heap);
		D3D12_CPU_DESCRIPTOR_HANDLE handle;
		output->impl.rtv_descriptor_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(output->impl.rtv_descriptor_heap, &handle);
		device->lpVtbl->CreateRenderTargetView(device, output->impl.image, &view, handle);

		D3D12_DESCRIPTOR_HEAP_DESC descriptor_heap_desc = {
		    .NumDescriptors = 1,
		    .Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		    .NodeMask       = 0,
		    .Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		};
		device->lpVtbl->CreateDescriptorHeap(device, &descriptor_heap_desc, &IID_ID3D12DescriptorHeap, &output->impl.srv_descriptor_heap);

		D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {
		    .ViewDimension                 = D3D12_SRV_DIMENSION_TEXTURE2D,
		    .Shader4ComponentMapping       = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
		    .Format                        = DXGI_FORMAT_R16G16B16A16_FLOAT,
		    .Texture2D.MipLevels           = 1,
		    .Texture2D.MostDetailedMip     = 0,
		    .Texture2D.ResourceMinLODClamp = 0.0f,
		};
		output->impl.srv_descriptor_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(output->impl.srv_descriptor_heap, &handle);
		device->lpVtbl->CreateShaderResourceView(device, output->impl.image, &srv_desc, handle);

		D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {
		    .ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D,
		};
		device->lpVtbl->CreateUnorderedAccessView(device, output->impl.image, NULL, &UAVDesc, dxr_output_cpu_descriptor);
	}
	dxr_output = output;
}

void gpu_raytrace_dispatch_rays() {
	command_list->lpVtbl->SetComputeRootSignature(command_list, dxr_root_signature);
	command_list->lpVtbl->SetDescriptorHeaps(command_list, 1, &dxr_descriptor_heap);
	command_list->lpVtbl->SetComputeRootDescriptorTable(command_list, 0, dxr_output_descriptor_handle);
	command_list->lpVtbl->SetComputeRootShaderResourceView(command_list, 1,
	                                                       dxr_accel->impl.top_level_accel->lpVtbl->GetGPUVirtualAddress(dxr_accel->impl.top_level_accel));
	command_list->lpVtbl->SetComputeRootDescriptorTable(command_list, 2, dxr_ibgpu_descriptor_handle);
	command_list->lpVtbl->SetComputeRootDescriptorTable(command_list, 3, dxr_vbgpu_descriptor_handle);
	command_list->lpVtbl->SetComputeRootConstantBufferView(
	    command_list, 4, dxr_pipeline->constant_buffer->impl.buffer->lpVtbl->GetGPUVirtualAddress(dxr_pipeline->constant_buffer->impl.buffer));
	command_list->lpVtbl->SetComputeRootDescriptorTable(command_list, 5, dxr_tex0gpu_descriptor_handle);
	command_list->lpVtbl->SetComputeRootDescriptorTable(command_list, 6, dxr_tex1gpu_descriptor_handle);
	command_list->lpVtbl->SetComputeRootDescriptorTable(command_list, 7, dxr_tex2gpu_descriptor_handle);
	command_list->lpVtbl->SetComputeRootDescriptorTable(command_list, 8, dxr_texenvgpu_descriptor_handle);
	command_list->lpVtbl->SetComputeRootDescriptorTable(command_list, 9, dxr_texsobolgpu_descriptor_handle);
	command_list->lpVtbl->SetComputeRootDescriptorTable(command_list, 10, dxr_texscramblegpu_descriptor_handle);
	command_list->lpVtbl->SetComputeRootDescriptorTable(command_list, 11, dxr_texrankgpu_descriptor_handle);

	D3D12_DISPATCH_RAYS_DESC dispatchDesc = {0};
	D3D12_RESOURCE_DESC      desc;
	dxr_pipeline->impl.hitgroup_shader_table->lpVtbl->GetDesc(dxr_pipeline->impl.hitgroup_shader_table, &desc);
	dispatchDesc.HitGroupTable.StartAddress  = dxr_pipeline->impl.hitgroup_shader_table->lpVtbl->GetGPUVirtualAddress(dxr_pipeline->impl.hitgroup_shader_table);
	dispatchDesc.HitGroupTable.SizeInBytes   = desc.Width;
	dispatchDesc.HitGroupTable.StrideInBytes = dispatchDesc.HitGroupTable.SizeInBytes;
	dispatchDesc.MissShaderTable.StartAddress = dxr_pipeline->impl.miss_shader_table->lpVtbl->GetGPUVirtualAddress(dxr_pipeline->impl.miss_shader_table);
	dxr_pipeline->impl.miss_shader_table->lpVtbl->GetDesc(dxr_pipeline->impl.miss_shader_table, &desc);
	dispatchDesc.MissShaderTable.SizeInBytes   = desc.Width;
	dispatchDesc.MissShaderTable.StrideInBytes = dispatchDesc.MissShaderTable.SizeInBytes;
	dispatchDesc.RayGenerationShaderRecord.StartAddress =
	    dxr_pipeline->impl.raygen_shader_table->lpVtbl->GetGPUVirtualAddress(dxr_pipeline->impl.raygen_shader_table);
	dxr_pipeline->impl.raygen_shader_table->lpVtbl->GetDesc(dxr_pipeline->impl.raygen_shader_table, &desc);
	dispatchDesc.RayGenerationShaderRecord.SizeInBytes = desc.Width;
	dispatchDesc.Width                                 = dxr_output->width;
	dispatchDesc.Height                                = dxr_output->height;
	dispatchDesc.Depth                                 = 1;
	dxr_command_list->lpVtbl->SetPipelineState1(dxr_command_list, dxr_pipeline->impl.state);
	dxr_command_list->lpVtbl->DispatchRays(dxr_command_list, &dispatchDesc);
}
