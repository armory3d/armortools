package arm.node;

import iron.data.SceneFormat;
import arm.ui.UITrait;
import arm.ui.UINodes;
import arm.node.MaterialShader;
import arm.Tool;

class MakePaint {

	public static function run(data: MaterialShaderData, matcon: TMaterialContext): MaterialShaderContext {
		var layered = Context.layer != Project.layers[0];
		var eraser = Context.tool == ToolEraser;
		var context_id = "paint";
		var con_paint:MaterialShaderContext = data.add_context({
			name: context_id,
			depth_write: false,
			compare_mode: "always", // TODO: align texcoords winding order
			// cull_mode: "counter_clockwise",
			cull_mode: "none",
			vertex_elements: [{name: "pos", data: "short4norm"}, {name: "nor", data: "short2norm"}, {name: "tex", data: "short2norm"}]
		});

		con_paint.data.color_writes_red = [true, true, true, true];
		con_paint.data.color_writes_green = [true, true, true, true];
		con_paint.data.color_writes_blue = [true, true, true, true];
		con_paint.data.color_writes_alpha = [true, true, true, true];

		var vert = con_paint.make_vert();
		var frag = con_paint.make_frag();
		frag.ins = vert.outs;

		#if kha_direct3d12
		if (Context.tool == ToolBake && UITrait.inst.bakeType == BakeInit) {
			// Init raytraced bake
			MakeBake.positionAndNormal(vert, frag);
			con_paint.data.shader_from_source = true;
			con_paint.data.vertex_shader = vert.get();
			con_paint.data.fragment_shader = frag.get();
			return con_paint;
		}
		#end

		if (Context.tool == ToolBake) {
			MakeBake.setColorWrites(con_paint);
		}

		if (Context.tool == ToolColorId || Context.tool == ToolPicker) {
			MakeColorIdPicker.run(vert, frag);
			con_paint.data.shader_from_source = true;
			con_paint.data.vertex_shader = vert.get();
			con_paint.data.fragment_shader = frag.get();
			return con_paint;
		}

		var faceFill = Context.tool == ToolFill && UITrait.inst.fillTypeHandle.position == FillFace;
		var decal = Context.tool == ToolDecal || Context.tool == ToolText;
		if (!faceFill && !decal) { // Fix seams at uv borders
			vert.add_uniform('vec2 sub', '_sub');
			vert.write('vec2 subtex = tex + sub;');
		}
		else {
			vert.write('vec2 subtex = tex;');
		}

		#if (kha_direct3d11 || kha_direct3d12)
		vert.write('vec2 tpos = vec2(subtex.x * 2.0 - 1.0, (1.0 - subtex.y) * 2.0 - 1.0);');
		#else
		vert.write('vec2 tpos = vec2(subtex.xy * 2.0 - 1.0);');
		#end

		vert.write('gl_Position = vec4(tpos, 0.0, 1.0);');

		vert.add_uniform('mat4 WVP', '_worldViewProjectionMatrix');
		vert.add_out('vec4 ndc');
		vert.write_attrib('ndc = mul(vec4(pos.xyz, 1.0), WVP);');

		frag.write_attrib('vec3 sp = vec3((ndc.xyz / ndc.w) * 0.5 + 0.5);');
		frag.write_attrib('sp.y = 1.0 - sp.y;');
		frag.write_attrib('sp.z -= 0.0001;'); // small bias

		var uvType = Context.layer.material_mask != null ? Context.layer.uvType : UITrait.inst.brushPaint;
		if (uvType == UVProject) frag.ndcpos = true;

		frag.add_uniform('vec4 inp', '_inputBrush');
		frag.add_uniform('vec4 inplast', '_inputBrushLast');
		frag.add_uniform('float aspectRatio', '_aspectRatioWindowF');
		frag.write('vec2 bsp = sp.xy * 2.0 - 1.0;');
		frag.write('bsp.x *= aspectRatio;');
		frag.write('bsp = bsp * 0.5 + 0.5;');

		frag.add_uniform('sampler2D gbufferD');

		frag.add_out('vec4 fragColor[4]');

		frag.add_uniform('float brushRadius', '_brushRadius');
		frag.add_uniform('float brushOpacity', '_brushOpacity');
		frag.add_uniform('float brushHardness', '_brushHardness');

		if (Context.tool == ToolBrush  ||
			Context.tool == ToolEraser ||
			Context.tool == ToolClone  ||
			Context.tool == ToolBlur   ||
			Context.tool == ToolParticle ||
			decal) {

			var depthReject = !UITrait.inst.xray;
			if (UITrait.inst.brush3d && !UITrait.inst.brushDepthReject) depthReject = false;

			// TODO: sp.z needs to take height channel into account
			if (MaterialBuilder.heightUsed) depthReject = false;

			if (depthReject) {
				#if (kha_opengl || kha_webgl)
				frag.write('if (sp.z > textureLod(gbufferD, vec2(sp.x, 1.0 - sp.y), 0.0).r) discard;');
				#else
				frag.write('if (sp.z > textureLod(gbufferD, sp.xy, 0.0).r) discard;');
				#end
			}

			if (decal || Context.tool == ToolParticle) {
				frag.write('float dist = 0.0;');
			}
			else {
				MakeBrush.run(vert, frag);
			}
		}
		else { // Fill, Bake
			frag.write('float dist = 0.0;');
			var angleFill = Context.tool == ToolFill && UITrait.inst.fillTypeHandle.position == FillAngle;
			if (angleFill) {
				frag.add_function(MaterialFunctions.str_octahedronWrap);
				frag.add_uniform('sampler2D gbuffer0');
				frag.write('vec2 g0 = textureLod(gbuffer0, inp.xy, 0.0).rg;');
				frag.write('vec3 wn;');
				frag.write('wn.z = 1.0 - abs(g0.x) - abs(g0.y);');
				frag.write('wn.xy = wn.z >= 0.0 ? g0.xy : octahedronWrap(g0.xy);');
				frag.write('wn = normalize(wn);');
				frag.n = true;
				var angle = UITrait.inst.brushAngleRejectDot;
				frag.write('if (dot(wn, n) < $angle) discard;');
			}
		}

		if (UITrait.inst.colorIdPicked) {
			MakeDiscard.colorId(vert, frag);
		}
		else if (faceFill) { // TODO: allow to combine with colorid mask
			MakeDiscard.face(vert, frag);
		}
		if (UITrait.inst.pickerMaskHandle.position == MaskMaterial) {
			MakeDiscard.materialId(vert, frag);
		}

		MakeTexcoord.run(vert, frag);

		if (Context.tool == ToolClone || Context.tool == ToolBlur) {

			frag.add_uniform('sampler2D gbuffer2');
			frag.add_uniform('vec2 gbufferSize', '_gbufferSize');
			frag.add_uniform('sampler2D texpaint_undo', '_texpaint_undo');
			frag.add_uniform('sampler2D texpaint_nor_undo', '_texpaint_nor_undo');
			frag.add_uniform('sampler2D texpaint_pack_undo', '_texpaint_pack_undo');

			if (Context.tool == ToolClone) {
				MakeClone.run(vert, frag);
			}
			else { // Blur
				MakeBlur.run(vert, frag);
			}
		}
		else {
			Material.parse_emission = Context.material.paintEmis;
			Material.parse_subsurface = Context.material.paintSubs;
			Material.parse_height = Context.material.paintHeight;
			Material.parse_height_as_channel = true;
			var uvType = Context.layer.material_mask != null ? Context.layer.uvType : UITrait.inst.brushPaint;
			Material.triplanar = uvType == UVTriplanar && !decal;
			var sout = Material.parse(UINodes.inst.getCanvasMaterial(), con_paint, vert, frag, null, null, null, matcon);
			Material.parse_emission = false;
			Material.parse_subsurface = false;
			Material.parse_height_as_channel = false;
			Material.parse_height = false;
			var base = sout.out_basecol;
			var rough = sout.out_roughness;
			var met = sout.out_metallic;
			var occ = sout.out_occlusion;
			var nortan = Material.out_normaltan;
			var height = sout.out_height;
			var opac = sout.out_opacity;
			var emis = sout.out_emission;
			var subs = sout.out_subsurface;
			frag.write('vec3 basecol = $base;');
			frag.write('float roughness = $rough;');
			frag.write('float metallic = $met;');
			frag.write('float occlusion = $occ;');
			frag.write('vec3 nortan = $nortan;');
			frag.write('float height = $height;');
			frag.write('float opacity = $opac;');
			if (Context.layer.material_mask == null) {
				frag.write('opacity *= brushOpacity;');
			}
			if (Context.material.paintEmis) {
				frag.write('float emis = $emis;');
			}
			if (Context.material.paintSubs) {
				frag.write('float subs = $subs;');
			}
			if (height != "0" && !MaterialBuilder.heightUsed) {
				MaterialBuilder.heightUsed = true;
				// Height used for the first time, also rebuild vertex shader
				return run(data, matcon);
			}
			if (emis != "0") MaterialBuilder.emisUsed = true;
			if (subs != "0") MaterialBuilder.subsUsed = true;
		}

		if (Context.tool == ToolDecal) {
			frag.add_uniform('sampler2D texdecalmask', '_texdecalmask');
			frag.write('opacity *= textureLod(texdecalmask, texCoord, 0.0).r;');
		}
		else if (Context.tool == ToolText) {
			frag.add_uniform('sampler2D textexttool', '_textexttool');
			frag.write('opacity *= textureLod(textexttool, texCoord, 0.0).r;');
		}

		if (UITrait.inst.brushMaskImage != null && Context.tool == ToolBrush) {
			frag.add_uniform('sampler2D texbrushmask', '_texbrushmask');
			frag.write('vec2 binp_mask = inp.xy * 2.0 - 1.0;');
			frag.write('binp_mask.x *= aspectRatio;');
			frag.write('binp_mask = binp_mask * 0.5 + 0.5;');
			frag.write('vec2 pa_mask = bsp.xy - binp_mask.xy;');
			frag.write('pa_mask /= brushRadius;');
			if (UITrait.inst.brush3d) {
				frag.add_uniform('vec3 eye', '_cameraPosition');
				frag.write('pa_mask *= distance(eye, winp.xyz) / 1.5;');
			}
			frag.write('pa_mask = pa_mask.xy * 0.5 + 0.5;');
			frag.write('opacity *= textureLod(texbrushmask, pa_mask, 0).r;');
		}

		if (Context.tool == ToolParticle) { // particle mask
			frag.add_uniform('sampler2D texparticle', '_texparticle');
			#if (kha_opengl || kha_webgl)
			frag.write('float str = textureLod(texparticle, vec2(sp.x, (1.0 - sp.y)), 0).r;');
			#else
			frag.write('float str = textureLod(texparticle, sp.xy, 0).r;');
			#end
		}
		else { // brush cursor mask
			frag.write('float str = clamp((brushRadius - dist) * brushHardness * 400.0, 0.0, 1.0) * opacity;');
		}

		// Manual blending to preserve memory
		frag.wvpposition = true;
		frag.write('vec2 sample_tc = vec2(wvpposition.x / wvpposition.w, wvpposition.y / wvpposition.w) * 0.5 + 0.5;');
		#if (kha_direct3d11 || kha_direct3d12)
		frag.write('sample_tc.y = 1.0 - sample_tc.y;');
		#end
		frag.add_uniform('sampler2D paintmask');
		frag.write('float sample_mask = textureLod(paintmask, sample_tc, 0.0).r;');
		frag.write('str = max(str, sample_mask);');

		frag.add_uniform('sampler2D texpaint_undo', '_texpaint_undo');
		frag.write('vec4 sample_undo = textureLod(texpaint_undo, sample_tc, 0.0);');

		var matid = Context.material.id / 255;
		if (UITrait.inst.pickerMaskHandle.position == MaskMaterial) {
			matid = UITrait.inst.materialIdPicked / 255; // Keep existing material id in place when mask is set
		}
		frag.write('float matid = $matid;');

		// TODO: Use emission/subsurface matid
		// matid % 3 == 0 - normal, 1 - emission, 2 - subsurface
		if (Context.material.paintSubs) {
			frag.write('if (subs > 0) { matid = 254 / 255; }');
		}
		if (Context.material.paintEmis) {
			frag.write('if (emis > 0) { matid = 255 / 255; }');
		}

		if (layered) {
			if (eraser) {
				frag.write('fragColor[0] = vec4(mix(sample_undo.rgb, vec3(0.0, 0.0, 0.0), str), sample_undo.a - str);');
				frag.write('nortan = vec3(0.5, 0.5, 1.0);');
				frag.write('occlusion = 1.0;');
				frag.write('roughness = 0.0;');
				frag.write('metallic = 0.0;');
				frag.write('matid = 0.0;');
			}
			else if (decal) {
				frag.write('fragColor[0] = vec4(mix(sample_undo.rgb, basecol, str), max(str, sample_undo.a));');
			}
			else {
				frag.write('fragColor[0] = vec4(' + MaterialBuilder.blendMode(frag, UITrait.inst.brushBlending, 'sample_undo.rgb', 'basecol', 'opacity') + ', max(str, sample_undo.a));');

			}
			frag.write('fragColor[1] = vec4(nortan, matid);');

			var height = "0.0";
			if (Context.material.paintHeight && MaterialBuilder.heightUsed) {
				height = "height";
			}

			if (decal) {
				frag.add_uniform('sampler2D texpaint_pack_undo', '_texpaint_pack_undo');
				frag.write('vec4 sample_pack_undo = textureLod(texpaint_pack_undo, sample_tc, 0.0);');
				frag.write('fragColor[2] = mix(sample_pack_undo, vec4(occlusion, roughness, metallic, $height), str);');
			}
			else {
				frag.write('fragColor[2] = vec4(occlusion, roughness, metallic, $height);');
			}
		}
		else {
			if (eraser) {
				frag.write('fragColor[0] = vec4(mix(sample_undo.rgb, vec3(${Layers.defaultBase}, ${Layers.defaultBase}, ${Layers.defaultBase}), str), sample_undo.a - str);');
				frag.write('fragColor[1] = vec4(0.5, 0.5, 1.0, 0.0);');
				frag.write('fragColor[2] = vec4(1.0, ${Layers.defaultRough}, 0.0, 0.0);');
			}
			else {
				frag.add_uniform('sampler2D texpaint_nor_undo', '_texpaint_nor_undo');
				frag.add_uniform('sampler2D texpaint_pack_undo', '_texpaint_pack_undo');
				frag.write('vec4 sample_nor_undo = textureLod(texpaint_nor_undo, sample_tc, 0.0);');
				frag.write('vec4 sample_pack_undo = textureLod(texpaint_pack_undo, sample_tc, 0.0);');
				frag.write('fragColor[0] = vec4(' + MaterialBuilder.blendMode(frag, UITrait.inst.brushBlending, 'sample_undo.rgb', 'basecol', 'str') + ', 0.0);');
				frag.write('fragColor[1] = vec4(mix(sample_nor_undo.rgb, nortan, str), matid);');
				if (Context.material.paintHeight && MaterialBuilder.heightUsed) {
					frag.write('fragColor[2] = mix(sample_pack_undo, vec4(occlusion, roughness, metallic, height), str);');
				}
				else {
					frag.write('fragColor[2] = vec4(mix(sample_pack_undo.rgb, vec3(occlusion, roughness, metallic), str), 0.0);');
				}
			}
		}
		frag.write('fragColor[3] = vec4(str, 0.0, 0.0, 1.0);');

		if (!Context.material.paintBase) {
			con_paint.data.color_writes_red[0] = false;
			con_paint.data.color_writes_green[0] = false;
			con_paint.data.color_writes_blue[0] = false;
		}
		if (!Context.material.paintNor) {
			con_paint.data.color_writes_red[1] = false;
			con_paint.data.color_writes_green[1] = false;
			con_paint.data.color_writes_blue[1] = false;
		}
		if (!Context.material.paintOcc) {
			con_paint.data.color_writes_red[2] = false;
		}
		if (!Context.material.paintRough) {
			con_paint.data.color_writes_green[2] = false;
		}
		if (!Context.material.paintMet) {
			con_paint.data.color_writes_blue[2] = false;
		}
		if (!Context.material.paintHeight) {
			con_paint.data.color_writes_alpha[2] = false;
		}

		// Base color only as mask
		var isMask = Context.layerIsMask;
		if (isMask) {
			// TODO: Apply opacity into base
			// frag.write('fragColor[0].rgb *= fragColor[0].a;');
			con_paint.data.color_writes_red[1] = false;
			con_paint.data.color_writes_green[1] = false;
			con_paint.data.color_writes_blue[1] = false;
			con_paint.data.color_writes_red[2] = false;
			con_paint.data.color_writes_green[2] = false;
			con_paint.data.color_writes_blue[2] = false;
			con_paint.data.color_writes_alpha[2] = false;
		}

		if (Context.tool == ToolBake) {
			MakeBake.run(vert, frag);
		}

		Material.finalize(con_paint);
		Material.triplanar = false;
		con_paint.data.shader_from_source = true;
		con_paint.data.vertex_shader = vert.get();
		con_paint.data.fragment_shader = frag.get();

		return con_paint;
	}
}
