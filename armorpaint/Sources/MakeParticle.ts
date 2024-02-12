
class MakeParticle {

	static run = (data: TMaterial): NodeShaderContextRaw => {
		let context_id = "mesh";
		let con_part = NodeShaderContext.create(data, {
			name: context_id,
			depth_write: false,
			compare_mode: "always",
			cull_mode: "clockwise",
			vertex_elements: [{name: "pos", data: "short4norm"}],
			color_attachments: ["R8"]
		});

		let vert = NodeShaderContext.make_vert(con_part);
		let frag = NodeShaderContext.make_frag(con_part);
		frag.ins = vert.outs;

		NodeShader.write_attrib(vert, 'vec4 spos = vec4(pos.xyz, 1.0);');

		NodeShader.add_uniform(vert, 'float brushRadius', '_brushRadius');
		NodeShader.write_attrib(vert, 'vec3 emitFrom = vec3(fhash(gl_InstanceID), fhash(gl_InstanceID * 2), fhash(gl_InstanceID * 3));');
		NodeShader.write_attrib(vert, 'emitFrom = emitFrom * brushRadius - brushRadius / 2.0;');
		NodeShader.write_attrib(vert, 'spos.xyz += emitFrom * vec3(256.0, 256.0, 256.0);');

		NodeShader.add_uniform(vert, 'mat4 pd', '_particle_data');

		let str_tex_hash = "float fhash(int n) { return fract(sin(float(n)) * 43758.5453); }\n";
		NodeShader.add_function(vert, str_tex_hash);
		NodeShader.add_out(vert, 'float p_age');
		NodeShader.write(vert, 'p_age = pd[3][3] - float(gl_InstanceID) * pd[0][1];');
		NodeShader.write(vert, 'p_age -= p_age * fhash(gl_InstanceID) * pd[2][3];');

		NodeShader.write(vert, 'if (pd[0][0] > 0.0 && p_age < 0.0) p_age += float(int(-p_age / pd[0][0]) + 1) * pd[0][0];');

		NodeShader.add_out(vert, 'float p_lifetime');
		NodeShader.write(vert, 'p_lifetime = pd[0][2];');
		NodeShader.write(vert, 'if (p_age < 0.0 || p_age > p_lifetime) {');
		// NodeShader.write(vert, 'SPIRV_Cross_Output stage_output;');
		// NodeShader.write(vert, 'stage_output.svpos /= 0.0;');
		// NodeShader.write(vert, 'return stage_output;');
		NodeShader.write(vert, 'spos /= 0.0;');
		NodeShader.write(vert, '}');

		NodeShader.add_out(vert, 'vec3 p_velocity');
		NodeShader.write(vert, 'p_velocity = vec3(pd[1][0], pd[1][1], pd[1][2]);');
		NodeShader.write(vert, 'p_velocity.x += fhash(gl_InstanceID)                     * pd[1][3] - pd[1][3] / 2.0;');
		NodeShader.write(vert, 'p_velocity.y += fhash(gl_InstanceID +     int(pd[0][3])) * pd[1][3] - pd[1][3] / 2.0;');
		NodeShader.write(vert, 'p_velocity.z += fhash(gl_InstanceID + 2 * int(pd[0][3])) * pd[1][3] - pd[1][3] / 2.0;');
		NodeShader.write(vert, 'p_velocity.x += (pd[2][0] * p_age) / 5.0;');
		NodeShader.write(vert, 'p_velocity.y += (pd[2][1] * p_age) / 5.0;');
		NodeShader.write(vert, 'p_velocity.z += (pd[2][2] * p_age) / 5.0;');

		NodeShader.add_out(vert, 'vec3 p_location');
		NodeShader.write(vert, 'p_location = p_velocity * p_age;');
		NodeShader.write(vert, 'spos.xyz += p_location;');
		NodeShader.write(vert, 'spos.xyz *= vec3(0.01, 0.01, 0.01);');

		NodeShader.add_uniform(vert, 'mat4 WVP', '_world_view_proj_matrix');
		NodeShader.write(vert, 'gl_Position = mul(spos, WVP);');

		NodeShader.add_uniform(vert, 'vec4 inp', '_inputBrush');
		NodeShader.write(vert, 'vec2 binp = vec2(inp.x, 1.0 - inp.y);');
		NodeShader.write(vert, 'binp = binp * 2.0 - 1.0;');
		NodeShader.write(vert, 'binp *= gl_Position.w;');
		NodeShader.write(vert, 'gl_Position.xy += binp;');

		NodeShader.add_out(vert, 'float p_fade');
		NodeShader.write(vert, 'p_fade = sin(min((p_age / 8.0) * 3.141592, 3.141592));');

		NodeShader.add_out(frag, 'float fragColor');
		NodeShader.write(frag, 'fragColor = p_fade;');

		// NodeShader.add_out(vert, 'vec4 wvpposition');
		// NodeShader.write(vert, 'wvpposition = gl_Position;');
		// NodeShader.write(frag, 'vec2 texCoord = wvpposition.xy / wvpposition.w;');
		// NodeShader.add_uniform(frag, 'sampler2D gbufferD');
		// NodeShader.write(frag, 'fragColor *= 1.0 - clamp(distance(textureLod(gbufferD, texCoord, 0.0).r, wvpposition.z), 0.0, 1.0);');

		// Material.finalize(con_part);
		con_part.data.shader_from_source = true;
		con_part.data.vertex_shader = NodeShader.get(vert);
		con_part.data.fragment_shader = NodeShader.get(frag);

		return con_part;
	}

	static mask = (vert: NodeShaderRaw, frag: NodeShaderRaw) => {
		///if arm_physics
		if (Context.raw.particlePhysics) {
			NodeShader.add_out(vert, 'vec4 wpos');
			NodeShader.add_uniform(vert, 'mat4 W', '_world_matrix');
			NodeShader.write_attrib(vert, 'wpos = mul(vec4(pos.xyz, 1.0), W);');
			NodeShader.add_uniform(frag, 'vec3 particleHit', '_particleHit');
			NodeShader.add_uniform(frag, 'vec3 particleHitLast', '_particleHitLast');

			NodeShader.write(frag, 'vec3 pa = wpos.xyz - particleHit;');
			NodeShader.write(frag, 'vec3 ba = particleHitLast - particleHit;');
			NodeShader.write(frag, 'float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);');
			NodeShader.write(frag, 'dist = length(pa - ba * h) * 10.0;');
			// NodeShader.write(frag, 'dist = distance(particleHit, wpos.xyz) * 10.0;');

			NodeShader.write(frag, 'if (dist > 1.0) discard;');
			NodeShader.write(frag, 'float str = clamp(pow(1.0 / dist * brushHardness * 0.2, 4.0), 0.0, 1.0) * opacity;');
			NodeShader.write(frag, 'if (particleHit.x == 0.0 && particleHit.y == 0.0 && particleHit.z == 0.0) str = 0.0;');
			NodeShader.write(frag, 'if (str == 0.0) discard;');
			return;
		}
		///end

		NodeShader.add_uniform(frag, 'sampler2D texparticle', '_texparticle');
		///if (krom_direct3d11 || krom_direct3d12 || krom_metal || krom_vulkan)
		NodeShader.write(frag, 'float str = textureLod(texparticle, sp.xy, 0.0).r;');
		///else
		NodeShader.write(frag, 'float str = textureLod(texparticle, vec2(sp.x, (1.0 - sp.y)), 0.0).r;');
		///end
	}
}
