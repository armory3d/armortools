package arm;

import armory.system.Cycles;
import armory.system.CyclesFormat;
import armory.system.CyclesShader;
import armory.system.CyclesShader.CyclesShaderContext;
import iron.data.SceneFormat;
import iron.data.ShaderData;
import iron.data.ShaderData.ShaderContext;
import iron.data.MaterialData;
import iron.data.SceneFormat;

class MaterialBuilder {

	public static function make_paint(data:CyclesShaderData, matcon:TMaterialContext):CyclesShaderContext {
		var layered = UITrait.inst.selectedLayer != UITrait.inst.layers[0];
		var eraser = UITrait.inst.brushType == 1;
		var context_id = 'paint';
		var con_paint:CyclesShaderContext = data.add_context({
			name: context_id,
			depth_write: false,
			compare_mode: 'always', // TODO: align texcoords winding order
			// cull_mode: 'counter_clockwise',
			cull_mode: 'none',
			blend_source: layered ? 'blend_one' : 'source_alpha',
			blend_destination: layered ? 'blend_zero' : 'inverse_source_alpha',
			blend_operation: 'add',
			alpha_blend_source: 'blend_one',
			alpha_blend_destination: eraser ? 'blend_zero' : 'blend_one',
			alpha_blend_operation: 'add',
			vertex_elements: [{name: "pos", data: 'short4norm'},{name: "nor", data: 'short2norm'},{name: "tex", data: 'short2norm'}] });

		con_paint.data.color_writes_red = [true, true, true, true];
		con_paint.data.color_writes_green = [true, true, true, true];
		con_paint.data.color_writes_blue = [true, true, true, true];

		if (UITrait.inst.brushType == 3) { // Bake AO
			con_paint.data.color_writes_green[2] = false; // No rough
			con_paint.data.color_writes_blue[2] = false; // No met
		}

		var vert = con_paint.make_vert();
		var frag = con_paint.make_frag();
		frag.ins = vert.outs;

		if (UITrait.inst.brushType == 4) { // Pick color id
			// Mangle vertices to form full screen triangle
			vert.write('gl_Position = vec4(-1.0 + float((gl_VertexID & 1) << 2), -1.0 + float((gl_VertexID & 2) << 1), 0.0, 1.0);');

			frag.add_out('vec4 fragColor');
			
			frag.add_uniform('sampler2D gbuffer2');
			frag.add_uniform('sampler2D texcolorid', '_texcolorid');
			frag.add_uniform('vec2 gbufferSize', '_gbufferSize');
			frag.add_uniform('vec4 inp', '_inputBrush');

			#if (kha_opengl || kha_webgl)
			frag.write('vec2 texCoordInp = texelFetch(gbuffer2, ivec2(inp.x * gbufferSize.x, (1.0 - inp.y) * gbufferSize.y), 0).ba;');
			#else
			frag.write('vec2 texCoordInp = texelFetch(gbuffer2, ivec2(inp.x * gbufferSize.x, inp.y * gbufferSize.y), 0).ba;');
			#end

			frag.write('vec3 idcol = textureLod(texcolorid, texCoordInp, 0).rgb;');
			frag.write('fragColor = vec4(idcol, 1.0);');

			con_paint.data.shader_from_source = true;
			con_paint.data.vertex_shader = vert.get();
			con_paint.data.fragment_shader = frag.get();
			return con_paint;
		}

		#if kha_direct3d11
		vert.write('vec2 tpos = vec2(tex.x * 2.0 - 1.0, (1.0 - tex.y) * 2.0 - 1.0);');
		#else
		vert.write('vec2 tpos = vec2(tex.x * 2.0 - 1.0, tex.y * 2.0 - 1.0);');
		#end

		var faceFill = UITrait.inst.brushType == 2 && UITrait.inst.fillTypeHandle.position == 1;
		var decal = UITrait.inst.brushType == 5;

		if (!faceFill && !decal) {
			// Fix seams at uv borders
			vert.add_uniform('vec2 sub', '_sub');
			vert.write('tpos += sub;');
		}

		vert.write('gl_Position = vec4(tpos, 0.0, 1.0);');

		vert.add_uniform('mat4 WVP', '_worldViewProjectionMatrix');
		vert.add_out('vec4 ndc');
		vert.write_attrib('ndc = mul(vec4(pos.xyz, 1.0), WVP);');
		frag.write_attrib('vec3 sp = vec3((ndc.xyz / ndc.w) * 0.5 + 0.5);');
		frag.write_attrib('sp.y = 1.0 - sp.y;');

		frag.add_uniform('float paintDepthBias', '_paintDepthBias');
		frag.write_attrib('sp.z -= paintDepthBias;'); // small bias or !paintVisible

		if (UITrait.inst.brushPaint != 0) frag.ndcpos = true;

		if (UITrait.inst.brushType == 3) { // Bake ao
			frag.wposition = true;
			frag.n = true;
		}

		frag.add_uniform('vec4 inp', '_inputBrush');
		frag.add_uniform('vec4 inplast', '_inputBrushLast');
		frag.add_uniform('float aspectRatio', '_aspectRatioWindowF');
		frag.write('vec2 bsp = sp.xy * 2.0 - 1.0;');
		frag.write('bsp.x *= aspectRatio;');
		frag.write('bsp = bsp * 0.5 + 0.5;');

		frag.add_uniform('sampler2D paintdb');

		var numTex = UITrait.inst.paintHeight ? 4 : 3;
		frag.add_out('vec4 fragColor[$numTex]');

		frag.add_uniform('float brushRadius', '_brushRadius');
		frag.add_uniform('float brushOpacity', '_brushOpacity');
		frag.add_uniform('float brushStrength', '_brushStrength');

		if (UITrait.inst.brushType == 0 || UITrait.inst.brushType == 1 || decal) { // Draw / Erase
			
			#if (kha_opengl || kha_webgl)
			frag.write('if (sp.z > textureLod(paintdb, vec2(sp.x, 1.0 - bsp.y), 0).r) { discard; }');
			#else
			frag.write('if (sp.z > textureLod(paintdb, vec2(sp.x, bsp.y), 0).r) { discard; }');
			#end

			if (decal) {
				frag.write('float dist = 0.0;');
			}
			else {
				frag.write('vec2 binp = inp.xy * 2.0 - 1.0;');
				frag.write('binp.x *= aspectRatio;');
				frag.write('binp = binp * 0.5 + 0.5;');

				// Continuos paint
				frag.write('vec2 binplast = inplast.xy * 2.0 - 1.0;');
				frag.write('binplast.x *= aspectRatio;');
				frag.write('binplast = binplast * 0.5 + 0.5;');
				
				frag.write('vec2 pa = bsp.xy - binp.xy, ba = binplast.xy - binp.xy;');
				frag.write('float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);');
				frag.write('float dist = length(pa - ba * h);');
				
				if (!UITrait.inst.mirrorX) {
					frag.write('if (dist > brushRadius) { discard; }');
				}
				else {
					frag.write('vec2 binp2 = vec2(0.5 - (binp.x - 0.5), binp.y), binplast2 = vec2(0.5 - (binplast.x - 0.5), binplast.y);');
					frag.write('vec2 pa2 = bsp.xy - binp2.xy, ba2 = binplast2.xy - binp2.xy;');
					frag.write('float h2 = clamp(dot(pa2, ba2) / dot(ba2, ba2), 0.0, 1.0);');
					frag.write('float dist2 = length(pa2 - ba2 * h2);');
					frag.write('if (dist > brushRadius && dist2 > brushRadius) { discard; }');
				}
				//

				// frag.write('float dist = distance(bsp.xy, binp.xy);');
				// frag.write('if (dist > brushRadius) { discard; }');
			}
		}
		else { // Fill, Bake
			frag.write('float dist = 0.0;');
		}

		if (UITrait.inst.colorIdPicked) {
			vert.add_out('vec2 texCoordPick');
			vert.write('texCoordPick = fract(tex);');
			frag.add_uniform('sampler2D texpaint_colorid'); // 1x1 picker
			frag.add_uniform('sampler2D texcolorid', '_texcolorid'); // color map
			frag.add_uniform('vec2 texcoloridSize', '_texcoloridSize'); // color map
			frag.write('vec3 c1 = texelFetch(texpaint_colorid, ivec2(0, 0), 0).rgb;');
			frag.write('vec3 c2 = texelFetch(texcolorid, ivec2(texCoordPick * texcoloridSize), 0).rgb;');
			frag.write('if (any(c1 != c2)) { discard; }');
		}
		else if (faceFill) {
			vert.add_out('vec2 texCoordPick');
			vert.write('texCoordPick = fract(tex);');
			frag.add_uniform('sampler2D gbuffer2');
			frag.add_uniform('sampler2D textrianglemap', '_textrianglemap'); // triangle map
			frag.add_uniform('vec2 textrianglemapSize', '_textrianglemapSize');
			frag.add_uniform('vec2 gbufferSize', '_gbufferSize');
			#if (kha_opengl || kha_webgl)
			frag.write('vec2 texCoordInp = texelFetch(gbuffer2, ivec2(inp.x * gbufferSize.x, (1.0 - inp.y) * gbufferSize.y), 0).ba;');
			#else
			frag.write('vec2 texCoordInp = texelFetch(gbuffer2, ivec2(inp.x * gbufferSize.x, inp.y * gbufferSize.y), 0).ba;');
			#end
			frag.write('vec4 c1 = texelFetch(textrianglemap, ivec2(texCoordInp * textrianglemapSize), 0);');
			frag.write('vec4 c2 = texelFetch(textrianglemap, ivec2(texCoordPick * textrianglemapSize), 0);');
			frag.write('if (any(c1 != c2)) { discard; }');
		}

		// Texture projection - texcoords
		if (UITrait.inst.brushPaint == 0 && !decal) {
			vert.add_uniform('float brushScale', '_brushScale'); // TODO: Will throw uniform not found
			vert.add_out('vec2 texCoord');
			vert.write('texCoord = tex * brushScale;');
		}
		// Texture projection - project
		else {
			frag.add_uniform('float brushScale', '_brushScale');
			frag.write_attrib('vec2 uvsp = sp.xy;');

			if (decal) {
				frag.write_attrib('vec2 binp = inp.xy * 2.0 - 1.0;');
				frag.write_attrib('binp = binp * 0.5 + 0.5;');

				frag.write_attrib('uvsp -= binp;');
				frag.write_attrib('uvsp.x *= aspectRatio;');

				// frag.write_attrib('uvsp *= brushScale;');
				frag.write_attrib('uvsp *= vec2(7.2, 7.2);');
				frag.write_attrib('uvsp += vec2(0.5, 0.5);');

				frag.write_attrib('if (uvsp.x < 0.0 || uvsp.y < 0.0 || uvsp.x > 1.0 || uvsp.y > 1.0) { discard; }');
			}
			else {
				frag.write_attrib('uvsp.x *= aspectRatio;');
			}
			
			frag.write_attrib('vec2 texCoord = fract(uvsp * brushScale);');
			// Cycles.texCoordName = 'fract(uvsp * brushScale)'; // TODO: use prescaled value from VS
		}

		Cycles.parse_height_as_channel = true;
		var sout = Cycles.parse(UINodes.inst.canvas, con_paint, vert, frag, null, null, null, matcon);
		Cycles.parse_height_as_channel = false;
		Cycles.texCoordName = 'texCoord';
		var base = sout.out_basecol;
		var rough = sout.out_roughness;
		var met = sout.out_metallic;
		var occ = sout.out_occlusion;
		var nortan = Cycles.out_normaltan;
		var height = sout.out_height;
		var opac = sout.out_opacity;
		frag.write('vec3 basecol = $base;');
		frag.write('float roughness = $rough;');
		frag.write('float metallic = $met;');
		frag.write('float occlusion = $occ;');
		frag.write('vec3 nortan = $nortan;');
		frag.write('float height = $height;');
		frag.write('float opacity = $opac * brushOpacity;');

		if (eraser) frag.write('    float str = 1.0 - opacity;');
		else frag.write('    float str = clamp(opacity * (brushRadius - dist) * brushStrength, 0.0, 1.0);');

		if (UITrait.inst.mirrorX && UITrait.inst.brushType == 0) { // Draw
			frag.write('str += clamp(opacity * (brushRadius - dist2) * brushStrength, 0.0, 1.0);');
			frag.write('str = clamp(str, 0.0, 1.0);');
		}

		frag.write('    fragColor[0] = vec4(basecol, str);');
		frag.write('    fragColor[1] = vec4(nortan, 1.0);'); // nortan.xy *= str
		frag.write('    fragColor[2] = vec4(occlusion, roughness, metallic, str);');
		if (UITrait.inst.paintHeight) {
			frag.write('    fragColor[3] = vec4(1.0, 0.0, height, str);');
		}

		if (!UITrait.inst.paintBase) frag.write('fragColor[0].a = 0.0;');
		if (!UITrait.inst.paintNor) frag.write('fragColor[1].a = 0.0;');
		if (!UITrait.inst.paintOcc) con_paint.data.color_writes_red[2] = false;
		if (!UITrait.inst.paintRough) con_paint.data.color_writes_green[2] = false;
		if (!UITrait.inst.paintMet) con_paint.data.color_writes_blue[2] = false;

		if (UITrait.inst.brushType == 3) { // Bake AO
			frag.write('fragColor[0].a = 0.0;');
			frag.write('fragColor[1].a = 0.0;');

			// Apply normal channel
			frag.vVec = true;
			frag.add_function(armory.system.CyclesFunctions.str_cotangentFrame);
			#if kha_direct3d11
			frag.write('mat3 TBN = cotangentFrame(n, vVec, texCoord);');
			#else
			frag.write('mat3 TBN = cotangentFrame(n, -vVec, texCoord);');
			#end
			frag.write('n = nortan * 2.0 - 1.0;');
			frag.write('n.y = -n.y;');
			frag.write('n = normalize(mul(n, TBN));');

			frag.write('const vec3 voxelgiHalfExtents = vec3(1.0, 1.0, 1.0);');
			frag.write('vec3 voxpos = wposition / voxelgiHalfExtents;');
			frag.add_uniform('sampler3D voxels');
			frag.add_function(armory.system.CyclesFunctions.str_traceAO);
			frag.n = true;
			var strength = UITrait.inst.bakeStrength;
			var radius = UITrait.inst.bakeRadius;
			var offset = UITrait.inst.bakeOffset;
			frag.write('fragColor[2].r = 1.0 - traceAO(voxpos, n, $radius, $offset) * $strength;');
		}

		Cycles.finalize(con_paint);
		con_paint.data.shader_from_source = true;
		con_paint.data.vertex_shader = vert.get();
		con_paint.data.fragment_shader = frag.get();

		return con_paint;
	}

	public static function make_mesh_preview(data:CyclesShaderData, matcon:TMaterialContext):CyclesShaderContext {
		var context_id = 'mesh';
		var con_mesh:CyclesShaderContext = data.add_context({
			name: context_id,
			depth_write: true,
			compare_mode: 'less',
			cull_mode: 'clockwise',
			vertex_elements: [{name: "pos", data: 'short4norm'},{name: "nor", data: 'short2norm'},{name: "tex", data: 'short2norm'}] });

		var vert = con_mesh.make_vert();
		var frag = con_mesh.make_frag();

		frag.ins = vert.outs;
		vert.add_uniform('mat4 WVP', '_worldViewProjectionMatrix');
		vert.add_out('vec2 texCoord');
		vert.write('gl_Position = mul(vec4(pos.xyz, 1.0), WVP);');
		vert.write('texCoord = tex;');

		var sout = Cycles.parse(UINodes.inst.canvas, con_mesh, vert, frag, null, null, null, matcon);
		var base = sout.out_basecol;
		var rough = sout.out_roughness;
		var met = sout.out_metallic;
		var occ = sout.out_occlusion;
		var opac = sout.out_opacity;
		var nortan = Cycles.out_normaltan;
		frag.write('vec3 basecol = pow($base, vec3(2.2, 2.2, 2.2));');
		frag.write('float roughness = $rough;');
		frag.write('float metallic = $met;');
		frag.write('float occlusion = $occ;');
		frag.write('float opacity = $opac;');
		frag.write('vec3 nortan = $nortan;');

		//if discard_transparent:
			var opac = 0.2;//mat_state.material.discard_transparent_opacity
			frag.write('if (opacity < $opac) discard;');

		frag.add_out('vec4 fragColor[3]');
		frag.n = true;

		frag.add_function(armory.system.CyclesFunctions.str_packFloat);
		frag.add_function(armory.system.CyclesFunctions.str_packFloat2);
		frag.add_function(armory.system.CyclesFunctions.str_cotangentFrame);
		frag.add_function(armory.system.CyclesFunctions.str_octahedronWrap);

		// Apply normal channel
		frag.vVec = true;
		#if kha_direct3d11
		frag.write('mat3 TBN = cotangentFrame(n, vVec, texCoord);');
		#else
		frag.write('mat3 TBN = cotangentFrame(n, -vVec, texCoord);');
		#end
		frag.write('n = nortan * 2.0 - 1.0;');
		frag.write('n.y = -n.y;');
		frag.write('n = normalize(mul(n, TBN));');

		frag.write('n /= (abs(n.x) + abs(n.y) + abs(n.z));');
		frag.write('n.xy = n.z >= 0.0 ? n.xy : octahedronWrap(n.xy);');
		frag.write('fragColor[0] = vec4(n.x, n.y, packFloat(metallic, roughness), 1.0);');
		frag.write('fragColor[1] = vec4(basecol.r, basecol.g, basecol.b, packFloat2(occlusion, 1.0));'); // occ/spec
		frag.write('fragColor[2] = vec4(0.0, 0.0, 0.0, 0.0);'); // veloc

		Cycles.finalize(con_mesh);
		con_mesh.data.shader_from_source = true;
		con_mesh.data.vertex_shader = vert.get();
		con_mesh.data.fragment_shader = frag.get();

		return con_mesh;
	}

	public static function make_depth(data:CyclesShaderData, matcon:TMaterialContext, shadowmap = false):CyclesShaderContext {
		var context_id = shadowmap ? 'shadowmap' : 'depth';
		var con_depth:CyclesShaderContext = data.add_context({
			name: context_id,
			depth_write: true,
			compare_mode: 'less',
			cull_mode: 'clockwise',
			color_write_red: false,
			color_write_green: false,
			color_write_blue: false,
			color_write_alpha: false,
			vertex_elements: [{name: "pos", data: 'short4norm'}]
		});

		var vert = con_depth.make_vert();
		var frag = con_depth.make_frag();

		frag.ins = vert.outs;

		// if (shadowmap) {
		// 	if (UITrait.inst.paintHeight) {
		// 		con_depth.add_elem('nor', 'short2norm');
		// 		vert.wposition = true;
		// 		vert.n = true;
		// 		vert.add_uniform('sampler2D texpaint_opt');
		// 		vert.write('vec4 opt = textureLod(texpaint_opt, tex, 0);');
		// 		vert.write('float height = opt.b;');
		// 		var displaceStrength = UITrait.inst.displaceStrength * 0.1;
		// 		vert.write('wposition += wnormal * height * $displaceStrength;');
				
		// 		vert.add_uniform('mat4 LVP', '_lightViewProjectionMatrix');
		// 		vert.write('gl_Position = LVP * vec4(wposition, 1.0);');
		// 	}
		// 	else {
		// 		vert.add_uniform('mat4 LWVP', '_lightWorldViewProjectionMatrix');
		// 		vert.write('gl_Position = mul(vec4(pos.xyz, 1.0), LWVP);');
		// 	}
		// }
		// else {
			vert.add_uniform('mat4 WVP', '_worldViewProjectionMatrix');
			vert.write('gl_Position = mul(vec4(pos.xyz, 1.0), WVP);');
		// }

		var parse_opacity = shadowmap; // arm_discard
		if (parse_opacity) {
			frag.write_attrib('vec3 n;'); // discard at compile time
			frag.write_attrib('float dotNV;'); // discard at compile time
			// frag.write('float opacity;');

			Cycles.parse_surface = false;
			var sout = Cycles.parse(UINodes.inst.canvas, con_depth, vert, frag, null, null, null, matcon);
			Cycles.parse_surface = true;
			// parse_surface=False, parse_opacity=True
			var opac = sout.out_opacity;
			frag.write('float opacity = $opac;');
			if (con_depth.is_elem('tex')) {
				vert.add_out('vec2 texCoord');
				vert.write('texCoord = tex;');
			}
			
			var opac = 0.2;//mat_state.material.discard_transparent_opacity_shadows
			frag.write('if (opacity < $opac) discard;');
		}

		Cycles.finalize(con_depth);
		con_depth.data.shader_from_source = true;
		con_depth.data.vertex_shader = vert.get();
		con_depth.data.fragment_shader = frag.get();

		return con_depth;
	}

	public static function make_mesh(data:CyclesShaderData):CyclesShaderContext {
		var context_id = 'mesh';
		var con_mesh:CyclesShaderContext = data.add_context({
			name: context_id,
			depth_write: true,
			compare_mode: 'less',
			cull_mode: UITrait.inst.culling ? 'clockwise' : 'none',
			vertex_elements: [{name: "pos", data: 'short4norm'},{name: "nor", data: 'short2norm'},{name: "tex", data: 'short2norm'}] });

		var vert = con_mesh.make_vert();
		var frag = con_mesh.make_frag();

		vert.add_out('vec2 texCoord');
		frag.wvpposition = true;
		vert.add_out('vec4 prevwvpposition');
		vert.add_uniform('mat4 VP', '_viewProjectionMatrix');
		vert.add_uniform('mat4 prevWVP', '_prevWorldViewProjectionMatrix');
		vert.wposition = true;

		// Height
		// TODO: can cause TAA issues
		if (UITrait.inst.paintHeight) {
			#if (!kha_direct3d11) // TODO: unable to bind texpaint_opt to both vs and fs in d3d11
			vert.add_uniform('sampler2D texpaint_opt');
			vert.write('vec4 opt = textureLod(texpaint_opt, tex, 0);');
			vert.write('float height = opt.b;');
			var displaceStrength = UITrait.inst.displaceStrength * 0.1;
			vert.n = true;
			vert.write('wposition += wnormal * height * $displaceStrength;');
			#end
		}
		//

		vert.write('gl_Position = mul(vec4(wposition.xyz, 1.0), VP);');
		vert.write('texCoord = tex;');
		if (UITrait.inst.paintHeight) {
			vert.add_uniform('mat4 invW', '_inverseWorldMatrix');
			vert.write('prevwvpposition = mul(mul(vec4(wposition, 1.0), invW), prevWVP);');
		}
		else {
			vert.write('prevwvpposition = mul(vec4(pos.xyz, 1.0), prevWVP);');
		}

		frag.ins = vert.outs;

		frag.add_out('vec4 fragColor[3]');
		frag.n = true;
		frag.add_function(armory.system.CyclesFunctions.str_packFloat);
		frag.add_function(armory.system.CyclesFunctions.str_packFloat2);

		if (arm.UITrait.inst.brushType == 4) { // Show color map
			frag.add_uniform('sampler2D texcolorid', '_texcolorid');
			frag.write('fragColor[0] = vec4(n.xy, packFloat(0.0, 1.0), 1.0);'); // met/rough
			frag.write('vec3 idcol = pow(texture(texcolorid, texCoord).rgb, vec3(2.2, 2.2, 2.2));');
			frag.write('fragColor[1] = vec4(idcol.rgb, packFloat2(1.0, 1.0));'); // occ/spec
		}
		else {
			frag.add_function(armory.system.CyclesFunctions.str_octahedronWrap);

			frag.add_uniform('sampler2D texpaint');
			frag.add_uniform('sampler2D texpaint_nor');
			frag.add_uniform('sampler2D texpaint_pack');

			frag.write('vec3 basecol;');
			frag.write('float roughness;');
			frag.write('float metallic;');
			frag.write('float occlusion;');
			frag.write('float opacity;');
			frag.write('float specular;');

			if (UITrait.inst.layers[0].visible) {
				frag.write('basecol = pow(texture(texpaint, texCoord).rgb, vec3(2.2, 2.2, 2.2));');

				frag.vVec = true;
				frag.add_function(armory.system.CyclesFunctions.str_cotangentFrame);
				#if kha_direct3d11
				frag.write('mat3 TBN = cotangentFrame(n, vVec, texCoord);');
				#else
				frag.write('mat3 TBN = cotangentFrame(n, -vVec, texCoord);');
				#end
				frag.write('vec3 ntex = texture(texpaint_nor, texCoord).rgb;');
				frag.write('n = ntex * 2.0 - 1.0;');
				frag.write('n.y = -n.y;');
				frag.write('n = normalize(mul(n, TBN));');

				// Height
				if (UITrait.inst.paintHeight) {
					frag.add_uniform('sampler2D texpaint_opt');
					frag.write('vec4 vech;');
					frag.write('vech.x = textureOffset(texpaint_opt, texCoord, ivec2(-1, 0)).b;');
					frag.write('vech.y = textureOffset(texpaint_opt, texCoord, ivec2(1, 0)).b;');
					frag.write('vech.z = textureOffset(texpaint_opt, texCoord, ivec2(0, -1)).b;');
					frag.write('vech.w = textureOffset(texpaint_opt, texCoord, ivec2(0, 1)).b;');
					// Displace normal strength
					frag.write('vech *= 15 * 7; float h1 = vech.x - vech.y; float h2 = vech.z - vech.w;');
					frag.write('vec3 va = normalize(vec3(2.0, 0.0, h1));');
					frag.write('vec3 vb = normalize(vec3(0.0, 2.0, h2));');
					frag.write('vec3 vc = normalize(vec3(h1, h2, 2.0));');
					frag.write('n = normalize(mul(n, mat3(va, vb, vc)));');
				}
				//

				frag.write('vec4 pack = texture(texpaint_pack, texCoord);');
				frag.write('occlusion = pack.r;');
				frag.write('roughness = pack.g;');
				frag.write('metallic = pack.b;');
			}
			else {
				frag.write('basecol = vec3(0.0, 0.0, 0.0);');
				frag.write('occlusion = 1.0;');
				frag.write('roughness = 1.0;');
				frag.write('metallic = 0.0;');
				frag.write('specular = 1.0;');
			}

			if (UITrait.inst.layers.length > 1) {
				frag.write('float factor0;');
				frag.write('float factorinv0;');
				frag.write('float nfactor0;');
				frag.write('float nfactorinv0;');
				frag.write('vec4 col_tex0;');
				frag.write('vec4 col_nor0;');
				frag.write('vec3 n0;');
				frag.write('vec4 pack0;');
				for (i in 1...UITrait.inst.layers.length) {
					if (!UITrait.inst.layers[i].visible) continue;
					var id = UITrait.inst.layers[i].id;
					frag.add_uniform('sampler2D texpaint' + id);
					frag.add_uniform('sampler2D texpaint_nor' + id);
					frag.add_uniform('sampler2D texpaint_pack' + id);
					// frag.add_uniform('sampler2D texpaint_opt' + id);

					frag.write('col_tex0 = texture(texpaint' + id + ', texCoord);');
					frag.write('col_nor0 = texture(texpaint_nor' + id + ', texCoord);');

					frag.write('factor0 = col_tex0.a;');
					frag.write('factorinv0 = 1.0 - factor0;');
					frag.write('basecol = basecol * factorinv0 + pow(col_tex0.rgb, vec3(2.2, 2.2, 2.2)) * factor0;');

					frag.write('nfactor0 = col_nor0.a;');
					frag.write('nfactorinv0 = 1.0 - nfactor0;');					
					frag.write('n0 = texture(texpaint_nor' + id + ', texCoord).rgb * 2.0 - 1.0;');
					frag.write('n0.y = -n0.y;');
					frag.write('n0 = normalize(mul(n0, TBN));');
					frag.write('n = normalize(n * nfactorinv0 + n0 * nfactor0);');

					frag.write('pack0 = texture(texpaint_pack' + id + ', texCoord);');
					frag.write('occlusion = occlusion * factorinv0 + pack0.r * factor0;');
					frag.write('roughness = roughness * factorinv0 + pack0.g * factor0;');
					frag.write('metallic = metallic * factorinv0 + pack0.b * factor0;');
				}
			}

			if (UITrait.inst.drawWireframe) {
				// GL_NV_fragment_shader_barycentric
				// VK_AMD_shader_explicit_vertex_parameter
				frag.add_uniform('sampler2D texuvmap', '_texuvmap');
				frag.write('basecol += texture(texuvmap, texCoord).rgb;');
				// frag.write('if (basecol == vec3(0,0,0)) discard;');
			}

			frag.write('n /= (abs(n.x) + abs(n.y) + abs(n.z));');
			frag.write('n.xy = n.z >= 0.0 ? n.xy : octahedronWrap(n.xy);');
			if (UITrait.inst.viewportMode == 0) { // Render
				frag.write('fragColor[0] = vec4(n.xy, packFloat(metallic, roughness), 0.0);');
				frag.write('fragColor[1] = vec4(basecol.rgb, packFloat2(occlusion, 1.0));'); // occ/spec
			}
			else if (UITrait.inst.viewportMode == 1) { // Basecol
				frag.write('fragColor[0] = vec4(n.xy, packFloat(0.0, 1.0), 0.0);');
				frag.write('fragColor[1] = vec4(basecol.rgb, packFloat2(1.0, 1.0));'); // occ/spec
			}
			else if (UITrait.inst.viewportMode == 2) { // Normal
				frag.write('fragColor[0] = vec4(n.xy, packFloat(0.0, 1.0), 0.0);');
				frag.write('fragColor[1] = vec4(ntex.rgb, packFloat2(1.0, 1.0));'); // occ/spec
			}
			else if (UITrait.inst.viewportMode == 3) { // Occ
				frag.write('fragColor[0] = vec4(n.xy, packFloat(0.0, 1.0), 0.0);');
				frag.write('fragColor[1] = vec4(vec3(occlusion, occlusion, occlusion), packFloat2(1.0, 1.0));'); // occ/spec
			}
			else if (UITrait.inst.viewportMode == 4) { // Rough
				frag.write('fragColor[0] = vec4(n.xy, packFloat(0.0, 1.0), 0.0);');
				frag.write('fragColor[1] = vec4(vec3(roughness, roughness, roughness), packFloat2(1.0, 1.0));'); // occ/spec
			}
			else if (UITrait.inst.viewportMode == 5) { // Met
				frag.write('fragColor[0] = vec4(n.xy, packFloat(0.0, 1.0), 0.0);');
				frag.write('fragColor[1] = vec4(vec3(metallic, metallic, metallic), packFloat2(1.0, 1.0));'); // occ/spec
			}
			else if (UITrait.inst.viewportMode == 6) { // Texcoord
				frag.write('fragColor[0] = vec4(n.xy, packFloat(0.0, 1.0), 0.0);');
				frag.write('fragColor[1] = vec4(texCoord, 0.0, packFloat2(1.0, 1.0));'); // occ/spec
			}
		}

		frag.write('vec2 posa = (wvpposition.xy / wvpposition.w) * 0.5 + 0.5;');
		frag.write('vec2 posb = (prevwvpposition.xy / prevwvpposition.w) * 0.5 + 0.5;');

		var writeTC = (UITrait.inst.brushType == 2 && UITrait.inst.fillTypeHandle.position == 1) || // Face fill
					   UITrait.inst.brushType == 4; // Colorid pick

		if (writeTC) {
			frag.write('fragColor[2] = vec4(posa - posb, texCoord.xy);');
		}
		else {
			frag.write('fragColor[2] = vec4(posa - posb, 0.0, 0.0);');
		}

		Cycles.finalize(con_mesh);
		con_mesh.data.shader_from_source = true;
		con_mesh.data.vertex_shader = vert.get();
		con_mesh.data.fragment_shader = frag.get();

		return con_mesh;
	}
}
