#include "funcs.h"
#include <kinc/display.h>
#include <kinc/graphics4/graphics.h>
#include <kinc/window.h>

#include <string.h>

void kinc_vulkan_get_instance_extensions(const char **extensions, int *count, int max) {
	procs.vulkan_get_instance_extensions(extensions, count, max);
}

VkBool32 kinc_vulkan_get_physical_device_presentation_support(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex) {
	return procs.vulkan_get_physical_device_presentation_support(physicalDevice, queueFamilyIndex);
}
VkResult kinc_vulkan_create_surface(VkInstance instance, int window_index, VkSurfaceKHR *surface) {
	return procs.vulkan_create_surface(instance, window_index, surface);
}

int kinc_count_windows(void) {
	return procs.count_windows();
}

int kinc_window_x(int window_index) {
	return procs.window_x(window_index);
}

int kinc_window_y(int window_index) {
	return procs.window_y(window_index);
}

int kinc_window_width(int window_index) {
	return procs.window_width(window_index);
}

int kinc_window_height(int window_index) {
	return procs.window_height(window_index);
}

void kinc_window_resize(int window_index, int width, int height) {
	procs.window_resize(window_index, width, height);
}

void kinc_window_move(int window_index, int x, int y) {
	procs.window_move(window_index, x, y);
}

void kinc_window_change_framebuffer(int window_index, kinc_framebuffer_options_t *frame) {}

void kinc_window_change_features(int window_index, int features) {}

void kinc_window_change_mode(int window_index, kinc_window_mode_t mode) {
	procs.window_change_mode(window_index, mode);
}

int kinc_window_display(int window_index) {
	return procs.window_display(window_index);
}

void kinc_window_destroy(int window_index) {
	kinc_g4_internal_destroy_window(window_index);
	procs.window_destroy(window_index);
}

void kinc_window_show(int window_index) {
	procs.window_show(window_index);
}

void kinc_window_hide(int window_index) {
	procs.window_hide(window_index);
}

void kinc_window_set_title(int window_index, const char *title) {
	procs.window_set_title(window_index, title);
}

int kinc_window_create(kinc_window_options_t *win, kinc_framebuffer_options_t *frame) {
	int index = procs.window_create(win, frame);
	kinc_g4_internal_init_window(index, frame->depth_bits, frame->vertical_sync);
	return index;
}

static struct {
	void (*resize_callback)(int width, int height, void *data);
	void *resize_data;
	void *ppi_data;
	bool (*close_callback)(void *data);
	void *close_data;
} kinc_internal_window_callbacks[16];

void kinc_window_set_resize_callback(int window_index, void (*callback)(int width, int height, void *data), void *data) {
	kinc_internal_window_callbacks[window_index].resize_callback = callback;
	kinc_internal_window_callbacks[window_index].resize_data = data;
}

void kinc_internal_call_resize_callback(int window_index, int width, int height) {
	if (kinc_internal_window_callbacks[window_index].resize_callback != NULL) {
		kinc_internal_window_callbacks[window_index].resize_callback(width, height, kinc_internal_window_callbacks[window_index].resize_data);
	}
}

void kinc_window_set_close_callback(int window_index, bool (*callback)(void *data), void *data) {
	kinc_internal_window_callbacks[window_index].close_callback = callback;
	kinc_internal_window_callbacks[window_index].close_data = data;
}

bool kinc_internal_call_close_callback(int window_index) {
	if (kinc_internal_window_callbacks[window_index].close_callback != NULL) {
		return kinc_internal_window_callbacks[window_index].close_callback(kinc_internal_window_callbacks[window_index].close_data);
	}
	else {
		return true;
	}
}

kinc_window_mode_t kinc_window_get_mode(int window_index) {
	return procs.window_get_mode(window_index);
}
