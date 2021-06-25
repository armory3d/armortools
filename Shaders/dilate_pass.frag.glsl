#version 450

uniform sampler2D tex;
uniform sampler2D texdilate;
uniform float dilateRadius;
in vec2 texCoord;
out vec4 fragColor;

const vec2 offsets[8] = vec2[] (
	vec2(-1, 0),
	vec2( 1, 0),
	vec2( 0, 1),
	vec2( 0,-1),
	vec2(-1, 1),
	vec2( 1, 1),
	vec2( 1,-1),
	vec2(-1,-1)
);

void main() {
	// Based on https://shaderbits.com/blog/uv-dilation by Ryan Brucks
	vec2 size = textureSize(tex, 0).xy;
	vec2 texelSize = 1.0 / size;
	float minDist = 10000000;
	ivec2 coord = ivec2(texCoord * size);
	float mask = texelFetch(texdilate, coord, 0).r;
	if (mask > 0) discard;

	fragColor = texelFetch(tex, coord, 0);
	int i = 0;
	while (i < dilateRadius) {
		i++;
		int j = 0;
		while (j < 8) {
			vec2 curUV = texCoord + offsets[j] * texelSize * i;
			coord = ivec2(curUV * size);
			float offsetMask = texelFetch(texdilate, coord, 0).r;
			vec4 offsetCol = texelFetch(tex, coord, 0);

			if (offsetMask != 0) {
				float curDist = length(texCoord - curUV);
				if (curDist < minDist) {
					vec2 projectUV = curUV + offsets[j] * texelSize * i * 0.25;
					vec4 direction = textureLod(tex, projectUV, 0.0);
					minDist = curDist;
					if (direction.x != 0 || direction.y != 0 || direction.z != 0) {
						vec4 delta = offsetCol - direction;
						fragColor = offsetCol + delta * 4;
					}
					else {
						fragColor = offsetCol;
					}
				}
			}
			j++;
		}
	}
}
