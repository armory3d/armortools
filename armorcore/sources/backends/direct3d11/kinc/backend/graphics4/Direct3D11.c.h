#include "Direct3D11.h"

#include <kinc/log.h>

#include <kinc/math/core.h>
#include <kinc/graphics4/pipeline.h>
#include <kinc/graphics4/shader.h>

#include <kinc/graphics4/graphics.h>
#include <kinc/graphics4/indexbuffer.h>
#include <kinc/graphics4/rendertarget.h>
#include <kinc/graphics4/vertexbuffer.h>
#include <kinc/graphics4/texture.h>

#undef CreateWindow

#include <kinc/system.h>

#include <kinc/display.h>
#include <kinc/window.h>

#ifdef KINC_WINDOWS
#include <kinc/backend/Windows.h>
#else
int antialiasingSamples(void) {
	return 1;
}
#endif

// MinGW workaround for missing defines
#ifndef D3D11_MIN_DEPTH
#define D3D11_MIN_DEPTH (0.0f)
#endif

#ifndef D3D11_MAX_DEPTH
#define D3D11_MAX_DEPTH (1.0f)
#endif

extern kinc_g4_pipeline_t *currentPipeline;
extern float currentBlendFactor[4];
extern bool needPipelineRebind;
void kinc_internal_set_constants(void);
void kinc_internal_pipeline_rebind(void);

bool kinc_internal_scissoring = false;

// static unsigned hz;
// static bool vsync;

static D3D_FEATURE_LEVEL featureLevel;
// static IDXGISwapChain *swapChain = NULL;
static D3D11_SAMPLER_DESC lastSamplers[16];

struct Sampler {
	D3D11_SAMPLER_DESC desc;
	ID3D11SamplerState *state;
};

#define MAX_SAMPLERS 256
static struct Sampler samplers[MAX_SAMPLERS];
static uint32_t samplers_size = 0;

static ID3D11SamplerState *getSamplerState(D3D11_SAMPLER_DESC *desc) {
	for (unsigned i = 0; i < samplers_size; ++i) {
		if (memcmp(desc, &samplers[i].desc, sizeof(D3D11_SAMPLER_DESC)) == 0) {
			return samplers[i].state;
		}
	}
	struct Sampler s;
	s.desc = *desc;
	dx_ctx.device->lpVtbl->CreateSamplerState(dx_ctx.device, &s.desc, &s.state);
	assert(samplers_size < MAX_SAMPLERS);
	samplers[samplers_size++] = s;
	return s.state;
}

static void initSamplers(void) {
	D3D11_SAMPLER_DESC samplerDesc;
	memset(&samplerDesc, 0, sizeof(samplerDesc));
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	ID3D11SamplerState *state;
	dx_ctx.device->lpVtbl->CreateSamplerState(dx_ctx.device, &samplerDesc, &state);

	ID3D11SamplerState *states[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];
	for (int i = 0; i < D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT; ++i) {
		states[i] = state;
	}

	dx_ctx.context->lpVtbl->VSSetSamplers(dx_ctx.context, 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, states);
	dx_ctx.context->lpVtbl->PSSetSamplers(dx_ctx.context, 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, states);
}

static ID3D11RenderTargetView *currentRenderTargetViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
static int renderTargetCount = 1;
static ID3D11DepthStencilView *currentDepthStencilView;

void kinc_g4_internal_destroy_window(int window) {}

void kinc_g4_internal_destroy(void) {}

static void createBackbuffer(struct dx_window *window, int antialiasingSamples) {
	if (window->depthStencil) {
		window->depthStencil->lpVtbl->Release(window->depthStencil);
		window->depthStencilView->lpVtbl->Release(window->depthStencilView);
		window->renderTargetView->lpVtbl->Release(window->renderTargetView);
		window->backBuffer->lpVtbl->Release(window->backBuffer);
		window->depthStencil = NULL;
		window->depthStencilView = NULL;
		window->renderTargetView = NULL;
		window->backBuffer = NULL;
	}

	window->swapChain->lpVtbl->ResizeBuffers(window->swapChain, 1, window->width, window->height, DXGI_FORMAT_B8G8R8A8_UNORM, 0);

	kinc_microsoft_affirm(window->swapChain->lpVtbl->GetBuffer(window->swapChain, 0, &IID_ID3D11Texture2D, (void **)&window->backBuffer));

	kinc_microsoft_affirm(dx_ctx.device->lpVtbl->CreateRenderTargetView(dx_ctx.device, (ID3D11Resource *)window->backBuffer, NULL, &window->renderTargetView));

	D3D11_TEXTURE2D_DESC backBufferDesc;
	window->backBuffer->lpVtbl->GetDesc(window->backBuffer, &backBufferDesc);
	window->width = window->new_width = backBufferDesc.Width;
	window->height = window->new_height = backBufferDesc.Height;

	// TODO (DK) map depth/stencilBufferBits arguments
	D3D11_TEXTURE2D_DESC depth_stencil_desc;
	depth_stencil_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depth_stencil_desc.Width = backBufferDesc.Width;
	depth_stencil_desc.Height = backBufferDesc.Height;
	depth_stencil_desc.ArraySize = 1;
	depth_stencil_desc.MipLevels = 1;
	depth_stencil_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depth_stencil_desc.Usage = D3D11_USAGE_DEFAULT;
	depth_stencil_desc.CPUAccessFlags = 0;
	depth_stencil_desc.SampleDesc.Count = antialiasingSamples > 1 ? antialiasingSamples : 1,
	depth_stencil_desc.SampleDesc.Quality = antialiasingSamples > 1 ? D3D11_STANDARD_MULTISAMPLE_PATTERN : 0;
	depth_stencil_desc.MiscFlags = 0;
	kinc_microsoft_affirm(dx_ctx.device->lpVtbl->CreateTexture2D(dx_ctx.device, &depth_stencil_desc, NULL, &window->depthStencil));

	D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc;
	depth_stencil_view_desc.ViewDimension = antialiasingSamples > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
	depth_stencil_view_desc.Format = DXGI_FORMAT_UNKNOWN;
	depth_stencil_view_desc.Flags = 0;
	if (antialiasingSamples <= 1) {
		depth_stencil_view_desc.Texture2D.MipSlice = 0;
	}
	kinc_microsoft_affirm(dx_ctx.device->lpVtbl->CreateDepthStencilView(dx_ctx.device, (ID3D11Resource *)window->depthStencil, &depth_stencil_view_desc,
	                                                                    &window->depthStencilView));
}

#ifdef KINC_WINDOWS
static bool isWindowsVersionOrGreater(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor) {
	OSVERSIONINFOEXW osvi = {sizeof(osvi), 0, 0, 0, 0, {0}, 0, 0};
	DWORDLONG const dwlConditionMask =
	    VerSetConditionMask(VerSetConditionMask(VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL), VER_MINORVERSION, VER_GREATER_EQUAL),
	                        VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);

	osvi.dwMajorVersion = wMajorVersion;
	osvi.dwMinorVersion = wMinorVersion;
	osvi.wServicePackMajor = wServicePackMajor;

	return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlConditionMask) != FALSE;
}

#ifndef _WIN32_WINNT_WIN8
#define _WIN32_WINNT_WIN8 0x0602
#endif

#ifndef _WIN32_WINNT_WIN10
#define _WIN32_WINNT_WIN10 0x0A00
#endif

static bool isWindows8OrGreater(void) {
	return isWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WIN8), LOBYTE(_WIN32_WINNT_WIN8), 0);
}

static bool isWindows10OrGreater(void) {
	return isWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WIN10), LOBYTE(_WIN32_WINNT_WIN10), 0);
}
#endif

void kinc_g4_internal_init(void) {
	D3D_FEATURE_LEVEL featureLevels[] = {
	    D3D_FEATURE_LEVEL_11_0,
	    D3D_FEATURE_LEVEL_10_1,
	    D3D_FEATURE_LEVEL_10_0,
	};
	UINT flags = 0;

#ifdef _DEBUG
	flags = D3D11_CREATE_DEVICE_DEBUG;
#endif

	IDXGIAdapter *adapter = NULL;
	HRESULT result = D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION,
	                                   &dx_ctx.device, &featureLevel, &dx_ctx.context);

#ifdef _DEBUG
	if (result == E_FAIL || result == DXGI_ERROR_SDK_COMPONENT_MISSING) {
		kinc_log(KINC_LOG_LEVEL_WARNING, "%s", "Failed to create device with D3D11_CREATE_DEVICE_DEBUG, trying without");
		flags &= ~D3D11_CREATE_DEVICE_DEBUG;
		result = D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &dx_ctx.device,
		                           &featureLevel, &dx_ctx.context);
	}
#endif

	if (result != S_OK) {
		kinc_log(KINC_LOG_LEVEL_WARNING, "%s", "Falling back to the WARP driver, things will be slow.");
		kinc_microsoft_affirm(D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_WARP, NULL, flags, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION,
		                                        &dx_ctx.device, &featureLevel, &dx_ctx.context));
	}
	kinc_microsoft_affirm(dx_ctx.device->lpVtbl->QueryInterface(dx_ctx.device, &IID_IDXGIDevice, (void **)&dx_ctx.dxgiDevice));
	kinc_microsoft_affirm(dx_ctx.dxgiDevice->lpVtbl->GetAdapter(dx_ctx.dxgiDevice, &dx_ctx.dxgiAdapter));
	kinc_microsoft_affirm(dx_ctx.dxgiAdapter->lpVtbl->GetParent(dx_ctx.dxgiAdapter, &IID_IDXGIFactory, (void **)&dx_ctx.dxgiFactory));
}

void kinc_g4_internal_init_window(int windowId, int depthBufferBits, int stencilBufferBits, bool vSync) {
	for (int i = 0; i < 1024 * 4; ++i)
		vertexConstants[i] = 0;
	for (int i = 0; i < 1024 * 4; ++i)
		fragmentConstants[i] = 0;

	struct dx_window *window = &dx_ctx.windows[windowId];
#ifdef KINC_WINDOWS
	window->hwnd = kinc_windows_window_handle(windowId);
#endif
	window->depth_bits = depthBufferBits;
	window->stencil_bits = stencilBufferBits;
	window->vsync = vSync;

	const int _DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL = 3;
	const int _DXGI_SWAP_EFFECT_FLIP_DISCARD = 4;

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {0};
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1; // 60Hz
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.Width = kinc_window_width(windowId); // use automatic sizing
	swapChainDesc.BufferDesc.Height = kinc_window_height(windowId);
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // this is the most common swapchain format
	// swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc.Count = kinc_g4_antialiasing_samples() > 1 ? kinc_g4_antialiasing_samples() : 1;
	swapChainDesc.SampleDesc.Quality = kinc_g4_antialiasing_samples() > 1 ? D3D11_STANDARD_MULTISAMPLE_PATTERN : 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2; // use two buffers to enable flip effect
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED; // DXGI_SCALING_NONE;
#ifdef KINC_WINDOWS
	if (isWindows10OrGreater()) {
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		//(DXGI_SWAP_EFFECT) _DXGI_SWAP_EFFECT_FLIP_DISCARD;
	}
	else if (isWindows8OrGreater()) {
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		//(DXGI_SWAP_EFFECT) _DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	}
	else
#endif
	{
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	}
	swapChainDesc.Flags = 0;
	swapChainDesc.OutputWindow = window->hwnd;
	swapChainDesc.Windowed = true;

	kinc_microsoft_affirm(dx_ctx.dxgiFactory->lpVtbl->CreateSwapChain(dx_ctx.dxgiFactory, (IUnknown *)dx_ctx.dxgiDevice, &swapChainDesc, &window->swapChain));

	createBackbuffer(window, kinc_g4_antialiasing_samples());
	currentRenderTargetViews[0] = window->renderTargetView;
	currentDepthStencilView = window->depthStencilView;
	dx_ctx.context->lpVtbl->OMSetRenderTargets(dx_ctx.context, 1, &window->renderTargetView, window->depthStencilView);

	D3D11_VIEWPORT viewPort;
	viewPort.TopLeftX = 0.0f;
	viewPort.TopLeftY = 0.0f;
	viewPort.Width = (float)window->width;
	viewPort.Height = (float)window->height;
	viewPort.MinDepth = D3D11_MIN_DEPTH;
	viewPort.MaxDepth = D3D11_MAX_DEPTH;
	dx_ctx.context->lpVtbl->RSSetViewports(dx_ctx.context, 1, &viewPort);

	D3D11_SAMPLER_DESC samplerDesc;
	memset(&samplerDesc, 0, sizeof(samplerDesc));
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	for (int i = 0; i < 16; ++i) {
		lastSamplers[i] = samplerDesc;
	}

	initSamplers();

	D3D11_BLEND_DESC blendDesc;
	memset(&blendDesc, 0, sizeof(blendDesc));

	D3D11_RENDER_TARGET_BLEND_DESC rtbd;
	memset(&rtbd, 0, sizeof(rtbd));

	rtbd.BlendEnable = true;
	rtbd.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	rtbd.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	rtbd.BlendOp = D3D11_BLEND_OP_ADD;
	rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
	rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rtbd.RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;

	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.RenderTarget[0] = rtbd;

	ID3D11BlendState *blending;
	dx_ctx.device->lpVtbl->CreateBlendState(dx_ctx.device, &blendDesc, &blending);

	kinc_microsoft_affirm(dx_ctx.device->lpVtbl->CreateBlendState(dx_ctx.device, &blendDesc, &blending));
	dx_ctx.context->lpVtbl->OMSetBlendState(dx_ctx.context, blending, NULL, 0xffffffff);
}

void kinc_g4_flush() {}

static ID3D11ShaderResourceView *nullviews[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {0};

static kinc_g4_index_buffer_t *currentIndexBuffer = NULL;

void kinc_internal_g4_index_buffer_set(kinc_g4_index_buffer_t *buffer) {
	currentIndexBuffer = buffer;
	dx_ctx.context->lpVtbl->IASetIndexBuffer(dx_ctx.context, buffer->impl.ib, buffer->impl.sixteen ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
}

void kinc_g4_draw_indexed_vertices() {
	kinc_internal_pipeline_rebind();
	if (currentPipeline->tessellation_control_shader != NULL) {
		dx_ctx.context->lpVtbl->IASetPrimitiveTopology(dx_ctx.context, D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	}
	else {
		dx_ctx.context->lpVtbl->IASetPrimitiveTopology(dx_ctx.context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
	kinc_internal_set_constants();
	dx_ctx.context->lpVtbl->DrawIndexed(dx_ctx.context, currentIndexBuffer->impl.count, 0, 0);
}

void kinc_g4_draw_indexed_vertices_from_to(int start, int count) {
	kinc_g4_draw_indexed_vertices_from_to_from(start, count, 0);
}

void kinc_g4_draw_indexed_vertices_from_to_from(int start, int count, int vertex_offset) {
	kinc_internal_pipeline_rebind();
	if (currentPipeline->tessellation_control_shader != NULL) {
		dx_ctx.context->lpVtbl->IASetPrimitiveTopology(dx_ctx.context, D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	}
	else {
		dx_ctx.context->lpVtbl->IASetPrimitiveTopology(dx_ctx.context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
	kinc_internal_set_constants();
	dx_ctx.context->lpVtbl->DrawIndexed(dx_ctx.context, count, start, vertex_offset);
}

void kinc_g4_draw_indexed_vertices_instanced(int instanceCount) {
	kinc_g4_draw_indexed_vertices_instanced_from_to(instanceCount, 0, currentIndexBuffer->impl.count);
}

void kinc_g4_draw_indexed_vertices_instanced_from_to(int instanceCount, int start, int count) {
	kinc_internal_pipeline_rebind();
	if (currentPipeline->tessellation_control_shader != NULL) {
		dx_ctx.context->lpVtbl->IASetPrimitiveTopology(dx_ctx.context, D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	}
	else {
		dx_ctx.context->lpVtbl->IASetPrimitiveTopology(dx_ctx.context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
	kinc_internal_set_constants();
	dx_ctx.context->lpVtbl->DrawIndexedInstanced(dx_ctx.context, count, instanceCount, start, 0, 0);

	dx_ctx.context->lpVtbl->PSSetShaderResources(dx_ctx.context, 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, nullviews);
}

static D3D11_TEXTURE_ADDRESS_MODE convertAddressing(kinc_g4_texture_addressing_t addressing) {
	switch (addressing) {
	default:
	case KINC_G4_TEXTURE_ADDRESSING_REPEAT:
		return D3D11_TEXTURE_ADDRESS_WRAP;
	case KINC_G4_TEXTURE_ADDRESSING_MIRROR:
		return D3D11_TEXTURE_ADDRESS_MIRROR;
	case KINC_G4_TEXTURE_ADDRESSING_CLAMP:
		return D3D11_TEXTURE_ADDRESS_CLAMP;
	case KINC_G4_TEXTURE_ADDRESSING_BORDER:
		return D3D11_TEXTURE_ADDRESS_BORDER;
	}
}

void kinc_g4_set_texture_addressing(kinc_g4_texture_unit_t unit, kinc_g4_texture_direction_t dir, kinc_g4_texture_addressing_t addressing) {
	if (unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT] < 0) {
		return;
	}

	switch (dir) {
	case KINC_G4_TEXTURE_DIRECTION_U:
		lastSamplers[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]].AddressU = convertAddressing(addressing);
		break;
	case KINC_G4_TEXTURE_DIRECTION_V:
		lastSamplers[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]].AddressV = convertAddressing(addressing);
		break;
	case KINC_G4_TEXTURE_DIRECTION_W:
		lastSamplers[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]].AddressW = convertAddressing(addressing);
		break;
	}

	ID3D11SamplerState *sampler = getSamplerState(&lastSamplers[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]]);
	dx_ctx.context->lpVtbl->PSSetSamplers(dx_ctx.context, unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT], 1, &sampler);
}

void kinc_g4_set_texture3d_addressing(kinc_g4_texture_unit_t unit, kinc_g4_texture_direction_t dir, kinc_g4_texture_addressing_t addressing) {
	kinc_g4_set_texture_addressing(unit, dir, addressing);
}

int kinc_g4_max_bound_textures(void) {
	return D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;
}

void kinc_g4_clear(unsigned flags, unsigned color, float depth, int stencil) {
	const float clearColor[] = {((color & 0x00ff0000) >> 16) / 255.0f, ((color & 0x0000ff00) >> 8) / 255.0f, (color & 0x000000ff) / 255.0f,
	                            ((color & 0xff000000) >> 24) / 255.0f};
	for (int i = 0; i < renderTargetCount; ++i) {
		if (currentRenderTargetViews[i] != NULL && flags & KINC_G4_CLEAR_COLOR) {
			dx_ctx.context->lpVtbl->ClearRenderTargetView(dx_ctx.context, currentRenderTargetViews[i], clearColor);
		}
	}
	if (currentDepthStencilView != NULL && (flags & KINC_G4_CLEAR_DEPTH) || (flags & KINC_G4_CLEAR_STENCIL)) {
		unsigned d3dflags = ((flags & KINC_G4_CLEAR_DEPTH) ? D3D11_CLEAR_DEPTH : 0) | ((flags & KINC_G4_CLEAR_STENCIL) ? D3D11_CLEAR_STENCIL : 0);
		dx_ctx.context->lpVtbl->ClearDepthStencilView(dx_ctx.context, currentDepthStencilView, d3dflags, kinc_clamp(depth, 0.0f, 1.0f), stencil);
	}
}

void kinc_g4_begin(int windowId) {
	dx_ctx.current_window = windowId;
	struct dx_window *window = &dx_ctx.windows[windowId];
	if (window->new_width != window->width || window->new_height != window->height) {
		window->width = window->new_width;
		window->height = window->new_height;
		createBackbuffer(window, kinc_g4_antialiasing_samples());
	}
	kinc_g4_restore_render_target();
}

void kinc_g4_viewport(int x, int y, int width, int height) {
	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = (float)x;
	viewport.TopLeftY = (float)y;
	viewport.Width = (float)width;
	viewport.Height = (float)height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	dx_ctx.context->lpVtbl->RSSetViewports(dx_ctx.context, 1, &viewport);
}

void kinc_internal_set_rasterizer_state(kinc_g4_pipeline_t *pipeline, bool scissoring);

void kinc_g4_scissor(int x, int y, int width, int height) {
	D3D11_RECT rect;
	rect.left = x;
	rect.top = y;
	rect.right = x + width;
	rect.bottom = y + height;
	dx_ctx.context->lpVtbl->RSSetScissorRects(dx_ctx.context, 1, &rect);
	kinc_internal_scissoring = true;
	if (currentPipeline != NULL) {
		kinc_internal_set_rasterizer_state(currentPipeline, kinc_internal_scissoring);
	}
}

void kinc_g4_disable_scissor() {
	dx_ctx.context->lpVtbl->RSSetScissorRects(dx_ctx.context, 0, NULL);
	kinc_internal_scissoring = false;
	if (currentPipeline != NULL) {
		kinc_internal_set_rasterizer_state(currentPipeline, kinc_internal_scissoring);
	}
}

void kinc_g4_set_pipeline(kinc_g4_pipeline_t *pipeline) {
	if (pipeline != currentPipeline) {
		currentPipeline = pipeline;
		needPipelineRebind = true;
	}
}

void kinc_g4_set_stencil_reference_value(int value) {
	if (currentPipeline != NULL) {
		dx_ctx.context->lpVtbl->OMSetDepthStencilState(dx_ctx.context, currentPipeline->impl.depthStencilState, value);
	}
}

void kinc_g4_set_blend_constant(float r, float g, float b, float a) {
	if (currentBlendFactor[0] != r || currentBlendFactor[1] != g || currentBlendFactor[2] != b || currentBlendFactor[3] != a) {
		currentBlendFactor[0] = r;
		currentBlendFactor[1] = g;
		currentBlendFactor[2] = b;
		currentBlendFactor[3] = a;
		needPipelineRebind = true;
	}
}

void kinc_g4_end(int windowId) {}

bool kinc_g4_swap_buffers(void) {
	bool success = true;
	for (int i = 0; i < MAXIMUM_WINDOWS; i++) {
		if (dx_ctx.windows[i].swapChain) {
			if (!SUCCEEDED(dx_ctx.windows[i].swapChain->lpVtbl->Present(dx_ctx.windows[i].swapChain, dx_ctx.windows[i].vsync, 0))) {
				success = false;
			}
		}
	}
	// TODO: if (hr == DXGI_STATUS_OCCLUDED)...
	// http://www.pouet.net/topic.php?which=10454
	// "Proper handling of DXGI_STATUS_OCCLUDED would be to pause the application,
	// and periodically call Present with the TEST flag, and when it returns S_OK, resume rendering."

	// if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
	//	Initialize(m_window);
	//}
	// else {
	return success;
	//}
}

static void setInt(uint8_t *constants, uint32_t offset, uint32_t size, int value) {
	if (size == 0)
		return;
	int *ints = (int *)&constants[offset];
	ints[0] = value;
}

static void setInt2(uint8_t *constants, uint32_t offset, uint32_t size, int value1, int value2) {
	if (size == 0)
		return;
	int *ints = (int *)&constants[offset];
	ints[0] = value1;
	ints[1] = value2;
}

static void setInt3(uint8_t *constants, uint32_t offset, uint32_t size, int value1, int value2, int value3) {
	if (size == 0)
		return;
	int *ints = (int *)&constants[offset];
	ints[0] = value1;
	ints[1] = value2;
	ints[2] = value3;
}

static void setInt4(uint8_t *constants, uint32_t offset, uint32_t size, int value1, int value2, int value3, int value4) {
	if (size == 0)
		return;
	int *ints = (int *)&constants[offset];
	ints[0] = value1;
	ints[1] = value2;
	ints[2] = value3;
	ints[3] = value4;
}

static void setInts(uint8_t *constants, uint32_t offset, uint32_t size, uint8_t columns, uint8_t rows, int *values, int count) {
	if (size == 0)
		return;
	int *ints = (int *)&constants[offset];
	if (columns == 4 && rows == 4) {
		for (int i = 0; i < count / 16 && i < (int)size / 4; ++i) {
			for (int y = 0; y < 4; ++y) {
				for (int x = 0; x < 4; ++x) {
					ints[i * 16 + x + y * 4] = values[i * 16 + y + x * 4];
				}
			}
		}
	}
	else if (columns == 3 && rows == 3) {
		for (int i = 0; i < count / 9 && i < (int)size / 3; ++i) {
			for (int y = 0; y < 4; ++y) {
				for (int x = 0; x < 4; ++x) {
					ints[i * 12 + x + y * 4] = values[i * 9 + y + x * 3];
				}
			}
		}
	}
	else if (columns == 2 && rows == 2) {
		for (int i = 0; i < count / 4 && i < (int)size / 2; ++i) {
			for (int y = 0; y < 4; ++y) {
				for (int x = 0; x < 4; ++x) {
					ints[i * 8 + x + y * 4] = values[i * 4 + y + x * 2];
				}
			}
		}
	}
	else {
		for (int i = 0; i < count && i * 4 < (int)size; ++i) {
			ints[i] = values[i];
		}
	}
}

static void setFloat(uint8_t *constants, uint32_t offset, uint32_t size, float value) {
	if (size == 0)
		return;
	float *floats = (float *)&constants[offset];
	floats[0] = value;
}

static void setFloat2(uint8_t *constants, uint32_t offset, uint32_t size, float value1, float value2) {
	if (size == 0)
		return;
	float *floats = (float *)&constants[offset];
	floats[0] = value1;
	floats[1] = value2;
}

static void setFloat3(uint8_t *constants, uint32_t offset, uint32_t size, float value1, float value2, float value3) {
	if (size == 0)
		return;
	float *floats = (float *)&constants[offset];
	floats[0] = value1;
	floats[1] = value2;
	floats[2] = value3;
}

static void setFloat4(uint8_t *constants, uint32_t offset, uint32_t size, float value1, float value2, float value3, float value4) {
	if (size == 0)
		return;
	float *floats = (float *)&constants[offset];
	floats[0] = value1;
	floats[1] = value2;
	floats[2] = value3;
	floats[3] = value4;
}

static void setFloats(uint8_t *constants, uint32_t offset, uint32_t size, uint8_t columns, uint8_t rows, float *values, int count) {
	if (size == 0)
		return;
	float *floats = (float *)&constants[offset];
	if (columns == 4 && rows == 4) {
		for (int i = 0; i < count / 16 && i < (int)size / 4; ++i) {
			for (int y = 0; y < 4; ++y) {
				for (int x = 0; x < 4; ++x) {
					floats[i * 16 + x + y * 4] = values[i * 16 + y + x * 4];
				}
			}
		}
	}
	else if (columns == 3 && rows == 3) {
		for (int i = 0; i < count / 9 && i < (int)size / 3; ++i) {
			for (int y = 0; y < 4; ++y) {
				for (int x = 0; x < 4; ++x) {
					floats[i * 12 + x + y * 4] = values[i * 9 + y + x * 3];
				}
			}
		}
	}
	else if (columns == 2 && rows == 2) {
		for (int i = 0; i < count / 4 && i < (int)size / 2; ++i) {
			for (int y = 0; y < 4; ++y) {
				for (int x = 0; x < 4; ++x) {
					floats[i * 8 + x + y * 4] = values[i * 4 + y + x * 2];
				}
			}
		}
	}
	else {
		for (int i = 0; i < count && i * 4 < (int)size; ++i) {
			floats[i] = values[i];
		}
	}
}

static void setBool(uint8_t *constants, uint32_t offset, uint32_t size, bool value) {
	if (size == 0)
		return;
	int *ints = (int *)&constants[offset];
	ints[0] = value ? 1 : 0;
}

static void setMatrix4(uint8_t *constants, uint32_t offset, uint32_t size, kinc_matrix4x4_t *value) {
	if (size == 0)
		return;
	float *floats = (float *)&constants[offset];
	for (int y = 0; y < 4; ++y) {
		for (int x = 0; x < 4; ++x) {
			floats[x + y * 4] = value->m[y + x * 4];
		}
	}
}

static void setMatrix3(uint8_t *constants, uint32_t offset, uint32_t size, kinc_matrix3x3_t *value) {
	if (size == 0)
		return;
	float *floats = (float *)&constants[offset];
	for (int y = 0; y < 3; ++y) {
		for (int x = 0; x < 3; ++x) {
			floats[x + y * 4] = value->m[y + x * 3];
		}
	}
}

void kinc_g4_set_int(kinc_g4_constant_location_t location, int value) {
	setInt(vertexConstants, location.impl.vertexOffset, location.impl.vertexSize, value);
	setInt(fragmentConstants, location.impl.fragmentOffset, location.impl.fragmentSize, value);
	setInt(geometryConstants, location.impl.geometryOffset, location.impl.geometrySize, value);
	setInt(tessEvalConstants, location.impl.tessEvalOffset, location.impl.tessEvalSize, value);
	setInt(tessControlConstants, location.impl.tessControlOffset, location.impl.tessControlSize, value);
	setInt(computeConstants, location.impl.computeOffset, location.impl.computeSize, value);
}

void kinc_g4_set_int2(kinc_g4_constant_location_t location, int value1, int value2) {
	setInt2(vertexConstants, location.impl.vertexOffset, location.impl.vertexSize, value1, value2);
	setInt2(fragmentConstants, location.impl.fragmentOffset, location.impl.fragmentSize, value1, value2);
	setInt2(geometryConstants, location.impl.geometryOffset, location.impl.geometrySize, value1, value2);
	setInt2(tessEvalConstants, location.impl.tessEvalOffset, location.impl.tessEvalSize, value1, value2);
	setInt2(tessControlConstants, location.impl.tessControlOffset, location.impl.tessControlSize, value1, value2);
	setInt2(computeConstants, location.impl.computeOffset, location.impl.computeSize, value1, value2);
}

void kinc_g4_set_int3(kinc_g4_constant_location_t location, int value1, int value2, int value3) {
	setInt3(vertexConstants, location.impl.vertexOffset, location.impl.vertexSize, value1, value2, value3);
	setInt3(fragmentConstants, location.impl.fragmentOffset, location.impl.fragmentSize, value1, value2, value3);
	setInt3(geometryConstants, location.impl.geometryOffset, location.impl.geometrySize, value1, value2, value3);
	setInt3(tessEvalConstants, location.impl.tessEvalOffset, location.impl.tessEvalSize, value1, value2, value3);
	setInt3(tessControlConstants, location.impl.tessControlOffset, location.impl.tessControlSize, value1, value2, value3);
	setInt3(computeConstants, location.impl.computeOffset, location.impl.computeSize, value1, value2, value3);
}

void kinc_g4_set_int4(kinc_g4_constant_location_t location, int value1, int value2, int value3, int value4) {
	setInt4(vertexConstants, location.impl.vertexOffset, location.impl.vertexSize, value1, value2, value3, value4);
	setInt4(fragmentConstants, location.impl.fragmentOffset, location.impl.fragmentSize, value1, value2, value3, value4);
	setInt4(geometryConstants, location.impl.geometryOffset, location.impl.geometrySize, value1, value2, value3, value4);
	setInt4(tessEvalConstants, location.impl.tessEvalOffset, location.impl.tessEvalSize, value1, value2, value3, value4);
	setInt4(tessControlConstants, location.impl.tessControlOffset, location.impl.tessControlSize, value1, value2, value3, value4);
	setInt4(computeConstants, location.impl.computeOffset, location.impl.computeSize, value1, value2, value3, value4);
}

void kinc_g4_set_ints(kinc_g4_constant_location_t location, int *values, int count) {
	setInts(vertexConstants, location.impl.vertexOffset, location.impl.vertexSize, location.impl.vertexColumns, location.impl.vertexRows, values, count);
	setInts(fragmentConstants, location.impl.fragmentOffset, location.impl.fragmentSize, location.impl.fragmentColumns, location.impl.fragmentRows, values,
	        count);
	setInts(geometryConstants, location.impl.geometryOffset, location.impl.geometrySize, location.impl.geometryColumns, location.impl.geometryRows, values,
	        count);
	setInts(tessEvalConstants, location.impl.tessEvalOffset, location.impl.tessEvalSize, location.impl.tessEvalColumns, location.impl.tessEvalRows, values,
	        count);
	setInts(tessControlConstants, location.impl.tessControlOffset, location.impl.tessControlSize, location.impl.tessControlColumns,
	        location.impl.tessControlRows, values, count);
	setInts(computeConstants, location.impl.computeOffset, location.impl.computeSize, location.impl.computeColumns, location.impl.computeRows, values, count);
}

void kinc_g4_set_float(kinc_g4_constant_location_t location, float value) {
	setFloat(vertexConstants, location.impl.vertexOffset, location.impl.vertexSize, value);
	setFloat(fragmentConstants, location.impl.fragmentOffset, location.impl.fragmentSize, value);
	setFloat(geometryConstants, location.impl.geometryOffset, location.impl.geometrySize, value);
	setFloat(tessEvalConstants, location.impl.tessEvalOffset, location.impl.tessEvalSize, value);
	setFloat(tessControlConstants, location.impl.tessControlOffset, location.impl.tessControlSize, value);
	setFloat(computeConstants, location.impl.computeOffset, location.impl.computeSize, value);
}

void kinc_g4_set_float2(kinc_g4_constant_location_t location, float value1, float value2) {
	setFloat2(vertexConstants, location.impl.vertexOffset, location.impl.vertexSize, value1, value2);
	setFloat2(fragmentConstants, location.impl.fragmentOffset, location.impl.fragmentSize, value1, value2);
	setFloat2(geometryConstants, location.impl.geometryOffset, location.impl.geometrySize, value1, value2);
	setFloat2(tessEvalConstants, location.impl.tessEvalOffset, location.impl.tessEvalSize, value1, value2);
	setFloat2(tessControlConstants, location.impl.tessControlOffset, location.impl.tessControlSize, value1, value2);
	setFloat2(computeConstants, location.impl.computeOffset, location.impl.computeSize, value1, value2);
}

void kinc_g4_set_float3(kinc_g4_constant_location_t location, float value1, float value2, float value3) {
	setFloat3(vertexConstants, location.impl.vertexOffset, location.impl.vertexSize, value1, value2, value3);
	setFloat3(fragmentConstants, location.impl.fragmentOffset, location.impl.fragmentSize, value1, value2, value3);
	setFloat3(geometryConstants, location.impl.geometryOffset, location.impl.geometrySize, value1, value2, value3);
	setFloat3(tessEvalConstants, location.impl.tessEvalOffset, location.impl.tessEvalSize, value1, value2, value3);
	setFloat3(tessControlConstants, location.impl.tessControlOffset, location.impl.tessControlSize, value1, value2, value3);
	setFloat3(computeConstants, location.impl.computeOffset, location.impl.computeSize, value1, value2, value3);
}

void kinc_g4_set_float4(kinc_g4_constant_location_t location, float value1, float value2, float value3, float value4) {
	setFloat4(vertexConstants, location.impl.vertexOffset, location.impl.vertexSize, value1, value2, value3, value4);
	setFloat4(fragmentConstants, location.impl.fragmentOffset, location.impl.fragmentSize, value1, value2, value3, value4);
	setFloat4(geometryConstants, location.impl.geometryOffset, location.impl.geometrySize, value1, value2, value3, value4);
	setFloat4(tessEvalConstants, location.impl.tessEvalOffset, location.impl.tessEvalSize, value1, value2, value3, value4);
	setFloat4(tessControlConstants, location.impl.tessControlOffset, location.impl.tessControlSize, value1, value2, value3, value4);
	setFloat4(computeConstants, location.impl.computeOffset, location.impl.computeSize, value1, value2, value3, value4);
}

void kinc_g4_set_floats(kinc_g4_constant_location_t location, float *values, int count) {
	setFloats(vertexConstants, location.impl.vertexOffset, location.impl.vertexSize, location.impl.vertexColumns, location.impl.vertexRows, values, count);
	setFloats(fragmentConstants, location.impl.fragmentOffset, location.impl.fragmentSize, location.impl.fragmentColumns, location.impl.fragmentRows, values,
	          count);
	setFloats(geometryConstants, location.impl.geometryOffset, location.impl.geometrySize, location.impl.geometryColumns, location.impl.geometryRows, values,
	          count);
	setFloats(tessEvalConstants, location.impl.tessEvalOffset, location.impl.tessEvalSize, location.impl.tessEvalColumns, location.impl.tessEvalRows, values,
	          count);
	setFloats(tessControlConstants, location.impl.tessControlOffset, location.impl.tessControlSize, location.impl.tessControlColumns,
	          location.impl.tessControlRows, values, count);
	setFloats(computeConstants, location.impl.computeOffset, location.impl.computeSize, location.impl.computeColumns, location.impl.computeRows, values, count);
}

void kinc_g4_set_bool(kinc_g4_constant_location_t location, bool value) {
	setBool(vertexConstants, location.impl.vertexOffset, location.impl.vertexSize, value);
	setBool(fragmentConstants, location.impl.fragmentOffset, location.impl.fragmentSize, value);
	setBool(geometryConstants, location.impl.geometryOffset, location.impl.geometrySize, value);
	setBool(tessEvalConstants, location.impl.tessEvalOffset, location.impl.tessEvalSize, value);
	setBool(tessControlConstants, location.impl.tessControlOffset, location.impl.tessControlSize, value);
	setBool(computeConstants, location.impl.computeOffset, location.impl.computeSize, value);
}

void kinc_g4_set_matrix4(kinc_g4_constant_location_t location, kinc_matrix4x4_t *value) {
	setMatrix4(vertexConstants, location.impl.vertexOffset, location.impl.vertexSize, value);
	setMatrix4(fragmentConstants, location.impl.fragmentOffset, location.impl.fragmentSize, value);
	setMatrix4(geometryConstants, location.impl.geometryOffset, location.impl.geometrySize, value);
	setMatrix4(tessEvalConstants, location.impl.tessEvalOffset, location.impl.tessEvalSize, value);
	setMatrix4(tessControlConstants, location.impl.tessControlOffset, location.impl.tessControlSize, value);
	setMatrix4(computeConstants, location.impl.computeOffset, location.impl.computeSize, value);
}

void kinc_g4_set_matrix3(kinc_g4_constant_location_t location, kinc_matrix3x3_t *value) {
	setMatrix3(vertexConstants, location.impl.vertexOffset, location.impl.vertexSize, value);
	setMatrix3(fragmentConstants, location.impl.fragmentOffset, location.impl.fragmentSize, value);
	setMatrix3(geometryConstants, location.impl.geometryOffset, location.impl.geometrySize, value);
	setMatrix3(tessEvalConstants, location.impl.tessEvalOffset, location.impl.tessEvalSize, value);
	setMatrix3(tessControlConstants, location.impl.tessControlOffset, location.impl.tessControlSize, value);
	setMatrix3(computeConstants, location.impl.computeOffset, location.impl.computeSize, value);
}

void kinc_g4_set_texture_magnification_filter(kinc_g4_texture_unit_t unit, kinc_g4_texture_filter_t filter) {
	if (unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT] < 0)
		return;

	D3D11_FILTER d3d11filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

	switch (filter) {
	case KINC_G4_TEXTURE_FILTER_POINT:
		switch (lastSamplers[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]].Filter) {
		case D3D11_FILTER_MIN_MAG_MIP_POINT:
			d3d11filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			break;
		case D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR:
		case D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT:
		case D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR:
			d3d11filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
			break;
		case D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT:
		case D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT:
			d3d11filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
			break;
		case D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
		case D3D11_FILTER_MIN_MAG_MIP_LINEAR:
		case D3D11_FILTER_ANISOTROPIC:
			d3d11filter = D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
			break;
		default:
			break;
		}
		break;
	case KINC_G4_TEXTURE_FILTER_LINEAR:
		switch (lastSamplers[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]].Filter) {
		case D3D11_FILTER_MIN_MAG_MIP_POINT:
		case D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT:
			d3d11filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
			break;
		case D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR:
			d3d11filter = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
			break;
		case D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT:
		case D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT:
			d3d11filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			break;
		case D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
		case D3D11_FILTER_MIN_MAG_MIP_LINEAR:
		case D3D11_FILTER_ANISOTROPIC:
			d3d11filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			break;
		default:
			break;
		}
		break;
	case KINC_G4_TEXTURE_FILTER_ANISOTROPIC:
		d3d11filter = D3D11_FILTER_ANISOTROPIC;
		break;
	}

	lastSamplers[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]].Filter = d3d11filter;
	lastSamplers[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]].MaxAnisotropy = d3d11filter == D3D11_FILTER_ANISOTROPIC ? D3D11_REQ_MAXANISOTROPY : 0;

	ID3D11SamplerState *sampler = getSamplerState(&lastSamplers[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]]);
	dx_ctx.context->lpVtbl->PSSetSamplers(dx_ctx.context, unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT], 1, &sampler);
}

void kinc_g4_set_texture3d_magnification_filter(kinc_g4_texture_unit_t texunit, kinc_g4_texture_filter_t filter) {
	kinc_g4_set_texture_magnification_filter(texunit, filter);
}

void kinc_g4_set_texture_minification_filter(kinc_g4_texture_unit_t unit, kinc_g4_texture_filter_t filter) {
	if (unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT] < 0)
		return;

	D3D11_FILTER d3d11filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

	switch (filter) {
	case KINC_G4_TEXTURE_FILTER_POINT:
		switch (lastSamplers[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]].Filter) {
		case D3D11_FILTER_MIN_MAG_MIP_POINT:
		case D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT:
			d3d11filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			break;
		case D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR:
		case D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
			d3d11filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
			break;
		case D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT:
		case D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT:
			d3d11filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
			break;
		case D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR:
		case D3D11_FILTER_MIN_MAG_MIP_LINEAR:
		case D3D11_FILTER_ANISOTROPIC:
			d3d11filter = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
			break;
		default:
			break;
		}
		break;
	case KINC_G4_TEXTURE_FILTER_LINEAR:
		switch (lastSamplers[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]].Filter) {
		case D3D11_FILTER_MIN_MAG_MIP_POINT:
		case D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT:
			d3d11filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
			break;
		case D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR:
		case D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
			d3d11filter = D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
			break;
		case D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT:
		case D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT:
			d3d11filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			break;
		case D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR:
		case D3D11_FILTER_MIN_MAG_MIP_LINEAR:
		case D3D11_FILTER_ANISOTROPIC:
			d3d11filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			break;
		default:
			break;
		}
		break;
	case KINC_G4_TEXTURE_FILTER_ANISOTROPIC:
		d3d11filter = D3D11_FILTER_ANISOTROPIC;
		break;
	default:
		break;
	}

	lastSamplers[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]].Filter = d3d11filter;
	lastSamplers[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]].MaxAnisotropy = d3d11filter == D3D11_FILTER_ANISOTROPIC ? D3D11_REQ_MAXANISOTROPY : 0;

	ID3D11SamplerState *sampler = getSamplerState(&lastSamplers[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]]);
	dx_ctx.context->lpVtbl->PSSetSamplers(dx_ctx.context, unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT], 1, &sampler);
}

void kinc_g4_set_texture3d_minification_filter(kinc_g4_texture_unit_t texunit, kinc_g4_texture_filter_t filter) {
	kinc_g4_set_texture_minification_filter(texunit, filter);
}

void kinc_g4_set_texture_mipmap_filter(kinc_g4_texture_unit_t unit, kinc_g4_mipmap_filter_t filter) {
	if (unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT] < 0)
		return;

	D3D11_FILTER d3d11filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

	switch (filter) {
	case KINC_G4_MIPMAP_FILTER_NONE:
	case KINC_G4_MIPMAP_FILTER_POINT:
		switch (lastSamplers[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]].Filter) {
		case D3D11_FILTER_MIN_MAG_MIP_POINT:
		case D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR:
			d3d11filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			break;
		case D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT:
		case D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR:
			d3d11filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
			break;
		case D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT:
		case D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
			d3d11filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
			break;
		case D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT:
		case D3D11_FILTER_MIN_MAG_MIP_LINEAR:
			d3d11filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			break;
		case D3D11_FILTER_ANISOTROPIC:
			d3d11filter = D3D11_FILTER_ANISOTROPIC;
			break;
		default:
			break;
		}
		break;
	case KINC_G4_MIPMAP_FILTER_LINEAR:
		switch (lastSamplers[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]].Filter) {
		case D3D11_FILTER_MIN_MAG_MIP_POINT:
		case D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR:
			d3d11filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
			break;
		case D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT:
		case D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR:
			d3d11filter = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
			break;
		case D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT:
		case D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
			d3d11filter = D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
			break;
		case D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT:
		case D3D11_FILTER_MIN_MAG_MIP_LINEAR:
			d3d11filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			break;
		case D3D11_FILTER_ANISOTROPIC:
			d3d11filter = D3D11_FILTER_ANISOTROPIC;
			break;
		default:
			break;
		}
		break;
	}

	lastSamplers[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]].Filter = d3d11filter;
	lastSamplers[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]].MaxAnisotropy = d3d11filter == D3D11_FILTER_ANISOTROPIC ? D3D11_REQ_MAXANISOTROPY : 0;

	ID3D11SamplerState *sampler = getSamplerState(&lastSamplers[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]]);
	dx_ctx.context->lpVtbl->PSSetSamplers(dx_ctx.context, unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT], 1, &sampler);
}

void kinc_g4_set_texture3d_mipmap_filter(kinc_g4_texture_unit_t texunit, kinc_g4_mipmap_filter_t filter) {
	kinc_g4_set_texture_mipmap_filter(texunit, filter);
}

void kinc_g4_set_texture_compare_mode(kinc_g4_texture_unit_t unit, bool enabled) {
	if (unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT] < 0)
		return;

	if (enabled) {
		lastSamplers[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]].ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
		lastSamplers[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]].Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	}
	else {
		lastSamplers[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]].ComparisonFunc = D3D11_COMPARISON_NEVER;
	}

	ID3D11SamplerState *sampler = getSamplerState(&lastSamplers[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]]);
	dx_ctx.context->lpVtbl->PSSetSamplers(dx_ctx.context, unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT], 1, &sampler);
}

void kinc_g4_set_texture_compare_func(kinc_g4_texture_unit_t unit, kinc_g4_compare_mode_t mode) {
	if (unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT] < 0)
		return;

	lastSamplers[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]].ComparisonFunc = get_comparison(mode);

	ID3D11SamplerState *sampler = getSamplerState(&lastSamplers[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]]);
	dx_ctx.context->lpVtbl->PSSetSamplers(dx_ctx.context, unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT], 1, &sampler);
}

void kinc_g4_set_cubemap_compare_mode(kinc_g4_texture_unit_t unit, bool enabled) {
	kinc_g4_set_texture_compare_mode(unit, enabled);
}

void kinc_g4_set_cubemap_compare_func(kinc_g4_texture_unit_t unit, kinc_g4_compare_mode_t mode) {
	kinc_g4_set_texture_compare_func(unit, mode);
}

void kinc_g4_set_texture_max_anisotropy(kinc_g4_texture_unit_t unit, uint16_t max_anisotropy) {
	if (unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT] < 0)
		return;

	lastSamplers[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]].MaxAnisotropy = max_anisotropy;

	ID3D11SamplerState *sampler = getSamplerState(&lastSamplers[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]]);
	dx_ctx.context->lpVtbl->PSSetSamplers(dx_ctx.context, unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT], 1, &sampler);
}

void kinc_g4_set_cubemap_max_anisotropy(kinc_g4_texture_unit_t unit, uint16_t max_anisotropy) {
	kinc_g4_set_texture_max_anisotropy(unit, max_anisotropy);
}

void kinc_g4_set_texture_lod(kinc_g4_texture_unit_t unit, float lod_min_clamp, float lod_max_clamp) {
	if (unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT] < 0)
		return;

	lastSamplers[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]].MinLOD = lod_min_clamp;
	lastSamplers[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]].MaxLOD = lod_max_clamp;

	ID3D11SamplerState *sampler = getSamplerState(&lastSamplers[unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT]]);
	dx_ctx.context->lpVtbl->PSSetSamplers(dx_ctx.context, unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT], 1, &sampler);
}

void kinc_g4_set_cubemap_lod(kinc_g4_texture_unit_t unit, float lod_min_clamp, float lod_max_clamp) {
	kinc_g4_set_texture_lod(unit, lod_min_clamp, lod_max_clamp);
}

void kinc_g4_restore_render_target(void) {
	struct dx_window *window = &dx_ctx.windows[dx_ctx.current_window];
	currentRenderTargetViews[0] = window->renderTargetView;
	currentDepthStencilView = window->depthStencilView;
	dx_ctx.context->lpVtbl->OMSetRenderTargets(dx_ctx.context, 1, &window->renderTargetView, window->depthStencilView);
	renderTargetCount = 1;
	D3D11_VIEWPORT viewPort;
	viewPort.TopLeftX = 0.0f;
	viewPort.TopLeftY = 0.0f;
	viewPort.Width = (float)window->width;
	viewPort.Height = (float)window->height;
	viewPort.MinDepth = D3D11_MIN_DEPTH;
	viewPort.MaxDepth = D3D11_MAX_DEPTH;
	dx_ctx.context->lpVtbl->RSSetViewports(dx_ctx.context, 1, &viewPort);
}

void kinc_g4_set_render_targets(struct kinc_g4_render_target **targets, int count) {
	currentDepthStencilView = targets[0]->impl.depthStencilView[0];

	renderTargetCount = count;
	for (int i = 0; i < count; ++i) {
		currentRenderTargetViews[i] = targets[i]->impl.renderTargetViewRender[0];
	}

	dx_ctx.context->lpVtbl->OMSetRenderTargets(dx_ctx.context, count, currentRenderTargetViews, currentDepthStencilView);
	D3D11_VIEWPORT viewPort;
	viewPort.TopLeftX = 0.0f;
	viewPort.TopLeftY = 0.0f;
	viewPort.Width = (float)targets[0]->width;
	viewPort.Height = (float)targets[0]->height;
	viewPort.MinDepth = D3D11_MIN_DEPTH;
	viewPort.MaxDepth = D3D11_MAX_DEPTH;
	dx_ctx.context->lpVtbl->RSSetViewports(dx_ctx.context, 1, &viewPort);
}

void kinc_g4_set_render_target_face(struct kinc_g4_render_target *texture, int face) {
	renderTargetCount = 1;
	currentRenderTargetViews[0] = texture->impl.renderTargetViewRender[face];
	currentDepthStencilView = texture->impl.depthStencilView[face];
	dx_ctx.context->lpVtbl->OMSetRenderTargets(dx_ctx.context, 1, currentRenderTargetViews, currentDepthStencilView);
	D3D11_VIEWPORT viewPort;
	viewPort.TopLeftX = 0.0f;
	viewPort.TopLeftY = 0.0f;
	viewPort.Width = (float)texture->width;
	viewPort.Height = (float)texture->height;
	viewPort.MinDepth = D3D11_MIN_DEPTH;
	viewPort.MaxDepth = D3D11_MAX_DEPTH;
	dx_ctx.context->lpVtbl->RSSetViewports(dx_ctx.context, 1, &viewPort);
}

void kinc_g4_set_vertex_buffers(kinc_g4_vertex_buffer_t **buffers, int count) {
	kinc_internal_g4_vertex_buffer_set(buffers[0], 0);

	ID3D11Buffer **d3dbuffers = (ID3D11Buffer **)alloca(count * sizeof(ID3D11Buffer *));
	for (int i = 0; i < count; ++i) {
		d3dbuffers[i] = buffers[i]->impl.vb;
	}

	UINT *strides = (UINT *)alloca(count * sizeof(UINT));
	for (int i = 0; i < count; ++i) {
		strides[i] = buffers[i]->impl.stride;
	}

	UINT *internaloffsets = (UINT *)alloca(count * sizeof(UINT));
	for (int i = 0; i < count; ++i) {
		internaloffsets[i] = 0;
	}

	dx_ctx.context->lpVtbl->IASetVertexBuffers(dx_ctx.context, 0, count, d3dbuffers, strides, internaloffsets);
}

void kinc_g4_set_index_buffer(kinc_g4_index_buffer_t *buffer) {
	kinc_internal_g4_index_buffer_set(buffer);
}

void kinc_internal_texture_set(kinc_g4_texture_t *texture, kinc_g4_texture_unit_t unit);

void kinc_g4_set_texture(kinc_g4_texture_unit_t unit, kinc_g4_texture_t *texture) {
	kinc_internal_texture_set(texture, unit);
}

void kinc_internal_texture_set_image(kinc_g4_texture_t *texture, kinc_g4_texture_unit_t unit);

void kinc_g4_set_image_texture(kinc_g4_texture_unit_t unit, kinc_g4_texture_t *texture) {
	kinc_internal_texture_set_image(texture, unit);
}

void kinc_internal_resize(int windowId, int width, int height) {
	struct dx_window *window = &dx_ctx.windows[windowId];
	window->new_width = width;
	window->new_height = height;
}

void kinc_internal_change_framebuffer(int window_index, kinc_framebuffer_options_t *frame) {
	struct dx_window *window = &dx_ctx.windows[window_index];
	kinc_g4_set_antialiasing_samples(frame->samples_per_pixel);
	window->vsync = frame->vertical_sync;
}

bool kinc_window_vsynced(int window_index) {
	struct dx_window *window = &dx_ctx.windows[window_index];
	return window->vsync;
}

bool kinc_g4_supports_instanced_rendering(void) {
	return true;
}

bool kinc_g4_supports_compute_shaders(void) {
	return true;
}

bool kinc_g4_supports_blend_constants(void) {
	return true;
}

bool kinc_g4_supports_non_pow2_textures(void) {
	return true; // we always request feature level >= 10
}

bool kinc_g4_render_targets_inverted_y(void) {
	return false;
}
