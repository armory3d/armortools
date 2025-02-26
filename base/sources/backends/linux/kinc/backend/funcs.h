#pragma once

#include <iron_global.h>
#include <iron_system.h>
#include <vulkan/vulkan.h>

struct linux_procs {
	bool (*handle_messages)(void);
	void (*shutdown)(void);

	void (*display_init)(void);
	kinc_display_mode_t (*display_available_mode)(int display, int mode);
	int (*display_count_available_modes)(int display);
	bool (*display_available)(int display_index);
	const char *(*display_name)(int display_index);
	kinc_display_mode_t (*display_current_mode)(int display_index);
	int (*display_primary)(void);
	int (*count_displays)(void);

	void (*window_create)(kinc_window_options_t *window_options, kinc_framebuffer_options_t *framebuffer_options);
	void (*window_destroy)();
	int (*window_display)();
	void (*window_show)();
	void (*window_hide)();
	void (*window_set_title)(const char *title);
	void (*window_change_mode)(kinc_window_mode_t mode);
	kinc_window_mode_t (*window_get_mode)();
	void (*window_move)(int x, int y);
	void (*window_resize)(int width, int height);
	int (*window_x)();
	int (*window_y)();
	int (*window_width)();
	int (*window_height)();

	bool (*mouse_can_lock)(void);
	bool (*mouse_is_locked)(void);
	void (*mouse_lock)();
	void (*mouse_unlock)(void);
	void (*mouse_show)(void);
	void (*mouse_hide)(void);
	void (*mouse_set_position)(int x, int y);
	void (*mouse_get_position)(int *x, int *y);
	void (*mouse_set_cursor)(int cursor);

	void (*copy_to_clipboard)(const char *text);

	void (*vulkan_get_instance_extensions)(const char **extensions, int *count, int max);
	VkResult (*vulkan_create_surface)(VkInstance instance, VkSurfaceKHR *surface);
	VkBool32 (*vulkan_get_physical_device_presentation_support)(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex);
};

extern struct linux_procs procs;

void kinc_linux_init_procs();