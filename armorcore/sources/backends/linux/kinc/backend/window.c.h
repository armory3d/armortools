#include "funcs.h"
#include <kinc/display.h>
#include <kinc/graphics5/g5.h>
#include <kinc/window.h>

#include <string.h>

void kinc_vulkan_get_instance_extensions(const char **extensions, int *count, int max) {
	procs.vulkan_get_instance_extensions(extensions, count, max);
}

VkBool32 kinc_vulkan_get_physical_device_presentation_support(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex) {
	return procs.vulkan_get_physical_device_presentation_support(physicalDevice, queueFamilyIndex);
}
VkResult kinc_vulkan_create_surface(VkInstance instance, VkSurfaceKHR *surface) {
	return procs.vulkan_create_surface(instance, surface);
}

int kinc_window_x() {
	return procs.window_x();
}

int kinc_window_y() {
	return procs.window_y();
}

int kinc_window_width() {
	return procs.window_width();
}

int kinc_window_height() {
	return procs.window_height();
}

void kinc_window_resize(int width, int height) {
	procs.window_resize(width, height);
}

void kinc_window_move(int x, int y) {
	procs.window_move(x, y);
}

void kinc_window_change_features(int features) {}

void kinc_window_change_mode(kinc_window_mode_t mode) {
	procs.window_change_mode(mode);
}

int kinc_window_display() {
	return procs.window_display();
}

void kinc_window_destroy() {
	kinc_g5_internal_destroy_window();
	procs.window_destroy();
}

void kinc_window_show() {
	procs.window_show();
}

void kinc_window_hide() {
	procs.window_hide();
}

void kinc_window_set_title(const char *title) {
	procs.window_set_title(title);
}

void kinc_window_create(kinc_window_options_t *win, kinc_framebuffer_options_t *frame) {
	procs.window_create(win, frame);
	kinc_g4_internal_init_window(frame->depth_bits, frame->vertical_sync);
}

static struct {
	void (*resize_callback)(int width, int height, void *data);
	void *resize_data;
	void *ppi_data;
	bool (*close_callback)(void *data);
	void *close_data;
} kinc_internal_window_callbacks[16];

void kinc_window_set_resize_callback(void (*callback)(int width, int height, void *data), void *data) {
	kinc_internal_window_callbacks[0].resize_callback = callback;
	kinc_internal_window_callbacks[0].resize_data = data;
}

void kinc_internal_call_resize_callback(int width, int height) {
	if (kinc_internal_window_callbacks[0].resize_callback != NULL) {
		kinc_internal_window_callbacks[0].resize_callback(width, height, kinc_internal_window_callbacks[0].resize_data);
	}
}

void kinc_window_set_close_callback(bool (*callback)(void *data), void *data) {
	kinc_internal_window_callbacks[0].close_callback = callback;
	kinc_internal_window_callbacks[0].close_data = data;
}

bool kinc_internal_call_close_callback() {
	if (kinc_internal_window_callbacks[0].close_callback != NULL) {
		return kinc_internal_window_callbacks[0].close_callback(kinc_internal_window_callbacks[0].close_data);
	}
	else {
		return true;
	}
}

kinc_window_mode_t kinc_window_get_mode() {
	return procs.window_get_mode();
}
