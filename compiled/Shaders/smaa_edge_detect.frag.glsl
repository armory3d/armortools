/**
 * Copyright (C) 2013 Jorge Jimenez (jorge@iryoku.com)
 * Copyright (C) 2013 Jose I. Echevarria (joseignacioechevarria@gmail.com)
 * Copyright (C) 2013 Belen Masia (bmasia@unizar.es)
 * Copyright (C) 2013 Fernando Navarro (fernandn@microsoft.com)
 * Copyright (C) 2013 Diego Gutierrez (diegog@unizar.es)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to
 * do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software. As clarification, there
 * is no requirement that the copyright notice and permission be included in
 * binary distributions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 *                  _______  ___  ___       ___           ___
 *                 /       ||   \/   |     /   \         /   \
 *                |   (---- |  \  /  |    /  ^  \       /  ^  \
 *                 \   \    |  |\/|  |   /  /_\  \     /  /_\  \
 *              ----)   |   |  |  |  |  /  _____  \   /  _____  \
 *             |_______/    |__|  |__| /__/     \__\ /__/     \__\
 * 
 *                               E N H A N C E D
 *       S U B P I X E L   M O R P H O L O G I C A L   A N T I A L I A S I N G
 *
 *                         http://www.iryoku.com/smaa/
 */
#version 450

#define SMAA_THRESHOLD 0.1
#define SMAA_DEPTH_THRESHOLD (0.1 * SMAA_THRESHOLD) // For depth edge detection, depends on the depth range of the scene
#define SMAA_LOCAL_CONTRAST_ADAPTATION_FACTOR 2.0

uniform sampler2D colorTex;

in vec2 texCoord;
in vec4 offset0;
in vec4 offset1;
in vec4 offset2;
out vec4 fragColor;

// Misc functions
// Gathers current pixel, and the top-left neighbors.
// vec3 SMAAGatherNeighbours(vec2 texcoord/*, vec4 offset[3], sampler2D tex*/) {
	// float P = textureLod(tex, texcoord, 0.0).r;
	// float Pleft = textureLod(tex, offset0.xy, 0.0).r;
	// float Ptop  = textureLod(tex, offset0.zw, 0.0).r;
	// return vec3(P, Pleft, Ptop);
// }

// Edge Detection Pixel Shaders (First Pass)
// Adjusts the threshold by means of predication.
// vec2 SMAACalculatePredicatedThreshold(vec2 texcoord, vec4 offset[3], sampler2D predicationTex) {
//     vec3 neighbours = SMAAGatherNeighbours(texcoord, offset, predicationTex);
//     vec2 delta = abs(neighbours.xx - neighbours.yz);
//     vec2 edges = step(SMAA_PREDICATION_THRESHOLD, delta);
//     return SMAA_PREDICATION_SCALE * SMAA_THRESHOLD * (1.0 - SMAA_PREDICATION_STRENGTH * edges);
// }

// Luma Edge Detection
// IMPORTANT NOTICE: luma edge detection requires gamma-corrected colors, and
// thus 'colorTex' should be a non-sRGB texture.
vec2 SMAALumaEdgeDetectionPS(vec2 texcoord
							   //#if SMAA_PREDICATION
							   //, sampler2D predicationTex
							   //#endif
							   ) {
	// Calculate the threshold:
	//#if SMAA_PREDICATION
	//vec2 threshold = SMAACalculatePredicatedThreshold(texcoord, offset, SMAATexturePass2D(predicationTex));
	//#else
	vec2 threshold = vec2(SMAA_THRESHOLD, SMAA_THRESHOLD);
	//#endif

	// Calculate lumas:
	vec3 weights = vec3(0.2126, 0.7152, 0.0722);
	float L = dot(textureLod(colorTex, texcoord, 0.0).rgb, weights);

	float Lleft = dot(textureLod(colorTex, offset0.xy, 0.0).rgb, weights);
	float Ltop  = dot(textureLod(colorTex, offset0.zw, 0.0).rgb, weights);

	// We do the usual threshold:
	vec4 delta;
	delta.xy = abs(L - vec2(Lleft, Ltop));
	vec2 edges = step(threshold, delta.xy);

	// Then discard if there is no edge:
	if (dot(edges, vec2(1.0, 1.0)) == 0.0)
		discard;

	// Calculate right and bottom deltas:
	float Lright = dot(textureLod(colorTex, offset1.xy, 0.0).rgb, weights);
	float Lbottom  = dot(textureLod(colorTex, offset1.zw, 0.0).rgb, weights);
	delta.zw = abs(L - vec2(Lright, Lbottom));

	// Calculate the maximum delta in the direct neighborhood:
	vec2 maxDelta = max(delta.xy, delta.zw);

	// Calculate left-left and top-top deltas:
	float Lleftleft = dot(textureLod(colorTex, offset2.xy, 0.0).rgb, weights);
	float Ltoptop = dot(textureLod(colorTex, offset2.zw, 0.0).rgb, weights);
	delta.zw = abs(vec2(Lleft, Ltop) - vec2(Lleftleft, Ltoptop));

	// Calculate the final maximum delta:
	maxDelta = max(maxDelta.xy, delta.zw);
	float finalDelta = max(maxDelta.x, maxDelta.y);

	// Local contrast adaptation:
	edges.xy *= step(finalDelta, SMAA_LOCAL_CONTRAST_ADAPTATION_FACTOR * delta.xy);

	return edges;
}

// Color Edge Detection
// IMPORTANT NOTICE: color edge detection requires gamma-corrected colors, and
// thus 'colorTex' should be a non-sRGB texture.
vec2 SMAAColorEdgeDetectionPS(vec2 texcoord
								//#if SMAA_PREDICATION
								//, sampler2D predicationTex
								//#endif
								) {
	// Calculate the threshold:
	//#if SMAA_PREDICATION
	//vec2 threshold = SMAACalculatePredicatedThreshold(texcoord, offset, predicationTex);
	//#else
	vec2 threshold = vec2(SMAA_THRESHOLD, SMAA_THRESHOLD);
	//#endif

	// Calculate color deltas:
	vec4 delta;
	vec3 C = textureLod(colorTex, texcoord, 0.0).rgb;

	vec3 Cleft = textureLod(colorTex, offset0.xy, 0.0).rgb;
	vec3 t = abs(C - Cleft);
	delta.x = max(max(t.r, t.g), t.b);

	vec3 Ctop  = textureLod(colorTex, offset0.zw, 0.0).rgb;
	t = abs(C - Ctop);
	delta.y = max(max(t.r, t.g), t.b);

	// We do the usual threshold:
	vec2 edges = step(threshold, delta.xy);

	// Then discard if there is no edge:
	if (dot(edges, vec2(1.0, 1.0)) == 0.0)
		discard;

	// Calculate right and bottom deltas:
	vec3 Cright = textureLod(colorTex, offset1.xy, 0.0).rgb;
	t = abs(C - Cright);
	delta.z = max(max(t.r, t.g), t.b);

	vec3 Cbottom  = textureLod(colorTex, offset1.zw, 0.0).rgb;
	t = abs(C - Cbottom);
	delta.w = max(max(t.r, t.g), t.b);

	// Calculate the maximum delta in the direct neighborhood:
	vec2 maxDelta = max(delta.xy, delta.zw);

	// Calculate left-left and top-top deltas:
	vec3 Cleftleft  = textureLod(colorTex, offset2.xy, 0.0).rgb;
	t = abs(C - Cleftleft);
	delta.z = max(max(t.r, t.g), t.b);

	vec3 Ctoptop = textureLod(colorTex, offset2.zw, 0.0).rgb;
	t = abs(C - Ctoptop);
	delta.w = max(max(t.r, t.g), t.b);

	// Calculate the final maximum delta:
	maxDelta = max(maxDelta.xy, delta.zw);
	float finalDelta = max(maxDelta.x, maxDelta.y);

	// Local contrast adaptation:
	edges.xy *= step(finalDelta, SMAA_LOCAL_CONTRAST_ADAPTATION_FACTOR * delta.xy);

	return edges;
}

// Depth Edge Detection
// vec2 SMAADepthEdgeDetectionPS(vec2 texcoord, /*vec4 offset[3],*/ sampler2D depthTex) {
	// vec3 neighbours = SMAAGatherNeighbours(texcoord, /*offset,*/ depthTex);
	// vec2 delta = abs(neighbours.xx - vec2(neighbours.y, neighbours.z));
	// vec2 edges = step(SMAA_DEPTH_THRESHOLD, delta);

	// if (dot(edges, vec2(1.0, 1.0)) == 0.0)
		// discard;

	// return edges;
// }

void main() {
	fragColor.rg = SMAAColorEdgeDetectionPS(texCoord);
}
