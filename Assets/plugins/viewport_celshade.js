
let plugin = new arm.Plugin();

// Register custom viewport shader
arm.Context.setViewportShader(function(shader) {
	shader.add_uniform('vec3 lightDir', '_lightDirection');
	shader.write(`
		float dotNL = max(dot(n, lightDir), 0.0);
		vec3 outputColor = basecol * step(0.5, dotNL) + basecol;
	`);
	return 'outputColor';
});

plugin.delete = function() {
	arm.Context.setViewportShader(null);
}
