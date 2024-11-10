#include "commandlist.h"
#include "indexbuffer.h"
#include "pipeline.h"
#include "vertexbuffer.h"

#include <kinc/graphics5/graphics.h>
#include <kinc/graphics5/pipeline.h>
#include <kinc/math/core.h>
#include <kinc/window.h>
#ifdef KINC_WINDOWS
#include <dxgi1_4.h>
#undef CreateWindow
#endif
#include <kinc/system.h>
#ifdef KINC_WINDOWS
#include <kinc/backend/Windows.h>
#endif
#include <kinc/backend/SystemMicrosoft.h>

/*IDXGIFactory4* dxgiFactory;
ID3D12Device* device;
ID3D12GraphicsCommandList* commandList;
ID3D12CommandQueue* commandQueue;
ID3D12CommandAllocator* commandAllocators[frameCount];
unsigned currentFrame = 0;
ID3D12Fence* fence;
UINT64 window->current_fence_value[frameCount];
HANDLE fenceEvent;
ID3D12Resource* renderTargets[frameCount];
ID3D12DescriptorHeap* rtvHeap;
unsigned rtvDescriptorSize;
ID3D12CommandAllocator* bundleAllocator;
ID3D12RootSignature* rootSignature;
D3D12_VIEWPORT screenViewport;
D3D12_RECT scissorRect;
ID3D12DescriptorHeap* cbvHeap;*/
// ID3D12DeviceContext* context;
// ID3D12RenderTargetView* renderTargetView;
// ID3D12DepthStencilView* depthStencilView;

// ID3D12GraphicsCommandList* commandList;

extern "C" {
ID3D12CommandQueue *commandQueue;
#ifdef KINC_DIRECT3D_HAS_NO_SWAPCHAIN
ID3D12Resource *swapChainRenderTargets[QUEUE_SLOT_COUNT];
#else
// IDXGISwapChain *swapChain;
#endif
}

// int window->width;
// int window->height;
// int window->new_width;
// int window->new_height;

#ifndef KINC_WINDOWS
#define DXGI_SWAP_CHAIN_DESC DXGI_SWAP_CHAIN_DESC1
#define IDXGISwapChain IDXGISwapChain1
#endif

struct RenderEnvironment {
	ID3D12Device *device;
	ID3D12CommandQueue *queue;
#ifdef KINC_DIRECT3D_HAS_NO_SWAPCHAIN
	ID3D12Resource *renderTargets[QUEUE_SLOT_COUNT];
#else
	IDXGISwapChain *swapChain;
#endif
};

#ifndef KINC_WINDOWS
#ifdef KINC_DIRECT3D_HAS_NO_SWAPCHAIN
extern "C" void createSwapChain(struct RenderEnvironment *env, int bufferCount);
#else
extern "C" void createSwapChain(struct RenderEnvironment *env, const DXGI_SWAP_CHAIN_DESC1 *desc);
#endif
#endif

extern bool bilinearFiltering;

// ID3D12Resource* renderTarget;
// ID3D12DescriptorHeap* renderTargetDescriptorHeap;

// static UINT64 window->current_fence_value;
// static UINT64 window->current_fence_value[QUEUE_SLOT_COUNT];
// static HANDLE window->frame_fence_events[QUEUE_SLOT_COUNT];
// static ID3D12Fence *window->frame_fences[QUEUE_SLOT_COUNT];
static ID3D12Fence *uploadFence;
static ID3D12GraphicsCommandList *initCommandList;
static ID3D12CommandAllocator *initCommandAllocator;

extern "C" struct RenderEnvironment createDeviceAndSwapChainHelper(D3D_FEATURE_LEVEL minimumFeatureLevel, const struct DXGI_SWAP_CHAIN_DESC *swapChainDesc) {
	struct RenderEnvironment result = {0};
#ifdef KINC_WINDOWS
	kinc_microsoft_affirm(D3D12CreateDevice(NULL, minimumFeatureLevel, IID_PPV_ARGS(&result.device)));

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	kinc_microsoft_affirm(result.device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&result.queue)));

	IDXGIFactory4 *dxgiFactory;
	kinc_microsoft_affirm(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));

	DXGI_SWAP_CHAIN_DESC swapChainDescCopy = *swapChainDesc;
	kinc_microsoft_affirm(dxgiFactory->CreateSwapChain((IUnknown *)result.queue, &swapChainDescCopy, &result.swapChain));
#else
#ifdef KINC_DIRECT3D_HAS_NO_SWAPCHAIN
	createSwapChain(&result, QUEUE_SLOT_COUNT);
#else
	createSwapChain(&result, swapChainDesc);
#endif
#endif
	return result;
}

static void waitForFence(ID3D12Fence *fence, UINT64 completionValue, HANDLE waitEvent) {
	if (fence->GetCompletedValue() < completionValue) {
		kinc_microsoft_affirm(fence->SetEventOnCompletion(completionValue, waitEvent));
		WaitForSingleObject(waitEvent, INFINITE);
	}
}

extern "C" void setupSwapChain(struct dx_window *window) {
	/*D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	device->CreateDescriptorHeap(&heapDesc, IID_GRAPHICS_PPV_ARGS(&renderTargetDescriptorHeap));*/

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
		device->CreateFence(window->current_fence_value, D3D12_FENCE_FLAG_NONE, IID_GRAPHICS_PPV_ARGS(&window->frame_fences[i]));
	}

	//**swapChain->GetBuffer(window->current_backbuffer, IID_GRAPHICS_PPV_ARGS(&renderTarget));
	//**createRenderTargetView();
}

#ifdef KINC_CONSOLE
extern "C" void createDeviceAndSwapChain(struct dx_window *window);
#else
static void createDeviceAndSwapChain(struct dx_window *window) {
#ifdef _DEBUG
	ID3D12Debug *debugController = NULL;
	D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
	debugController->EnableDebugLayer();
#endif

	struct DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	swapChainDesc.BufferCount = QUEUE_SLOT_COUNT;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferDesc.Width = window->width;
	swapChainDesc.BufferDesc.Height = window->height;
	swapChainDesc.OutputWindow = kinc_windows_window_handle(window->window_index);
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.Windowed = true;

	struct RenderEnvironment renderEnv = createDeviceAndSwapChainHelper(D3D_FEATURE_LEVEL_11_0, &swapChainDesc);

	device = renderEnv.device;
	commandQueue = renderEnv.queue;

	// swapChain = renderEnv.swapChain;

	setupSwapChain(NULL);
}
#endif

static void createRootSignature() {
	ID3DBlob *rootBlob;
	ID3DBlob *errorBlob;

	D3D12_ROOT_PARAMETER parameters[4] = {};

	D3D12_DESCRIPTOR_RANGE range;
	range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range.NumDescriptors = (UINT)KINC_INTERNAL_G5_TEXTURE_COUNT;
	range.BaseShaderRegister = 0;
	range.RegisterSpace = 0;
	range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	parameters[0].DescriptorTable.NumDescriptorRanges = 1;
	parameters[0].DescriptorTable.pDescriptorRanges = &range;

	D3D12_DESCRIPTOR_RANGE range2;
	range2.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
	range2.NumDescriptors = (UINT)KINC_INTERNAL_G5_TEXTURE_COUNT;
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

	D3D12_STATIC_SAMPLER_DESC samplers[KINC_INTERNAL_G5_TEXTURE_COUNT * 2];
	for (int i = 0; i < KINC_INTERNAL_G5_TEXTURE_COUNT; ++i) {
		samplers[i].ShaderRegister = i;
		samplers[i].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		samplers[i].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplers[i].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplers[i].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplers[i].MipLODBias = 0;
		samplers[i].MaxAnisotropy = 16;
		samplers[i].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		samplers[i].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		samplers[i].MinLOD = 0.0f;
		samplers[i].MaxLOD = D3D12_FLOAT32_MAX;
		samplers[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		samplers[i].RegisterSpace = 0;
	}
	for (int i = KINC_INTERNAL_G5_TEXTURE_COUNT; i < KINC_INTERNAL_G5_TEXTURE_COUNT * 2; ++i) {
		samplers[i].ShaderRegister = i;
		samplers[i].Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		samplers[i].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplers[i].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplers[i].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplers[i].MipLODBias = 0;
		samplers[i].MaxAnisotropy = 16;
		samplers[i].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
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

	kinc_microsoft_affirm(D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &rootBlob, &errorBlob));
	device->CreateRootSignature(0, rootBlob->GetBufferPointer(), rootBlob->GetBufferSize(), IID_GRAPHICS_PPV_ARGS(&globalRootSignature));
}

static void createComputeRootSignature() {
	ID3DBlob *rootBlob;
	ID3DBlob *errorBlob;

	D3D12_ROOT_PARAMETER parameters[4] = {};

	D3D12_DESCRIPTOR_RANGE range;
	range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range.NumDescriptors = (UINT)KINC_INTERNAL_G5_TEXTURE_COUNT;
	range.BaseShaderRegister = 0;
	range.RegisterSpace = 0;
	range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	parameters[0].DescriptorTable.NumDescriptorRanges = 1;
	parameters[0].DescriptorTable.pDescriptorRanges = &range;

	D3D12_DESCRIPTOR_RANGE uav_range;
	uav_range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	uav_range.NumDescriptors = (UINT)KINC_INTERNAL_G5_TEXTURE_COUNT;
	uav_range.BaseShaderRegister = 0;
	uav_range.RegisterSpace = 0;
	uav_range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	parameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	parameters[1].DescriptorTable.NumDescriptorRanges = 1;
	parameters[1].DescriptorTable.pDescriptorRanges = &uav_range;

	D3D12_DESCRIPTOR_RANGE range2;
	range2.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
	range2.NumDescriptors = (UINT)KINC_INTERNAL_G5_TEXTURE_COUNT;
	range2.BaseShaderRegister = 0;
	range2.RegisterSpace = 0;
	range2.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	parameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	parameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	parameters[2].DescriptorTable.NumDescriptorRanges = 1;
	parameters[2].DescriptorTable.pDescriptorRanges = &range2;

	parameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	parameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	parameters[3].Descriptor.ShaderRegister = 0;
	parameters[3].Descriptor.RegisterSpace = 0;

	D3D12_STATIC_SAMPLER_DESC samplers[KINC_INTERNAL_G5_TEXTURE_COUNT * 2];
	for (int i = 0; i < KINC_INTERNAL_G5_TEXTURE_COUNT; ++i) {
		samplers[i].ShaderRegister = i;
		samplers[i].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		samplers[i].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplers[i].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplers[i].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplers[i].MipLODBias = 0;
		samplers[i].MaxAnisotropy = 16;
		samplers[i].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		samplers[i].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		samplers[i].MinLOD = 0.0f;
		samplers[i].MaxLOD = D3D12_FLOAT32_MAX;
		samplers[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		samplers[i].RegisterSpace = 0;
	}
	for (int i = KINC_INTERNAL_G5_TEXTURE_COUNT; i < KINC_INTERNAL_G5_TEXTURE_COUNT * 2; ++i) {
		samplers[i].ShaderRegister = i;
		samplers[i].Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		samplers[i].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplers[i].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplers[i].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplers[i].MipLODBias = 0;
		samplers[i].MaxAnisotropy = 16;
		samplers[i].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
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

	kinc_microsoft_affirm(D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &rootBlob, &errorBlob));
	device->CreateRootSignature(0, rootBlob->GetBufferPointer(), rootBlob->GetBufferSize(), IID_GRAPHICS_PPV_ARGS(&globalComputeRootSignature));

	// createSamplersAndHeaps();
}

static void initialize(struct dx_window *window) {
	createDeviceAndSwapChain(window);
	createRootSignature();
	createComputeRootSignature();

	device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_GRAPHICS_PPV_ARGS(&uploadFence));

	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_GRAPHICS_PPV_ARGS(&initCommandAllocator));
	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, initCommandAllocator, NULL, IID_GRAPHICS_PPV_ARGS(&initCommandList));

	initCommandList->Close();

	ID3D12CommandList *commandLists[] = {(ID3D12CommandList *)initCommandList};
	commandQueue->ExecuteCommandLists(1, commandLists);
	commandQueue->Signal(uploadFence, 1);

	HANDLE waitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	waitForFence(uploadFence, 1, waitEvent);

	initCommandAllocator->Reset();
	initCommandList->Release();      // check me
	initCommandAllocator->Release(); // check me

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

#ifdef KINC_WINDOWS
static void initWindow(struct dx_window *window, int windowIndex) {
	HWND hwnd = kinc_windows_window_handle(windowIndex);

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	swapChainDesc.BufferCount = QUEUE_SLOT_COUNT;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferDesc.Width = kinc_window_width(windowIndex);
	swapChainDesc.BufferDesc.Height = kinc_window_height(windowIndex);
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.Windowed = true;

	IDXGIFactory4 *dxgiFactory = NULL;
	kinc_microsoft_affirm(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));

	kinc_microsoft_affirm(dxgiFactory->CreateSwapChain((IUnknown *)commandQueue, &swapChainDesc, &window->swapChain));

	setupSwapChain(window);
}
#endif

void kinc_g5_internal_destroy_window(int window) {}

void kinc_g5_internal_destroy() {
#ifdef KINC_WINDOWS
	if (device) {
		device->Release();
		device = NULL;
	}
#endif
}

void kinc_g5_internal_init() {
#ifdef KINC_WINDOWS
#ifdef _DEBUG
	ID3D12Debug *debugController = NULL;
	if (D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)) == S_OK) {
		debugController->EnableDebugLayer();
	}
#endif
	kinc_microsoft_affirm(D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)));

	createRootSignature();
	createComputeRootSignature();

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	kinc_microsoft_affirm(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));

	device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&uploadFence));

	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&initCommandAllocator));
	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, initCommandAllocator, NULL, IID_PPV_ARGS(&initCommandList));

	initCommandList->Close();

	ID3D12CommandList *commandLists[] = {(ID3D12CommandList *)initCommandList};
	commandQueue->ExecuteCommandLists(1, commandLists);
	commandQueue->Signal(uploadFence, 1);

	HANDLE waitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	waitForFence(uploadFence, 1, waitEvent);

	initCommandAllocator->Reset();
	initCommandList->Release();      // check me
	initCommandAllocator->Release(); // check me

	CloseHandle(waitEvent);
#endif
}

void kinc_g5_internal_init_window(int windowIndex, int depthBufferBits, int stencilBufferBits, bool verticalSync) {
	struct dx_window *window = &dx_ctx.windows[windowIndex];
	window->window_index = windowIndex;
	window->vsync = verticalSync;
	window->width = window->new_width = kinc_window_width(windowIndex);
	window->height = window->new_height = kinc_window_height(windowIndex);
#ifdef KINC_WINDOWS
	initWindow(window, windowIndex);
#else
	HWND hwnd = NULL;
	window->vsync = verticalSync;
	window->new_width = window->width = kinc_width();
	window->new_height = window->height = kinc_height();
	initialize(window);
#endif
}

int kinc_g5_max_bound_textures(void) {
	return D3D12_COMMONSHADER_SAMPLER_SLOT_COUNT;
}

#ifndef KINC_WINDOWS
extern "C" void kinc_internal_wait_for_frame();
#endif

static bool began = false;

void kinc_g5_begin(kinc_g5_render_target_t *renderTarget, int windowId) {
	if (began)
		return;
	began = true;

#ifndef KINC_WINDOWS
	kinc_internal_wait_for_frame();
#endif

	struct dx_window *window = &dx_ctx.windows[windowId];
	dx_ctx.current_window = windowId;

	window->current_backbuffer = (window->current_backbuffer + 1) % QUEUE_SLOT_COUNT;

	if (window->new_width != window->width || window->new_height != window->height) {
#ifndef KINC_DIRECT3D_HAS_NO_SWAPCHAIN
		kinc_microsoft_affirm(window->swapChain->ResizeBuffers(QUEUE_SLOT_COUNT, window->new_width, window->new_height, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
#endif
		setupSwapChain(window);
		window->width = window->new_width;
		window->height = window->new_height;
		window->current_backbuffer = 0;
	}

	const UINT64 fenceValue = window->current_fence_value;
	commandQueue->Signal(window->frame_fences[window->current_backbuffer], fenceValue);
	window->fence_values[window->current_backbuffer] = fenceValue;
	++window->current_fence_value;

	waitForFence(window->frame_fences[window->current_backbuffer], window->fence_values[window->current_backbuffer],
	             window->frame_fence_events[window->current_backbuffer]);

	// static const float clearColor[] = {0.042f, 0.042f, 0.042f, 1};

	// commandList->ClearRenderTargetView(GetCPUDescriptorHandle(renderTargetDescriptorHeap), clearColor, 0, nullptr);

	// commandList->ClearDepthStencilView(GetCPUDescriptorHandle(depthStencilDescriptorHeap), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	static int frameNumber = 0;
	frameNumber++;
}

void kinc_g5_end(int window) {
	began = false;
}

bool kinc_g5_vsynced() {
	return true;
}

bool kinc_window_vsynced(int window) {
	return true;
}

extern "C" void kinc_g4_on_g5_internal_resize(int, int, int);

extern "C" void kinc_internal_resize(int windowId, int width, int height) {
	if (width == 0 || height == 0)
		return;
	struct dx_window *window = &dx_ctx.windows[windowId];
	window->new_width = width;
	window->new_height = height;
	kinc_g4_on_g5_internal_resize(windowId, width, height);
}

extern "C" void kinc_internal_change_framebuffer(int window, kinc_framebuffer_options_t *frame) {}

#ifndef KINC_DIRECT3D_HAS_NO_SWAPCHAIN
bool kinc_g5_swap_buffers() {
	for (int i = 0; i < MAXIMUM_WINDOWS; i++) {
		struct dx_window *window = &dx_ctx.windows[i];
		if (window->swapChain) {
			kinc_microsoft_affirm(window->swapChain->Present(window->vsync, 0));
		}
	}
	return true;
}
#endif

void kinc_g5_flush() {}

bool kinc_g5_render_targets_inverted_y() {
	return false;
}

bool kinc_g5_supports_raytracing() {
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 options;
	if (device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options, sizeof(options)) == S_OK) {
		return options.RaytracingTier >= D3D12_RAYTRACING_TIER_1_0;
	}
	return false;
}

bool kinc_g5_supports_instanced_rendering() {
	return true;
}

bool kinc_g5_supports_compute_shaders() {
	return true;
}

bool kinc_g5_supports_blend_constants() {
	return true;
}

bool kinc_g5_supports_non_pow2_textures() {
	return true;
}