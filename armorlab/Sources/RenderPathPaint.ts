
class RenderPathPaint {

	static liveLayerDrawn = 0; ////

	static init = () => {

		{
			let t = render_target_create();
			t.name = "texpaint_blend0";
			t.width = Config.getTextureResX();
			t.height = Config.getTextureResY();
			t.format = "R8";
			render_path_create_render_target(t);
		}
		{
			let t = render_target_create();
			t.name = "texpaint_blend1";
			t.width = Config.getTextureResX();
			t.height = Config.getTextureResY();
			t.format = "R8";
			render_path_create_render_target(t);
		}
		{
			let t = render_target_create();
			t.name = "texpaint_picker";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			render_path_create_render_target(t);
		}
		{
			let t = render_target_create();
			t.name = "texpaint_nor_picker";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			render_path_create_render_target(t);
		}
		{
			let t = render_target_create();
			t.name = "texpaint_pack_picker";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			render_path_create_render_target(t);
		}
		{
			let t = render_target_create();
			t.name = "texpaint_uv_picker";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			render_path_create_render_target(t);
		}

		render_path_load_shader("shader_datas/copy_mrt3_pass/copy_mrt3_pass");
	}

	static commandsPaint = (dilation = true) => {
		let tid = "";

		if (Context.raw.pdirty > 0) {

			if (Context.raw.tool == WorkspaceTool.ToolPicker) {

					///if krom_metal
					// render_path_set_target("texpaint_picker");
					// render_path_clear_target(0xff000000);
					// render_path_set_target("texpaint_nor_picker");
					// render_path_clear_target(0xff000000);
					// render_path_set_target("texpaint_pack_picker");
					// render_path_clear_target(0xff000000);
					render_path_set_target("texpaint_picker", ["texpaint_nor_picker", "texpaint_pack_picker", "texpaint_uv_picker"]);
					///else
					render_path_set_target("texpaint_picker", ["texpaint_nor_picker", "texpaint_pack_picker", "texpaint_uv_picker"]);
					// render_path_clear_target(0xff000000);
					///end
					render_path_bind_target("gbuffer2", "gbuffer2");
					// tid = Context.raw.layer.id;
					render_path_bind_target("texpaint" + tid, "texpaint");
					render_path_bind_target("texpaint_nor" + tid, "texpaint_nor");
					render_path_bind_target("texpaint_pack" + tid, "texpaint_pack");
					render_path_draw_meshes("paint");
					UIHeader.headerHandle.redraws = 2;

					let texpaint_picker = render_path_render_targets.get("texpaint_picker").image;
					let texpaint_nor_picker = render_path_render_targets.get("texpaint_nor_picker").image;
					let texpaint_pack_picker = render_path_render_targets.get("texpaint_pack_picker").image;
					let texpaint_uv_picker = render_path_render_targets.get("texpaint_uv_picker").image;
					let a = image_get_pixels(texpaint_picker);
					let b = image_get_pixels(texpaint_nor_picker);
					let c = image_get_pixels(texpaint_pack_picker);
					let d = image_get_pixels(texpaint_uv_picker);

					if (Context.raw.colorPickerCallback != null) {
						Context.raw.colorPickerCallback(Context.raw.pickedColor);
					}

					// Picked surface values
					// ///if (krom_metal || krom_vulkan)
					// Context.raw.pickedColor.base.Rb = a.get(2);
					// Context.raw.pickedColor.base.Gb = a.get(1);
					// Context.raw.pickedColor.base.Bb = a.get(0);
					// Context.raw.pickedColor.normal.Rb = b.get(2);
					// Context.raw.pickedColor.normal.Gb = b.get(1);
					// Context.raw.pickedColor.normal.Bb = b.get(0);
					// Context.raw.pickedColor.occlusion = c.get(2) / 255;
					// Context.raw.pickedColor.roughness = c.get(1) / 255;
					// Context.raw.pickedColor.metallic = c.get(0) / 255;
					// Context.raw.pickedColor.height = c.get(3) / 255;
					// Context.raw.pickedColor.opacity = a.get(3) / 255;
					// Context.raw.uvxPicked = d.get(2) / 255;
					// Context.raw.uvyPicked = d.get(1) / 255;
					// ///else
					// Context.raw.pickedColor.base.Rb = a.get(0);
					// Context.raw.pickedColor.base.Gb = a.get(1);
					// Context.raw.pickedColor.base.Bb = a.get(2);
					// Context.raw.pickedColor.normal.Rb = b.get(0);
					// Context.raw.pickedColor.normal.Gb = b.get(1);
					// Context.raw.pickedColor.normal.Bb = b.get(2);
					// Context.raw.pickedColor.occlusion = c.get(0) / 255;
					// Context.raw.pickedColor.roughness = c.get(1) / 255;
					// Context.raw.pickedColor.metallic = c.get(2) / 255;
					// Context.raw.pickedColor.height = c.get(3) / 255;
					// Context.raw.pickedColor.opacity = a.get(3) / 255;
					// Context.raw.uvxPicked = d.get(0) / 255;
					// Context.raw.uvyPicked = d.get(1) / 255;
					// ///end
			}
			else {
				let texpaint = "texpaint_node_target";

				render_path_set_target("texpaint_blend1");
				render_path_bind_target("texpaint_blend0", "tex");
				render_path_draw_shader("shader_datas/copy_pass/copyR8_pass");

				render_path_set_target(texpaint, ["texpaint_nor" + tid, "texpaint_pack" + tid, "texpaint_blend0"]);

				render_path_bind_target("_main", "gbufferD");

				render_path_bind_target("texpaint_blend1", "paintmask");

				// Read texcoords from gbuffer
				let readTC = Context.raw.tool == WorkspaceTool.ToolClone ||
							 Context.raw.tool == WorkspaceTool.ToolBlur ||
							 Context.raw.tool == WorkspaceTool.ToolSmudge;
				if (readTC) {
					render_path_bind_target("gbuffer2", "gbuffer2");
				}

				render_path_draw_meshes("paint");
			}
		}
	}

	static commandsCursor = () => {
		let tool = Context.raw.tool;
		if (tool != WorkspaceTool.ToolEraser &&
			tool != WorkspaceTool.ToolClone &&
			tool != WorkspaceTool.ToolBlur &&
			tool != WorkspaceTool.ToolSmudge) {
			return;
		}

		let nodes = UINodes.getNodes();
		let canvas = UINodes.getCanvas(true);
		let inpaint = nodes.nodesSelectedId.length > 0 && zui_get_node(canvas.nodes, nodes.nodesSelectedId[0]).type == "InpaintNode";

		if (!Base.uiEnabled || Base.isDragging || !inpaint) {
			return;
		}

		let mx = Context.raw.paintVec.x;
		let my = 1.0 - Context.raw.paintVec.y;
		if (Context.raw.brushLocked) {
			mx = (Context.raw.lockStartedX - app_x()) / app_w();
			my = 1.0 - (Context.raw.lockStartedY - app_y()) / app_h();
		}
		let radius = Context.raw.brushRadius;
		RenderPathPaint.drawCursor(mx, my, radius / 3.4);
	}

	static drawCursor = (mx: f32, my: f32, radius: f32, tintR = 1.0, tintG = 1.0, tintB = 1.0) => {
		let plane = scene_get_child(".Plane").ext;
		let geom = plane.data;
		if (Base.pipeCursor == null) Base.makeCursorPipe();

		render_path_set_target("");
		g4_set_pipeline(Base.pipeCursor);
		let img = Res.get("cursor.k");
		g4_set_tex(Base.cursorTex, img);
		let gbuffer0 = render_path_render_targets.get("gbuffer0").image;
		g4_set_tex_depth(Base.cursorGbufferD, gbuffer0);
		g4_set_float2(Base.cursorMouse, mx, my);
		g4_set_float2(Base.cursorTexStep, 1 / gbuffer0.width, 1 / gbuffer0.height);
		g4_set_float(Base.cursorRadius, radius);
		let right = vec4_normalize(camera_object_right_world(scene_camera));
		g4_set_float3(Base.cursorCameraRight, right.x, right.y, right.z);
		g4_set_float3(Base.cursorTint, tintR, tintG, tintB);
		g4_set_mat(Base.cursorVP, scene_camera.vp);
		let helpMat = mat4_identity();
		mat4_get_inv(helpMat, scene_camera.vp);
		g4_set_mat(Base.cursorInvVP, helpMat);
		///if (krom_metal || krom_vulkan)
		g4_set_vertex_buffer(mesh_data_get(geom, [{name: "tex", data: "short2norm"}]));
		///else
		g4_set_vertex_buffer(geom._vertexBuffer);
		///end
		g4_set_index_buffer(geom._indexBuffers[0]);
		g4_draw();

		g4_disable_scissor();
		render_path_end();
	}

	static paintEnabled = (): bool => {
		return !Context.raw.foregroundEvent;
	}

	static begin = () => {
		if (!RenderPathPaint.paintEnabled()) return;
	}

	static end = () => {
		RenderPathPaint.commandsCursor();
		Context.raw.ddirty--;
		Context.raw.rdirty--;

		if (!RenderPathPaint.paintEnabled()) return;
		Context.raw.pdirty--;
	}

	static draw = () => {
		if (!RenderPathPaint.paintEnabled()) return;

		RenderPathPaint.commandsPaint();

		if (Context.raw.brushBlendDirty) {
			Context.raw.brushBlendDirty = false;
			///if krom_metal
			render_path_set_target("texpaint_blend0");
			render_path_clear_target(0x00000000);
			render_path_set_target("texpaint_blend1");
			render_path_clear_target(0x00000000);
			///else
			render_path_set_target("texpaint_blend0", ["texpaint_blend1"]);
			render_path_clear_target(0x00000000);
			///end
		}
	}

	static bindLayers = () => {
		let image: image_t = null;
		let nodes = UINodes.getNodes();
		let canvas = UINodes.getCanvas(true);
		if (nodes.nodesSelectedId.length > 0) {
			let node = zui_get_node(canvas.nodes, nodes.nodesSelectedId[0]);
			let brushNode = ParserLogic.getLogicNode(node);
			if (brushNode != null) {
				image = brushNode.getCachedImage();
			}
		}
		if (image != null) {
			if (render_path_render_targets.get("texpaint_node") == null) {
				let t = render_target_create();
				t.name = "texpaint_node";
				t.width = Config.getTextureResX();
				t.height = Config.getTextureResY();
				t.format = "RGBA32";
				render_path_render_targets.set(t.name, t);
			}
			if (render_path_render_targets.get("texpaint_node_target") == null) {
				let t = render_target_create();
				t.name = "texpaint_node_target";
				t.width = Config.getTextureResX();
				t.height = Config.getTextureResY();
				t.format = "RGBA32";
				render_path_render_targets.set(t.name, t);
			}
			render_path_render_targets.get("texpaint_node").image = image;
			render_path_bind_target("texpaint_node", "texpaint");
			render_path_bind_target("texpaint_nor_empty", "texpaint_nor");
			render_path_bind_target("texpaint_pack_empty", "texpaint_pack");

			let nodes = UINodes.getNodes();
			let canvas = UINodes.getCanvas(true);
			let node = zui_get_node(canvas.nodes, nodes.nodesSelectedId[0]);
			let inpaint = node.type == "InpaintNode";
			if (inpaint) {
				let brushNode = ParserLogic.getLogicNode(node);
				render_path_render_targets.get("texpaint_node_target").image = (brushNode as InpaintNode).getTarget();
			}
		}
		else {
			render_path_bind_target("texpaint", "texpaint");
			render_path_bind_target("texpaint_nor", "texpaint_nor");
			render_path_bind_target("texpaint_pack", "texpaint_pack");
		}
	}

	static unbindLayers = () => {

	}
}
