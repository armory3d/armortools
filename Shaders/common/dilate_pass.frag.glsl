#version 450

uniform sampler2D tex;
uniform float dilateRadius;
in vec2 texCoord;
out vec4 fragColor;

void main() {
	// Based on https://shaderbits.com/blog/uv-dilation by Ryan Brucks
	float texelSize = 1.0 / textureSize(tex, 0).x;
	float minDist = 10000000;
	vec2 offsets[8] = {
		vec2(-1, 0),
		vec2( 1, 0),
		vec2( 0, 1),
		vec2( 0,-1),
		vec2(-1, 1),
		vec2( 1, 1),
		vec2( 1,-1),
		vec2(-1,-1)
	};

	vec3 col = textureLod(tex, texCoord, 0.0).rgb;
	vec3 minCol = col;
	if (col.x == 0 && col.y == 0 && col.z == 0) {
		int i = 0;
		while (i < dilateRadius) {
			i++;
			int j = 0;
			while (j < 8) {
				vec2 curUV = texCoord + offsets[j] * texelSize * i;
				vec3 offsetCol = textureLod(tex, curUV, 0.0).rgb;

				if (offsetCol.x != 0 || offsetCol.y != 0 || offsetCol.z != 0) {
					float curDist = length(texCoord - curUV);

					if (curDist < minDist) {
						vec2 projectUV = curUV + offsets[j] * texelSize * i * 0.25;
						vec3 direction = textureLod(tex, projectUV, 0.0).rgb;
						minDist = curDist;

						if (direction.x != 0 || direction.y != 0 || direction.z != 0) {
							vec3 delta = offsetCol - direction;
							minCol = offsetCol + delta * 4;
						}
						else {
							minCol = offsetCol;
						}
					}
				}
				j++;
			}
		}
	}
	fragColor = vec4(minCol, 1.0);
}
