#include <iron_global.h>

// Windows 7
#define WINVER 0x0601
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0601

#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCOMM
#define NOCTLMGR
#define NODEFERWINDOWPOS
#define NODRAWTEXT
#define NOGDI
#define NOGDICAPMASKS
#define NOHELP
#define NOICONS
#define NOKANJI
#define NOKEYSTATES
#define NOMB
#define NOMCX
#define NOMEMMGR
#define NOMENUS
#define NOMETAFILE
#define NOMINMAX
// #define NOMSG
#define NONLS
#define NOOPENFILE
#define NOPROFILER
#define NORASTEROPS
#define NOSCROLL
#define NOSERVICE
#define NOSHOWWINDOW
#define NOSOUND
#define NOSYSCOMMANDS
#define NOSYSMETRICS
#define NOTEXTMETRIC
// #define NOUSER
#define NOVIRTUALKEYCODES
#define NOWH
#define NOWINMESSAGES
#define NOWINOFFSETS
#define NOWINSTYLES
#define WIN32_LEAN_AND_MEAN

#include <d3d12.h>
#include <dxgi.h>
#include <iron_gpu.h>
#include <iron_system.h>
#include <stdbool.h>
#include <assert.h>
#include <malloc.h>
#include <iron_math.h>
#include <dxgi1_4.h>
#undef CreateWindow
#include <backends/windows_system.h>
#include <iron_gpu.h>
#include <iron_math.h>
#include <math.h>

ID3D12Device *device = NULL;
static ID3D12RootSignature *globalRootSignature = NULL;
// extern ID3D12GraphicsCommandList* commandList;
static int framebuffer_count = 0;

#define MAXIMUM_WINDOWS 1

struct dx_ctx {
	struct dx_window windows[MAXIMUM_WINDOWS];
};

static struct dx_ctx dx_ctx = {0};

inline struct dx_window *iron_dx_current_window() {
	return &dx_ctx.windows[0];
}

ID3D12CommandQueue *commandQueue;

struct RenderEnvironment {
	ID3D12Device *device;
	ID3D12CommandQueue *queue;
	IDXGISwapChain *swapChain;
};

extern bool bilinearFiltering;
static ID3D12Fence *uploadFence;
static ID3D12GraphicsCommandList *initCommandList;
static ID3D12CommandAllocator *initCommandAllocator;

struct RenderEnvironment createDeviceAndSwapChainHelper(D3D_FEATURE_LEVEL minimumFeatureLevel, const struct DXGI_SWAP_CHAIN_DESC *swapChainDesc) {
	struct RenderEnvironment result = {0};

	iron_microsoft_affirm(D3D12CreateDevice(NULL, minimumFeatureLevel, &IID_ID3D12Device, &result.device));

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	iron_microsoft_affirm(result.device->lpVtbl->CreateCommandQueue(result.device, &queueDesc, &IID_ID3D12CommandQueue, &result.queue));

	IDXGIFactory4 *dxgiFactory;
	iron_microsoft_affirm(CreateDXGIFactory1(&IID_IDXGIFactory4, &dxgiFactory));

	DXGI_SWAP_CHAIN_DESC swapChainDescCopy = *swapChainDesc;
	iron_microsoft_affirm(dxgiFactory->lpVtbl->CreateSwapChain(dxgiFactory, (IUnknown *)result.queue, &swapChainDescCopy, &result.swapChain));

	return result;
}

static void waitForFence(ID3D12Fence *fence, UINT64 completionValue, HANDLE waitEvent) {
	if (fence->lpVtbl->GetCompletedValue(fence) < completionValue) {
		iron_microsoft_affirm(fence->lpVtbl->SetEventOnCompletion(fence, completionValue, waitEvent));
		WaitForSingleObject(waitEvent, INFINITE);
	}
}

void setupSwapChain(struct dx_window *window) {
	D3D12_RESOURCE_DESC depthTexture = {};
	depthTexture.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthTexture.Alignment = 0;
	depthTexture.Width = window->width;
	depthTexture.Height = window->height;
	depthTexture.DepthOrArraySize = 1;
	depthTexture.MipLevels = 1;
	depthTexture.Format = DXGI_FORMAT_D32_FLOAT;
	depthTexture.SampleDesc.Count = 1;
	depthTexture.SampleDesc.Quality = 0;
	depthTexture.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthTexture.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = DXGI_FORMAT_D32_FLOAT;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 1;
	heapProperties.VisibleNodeMask = 1;

	window->current_fence_value = 0;

	for (int i = 0; i < QUEUE_SLOT_COUNT; ++i) {
		window->frame_fence_events[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
		window->fence_values[i] = 0;
		device->lpVtbl->CreateFence(device, window->current_fence_value, D3D12_FENCE_FLAG_NONE, &IID_ID3D12Fence, &window->frame_fences[i]);
	}
}

static void createDeviceAndSwapChain(struct dx_window *window) {
#ifdef _DEBUG
	ID3D12Debug *debugController = NULL;
	D3D12GetDebugInterface(&IID_ID3D12Debug, &debugController);
	debugController->lpVtbl->EnableDebugLayer(debugController);
#endif

	struct DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	swapChainDesc.BufferCount = QUEUE_SLOT_COUNT;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferDesc.Width = window->width;
	swapChainDesc.BufferDesc.Height = window->height;
	swapChainDesc.OutputWindow = iron_windows_window_handle();
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.Windowed = true;

	struct RenderEnvironment renderEnv = createDeviceAndSwapChainHelper(D3D_FEATURE_LEVEL_11_0, &swapChainDesc);

	device = renderEnv.device;
	commandQueue = renderEnv.queue;

	setupSwapChain(NULL);
}

static void createRootSignature() {
	ID3DBlob *rootBlob;
	ID3DBlob *errorBlob;

	D3D12_ROOT_PARAMETER parameters[4] = {};

	D3D12_DESCRIPTOR_RANGE range;
	range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range.NumDescriptors = (UINT)IRON_INTERNAL_G5_TEXTURE_COUNT;
	range.BaseShaderRegister = 0;
	range.RegisterSpace = 0;
	range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	parameters[0].DescriptorTable.NumDescriptorRanges = 1;
	parameters[0].DescriptorTable.pDescriptorRanges = &range;

	D3D12_DESCRIPTOR_RANGE range2;
	range2.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
	range2.NumDescriptors = (UINT)IRON_INTERNAL_G5_TEXTURE_COUNT;
	range2.BaseShaderRegister = 0;
	range2.RegisterSpace = 0;
	range2.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	parameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	parameters[1].DescriptorTable.NumDescriptorRanges = 1;
	parameters[1].DescriptorTable.pDescriptorRanges = &range2;

	parameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	parameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	parameters[2].Descriptor.ShaderRegister = 0;
	parameters[2].Descriptor.RegisterSpace = 0;

	parameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	parameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	parameters[3].Descriptor.ShaderRegister = 0;
	parameters[3].Descriptor.RegisterSpace = 0;

	D3D12_STATIC_SAMPLER_DESC samplers[IRON_INTERNAL_G5_TEXTURE_COUNT * 2];
	for (int i = 0; i < IRON_INTERNAL_G5_TEXTURE_COUNT; ++i) {
		samplers[i].ShaderRegister = i;
		samplers[i].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		samplers[i].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplers[i].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplers[i].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplers[i].MipLODBias = 0;
		samplers[i].MaxAnisotropy = 16;
		samplers[i].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		samplers[i].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		samplers[i].MinLOD = 0.0f;
		samplers[i].MaxLOD = D3D12_FLOAT32_MAX;
		samplers[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		samplers[i].RegisterSpace = 0;
	}
	for (int i = IRON_INTERNAL_G5_TEXTURE_COUNT; i < IRON_INTERNAL_G5_TEXTURE_COUNT * 2; ++i) {
		samplers[i].ShaderRegister = i;
		samplers[i].Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		samplers[i].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplers[i].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplers[i].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplers[i].MipLODBias = 0;
		samplers[i].MaxAnisotropy = 16;
		samplers[i].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		samplers[i].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		samplers[i].MinLOD = 0.0f;
		samplers[i].MaxLOD = D3D12_FLOAT32_MAX;
		samplers[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		samplers[i].RegisterSpace = 0;
	}

	D3D12_ROOT_SIGNATURE_DESC descRootSignature;
	descRootSignature.NumParameters = 4;
	descRootSignature.pParameters = parameters;
	descRootSignature.NumStaticSamplers = 0;
	descRootSignature.pStaticSamplers = NULL;
	descRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	iron_microsoft_affirm(D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &rootBlob, &errorBlob));
	device->lpVtbl->CreateRootSignature(device, 0, rootBlob->lpVtbl->GetBufferPointer(rootBlob), rootBlob->lpVtbl->GetBufferSize(rootBlob), &IID_ID3D12RootSignature,
	                                    &globalRootSignature);
}

static void initialize(struct dx_window *window) {
	createDeviceAndSwapChain(window);
	createRootSignature();

	device->lpVtbl->CreateFence(device, 0, D3D12_FENCE_FLAG_NONE, &IID_ID3D12Fence, &uploadFence);

	device->lpVtbl->CreateCommandAllocator(device, D3D12_COMMAND_LIST_TYPE_DIRECT, &IID_ID3D12CommandAllocator, &initCommandAllocator);
	device->lpVtbl->CreateCommandList(device, 0, D3D12_COMMAND_LIST_TYPE_DIRECT, initCommandAllocator, NULL, &IID_ID3D12CommandList, &initCommandList);

	initCommandList->lpVtbl->Close(initCommandList);

	ID3D12CommandList *commandLists[] = {(ID3D12CommandList *)initCommandList};
	commandQueue->lpVtbl->ExecuteCommandLists(commandQueue, 1, commandLists);
	commandQueue->lpVtbl->Signal(commandQueue, uploadFence, 1);

	HANDLE waitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	waitForFence(uploadFence, 1, waitEvent);

	initCommandAllocator->lpVtbl->Reset(initCommandAllocator);
	initCommandList->lpVtbl->Release(initCommandList); // check me
	initCommandAllocator->lpVtbl->Release(initCommandAllocator);            // check me

	CloseHandle(waitEvent);
}

static void shutdown() {
	for (int i = 0; i < QUEUE_SLOT_COUNT; ++i) {
		// waitForFence(window->frame_fences[i], window->current_fence_value[i], window->frame_fence_events[i]);
	}

	for (int i = 0; i < QUEUE_SLOT_COUNT; ++i) {
		// CloseHandle(window->frame_fence_events[i]);
	}
}

static void initWindow(struct dx_window *window) {
	HWND hwnd = iron_windows_window_handle();

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	swapChainDesc.BufferCount = QUEUE_SLOT_COUNT;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferDesc.Width = iron_window_width();
	swapChainDesc.BufferDesc.Height = iron_window_height();
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.Windowed = true;

	IDXGIFactory4 *dxgiFactory = NULL;
	iron_microsoft_affirm(CreateDXGIFactory1(&IID_IDXGIFactory4, &dxgiFactory));

	iron_microsoft_affirm(dxgiFactory->lpVtbl->CreateSwapChain(dxgiFactory, (IUnknown *)commandQueue, &swapChainDesc, &window->swapChain));

	setupSwapChain(window);
}

void iron_gpu_internal_destroy_window() {}

void iron_gpu_internal_destroy() {
	if (device) {
		device->lpVtbl->Release(device);
		device = NULL;
	}
}

void iron_gpu_internal_init() {
#ifdef _DEBUG
	ID3D12Debug *debugController = NULL;
	if (D3D12GetDebugInterface(&IID_ID3D12Debug, &debugController) == S_OK) {
		debugController->lpVtbl->EnableDebugLayer(debugController);
	}
#endif
	iron_microsoft_affirm(D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_11_0, &IID_ID3D12Device, &device));

	createRootSignature();

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	iron_microsoft_affirm(device->lpVtbl->CreateCommandQueue(device, &queueDesc, &IID_ID3D12CommandQueue, &commandQueue));

	device->lpVtbl->CreateFence(device, 0, D3D12_FENCE_FLAG_NONE, &IID_ID3D12Fence, &uploadFence);

	device->lpVtbl->CreateCommandAllocator(device, D3D12_COMMAND_LIST_TYPE_DIRECT, &IID_ID3D12CommandAllocator, &initCommandAllocator);
	device->lpVtbl->CreateCommandList(device, 0, D3D12_COMMAND_LIST_TYPE_DIRECT, initCommandAllocator, NULL, &IID_ID3D12CommandList, &initCommandList);

	initCommandList->lpVtbl->Close(initCommandList);

	ID3D12CommandList *commandLists[] = {(ID3D12CommandList *)initCommandList};
	commandQueue->lpVtbl->ExecuteCommandLists(commandQueue, 1, commandLists);
	commandQueue->lpVtbl->Signal(commandQueue, uploadFence, 1);

	HANDLE waitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	waitForFence(uploadFence, 1, waitEvent);

	initCommandAllocator->lpVtbl->Reset(initCommandAllocator);
	initCommandList->lpVtbl->Release(initCommandList); // check me
	initCommandAllocator->lpVtbl->Release(initCommandAllocator); // check me

	CloseHandle(waitEvent);
}

void iron_gpu_internal_init_window(int depthBufferBits, bool verticalSync) {
	struct dx_window *window = &dx_ctx.windows[0];
	window->vsync = verticalSync;
	window->width = window->new_width = iron_window_width();
	window->height = window->new_height = iron_window_height();
	initWindow(window);
}

int iron_gpu_max_bound_textures(void) {
	return D3D12_COMMONSHADER_SAMPLER_SLOT_COUNT;
}

static bool began = false;

void iron_gpu_begin(iron_gpu_texture_t *renderTarget) {
	if (began)
		return;
	began = true;

	struct dx_window *window = &dx_ctx.windows[0];

	window->current_backbuffer = (window->current_backbuffer + 1) % QUEUE_SLOT_COUNT;

	if (window->new_width != window->width || window->new_height != window->height) {

		iron_microsoft_affirm(window->swapChain->lpVtbl->ResizeBuffers(window->swapChain, QUEUE_SLOT_COUNT, window->new_width, window->new_height, DXGI_FORMAT_R8G8B8A8_UNORM, 0));

		setupSwapChain(window);
		window->width = window->new_width;
		window->height = window->new_height;
		window->current_backbuffer = 0;
	}

	const UINT64 fenceValue = window->current_fence_value;
	commandQueue->lpVtbl->Signal(commandQueue, window->frame_fences[window->current_backbuffer], fenceValue);
	window->fence_values[window->current_backbuffer] = fenceValue;
	++window->current_fence_value;

	waitForFence(window->frame_fences[window->current_backbuffer], window->fence_values[window->current_backbuffer],
	             window->frame_fence_events[window->current_backbuffer]);

	// static const float clearColor[] = {0.042f, 0.042f, 0.042f, 1};

	// commandList->ClearRenderTargetView(GetCPUDescriptorHandle(renderTargetDescriptorHeap), clearColor, 0, nullptr);

	// commandList->ClearDepthStencilView(GetCPUDescriptorHandle(depthStencilDescriptorHeap), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void iron_gpu_end() {
	began = false;
}

void iron_gpu_internal_resize(int, int);

void iron_internal_resize(int width, int height) {
	if (width == 0 || height == 0)
		return;
	struct dx_window *window = &dx_ctx.windows[0];
	window->new_width = width;
	window->new_height = height;
	iron_gpu_internal_resize(width, height);
}

bool iron_gpu_swap_buffers() {
	for (int i = 0; i < MAXIMUM_WINDOWS; i++) {
		struct dx_window *window = &dx_ctx.windows[i];
		if (window->swapChain) {
			iron_microsoft_affirm(window->swapChain->lpVtbl->Present(window->swapChain, window->vsync, 0));
		}
	}
	return true;
}

void iron_gpu_flush() {}

bool iron_gpu_raytrace_supported() {
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 options;
	if (device->lpVtbl->CheckFeatureSupport(device, D3D12_FEATURE_D3D12_OPTIONS5, &options, sizeof(options)) == S_OK) {
		return options.RaytracingTier >= D3D12_RAYTRACING_TIER_1_0;
	}
	return false;
}

void createHeaps(iron_gpu_command_list_t *list);

static int formatSize(DXGI_FORMAT format) {
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

void iron_gpu_command_list_init(struct iron_gpu_command_list *list) {
#ifndef NDEBUG
	list->impl.open = false;
#endif

	device->lpVtbl->CreateCommandAllocator(device, D3D12_COMMAND_LIST_TYPE_DIRECT, &IID_ID3D12CommandAllocator, &list->impl._commandAllocator);
	device->lpVtbl->CreateCommandList(device, 0, D3D12_COMMAND_LIST_TYPE_DIRECT, list->impl._commandAllocator, NULL, &IID_ID3D12CommandList, &list->impl._commandList);

	list->impl.fence_value = 0;
	list->impl.fence_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	device->lpVtbl->CreateFence(device, 0, D3D12_FENCE_FLAG_NONE, &IID_ID3D12Fence, &list->impl.fence);

	list->impl._indexCount = 0;

	list->impl.current_full_scissor.left = -1;

	for (int i = 0; i < IRON_INTERNAL_G5_TEXTURE_COUNT; ++i) {
		list->impl.currentTextures[i] = NULL;
		list->impl.current_samplers[i] = NULL;
	}
	list->impl.heapIndex = 0;
	createHeaps(list);
}

void iron_gpu_command_list_destroy(struct iron_gpu_command_list *list) {}

void iron_gpu_internal_reset_textures(struct iron_gpu_command_list *list);

void iron_gpu_command_list_begin(struct iron_gpu_command_list *list) {
	assert(!list->impl.open);

	if (list->impl.fence_value > 0) {
		waitForFence(list->impl.fence, list->impl.fence_value, list->impl.fence_event);
		list->impl._commandAllocator->lpVtbl->Reset(list->impl._commandAllocator);
		list->impl._commandList->lpVtbl->Reset(list->impl._commandList, list->impl._commandAllocator, NULL);
	}

	iron_gpu_internal_reset_textures(list);

#ifndef NDEBUG
	list->impl.open = true;
#endif
}

void iron_gpu_command_list_end(struct iron_gpu_command_list *list) {
	assert(list->impl.open);

	list->impl._commandList->lpVtbl->Close(list->impl._commandList);

#ifndef NDEBUG
	list->impl.open = false;
#endif
}

void iron_gpu_command_list_clear(struct iron_gpu_command_list *list, iron_gpu_texture_t *renderTarget, unsigned flags, unsigned color, float depth) {
	assert(list->impl.open);

	if (flags & IRON_GPU_CLEAR_COLOR) {
		float clearColor[] = {((color & 0x00ff0000) >> 16) / 255.0f, ((color & 0x0000ff00) >> 8) / 255.0f, (color & 0x000000ff) / 255.0f,
		                      ((color & 0xff000000) >> 24) / 255.0f};

		D3D12_CPU_DESCRIPTOR_HANDLE handle;
		renderTarget->impl.renderTargetDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(renderTarget->impl.renderTargetDescriptorHeap, &handle);

		list->impl._commandList->lpVtbl->ClearRenderTargetView(
		    list->impl._commandList,
		    handle,
		    clearColor,
		                                                       0,
		                                               NULL);
	}
	if (flags & IRON_GPU_CLEAR_DEPTH) {
		D3D12_CLEAR_FLAGS d3dflags = D3D12_CLEAR_FLAG_DEPTH;

		if (renderTarget->impl.depthStencilDescriptorHeap != NULL) {
			D3D12_CPU_DESCRIPTOR_HANDLE handle;
			renderTarget->impl.depthStencilDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(renderTarget->impl.depthStencilDescriptorHeap, &handle);
			list->impl._commandList->lpVtbl->ClearDepthStencilView(
			    list->impl._commandList,
			    handle,
			                                                       d3dflags, depth,
			                                               0, 0, NULL);
		}
	}
}

void iron_gpu_command_list_render_target_to_framebuffer_barrier(struct iron_gpu_command_list *list, iron_gpu_texture_t *renderTarget) {
	assert(list->impl.open);

	D3D12_RESOURCE_BARRIER barrier;
	barrier.Transition.pResource = renderTarget->impl.renderTarget;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	list->impl._commandList->lpVtbl->ResourceBarrier(list->impl._commandList, 1, &barrier);
}

void iron_gpu_command_list_framebuffer_to_render_target_barrier(struct iron_gpu_command_list *list, iron_gpu_texture_t *renderTarget) {
	assert(list->impl.open);

	D3D12_RESOURCE_BARRIER barrier;
	barrier.Transition.pResource = renderTarget->impl.renderTarget;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	list->impl._commandList->lpVtbl->ResourceBarrier(list->impl._commandList, 1, &barrier);
}

void iron_gpu_command_list_texture_to_render_target_barrier(struct iron_gpu_command_list *list, iron_gpu_texture_t *renderTarget) {
	assert(list->impl.open);

	D3D12_RESOURCE_BARRIER barrier;
	barrier.Transition.pResource = renderTarget->impl.renderTarget;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	list->impl._commandList->lpVtbl->ResourceBarrier(list->impl._commandList, 1, &barrier);
}

void iron_gpu_command_list_render_target_to_texture_barrier(struct iron_gpu_command_list *list, iron_gpu_texture_t *renderTarget) {
	assert(list->impl.open);

	D3D12_RESOURCE_BARRIER barrier;
	barrier.Transition.pResource = renderTarget->impl.renderTarget;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	list->impl._commandList->lpVtbl->ResourceBarrier(list->impl._commandList, 1, &barrier);
}

void iron_gpu_command_list_set_vertex_constant_buffer(struct iron_gpu_command_list *list, iron_gpu_constant_buffer_t *buffer, int offset, size_t size) {
	assert(list->impl.open);
	list->impl._commandList->lpVtbl->SetGraphicsRootConstantBufferView(list->impl._commandList, 2, buffer->impl.constant_buffer->lpVtbl->GetGPUVirtualAddress(buffer->impl.constant_buffer) + offset);
}

void iron_gpu_command_list_set_fragment_constant_buffer(struct iron_gpu_command_list *list, iron_gpu_constant_buffer_t *buffer, int offset, size_t size) {
	assert(list->impl.open);
	list->impl._commandList->lpVtbl->SetGraphicsRootConstantBufferView(list->impl._commandList, 3, buffer->impl.constant_buffer->lpVtbl->GetGPUVirtualAddress(buffer->impl.constant_buffer) + offset);
}

void iron_gpu_command_list_draw_indexed_vertices(struct iron_gpu_command_list *list) {
	iron_gpu_command_list_draw_indexed_vertices_from_to(list, 0, list->impl._indexCount);
}

void iron_gpu_command_list_draw_indexed_vertices_from_to(struct iron_gpu_command_list *list, int start, int count) {
	assert(list->impl.open);
	list->impl._commandList->lpVtbl->IASetPrimitiveTopology(list->impl._commandList, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	list->impl._commandList->lpVtbl->DrawIndexedInstanced(list->impl._commandList, count, 1, start, 0, 0);
}

void iron_gpu_command_list_execute(iron_gpu_command_list_t *list) {
	assert(!list->impl.open);

	ID3D12CommandList *commandLists[] = {(ID3D12CommandList *)list->impl._commandList};
	commandQueue->lpVtbl->ExecuteCommandLists(commandQueue, 1, commandLists);

	commandQueue->lpVtbl->Signal(commandQueue, list->impl.fence, ++list->impl.fence_value);
}

void iron_gpu_command_list_wait_for_execution_to_finish(iron_gpu_command_list_t *list) {
	waitForFence(list->impl.fence, list->impl.fence_value, list->impl.fence_event);
}

bool iron_gpu_non_pow2_textures_supported(void) {
	return true;
}

void iron_gpu_command_list_viewport(struct iron_gpu_command_list *list, int x, int y, int width, int height) {
	assert(list->impl.open);

	D3D12_VIEWPORT viewport;
	viewport.TopLeftX = (float)x;
	viewport.TopLeftY = (float)y;
	viewport.Width = (float)width;
	viewport.Height = (float)height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	list->impl._commandList->lpVtbl->RSSetViewports(list->impl._commandList, 1, &viewport);
}

void iron_gpu_command_list_scissor(struct iron_gpu_command_list *list, int x, int y, int width, int height) {
	assert(list->impl.open);

	D3D12_RECT scissor;
	scissor.left = x;
	scissor.top = y;
	scissor.right = x + width;
	scissor.bottom = y + height;
	list->impl._commandList->lpVtbl->RSSetScissorRects(list->impl._commandList, 1, &scissor);
}

void iron_gpu_command_list_disable_scissor(struct iron_gpu_command_list *list) {
	assert(list->impl.open);

	if (list->impl.current_full_scissor.left >= 0) {
		list->impl._commandList->lpVtbl->RSSetScissorRects(list->impl._commandList, 1, (D3D12_RECT *)&list->impl.current_full_scissor);
	}
	else {
		D3D12_RECT scissor;
		scissor.left = 0;
		scissor.top = 0;
		scissor.right = iron_window_width();
		scissor.bottom = iron_window_height();
		list->impl._commandList->lpVtbl->RSSetScissorRects(list->impl._commandList, 1, &scissor);
	}
}

void iron_gpu_command_list_set_pipeline(struct iron_gpu_command_list *list, iron_gpu_pipeline_t *pipeline) {
	assert(list->impl.open);

	list->impl._currentPipeline = pipeline;
	list->impl._commandList->lpVtbl->SetPipelineState(list->impl._commandList, pipeline->impl.pso);

	for (int i = 0; i < IRON_INTERNAL_G5_TEXTURE_COUNT; ++i) {
		list->impl.currentTextures[i] = NULL;
	}
	iron_gpu_internal_setConstants(list, list->impl._currentPipeline);
}

void iron_gpu_command_list_set_vertex_buffer(struct iron_gpu_command_list *list, iron_gpu_vertex_buffer_t *buffer) {
	assert(list->impl.open);

	D3D12_VERTEX_BUFFER_VIEW *views = (D3D12_VERTEX_BUFFER_VIEW *)alloca(sizeof(D3D12_VERTEX_BUFFER_VIEW) * 1);
	ZeroMemory(views, sizeof(D3D12_VERTEX_BUFFER_VIEW) * 1);

	views[0].BufferLocation = buffer->impl.uploadBuffer->lpVtbl->GetGPUVirtualAddress(buffer->impl.uploadBuffer);
	views[0].SizeInBytes = (iron_gpu_vertex_buffer_count(buffer)) * iron_gpu_vertex_buffer_stride(buffer);
	views[0].StrideInBytes = iron_gpu_vertex_buffer_stride(buffer);

	list->impl._commandList->lpVtbl->IASetVertexBuffers(list->impl._commandList, 0, 1, views);
}

void iron_gpu_command_list_set_index_buffer(struct iron_gpu_command_list *list, iron_gpu_index_buffer_t *buffer) {
	assert(list->impl.open);

	list->impl._indexCount = iron_gpu_index_buffer_count(buffer);
	list->impl._commandList->lpVtbl->IASetIndexBuffer(list->impl._commandList, (D3D12_INDEX_BUFFER_VIEW *) & buffer->impl.index_buffer_view);
}

void iron_gpu_command_list_set_render_targets(struct iron_gpu_command_list *list, iron_gpu_texture_t **targets, int count) {
	assert(list->impl.open);

	iron_gpu_texture_t *render_target = targets[0];

	D3D12_CPU_DESCRIPTOR_HANDLE target_descriptors[16];
	for (int i = 0; i < count; ++i) {
		targets[i]->impl.renderTargetDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(targets[i]->impl.renderTargetDescriptorHeap,
		                                                                                        &target_descriptors[i]);
	}

	assert(render_target != NULL);

	if (render_target->impl.depthStencilDescriptorHeap != NULL) {
		D3D12_CPU_DESCRIPTOR_HANDLE heapStart;
		render_target->impl.depthStencilDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(render_target->impl.depthStencilDescriptorHeap, &heapStart);
		list->impl._commandList->lpVtbl->OMSetRenderTargets(list->impl._commandList, count, &target_descriptors[0], false, &heapStart);
	}
	else {
		list->impl._commandList->lpVtbl->OMSetRenderTargets(list->impl._commandList, count, &target_descriptors[0], false, NULL);
	}

	list->impl._commandList->lpVtbl->RSSetViewports(list->impl._commandList, 1, (D3D12_VIEWPORT *)&render_target->impl.viewport);
	list->impl._commandList->lpVtbl->RSSetScissorRects(list->impl._commandList, 1, (D3D12_RECT *)&render_target->impl.scissor);

	list->impl.current_full_scissor = render_target->impl.scissor;
}

void iron_gpu_command_list_upload_vertex_buffer(iron_gpu_command_list_t *list, struct iron_gpu_vertex_buffer *buffer) {}

void iron_gpu_command_list_upload_index_buffer(iron_gpu_command_list_t *list, iron_gpu_index_buffer_t *buffer) {
	assert(list->impl.open);
	iron_gpu_internal_index_buffer_upload(buffer, list->impl._commandList);
}

void iron_gpu_command_list_upload_texture(iron_gpu_command_list_t *list, iron_gpu_texture_t *texture) {
	assert(list->impl.open);

	{
		D3D12_RESOURCE_BARRIER transition = {};
		transition.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		transition.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		transition.Transition.pResource = texture->impl.image;
		transition.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		transition.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
		transition.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		list->impl._commandList->lpVtbl->ResourceBarrier(list->impl._commandList, 1, &transition);
	}

	D3D12_RESOURCE_DESC Desc;
	texture->impl.image->lpVtbl->GetDesc(texture->impl.image, &Desc);
	ID3D12Device *device = NULL;
	texture->impl.image->lpVtbl->GetDevice(texture->impl.image, &IID_ID3D12Device, &device);
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
	device->lpVtbl->GetCopyableFootprints(device , &Desc, 0, 1, 0, &footprint, NULL, NULL, NULL);
	device->lpVtbl->Release(device);

	D3D12_TEXTURE_COPY_LOCATION source = {0};
	source.pResource = texture->impl.uploadImage;
	source.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	source.PlacedFootprint = footprint;

	D3D12_TEXTURE_COPY_LOCATION destination = {0};
	destination.pResource = texture->impl.image;
	destination.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	destination.SubresourceIndex = 0;

	list->impl._commandList->lpVtbl->CopyTextureRegion(list->impl._commandList, &destination, 0, 0, 0, &source, NULL);

	{
		D3D12_RESOURCE_BARRIER transition = {};
		transition.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		transition.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		transition.Transition.pResource = texture->impl.image;
		transition.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		transition.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		transition.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		list->impl._commandList->lpVtbl->ResourceBarrier(list->impl._commandList, 1, &transition);
	}
}

static int d3d12_textureAlignment() {
	return D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
}

void iron_gpu_command_list_get_render_target_pixels(iron_gpu_command_list_t *list, iron_gpu_texture_t *render_target, uint8_t *data) {
	assert(list->impl.open);

	D3D12_RESOURCE_DESC desc;
	render_target->impl.renderTarget->lpVtbl->GetDesc(render_target->impl.renderTarget, &desc);
	DXGI_FORMAT dxgiFormat = desc.Format;
	int formatByteSize = formatSize(dxgiFormat);
	int rowPitch = render_target->width * formatByteSize;
	int align = rowPitch % d3d12_textureAlignment();
	if (align != 0)
		rowPitch = rowPitch + (d3d12_textureAlignment() - align);

	// Create readback buffer
	if (render_target->impl.renderTargetReadback == NULL) {
		D3D12_HEAP_PROPERTIES heapProperties;
		heapProperties.Type = D3D12_HEAP_TYPE_READBACK;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask = 1;
		heapProperties.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC resourceDesc;
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = rowPitch * render_target->height;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		device->lpVtbl->CreateCommittedResource(device , &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COMMON, NULL,
		                                &IID_ID3D12Resource, &render_target->impl.renderTargetReadback);
	}

	// Copy render target to readback buffer
	D3D12_RESOURCE_STATES sourceState = D3D12_RESOURCE_STATE_RENDER_TARGET;

	{
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Transition.pResource = render_target->impl.renderTarget;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.StateBefore = sourceState;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		list->impl._commandList->lpVtbl->ResourceBarrier(list->impl._commandList, 1, &barrier);
	}

	D3D12_TEXTURE_COPY_LOCATION source;
	source.pResource = render_target->impl.renderTarget;
	source.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	source.SubresourceIndex = 0;

	D3D12_TEXTURE_COPY_LOCATION dest;
	dest.pResource = render_target->impl.renderTargetReadback;
	dest.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	dest.PlacedFootprint.Offset = 0;
	dest.PlacedFootprint.Footprint.Format = dxgiFormat;
	dest.PlacedFootprint.Footprint.Width = render_target->width;
	dest.PlacedFootprint.Footprint.Height = render_target->height;
	dest.PlacedFootprint.Footprint.Depth = 1;
	dest.PlacedFootprint.Footprint.RowPitch = rowPitch;

	list->impl._commandList->lpVtbl->CopyTextureRegion(list->impl._commandList , &dest, 0, 0, 0, &source, NULL);

	{
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Transition.pResource = render_target->impl.renderTarget;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
		barrier.Transition.StateAfter = sourceState;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		list->impl._commandList->lpVtbl->ResourceBarrier(list->impl._commandList, 1, &barrier);
	}

	iron_gpu_command_list_end(list);
	iron_gpu_command_list_execute(list);
	iron_gpu_command_list_wait_for_execution_to_finish(list);
	iron_gpu_command_list_begin(list);

	// Read buffer
	void *p;
	render_target->impl.renderTargetReadback->lpVtbl->Map(render_target->impl.renderTargetReadback, 0, NULL, &p);
	memcpy(data, p, render_target->width * render_target->height * formatByteSize);
	render_target->impl.renderTargetReadback->lpVtbl->Unmap(render_target->impl.renderTargetReadback, 0, NULL);
}

void iron_gpu_internal_sampler_set(iron_gpu_command_list_t *list, iron_gpu_sampler_t *sampler, int unit);

void iron_gpu_command_list_set_sampler(iron_gpu_command_list_t *list, iron_gpu_texture_unit_t unit, iron_gpu_sampler_t *sampler) {
	if (unit.stages[IRON_GPU_SHADER_TYPE_FRAGMENT] >= 0) {
		iron_gpu_internal_sampler_set(list, sampler, unit.stages[IRON_GPU_SHADER_TYPE_FRAGMENT]);
	}
	else if (unit.stages[IRON_GPU_SHADER_TYPE_VERTEX] >= 0) {
		iron_gpu_internal_sampler_set(list, sampler, unit.stages[IRON_GPU_SHADER_TYPE_VERTEX]);
	}
	iron_gpu_internal_set_textures(list);
}

void iron_gpu_command_list_set_texture(iron_gpu_command_list_t *list, iron_gpu_texture_unit_t unit, iron_gpu_texture_t *texture) {
	if (unit.stages[IRON_GPU_SHADER_TYPE_FRAGMENT] >= 0) {
		texture->impl.stage = unit.stages[IRON_GPU_SHADER_TYPE_FRAGMENT];
	}
	else if (unit.stages[IRON_GPU_SHADER_TYPE_VERTEX] >= 0) {
		texture->impl.stage = unit.stages[IRON_GPU_SHADER_TYPE_VERTEX];
	}
	list->impl.currentTextures[texture->impl.stage] = texture;
	iron_gpu_internal_set_textures(list);
}

void iron_gpu_command_list_set_texture_from_render_target_depth(iron_gpu_command_list_t *list, iron_gpu_texture_unit_t unit, iron_gpu_texture_t *texture) {
	if (unit.stages[IRON_GPU_SHADER_TYPE_FRAGMENT] >= 0) {
		texture->impl.stage_depth = unit.stages[IRON_GPU_SHADER_TYPE_FRAGMENT];
	}
	else if (unit.stages[IRON_GPU_SHADER_TYPE_VERTEX] >= 0) {
		texture->impl.stage_depth = unit.stages[IRON_GPU_SHADER_TYPE_VERTEX];
	}
	list->impl.currentTextures[texture->impl.stage_depth] = texture;
}

void iron_gpu_internal_setConstants(iron_gpu_command_list_t *commandList, iron_gpu_pipeline_t *pipeline) {
	commandList->impl._commandList->lpVtbl->SetGraphicsRootSignature(commandList->impl._commandList, globalRootSignature);

	if (pipeline->impl.textures > 0) {
		iron_gpu_internal_set_textures(commandList);
	}
}

void iron_gpu_pipeline_init(iron_gpu_pipeline_t *pipe) {
	iron_gpu_internal_pipeline_set_defaults(pipe);
	iron_gpu_internal_pipeline_init(pipe);
}

void iron_gpu_pipeline_destroy(iron_gpu_pipeline_t *pipe) {
	if (pipe->impl.pso != NULL) {
		pipe->impl.pso->lpVtbl->Release(pipe->impl.pso);
		pipe->impl.pso = NULL;
	}
}

#define MAX_SHADER_THING 32

static ShaderConstant findConstant(iron_gpu_shader_t *shader, const char *name) {
	if (shader != NULL) {
		for (int i = 0; i < MAX_SHADER_THING; ++i) {
			if (strcmp(shader->impl.constants[i].name, name) == 0) {
				return shader->impl.constants[i];
			}
		}
	}

	ShaderConstant constant;
	constant.name[0] = 0;
	constant.offset = -1;
	constant.size = 0;
	return constant;
}

static ShaderTexture findTexture(iron_gpu_shader_t *shader, const char *name) {
	for (int i = 0; i < MAX_SHADER_THING; ++i) {
		if (strcmp(shader->impl.textures[i].name, name) == 0) {
			return shader->impl.textures[i];
		}
	}

	ShaderTexture texture;
	texture.name[0] = 0;
	texture.texture = -1;
	return texture;
}

static ShaderAttribute findAttribute(iron_gpu_shader_t *shader, const char *name) {
	for (int i = 0; i < MAX_SHADER_THING; ++i) {
		if (strcmp(shader->impl.attributes[i].name, name) == 0) {
			return shader->impl.attributes[i];
		}
	}

	ShaderAttribute attribute;
	attribute.name[0] = 0;
	attribute.attribute = -1;
	return attribute;
}

iron_gpu_constant_location_t iron_gpu_pipeline_get_constant_location(struct iron_gpu_pipeline *pipe, const char *name) {
	iron_gpu_constant_location_t location;

	{
		ShaderConstant constant = findConstant(pipe->vertex_shader, name);
		location.impl.vertexOffset = constant.offset;
		location.impl.vertexSize = constant.size;
	}

	{
		ShaderConstant constant = findConstant(pipe->fragment_shader, name);
		location.impl.fragmentOffset = constant.offset;
		location.impl.fragmentSize = constant.size;
	}

	return location;
}

iron_gpu_texture_unit_t iron_gpu_pipeline_get_texture_unit(iron_gpu_pipeline_t *pipe, const char *name) {
	iron_gpu_texture_unit_t unit;
	for (int i = 0; i < IRON_GPU_SHADER_TYPE_COUNT; ++i) {
		unit.stages[i] = -1;
	}

	ShaderTexture vertexTexture = findTexture(pipe->vertex_shader, name);
	if (vertexTexture.texture != -1) {
		unit.stages[IRON_GPU_SHADER_TYPE_VERTEX] = vertexTexture.texture;
	}
	else {
		ShaderTexture fragmentTexture = findTexture(pipe->fragment_shader, name);
		unit.stages[IRON_GPU_SHADER_TYPE_FRAGMENT] = fragmentTexture.texture;
	}
	return unit;
}

static D3D12_BLEND convert_blend_factor(iron_gpu_blending_factor_t factor) {
	switch (factor) {
	case IRON_GPU_BLEND_ONE:
		return D3D12_BLEND_ONE;
	case IRON_GPU_BLEND_ZERO:
		return D3D12_BLEND_ZERO;
	case IRON_GPU_BLEND_SOURCE_ALPHA:
		return D3D12_BLEND_SRC_ALPHA;
	case IRON_GPU_BLEND_DEST_ALPHA:
		return D3D12_BLEND_DEST_ALPHA;
	case IRON_GPU_BLEND_INV_SOURCE_ALPHA:
		return D3D12_BLEND_INV_SRC_ALPHA;
	case IRON_GPU_BLEND_INV_DEST_ALPHA:
		return D3D12_BLEND_INV_DEST_ALPHA;
	case IRON_GPU_BLEND_SOURCE_COLOR:
		return D3D12_BLEND_SRC_COLOR;
	case IRON_GPU_BLEND_DEST_COLOR:
		return D3D12_BLEND_DEST_COLOR;
	case IRON_GPU_BLEND_INV_SOURCE_COLOR:
		return D3D12_BLEND_INV_SRC_COLOR;
	case IRON_GPU_BLEND_INV_DEST_COLOR:
		return D3D12_BLEND_INV_DEST_COLOR;
	case IRON_GPU_BLEND_CONSTANT:
		return D3D12_BLEND_BLEND_FACTOR;
	case IRON_GPU_BLEND_INV_CONSTANT:
		return D3D12_BLEND_INV_BLEND_FACTOR;
	default:
		assert(false);
		return D3D12_BLEND_ONE;
	}
}

static D3D12_BLEND_OP convert_blend_operation(iron_gpu_blending_operation_t op) {
	switch (op) {
	case IRON_GPU_BLENDOP_ADD:
		return D3D12_BLEND_OP_ADD;
	case IRON_GPU_BLENDOP_SUBTRACT:
		return D3D12_BLEND_OP_SUBTRACT;
	case IRON_GPU_BLENDOP_REVERSE_SUBTRACT:
		return D3D12_BLEND_OP_REV_SUBTRACT;
	case IRON_GPU_BLENDOP_MIN:
		return D3D12_BLEND_OP_MIN;
	case IRON_GPU_BLENDOP_MAX:
		return D3D12_BLEND_OP_MAX;
	default:
		assert(false);
		return D3D12_BLEND_OP_ADD;
	}
}

static D3D12_CULL_MODE convert_cull_mode(iron_gpu_cull_mode_t cull_mode) {
	switch (cull_mode) {
	case IRON_GPU_CULL_MODE_CLOCKWISE:
		return D3D12_CULL_MODE_FRONT;
	case IRON_GPU_CULL_MODE_COUNTERCLOCKWISE:
		return D3D12_CULL_MODE_BACK;
	case IRON_GPU_CULL_MODE_NEVER:
	default:
		return D3D12_CULL_MODE_NONE;
	}
}

static D3D12_COMPARISON_FUNC convert_compare_mode(iron_gpu_compare_mode_t compare) {
	switch (compare) {
	default:
	case IRON_GPU_COMPARE_MODE_ALWAYS:
		return D3D12_COMPARISON_FUNC_ALWAYS;
	case IRON_GPU_COMPARE_MODE_NEVER:
		return D3D12_COMPARISON_FUNC_NEVER;
	case IRON_GPU_COMPARE_MODE_EQUAL:
		return D3D12_COMPARISON_FUNC_EQUAL;
	case IRON_GPU_COMPARE_MODE_NOT_EQUAL:
		return D3D12_COMPARISON_FUNC_NOT_EQUAL;
	case IRON_GPU_COMPARE_MODE_LESS:
		return D3D12_COMPARISON_FUNC_LESS;
	case IRON_GPU_COMPARE_MODE_LESS_EQUAL:
		return D3D12_COMPARISON_FUNC_LESS_EQUAL;
	case IRON_GPU_COMPARE_MODE_GREATER:
		return D3D12_COMPARISON_FUNC_GREATER;
	case IRON_GPU_COMPARE_MODE_GREATER_EQUAL:
		return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
	}
}

static DXGI_FORMAT convert_format(iron_image_format_t format) {
	switch (format) {
	case IRON_IMAGE_FORMAT_RGBA128:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case IRON_IMAGE_FORMAT_RGBA64:
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case IRON_IMAGE_FORMAT_R32:
		return DXGI_FORMAT_R32_FLOAT;
	case IRON_IMAGE_FORMAT_R16:
		return DXGI_FORMAT_R16_FLOAT;
	case IRON_IMAGE_FORMAT_R8:
		return DXGI_FORMAT_R8_UNORM;
	case IRON_IMAGE_FORMAT_RGBA32:
	default:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	}
}

static void set_blend_state(D3D12_BLEND_DESC *blend_desc, iron_gpu_pipeline_t *pipe, int target) {
	blend_desc->RenderTarget[target].BlendEnable = pipe->blend_source != IRON_GPU_BLEND_ONE || pipe->blend_destination != IRON_GPU_BLEND_ZERO ||
	                                               pipe->alpha_blend_source != IRON_GPU_BLEND_ONE || pipe->alpha_blend_destination != IRON_GPU_BLEND_ZERO;
	blend_desc->RenderTarget[target].SrcBlend = convert_blend_factor(pipe->blend_source);
	blend_desc->RenderTarget[target].DestBlend = convert_blend_factor(pipe->blend_destination);
	blend_desc->RenderTarget[target].BlendOp = convert_blend_operation(pipe->blend_operation);
	blend_desc->RenderTarget[target].SrcBlendAlpha = convert_blend_factor(pipe->alpha_blend_source);
	blend_desc->RenderTarget[target].DestBlendAlpha = convert_blend_factor(pipe->alpha_blend_destination);
	blend_desc->RenderTarget[target].BlendOpAlpha = convert_blend_operation(pipe->alpha_blend_operation);
	blend_desc->RenderTarget[target].RenderTargetWriteMask =
	    (((pipe->color_write_mask_red[target] ? D3D12_COLOR_WRITE_ENABLE_RED : 0) |
		  (pipe->color_write_mask_green[target] ? D3D12_COLOR_WRITE_ENABLE_GREEN : 0)) |
	      (pipe->color_write_mask_blue[target] ? D3D12_COLOR_WRITE_ENABLE_BLUE : 0)) |
	      (pipe->color_write_mask_alpha[target] ? D3D12_COLOR_WRITE_ENABLE_ALPHA : 0);
}

void iron_gpu_pipeline_compile(iron_gpu_pipeline_t *pipe) {
	int vertexAttributeCount = pipe->input_layout->size;

	D3D12_INPUT_ELEMENT_DESC *vertexDesc = (D3D12_INPUT_ELEMENT_DESC *)alloca(sizeof(D3D12_INPUT_ELEMENT_DESC) * vertexAttributeCount);
	ZeroMemory(vertexDesc, sizeof(D3D12_INPUT_ELEMENT_DESC) * vertexAttributeCount);

	for (int i = 0; i < pipe->input_layout->size; ++i) {
		vertexDesc[i].SemanticName = "TEXCOORD";
		vertexDesc[i].SemanticIndex = findAttribute(pipe->vertex_shader, pipe->input_layout->elements[i].name).attribute;
		vertexDesc[i].InputSlot = 0;
		vertexDesc[i].AlignedByteOffset = (i == 0) ? 0 : D3D12_APPEND_ALIGNED_ELEMENT;
		vertexDesc[i].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		vertexDesc[i].InstanceDataStepRate = 0;

		switch (pipe->input_layout->elements[i].data) {
		case IRON_GPU_VERTEX_DATA_F32_1X:
			vertexDesc[i].Format = DXGI_FORMAT_R32_FLOAT;
			break;
		case IRON_GPU_VERTEX_DATA_F32_2X:
			vertexDesc[i].Format = DXGI_FORMAT_R32G32_FLOAT;
			break;
		case IRON_GPU_VERTEX_DATA_F32_3X:
			vertexDesc[i].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			break;
		case IRON_GPU_VERTEX_DATA_F32_4X:
			vertexDesc[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			break;
		case IRON_GPU_VERTEX_DATA_U8_4X_NORM:
			vertexDesc[i].Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			break;
		case IRON_GPU_VERTEX_DATA_I16_2X_NORM:
			vertexDesc[i].Format = DXGI_FORMAT_R16G16_SNORM;
			break;
		case IRON_GPU_VERTEX_DATA_I16_4X_NORM:
			vertexDesc[i].Format = DXGI_FORMAT_R16G16B16A16_SNORM;
			break;
		default:
			break;
		}
	}

	HRESULT hr = S_OK;
	pipe->impl.textures = pipe->fragment_shader->impl.texturesCount;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {0};
	psoDesc.VS.BytecodeLength = pipe->vertex_shader->impl.length;
	psoDesc.VS.pShaderBytecode = pipe->vertex_shader->impl.data;
	psoDesc.PS.BytecodeLength = pipe->fragment_shader->impl.length;
	psoDesc.PS.pShaderBytecode = pipe->fragment_shader->impl.data;
	psoDesc.pRootSignature = globalRootSignature;
	psoDesc.NumRenderTargets = pipe->color_attachment_count;
	for (int i = 0; i < pipe->color_attachment_count; ++i) {
		psoDesc.RTVFormats[i] = convert_format(pipe->color_attachment[i]);
	}
	psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;

	psoDesc.InputLayout.NumElements = vertexAttributeCount;
	psoDesc.InputLayout.pInputElementDescs = vertexDesc;

	psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	psoDesc.RasterizerState.CullMode = convert_cull_mode(pipe->cull_mode);
	psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
	psoDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	psoDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	psoDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	psoDesc.RasterizerState.DepthClipEnable = TRUE;
	psoDesc.RasterizerState.MultisampleEnable = FALSE;
	psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;
	psoDesc.RasterizerState.ForcedSampleCount = 0;
	psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
	psoDesc.BlendState.IndependentBlendEnable = FALSE;
	const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc = {
	    FALSE,
	    FALSE,
	    D3D12_BLEND_ONE,
	    D3D12_BLEND_ZERO,
	    D3D12_BLEND_OP_ADD,
	    D3D12_BLEND_ONE,
	    D3D12_BLEND_ZERO,
	    D3D12_BLEND_OP_ADD,
	    D3D12_LOGIC_OP_NOOP,
	    D3D12_COLOR_WRITE_ENABLE_ALL,
	};
	for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i) {
		psoDesc.BlendState.RenderTarget[i] = defaultRenderTargetBlendDesc;
	}

	bool independentBlend = false;
	for (int i = 1; i < 8; ++i) {
		if (pipe->color_write_mask_red[0] != pipe->color_write_mask_red[i] ||
			pipe->color_write_mask_green[0] != pipe->color_write_mask_green[i] ||
		    pipe->color_write_mask_blue[0] != pipe->color_write_mask_blue[i] ||
			pipe->color_write_mask_alpha[0] != pipe->color_write_mask_alpha[i]) {
			independentBlend = true;
			break;
		}
	}

	set_blend_state(&psoDesc.BlendState, pipe, 0);
	if (independentBlend) {
		psoDesc.BlendState.IndependentBlendEnable = true;
		for (int i = 1; i < 8; ++i) {
			set_blend_state(&psoDesc.BlendState, pipe, i);
		}
	}

	psoDesc.DepthStencilState.DepthEnable = TRUE;
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	psoDesc.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp = {D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_NEVER};
	psoDesc.DepthStencilState.FrontFace = defaultStencilOp;
	psoDesc.DepthStencilState.BackFace = defaultStencilOp;

	psoDesc.DepthStencilState.DepthEnable = pipe->depth_mode != IRON_GPU_COMPARE_MODE_ALWAYS;
	psoDesc.DepthStencilState.DepthWriteMask = pipe->depth_write ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
	psoDesc.DepthStencilState.DepthFunc = convert_compare_mode(pipe->depth_mode);
	psoDesc.DepthStencilState.StencilEnable = false;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleMask = 0xFFFFFFFF;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	hr = device->lpVtbl->CreateGraphicsPipelineState(device , &psoDesc, &IID_ID3D12PipelineState, &pipe->impl.pso);
	if (hr != S_OK) {
		iron_log("Could not create pipeline.");
	}
}

static D3D12_TEXTURE_ADDRESS_MODE convert_texture_addressing(iron_gpu_texture_addressing_t addressing) {
	switch (addressing) {
	case IRON_GPU_TEXTURE_ADDRESSING_REPEAT:
		return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	case IRON_GPU_TEXTURE_ADDRESSING_MIRROR:
		return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
	case IRON_GPU_TEXTURE_ADDRESSING_CLAMP:
		return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	case IRON_GPU_TEXTURE_ADDRESSING_BORDER:
		return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	default:
		assert(false);
		return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	}
}

static D3D12_FILTER convert_filter(iron_gpu_texture_filter_t minification, iron_gpu_texture_filter_t magnification, iron_gpu_mipmap_filter_t mipmap) {
	switch (minification) {
	case IRON_GPU_TEXTURE_FILTER_POINT:
		switch (magnification) {
		case IRON_GPU_TEXTURE_FILTER_POINT:
			switch (mipmap) {
			case IRON_GPU_MIPMAP_FILTER_NONE:
			case IRON_GPU_MIPMAP_FILTER_POINT:
				return D3D12_FILTER_MIN_MAG_MIP_POINT;
			case IRON_GPU_MIPMAP_FILTER_LINEAR:
				return D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
			}
		case IRON_GPU_TEXTURE_FILTER_LINEAR:
			switch (mipmap) {
			case IRON_GPU_MIPMAP_FILTER_NONE:
			case IRON_GPU_MIPMAP_FILTER_POINT:
				return D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
			case IRON_GPU_MIPMAP_FILTER_LINEAR:
				return D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
			}
		case IRON_GPU_TEXTURE_FILTER_ANISOTROPIC:
			return D3D12_FILTER_ANISOTROPIC;
		}
	case IRON_GPU_TEXTURE_FILTER_LINEAR:
		switch (magnification) {
		case IRON_GPU_TEXTURE_FILTER_POINT:
			switch (mipmap) {
			case IRON_GPU_MIPMAP_FILTER_NONE:
			case IRON_GPU_MIPMAP_FILTER_POINT:
				return D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
			case IRON_GPU_MIPMAP_FILTER_LINEAR:
				return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			}
		case IRON_GPU_TEXTURE_FILTER_LINEAR:
			switch (mipmap) {
			case IRON_GPU_MIPMAP_FILTER_NONE:
			case IRON_GPU_MIPMAP_FILTER_POINT:
				return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			case IRON_GPU_MIPMAP_FILTER_LINEAR:
				return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			}
		case IRON_GPU_TEXTURE_FILTER_ANISOTROPIC:
			return D3D12_FILTER_ANISOTROPIC;
		}
	case IRON_GPU_TEXTURE_FILTER_ANISOTROPIC:
		return D3D12_FILTER_ANISOTROPIC;
	}

	assert(false);
	return D3D12_FILTER_MIN_MAG_MIP_POINT;
}

void iron_gpu_sampler_init(iron_gpu_sampler_t *sampler, const iron_gpu_sampler_options_t *options) {
	D3D12_DESCRIPTOR_HEAP_DESC descHeapSampler = {};
	descHeapSampler.NumDescriptors = 2;
	descHeapSampler.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	descHeapSampler.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	device->lpVtbl->CreateDescriptorHeap(device, &descHeapSampler, &IID_ID3D12DescriptorHeap, &sampler->impl.sampler_heap);

	D3D12_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D12_SAMPLER_DESC));
	samplerDesc.Filter = convert_filter(options->minification_filter, options->magnification_filter, options->mipmap_filter);
	samplerDesc.AddressU = convert_texture_addressing(options->u_addressing);
	samplerDesc.AddressV = convert_texture_addressing(options->v_addressing);
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = 32;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	D3D12_CPU_DESCRIPTOR_HANDLE handle;
	sampler->impl.sampler_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(sampler->impl.sampler_heap, &handle);
	device->lpVtbl->CreateSampler(device, &samplerDesc, handle);
}

void iron_gpu_sampler_destroy(iron_gpu_sampler_t *sampler) {}

void iron_gpu_shader_init(iron_gpu_shader_t *shader, const void *_data, size_t length, iron_gpu_shader_type_t type) {
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

void iron_gpu_shader_destroy(iron_gpu_shader_t *shader) {
	free(shader->impl.data);
}

// djb2
uint32_t iron_internal_hash_name(unsigned char *str) {
	unsigned long hash = 5381;
	int c;
	while ((c = *str++)) {
		hash = hash * 33 ^ c;
	}
	return hash;
}

static const int heapSize = 1024;

void iron_gpu_internal_reset_textures(struct iron_gpu_command_list *list) {
	for (int i = 0; i < IRON_INTERNAL_G5_TEXTURE_COUNT; ++i) {
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

static DXGI_FORMAT convertImageFormat(iron_image_format_t format) {
	switch (format) {
	case IRON_IMAGE_FORMAT_RGBA128:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case IRON_IMAGE_FORMAT_RGBA64:
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case IRON_IMAGE_FORMAT_R8:
		return DXGI_FORMAT_R8_UNORM;
	case IRON_IMAGE_FORMAT_BGRA32:
		return DXGI_FORMAT_B8G8R8A8_UNORM;
	case IRON_IMAGE_FORMAT_RGBA32:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	default:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	}
}

static int formatByteSize(iron_image_format_t format) {
	switch (format) {
	case IRON_IMAGE_FORMAT_RGBA128:
		return 16;
	case IRON_IMAGE_FORMAT_RGBA64:
		return 8;
	case IRON_IMAGE_FORMAT_R8:
		return 1;
	case IRON_IMAGE_FORMAT_R16:
		return 2;
	case IRON_IMAGE_FORMAT_BGRA32:
	case IRON_IMAGE_FORMAT_RGBA32:
	case IRON_IMAGE_FORMAT_R32:
		return 4;
	default:
		return 4;
	}
}

void iron_gpu_internal_set_textures(iron_gpu_command_list_t *list) {
	if (list->impl.currentTextures[0] != NULL) {
		int srvStep = device->lpVtbl->GetDescriptorHandleIncrementSize(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		int samplerStep = device->lpVtbl->GetDescriptorHandleIncrementSize(device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

		if (list->impl.heapIndex + IRON_INTERNAL_G5_TEXTURE_COUNT >= heapSize) {
			list->impl.heapIndex = 0;
		}

		D3D12_GPU_DESCRIPTOR_HANDLE srvGpu;
		list->impl.srvHeap->lpVtbl->GetGPUDescriptorHandleForHeapStart(list->impl.srvHeap, &srvGpu);
		D3D12_GPU_DESCRIPTOR_HANDLE samplerGpu;
		list->impl.samplerHeap->lpVtbl->GetGPUDescriptorHandleForHeapStart(list->impl.samplerHeap, &samplerGpu);
		srvGpu.ptr += list->impl.heapIndex * srvStep;
		samplerGpu.ptr += list->impl.heapIndex * samplerStep;

		for (int i = 0; i < IRON_INTERNAL_G5_TEXTURE_COUNT; ++i) {
			if (list->impl.currentTextures[i] != NULL && list->impl.current_samplers[i] != NULL) {
				ID3D12DescriptorHeap *samplerDescriptorHeap = list->impl.current_samplers[i]->impl.sampler_heap;

				D3D12_CPU_DESCRIPTOR_HANDLE srvCpu;
				list->impl.srvHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(list->impl.srvHeap, &srvCpu);
				D3D12_CPU_DESCRIPTOR_HANDLE samplerCpu;
				list->impl.samplerHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(list->impl.samplerHeap, &samplerCpu);
				srvCpu.ptr += list->impl.heapIndex * srvStep;
				samplerCpu.ptr += list->impl.heapIndex * samplerStep;
				++list->impl.heapIndex;

				bool is_depth = list->impl.currentTextures[i]->impl.stage_depth == i;
				D3D12_CPU_DESCRIPTOR_HANDLE sourceCpu;

				if (is_depth) {
					list->impl.currentTextures[i]->impl.srvDepthDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(
						list->impl.currentTextures[i]->impl.srvDepthDescriptorHeap, &sourceCpu);
				}
				else {
					list->impl.currentTextures[i]->impl.srvDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(
						list->impl.currentTextures[i]->impl.srvDescriptorHeap, &sourceCpu);
				}

				D3D12_CPU_DESCRIPTOR_HANDLE handle;
				samplerDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(samplerDescriptorHeap, &handle);

				device->lpVtbl->CopyDescriptorsSimple(device, 1, srvCpu, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				device->lpVtbl->CopyDescriptorsSimple(device, 1, samplerCpu, handle, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
			}
		}

		ID3D12DescriptorHeap *heaps[2] = {list->impl.srvHeap, list->impl.samplerHeap};
		list->impl._commandList->lpVtbl->SetDescriptorHeaps(list->impl._commandList, 2, heaps);
		list->impl._commandList->lpVtbl->SetGraphicsRootDescriptorTable(list->impl._commandList, 0, srvGpu);
		list->impl._commandList->lpVtbl->SetGraphicsRootDescriptorTable(list->impl._commandList, 1, samplerGpu);
	}
}

void createHeaps(iron_gpu_command_list_t *list) {
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

void iron_memory_emergency();

void iron_gpu_texture_init_from_bytes(iron_gpu_texture_t *texture, void *data, int width, int height, iron_image_format_t format) {
	memset(&texture->impl, 0, sizeof(texture->impl));
	texture->impl.stage = 0;
	texture->impl.stage_depth = -1;
	texture->impl.mipmap = true;
	texture->width = width;
	texture->height = height;
	texture->_uploaded = false;
	texture->format = format;
	texture->data = data;
	texture->state = IRON_INTERNAL_RENDER_TARGET_STATE_TEXTURE;
	texture->framebuffer_index = -1;
	texture->impl.renderTarget = NULL;

	DXGI_FORMAT d3dformat = convertImageFormat(format);
	int formatSize = formatByteSize(format);

	D3D12_HEAP_PROPERTIES heapPropertiesDefault;
	heapPropertiesDefault.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapPropertiesDefault.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapPropertiesDefault.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapPropertiesDefault.CreationNodeMask = 1;
	heapPropertiesDefault.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC resourceDescTex;
	resourceDescTex.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDescTex.Alignment = 0;
	resourceDescTex.Width = texture->width;
	resourceDescTex.Height = texture->height;
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
			iron_memory_emergency();
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
			iron_memory_emergency();
			result = device->lpVtbl->CreateCommittedResource(device, &heapPropertiesUpload, D3D12_HEAP_FLAG_NONE, &resourceDescBuffer,
			                                                 D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
			                                         &IID_ID3D12Resource, &texture->impl.uploadImage);
			if (result == S_OK) {
				break;
			}
		}
	}

	texture->impl.stride = (int)ceilf(uploadBufferSize / (float)(height * d3d12_textureAlignment())) * d3d12_textureAlignment();

	BYTE *pixel;
	texture->impl.uploadImage->lpVtbl->Map(texture->impl.uploadImage, 0, NULL, (void **)&pixel);
	int pitch = iron_gpu_texture_stride(texture);
	for (int y = 0; y < texture->height; ++y) {
		memcpy(&pixel[y * pitch], &((uint8_t *)data)[y * texture->width * formatSize], texture->width * formatSize);
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

void create_texture(struct iron_gpu_texture *texture, int width, int height, iron_image_format_t format, D3D12_RESOURCE_FLAGS flags) {
	memset(&texture->impl, 0, sizeof(texture->impl));
	texture->impl.stage = 0;
	texture->impl.stage_depth = -1;
	texture->impl.mipmap = true;
	texture->width = width;
	texture->height = height;
	texture->data = NULL;
	texture->framebuffer_index = -1;
	texture->impl.renderTarget = NULL;

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
	resourceDescTex.Width = texture->width;
	resourceDescTex.Height = texture->height;
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
			iron_memory_emergency();
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
			iron_memory_emergency();
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

void iron_gpu_texture_init(struct iron_gpu_texture *texture, int width, int height, iron_image_format_t format) {
	create_texture(texture, width, height, format, D3D12_RESOURCE_FLAG_NONE);
	texture->_uploaded = true;
	texture->state = IRON_INTERNAL_RENDER_TARGET_STATE_TEXTURE;
	texture->format = format;
}

void iron_gpu_texture_init_non_sampled_access(struct iron_gpu_texture *texture, int width, int height, iron_image_format_t format) {
	create_texture(texture, width, height, format, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	texture->_uploaded = true;
	texture->state = IRON_INTERNAL_RENDER_TARGET_STATE_TEXTURE;
}

void iron_gpu_texture_destroy(iron_gpu_texture_t *render_target) {
	if (render_target->impl.framebuffer_index >= 0) {
		framebuffer_count -= 1;
	}

	if (render_target->impl.renderTarget != NULL) {
		render_target->impl.renderTarget->lpVtbl->Release(render_target->impl.renderTarget);
		render_target->impl.renderTargetDescriptorHeap->lpVtbl->Release(render_target->impl.renderTargetDescriptorHeap);
		render_target->impl.srvDescriptorHeap->lpVtbl->Release(render_target->impl.srvDescriptorHeap);
		if (render_target->impl.depthStencilTexture != NULL) {
			render_target->impl.depthStencilTexture->lpVtbl->Release(render_target->impl.depthStencilTexture);
			render_target->impl.depthStencilDescriptorHeap->lpVtbl->Release(render_target->impl.depthStencilDescriptorHeap);
			render_target->impl.srvDepthDescriptorHeap->lpVtbl->Release(render_target->impl.srvDepthDescriptorHeap);
		}
		if (render_target->impl.renderTargetReadback != NULL) {
			render_target->impl.renderTargetReadback->lpVtbl->Release(render_target->impl.renderTargetReadback);
		}
	}

	if (render_target->impl.image != NULL) {
		render_target->impl.image->lpVtbl->Release(render_target->impl.image);
		render_target->impl.uploadImage->lpVtbl->Release(render_target->impl.uploadImage);
		// render_target->impl.srvDescriptorHeap->lpVtbl->Release(render_target->impl.srvDescriptorHeap);
	}
}

void iron_gpu_internal_texture_unmipmap(struct iron_gpu_texture *texture) {
	texture->impl.mipmap = false;
}

void iron_gpu_internal_sampler_set(iron_gpu_command_list_t *list, iron_gpu_sampler_t *sampler, int unit) {
	if (unit < 0)
		return;

	list->impl.current_samplers[unit] = sampler;
}

int iron_gpu_texture_stride(struct iron_gpu_texture *texture) {
	return texture->impl.stride;
}

void iron_gpu_texture_generate_mipmaps(struct iron_gpu_texture *texture, int levels) {}

void iron_gpu_texture_set_mipmap(struct iron_gpu_texture *texture, struct iron_gpu_texture *mipmap, int level) {}

static void WaitForFence(ID3D12Fence *fence, UINT64 completionValue, HANDLE waitEvent) {
	if (fence->lpVtbl->GetCompletedValue(fence) < completionValue) {
		fence->lpVtbl->SetEventOnCompletion(fence, completionValue, waitEvent);
		WaitForSingleObject(waitEvent, INFINITE);
	}
}

static void createRenderTargetView(ID3D12Resource *renderTarget, ID3D12DescriptorHeap *renderTargetDescriptorHeap, DXGI_FORMAT format) {

	D3D12_RENDER_TARGET_VIEW_DESC viewDesc;
	viewDesc.Format = format;
	viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	viewDesc.Texture2D.MipSlice = 0;
	viewDesc.Texture2D.PlaneSlice = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE handle;
	renderTargetDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(renderTargetDescriptorHeap, &handle);
	device->lpVtbl->CreateRenderTargetView(device, renderTarget, &viewDesc, handle);
}

static DXGI_FORMAT convertFormat(iron_image_format_t format) {
	switch (format) {
	case IRON_IMAGE_FORMAT_RGBA128:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case IRON_IMAGE_FORMAT_RGBA64:
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case IRON_IMAGE_FORMAT_R32:
		return DXGI_FORMAT_R32_FLOAT;
	case IRON_IMAGE_FORMAT_R16:
		return DXGI_FORMAT_R16_FLOAT;
	case IRON_IMAGE_FORMAT_R8:
		return DXGI_FORMAT_R8_UNORM;
	case IRON_IMAGE_FORMAT_RGBA32:
	default:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	}
}

void iron_memory_emergency();

static void render_target_init(iron_gpu_texture_t *render_target, int width, int height, iron_image_format_t format, int depthBufferBits, int framebuffer_index) {
	render_target->width = render_target->width = width;
	render_target->height = render_target->height = height;
	render_target->impl.stage = 0;
	render_target->impl.stage_depth = -1;
	render_target->impl.renderTargetReadback = NULL;
	render_target->impl.framebuffer_index = framebuffer_index;
	render_target->impl.image = NULL;
	render_target->impl.uploadImage = NULL;
	render_target->data = NULL;

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
	texResourceDesc.Width = render_target->width;
	texResourceDesc.Height = render_target->height;
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
	iron_microsoft_affirm(device->lpVtbl->CreateDescriptorHeap(device , &heapDesc, &IID_ID3D12DescriptorHeap, &render_target->impl.renderTargetDescriptorHeap));

	if (framebuffer_index >= 0) {
		IDXGISwapChain *swapChain = iron_dx_current_window()->swapChain;
		iron_microsoft_affirm(swapChain->lpVtbl->GetBuffer(swapChain, framebuffer_index, &IID_ID3D12Resource, &render_target->impl.renderTarget));
		wchar_t buffer[128];
		wsprintf(buffer, L"Backbuffer (index %i)", framebuffer_index);
		render_target->impl.renderTarget->lpVtbl->SetName(render_target->impl.renderTarget, buffer);

		createRenderTargetView(render_target->impl.renderTarget, render_target->impl.renderTargetDescriptorHeap, dxgiFormat);
	}
	else {
		HRESULT result = device->lpVtbl->CreateCommittedResource(device, &heapProperties, D3D12_HEAP_FLAG_NONE, &texResourceDesc, D3D12_RESOURCE_STATE_RENDER_TARGET,
		                                                 &clearValue, &IID_ID3D12Resource, &render_target->impl.renderTarget);
		if (result != S_OK) {
			for (int i = 0; i < 10; ++i) {
				iron_memory_emergency();
				result = device->lpVtbl->CreateCommittedResource(device, &heapProperties, D3D12_HEAP_FLAG_NONE, &texResourceDesc, D3D12_RESOURCE_STATE_RENDER_TARGET,
				                                         &clearValue, &IID_ID3D12Resource, &render_target->impl.renderTarget);
				if (result == S_OK) {
					break;
				}
			}
		}

		D3D12_RENDER_TARGET_VIEW_DESC view;
		view.Format = dxgiFormat;
		view.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		view.Texture2D.MipSlice = 0;
		view.Texture2D.PlaneSlice = 0;

		D3D12_CPU_DESCRIPTOR_HANDLE handle;
		render_target->impl.renderTargetDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(render_target->impl.renderTargetDescriptorHeap, &handle);

		device->lpVtbl->CreateRenderTargetView(device, render_target->impl.renderTarget, &view,
		    handle);
	}

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
	descriptorHeapDesc.NumDescriptors = 1;

	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorHeapDesc.NodeMask = 0;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	iron_microsoft_affirm(device->lpVtbl->CreateDescriptorHeap(device, &descriptorHeapDesc, &IID_ID3D12DescriptorHeap, &render_target->impl.srvDescriptorHeap));

	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shaderResourceViewDesc.Format = dxgiFormat;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	D3D12_CPU_DESCRIPTOR_HANDLE handle;
	render_target->impl.srvDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(render_target->impl.srvDescriptorHeap, &handle);

	device->lpVtbl->CreateShaderResourceView(device, render_target->impl.renderTarget, &shaderResourceViewDesc,
	    handle);

	if (depthBufferBits > 0) {
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		iron_microsoft_affirm(device->lpVtbl->CreateDescriptorHeap(device, &dsvHeapDesc, &IID_ID3D12DescriptorHeap, &render_target->impl.depthStencilDescriptorHeap));

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

		HRESULT result = device->lpVtbl->CreateCommittedResource(device, &heapProperties, D3D12_HEAP_FLAG_NONE, &depthTexture, D3D12_RESOURCE_STATE_DEPTH_WRITE,
		                                                         &clearValue,
		                                                 &IID_ID3D12Resource, &render_target->impl.depthStencilTexture);
		if (result != S_OK) {
			for (int i = 0; i < 10; ++i) {
				iron_memory_emergency();
				result = device->lpVtbl->CreateCommittedResource(device, &heapProperties, D3D12_HEAP_FLAG_NONE, &depthTexture, D3D12_RESOURCE_STATE_DEPTH_WRITE,
				                                                 &clearValue,
				                                         &IID_ID3D12Resource, &render_target->impl.depthStencilTexture);
				if (result == S_OK) {
					break;
				}
			}
		}

		D3D12_CPU_DESCRIPTOR_HANDLE handle;
		render_target->impl.depthStencilDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(render_target->impl.depthStencilDescriptorHeap, &handle);
		device->lpVtbl->CreateDepthStencilView(device, render_target->impl.depthStencilTexture, NULL,
		    handle);

		// Reading depth texture as a shader resource
		D3D12_DESCRIPTOR_HEAP_DESC srvDepthHeapDesc = {};
		srvDepthHeapDesc.NumDescriptors = 1;
		srvDepthHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvDepthHeapDesc.NodeMask = 0;
		srvDepthHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		iron_microsoft_affirm(device->lpVtbl->CreateDescriptorHeap(device, &srvDepthHeapDesc, &IID_ID3D12DescriptorHeap, &render_target->impl.srvDepthDescriptorHeap));

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDepthViewDesc;
		ZeroMemory(&srvDepthViewDesc, sizeof(srvDepthViewDesc));
		srvDepthViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDepthViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDepthViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDepthViewDesc.Texture2D.MipLevels = 1;
		srvDepthViewDesc.Texture2D.MostDetailedMip = 0;
		srvDepthViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;

		render_target->impl.srvDepthDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(render_target->impl.srvDepthDescriptorHeap, &handle);
		device->lpVtbl->CreateShaderResourceView(device, render_target->impl.depthStencilTexture, &srvDepthViewDesc,
		    handle);
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

void iron_gpu_render_target_init(iron_gpu_texture_t *target, int width, int height, iron_image_format_t format, int depthBufferBits) {
	render_target_init(target, width, height, format, depthBufferBits, -1);
	target->_uploaded = true;
	target->state = IRON_INTERNAL_RENDER_TARGET_STATE_RENDER_TARGET;
}

void iron_gpu_render_target_init_framebuffer(iron_gpu_texture_t *target, int width, int height, iron_image_format_t format, int depthBufferBits) {
	render_target_init(target, width, height, format, depthBufferBits, framebuffer_count);
	framebuffer_count += 1;
	target->_uploaded = true;
	target->state = IRON_INTERNAL_RENDER_TARGET_STATE_RENDER_TARGET;
}

void iron_gpu_render_target_set_depth_from(iron_gpu_texture_t *render_target, iron_gpu_texture_t *source) {
	render_target->impl.depthStencilDescriptorHeap = source->impl.depthStencilDescriptorHeap;
	render_target->impl.srvDepthDescriptorHeap = source->impl.srvDepthDescriptorHeap;
	render_target->impl.depthStencilTexture = source->impl.depthStencilTexture;
}

iron_gpu_vertex_buffer_t *_current_vertex_buffer = NULL;

void iron_gpu_vertex_buffer_init(iron_gpu_vertex_buffer_t *buffer, int count, iron_gpu_vertex_structure_t *structure, bool gpuMemory) {
	buffer->impl.myCount = count;

	buffer->impl.myStride = 0;
	for (int i = 0; i < structure->size; ++i) {
		buffer->impl.myStride += iron_gpu_vertex_data_size(structure->elements[i].data);
	}

	int uploadBufferSize = buffer->impl.myStride * buffer->impl.myCount;

	D3D12_HEAP_PROPERTIES heapProperties;
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 1;
	heapProperties.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC resourceDesc;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = uploadBufferSize;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	device->lpVtbl->CreateCommittedResource(device, &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
	                                &IID_ID3D12Resource, &buffer->impl.uploadBuffer);

	buffer->impl.view.BufferLocation = buffer->impl.uploadBuffer->lpVtbl->GetGPUVirtualAddress(buffer->impl.uploadBuffer);
	buffer->impl.view.SizeInBytes = uploadBufferSize;
	buffer->impl.view.StrideInBytes = buffer->impl.myStride;

	buffer->impl.lastStart = 0;
	buffer->impl.lastCount = iron_gpu_vertex_buffer_count(buffer);
}

void iron_gpu_vertex_buffer_destroy(iron_gpu_vertex_buffer_t *buffer) {
	// buffer->impl.vertexBuffer->lpVtbl->Release();
	buffer->impl.uploadBuffer->lpVtbl->Release(buffer->impl.uploadBuffer);
}

float *iron_gpu_vertex_buffer_lock_all(iron_gpu_vertex_buffer_t *buffer) {
	return iron_gpu_vertex_buffer_lock(buffer, 0, iron_gpu_vertex_buffer_count(buffer));
}

float *iron_gpu_vertex_buffer_lock(iron_gpu_vertex_buffer_t *buffer, int start, int count) {
	buffer->impl.lastStart = start;
	buffer->impl.lastCount = count;

	D3D12_RANGE range;
	range.Begin = start * buffer->impl.myStride;
	range.End = (start + count) * buffer->impl.myStride;

	void *p;
	buffer->impl.uploadBuffer->lpVtbl->Map(buffer->impl.uploadBuffer, 0, &range, &p);
	byte *bytes = (byte *)p;
	bytes += start * buffer->impl.myStride;
	return (float *)bytes;
}

void iron_gpu_vertex_buffer_unlock_all(iron_gpu_vertex_buffer_t *buffer) {
	D3D12_RANGE range;
	range.Begin = buffer->impl.lastStart * buffer->impl.myStride;
	range.End = (buffer->impl.lastStart + buffer->impl.lastCount) * buffer->impl.myStride;
	buffer->impl.uploadBuffer->lpVtbl->Unmap(buffer->impl.uploadBuffer, 0, &range);
}

void iron_gpu_vertex_buffer_unlock(iron_gpu_vertex_buffer_t *buffer, int count) {
	D3D12_RANGE range;
	range.Begin = buffer->impl.lastStart * buffer->impl.myStride;
	range.End = (buffer->impl.lastStart + count) * buffer->impl.myStride;
	buffer->impl.uploadBuffer->lpVtbl->Unmap(buffer->impl.uploadBuffer, 0, &range);
}

int iron_gpu_internal_vertex_buffer_set(iron_gpu_vertex_buffer_t *buffer) {
	_current_vertex_buffer = buffer;
	return 0;
}

int iron_gpu_vertex_buffer_count(iron_gpu_vertex_buffer_t *buffer) {
	return buffer->impl.myCount;
}

int iron_gpu_vertex_buffer_stride(iron_gpu_vertex_buffer_t *buffer) {
	return buffer->impl.myStride;
}

bool iron_gpu_transpose_mat = false;

void iron_gpu_constant_buffer_init(iron_gpu_constant_buffer_t *buffer, int size) {
	buffer->impl.mySize = size;
	buffer->data = NULL;

	D3D12_HEAP_PROPERTIES heapProperties;
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 1;
	heapProperties.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC resourceDesc;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = size;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	iron_microsoft_affirm(device->lpVtbl->CreateCommittedResource(device , &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
	                                                              D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
	                                                      &IID_ID3D12Resource, &buffer->impl.constant_buffer));

	void *p;
	buffer->impl.constant_buffer->lpVtbl->Map(buffer->impl.constant_buffer, 0, NULL, &p);
	ZeroMemory(p, size);
	buffer->impl.constant_buffer->lpVtbl->Unmap(buffer->impl.constant_buffer, 0, NULL);
}

void iron_gpu_constant_buffer_destroy(iron_gpu_constant_buffer_t *buffer) {
	buffer->impl.constant_buffer->lpVtbl->Release(buffer->impl.constant_buffer);
}

void iron_gpu_constant_buffer_lock_all(iron_gpu_constant_buffer_t *buffer) {
	iron_gpu_constant_buffer_lock(buffer, 0, iron_gpu_constant_buffer_size(buffer));
}

void iron_gpu_constant_buffer_lock(iron_gpu_constant_buffer_t *buffer, int start, int count) {
	buffer->impl.lastStart = start;
	buffer->impl.lastCount = count;
	D3D12_RANGE range;
	range.Begin = start;
	range.End = range.Begin + count;
	uint8_t *p;
	buffer->impl.constant_buffer->lpVtbl->Map(buffer->impl.constant_buffer, 0, &range, (void **)&p);
	buffer->data = &p[start];
}

void iron_gpu_constant_buffer_unlock(iron_gpu_constant_buffer_t *buffer) {
	D3D12_RANGE range;
	range.Begin = buffer->impl.lastStart;
	range.End = range.Begin + buffer->impl.lastCount;
	buffer->impl.constant_buffer->lpVtbl->Unmap(buffer->impl.constant_buffer, 0, &range);
	buffer->data = NULL;
}

int iron_gpu_constant_buffer_size(iron_gpu_constant_buffer_t *buffer) {
	return buffer->impl.mySize;
}

void iron_gpu_index_buffer_init(iron_gpu_index_buffer_t *buffer, int count, bool gpuMemory) {
	buffer->impl.count = count;
	buffer->impl.gpu_memory = gpuMemory;

	int uploadBufferSize = sizeof(uint32_t) * count;

	D3D12_HEAP_PROPERTIES heapProperties;
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 1;
	heapProperties.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC resourceDesc;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = uploadBufferSize;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	iron_microsoft_affirm(device->lpVtbl->CreateCommittedResource(device , &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
	                                                              D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
	                                                      &IID_ID3D12Resource, &buffer->impl.upload_buffer));

	if (gpuMemory) {
		D3D12_HEAP_PROPERTIES heapProperties;
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask = 1;
		heapProperties.VisibleNodeMask = 1;

		iron_microsoft_affirm(device->lpVtbl->CreateCommittedResource(device , &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
			D3D12_RESOURCE_STATE_COMMON, NULL,
		                                                      &IID_ID3D12Resource, &buffer->impl.index_buffer));

		buffer->impl.index_buffer_view.BufferLocation = buffer->impl.index_buffer->lpVtbl->GetGPUVirtualAddress(buffer->impl.index_buffer);
	}
	else {
		buffer->impl.index_buffer_view.BufferLocation = buffer->impl.upload_buffer->lpVtbl->GetGPUVirtualAddress(buffer->impl.upload_buffer);
	}
	buffer->impl.index_buffer_view.SizeInBytes = uploadBufferSize;
	buffer->impl.index_buffer_view.Format = DXGI_FORMAT_R32_UINT;

	buffer->impl.last_start = 0;
	buffer->impl.last_count = iron_gpu_index_buffer_count(buffer);
}

void iron_gpu_index_buffer_destroy(iron_gpu_index_buffer_t *buffer) {
	if (buffer->impl.index_buffer != NULL) {
		buffer->impl.index_buffer->lpVtbl->Release(buffer->impl.index_buffer);
		buffer->impl.index_buffer = NULL;
	}

	buffer->impl.upload_buffer->lpVtbl->Release(buffer->impl.upload_buffer);
	buffer->impl.upload_buffer = NULL;
}

static int iron_gpu_internal_index_buffer_stride(iron_gpu_index_buffer_t *buffer) {
	return 4;
}

void *iron_gpu_index_buffer_lock_all(iron_gpu_index_buffer_t *buffer) {
	return iron_gpu_index_buffer_lock(buffer, 0, iron_gpu_index_buffer_count(buffer));
}

void *iron_gpu_index_buffer_lock(iron_gpu_index_buffer_t *buffer, int start, int count) {
	buffer->impl.last_start = start;
	buffer->impl.last_count = count;

	D3D12_RANGE range;
	range.Begin = start * iron_gpu_internal_index_buffer_stride(buffer);
	range.End = (start + count) * iron_gpu_internal_index_buffer_stride(buffer);

	void *p;
	buffer->impl.upload_buffer->lpVtbl->Map(buffer->impl.upload_buffer, 0, &range, &p);
	byte *bytes = (byte *)p;
	bytes += start * iron_gpu_internal_index_buffer_stride(buffer);
	return bytes;
}

void iron_gpu_index_buffer_unlock_all(iron_gpu_index_buffer_t *buffer) {
	D3D12_RANGE range;
	range.Begin = buffer->impl.last_start * iron_gpu_internal_index_buffer_stride(buffer);
	range.End = (buffer->impl.last_start + buffer->impl.last_count) * iron_gpu_internal_index_buffer_stride(buffer);

	buffer->impl.upload_buffer->lpVtbl->Unmap(buffer->impl.upload_buffer, 0, &range);
}

void iron_gpu_index_buffer_unlock(iron_gpu_index_buffer_t *buffer, int count) {
	D3D12_RANGE range;
	range.Begin = buffer->impl.last_start * iron_gpu_internal_index_buffer_stride(buffer);
	range.End = (buffer->impl.last_start + count) * iron_gpu_internal_index_buffer_stride(buffer);

	buffer->impl.upload_buffer->lpVtbl->Unmap(buffer->impl.upload_buffer, 0, &range);
}

void iron_gpu_internal_index_buffer_upload(iron_gpu_index_buffer_t *buffer, ID3D12GraphicsCommandList *commandList) {
	if (!buffer->impl.gpu_memory)
		return;

	commandList->lpVtbl->CopyBufferRegion(commandList, buffer->impl.index_buffer, 0, buffer->impl.upload_buffer, 0, sizeof(uint32_t) * buffer->impl.count);

	D3D12_RESOURCE_BARRIER barriers[1] = {};
	barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barriers[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barriers[0].Transition.pResource = buffer->impl.index_buffer;
	barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_INDEX_BUFFER;
	barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	commandList->lpVtbl->ResourceBarrier(commandList, 1, barriers);
}

int iron_gpu_index_buffer_count(iron_gpu_index_buffer_t *buffer) {
	return buffer->impl.count;
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
static iron_gpu_raytrace_acceleration_structure_t *accel;
static iron_gpu_raytrace_pipeline_t *pipeline;
static iron_gpu_texture_t *output = NULL;
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

static iron_gpu_vertex_buffer_t *vb[16];
static iron_gpu_vertex_buffer_t *vb_last[16];
static iron_gpu_index_buffer_t *ib[16];
static int vb_count = 0;
static int vb_count_last = 0;
static inst_t instances[1024];
static int instances_count = 0;

void iron_gpu_raytrace_pipeline_init(iron_gpu_raytrace_pipeline_t *pipeline, iron_gpu_command_list_t *command_list, void *ray_shader, int ray_shader_size,
                                 iron_gpu_constant_buffer_t *constant_buffer) {
	output = NULL;
	descriptorsAllocated = 0;
	pipeline->_constant_buffer = constant_buffer;
	// Descriptor heap
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
	// Allocate a heap for 3 descriptors:
	// 2 - bottom and top level acceleration structure
	// 1 - raytracing output texture SRV
	descriptorHeapDesc.NumDescriptors = 12;
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descriptorHeapDesc.NodeMask = 0;
	if (descriptorHeap != NULL)
		descriptorHeap->lpVtbl->Release(descriptorHeap);
	device->lpVtbl->CreateDescriptorHeap(device , &descriptorHeapDesc, &IID_ID3D12DescriptorHeap, &descriptorHeap);
	descriptorSize = device->lpVtbl->GetDescriptorHandleIncrementSize(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Device
	if (dxrDevice != NULL)
		dxrDevice->lpVtbl->Release(dxrDevice);
	device->lpVtbl->QueryInterface(device , &IID_ID3D12Device5, &dxrDevice);
	if (dxrCommandList != NULL)
		dxrCommandList->lpVtbl->Release(dxrCommandList);
	command_list->impl._commandList->lpVtbl->QueryInterface(command_list->impl._commandList , &IID_ID3D12GraphicsCommandList4, &dxrCommandList);

	// Root signatures
	// This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.
	D3D12_DESCRIPTOR_RANGE UAVDescriptor = {};
	UAVDescriptor.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	UAVDescriptor.NumDescriptors = 1;
	UAVDescriptor.BaseShaderRegister = 0;
	UAVDescriptor.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_DESCRIPTOR_RANGE SRVDescriptorA = {};
	SRVDescriptorA.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	SRVDescriptorA.NumDescriptors = 1;
	SRVDescriptorA.BaseShaderRegister = 1;
	SRVDescriptorA.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_DESCRIPTOR_RANGE SRVDescriptorB = {};
	SRVDescriptorB.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	SRVDescriptorB.NumDescriptors = 1;
	SRVDescriptorB.BaseShaderRegister = 2;
	SRVDescriptorB.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_DESCRIPTOR_RANGE SRVDescriptor0 = {};
	SRVDescriptor0.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	SRVDescriptor0.NumDescriptors = 1;
	SRVDescriptor0.BaseShaderRegister = 3;
	SRVDescriptor0.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_DESCRIPTOR_RANGE SRVDescriptor1 = {};
	SRVDescriptor1.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	SRVDescriptor1.NumDescriptors = 1;
	SRVDescriptor1.BaseShaderRegister = 4;
	SRVDescriptor1.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_DESCRIPTOR_RANGE SRVDescriptor2 = {};
	SRVDescriptor2.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	SRVDescriptor2.NumDescriptors = 1;
	SRVDescriptor2.BaseShaderRegister = 5;
	SRVDescriptor2.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_DESCRIPTOR_RANGE SRVDescriptorEnv = {};
	SRVDescriptorEnv.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	SRVDescriptorEnv.NumDescriptors = 1;
	SRVDescriptorEnv.BaseShaderRegister = 6;
	SRVDescriptorEnv.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_DESCRIPTOR_RANGE SRVDescriptorSobol = {};
	SRVDescriptorSobol.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	SRVDescriptorSobol.NumDescriptors = 1;
	SRVDescriptorSobol.BaseShaderRegister = 7;
	SRVDescriptorSobol.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_DESCRIPTOR_RANGE SRVDescriptorScramble = {};
	SRVDescriptorScramble.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	SRVDescriptorScramble.NumDescriptors = 1;
	SRVDescriptorScramble.BaseShaderRegister = 8;
	SRVDescriptorScramble.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_DESCRIPTOR_RANGE SRVDescriptorRank = {};
	SRVDescriptorRank.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	SRVDescriptorRank.NumDescriptors = 1;
	SRVDescriptorRank.BaseShaderRegister = 9;
	SRVDescriptorRank.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

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

	D3D12_ROOT_SIGNATURE_DESC dxrRootSignatureDesc = {0};
	dxrRootSignatureDesc.NumParameters = ARRAYSIZE(rootParameters);
	dxrRootSignatureDesc.pParameters = rootParameters;
	ID3DBlob *blob = NULL;
	ID3DBlob *error = NULL;
	D3D12SerializeRootSignature(&dxrRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error);
	if (dxrRootSignature != NULL)
		dxrRootSignature->lpVtbl->Release(dxrRootSignature);
	device->lpVtbl->CreateRootSignature(device, 1, blob->lpVtbl->GetBufferPointer(blob), blob->lpVtbl->GetBufferSize(blob), &IID_ID3D12RootSignature,
	                                    &dxrRootSignature);

	// Pipeline
	D3D12_STATE_OBJECT_DESC raytracingPipeline = {};
	raytracingPipeline.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;

	D3D12_SHADER_BYTECODE shaderBytecode = {0};
	shaderBytecode.pShaderBytecode = ray_shader;
	shaderBytecode.BytecodeLength = ray_shader_size;

	D3D12_DXIL_LIBRARY_DESC dxilLibrary = {0};
	dxilLibrary.DXILLibrary = shaderBytecode;
	D3D12_EXPORT_DESC exports[3] = {0};
	exports[0].Name = raygen_shader_name;
	exports[1].Name = closesthit_shader_name;
	exports[2].Name = miss_shader_name;
	dxilLibrary.pExports = exports;
	dxilLibrary.NumExports = 3;

	D3D12_HIT_GROUP_DESC hitGroup = {0};
	hitGroup.ClosestHitShaderImport = closesthit_shader_name;
	hitGroup.HitGroupExport = hit_group_name;
	hitGroup.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;

	D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {0};
	shaderConfig.MaxPayloadSizeInBytes = 10 * sizeof(float); // float4 color
	shaderConfig.MaxAttributeSizeInBytes = 8 * sizeof(float); // float2 barycentrics

	D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {0};
	pipelineConfig.MaxTraceRecursionDepth = 1; // ~ primary rays only

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

	dxrDevice->lpVtbl->CreateStateObject(dxrDevice , &raytracingPipeline, &IID_ID3D12StateObject, &pipeline->impl.dxr_state);

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
		UINT size = shaderIdSize + constant_buffer->impl.mySize;
		UINT shaderRecordSize = (size + (align - 1)) & ~(align - 1);
		D3D12_RESOURCE_DESC bufferDesc = {};
		bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferDesc.Width = shaderRecordSize;
		bufferDesc.Height = 1;
		bufferDesc.DepthOrArraySize = 1;
		bufferDesc.MipLevels = 1;
		bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufferDesc.SampleDesc.Count = 1;
		bufferDesc.SampleDesc.Quality = 0;
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		D3D12_HEAP_PROPERTIES uploadHeapProperties = {};
		uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
		uploadHeapProperties.CreationNodeMask = 1;
		uploadHeapProperties.VisibleNodeMask = 1;

		device->lpVtbl->CreateCommittedResource(device, &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
		                                &IID_ID3D12Resource, &pipeline->impl.raygen_shader_table);

		D3D12_RANGE rstRange = {0};
		rstRange.Begin = 0;
		rstRange.End = 0;
		uint8_t *byteDest;
		pipeline->impl.raygen_shader_table->lpVtbl->Map(pipeline->impl.raygen_shader_table, 0, &rstRange, (void **)(&byteDest));

		D3D12_RANGE cbRange = {0};
		cbRange.Begin = 0;
		cbRange.End = constant_buffer->impl.mySize;
		void *constantBufferData;
		constant_buffer->impl.constant_buffer->lpVtbl->Map(constant_buffer->impl.constant_buffer, 0, &cbRange, (void **)&constantBufferData);
		memcpy(byteDest, rayGenShaderId, size);
		memcpy(byteDest + size, constantBufferData, constant_buffer->impl.mySize);
		pipeline->impl.raygen_shader_table->lpVtbl->Unmap(pipeline->impl.raygen_shader_table, 0, NULL);
	}

	// Miss shader table
	{
		UINT size = shaderIdSize;
		UINT shaderRecordSize = (size + (align - 1)) & ~(align - 1);
		D3D12_RESOURCE_DESC bufferDesc = {};
		bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferDesc.Width = shaderRecordSize;
		bufferDesc.Height = 1;
		bufferDesc.DepthOrArraySize = 1;
		bufferDesc.MipLevels = 1;
		bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufferDesc.SampleDesc.Count = 1;
		bufferDesc.SampleDesc.Quality = 0;
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		D3D12_HEAP_PROPERTIES uploadHeapProperties = {};
		uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
		uploadHeapProperties.CreationNodeMask = 1;
		uploadHeapProperties.VisibleNodeMask = 1;

		device->lpVtbl->CreateCommittedResource(device , &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
		                                &IID_ID3D12Resource, &pipeline->impl.miss_shader_table);

		D3D12_RANGE mstRange = {0};
		mstRange.Begin = 0;
		mstRange.End = 0;
		uint8_t *byteDest;
		pipeline->impl.miss_shader_table->lpVtbl->Map(pipeline->impl.miss_shader_table, 0, &mstRange, (void **)(&byteDest));
		memcpy(byteDest, missShaderId, size);
		pipeline->impl.miss_shader_table->lpVtbl->Unmap(pipeline->impl.miss_shader_table, 0, NULL);
	}

	// Hit group shader table
	{
		UINT size = shaderIdSize;
		UINT shaderRecordSize = (size + (align - 1)) & ~(align - 1);
		D3D12_RESOURCE_DESC bufferDesc = {};
		bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferDesc.Width = shaderRecordSize;
		bufferDesc.Height = 1;
		bufferDesc.DepthOrArraySize = 1;
		bufferDesc.MipLevels = 1;
		bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufferDesc.SampleDesc.Count = 1;
		bufferDesc.SampleDesc.Quality = 0;
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		D3D12_HEAP_PROPERTIES uploadHeapProperties = {};
		uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
		uploadHeapProperties.CreationNodeMask = 1;
		uploadHeapProperties.VisibleNodeMask = 1;

		device->lpVtbl->CreateCommittedResource(device, &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
		                                &IID_ID3D12Resource, &pipeline->impl.hitgroup_shader_table);

		D3D12_RANGE hstRange = {0};
		hstRange.Begin = 0;
		hstRange.End = 0;
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
	outputDescriptorHandle.ptr =
	    handle.ptr + (INT64)(descriptorHeapIndex) * (UINT64)(descriptorSize);
}

void iron_gpu_raytrace_pipeline_destroy(iron_gpu_raytrace_pipeline_t *pipeline) {
	pipeline->impl.dxr_state->lpVtbl->Release(pipeline->impl.dxr_state);
	pipeline->impl.raygen_shader_table->lpVtbl->Release(pipeline->impl.raygen_shader_table);
	pipeline->impl.miss_shader_table->lpVtbl->Release(pipeline->impl.miss_shader_table);
	pipeline->impl.hitgroup_shader_table->lpVtbl->Release(pipeline->impl.hitgroup_shader_table);
}

UINT create_srv_vb(iron_gpu_vertex_buffer_t *vb, UINT numElements, UINT elementSize) {
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.NumElements = numElements;
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

	D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor = {0};
	cpuDescriptor.ptr =
	    handle.ptr + (INT64)(descriptorsAllocated) * (UINT64)(descriptorSize);
	UINT descriptorIndex = descriptorsAllocated++;

	device->lpVtbl->CreateShaderResourceView(device, vb->impl.uploadBuffer, &srvDesc, cpuDescriptor);

	descriptorHeap->lpVtbl->GetGPUDescriptorHandleForHeapStart(descriptorHeap, &handle);

	vbgpuDescriptorHandle.ptr =
	    handle.ptr + (INT64)(descriptorIndex) * (UINT64)(descriptorSize);

	return descriptorIndex;
}

UINT create_srv_ib(iron_gpu_index_buffer_t *ib, UINT numElements, UINT elementSize) {
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.NumElements = numElements;
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

	D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor = {0};
	cpuDescriptor.ptr =
	    handle.ptr + (INT64)(descriptorsAllocated) * (UINT64)(descriptorSize);
	UINT descriptorIndex = descriptorsAllocated++;

	descriptorHeap->lpVtbl->GetGPUDescriptorHandleForHeapStart(descriptorHeap, &handle);

	device->lpVtbl->CreateShaderResourceView(device, ib->impl.upload_buffer, &srvDesc, cpuDescriptor);
	ibgpuDescriptorHandle.ptr =
	    handle.ptr + (INT64)(descriptorIndex) * (UINT64)(descriptorSize);

	return descriptorIndex;
}

void iron_gpu_raytrace_acceleration_structure_init(iron_gpu_raytrace_acceleration_structure_t *accel) {
	vb_count = 0;
	instances_count = 0;
}

void iron_gpu_raytrace_acceleration_structure_add(iron_gpu_raytrace_acceleration_structure_t *accel, iron_gpu_vertex_buffer_t *_vb, iron_gpu_index_buffer_t *_ib,
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

void _iron_gpu_raytrace_acceleration_structure_destroy_bottom(iron_gpu_raytrace_acceleration_structure_t *accel) {
	for (int i = 0; i < vb_count_last; ++i) {
		accel->impl.bottom_level_accel[i]->lpVtbl->Release(accel->impl.bottom_level_accel[i]);
	}
}

void _iron_gpu_raytrace_acceleration_structure_destroy_top(iron_gpu_raytrace_acceleration_structure_t *accel) {
	accel->impl.top_level_accel->lpVtbl->Release(accel->impl.top_level_accel);
}

void iron_gpu_raytrace_acceleration_structure_build(iron_gpu_raytrace_acceleration_structure_t *accel, iron_gpu_command_list_t *command_list,
	iron_gpu_vertex_buffer_t *_vb_full, iron_gpu_index_buffer_t *_ib_full) {

	bool build_bottom = false;
	for (int i = 0; i < 16; ++i) {
		if (vb_last[i] != vb[i]) {
			build_bottom = true;
		}
		vb_last[i] = vb[i];
	}

	if (vb_count_last > 0) {
		if (build_bottom) {
			_iron_gpu_raytrace_acceleration_structure_destroy_bottom(accel);
		}
		_iron_gpu_raytrace_acceleration_structure_destroy_top(accel);
	}

	vb_count_last = vb_count;

	if (vb_count == 0) {
		return;
	}

	descriptorsAllocated = 1; // 1 descriptor already allocated in iron_gpu_raytrace_pipeline_init

	#ifdef is_forge
	create_srv_ib(_ib_full, _ib_full->impl.count, 0);
	create_srv_vb(_vb_full, _vb_full->impl.myCount, vb[0]->impl.myStride);
	#else
	create_srv_ib(ib[0], ib[0]->impl.count, 0);
	create_srv_vb(vb[0], vb[0]->impl.myCount, vb[0]->impl.myStride);
	#endif

	// Reset the command list for the acceleration structure construction
	command_list->impl._commandList->lpVtbl->Reset(command_list->impl._commandList, command_list->impl._commandAllocator, NULL);

	// Get required sizes for an acceleration structure
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS topLevelInputs = {};
	topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	topLevelInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	topLevelInputs.NumDescs = 1;
	topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {0};
	dxrDevice->lpVtbl->GetRaytracingAccelerationStructurePrebuildInfo(dxrDevice , &topLevelInputs, &topLevelPrebuildInfo);

	UINT64 scratch_size = topLevelPrebuildInfo.ScratchDataSizeInBytes;

	// Bottom AS
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS bottomLevelInputs[16];
	D3D12_RAYTRACING_GEOMETRY_DESC geometryDescs[16];
	if (build_bottom) {
		for (int i = 0; i < vb_count; ++i) {
			D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
			geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
			geometryDesc.Triangles.IndexBuffer = ib[i]->impl.upload_buffer->lpVtbl->GetGPUVirtualAddress(ib[i]->impl.upload_buffer);
			geometryDesc.Triangles.IndexCount = ib[i]->impl.count;
			geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
			geometryDesc.Triangles.Transform3x4 = 0;
			geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R16G16B16A16_SNORM;
			geometryDesc.Triangles.VertexCount = vb[i]->impl.myCount;

			D3D12_RESOURCE_DESC desc;
			vb[i]->impl.uploadBuffer->lpVtbl->GetDesc(vb[i]->impl.uploadBuffer, &desc);

			geometryDesc.Triangles.VertexBuffer.StartAddress = vb[i]->impl.uploadBuffer->lpVtbl->GetGPUVirtualAddress(vb[i]->impl.uploadBuffer);
			geometryDesc.Triangles.VertexBuffer.StrideInBytes =
			    desc.Width / vb[i]->impl.myCount;
			geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
			geometryDescs[i] = geometryDesc;

			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {0};
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
			inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
			inputs.NumDescs = 1;
			inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
			inputs.pGeometryDescs = &geometryDescs[i];
			inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
			dxrDevice->lpVtbl->GetRaytracingAccelerationStructurePrebuildInfo(dxrDevice , &inputs, &bottomLevelPrebuildInfo);
			bottomLevelInputs[i] = inputs;

			UINT64 blSize = bottomLevelPrebuildInfo.ScratchDataSizeInBytes;
			if (scratch_size < blSize) {
				scratch_size = blSize;
			}

			// Allocate resources for acceleration structures
			// The resources that will contain acceleration structures must be created in the state D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
			// and must have resource flag D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS.
			{
				D3D12_RESOURCE_DESC bufferDesc = {};
				bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
				bufferDesc.Width = bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes;
				bufferDesc.Height = 1;
				bufferDesc.DepthOrArraySize = 1;
				bufferDesc.MipLevels = 1;
				bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
				bufferDesc.SampleDesc.Count = 1;
				bufferDesc.SampleDesc.Quality = 0;
				bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
				bufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
				D3D12_HEAP_PROPERTIES uploadHeapProperties = {};
				uploadHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
				uploadHeapProperties.CreationNodeMask = 1;
				uploadHeapProperties.VisibleNodeMask = 1;

				device->lpVtbl->CreateCommittedResource(dxrDevice, &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, NULL,
				                                &IID_ID3D12Resource, &accel->impl.bottom_level_accel[i]);
			}
		}
	}

	// Create scratch memory
	ID3D12Resource *scratchResource;
	{
		D3D12_RESOURCE_DESC bufferDesc = {};
		bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferDesc.Width = scratch_size;
		bufferDesc.Height = 1;
		bufferDesc.DepthOrArraySize = 1;
		bufferDesc.MipLevels = 1;
		bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufferDesc.SampleDesc.Count = 1;
		bufferDesc.SampleDesc.Quality = 0;
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		bufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		D3D12_HEAP_PROPERTIES uploadHeapProperties = {};
		uploadHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		uploadHeapProperties.CreationNodeMask = 1;
		uploadHeapProperties.VisibleNodeMask = 1;

		device->lpVtbl->CreateCommittedResource(dxrDevice, &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, NULL,
		                                &IID_ID3D12Resource, &scratchResource);
	}

	// Bottom AS
	if (build_bottom) {
		for (int i = 0; i < vb_count; ++i) {
			// Bottom Level Acceleration Structure desc
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {0};
			bottomLevelBuildDesc.Inputs = bottomLevelInputs[i];
			bottomLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->lpVtbl->GetGPUVirtualAddress(scratchResource);
			bottomLevelBuildDesc.DestAccelerationStructureData =
			    accel->impl.bottom_level_accel[i]->lpVtbl->GetGPUVirtualAddress(accel->impl.bottom_level_accel[i]);

			// Build acceleration structure
			dxrCommandList->lpVtbl->BuildRaytracingAccelerationStructure(dxrCommandList, &bottomLevelBuildDesc, 0, NULL);
		}
	}

	// Top AS
	{
		D3D12_RESOURCE_DESC bufferDesc = {};
		bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferDesc.Width = topLevelPrebuildInfo.ResultDataMaxSizeInBytes;
		bufferDesc.Height = 1;
		bufferDesc.DepthOrArraySize = 1;
		bufferDesc.MipLevels = 1;
		bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufferDesc.SampleDesc.Count = 1;
		bufferDesc.SampleDesc.Quality = 0;
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		bufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		D3D12_HEAP_PROPERTIES uploadHeapProperties = {};
		uploadHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		uploadHeapProperties.CreationNodeMask = 1;
		uploadHeapProperties.VisibleNodeMask = 1;

		device->lpVtbl->CreateCommittedResource(device, &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, NULL,
		                                &IID_ID3D12Resource, &accel->impl.top_level_accel);
	}

	// Create an instance desc for the bottom-level acceleration structure
	D3D12_RESOURCE_DESC bufferDesc = {};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Width = sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * instances_count;
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.SampleDesc.Quality = 0;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	D3D12_HEAP_PROPERTIES uploadHeapProperties = {};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	uploadHeapProperties.CreationNodeMask = 1;
	uploadHeapProperties.VisibleNodeMask = 1;

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
			ib_off += ib[j]->impl.count * 4;
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

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barrier.UAV.pResource = accel->impl.bottom_level_accel[0];
	command_list->impl._commandList->lpVtbl->ResourceBarrier(command_list->impl._commandList, 1, &barrier);
	dxrCommandList->lpVtbl->BuildRaytracingAccelerationStructure(dxrCommandList, &topLevelBuildDesc, 0, NULL);

	iron_gpu_command_list_end(command_list);
	iron_gpu_command_list_execute(command_list);
	iron_gpu_command_list_wait_for_execution_to_finish(command_list);
	iron_gpu_command_list_begin(command_list);

	scratchResource->lpVtbl->Release(scratchResource);
	instanceDescs->lpVtbl->Release(instanceDescs);
}

void iron_gpu_raytrace_acceleration_structure_destroy(iron_gpu_raytrace_acceleration_structure_t *accel) {
	// accel->impl.bottom_level_accel->Release();
	// accel->impl.top_level_accel->Release();
}

void iron_gpu_raytrace_set_textures(iron_gpu_texture_t *texpaint0, iron_gpu_texture_t *texpaint1, iron_gpu_texture_t *texpaint2, iron_gpu_texture_t *texenv, iron_gpu_texture_t *texsobol, iron_gpu_texture_t *texscramble, iron_gpu_texture_t *texrank) {
	D3D12_CPU_DESCRIPTOR_HANDLE handle;
	descriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(descriptorHeap, &handle);

	D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor = {0};
	cpuDescriptor.ptr = handle.ptr + 5 * (UINT64)(descriptorSize);

	D3D12_CPU_DESCRIPTOR_HANDLE sourceCpu;
	texpaint0->impl.srvDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(texpaint0->impl.srvDescriptorHeap, &sourceCpu);
	device->lpVtbl->CopyDescriptorsSimple(device, 1, cpuDescriptor, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_GPU_DESCRIPTOR_HANDLE ghandle;
	descriptorHeap->lpVtbl->GetGPUDescriptorHandleForHeapStart(descriptorHeap, &ghandle);
	tex0gpuDescriptorHandle.ptr = ghandle.ptr + 5 * (UINT64)(descriptorSize);

	descriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(descriptorHeap, &handle);
	cpuDescriptor.ptr = handle.ptr + 6 * (UINT64)(descriptorSize);
	texpaint1->impl.srvDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(texpaint1->impl.srvDescriptorHeap, &sourceCpu);
	device->lpVtbl->CopyDescriptorsSimple(device, 1, cpuDescriptor, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorHeap->lpVtbl->GetGPUDescriptorHandleForHeapStart(descriptorHeap, &ghandle);
	tex1gpuDescriptorHandle.ptr = ghandle.ptr + 6 * (UINT64)(descriptorSize);

	descriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(descriptorHeap, &handle);
	cpuDescriptor.ptr = handle.ptr + 7 * (UINT64)(descriptorSize);
	texpaint2->impl.srvDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(texpaint2->impl.srvDescriptorHeap, &sourceCpu);
	device->lpVtbl->CopyDescriptorsSimple(device, 1, cpuDescriptor, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorHeap->lpVtbl->GetGPUDescriptorHandleForHeapStart(descriptorHeap, &ghandle);
	tex2gpuDescriptorHandle.ptr = ghandle.ptr + 7 * (UINT64)(descriptorSize);

	descriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(descriptorHeap, &handle);
	cpuDescriptor.ptr = handle.ptr + 8 * (UINT64)(descriptorSize);
	texenv->impl.srvDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(texenv->impl.srvDescriptorHeap, &sourceCpu);
	device->lpVtbl->CopyDescriptorsSimple(device, 1, cpuDescriptor, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorHeap->lpVtbl->GetGPUDescriptorHandleForHeapStart(descriptorHeap, &ghandle);
	texenvgpuDescriptorHandle.ptr = ghandle.ptr + 8 * (UINT64)(descriptorSize);

	descriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(descriptorHeap, &handle);
	cpuDescriptor.ptr = handle.ptr + 9 * (UINT64)(descriptorSize);
	texsobol->impl.srvDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(texsobol->impl.srvDescriptorHeap, &sourceCpu);
	device->lpVtbl->CopyDescriptorsSimple(device, 1, cpuDescriptor, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorHeap->lpVtbl->GetGPUDescriptorHandleForHeapStart(descriptorHeap, &ghandle);
	texsobolgpuDescriptorHandle.ptr = ghandle.ptr + 9 * (UINT64)(descriptorSize);

	descriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(descriptorHeap, &handle);
	cpuDescriptor.ptr = handle.ptr + 10 * (UINT64)(descriptorSize);
	texscramble->impl.srvDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(texscramble->impl.srvDescriptorHeap , &sourceCpu);
	device->lpVtbl->CopyDescriptorsSimple(device, 1, cpuDescriptor, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorHeap->lpVtbl->GetGPUDescriptorHandleForHeapStart(descriptorHeap, &ghandle);
	texscramblegpuDescriptorHandle.ptr = ghandle.ptr + 10 * (UINT64)(descriptorSize);

	descriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(descriptorHeap, &handle);
	cpuDescriptor.ptr = handle.ptr + 11 * (UINT64)(descriptorSize);
	texrank->impl.srvDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(texrank->impl.srvDescriptorHeap , &sourceCpu);
	device->lpVtbl->CopyDescriptorsSimple(device, 1, cpuDescriptor, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorHeap->lpVtbl->GetGPUDescriptorHandleForHeapStart(descriptorHeap, &ghandle);
	texrankgpuDescriptorHandle.ptr = ghandle.ptr + 11 * (UINT64)(descriptorSize);
}

void iron_gpu_raytrace_set_acceleration_structure(iron_gpu_raytrace_acceleration_structure_t *_accel) {
	accel = _accel;
}

void iron_gpu_raytrace_set_pipeline(iron_gpu_raytrace_pipeline_t *_pipeline) {
	pipeline = _pipeline;
}

void iron_gpu_raytrace_set_target(iron_gpu_texture_t *_output) {
	if (_output != output) {
		_output->impl.renderTarget->lpVtbl->Release(_output->impl.renderTarget);
		_output->impl.renderTargetDescriptorHeap->lpVtbl->Release(_output->impl.renderTargetDescriptorHeap);
		_output->impl.srvDescriptorHeap->lpVtbl->Release(_output->impl.srvDescriptorHeap);

		D3D12_HEAP_PROPERTIES heapProperties = {};
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProperties.CreationNodeMask = 1;
		heapProperties.VisibleNodeMask = 1;
		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Width = _output->width;
		desc.Height = _output->height;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		D3D12_CLEAR_VALUE clearValue;
		clearValue.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		clearValue.Color[0] = 0.0f;
		clearValue.Color[1] = 0.0f;
		clearValue.Color[2] = 0.0f;
		clearValue.Color[3] = 1.0f;

		device->lpVtbl->CreateCommittedResource(device, &heapProperties, D3D12_HEAP_FLAG_NONE, &desc,
		                                D3D12_RESOURCE_STATE_COMMON, &clearValue, &IID_ID3D12Resource, &_output->impl.renderTarget);

		D3D12_RENDER_TARGET_VIEW_DESC view;
		view.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		view.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		view.Texture2D.MipSlice = 0;
		view.Texture2D.PlaneSlice = 0;
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = 1;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		device->lpVtbl->CreateDescriptorHeap(device, &heapDesc, &IID_ID3D12DescriptorHeap, &_output->impl.renderTargetDescriptorHeap);
		D3D12_CPU_DESCRIPTOR_HANDLE handle;
		_output->impl.renderTargetDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(_output->impl.renderTargetDescriptorHeap, &handle);
		device->lpVtbl->CreateRenderTargetView(device, _output->impl.renderTarget, &view,
		    handle);

		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
		descriptorHeapDesc.NumDescriptors = 1;
		descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		descriptorHeapDesc.NodeMask = 0;
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		device->lpVtbl->CreateDescriptorHeap(device, &descriptorHeapDesc, &IID_ID3D12DescriptorHeap, &_output->impl.srvDescriptorHeap);

		D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
		shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		shaderResourceViewDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		_output->impl.srvDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(_output->impl.srvDescriptorHeap, &handle);
		device->lpVtbl->CreateShaderResourceView(device, _output->impl.renderTarget, &shaderResourceViewDesc,
		                                         handle);

		D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
		UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		device->lpVtbl->CreateUnorderedAccessView(device, _output->impl.renderTarget, NULL, &UAVDesc, outputCpuDescriptor);
	}
	output = _output;
}

void iron_gpu_raytrace_dispatch_rays(iron_gpu_command_list_t *command_list) {
	command_list->impl._commandList->lpVtbl->SetComputeRootSignature(command_list->impl._commandList, dxrRootSignature);

	// Bind the heaps, acceleration structure and dispatch rays
	command_list->impl._commandList->lpVtbl->SetDescriptorHeaps(command_list->impl._commandList, 1, &descriptorHeap);
	command_list->impl._commandList->lpVtbl->SetComputeRootDescriptorTable(command_list->impl._commandList, 0, outputDescriptorHandle);
	command_list->impl._commandList->lpVtbl->SetComputeRootShaderResourceView(command_list->impl._commandList, 1, accel->impl.top_level_accel->lpVtbl->GetGPUVirtualAddress(accel->impl.top_level_accel));
	command_list->impl._commandList->lpVtbl->SetComputeRootDescriptorTable(command_list->impl._commandList, 2, ibgpuDescriptorHandle);
	command_list->impl._commandList->lpVtbl->SetComputeRootDescriptorTable(command_list->impl._commandList, 3, vbgpuDescriptorHandle);
	command_list->impl._commandList->lpVtbl->SetComputeRootConstantBufferView(command_list->impl._commandList, 4, pipeline->_constant_buffer->impl.constant_buffer->lpVtbl->GetGPUVirtualAddress(pipeline->_constant_buffer->impl.constant_buffer));
	command_list->impl._commandList->lpVtbl->SetComputeRootDescriptorTable(command_list->impl._commandList, 5, tex0gpuDescriptorHandle);
	command_list->impl._commandList->lpVtbl->SetComputeRootDescriptorTable(command_list->impl._commandList, 6, tex1gpuDescriptorHandle);
	command_list->impl._commandList->lpVtbl->SetComputeRootDescriptorTable(command_list->impl._commandList, 7, tex2gpuDescriptorHandle);
	command_list->impl._commandList->lpVtbl->SetComputeRootDescriptorTable(command_list->impl._commandList, 8, texenvgpuDescriptorHandle);
	command_list->impl._commandList->lpVtbl->SetComputeRootDescriptorTable(command_list->impl._commandList, 9, texsobolgpuDescriptorHandle);
	command_list->impl._commandList->lpVtbl->SetComputeRootDescriptorTable(command_list->impl._commandList, 10, texscramblegpuDescriptorHandle);
	command_list->impl._commandList->lpVtbl->SetComputeRootDescriptorTable(command_list->impl._commandList, 11, texrankgpuDescriptorHandle);

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
