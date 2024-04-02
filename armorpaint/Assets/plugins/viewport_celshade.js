
let plugin = plugin_create();

// Register custom viewport shader
context_set_viewport_shader(function(shader) {
	node_shader_add_uniform(shader, "vec3 lightDir", "_light_dir");
	node_shader_write(shader, `
		float dotNL = max(dot(n, lightDir), 0.0);
		vec3 outputColor = basecol * step(0.5, dotNL) + basecol;
	`);
	return "outputColor";
});

plugin.delete = function() {
	context_set_viewport_shader(null);
}
