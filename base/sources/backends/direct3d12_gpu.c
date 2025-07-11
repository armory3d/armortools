#define WIN32_LEAN_AND_MEAN
#define HEAP_SIZE 2048
#define TEXTURE_COUNT 16
#include <iron_global.h>
#include <stdbool.h>
#include <malloc.h>
#include <math.h>
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <iron_gpu.h>
#include <iron_system.h>
#include <iron_math.h>
#include <backends/windows_system.h>

bool gpu_transpose_mat = false;
static ID3D12Device *device = NULL;
static ID3D12CommandQueue *queue;
static struct IDXGISwapChain *window_swapchain;
static ID3D12RootSignature *root_signature = NULL;
static struct ID3D12CommandAllocator *command_allocator;
static struct ID3D12GraphicsCommandList *command_list;
static gpu_pipeline_t *current_pipeline;
static D3D12_VIEWPORT current_viewport;
static D3D12_RECT current_scissor;
static gpu_buffer_t *current_vb;
static gpu_buffer_t *current_ib;
static D3D12_CPU_DESCRIPTOR_HANDLE target_descriptors[16];
static D3D12_CPU_DESCRIPTOR_HANDLE depth_handle;
static D3D12_CPU_DESCRIPTOR_HANDLE *current_depth_handle;
static gpu_texture_t *current_textures[TEXTURE_COUNT] = {
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};
static bool window_vsync;
static struct ID3D12DescriptorHeap *srv_heap;
static int srv_heap_index = 0;
static UINT64 fence_value;
static ID3D12Fence *fence;
static HANDLE fence_event;
static UINT64 frame_fence_values[GPU_FRAMEBUFFER_COUNT] = {0, 0};
static bool resized = false;

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

static D3D12_BLEND_OP convert_blend_operation(gpu_blending_operation_t op) {
	switch (op) {
	case GPU_BLENDOP_ADD:
		return D3D12_BLEND_OP_ADD;
	}
}

static D3D12_CULL_MODE convert_cull_mode(gpu_cull_mode_t cull_mode) {
	switch (cull_mode) {
	case GPU_CULL_MODE_CLOCKWISE:
		return D3D12_CULL_MODE_FRONT;
	case GPU_CULL_MODE_COUNTERCLOCKWISE:
		return D3D12_CULL_MODE_BACK;
	case GPU_CULL_MODE_NEVER:
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

static int format_size(DXGI_FORMAT format) {
	switch (format) {
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
		return 16;
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
		return 8;
	case DXGI_FORMAT_R16_FLOAT:
		return 2;
	case DXGI_FORMAT_R8_UNORM:
		return 1;
	default:
		return 4;
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

void gpu_barrier(gpu_texture_t *render_target, gpu_texture_state_t state_after) {
	if (render_target->state == state_after) {
		return;
	}
	D3D12_RESOURCE_BARRIER barrier = {
		.Transition.pResource = render_target->impl.image,
		.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
		.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
		.Transition.StateBefore = convert_texture_state(render_target->state),
		.Transition.StateAfter = convert_texture_state(state_after),
		.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
	};
	command_list->lpVtbl->ResourceBarrier(command_list, 1, &barrier);
	render_target->state = state_after;
}

void gpu_destroy() {
	gpu_wait();
	for (int i = 0; i < GPU_FRAMEBUFFER_COUNT; ++i) {
		gpu_texture_destroy(&framebuffers[i]);
	}
	if (framebuffer_depth.width > 0) {
		gpu_texture_destroy(&framebuffer_depth);
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
	render_target->width = width;
	render_target->height = height;
	render_target->impl.readback = NULL;
	render_target->state = (framebuffer_index >= 0) ? GPU_TEXTURE_STATE_PRESENT : GPU_TEXTURE_STATE_SHADER_RESOURCE;

	DXGI_FORMAT dxgi_format = convert_format(format);

	D3D12_CLEAR_VALUE clear_value;
	clear_value.Format = dxgi_format;
	clear_value.Color[0] = 0.0f;
	clear_value.Color[1] = 0.0f;
	clear_value.Color[2] = 0.0f;
	clear_value.Color[3] = 0.0f;
	clear_value.DepthStencil.Depth = 1.0f;

	D3D12_HEAP_PROPERTIES heap_properties = {
		.Type = D3D12_HEAP_TYPE_DEFAULT,
		.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
		.CreationNodeMask = 1,
		.VisibleNodeMask = 1,
	};

	D3D12_RESOURCE_DESC resource_desc = {
		.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		.Alignment = 0,
		.Width = width,
		.Height = height,
		.DepthOrArraySize = 1,
		.MipLevels = 1,
		.Format = dxgi_format,
		.SampleDesc.Count = 1,
		.SampleDesc.Quality = 0,
		.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
		.Flags = format == GPU_TEXTURE_FORMAT_D32 ? D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL : D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
	};

	D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {
		.NumDescriptors = 1,
		.Type = format == GPU_TEXTURE_FORMAT_D32 ? D3D12_DESCRIPTOR_HEAP_TYPE_DSV : D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
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
		.Format = dxgi_format,
		.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D,
		.Texture2D.MipSlice = 0,
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
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		.NodeMask = 0,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
	};

	device->lpVtbl->CreateDescriptorHeap(device, &descriptor_heap_desc, &IID_ID3D12DescriptorHeap, &render_target->impl.srv_descriptor_heap);

	D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {
		.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
		.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
		.Format = format == GPU_TEXTURE_FORMAT_D32 ? DXGI_FORMAT_R32_FLOAT : dxgi_format,
		.Texture2D.MipLevels = 1,
		.Texture2D.MostDetailedMip = 0,
		.Texture2D.ResourceMinLODClamp = 0.0f,
	};

	render_target->impl.srv_descriptor_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(render_target->impl.srv_descriptor_heap, &handle);
	device->lpVtbl->CreateShaderResourceView(device, render_target->impl.image, &srv_desc, handle);
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

	// Root signature
	ID3DBlob *root_blob;
	ID3DBlob *error_blob;
	D3D12_ROOT_PARAMETER parameters[2] = {};
	D3D12_DESCRIPTOR_RANGE range = {
		.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		.NumDescriptors = (UINT)TEXTURE_COUNT,
		.BaseShaderRegister = 0,
		.RegisterSpace = 0,
		.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND,
	};
	parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	parameters[0].DescriptorTable.NumDescriptorRanges = 1;
	parameters[0].DescriptorTable.pDescriptorRanges = &range;
	parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	parameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	parameters[1].Descriptor.ShaderRegister = 0;
	parameters[1].Descriptor.RegisterSpace = 0;
	D3D12_STATIC_SAMPLER_DESC samplers[TEXTURE_COUNT];
 	for (int i = 0; i < TEXTURE_COUNT; ++i) {
 		samplers[i].ShaderRegister = i;
 		samplers[i].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
 		samplers[i].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
 		samplers[i].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
 		samplers[i].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
 		samplers[i].MipLODBias = 0;
 		samplers[i].MaxAnisotropy = 16;
 		samplers[i].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
 		samplers[i].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
 		samplers[i].MinLOD = 0.0f;
 		samplers[i].MaxLOD = D3D12_FLOAT32_MAX;
 		samplers[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
 		samplers[i].RegisterSpace = 0;
 	}
	D3D12_ROOT_SIGNATURE_DESC root_signature_desc = {
		.NumParameters = 2,
		.pParameters = parameters,
		.NumStaticSamplers = TEXTURE_COUNT,
		.pStaticSamplers = samplers,
		.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT,
	};
	D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &root_blob, &error_blob);
	device->lpVtbl->CreateRootSignature(device, 0, root_blob->lpVtbl->GetBufferPointer(root_blob), root_blob->lpVtbl->GetBufferSize(root_blob), &IID_ID3D12RootSignature, &root_signature);

	D3D12_COMMAND_QUEUE_DESC queue_desc = {
		.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
		.Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
	};

	device->lpVtbl->CreateCommandQueue(device, &queue_desc, &IID_ID3D12CommandQueue, &queue);

	HWND hwnd = iron_windows_window_handle();
	DXGI_SWAP_CHAIN_DESC swapchain_desc = {
		.BufferCount = GPU_FRAMEBUFFER_COUNT,
		.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
		.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
		.BufferDesc.Width = iron_window_width(),
		.BufferDesc.Height = iron_window_height(),
		.OutputWindow = hwnd,
		.SampleDesc.Count = 1,
		.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
		.Windowed = true,
	};

	IDXGIFactory4 *dxgi_factory = NULL;
	CreateDXGIFactory1(&IID_IDXGIFactory4, &dxgi_factory);
	dxgi_factory->lpVtbl->CreateSwapChain(dxgi_factory, (IUnknown *)queue, &swapchain_desc, &window_swapchain);

	fence_value = 0;
	fence_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	device->lpVtbl->CreateFence(device, 0, D3D12_FENCE_FLAG_NONE, &IID_ID3D12Fence, &fence);

	gpu_create_framebuffers(depth_buffer_bits);

	D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {
		.NumDescriptors = HEAP_SIZE,
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
	};
	device->lpVtbl->CreateDescriptorHeap(device, &heap_desc, &IID_ID3D12DescriptorHeap, &srv_heap);

	device->lpVtbl->CreateCommandAllocator(device, D3D12_COMMAND_LIST_TYPE_DIRECT, &IID_ID3D12CommandAllocator, &command_allocator);
	device->lpVtbl->CreateCommandList(device, 0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator, NULL, &IID_ID3D12CommandList, &command_list);
}

int gpu_max_bound_textures(void) {
	return TEXTURE_COUNT;
}

void gpu_begin_internal(gpu_texture_t **targets, int count, gpu_texture_t *depth_buffer, unsigned flags, unsigned color, float depth) {
	for (int i = 0; i < current_render_targets_count; ++i) {
		current_render_targets[i]->impl.rtv_descriptor_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(current_render_targets[i]->impl.rtv_descriptor_heap, &target_descriptors[i]);
	}

	if (depth_buffer != NULL) {
		depth_buffer->impl.rtv_descriptor_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(depth_buffer->impl.rtv_descriptor_heap, &depth_handle);
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
		float clearColor[] = {((color & 0x00ff0000) >> 16) / 255.0f,
							  ((color & 0x0000ff00) >> 8) / 255.0f,
							  (color & 0x000000ff) / 255.0f,
							  ((color & 0xff000000) >> 24) / 255.0f};

		D3D12_CPU_DESCRIPTOR_HANDLE handle;
		target->impl.rtv_descriptor_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(target->impl.rtv_descriptor_heap, &handle);
		command_list->lpVtbl->ClearRenderTargetView(command_list, handle, clearColor, 0, NULL);
	}
	if (flags & GPU_CLEAR_DEPTH && depth_buffer != NULL) {
		D3D12_CPU_DESCRIPTOR_HANDLE handle;
		depth_buffer->impl.rtv_descriptor_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(depth_buffer->impl.rtv_descriptor_heap, &handle);
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

void gpu_wait() {
	wait_for_fence(fence, fence_value, fence_event);
}

void gpu_flush() {
	command_list->lpVtbl->Close(command_list);
	ID3D12CommandList *command_lists[] = {(ID3D12CommandList *)command_list};
	queue->lpVtbl->ExecuteCommandLists(queue, 1, command_lists);
	queue->lpVtbl->Signal(queue, fence, ++fence_value);
	gpu_wait();
	command_allocator->lpVtbl->Reset(command_allocator);
	command_list->lpVtbl->Reset(command_list, command_allocator, NULL);

	if (gpu_in_use) {
		command_list->lpVtbl->OMSetRenderTargets(command_list, current_render_targets_count, &target_descriptors[0], false, current_depth_handle);
		command_list->lpVtbl->SetPipelineState(command_list, current_pipeline->impl.pso);
		command_list->lpVtbl->SetGraphicsRootSignature(command_list, root_signature);
		command_list->lpVtbl->IASetVertexBuffers(command_list, 0, 1, (D3D12_VERTEX_BUFFER_VIEW *)&current_vb->impl.vertex_buffer_view);
		command_list->lpVtbl->IASetIndexBuffer(command_list, (D3D12_INDEX_BUFFER_VIEW *)&current_ib->impl.index_buffer_view);
		command_list->lpVtbl->RSSetViewports(command_list, 1, &current_viewport);
		command_list->lpVtbl->RSSetScissorRects(command_list, 1, &current_scissor);
	}
}

void gpu_present_internal() {
	command_list->lpVtbl->Close(command_list);

	ID3D12CommandList *command_lists[] = {(ID3D12CommandList *)command_list};
	queue->lpVtbl->ExecuteCommandLists(queue, 1, command_lists);
	queue->lpVtbl->Signal(queue, fence, ++fence_value);

	window_swapchain->lpVtbl->Present(window_swapchain, window_vsync, 0);

	queue->lpVtbl->Signal(queue, fence, ++fence_value);
	frame_fence_values[framebuffer_index] = fence_value;

	framebuffer_index = (framebuffer_index + 1) % GPU_FRAMEBUFFER_COUNT;
	wait_for_fence(fence, frame_fence_values[framebuffer_index], fence_event);
	gpu_wait();

	if (resized) {
		framebuffer_index = 0;

		for (int i = 0; i < GPU_FRAMEBUFFER_COUNT; ++i) {
			gpu_texture_destroy(&framebuffers[i]);
		}
		if (framebuffer_depth.width > 0) {
			gpu_texture_destroy(&framebuffer_depth);
		}

		window_swapchain->lpVtbl->ResizeBuffers(window_swapchain, GPU_FRAMEBUFFER_COUNT, iron_window_width(), iron_window_height(), DXGI_FORMAT_R8G8B8A8_UNORM, 0);

		for (int i = 0; i < GPU_FRAMEBUFFER_COUNT; ++i) {
			gpu_render_target_init2(&framebuffers[i], iron_window_width(), iron_window_height(), GPU_TEXTURE_FORMAT_RGBA32, i);
		}
		if (framebuffer_depth.width > 0) {
			gpu_render_target_init2(&framebuffer_depth, iron_window_width(), iron_window_height(), GPU_TEXTURE_FORMAT_D32, -1);
		}

		resized = false;
	}

	command_allocator->lpVtbl->Reset(command_allocator);
	command_list->lpVtbl->Reset(command_list, command_allocator, NULL);
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
	if (srv_heap_index + TEXTURE_COUNT > HEAP_SIZE) {
		srv_heap_index = 0;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE cpu_base;
	D3D12_GPU_DESCRIPTOR_HANDLE gpu_base;
	srv_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(srv_heap, &cpu_base);
	srv_heap->lpVtbl->GetGPUDescriptorHandleForHeapStart(srv_heap, &gpu_base);
	cpu_base.ptr += srv_heap_index * srv_step;
	gpu_base.ptr += srv_heap_index * srv_step;

	for (int i = 0; i < TEXTURE_COUNT; ++i) {
		if (current_textures[i] != NULL) {
			D3D12_CPU_DESCRIPTOR_HANDLE source_cpu;
			ID3D12DescriptorHeap *source_heap =  current_textures[i]->impl.srv_descriptor_heap;
			source_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(source_heap, &source_cpu);

			device->lpVtbl->CopyDescriptorsSimple(device, 1, cpu_base, source_cpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			cpu_base.ptr += srv_step;
			srv_heap_index++;
		}
	}

	ID3D12DescriptorHeap *heaps[] = {srv_heap};
	command_list->lpVtbl->SetDescriptorHeaps(command_list, 1, heaps);
	command_list->lpVtbl->SetGraphicsRootDescriptorTable(command_list, 0, gpu_base);
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
		.Width = (float)width,
		.Height = (float)height,
		.MinDepth = 0.0f,
		.MaxDepth = 1.0f,
	};
	command_list->lpVtbl->RSSetViewports(command_list, 1, &current_viewport);
}

void gpu_scissor(int x, int y, int width, int height) {
	current_scissor = (D3D12_RECT){
		.left = x,
		.top = y,
		.right = x + width,
		.bottom = y + height,
	};
	command_list->lpVtbl->RSSetScissorRects(command_list, 1, &current_scissor);
}

void gpu_disable_scissor() {
	current_scissor = (D3D12_RECT){
		.left = 0,
		.top = 0,
		.right = current_render_targets[0]->width,
		.bottom = current_render_targets[0]->height,
	};
	command_list->lpVtbl->RSSetScissorRects(command_list, 1, &current_scissor);
}

void gpu_set_pipeline(gpu_pipeline_t *pipeline) {
	current_pipeline = pipeline;
	command_list->lpVtbl->SetPipelineState(command_list, pipeline->impl.pso);
	command_list->lpVtbl->SetGraphicsRootSignature(command_list, root_signature);
	for (int i = 0; i < TEXTURE_COUNT; ++i) {
		current_textures[i] = NULL;
	}
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
	DXGI_FORMAT dxgi_format = desc.Format;
	int _format_size = format_size(dxgi_format);
	int row_pitch = render_target->width * _format_size;
	int align = row_pitch % D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
	if (align != 0) {
		row_pitch = row_pitch + (D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - align);
	}

	// Create readback buffer
	if (render_target->impl.readback == NULL) {
		D3D12_HEAP_PROPERTIES heap_properties = {
			.Type = D3D12_HEAP_TYPE_READBACK,
			.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
			.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
			.CreationNodeMask = 1,
			.VisibleNodeMask = 1,
		};

		D3D12_RESOURCE_DESC resource_desc = {
			.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
			.Alignment = 0,
			.Width = row_pitch * render_target->height,
			.Height = 1,
			.DepthOrArraySize = 1,
			.MipLevels = 1,
			.Format = DXGI_FORMAT_UNKNOWN,
			.SampleDesc.Count = 1,
			.SampleDesc.Quality = 0,
			.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
			.Flags = D3D12_RESOURCE_FLAG_NONE,
		};

		device->lpVtbl->CreateCommittedResource(device, &heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc, D3D12_RESOURCE_STATE_COMMON, NULL,
												&IID_ID3D12Resource, &render_target->impl.readback);
	}

	// Copy render target to readback buffer
	D3D12_RESOURCE_BARRIER barrier = {
		.Transition.pResource = render_target->impl.image,
		.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
		.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
		.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE,
		.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
	};
	command_list->lpVtbl->ResourceBarrier(command_list, 1, &barrier);

	D3D12_TEXTURE_COPY_LOCATION source = {
		.pResource = render_target->impl.image,
		.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
		.SubresourceIndex = 0,
	};

	D3D12_TEXTURE_COPY_LOCATION dest = {
		.pResource = render_target->impl.readback,
		.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
		.PlacedFootprint.Offset = 0,
		.PlacedFootprint.Footprint.Format = dxgi_format,
		.PlacedFootprint.Footprint.Width = render_target->width,
		.PlacedFootprint.Footprint.Height = render_target->height,
		.PlacedFootprint.Footprint.Depth = 1,
		.PlacedFootprint.Footprint.RowPitch = row_pitch,
	};

	command_list->lpVtbl->CopyTextureRegion(command_list , &dest, 0, 0, 0, &source, NULL);

	barrier = (D3D12_RESOURCE_BARRIER){
		.Transition.pResource = render_target->impl.image,
		.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
		.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
		.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE,
		.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
	};
	command_list->lpVtbl->ResourceBarrier(command_list, 1, &barrier);

	gpu_flush();

	// Read buffer
	void *p;
	render_target->impl.readback->lpVtbl->Map(render_target->impl.readback, 0, NULL, &p);
	memcpy(data, p, render_target->width * render_target->height * _format_size);
	render_target->impl.readback->lpVtbl->Unmap(render_target->impl.readback, 0, NULL);
}

void gpu_set_texture(int unit, gpu_texture_t *texture) {
	current_textures[unit] = texture;
}

void gpu_pipeline_init(gpu_pipeline_t *pipe) {
	gpu_internal_pipeline_init(pipe);
}

void gpu_pipeline_destroy(gpu_pipeline_t *pipe) {
	if (pipe->impl.pso != NULL) {
		pipe->impl.pso->lpVtbl->Release(pipe->impl.pso);
		pipe->impl.pso = NULL;
	}
}

void gpu_pipeline_compile(gpu_pipeline_t *pipe) {
	int vertex_attribute_count = pipe->input_layout->size;
	D3D12_INPUT_ELEMENT_DESC *vertex_desc = (D3D12_INPUT_ELEMENT_DESC *)alloca(sizeof(D3D12_INPUT_ELEMENT_DESC) * vertex_attribute_count);
	ZeroMemory(vertex_desc, sizeof(D3D12_INPUT_ELEMENT_DESC) * vertex_attribute_count);

	for (int i = 0; i < pipe->input_layout->size; ++i) {
		vertex_desc[i].SemanticName = "TEXCOORD";
		vertex_desc[i].SemanticIndex = i;
		vertex_desc[i].InputSlot = 0;
		vertex_desc[i].AlignedByteOffset = (i == 0) ? 0 : D3D12_APPEND_ALIGNED_ELEMENT;
		vertex_desc[i].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
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

	const D3D12_DEPTH_STENCILOP_DESC default_stencil_op = {
		D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_NEVER
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {
		.VS.BytecodeLength = pipe->vertex_shader->impl.length,
		.VS.pShaderBytecode = pipe->vertex_shader->impl.data,
		.PS.BytecodeLength = pipe->fragment_shader->impl.length,
		.PS.pShaderBytecode = pipe->fragment_shader->impl.data,
		.pRootSignature = root_signature,
		.NumRenderTargets = pipe->color_attachment_count,
		.DSVFormat = DXGI_FORMAT_UNKNOWN,
		.InputLayout.NumElements = vertex_attribute_count,
		.InputLayout.pInputElementDescs = vertex_desc,
		.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID,
		.RasterizerState.CullMode = convert_cull_mode(pipe->cull_mode),
		.RasterizerState.FrontCounterClockwise = FALSE,
		.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS,
		.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
		.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
		.RasterizerState.DepthClipEnable = TRUE,
		.RasterizerState.MultisampleEnable = FALSE,
		.RasterizerState.AntialiasedLineEnable = FALSE,
		.RasterizerState.ForcedSampleCount = 0,
		.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF,
		.BlendState.AlphaToCoverageEnable = FALSE,
		.BlendState.IndependentBlendEnable = FALSE,
		.DepthStencilState.DepthEnable = TRUE,
		.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL,
		.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS,
		.DepthStencilState.StencilEnable = FALSE,
		.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK,
		.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK,
		.DepthStencilState.DepthEnable = pipe->depth_mode != GPU_COMPARE_MODE_ALWAYS,
		.DepthStencilState.DepthWriteMask = pipe->depth_write ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO,
		.DepthStencilState.DepthFunc = convert_compare_mode(pipe->depth_mode),
		.DepthStencilState.StencilEnable = false,
		.DSVFormat = DXGI_FORMAT_D32_FLOAT,
		.SampleDesc.Count = 1,
		.SampleMask = 0xFFFFFFFF,
		.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		.DepthStencilState.FrontFace = default_stencil_op,
		.DepthStencilState.BackFace = default_stencil_op,
	};

	for (int i = 0; i < pipe->color_attachment_count; ++i) {
		psoDesc.RTVFormats[i] = convert_format(pipe->color_attachment[i]);
	}

	psoDesc.BlendState.IndependentBlendEnable = true;
	for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i) {
		psoDesc.BlendState.RenderTarget[i].BlendEnable = pipe->blend_source != GPU_BLEND_ONE || pipe->blend_destination != GPU_BLEND_ZERO ||
												   		 pipe->alpha_blend_source != GPU_BLEND_ONE || pipe->alpha_blend_destination != GPU_BLEND_ZERO;
		psoDesc.BlendState.RenderTarget[i].SrcBlend = convert_blend_factor(pipe->blend_source);
		psoDesc.BlendState.RenderTarget[i].DestBlend = convert_blend_factor(pipe->blend_destination);
		psoDesc.BlendState.RenderTarget[i].BlendOp = convert_blend_operation(pipe->blend_operation);
		psoDesc.BlendState.RenderTarget[i].SrcBlendAlpha = convert_blend_factor(pipe->alpha_blend_source);
		psoDesc.BlendState.RenderTarget[i].DestBlendAlpha = convert_blend_factor(pipe->alpha_blend_destination);
		psoDesc.BlendState.RenderTarget[i].BlendOpAlpha = convert_blend_operation(pipe->alpha_blend_operation);
		psoDesc.BlendState.RenderTarget[i].RenderTargetWriteMask =
			(((pipe->color_write_mask_red[i] ? D3D12_COLOR_WRITE_ENABLE_RED : 0) |
			(pipe->color_write_mask_green[i] ? D3D12_COLOR_WRITE_ENABLE_GREEN : 0)) |
			(pipe->color_write_mask_blue[i] ? D3D12_COLOR_WRITE_ENABLE_BLUE : 0)) |
			(pipe->color_write_mask_alpha[i] ? D3D12_COLOR_WRITE_ENABLE_ALPHA : 0);
	}

	device->lpVtbl->CreateGraphicsPipelineState(device, &psoDesc, &IID_ID3D12PipelineState, &pipe->impl.pso);
}

void gpu_shader_init(gpu_shader_t *shader, const void *_data, size_t length, gpu_shader_type_t type) {
	uint8_t *data = (uint8_t *)_data;
	shader->impl.length = (int)length;
	shader->impl.data = (uint8_t *)malloc(shader->impl.length);
	memcpy(shader->impl.data, data, shader->impl.length);
}

void gpu_shader_destroy(gpu_shader_t *shader) {
	free(shader->impl.data);
}

void gpu_texture_init_from_bytes(gpu_texture_t *texture, void *data, int width, int height, gpu_texture_format_t format) {
	memset(&texture->impl, 0, sizeof(texture->impl));
	texture->width = width;
	texture->height = height;
	texture->format = format;
	texture->state = GPU_TEXTURE_STATE_SHADER_RESOURCE;
	DXGI_FORMAT d3d_format = convert_format(format);
	int _format_size = format_size(d3d_format);

	D3D12_HEAP_PROPERTIES heap_properties = {
		.Type = D3D12_HEAP_TYPE_DEFAULT,
		.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
		.CreationNodeMask = 1,
		.VisibleNodeMask = 1,
	};

	D3D12_RESOURCE_DESC resource_desc = {
		.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		.Alignment = 0,
		.Width = texture->width,
		.Height = texture->height,
		.DepthOrArraySize = 1,
		.MipLevels = 1,
		.Format = d3d_format,
		.SampleDesc.Count = 1,
		.SampleDesc.Quality = 0,
		.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
		.Flags = D3D12_RESOURCE_FLAG_NONE,
	};

	device->lpVtbl->CreateCommittedResource(device, &heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, NULL, &IID_ID3D12Resource, &texture->impl.image);

	D3D12_HEAP_PROPERTIES heap_properties_upload = {
		.Type = D3D12_HEAP_TYPE_UPLOAD,
		.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
		.CreationNodeMask = 1,
		.VisibleNodeMask = 1,
	};

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
	UINT64 upload_buffer_size;
	device->lpVtbl->GetCopyableFootprints(device, &resource_desc, 0, 1, 0, &footprint, NULL, NULL, &upload_buffer_size);

	D3D12_RESOURCE_DESC resource_desc_upload = {
		.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
		.Alignment = 0,
		.Width = upload_buffer_size,
		.Height = 1,
		.DepthOrArraySize = 1,
		.MipLevels = 1,
		.Format = DXGI_FORMAT_UNKNOWN,
		.SampleDesc.Count = 1,
		.SampleDesc.Quality = 0,
		.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		.Flags = D3D12_RESOURCE_FLAG_NONE,
	};

	struct ID3D12Resource *upload_image;
	device->lpVtbl->CreateCommittedResource(device, &heap_properties_upload, D3D12_HEAP_FLAG_NONE, &resource_desc_upload,
		D3D12_RESOURCE_STATE_GENERIC_READ, NULL, &IID_ID3D12Resource, &upload_image);

	int stride = (int)ceilf(upload_buffer_size / (float)(height * D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)) * D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;

	BYTE *pixel;
	upload_image->lpVtbl->Map(upload_image, 0, NULL, (void **)&pixel);
	for (int y = 0; y < texture->height; ++y) {
		memcpy(&pixel[y * stride], &((uint8_t *)data)[y * texture->width * _format_size], texture->width * _format_size);
	}
	upload_image->lpVtbl->Unmap(upload_image, 0, NULL);

	D3D12_DESCRIPTOR_HEAP_DESC descriptor_heap_desc = {
		.NumDescriptors = 1,
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		.NodeMask = 0,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
	};
	device->lpVtbl->CreateDescriptorHeap(device, &descriptor_heap_desc, &IID_ID3D12DescriptorHeap, &texture->impl.srv_descriptor_heap);

	D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {
		.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
		.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
		.Format = d3d_format,
		.Texture2D.MipLevels = 1,
		.Texture2D.MostDetailedMip = 0,
		.Texture2D.ResourceMinLODClamp = 0.0f,
	};

	D3D12_CPU_DESCRIPTOR_HANDLE handle;
	texture->impl.srv_descriptor_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(texture->impl.srv_descriptor_heap, &handle);
	device->lpVtbl->CreateShaderResourceView(device, texture->impl.image, &srv_desc, handle);

	D3D12_RESOURCE_BARRIER barrier = {
		.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
		.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
		.Transition.pResource = texture->impl.image,
		.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST,
		.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
	};
	command_list->lpVtbl->ResourceBarrier(command_list, 1, &barrier);

	D3D12_TEXTURE_COPY_LOCATION source = {
		.pResource = upload_image,
		.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
		.PlacedFootprint = footprint,
	};

	D3D12_TEXTURE_COPY_LOCATION destination = {
		.pResource = texture->impl.image,
		.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
		.SubresourceIndex = 0,
	};

	command_list->lpVtbl->CopyTextureRegion(command_list, &destination, 0, 0, 0, &source, NULL);

	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST,
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
	command_list->lpVtbl->ResourceBarrier(command_list, 1, &barrier);

	// TODO:
	gpu_flush();
	upload_image->lpVtbl->Release(upload_image);
}

void gpu_texture_destroy(gpu_texture_t *render_target) {
	if (render_target->impl.image != NULL) {
		render_target->impl.image->lpVtbl->Release(render_target->impl.image);
	}
	if (render_target->impl.rtv_descriptor_heap != NULL) {
		render_target->impl.rtv_descriptor_heap->lpVtbl->Release(render_target->impl.rtv_descriptor_heap);
	}
	if (render_target->impl.srv_descriptor_heap != NULL) {
		render_target->impl.srv_descriptor_heap->lpVtbl->Release(render_target->impl.srv_descriptor_heap);
	}
	if (render_target->impl.readback != NULL) {
		render_target->impl.readback->lpVtbl->Release(render_target->impl.readback);
	}
}

void gpu_render_target_init(gpu_texture_t *target, int width, int height, gpu_texture_format_t format) {
	gpu_render_target_init2(target, width, height, format, -1);
}

void gpu_vertex_buffer_init(gpu_buffer_t *buffer, int count, gpu_vertex_structure_t *structure) {
	buffer->count = count;

	buffer->stride = 0;
	for (int i = 0; i < structure->size; ++i) {
		buffer->stride += gpu_vertex_data_size(structure->elements[i].data);
	}

	int upload_buffer_size = buffer->stride * buffer->count;

	D3D12_HEAP_PROPERTIES heap_properties = {
		.Type = D3D12_HEAP_TYPE_UPLOAD,
		.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
		.CreationNodeMask = 1,
		.VisibleNodeMask = 1,
	};

	D3D12_RESOURCE_DESC resource_desc = {
		.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
		.Alignment = 0,
		.Width = upload_buffer_size,
		.Height = 1,
		.DepthOrArraySize = 1,
		.MipLevels = 1,
		.Format = DXGI_FORMAT_UNKNOWN,
		.SampleDesc.Count = 1,
		.SampleDesc.Quality = 0,
		.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		.Flags = D3D12_RESOURCE_FLAG_NONE,
	};

	device->lpVtbl->CreateCommittedResource(device, &heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
											&IID_ID3D12Resource, &buffer->impl.buffer);

	buffer->impl.vertex_buffer_view.BufferLocation = buffer->impl.buffer->lpVtbl->GetGPUVirtualAddress(buffer->impl.buffer);
	buffer->impl.vertex_buffer_view.SizeInBytes = upload_buffer_size;
	buffer->impl.vertex_buffer_view.StrideInBytes = buffer->stride;
}

float *gpu_vertex_buffer_lock(gpu_buffer_t *buffer) {
	D3D12_RANGE range = {
		.Begin = 0,
		.End = buffer->count * buffer->stride,
	};
	void *p;
	buffer->impl.buffer->lpVtbl->Map(buffer->impl.buffer, 0, &range, &p);
	return (float *)p;
}

void gpu_vertex_buffer_unlock(gpu_buffer_t *buffer) {
	D3D12_RANGE range = {
		.Begin = 0,
		.End = buffer->count * buffer->stride,
	};
	buffer->impl.buffer->lpVtbl->Unmap(buffer->impl.buffer, 0, &range);
}

void gpu_constant_buffer_init(gpu_buffer_t *buffer, int size) {
	buffer->count = size;
	buffer->data = NULL;

	D3D12_HEAP_PROPERTIES heap_properties = {
		.Type = D3D12_HEAP_TYPE_UPLOAD,
		.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
		.CreationNodeMask = 1,
		.VisibleNodeMask = 1,
	};

	D3D12_RESOURCE_DESC resource_desc = {
		.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
		.Alignment = 0,
		.Width = size,
		.Height = 1,
		.DepthOrArraySize = 1,
		.MipLevels = 1,
		.Format = DXGI_FORMAT_UNKNOWN,
		.SampleDesc.Count = 1,
		.SampleDesc.Quality = 0,
		.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		.Flags = D3D12_RESOURCE_FLAG_NONE,
	};

	device->lpVtbl->CreateCommittedResource(device, &heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, &IID_ID3D12Resource, &buffer->impl.buffer);
}

void gpu_constant_buffer_destroy(gpu_buffer_t *buffer) {
	buffer->impl.buffer->lpVtbl->Release(buffer->impl.buffer);
}

void gpu_constant_buffer_lock(gpu_buffer_t *buffer, int start, int count) {
	buffer->impl.last_start = start;
	buffer->impl.last_count = count;
	D3D12_RANGE range = {
		.Begin = start,
		.End = start + count,
	};
	uint8_t *p;
	buffer->impl.buffer->lpVtbl->Map(buffer->impl.buffer, 0, &range, (void **)&p);
	buffer->data = &p[start];
}

void gpu_constant_buffer_unlock(gpu_buffer_t *buffer) {
	D3D12_RANGE range = {
		.Begin = buffer->impl.last_start,
		.End = buffer->impl.last_start + buffer->impl.last_count,
	};
	buffer->impl.buffer->lpVtbl->Unmap(buffer->impl.buffer, 0, &range);
	buffer->data = NULL;
}

void gpu_index_buffer_init(gpu_buffer_t *buffer, int count) {
	buffer->count = count;

	int upload_buffer_size = sizeof(uint32_t) * count;

	D3D12_HEAP_PROPERTIES heap_properties = {
		.Type = D3D12_HEAP_TYPE_UPLOAD,
		.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
		.CreationNodeMask = 1,
		.VisibleNodeMask = 1,
	};

	D3D12_RESOURCE_DESC resource_desc = {
		.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
		.Alignment = 0,
		.Width = upload_buffer_size,
		.Height = 1,
		.DepthOrArraySize = 1,
		.MipLevels = 1,
		.Format = DXGI_FORMAT_UNKNOWN,
		.SampleDesc.Count = 1,
		.SampleDesc.Quality = 0,
		.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		.Flags = D3D12_RESOURCE_FLAG_NONE,
	};

	device->lpVtbl->CreateCommittedResource(device, &heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc,
		D3D12_RESOURCE_STATE_GENERIC_READ, NULL, &IID_ID3D12Resource, &buffer->impl.buffer);

	buffer->impl.index_buffer_view.BufferLocation = buffer->impl.buffer->lpVtbl->GetGPUVirtualAddress(buffer->impl.buffer);
	buffer->impl.index_buffer_view.SizeInBytes = upload_buffer_size;
	buffer->impl.index_buffer_view.Format = DXGI_FORMAT_R32_UINT;
}

void gpu_buffer_destroy(gpu_buffer_t *buffer) {
	buffer->impl.buffer->lpVtbl->Release(buffer->impl.buffer);
	buffer->impl.buffer = NULL;
}

void *gpu_index_buffer_lock(gpu_buffer_t *buffer) {
	D3D12_RANGE range = {
		.Begin = 0,
		.End = buffer->count * 4,
	};
	void *p;
	buffer->impl.buffer->lpVtbl->Map(buffer->impl.buffer, 0, &range, &p);
	return p;
}

void gpu_index_buffer_unlock(gpu_buffer_t *buffer) {
	D3D12_RANGE range = {
		.Begin = 0,
		.End = buffer->count * 4,
	};
	buffer->impl.buffer->lpVtbl->Unmap(buffer->impl.buffer, 0, &range);
}

static const wchar_t *hit_group_name = L"hitgroup";
static const wchar_t *raygen_shader_name = L"raygeneration";
static const wchar_t *closesthit_shader_name = L"closesthit";
static const wchar_t *miss_shader_name = L"miss";

typedef struct inst {
	iron_matrix4x4_t m;
	int i;
} inst_t;

static ID3D12Device5 *dxrDevice = NULL;
static ID3D12GraphicsCommandList4 *dxrCommandList = NULL;
static ID3D12RootSignature *dxrRootSignature = NULL;
static ID3D12DescriptorHeap *descriptorHeap = NULL;
static gpu_raytrace_acceleration_structure_t *accel;
static gpu_raytrace_pipeline_t *pipeline;
static gpu_texture_t *output = NULL;
static D3D12_CPU_DESCRIPTOR_HANDLE outputCpuDescriptor;
static D3D12_GPU_DESCRIPTOR_HANDLE outputDescriptorHandle;
static D3D12_GPU_DESCRIPTOR_HANDLE vbgpuDescriptorHandle;
static D3D12_GPU_DESCRIPTOR_HANDLE ibgpuDescriptorHandle;
static D3D12_GPU_DESCRIPTOR_HANDLE tex0gpuDescriptorHandle;
static D3D12_GPU_DESCRIPTOR_HANDLE tex1gpuDescriptorHandle;
static D3D12_GPU_DESCRIPTOR_HANDLE tex2gpuDescriptorHandle;
static D3D12_GPU_DESCRIPTOR_HANDLE texenvgpuDescriptorHandle;
static D3D12_GPU_DESCRIPTOR_HANDLE texsobolgpuDescriptorHandle;
static D3D12_GPU_DESCRIPTOR_HANDLE texscramblegpuDescriptorHandle;
static D3D12_GPU_DESCRIPTOR_HANDLE texrankgpuDescriptorHandle;
static int descriptorsAllocated = 0;
static UINT descriptorSize;
static gpu_buffer_t *vb[16];
static gpu_buffer_t *vb_last[16];
static gpu_buffer_t *ib[16];
static int vb_count = 0;
static int vb_count_last = 0;
static inst_t instances[1024];
static int instances_count = 0;

void gpu_raytrace_pipeline_init(gpu_raytrace_pipeline_t *pipeline, void *ray_shader, int ray_shader_size, gpu_buffer_t *constant_buffer) {
	output = NULL;
	descriptorsAllocated = 0;
	pipeline->_constant_buffer = constant_buffer;
	// Descriptor heap
	// Allocate a heap for 3 descriptors:
	// 2 - bottom and top level acceleration structure
	// 1 - raytracing output texture SRV
	D3D12_DESCRIPTOR_HEAP_DESC descriptor_heap_desc = {
		.NumDescriptors = 12,
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
		.NodeMask = 0,
	};
	if (descriptorHeap != NULL) {
		descriptorHeap->lpVtbl->Release(descriptorHeap);
	}
	device->lpVtbl->CreateDescriptorHeap(device, &descriptor_heap_desc, &IID_ID3D12DescriptorHeap, &descriptorHeap);
	descriptorSize = device->lpVtbl->GetDescriptorHandleIncrementSize(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Device
	if (dxrDevice != NULL) {
		dxrDevice->lpVtbl->Release(dxrDevice);
	}
	device->lpVtbl->QueryInterface(device, &IID_ID3D12Device5, &dxrDevice);
	if (dxrCommandList != NULL) {
		dxrCommandList->lpVtbl->Release(dxrCommandList);
	}
	command_list->lpVtbl->QueryInterface(command_list , &IID_ID3D12GraphicsCommandList4, &dxrCommandList);

	// Root signatures
	// This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.
	D3D12_DESCRIPTOR_RANGE UAVDescriptor = {
		.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
		.NumDescriptors = 1,
		.BaseShaderRegister = 0,
		.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND,
	};

	D3D12_DESCRIPTOR_RANGE SRVDescriptorA = {
		.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		.NumDescriptors = 1,
		.BaseShaderRegister = 1,
		.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND,
	};

	D3D12_DESCRIPTOR_RANGE SRVDescriptorB = {
		.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		.NumDescriptors = 1,
		.BaseShaderRegister = 2,
		.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND,
	};

	D3D12_DESCRIPTOR_RANGE SRVDescriptor0 = {
		.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		.NumDescriptors = 1,
		.BaseShaderRegister = 3,
		.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND,
	};

	D3D12_DESCRIPTOR_RANGE SRVDescriptor1 = {
		.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		.NumDescriptors = 1,
		.BaseShaderRegister = 4,
		.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND,
	};

	D3D12_DESCRIPTOR_RANGE SRVDescriptor2 = {
		.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		.NumDescriptors = 1,
		.BaseShaderRegister = 5,
		.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND,
	};

	D3D12_DESCRIPTOR_RANGE SRVDescriptorEnv = {
		.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		.NumDescriptors = 1,
		.BaseShaderRegister = 6,
		.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND,
	};

	D3D12_DESCRIPTOR_RANGE SRVDescriptorSobol = {
		.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		.NumDescriptors = 1,
		.BaseShaderRegister = 7,
		.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND,
	};

	D3D12_DESCRIPTOR_RANGE SRVDescriptorScramble = {
		.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		.NumDescriptors = 1,
		.BaseShaderRegister = 8,
		.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND,
	};

	D3D12_DESCRIPTOR_RANGE SRVDescriptorRank = {
		.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		.NumDescriptors = 1,
		.BaseShaderRegister = 9,
		.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND,
	};

	D3D12_ROOT_PARAMETER rootParameters[12] = {};
	// Output view
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[0].DescriptorTable.pDescriptorRanges = &UAVDescriptor;
	// Acceleration structure
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[1].Descriptor.ShaderRegister = 0;
	// Constant buffer
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[2].DescriptorTable.pDescriptorRanges = &SRVDescriptorA;
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[3].DescriptorTable.pDescriptorRanges = &SRVDescriptorB;
	rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[4].Descriptor.ShaderRegister = 0;
	rootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[5].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[5].DescriptorTable.pDescriptorRanges = &SRVDescriptor0;
	rootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[6].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[6].DescriptorTable.pDescriptorRanges = &SRVDescriptor1;
	rootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[7].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[7].DescriptorTable.pDescriptorRanges = &SRVDescriptor2;
	rootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[8].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[8].DescriptorTable.pDescriptorRanges = &SRVDescriptorEnv;
	rootParameters[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[9].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[9].DescriptorTable.pDescriptorRanges = &SRVDescriptorSobol;
	rootParameters[10].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[10].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[10].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[10].DescriptorTable.pDescriptorRanges = &SRVDescriptorScramble;
	rootParameters[11].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[11].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[11].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[11].DescriptorTable.pDescriptorRanges = &SRVDescriptorRank;

	D3D12_ROOT_SIGNATURE_DESC dxrRootSignatureDesc = {
		.NumParameters = ARRAYSIZE(rootParameters),
		.pParameters = rootParameters,
	};
	ID3DBlob *blob = NULL;
	ID3DBlob *error = NULL;
	D3D12SerializeRootSignature(&dxrRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error);
	if (dxrRootSignature != NULL) {
		dxrRootSignature->lpVtbl->Release(dxrRootSignature);
	}
	device->lpVtbl->CreateRootSignature(device, 1, blob->lpVtbl->GetBufferPointer(blob), blob->lpVtbl->GetBufferSize(blob), &IID_ID3D12RootSignature,
										&dxrRootSignature);

	// Pipeline
	D3D12_STATE_OBJECT_DESC raytracingPipeline = {
		.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE,
	};

	D3D12_SHADER_BYTECODE shaderBytecode = {
		.pShaderBytecode = ray_shader,
		.BytecodeLength = ray_shader_size,
	};

	D3D12_DXIL_LIBRARY_DESC dxilLibrary = {
		.DXILLibrary = shaderBytecode,
	};
	D3D12_EXPORT_DESC exports[3] = {0};
	exports[0].Name = raygen_shader_name;
	exports[1].Name = closesthit_shader_name;
	exports[2].Name = miss_shader_name;
	dxilLibrary.pExports = exports;
	dxilLibrary.NumExports = 3;

	D3D12_HIT_GROUP_DESC hitGroup = {
		.ClosestHitShaderImport = closesthit_shader_name,
		.HitGroupExport = hit_group_name,
		.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES,
	};

	D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {
		.MaxPayloadSizeInBytes = 10 * sizeof(float), // float4 color
		.MaxAttributeSizeInBytes = 8 * sizeof(float), // float2 barycentrics
	};

	D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {
		.MaxTraceRecursionDepth = 1, // ~ primary rays only
	};

	D3D12_STATE_SUBOBJECT subobjects[5] = {};
	subobjects[0].Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
	subobjects[0].pDesc = &dxilLibrary;
	subobjects[1].Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
	subobjects[1].pDesc = &hitGroup;
	subobjects[2].Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
	subobjects[2].pDesc = &shaderConfig;
	subobjects[3].Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
	subobjects[3].pDesc = &dxrRootSignature;
	subobjects[4].Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
	subobjects[4].pDesc = &pipelineConfig;
	raytracingPipeline.NumSubobjects = 5;
	raytracingPipeline.pSubobjects = subobjects;

	dxrDevice->lpVtbl->CreateStateObject(dxrDevice, &raytracingPipeline, &IID_ID3D12StateObject, &pipeline->impl.dxr_state);

	// Shader tables
	// Get shader identifiers
	ID3D12StateObjectProperties *stateObjectProps = NULL;
	pipeline->impl.dxr_state->lpVtbl->QueryInterface(pipeline->impl.dxr_state , &IID_ID3D12StateObjectProperties, &stateObjectProps);
	const void *rayGenShaderId = stateObjectProps->lpVtbl->GetShaderIdentifier(stateObjectProps, raygen_shader_name);
	const void *missShaderId = stateObjectProps->lpVtbl->GetShaderIdentifier(stateObjectProps, miss_shader_name);
	const void *hitGroupShaderId = stateObjectProps->lpVtbl->GetShaderIdentifier(stateObjectProps, hit_group_name);
	UINT shaderIdSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	int align = D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;

	// Ray gen shader table
	{
		UINT size = shaderIdSize + constant_buffer->count;
		UINT shaderRecordSize = (size + (align - 1)) & ~(align - 1);
		D3D12_RESOURCE_DESC bufferDesc = {
			.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
			.Width = shaderRecordSize,
			.Height = 1,
			.DepthOrArraySize = 1,
			.MipLevels = 1,
			.Format = DXGI_FORMAT_UNKNOWN,
			.SampleDesc.Count = 1,
			.SampleDesc.Quality = 0,
			.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		};
		D3D12_HEAP_PROPERTIES uploadHeapProperties = {
			.Type = D3D12_HEAP_TYPE_UPLOAD,
			.CreationNodeMask = 1,
			.VisibleNodeMask = 1,
		};

		device->lpVtbl->CreateCommittedResource(device, &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
										&IID_ID3D12Resource, &pipeline->impl.raygen_shader_table);

		D3D12_RANGE rstRange = {
			.Begin = 0,
			.End = 0,
		};
		uint8_t *byteDest;
		pipeline->impl.raygen_shader_table->lpVtbl->Map(pipeline->impl.raygen_shader_table, 0, &rstRange, (void **)(&byteDest));

		D3D12_RANGE cbRange = {
			.Begin = 0,
			.End = constant_buffer->count,
		};
		void *constantBufferData;
		constant_buffer->impl.buffer->lpVtbl->Map(constant_buffer->impl.buffer, 0, &cbRange, (void **)&constantBufferData);
		memcpy(byteDest, rayGenShaderId, size);
		memcpy(byteDest + size, constantBufferData, constant_buffer->count);
		pipeline->impl.raygen_shader_table->lpVtbl->Unmap(pipeline->impl.raygen_shader_table, 0, NULL);
	}

	// Miss shader table
	{
		UINT size = shaderIdSize;
		UINT shaderRecordSize = (size + (align - 1)) & ~(align - 1);
		D3D12_RESOURCE_DESC bufferDesc = {
			.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
			.Width = shaderRecordSize,
			.Height = 1,
			.DepthOrArraySize = 1,
			.MipLevels = 1,
			.Format = DXGI_FORMAT_UNKNOWN,
			.SampleDesc.Count = 1,
			.SampleDesc.Quality = 0,
			.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		};
		D3D12_HEAP_PROPERTIES uploadHeapProperties = {
			.Type = D3D12_HEAP_TYPE_UPLOAD,
			.CreationNodeMask = 1,
			.VisibleNodeMask = 1,
		};

		device->lpVtbl->CreateCommittedResource(device, &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
												&IID_ID3D12Resource, &pipeline->impl.miss_shader_table);

		D3D12_RANGE mstRange = {
			.Begin = 0,
			.End = 0,
		};
		uint8_t *byteDest;
		pipeline->impl.miss_shader_table->lpVtbl->Map(pipeline->impl.miss_shader_table, 0, &mstRange, (void **)(&byteDest));
		memcpy(byteDest, missShaderId, size);
		pipeline->impl.miss_shader_table->lpVtbl->Unmap(pipeline->impl.miss_shader_table, 0, NULL);
	}

	// Hit group shader table
	{
		UINT size = shaderIdSize;
		UINT shaderRecordSize = (size + (align - 1)) & ~(align - 1);
		D3D12_RESOURCE_DESC bufferDesc = {
			.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
			.Width = shaderRecordSize,
			.Height = 1,
			.DepthOrArraySize = 1,
			.MipLevels = 1,
			.Format = DXGI_FORMAT_UNKNOWN,
			.SampleDesc.Count = 1,
			.SampleDesc.Quality = 0,
			.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		};
		D3D12_HEAP_PROPERTIES uploadHeapProperties = {
			.Type = D3D12_HEAP_TYPE_UPLOAD,
			.CreationNodeMask = 1,
			.VisibleNodeMask = 1,
		};

		device->lpVtbl->CreateCommittedResource(device, &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
												&IID_ID3D12Resource, &pipeline->impl.hitgroup_shader_table);

		D3D12_RANGE hstRange = {
			.Begin = 0,
			.End = 0,
		};
		uint8_t *byteDest;
		pipeline->impl.hitgroup_shader_table->lpVtbl->Map(pipeline->impl.hitgroup_shader_table, 0, &hstRange, (void **)(&byteDest));
		memcpy(byteDest, hitGroupShaderId, size);
		pipeline->impl.hitgroup_shader_table->lpVtbl->Unmap(pipeline->impl.hitgroup_shader_table, 0, NULL);
	}

	// Output descriptor
	D3D12_CPU_DESCRIPTOR_HANDLE handle;
	descriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(descriptorHeap, &handle);
	outputCpuDescriptor.ptr = handle.ptr + (INT64)(descriptorsAllocated) * (UINT64)(descriptorSize);

	descriptorHeap->lpVtbl->GetGPUDescriptorHandleForHeapStart(descriptorHeap, &handle);
	int descriptorHeapIndex = descriptorsAllocated++;
	outputDescriptorHandle.ptr = handle.ptr + (INT64)(descriptorHeapIndex) * (UINT64)(descriptorSize);
}

void gpu_raytrace_pipeline_destroy(gpu_raytrace_pipeline_t *pipeline) {
	pipeline->impl.dxr_state->lpVtbl->Release(pipeline->impl.dxr_state);
	pipeline->impl.raygen_shader_table->lpVtbl->Release(pipeline->impl.raygen_shader_table);
	pipeline->impl.miss_shader_table->lpVtbl->Release(pipeline->impl.miss_shader_table);
	pipeline->impl.hitgroup_shader_table->lpVtbl->Release(pipeline->impl.hitgroup_shader_table);
}

UINT create_srv_vb(gpu_buffer_t *vb, UINT numElements, UINT elementSize) {
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {
		.ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
		.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
		.Buffer.NumElements = numElements,
	};

	if (elementSize == 0) {
		srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
		srvDesc.Buffer.StructureByteStride = 0;
	}
	else {
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		srvDesc.Buffer.StructureByteStride = elementSize;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE handle;
	descriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(descriptorHeap, &handle);

	D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor = {
		.ptr = handle.ptr + (INT64)(descriptorsAllocated) * (UINT64)(descriptorSize),
	};
	UINT descriptorIndex = descriptorsAllocated++;
	device->lpVtbl->CreateShaderResourceView(device, vb->impl.buffer, &srvDesc, cpuDescriptor);
	descriptorHeap->lpVtbl->GetGPUDescriptorHandleForHeapStart(descriptorHeap, &handle);
	vbgpuDescriptorHandle.ptr = handle.ptr + (INT64)(descriptorIndex) * (UINT64)(descriptorSize);

	return descriptorIndex;
}

UINT create_srv_ib(gpu_buffer_t *ib, UINT numElements, UINT elementSize) {
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {
		.ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
		.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
		.Buffer.NumElements = numElements,
	};

	if (elementSize == 0) {
		srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
		srvDesc.Buffer.StructureByteStride = 0;
	}
	else {
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		srvDesc.Buffer.StructureByteStride = elementSize;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE handle;
	descriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(descriptorHeap, &handle);

	D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor = {
		.ptr = handle.ptr + (INT64)(descriptorsAllocated) * (UINT64)(descriptorSize),
	};

	UINT descriptorIndex = descriptorsAllocated++;
	descriptorHeap->lpVtbl->GetGPUDescriptorHandleForHeapStart(descriptorHeap, &handle);
	device->lpVtbl->CreateShaderResourceView(device, ib->impl.buffer, &srvDesc, cpuDescriptor);
	ibgpuDescriptorHandle.ptr = handle.ptr + (INT64)(descriptorIndex) * (UINT64)(descriptorSize);

	return descriptorIndex;
}

void gpu_raytrace_acceleration_structure_init(gpu_raytrace_acceleration_structure_t *accel) {
	vb_count = 0;
	instances_count = 0;
}

void gpu_raytrace_acceleration_structure_add(gpu_raytrace_acceleration_structure_t *accel, gpu_buffer_t *_vb, gpu_buffer_t *_ib,
	iron_matrix4x4_t _transform) {

	int vb_i = -1;
	for (int i = 0; i < vb_count; ++i) {
		if (_vb == vb[i]) {
			vb_i = i;
			break;
		}
	}
	if (vb_i == -1) {
		vb_i = vb_count;
		vb[vb_count] = _vb;
		ib[vb_count] = _ib;
		vb_count++;
	}

	inst_t inst = { .i = vb_i, .m =  _transform };
	instances[instances_count] = inst;
	instances_count++;
}

void _gpu_raytrace_acceleration_structure_destroy_bottom(gpu_raytrace_acceleration_structure_t *accel) {
	for (int i = 0; i < vb_count_last; ++i) {
		accel->impl.bottom_level_accel[i]->lpVtbl->Release(accel->impl.bottom_level_accel[i]);
	}
}

void _gpu_raytrace_acceleration_structure_destroy_top(gpu_raytrace_acceleration_structure_t *accel) {
	accel->impl.top_level_accel->lpVtbl->Release(accel->impl.top_level_accel);
}

void gpu_raytrace_acceleration_structure_build(gpu_raytrace_acceleration_structure_t *accel, gpu_buffer_t *_vb_full, gpu_buffer_t *_ib_full) {

	bool build_bottom = false;
	for (int i = 0; i < 16; ++i) {
		if (vb_last[i] != vb[i]) {
			build_bottom = true;
		}
		vb_last[i] = vb[i];
	}

	if (vb_count_last > 0) {
		if (build_bottom) {
			_gpu_raytrace_acceleration_structure_destroy_bottom(accel);
		}
		_gpu_raytrace_acceleration_structure_destroy_top(accel);
	}

	vb_count_last = vb_count;

	if (vb_count == 0) {
		return;
	}

	descriptorsAllocated = 1; // 1 descriptor already allocated in gpu_raytrace_pipeline_init

	#ifdef is_forge
	create_srv_ib(_ib_full, _ib_full->count, 0);
	create_srv_vb(_vb_full, _vb_full->count, vb[0]->stride);
	#else
	create_srv_ib(ib[0], ib[0]->count, 0);
	create_srv_vb(vb[0], vb[0]->count, vb[0]->stride);
	#endif

	// Reset the command list for the acceleration structure construction
	command_list->lpVtbl->Reset(command_list, command_allocator, NULL);

	// Get required sizes for an acceleration structure
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS topLevelInputs = {
		.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
		.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
		.NumDescs = 1,
		.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL,
	};

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {0};
	dxrDevice->lpVtbl->GetRaytracingAccelerationStructurePrebuildInfo(dxrDevice, &topLevelInputs, &topLevelPrebuildInfo);

	UINT64 scratch_size = topLevelPrebuildInfo.ScratchDataSizeInBytes;

	// Bottom AS
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS bottomLevelInputs[16];
	D3D12_RAYTRACING_GEOMETRY_DESC geometryDescs[16];
	if (build_bottom) {
		for (int i = 0; i < vb_count; ++i) {
			D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {
				.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES,
				.Triangles.IndexBuffer = ib[i]->impl.buffer->lpVtbl->GetGPUVirtualAddress(ib[i]->impl.buffer),
				.Triangles.IndexCount = ib[i]->count,
				.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT,
				.Triangles.Transform3x4 = 0,
				.Triangles.VertexFormat = DXGI_FORMAT_R16G16B16A16_SNORM,
				.Triangles.VertexCount = vb[i]->count,
			};

			D3D12_RESOURCE_DESC desc;
			vb[i]->impl.buffer->lpVtbl->GetDesc(vb[i]->impl.buffer, &desc);

			geometryDesc.Triangles.VertexBuffer.StartAddress = vb[i]->impl.buffer->lpVtbl->GetGPUVirtualAddress(vb[i]->impl.buffer);
			geometryDesc.Triangles.VertexBuffer.StrideInBytes = desc.Width / vb[i]->count;
			geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
			geometryDescs[i] = geometryDesc;

			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {0};
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {
				.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
				.NumDescs = 1,
				.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL,
				.pGeometryDescs = &geometryDescs[i],
				.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
			};
			dxrDevice->lpVtbl->GetRaytracingAccelerationStructurePrebuildInfo(dxrDevice, &inputs, &bottomLevelPrebuildInfo);
			bottomLevelInputs[i] = inputs;

			UINT64 blSize = bottomLevelPrebuildInfo.ScratchDataSizeInBytes;
			if (scratch_size < blSize) {
				scratch_size = blSize;
			}

			// Allocate resources for acceleration structures
			// The resources that will contain acceleration structures must be created in the state D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
			// and must have resource flag D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS.
			{
				D3D12_RESOURCE_DESC bufferDesc = {
					.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
					.Width = bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes,
					.Height = 1,
					.DepthOrArraySize = 1,
					.MipLevels = 1,
					.Format = DXGI_FORMAT_UNKNOWN,
					.SampleDesc.Count = 1,
					.SampleDesc.Quality = 0,
					.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
					.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
				};
				D3D12_HEAP_PROPERTIES uploadHeapProperties = {
					.Type = D3D12_HEAP_TYPE_DEFAULT,
					.CreationNodeMask = 1,
					.VisibleNodeMask = 1,
				};

				device->lpVtbl->CreateCommittedResource(dxrDevice, &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, NULL,
														&IID_ID3D12Resource, &accel->impl.bottom_level_accel[i]);
			}
		}
	}

	// Create scratch memory
	ID3D12Resource *scratchResource;
	{
		D3D12_RESOURCE_DESC bufferDesc = {
			.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
			.Width = scratch_size,
			.Height = 1,
			.DepthOrArraySize = 1,
			.MipLevels = 1,
			.Format = DXGI_FORMAT_UNKNOWN,
			.SampleDesc.Count = 1,
			.SampleDesc.Quality = 0,
			.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
			.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		};
		D3D12_HEAP_PROPERTIES uploadHeapProperties = {
			.Type = D3D12_HEAP_TYPE_DEFAULT,
			.CreationNodeMask = 1,
			.VisibleNodeMask = 1,
		};

		device->lpVtbl->CreateCommittedResource(dxrDevice, &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, NULL,
												&IID_ID3D12Resource, &scratchResource);
	}

	// Bottom AS
	if (build_bottom) {
		for (int i = 0; i < vb_count; ++i) {
			// Bottom Level Acceleration Structure desc
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {
				.Inputs = bottomLevelInputs[i],
				.ScratchAccelerationStructureData = scratchResource->lpVtbl->GetGPUVirtualAddress(scratchResource),
				.DestAccelerationStructureData = accel->impl.bottom_level_accel[i]->lpVtbl->GetGPUVirtualAddress(accel->impl.bottom_level_accel[i]),
			};

			// Build acceleration structure
			dxrCommandList->lpVtbl->BuildRaytracingAccelerationStructure(dxrCommandList, &bottomLevelBuildDesc, 0, NULL);
		}
	}

	// Top AS
	{
		D3D12_RESOURCE_DESC bufferDesc = {
			.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
			.Width = topLevelPrebuildInfo.ResultDataMaxSizeInBytes,
			.Height = 1,
			.DepthOrArraySize = 1,
			.MipLevels = 1,
			.Format = DXGI_FORMAT_UNKNOWN,
			.SampleDesc.Count = 1,
			.SampleDesc.Quality = 0,
			.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
			.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		};
		D3D12_HEAP_PROPERTIES uploadHeapProperties = {
			.Type = D3D12_HEAP_TYPE_DEFAULT,
			.CreationNodeMask = 1,
			.VisibleNodeMask = 1,
		};

		device->lpVtbl->CreateCommittedResource(device, &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, NULL,
												&IID_ID3D12Resource, &accel->impl.top_level_accel);
	}

	// Create an instance desc for the bottom-level acceleration structure
	D3D12_RESOURCE_DESC bufferDesc = {
		.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
		.Width = sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * instances_count,
		.Height = 1,
		.DepthOrArraySize = 1,
		.MipLevels = 1,
		.Format = DXGI_FORMAT_UNKNOWN,
		.SampleDesc.Count = 1,
		.SampleDesc.Quality = 0,
		.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
	};
	D3D12_HEAP_PROPERTIES uploadHeapProperties = {
		.Type = D3D12_HEAP_TYPE_UPLOAD,
		.CreationNodeMask = 1,
		.VisibleNodeMask = 1,
	};

	ID3D12Resource *instanceDescs;
	device->lpVtbl->CreateCommittedResource(device, &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
											&IID_ID3D12Resource, &instanceDescs);
	void *mappedData;
	instanceDescs->lpVtbl->Map(instanceDescs, 0, NULL, &mappedData);

	for (int i = 0; i < instances_count; ++i) {
		D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {0};

		instanceDesc.Transform[0][0] = instances[i].m.m[0];
		instanceDesc.Transform[0][1] = instances[i].m.m[1];
		instanceDesc.Transform[0][2] = instances[i].m.m[2];
		instanceDesc.Transform[0][3] = instances[i].m.m[3];
		instanceDesc.Transform[1][0] = instances[i].m.m[4];
		instanceDesc.Transform[1][1] = instances[i].m.m[5];
		instanceDesc.Transform[1][2] = instances[i].m.m[6];
		instanceDesc.Transform[1][3] = instances[i].m.m[7];
		instanceDesc.Transform[2][0] = instances[i].m.m[8];
		instanceDesc.Transform[2][1] = instances[i].m.m[9];
		instanceDesc.Transform[2][2] = instances[i].m.m[10];
		instanceDesc.Transform[2][3] = instances[i].m.m[11];

		int ib_off = 0;
		for (int j = 0; j < instances[i].i; ++j) {
			ib_off += ib[j]->count * 4;
		}
		instanceDesc.InstanceID = ib_off;
		instanceDesc.InstanceMask = 1;
		instanceDesc.AccelerationStructure =
			accel->impl.bottom_level_accel[instances[i].i]->lpVtbl->GetGPUVirtualAddress(accel->impl.bottom_level_accel[instances[i].i]);
		memcpy((uint8_t *)mappedData + i * sizeof(D3D12_RAYTRACING_INSTANCE_DESC), &instanceDesc, sizeof(D3D12_RAYTRACING_INSTANCE_DESC));
	}

	instanceDescs->lpVtbl->Unmap(instanceDescs, 0, NULL);

	// Top Level Acceleration Structure desc
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {0};
	topLevelInputs.InstanceDescs = instanceDescs->lpVtbl->GetGPUVirtualAddress(instanceDescs);
	topLevelBuildDesc.Inputs = topLevelInputs;
	topLevelBuildDesc.DestAccelerationStructureData = accel->impl.top_level_accel->lpVtbl->GetGPUVirtualAddress(accel->impl.top_level_accel);
	topLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->lpVtbl->GetGPUVirtualAddress(scratchResource);

	D3D12_RESOURCE_BARRIER barrier = {
		.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV,
		.UAV.pResource = accel->impl.bottom_level_accel[0],
	};
	command_list->lpVtbl->ResourceBarrier(command_list, 1, &barrier);
	dxrCommandList->lpVtbl->BuildRaytracingAccelerationStructure(dxrCommandList, &topLevelBuildDesc, 0, NULL);

	gpu_flush();

	scratchResource->lpVtbl->Release(scratchResource);
	instanceDescs->lpVtbl->Release(instanceDescs);
}

void gpu_raytrace_acceleration_structure_destroy(gpu_raytrace_acceleration_structure_t *accel) {
	// accel->impl.bottom_level_accel->Release();
	// accel->impl.top_level_accel->Release();
}

void gpu_raytrace_set_textures(gpu_texture_t *texpaint0, gpu_texture_t *texpaint1, gpu_texture_t *texpaint2, gpu_texture_t *texenv, gpu_texture_t *texsobol, gpu_texture_t *texscramble, gpu_texture_t *texrank) {
	D3D12_CPU_DESCRIPTOR_HANDLE handle;
	descriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(descriptorHeap, &handle);

	D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor = {0};
	cpuDescriptor.ptr = handle.ptr + 5 * (UINT64)(descriptorSize);

	D3D12_CPU_DESCRIPTOR_HANDLE sourceCpu;
	texpaint0->impl.srv_descriptor_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(texpaint0->impl.srv_descriptor_heap, &sourceCpu);
	device->lpVtbl->CopyDescriptorsSimple(device, 1, cpuDescriptor, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_GPU_DESCRIPTOR_HANDLE ghandle;
	descriptorHeap->lpVtbl->GetGPUDescriptorHandleForHeapStart(descriptorHeap, &ghandle);
	tex0gpuDescriptorHandle.ptr = ghandle.ptr + 5 * (UINT64)(descriptorSize);

	descriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(descriptorHeap, &handle);
	cpuDescriptor.ptr = handle.ptr + 6 * (UINT64)(descriptorSize);
	texpaint1->impl.srv_descriptor_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(texpaint1->impl.srv_descriptor_heap, &sourceCpu);
	device->lpVtbl->CopyDescriptorsSimple(device, 1, cpuDescriptor, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorHeap->lpVtbl->GetGPUDescriptorHandleForHeapStart(descriptorHeap, &ghandle);
	tex1gpuDescriptorHandle.ptr = ghandle.ptr + 6 * (UINT64)(descriptorSize);

	descriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(descriptorHeap, &handle);
	cpuDescriptor.ptr = handle.ptr + 7 * (UINT64)(descriptorSize);
	texpaint2->impl.srv_descriptor_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(texpaint2->impl.srv_descriptor_heap, &sourceCpu);
	device->lpVtbl->CopyDescriptorsSimple(device, 1, cpuDescriptor, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorHeap->lpVtbl->GetGPUDescriptorHandleForHeapStart(descriptorHeap, &ghandle);
	tex2gpuDescriptorHandle.ptr = ghandle.ptr + 7 * (UINT64)(descriptorSize);

	descriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(descriptorHeap, &handle);
	cpuDescriptor.ptr = handle.ptr + 8 * (UINT64)(descriptorSize);
	texenv->impl.srv_descriptor_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(texenv->impl.srv_descriptor_heap, &sourceCpu);
	device->lpVtbl->CopyDescriptorsSimple(device, 1, cpuDescriptor, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorHeap->lpVtbl->GetGPUDescriptorHandleForHeapStart(descriptorHeap, &ghandle);
	texenvgpuDescriptorHandle.ptr = ghandle.ptr + 8 * (UINT64)(descriptorSize);

	descriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(descriptorHeap, &handle);
	cpuDescriptor.ptr = handle.ptr + 9 * (UINT64)(descriptorSize);
	texsobol->impl.srv_descriptor_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(texsobol->impl.srv_descriptor_heap, &sourceCpu);
	device->lpVtbl->CopyDescriptorsSimple(device, 1, cpuDescriptor, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorHeap->lpVtbl->GetGPUDescriptorHandleForHeapStart(descriptorHeap, &ghandle);
	texsobolgpuDescriptorHandle.ptr = ghandle.ptr + 9 * (UINT64)(descriptorSize);

	descriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(descriptorHeap, &handle);
	cpuDescriptor.ptr = handle.ptr + 10 * (UINT64)(descriptorSize);
	texscramble->impl.srv_descriptor_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(texscramble->impl.srv_descriptor_heap , &sourceCpu);
	device->lpVtbl->CopyDescriptorsSimple(device, 1, cpuDescriptor, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorHeap->lpVtbl->GetGPUDescriptorHandleForHeapStart(descriptorHeap, &ghandle);
	texscramblegpuDescriptorHandle.ptr = ghandle.ptr + 10 * (UINT64)(descriptorSize);

	descriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(descriptorHeap, &handle);
	cpuDescriptor.ptr = handle.ptr + 11 * (UINT64)(descriptorSize);
	texrank->impl.srv_descriptor_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(texrank->impl.srv_descriptor_heap , &sourceCpu);
	device->lpVtbl->CopyDescriptorsSimple(device, 1, cpuDescriptor, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorHeap->lpVtbl->GetGPUDescriptorHandleForHeapStart(descriptorHeap, &ghandle);
	texrankgpuDescriptorHandle.ptr = ghandle.ptr + 11 * (UINT64)(descriptorSize);
}

void gpu_raytrace_set_acceleration_structure(gpu_raytrace_acceleration_structure_t *_accel) {
	accel = _accel;
}

void gpu_raytrace_set_pipeline(gpu_raytrace_pipeline_t *_pipeline) {
	pipeline = _pipeline;
}

void gpu_raytrace_set_target(gpu_texture_t *_output) {
	if (_output != output) {
		_output->impl.image->lpVtbl->Release(_output->impl.image);
		_output->impl.rtv_descriptor_heap->lpVtbl->Release(_output->impl.rtv_descriptor_heap);
		_output->impl.srv_descriptor_heap->lpVtbl->Release(_output->impl.srv_descriptor_heap);

		D3D12_HEAP_PROPERTIES heap_properties = {
			.Type = D3D12_HEAP_TYPE_DEFAULT,
			.CreationNodeMask = 1,
			.VisibleNodeMask = 1,
		};
		D3D12_RESOURCE_DESC desc = {
			.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
			.Width = _output->width,
			.Height = _output->height,
			.DepthOrArraySize = 1,
			.MipLevels = 1,
			.Format = DXGI_FORMAT_R16G16B16A16_FLOAT,
			.SampleDesc.Count = 1,
			.SampleDesc.Quality = 0,
			.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
			.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		};
		D3D12_CLEAR_VALUE clear_value;
		clear_value.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		clear_value.Color[0] = 0.0f;
		clear_value.Color[1] = 0.0f;
		clear_value.Color[2] = 0.0f;
		clear_value.Color[3] = 0.0f;

		device->lpVtbl->CreateCommittedResource(device, &heap_properties, D3D12_HEAP_FLAG_NONE, &desc,
										D3D12_RESOURCE_STATE_COMMON, &clear_value, &IID_ID3D12Resource, &_output->impl.image);

		D3D12_RENDER_TARGET_VIEW_DESC view = {
			.Format = DXGI_FORMAT_R16G16B16A16_FLOAT,
			.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D,
			.Texture2D.MipSlice = 0,
			.Texture2D.PlaneSlice = 0,
		};
		D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {
			.NumDescriptors = 1,
			.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
			.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		};
		device->lpVtbl->CreateDescriptorHeap(device, &heap_desc, &IID_ID3D12DescriptorHeap, &_output->impl.rtv_descriptor_heap);
		D3D12_CPU_DESCRIPTOR_HANDLE handle;
		_output->impl.rtv_descriptor_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(_output->impl.rtv_descriptor_heap, &handle);
		device->lpVtbl->CreateRenderTargetView(device, _output->impl.image, &view, handle);

		D3D12_DESCRIPTOR_HEAP_DESC descriptor_heap_desc = {
			.NumDescriptors = 1,
			.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			.NodeMask = 0,
			.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		};
		device->lpVtbl->CreateDescriptorHeap(device, &descriptor_heap_desc, &IID_ID3D12DescriptorHeap, &_output->impl.srv_descriptor_heap);

		D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {
			.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
			.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
			.Format = DXGI_FORMAT_R16G16B16A16_FLOAT,
			.Texture2D.MipLevels = 1,
			.Texture2D.MostDetailedMip = 0,
			.Texture2D.ResourceMinLODClamp = 0.0f,
		};
		_output->impl.srv_descriptor_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(_output->impl.srv_descriptor_heap, &handle);
		device->lpVtbl->CreateShaderResourceView(device, _output->impl.image, &srv_desc,
												 handle);

		D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {
			.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D,
		};
		device->lpVtbl->CreateUnorderedAccessView(device, _output->impl.image, NULL, &UAVDesc, outputCpuDescriptor);
	}
	output = _output;
}

void gpu_raytrace_dispatch_rays() {
	command_list->lpVtbl->SetComputeRootSignature(command_list, dxrRootSignature);

	// Bind the heaps, acceleration structure and dispatch rays
	command_list->lpVtbl->SetDescriptorHeaps(command_list, 1, &descriptorHeap);
	command_list->lpVtbl->SetComputeRootDescriptorTable(command_list, 0, outputDescriptorHandle);
	command_list->lpVtbl->SetComputeRootShaderResourceView(command_list, 1, accel->impl.top_level_accel->lpVtbl->GetGPUVirtualAddress(accel->impl.top_level_accel));
	command_list->lpVtbl->SetComputeRootDescriptorTable(command_list, 2, ibgpuDescriptorHandle);
	command_list->lpVtbl->SetComputeRootDescriptorTable(command_list, 3, vbgpuDescriptorHandle);
	command_list->lpVtbl->SetComputeRootConstantBufferView(command_list, 4, pipeline->_constant_buffer->impl.buffer->lpVtbl->GetGPUVirtualAddress(pipeline->_constant_buffer->impl.buffer));
	command_list->lpVtbl->SetComputeRootDescriptorTable(command_list, 5, tex0gpuDescriptorHandle);
	command_list->lpVtbl->SetComputeRootDescriptorTable(command_list, 6, tex1gpuDescriptorHandle);
	command_list->lpVtbl->SetComputeRootDescriptorTable(command_list, 7, tex2gpuDescriptorHandle);
	command_list->lpVtbl->SetComputeRootDescriptorTable(command_list, 8, texenvgpuDescriptorHandle);
	command_list->lpVtbl->SetComputeRootDescriptorTable(command_list, 9, texsobolgpuDescriptorHandle);
	command_list->lpVtbl->SetComputeRootDescriptorTable(command_list, 10, texscramblegpuDescriptorHandle);
	command_list->lpVtbl->SetComputeRootDescriptorTable(command_list, 11, texrankgpuDescriptorHandle);

	// Since each shader table has only one shader record, the stride is same as the size.
	D3D12_DISPATCH_RAYS_DESC dispatchDesc = {0};
	D3D12_RESOURCE_DESC desc;
	pipeline->impl.hitgroup_shader_table->lpVtbl->GetDesc(pipeline->impl.hitgroup_shader_table, &desc);
	dispatchDesc.HitGroupTable.StartAddress = pipeline->impl.hitgroup_shader_table->lpVtbl->GetGPUVirtualAddress(pipeline->impl.hitgroup_shader_table);
	dispatchDesc.HitGroupTable.SizeInBytes = desc.Width;
	dispatchDesc.HitGroupTable.StrideInBytes = dispatchDesc.HitGroupTable.SizeInBytes;
	dispatchDesc.MissShaderTable.StartAddress = pipeline->impl.miss_shader_table->lpVtbl->GetGPUVirtualAddress(pipeline->impl.miss_shader_table);
	pipeline->impl.miss_shader_table->lpVtbl->GetDesc(pipeline->impl.miss_shader_table, &desc);
	dispatchDesc.MissShaderTable.SizeInBytes = desc.Width;
	dispatchDesc.MissShaderTable.StrideInBytes = dispatchDesc.MissShaderTable.SizeInBytes;
	dispatchDesc.RayGenerationShaderRecord.StartAddress = pipeline->impl.raygen_shader_table->lpVtbl->GetGPUVirtualAddress(pipeline->impl.raygen_shader_table);
	pipeline->impl.raygen_shader_table->lpVtbl->GetDesc(pipeline->impl.raygen_shader_table, &desc);
	dispatchDesc.RayGenerationShaderRecord.SizeInBytes = desc.Width;
	dispatchDesc.Width = output->width;
	dispatchDesc.Height = output->height;
	dispatchDesc.Depth = 1;
	dxrCommandList->lpVtbl->SetPipelineState1(dxrCommandList, pipeline->impl.dxr_state);
	dxrCommandList->lpVtbl->DispatchRays(dxrCommandList, &dispatchDesc);
}
