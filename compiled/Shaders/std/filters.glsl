
vec4 cubicCatmullrom(float x) {
	const float s = 0.5;
	float x2 = x * x;
	float x3 = x2 * x;
	vec4 w;
	w.x =    -s*x3 +     2*s*x2 - s*x + 0;
	w.y = (2-s)*x3 +   (s-3)*x2       + 1;
	w.z = (s-2)*x3 + (3-2*s)*x2 + s*x + 0;
	w.w =     s*x3 -       s*x2       + 0;
	return w;
}

vec4 cubic(float v) {
	vec4 n = vec4(1.0, 2.0, 3.0, 4.0) - v;
	vec4 s = n * n * n;
	float x = s.x;
	float y = s.y - 4.0 * s.x;
	float z = s.z - 4.0 * s.y + 6.0 * s.x;
	float w = 6.0 - x - y - z;
	return vec4(x, y, z, w) * (1.0/6.0);
}

vec3 textureBicubic(sampler2D tex, vec2 tc, vec2 texStep) {
	// http://www.java-gaming.org/index.php?topic=35123.0
	vec2 texSize = 1.0 / texStep;
	tc = tc * texSize - 0.5;

	vec2 fxy = fract(tc);
	tc -= fxy;
	vec4 xcubic = cubic(fxy.x);
	vec4 ycubic = cubic(fxy.y);

	vec4 c = tc.xxyy + vec2(-0.5, 1.5).xyxy;
	vec4 s = vec4(xcubic.xz + xcubic.yw, ycubic.xz + ycubic.yw);
	vec4 offset = c + vec4 (xcubic.yw, ycubic.yw) / s;
	offset *= texStep.xxyy;
	vec3 sample0 = texture(tex, offset.xz).rgb;
	vec3 sample1 = texture(tex, offset.yz).rgb;
	vec3 sample2 = texture(tex, offset.xw).rgb;
	vec3 sample3 = texture(tex, offset.yw).rgb;

	float sx = s.x / (s.x + s.y);
	float sy = s.z / (s.z + s.w);

	return mix(mix(sample3, sample2, sx), mix(sample1, sample0, sx), sy);
}

vec4 textureSS(sampler2D tex, vec2 tc, vec2 texStep) {
	vec4 col = texture(tex, tc);
	col += texture(tex, tc + vec2(1.5, 0.0) * texStep);
	col += texture(tex, tc + vec2(-1.5, 0.0) * texStep);
	col += texture(tex, tc + vec2(0.0, 1.5) * texStep);
	col += texture(tex, tc + vec2(0.0, -1.5) * texStep);
	col += texture(tex, tc + vec2(1.5, 1.5) * texStep);
	col += texture(tex, tc + vec2(-1.5, -1.5) * texStep);
	col += texture(tex, tc + vec2(1.5, -1.5) * texStep);
	col += texture(tex, tc + vec2(-1.5, 1.5) * texStep);
	return col / 9.0;
}
