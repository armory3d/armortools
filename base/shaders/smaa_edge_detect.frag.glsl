#version 450

// Copyright (C) 2013 Jorge Jimenez (jorge@iryoku.com)
// Copyright (C) 2013 Jose I. Echevarria (joseignacioechevarria@gmail.com)
// Copyright (C) 2013 Belen Masia (bmasia@unizar.es)
// Copyright (C) 2013 Fernando Navarro (fernandn@microsoft.com)
// Copyright (C) 2013 Diego Gutierrez (diegog@unizar.es)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to
// do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software. As clarification, there
// is no requirement that the copyright notice and permission be included in
// binary distributions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// http://www.iryoku.com/smaa/

#define SMAA_THRESHOLD 0.1
#define SMAA_DEPTH_THRESHOLD (0.1 * SMAA_THRESHOLD)
#define SMAA_LOCAL_CONTRAST_ADAPTATION_FACTOR 2.0

uniform sampler2D color_tex;

in vec2 tex_coord;
in vec4 offset0;
in vec4 offset1;
in vec4 offset2;
out vec4 frag_color;

vec2 smaa_luma_edge_detection_ps(vec2 texcoord) {
	vec2 threshold = vec2(SMAA_THRESHOLD, SMAA_THRESHOLD);

	vec3 weights = vec3(0.2126, 0.7152, 0.0722);
	float L = dot(textureLod(color_tex, texcoord, 0.0).rgb, weights);

	float Lleft = dot(textureLod(color_tex, offset0.xy, 0.0).rgb, weights);
	float Ltop  = dot(textureLod(color_tex, offset0.zw, 0.0).rgb, weights);

	vec4 delta;
	delta.xy = abs(L - vec2(Lleft, Ltop));
	vec2 edges = step(threshold, delta.xy);

	if (dot(edges, vec2(1.0, 1.0)) == 0.0)
		discard;

	float Lright = dot(textureLod(color_tex, offset1.xy, 0.0).rgb, weights);
	float Lbottom  = dot(textureLod(color_tex, offset1.zw, 0.0).rgb, weights);
	delta.zw = abs(L - vec2(Lright, Lbottom));

	vec2 max_delta = max(delta.xy, delta.zw);

	float Lleftleft = dot(textureLod(color_tex, offset2.xy, 0.0).rgb, weights);
	float Ltoptop = dot(textureLod(color_tex, offset2.zw, 0.0).rgb, weights);
	delta.zw = abs(vec2(Lleft, Ltop) - vec2(Lleftleft, Ltoptop));

	max_delta = max(max_delta.xy, delta.zw);
	float final_delta = max(max_delta.x, max_delta.y);

	edges.xy *= step(final_delta, SMAA_LOCAL_CONTRAST_ADAPTATION_FACTOR * delta.xy);

	return edges;
}

vec2 smaa_color_edge_detection_ps(vec2 texcoord) {
	vec2 threshold = vec2(SMAA_THRESHOLD, SMAA_THRESHOLD);

	vec4 delta;
	vec3 C = textureLod(color_tex, texcoord, 0.0).rgb;

	vec3 Cleft = textureLod(color_tex, offset0.xy, 0.0).rgb;
	vec3 t = abs(C - Cleft);
	delta.x = max(max(t.r, t.g), t.b);

	vec3 Ctop  = textureLod(color_tex, offset0.zw, 0.0).rgb;
	t = abs(C - Ctop);
	delta.y = max(max(t.r, t.g), t.b);

	vec2 edges = step(threshold, delta.xy);

	if (dot(edges, vec2(1.0, 1.0)) == 0.0)
		discard;

	vec3 Cright = textureLod(color_tex, offset1.xy, 0.0).rgb;
	t = abs(C - Cright);
	delta.z = max(max(t.r, t.g), t.b);

	vec3 Cbottom  = textureLod(color_tex, offset1.zw, 0.0).rgb;
	t = abs(C - Cbottom);
	delta.w = max(max(t.r, t.g), t.b);

	vec2 max_delta = max(delta.xy, delta.zw);

	vec3 Cleftleft  = textureLod(color_tex, offset2.xy, 0.0).rgb;
	t = abs(C - Cleftleft);
	delta.z = max(max(t.r, t.g), t.b);

	vec3 Ctoptop = textureLod(color_tex, offset2.zw, 0.0).rgb;
	t = abs(C - Ctoptop);
	delta.w = max(max(t.r, t.g), t.b);

	max_delta = max(max_delta.xy, delta.zw);
	float final_delta = max(max_delta.x, max_delta.y);

	edges.xy *= step(final_delta, SMAA_LOCAL_CONTRAST_ADAPTATION_FACTOR * delta.xy);

	return edges;
}

void main() {
	frag_color.rg = smaa_color_edge_detection_ps(tex_coord);
}
