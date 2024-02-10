
///if (is_paint || is_sculpt)

class UtilRender {

	static materialPreviewSize = 256;
	static decalPreviewSize = 512;
	static layerPreviewSize = 200;
	static screenAlignedFullVB: vertex_buffer_t = null;
	static screenAlignedFullIB: index_buffer_t = null;

	static makeMaterialPreview = () => {
		Context.raw.materialPreview = true;

		let sphere: mesh_object_t = scene_get_child(".Sphere").ext;
		sphere.base.visible = true;
		let meshes = scene_meshes;
		scene_meshes = [sphere];
		let painto = Context.raw.paintObject;
		Context.raw.paintObject = sphere;

		sphere.materials[0] = Project.materials[0].data;
		Context.raw.material.previewReady = true;

		mat4_set_from(Context.raw.savedCamera, scene_camera.base.transform.local);
		let m = mat4_create(0.9146286343879498, -0.0032648027153306235, 0.404281837254303, 0.4659988049397712, 0.404295023959927, 0.007367569133732468, -0.9145989516155143, -1.0687517188018691, 0.000007410128652369705, 0.9999675337275382, 0.008058532943908717, 0.015935682577325486, 0, 0, 0, 1);
		transform_set_matrix(scene_camera.base.transform, m);
		let savedFov = scene_camera.data.fov;
		scene_camera.data.fov = 0.92;
		Viewport.updateCameraType(CameraType.CameraPerspective);
		let light = scene_lights[0];
		let _lightStrength = light.data.strength;
		let probe = scene_world;
		let _probeStrength = probe.strength;
		light.data.strength = 0;
		probe.strength = 7;
		let _envmapAngle = Context.raw.envmapAngle;
		Context.raw.envmapAngle = 6.0;
		let _brushScale = Context.raw.brushScale;
		Context.raw.brushScale = 1.5;
		let _brushNodesScale = Context.raw.brushNodesScale;
		Context.raw.brushNodesScale = 1.0;

		scene_world._envmap = Context.raw.previewEnvmap;
		// No resize
		_render_path_last_w = UtilRender.materialPreviewSize;
		_render_path_last_h = UtilRender.materialPreviewSize;
		camera_object_build_proj(scene_camera);
		camera_object_build_mat(scene_camera);

		MakeMaterial.parseMeshPreviewMaterial();
		let _commands = render_path_commands;
		render_path_commands = RenderPathPreview.commandsPreview;
		render_path_render_frame();
		render_path_commands = _commands;

		Context.raw.materialPreview = false;
		_render_path_last_w = app_w();
		_render_path_last_h = app_h();

		// Restore
		sphere.base.visible = false;
		scene_meshes = meshes;
		Context.raw.paintObject = painto;

		transform_set_matrix(scene_camera.base.transform, Context.raw.savedCamera);
		Viewport.updateCameraType(Context.raw.cameraType);
		scene_camera.data.fov = savedFov;
		camera_object_build_proj(scene_camera);
		camera_object_build_mat(scene_camera);
		light.data.strength = _lightStrength;
		probe.strength = _probeStrength;
		Context.raw.envmapAngle = _envmapAngle;
		Context.raw.brushScale = _brushScale;
		Context.raw.brushNodesScale = _brushNodesScale;
		scene_world._envmap = Context.raw.showEnvmap ? Context.raw.savedEnvmap : Context.raw.emptyEnvmap;
		MakeMaterial.parseMeshMaterial();
		Context.raw.ddirty = 0;
	}

	static makeDecalPreview = () => {
		let current = _g2_current;
		if (current != null) g2_end();

		if (Context.raw.decalImage == null) {
			Context.raw.decalImage = image_create_render_target(UtilRender.decalPreviewSize, UtilRender.decalPreviewSize);
		}
		Context.raw.decalPreview = true;

		let plane: mesh_object_t = scene_get_child(".Plane").ext;
		vec4_set(plane.base.transform.scale, 1, 1, 1);
		quat_from_euler(plane.base.transform.rot, -Math.PI / 2, 0, 0);
		transform_build_matrix(plane.base.transform);
		plane.base.visible = true;
		let meshes = scene_meshes;
		scene_meshes = [plane];
		let painto = Context.raw.paintObject;
		Context.raw.paintObject = plane;

		mat4_set_from(Context.raw.savedCamera, scene_camera.base.transform.local);
		let m = mat4_identity();
		mat4_translate(m, 0, 0, 1);
		transform_set_matrix(scene_camera.base.transform, m);
		let savedFov = scene_camera.data.fov;
		scene_camera.data.fov = 0.92;
		Viewport.updateCameraType(CameraType.CameraPerspective);
		let light = scene_lights[0];
		light.base.visible = false;
		scene_world._envmap = Context.raw.previewEnvmap;

		// No resize
		_render_path_last_w = UtilRender.decalPreviewSize;
		_render_path_last_h = UtilRender.decalPreviewSize;
		camera_object_build_proj(scene_camera);
		camera_object_build_mat(scene_camera);

		MakeMaterial.parseMeshPreviewMaterial();
		let _commands = render_path_commands;
		render_path_commands = RenderPathPreview.commandsDecal;
		render_path_render_frame();
		render_path_commands = _commands;

		Context.raw.decalPreview = false;
		_render_path_last_w = app_w();
		_render_path_last_h = app_h();

		// Restore
		plane.base.visible = false;
		scene_meshes = meshes;
		Context.raw.paintObject = painto;

		transform_set_matrix(scene_camera.base.transform, Context.raw.savedCamera);
		scene_camera.data.fov = savedFov;
		Viewport.updateCameraType(Context.raw.cameraType);
		camera_object_build_proj(scene_camera);
		camera_object_build_mat(scene_camera);
		light = scene_lights[0];
		light.base.visible = true;
		scene_world._envmap = Context.raw.showEnvmap ? Context.raw.savedEnvmap : Context.raw.emptyEnvmap;

		MakeMaterial.parseMeshMaterial();
		Context.raw.ddirty = 1; // Refresh depth for decal paint

		if (current != null) g2_begin(current, false);
	}

	static makeTextPreview = () => {
		let current = _g2_current;
		if (current != null) g2_end();

		let text = Context.raw.textToolText;
		let font = Context.raw.font.font;
		let fontSize = 200;
		let textW = Math.floor(g2_font_width(font, fontSize, text));
		let textH = Math.floor(g2_font_height(font, fontSize));
		let texW = textW + 32;
		if (texW < 512) texW = 512;
		if (Context.raw.textToolImage != null && Context.raw.textToolImage.width < texW) {
			image_unload(Context.raw.textToolImage);
			Context.raw.textToolImage = null;
		}
		if (Context.raw.textToolImage == null) {
			///if krom_metal
			Context.raw.textToolImage = image_create_render_target(texW, texW, tex_format_t.RGBA32);
			///else
			Context.raw.textToolImage = image_create_render_target(texW, texW, tex_format_t.R8);
			///end
		}
		g2_begin(Context.raw.textToolImage, true, 0xff000000);
		g2_set_font(font);
		g2_set_font_size(fontSize);
		g2_set_color(0xffffffff);
		g2_draw_string(text, texW / 2 - textW / 2, texW / 2 - textH / 2);
		g2_end();

		if (current != null) g2_begin(current, false);
	}

	static makeFontPreview = () => {
		let current = _g2_current;
		if (current != null) g2_end();

		let text = "Abg";
		let font = Context.raw.font.font;
		let fontSize = 318;
		let textW = Math.floor(g2_font_width(font, fontSize, text)) + 8;
		let textH = Math.floor(g2_font_height(font, fontSize)) + 8;
		if (Context.raw.font.image == null) {
			Context.raw.font.image = image_create_render_target(512, 512, tex_format_t.RGBA32);
		}
		g2_begin(Context.raw.font.image, true, 0x00000000);
		g2_set_font(font);
		g2_set_font_size(fontSize);
		g2_set_color(0xffffffff);
		g2_draw_string(text, 512 / 2 - textW / 2, 512 / 2 - textH / 2);
		g2_end();
		Context.raw.font.previewReady = true;

		if (current != null) g2_begin(current, false);
	}

	static makeBrushPreview = () => {
		if (RenderPathPaint.liveLayerLocked) return;
		Context.raw.materialPreview = true;

		let current = _g2_current;
		if (current != null) g2_end();

		// Prepare layers
		if (RenderPathPaint.liveLayer == null) {
			RenderPathPaint.liveLayer = SlotLayer.create("_live");
		}

		let l = RenderPathPaint.liveLayer;
		SlotLayer.clear(l);

		if (Context.raw.brush.image == null) {
			Context.raw.brush.image = image_create_render_target(UtilRender.materialPreviewSize, UtilRender.materialPreviewSize);
			Context.raw.brush.imageIcon = image_create_render_target(50, 50);
		}

		let _material = Context.raw.material;
		Context.raw.material = SlotMaterial.create();
		let _tool = Context.raw.tool;
		Context.raw.tool = WorkspaceTool.ToolBrush;

		let _layer = Context.raw.layer;
		if (SlotLayer.isMask(Context.raw.layer)) {
			Context.raw.layer = Context.raw.layer.parent;
		}

		let _fill_layer = Context.raw.layer.fill_layer;
		Context.raw.layer.fill_layer = null;

		RenderPathPaint.useLiveLayer(true);
		MakeMaterial.parsePaintMaterial(false);

		let hid = History.undoI - 1 < 0 ? Config.raw.undo_steps - 1 : History.undoI - 1;
		render_path_render_targets.set("texpaint_undo" + hid,render_path_render_targets.get("empty_black"));

		// Set plane mesh
		let painto = Context.raw.paintObject;
		let visibles: bool[] = [];
		for (let p of Project.paintObjects) {
			visibles.push(p.base.visible);
			p.base.visible = false;
		}
		let mergedObjectVisible = false;
		if (Context.raw.mergedObject != null) {
			mergedObjectVisible = Context.raw.mergedObject.base.visible;
			Context.raw.mergedObject.base.visible = false;
		}

		let cam = scene_camera;
		mat4_set_from(Context.raw.savedCamera, cam.base.transform.local);
		let savedFov = cam.data.fov;
		Viewport.updateCameraType(CameraType.CameraPerspective);
		let m = mat4_identity();
		mat4_translate(m, 0, 0, 0.5);
		transform_set_matrix(cam.base.transform, m);
		cam.data.fov = 0.92;
		camera_object_build_proj(cam);
		camera_object_build_mat(cam);
		mat4_get_inv(m, scene_camera.vp);

		let planeo: mesh_object_t = scene_get_child(".Plane").ext;
		planeo.base.visible = true;
		Context.raw.paintObject = planeo;

		let v = vec4_create();
		let sx = vec4_len(vec4_set(v, m.m[0], m.m[1], m.m[2]));
		quat_from_euler(planeo.base.transform.rot, -Math.PI / 2, 0, 0);
		vec4_set(planeo.base.transform.scale, sx, 1.0, sx);
		vec4_set(planeo.base.transform.loc, m.m[12], -m.m[13], 0.0);
		transform_build_matrix(planeo.base.transform);

		RenderPathPaint.liveLayerDrawn = 0;
		RenderPathBase.drawGbuffer();

		// Paint brush preview
		let _brushRadius = Context.raw.brushRadius;
		let _brushOpacity = Context.raw.brushOpacity;
		let _brushHardness = Context.raw.brushHardness;
		Context.raw.brushRadius = 0.33;
		Context.raw.brushOpacity = 1.0;
		Context.raw.brushHardness = 0.8;
		let _x = Context.raw.paintVec.x;
		let _y = Context.raw.paintVec.y;
		let _lastX = Context.raw.lastPaintVecX;
		let _lastY = Context.raw.lastPaintVecY;
		let _pdirty = Context.raw.pdirty;
		Context.raw.pdirty = 2;

		let pointsX = [0.2, 0.2,  0.35, 0.5,  0.5, 0.5,  0.65, 0.8,  0.8, 0.8];
		let pointsY = [0.5, 0.5,  0.35 - 0.04, 0.2 - 0.08,  0.4 + 0.015, 0.6 + 0.03,  0.45 - 0.025, 0.3 - 0.05,  0.5 + 0.025, 0.7 + 0.05];
		for (let i = 1; i < pointsX.length; ++i) {
			Context.raw.lastPaintVecX = pointsX[i - 1];
			Context.raw.lastPaintVecY = pointsY[i - 1];
			Context.raw.paintVec.x = pointsX[i];
			Context.raw.paintVec.y = pointsY[i];
			RenderPathPaint.commandsPaint(false);
		}

		Context.raw.brushRadius = _brushRadius;
		Context.raw.brushOpacity = _brushOpacity;
		Context.raw.brushHardness = _brushHardness;
		Context.raw.paintVec.x = _x;
		Context.raw.paintVec.y = _y;
		Context.raw.lastPaintVecX = _lastX;
		Context.raw.lastPaintVecY = _lastY;
		Context.raw.prevPaintVecX = -1;
		Context.raw.prevPaintVecY = -1;
		Context.raw.pdirty = _pdirty;
		RenderPathPaint.useLiveLayer(false);
		Context.raw.layer.fill_layer = _fill_layer;
		Context.raw.layer = _layer;
		Context.raw.material = _material;
		Context.raw.tool = _tool;
		let _init = () => {
			MakeMaterial.parsePaintMaterial(false);
		}
		app_notify_on_init(_init);

		// Restore paint mesh
		Context.raw.materialPreview = false;
		planeo.base.visible = false;
		for (let i = 0; i < Project.paintObjects.length; ++i) {
			Project.paintObjects[i].base.visible = visibles[i];
		}
		if (Context.raw.mergedObject != null) {
			Context.raw.mergedObject.base.visible = mergedObjectVisible;
		}
		Context.raw.paintObject = painto;
		transform_set_matrix(scene_camera.base.transform, Context.raw.savedCamera);
		scene_camera.data.fov = savedFov;
		Viewport.updateCameraType(Context.raw.cameraType);
		camera_object_build_proj(scene_camera);
		camera_object_build_mat(scene_camera);

		// Scale layer down to to image preview
		if (Base.pipeMerge == null) Base.makePipe();
		l = RenderPathPaint.liveLayer;
		let target = Context.raw.brush.image;
		g2_begin(target, true, 0x00000000);
		g2_set_pipeline(Base.pipeCopy);
		g2_draw_scaled_image(l.texpaint, 0, 0, target.width, target.height);
		g2_set_pipeline(null);
		g2_end();

		// Scale image preview down to to icon
		render_path_render_targets.get("texpreview").image = Context.raw.brush.image;
		render_path_render_targets.get("texpreview_icon").image = Context.raw.brush.imageIcon;
		render_path_set_target("texpreview_icon");
		render_path_bind_target("texpreview", "tex");
		render_path_draw_shader("shader_datas/supersample_resolve/supersample_resolve");

		Context.raw.brush.previewReady = true;
		Context.raw.brushBlendDirty = true;

		if (current != null) g2_begin(current, false);
	}

	static makeNodePreview = (canvas: zui_node_canvas_t, node: zui_node_t, image: image_t, group: zui_node_canvas_t = null, parents: zui_node_t[] = null) => {
		let res = MakeMaterial.parseNodePreviewMaterial(node, group, parents);
		if (res == null || res.scon == null) return;

		if (UtilRender.screenAlignedFullVB == null) {
			UtilRender.createScreenAlignedFullData();
		}

		let _scaleWorld = Context.raw.paintObject.base.transform.scale_world;
		Context.raw.paintObject.base.transform.scale_world = 3.0;
		transform_build_matrix(Context.raw.paintObject.base.transform);

		g4_begin(image);
		g4_set_pipeline(res.scon._pipe_state);
		uniforms_set_context_consts(res.scon, [""]);
		uniforms_set_obj_consts(res.scon, Context.raw.paintObject.base);
		uniforms_set_material_consts(res.scon, res.mcon);
		g4_set_vertex_buffer(UtilRender.screenAlignedFullVB);
		g4_set_index_buffer(UtilRender.screenAlignedFullIB);
		g4_draw();
		g4_end();

		Context.raw.paintObject.base.transform.scale_world = _scaleWorld;
		transform_build_matrix(Context.raw.paintObject.base.transform);
	}

	static pickPosNorTex = () => {
		Context.raw.pickPosNorTex = true;
		Context.raw.pdirty = 1;
		let _tool = Context.raw.tool;
		Context.raw.tool = WorkspaceTool.ToolPicker;
		MakeMaterial.parsePaintMaterial();
		if (Context.raw.paint2d) {
			RenderPathPaint.setPlaneMesh();
		}
		RenderPathPaint.commandsPaint(false);
		///if krom_metal
		// Flush command list
		RenderPathPaint.commandsPaint(false);
		///end
		if (Context.raw.paint2d) {
			RenderPathPaint.restorePlaneMesh();
		}
		Context.raw.tool = _tool;
		Context.raw.pickPosNorTex = false;
		MakeMaterial.parsePaintMaterial();
		Context.raw.pdirty = 0;
	}

	static getDecalMat = (): mat4_t => {
		UtilRender.pickPosNorTex();
		let decalMat = mat4_identity();
		let loc = vec4_create(Context.raw.posXPicked, Context.raw.posYPicked, Context.raw.posZPicked);
		let rot = quat_from_to(quat_create(), vec4_create(0.0, 0.0, -1.0), vec4_create(Context.raw.norXPicked, Context.raw.norYPicked, Context.raw.norZPicked));
		let scale = vec4_create(Context.raw.brushRadius * 0.5, Context.raw.brushRadius * 0.5, Context.raw.brushRadius * 0.5);
		mat4_compose(decalMat, loc, rot, scale);
		return decalMat;
	}

	static createScreenAlignedFullData = () => {
		// Over-sized triangle
		let data = [-Math.floor(32767 / 3), -Math.floor(32767 / 3), 0, 32767, 0, 0, 0, 0, 0, 0, 0, 0,
					 32767,              -Math.floor(32767 / 3), 0, 32767, 0, 0, 0, 0, 0, 0, 0, 0,
					-Math.floor(32767 / 3),  32767,              0, 32767, 0, 0, 0, 0, 0, 0, 0, 0];
		let indices = [0, 1, 2];

		// Mandatory vertex data names and sizes
		let structure = g4_vertex_struct_create();
		g4_vertex_struct_add(structure, "pos", vertex_data_t.I16_4X_NORM);
		g4_vertex_struct_add(structure, "nor", vertex_data_t.I16_2X_NORM);
		g4_vertex_struct_add(structure, "tex", vertex_data_t.I16_2X_NORM);
		g4_vertex_struct_add(structure, "col", vertex_data_t.I16_4X_NORM);
		UtilRender.screenAlignedFullVB = g4_vertex_buffer_create(Math.floor(data.length / Math.floor(g4_vertex_struct_byte_size(structure) / 4)), structure, usage_t.STATIC);
		let vertices = g4_vertex_buffer_lock(UtilRender.screenAlignedFullVB);
		for (let i = 0; i < Math.floor(vertices.byteLength / 2); ++i) vertices.setInt16(i * 2, data[i], true);
		g4_vertex_buffer_unlock(UtilRender.screenAlignedFullVB);

		UtilRender.screenAlignedFullIB = g4_index_buffer_create(indices.length);
		let id = g4_index_buffer_lock(UtilRender.screenAlignedFullIB);
		for (let i = 0; i < id.length; ++i) id[i] = indices[i];
		g4_index_buffer_unlock(UtilRender.screenAlignedFullIB);
	}
}

///end
