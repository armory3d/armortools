// Exclusive to bloom for now
#version 450

const float bloomStrength = 3.5;
const float bloomRadius = 3.0;

uniform sampler2D tex;
uniform vec2 dir;
uniform vec2 screenSize;

in vec2 texCoord;
out vec4 fragColor;

const float weight[10] = float[] (0.132572, 0.125472, 0.106373, 0.08078, 0.05495, 0.033482, 0.018275, 0.008934, 0.003912, 0.001535);

void main() {
	vec2 step = (dir / screenSize.xy) * bloomRadius;
	fragColor.rgb = textureLod(tex, texCoord, 0.0).rgb * weight[0];
	for (int i = 1; i < 10; i++) {
		vec2 s = step * (float(i) + 0.5);
		fragColor.rgb += textureLod(tex, texCoord + s, 0.0).rgb * weight[i];
		fragColor.rgb += textureLod(tex, texCoord - s, 0.0).rgb * weight[i];
	}

	fragColor.rgb *= bloomStrength / 5;
	fragColor.rgb = min(fragColor.rgb, 64.0);
}
