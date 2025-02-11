
let plugin = plugin_create();

// Register custom viewport shader
context_set_viewport_shader(function(shader) {
	// node_shader_add_uniform(shader, "vec3 light_dir", "_light_dir");
	node_shader_write(shader, " \
		vec3 light_dir = vec3(0.5, 0.5, -0.5);\
		float dotnl = max(dot(n, light_dir), 0.0); \
		output_color = basecol * step(0.5, dotnl) + basecol; \
	");
});

plugin_notify_on_delete(plugin, function() {
	context_set_viewport_shader(null);
});
