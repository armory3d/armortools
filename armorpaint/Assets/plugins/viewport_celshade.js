
let plugin = Plugin.create();

// Register custom viewport shader
ContextBase.setViewportShader(function(shader) {
	shader.add_uniform('vec3 lightDir', '_light_dir');
	shader.write(`
		float dotNL = max(dot(n, lightDir), 0.0);
		vec3 outputColor = basecol * step(0.5, dotNL) + basecol;
	`);
	return 'outputColor';
});

plugin.delete = function() {
	ContextBase.setViewportShader(null);
}
