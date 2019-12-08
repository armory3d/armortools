package arm.node;

import arm.node.MaterialShader;

class MakeParticle {

	public static function run(data: MaterialShaderData): MaterialShaderContext {
		var context_id = "mesh";
		var con_part:MaterialShaderContext = data.add_context({
			name: context_id,
			depth_write: false,
			compare_mode: "always",
			cull_mode: "clockwise",
			vertex_elements: [{name: "pos", data: "short4norm"}]
		});

		var vert = con_part.make_vert();
		var frag = con_part.make_frag();
		frag.ins = vert.outs;

		vert.write_attrib('vec4 spos = vec4(pos.xyz, 1.0);');

		vert.add_uniform('float brushRadius', '_brushRadius');
		vert.write_attrib('vec3 emitFrom = vec3(fhash(gl_InstanceID), fhash(gl_InstanceID * 2), fhash(gl_InstanceID * 3));');
		vert.write_attrib('emitFrom = emitFrom * brushRadius - brushRadius / 2;');
		vert.write_attrib('spos.xyz += emitFrom * 256;');

		vert.add_uniform('mat4 pd', '_particleData');

		var str_tex_hash = "float fhash(float n) { return fract(sin(n) * 43758.5453); }\n";
		vert.add_function(str_tex_hash);
		vert.add_out('float p_age');
		vert.write('p_age = pd[3][3] - gl_InstanceID * pd[0][1];');
		vert.write('p_age -= p_age * fhash(gl_InstanceID) * pd[2][3];');

		vert.write('if (pd[0][0] > 0 && p_age < 0) p_age += (int(-p_age / pd[0][0]) + 1) * pd[0][0];');

		vert.add_out('float p_lifetime');
		vert.write('p_lifetime = pd[0][2];');
		vert.write('if (p_age < 0 || p_age > p_lifetime) {');
		// vert.write('SPIRV_Cross_Output stage_output;');
		// vert.write('stage_output.svpos /= 0.0;');
		// vert.write('return stage_output;');
		vert.write('spos /= 0.0;');
		vert.write('}');

		vert.add_out('vec3 p_velocity');
		vert.write('p_velocity = vec3(pd[1][0], pd[1][1], pd[1][2]);');
		vert.write('p_velocity.x += fhash(gl_InstanceID)                * pd[1][3] - pd[1][3] / 2;');
		vert.write('p_velocity.y += fhash(gl_InstanceID +     pd[0][3]) * pd[1][3] - pd[1][3] / 2;');
		vert.write('p_velocity.z += fhash(gl_InstanceID + 2 * pd[0][3]) * pd[1][3] - pd[1][3] / 2;');
		vert.write('p_velocity.x += (pd[2][0] * p_age) / 5;');
		vert.write('p_velocity.y += (pd[2][1] * p_age) / 5;');
		vert.write('p_velocity.z += (pd[2][2] * p_age) / 5;');

		vert.add_out('vec3 p_location');
		vert.write('p_location = p_velocity * p_age;');
		vert.write('spos.xyz += p_location;');

		vert.add_uniform('mat4 WVP', '_worldViewProjectionMatrix');
		vert.write('gl_Position = mul(spos, WVP);');

		vert.add_uniform('vec4 inp', '_inputBrush');
		vert.write('vec2 binp = vec2(inp.x, 1.0 - inp.y);');
		vert.write('binp = binp * 2.0 - 1.0;');
		vert.write('binp *= gl_Position.w;');
		vert.write('gl_Position.xy += binp;');

		vert.add_out('float p_fade');
		vert.write('p_fade = sin(min((p_age / 8) * 3.141592, 3.141592));');

		frag.add_out('float fragColor');
		frag.write('fragColor = p_fade;');

		// vert.add_out('vec4 wvpposition');
		// vert.write('wvpposition = gl_Position;');
		// frag.write('vec2 texCoord = wvpposition.xy / wvpposition.w;');
		// frag.add_uniform('sampler2D gbufferD');
		// frag.write('fragColor *= 1.0 - clamp(distance(textureLod(gbufferD, texCoord, 0).r, wvpposition.z), 0.0, 1.0);');

		// Material.finalize(con_part);
		con_part.data.shader_from_source = true;
		con_part.data.vertex_shader = vert.get();
		con_part.data.fragment_shader = frag.get();

		return con_part;
	}
}
