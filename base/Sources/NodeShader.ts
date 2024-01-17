
class NodeShader {

	context: NodeShaderContext;
	shader_type = '';
	includes: string[] = [];
	ins: string[] = [];
	outs: string[] = [];
	sharedSamplers: string[] = [];
	uniforms: string[] = [];
	functions = new Map<string, string>();
	main = '';
	main_init = '';
	main_end = '';
	main_normal = '';
	main_textures = '';
	main_attribs = '';
	header = '';
	write_pre = false;
	write_normal = 0;
	write_textures = 0;
	vstruct_as_vsin = true;
	lock = false;

	// References
	bposition = false;
	wposition = false;
	mposition = false;
	vposition = false;
	wvpposition = false;
	ndcpos = false;
	wtangent = false;
	vVec = false;
	vVecCam = false;
	n = false;
	nAttr = false;
	dotNV = false;
	invTBN = false;

	constructor(context: NodeShaderContext, shader_type: string) {
		this.context = context;
		this.shader_type = shader_type;
	}

	add_include = (s: string) => {
		this.includes.push(s);
	}

	add_in = (s: string) => {
		this.ins.push(s);
	}

	add_out = (s: string) => {
		this.outs.push(s);
	}

	add_uniform = (s: string, link: string = null, included = false) => {
		let ar = s.split(' ');
		// layout(RGBA8) image3D voxels
		let utype = ar[ar.length - 2];
		let uname = ar[ar.length - 1];
		if (utype.startsWith('sampler') || utype.startsWith('image') || utype.startsWith('uimage')) {
			let is_image = (utype.startsWith('image') || utype.startsWith('uimage')) ? true : false;
			this.context.add_texture_unit(utype, uname, link, is_image);
		}
		else {
			// Prefer vec4[] for d3d to avoid padding
			if (ar[0] == 'float' && ar[1].indexOf('[') >= 0) {
				ar[0] = 'floats';
				ar[1] = ar[1].split('[')[0];
			}
			else if (ar[0] == 'vec4' && ar[1].indexOf('[') >= 0) {
				ar[0] = 'floats';
				ar[1] = ar[1].split('[')[0];
			}
			this.context.add_constant(ar[0], ar[1], link);
		}
		if (included == false && this.uniforms.indexOf(s) == -1) {
			this.uniforms.push(s);
		}
	}

	add_shared_sampler = (s: string) => {
		if (this.sharedSamplers.indexOf(s) == -1) {
			this.sharedSamplers.push(s);
			let ar = s.split(' ');
			// layout(RGBA8) sampler2D tex
			let utype = ar[ar.length - 2];
			let uname = ar[ar.length - 1];
			this.context.add_texture_unit(utype, uname, null, false);
		}
	}

	add_function = (s: string) => {
		let fname = s.split('(')[0];
		if (this.functions.has(fname)) return;
		this.functions.set(fname, s);
	}

	contains = (s: string): bool => {
		return this.main.indexOf(s) >= 0 ||
			   this.main_init.indexOf(s) >= 0 ||
			   this.main_normal.indexOf(s) >= 0 ||
			   this.ins.indexOf(s) >= 0 ||
			   this.main_textures.indexOf(s) >= 0 ||
			   this.main_attribs.indexOf(s) >= 0;
	}

	write_init = (s: string) => {
		this.main_init = s + '\n' + this.main_init;
	}

	write = (s: string) => {
		if (this.lock) return;
		if (this.write_textures > 0) {
			this.main_textures += s + '\n';
		}
		else if (this.write_normal > 0) {
			this.main_normal += s + '\n';
		}
		else if (this.write_pre) {
			this.main_init += s + '\n';
		}
		else {
			this.main += s + '\n';
		}
	}

	write_header = (s: string) => {
		this.header += s + '\n';
	}

	write_end = (s: string) => {
		this.main_end += s + '\n';
	}

	write_attrib = (s: string) => {
		this.main_attribs += s + '\n';
	}

	dataSize = (data: string): string => {
		if (data == 'float1') return '1';
		else if (data == 'float2') return '2';
		else if (data == 'float3') return '3';
		else if (data == 'float4') return '4';
		else if (data == 'short2norm') return '2';
		else if (data == 'short4norm') return '4';
		else return '1';
	}

	vstruct_to_vsin = () => {
		// if self.shader_type != 'vert' or self.ins != [] or not self.vstruct_as_vsin: # Vertex structure as vertex shader input
			// return
		let vs = this.context.data.vertex_elements;
		for (let e of vs) {
			this.add_in('vec' + this.dataSize(e.data) + ' ' + e.name);
		}
	}

	get = (): string => {

		if (this.shader_type == 'vert' && this.vstruct_as_vsin) {
			this.vstruct_to_vsin();
		}

		let sharedSampler = 'shared_sampler';
		if (this.sharedSamplers.length > 0) {
			sharedSampler = this.sharedSamplers[0].split(' ')[1] + '_sampler';
		}

		///if (krom_direct3d11 || krom_direct3d12)
		let s = '#define HLSL\n';
		s += '#define textureArg(tex) Texture2D tex,SamplerState tex ## _sampler\n';
		s += '#define texturePass(tex) tex,tex ## _sampler\n';
		s += '#define sampler2D Texture2D\n';
		s += '#define sampler3D Texture3D\n';
		s += '#define texture(tex, coord) tex.Sample(tex ## _sampler, coord)\n';
		s += `#define textureShared(tex, coord) tex.Sample(${sharedSampler}, coord)\n`;
		s += '#define textureLod(tex, coord, lod) tex.SampleLevel(tex ## _sampler, coord, lod)\n';
		s += `#define textureLodShared(tex, coord, lod) tex.SampleLevel(${sharedSampler}, coord, lod)\n`;
		s += '#define texelFetch(tex, coord, lod) tex.Load(float3(coord.xy, lod))\n';
		s += 'uint2 _GetDimensions(Texture2D tex, uint lod) { uint x, y; tex.GetDimensions(x, y); return uint2(x, y); }\n';
		s += '#define textureSize _GetDimensions\n';
		s += '#define mod(a, b) (a % b)\n';
		s += '#define vec2 float2\n';
		s += '#define vec3 float3\n';
		s += '#define vec4 float4\n';
		s += '#define ivec2 int2\n';
		s += '#define ivec3 int3\n';
		s += '#define ivec4 int4\n';
		s += '#define mat2 float2x2\n';
		s += '#define mat3 float3x3\n';
		s += '#define mat4 float4x4\n';
		s += '#define dFdx ddx\n';
		s += '#define dFdy ddy\n';
		s += '#define inversesqrt rsqrt\n';
		s += '#define fract frac\n';
		s += '#define mix lerp\n';
		// s += '#define fma mad\n';

		s += this.header;

		let in_ext = '';
		let out_ext = '';

		for (let a of this.includes) {
			s += '#include "' + a + '"\n';
		}

		// Input structure
		let index = 0;
		if (this.ins.length > 0) {
			s += 'struct SPIRV_Cross_Input {\n';
			index = 0;
			this.ins.sort((a, b): i32 => {
				// Sort inputs by name
				return a.substring(4) >= b.substring(4) ? 1 : -1;
			});
			for (let a of this.ins) {
				s += `${a}${in_ext} : TEXCOORD${index};\n`;
				index++;
			}
			// Built-ins
			if (this.shader_type == 'vert' && this.main.indexOf("gl_VertexID") >= 0) {
				s += 'uint gl_VertexID : SV_VertexID;\n';
				this.ins.push('uint gl_VertexID');
			}
			if (this.shader_type == 'vert' && this.main.indexOf("gl_InstanceID") >= 0) {
				s += 'uint gl_InstanceID : SV_InstanceID;\n';
				this.ins.push('uint gl_InstanceID');
			}
			s += '};\n';
		}

		// Output structure
		let num = 0;
		if (this.outs.length > 0 || this.shader_type == 'vert') {
			s += 'struct SPIRV_Cross_Output {\n';
			this.outs.sort((a, b): i32 => {
				// Sort outputs by name
				return a.substring(4) >= b.substring(4) ? 1 : -1;
			});
			index = 0;
			if (this.shader_type == 'vert') {
				for (let a of this.outs) {
					s += `${a}${out_ext} : TEXCOORD${index};\n`;
					index++;
				}
				s += 'float4 svpos : SV_POSITION;\n';
			}
			else {
				let out = this.outs[0];
				// Multiple render targets
				if (out.charAt(out.length - 1) == ']') {
					num = parseInt(out.charAt(out.length - 2));
					s += `vec4 fragColor[${num}] : SV_TARGET0;\n`;
				}
				else {
					s += 'vec4 fragColor : SV_TARGET0;\n';
				}
			}
			s += '};\n';
		}

		for (let a of this.uniforms) {
			s += 'uniform ' + a + ';\n';
			if (a.startsWith('sampler')) {
				s += 'SamplerState ' + a.split(' ')[1] + '_sampler;\n';
			}
		}

		if (this.sharedSamplers.length > 0) {
			for (let a of this.sharedSamplers) {
				s += 'uniform ' + a + ';\n';
			}
			s += `SamplerState ${sharedSampler};\n`;
		}

		for (let f of this.functions.values()) {
			s += f + '\n';
		}

		// Begin main
		if (this.outs.length > 0 || this.shader_type == 'vert') {
			if (this.ins.length > 0) {
				s += 'SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input) {\n';
			}
			else {
				s += 'SPIRV_Cross_Output main() {\n';
			}
		}
		else {
			if (this.ins.length > 0) {
				s += 'void main(SPIRV_Cross_Input stage_input) {\n';
			}
			else {
				s += 'void main() {\n';
			}
		}

		// Declare inputs
		for (let a of this.ins) {
			let b = a.substring(5); // Remove type 'vec4 '
			s += `${a} = stage_input.${b};\n`;
		}

		if (this.shader_type == 'vert') {
			s += 'vec4 gl_Position;\n';
			for (let a of this.outs) {
				s += `${a};\n`;
			}
		}
		else {
			if (this.outs.length > 0) {
				if (num > 0) s += `vec4 fragColor[${num}];\n`;
				else s += 'vec4 fragColor;\n';
			}
		}

		s += this.main_attribs;
		s += this.main_textures;
		s += this.main_normal;
		s += this.main_init;
		s += this.main;
		s += this.main_end;

		// Write output structure
		if (this.outs.length > 0 || this.shader_type == 'vert') {
			s += 'SPIRV_Cross_Output stage_output;\n';
			if (this.shader_type == 'vert') {
				s += 'gl_Position.z = (gl_Position.z + gl_Position.w) * 0.5;\n';
				s += 'stage_output.svpos = gl_Position;\n';
				for (let a of this.outs) {
					let b = a.substring(5); // Remove type 'vec4 '
					s += `stage_output.${b} = ${b};\n`;
				}
			}
			else {
				if (num > 0) {
					for (let i = 0; i < num; ++i) {
						s += `stage_output.fragColor[${i}] = fragColor[${i}];\n`;
					}
				}
				else {
					s += 'stage_output.fragColor = fragColor;\n';
				}
			}
			s += 'return stage_output;\n';
		}
		s += '}\n';

		///elseif krom_metal

		let s = '#define METAL\n';
		s += '#include <metal_stdlib>\n';
		s += '#include <simd/simd.h>\n';
		s += 'using namespace metal;\n';

		s += '#define textureArg(tex) texture2d<float> tex,sampler tex ## _sampler\n';
		s += '#define texturePass(tex) tex,tex ## _sampler\n';
		s += '#define sampler2D texture2d<float>\n';
		s += '#define sampler3D texture3d<float>\n';
		s += '#define texture(tex, coord) tex.sample(tex ## _sampler, coord)\n';
		s += `#define textureShared(tex, coord) tex.sample(${sharedSampler}, coord)\n`;
		s += '#define textureLod(tex, coord, lod) tex.sample(tex ## _sampler, coord, level(lod))\n';
		s += `#define textureLodShared(tex, coord, lod) tex.sample(${sharedSampler}, coord, level(lod))\n`;
		s += '#define texelFetch(tex, coord, lod) tex.read(uint2(coord), uint(lod))\n';
		s += 'float2 _getDimensions(texture2d<float> tex, uint lod) { return float2(tex.get_width(lod), tex.get_height(lod)); }\n';
		s += '#define textureSize _getDimensions\n';
		s += '#define mod(a, b) fmod(a, b)\n';
		s += '#define vec2 float2\n';
		s += '#define vec3 float3\n';
		s += '#define vec4 float4\n';
		s += '#define ivec2 int2\n';
		s += '#define ivec3 int3\n';
		s += '#define ivec4 int4\n';
		s += '#define mat2 float2x2\n';
		s += '#define mat3 float3x3\n';
		s += '#define mat4 float4x4\n';
		s += '#define dFdx dfdx\n';
		s += '#define dFdy dfdy\n';
		s += '#define inversesqrt rsqrt\n';
		s += '#define mul(a, b) b * a\n';
		s += '#define discard discard_fragment()\n';

		for (let a of this.includes) {
			s += '#include "' + a + '"\n';
		}

		s += this.header;

		// Input structure
		let index = 0;
		//if (ins.length > 0) {
			s += 'struct main_in {\n';
			index = 0;
			this.ins.sort((a, b): i32 => {
				// Sort inputs by name
				return a.substring(4) >= b.substring(4) ? 1 : -1;
			});
			if (this.shader_type == 'vert') {
				for (let a of this.ins) {
					s += `${a} [[attribute(${index})]];\n`;
					index++;
				}
			}
			else {
				for (let a of this.ins) {
					s += `${a} [[user(locn${index})]];\n`;
					index++;
				}
			}
			s += '};\n';
		//}

		// Output structure
		let num = 0;
		if (this.outs.length > 0 || this.shader_type == 'vert') {
			s += 'struct main_out {\n';
			this.outs.sort((a, b): i32 => {
				// Sort outputs by name
				return a.substring(4) >= b.substring(4) ? 1 : -1;
			});
			index = 0;
			if (this.shader_type == 'vert') {
				for (let a of this.outs) {
					s += `${a} [[user(locn${index})]];\n`;
					index++;
				}
				s += 'float4 svpos [[position]];\n';
			}
			else {
				let out = this.outs[0];
				// Multiple render targets
				if (out.charAt(out.length - 1) == ']') {
					num = parseInt(out.charAt(out.length - 2));
					for (let i = 0; i < num; ++i) {
						s += `float4 fragColor_${i} [[color(${i})]];\n`;
					}
				}
				else {
					s += 'float4 fragColor [[color(0)]];\n';
				}
			}
			s += '};\n';
		}

		let samplers: string[] = [];

		if (this.uniforms.length > 0) {
			s += 'struct main_uniforms {\n';

			for (let a of this.uniforms) {
				if (a.startsWith('sampler')) {
					samplers.push(a);
				}
				else {
					s += a + ';\n';
				}
			}

			s += '};\n';
		}

		for (let f of this.functions.values()) {
			s += f + '\n';
		}

		// Begin main declaration
		s += '#undef texture\n';

		s += this.shader_type == 'vert' ? 'vertex ' : 'fragment ';
		s += (this.outs.length > 0 || this.shader_type == 'vert') ? 'main_out ' : 'void ';
		s += 'my_main(';
		//if (ins.length > 0) {
			s += 'main_in in [[stage_in]]';
		//}
		if (this.uniforms.length > 0) {
			let bufi = this.shader_type == 'vert' ? 1 : 0;
			s += `, constant main_uniforms& uniforms [[buffer(${bufi})]]`;
		}

		if (samplers.length > 0) {
			for (let i = 0; i < samplers.length; ++i) {
				s += `, ${samplers[i]} [[texture(${i})]]`;
				s += ', sampler ' + samplers[i].split(' ')[1] + `_sampler [[sampler(${i})]]`;
			}
		}

		if (this.sharedSamplers.length > 0) {
			for (let i = 0; i < this.sharedSamplers.length; ++i) {
				let index = samplers.length + i;
				s += `, ${this.sharedSamplers[i]} [[texture(${index})]]`;
			}
			s += `, sampler ${sharedSampler} [[sampler(${samplers.length})]]`;
		}

		// Built-ins
		if (this.shader_type == 'vert' && this.main.indexOf("gl_VertexID") >= 0) {
			s += ', uint gl_VertexID [[vertex_id]]';
		}
		if (this.shader_type == 'vert' && this.main.indexOf("gl_InstanceID") >= 0) {
			s += ', uint gl_InstanceID [[instance_id]]';
		}

		// End main declaration
		s += ') {\n';
		s += '#define texture(tex, coord) tex.sample(tex ## _sampler, coord)\n';

		// Declare inputs
		for (let a of this.ins) {
			let b = a.substring(5); // Remove type 'vec4 '
			s += `${a} = in.${b};\n`;
		}

		for (let a of this.uniforms) {
			if (!a.startsWith('sampler')) {
				let b = a.split(" ")[1]; // Remove type 'vec4 '
				if (b.indexOf("[") >= 0) {
					b = b.substring(0, b.indexOf("["));
					let type = a.split(" ")[0];
					s += `constant ${type} *${b} = uniforms.${b};\n`;
				}
				else {
					s += `${a} = uniforms.${b};\n`;
				}
			}
		}

		if (this.shader_type == 'vert') {
			s += 'vec4 gl_Position;\n';
			for (let a of this.outs) {
				s += `${a};\n`;
			}
		}
		else {
			if (this.outs.length > 0) {
				if (num > 0) s += `vec4 fragColor[${num}];\n`;
				else s += 'vec4 fragColor;\n';
			}
		}

		s += this.main_attribs;
		s += this.main_textures;
		s += this.main_normal;
		s += this.main_init;
		s += this.main;
		s += this.main_end;

		// Write output structure
		if (this.outs.length > 0 || this.shader_type == 'vert') {
			s += 'main_out out = {};\n';
			if (this.shader_type == 'vert') {
				s += 'gl_Position.z = (gl_Position.z + gl_Position.w) * 0.5;\n';
				s += 'out.svpos = gl_Position;\n';
				for (let a of this.outs) {
					let b = a.split(" ")[1]; // Remove type 'vec4 '
					s += `out.${b} = ${b};\n`;
				}
			}
			else {
				if (num > 0) {
					for (let i = 0; i < num; ++i) {
						s += `out.fragColor_${i} = fragColor[${i}];\n`;
					}
				}
				else {
					s += 'out.fragColor = fragColor;\n';
				}
			}
			s += 'return out;\n';
		}
		s += '}\n';

		///else // krom_opengl

		///if krom_vulkan
		let s = '#version 450\n';
		///elseif krom_android
		let s = '#version 300 es\n';
		if (this.shader_type == 'frag') {
			s += 'precision highp float;\n';
			s += 'precision mediump int;\n';
		}
		///else
		let s = '#version 330\n';
		///end

		s += '#define textureArg(tex) sampler2D tex\n';
		s += '#define texturePass(tex) tex\n';
		s += '#define mul(a, b) b * a\n';
		s += '#define textureShared texture\n';
		s += '#define textureLodShared textureLod\n';
		s += '#define atan2(x, y) atan(y, x)\n';
		s += this.header;

		let in_ext = '';
		let out_ext = '';

		for (let a of this.includes) {
			s += '#include "' + a + '"\n';
		}
		for (let a of this.ins) {
			s += `in ${a}${in_ext};\n`;
		}
		for (let a of this.outs) {
			s += `out ${a}${out_ext};\n`;
		}
		for (let a of this.uniforms) {
			s += 'uniform ' + a + ';\n';
		}
		for (let a of this.sharedSamplers) {
			s += 'uniform ' + a + ';\n';
		}
		for (let f of this.functions.values()) {
			s += f + '\n';
		}
		s += 'void main() {\n';
		s += this.main_attribs;
		s += this.main_textures;
		s += this.main_normal;
		s += this.main_init;
		s += this.main;
		s += this.main_end;
		s += '}\n';

		///end

		return s;
	}
}
