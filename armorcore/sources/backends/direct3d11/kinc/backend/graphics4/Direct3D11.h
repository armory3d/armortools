#pragma once

#include <d3d11.h>
#include <dxgi.h>
#include <stdbool.h>

#define MAXIMUM_WINDOWS 16

struct dx_window {
	HWND hwnd;
	IDXGISwapChain *swapChain;
	ID3D11Texture2D *backBuffer;
	ID3D11RenderTargetView *renderTargetView;
	ID3D11Texture2D *depthStencil;
	ID3D11DepthStencilView *depthStencilView;

	int width;
	int height;

	int new_width;
	int new_height;

	bool vsync;
	int depth_bits;
	int stencil_bits;
};

struct dx_context {
	ID3D11Device *device;
	ID3D11DeviceContext *context;
	IDXGIDevice *dxgiDevice;
	IDXGIAdapter *dxgiAdapter;
	IDXGIFactory *dxgiFactory;

	int current_window;
	struct dx_window windows[MAXIMUM_WINDOWS];
};

extern struct dx_context dx_ctx;

#include <kinc/backend/SystemMicrosoft.h>
