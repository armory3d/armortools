
///if (is_paint || is_sculpt)

class UtilRender {

	static materialPreviewSize = 256;
	static decalPreviewSize = 512;
	static layerPreviewSize = 200;
	static screenAlignedFullVB: VertexBufferRaw = null;
	static screenAlignedFullIB: IndexBufferRaw = null;

	static makeMaterialPreview = () => {
		Context.raw.materialPreview = true;

		let sphere: TMeshObject = Scene.getChild(".Sphere").ext;
		sphere.base.visible = true;
		let meshes = Scene.meshes;
		Scene.meshes = [sphere];
		let painto = Context.raw.paintObject;
		Context.raw.paintObject = sphere;

		sphere.materials[0] = Project.materials[0].data;
		Context.raw.material.previewReady = true;

		Mat4.setFrom(Context.raw.savedCamera, Scene.camera.base.transform.local);
		let m = Mat4.create(0.9146286343879498, -0.0032648027153306235, 0.404281837254303, 0.4659988049397712, 0.404295023959927, 0.007367569133732468, -0.9145989516155143, -1.0687517188018691, 0.000007410128652369705, 0.9999675337275382, 0.008058532943908717, 0.015935682577325486, 0, 0, 0, 1);
		Transform.setMatrix(Scene.camera.base.transform, m);
		let savedFov = Scene.camera.data.fov;
		Scene.camera.data.fov = 0.92;
		Viewport.updateCameraType(CameraType.CameraPerspective);
		let light = Scene.lights[0];
		let _lightStrength = light.data.strength;
		let probe = Scene.world;
		let _probeStrength = probe.strength;
		light.data.strength = 0;
		probe.strength = 7;
		let _envmapAngle = Context.raw.envmapAngle;
		Context.raw.envmapAngle = 6.0;
		let _brushScale = Context.raw.brushScale;
		Context.raw.brushScale = 1.5;
		let _brushNodesScale = Context.raw.brushNodesScale;
		Context.raw.brushNodesScale = 1.0;

		Scene.world._envmap = Context.raw.previewEnvmap;
		// No resize
		RenderPath.lastW = UtilRender.materialPreviewSize;
		RenderPath.lastH = UtilRender.materialPreviewSize;
		CameraObject.buildProjection(Scene.camera);
		CameraObject.buildMatrix(Scene.camera);

		MakeMaterial.parseMeshPreviewMaterial();
		let _commands = RenderPath.commands;
		RenderPath.commands = RenderPathPreview.commandsPreview;
		RenderPath.renderFrame(RenderPath.frameG);
		RenderPath.commands = _commands;

		Context.raw.materialPreview = false;
		RenderPath.lastW = App.w();
		RenderPath.lastH = App.h();

		// Restore
		sphere.base.visible = false;
		Scene.meshes = meshes;
		Context.raw.paintObject = painto;

		Transform.setMatrix(Scene.camera.base.transform, Context.raw.savedCamera);
		Viewport.updateCameraType(Context.raw.cameraType);
		Scene.camera.data.fov = savedFov;
		CameraObject.buildProjection(Scene.camera);
		CameraObject.buildMatrix(Scene.camera);
		light.data.strength = _lightStrength;
		probe.strength = _probeStrength;
		Context.raw.envmapAngle = _envmapAngle;
		Context.raw.brushScale = _brushScale;
		Context.raw.brushNodesScale = _brushNodesScale;
		Scene.world._envmap = Context.raw.showEnvmap ? Context.raw.savedEnvmap : Context.raw.emptyEnvmap;
		MakeMaterial.parseMeshMaterial();
		Context.raw.ddirty = 0;
	}

	static makeDecalPreview = () => {
		let current = Graphics2.current;
		if (current != null) Graphics2.end(current);

		if (Context.raw.decalImage == null) {
			Context.raw.decalImage = Image.createRenderTarget(UtilRender.decalPreviewSize, UtilRender.decalPreviewSize);
		}
		Context.raw.decalPreview = true;

		let plane: TMeshObject = Scene.getChild(".Plane").ext;
		Vec4.set(plane.base.transform.scale, 1, 1, 1);
		Quat.fromEuler(plane.base.transform.rot, -Math.PI / 2, 0, 0);
		Transform.buildMatrix(plane.base.transform);
		plane.base.visible = true;
		let meshes = Scene.meshes;
		Scene.meshes = [plane];
		let painto = Context.raw.paintObject;
		Context.raw.paintObject = plane;

		Mat4.setFrom(Context.raw.savedCamera, Scene.camera.base.transform.local);
		let m = Mat4.identity();
		Mat4.translate(m, 0, 0, 1);
		Transform.setMatrix(Scene.camera.base.transform, m);
		let savedFov = Scene.camera.data.fov;
		Scene.camera.data.fov = 0.92;
		Viewport.updateCameraType(CameraType.CameraPerspective);
		let light = Scene.lights[0];
		light.base.visible = false;
		Scene.world._envmap = Context.raw.previewEnvmap;

		// No resize
		RenderPath.lastW = UtilRender.decalPreviewSize;
		RenderPath.lastH = UtilRender.decalPreviewSize;
		CameraObject.buildProjection(Scene.camera);
		CameraObject.buildMatrix(Scene.camera);

		MakeMaterial.parseMeshPreviewMaterial();
		let _commands = RenderPath.commands;
		RenderPath.commands = RenderPathPreview.commandsDecal;
		RenderPath.renderFrame(RenderPath.frameG);
		RenderPath.commands = _commands;

		Context.raw.decalPreview = false;
		RenderPath.lastW = App.w();
		RenderPath.lastH = App.h();

		// Restore
		plane.base.visible = false;
		Scene.meshes = meshes;
		Context.raw.paintObject = painto;

		Transform.setMatrix(Scene.camera.base.transform, Context.raw.savedCamera);
		Scene.camera.data.fov = savedFov;
		Viewport.updateCameraType(Context.raw.cameraType);
		CameraObject.buildProjection(Scene.camera);
		CameraObject.buildMatrix(Scene.camera);
		light = Scene.lights[0];
		light.base.visible = true;
		Scene.world._envmap = Context.raw.showEnvmap ? Context.raw.savedEnvmap : Context.raw.emptyEnvmap;

		MakeMaterial.parseMeshMaterial();
		Context.raw.ddirty = 1; // Refresh depth for decal paint

		if (current != null) Graphics2.begin(current, false);
	}

	static makeTextPreview = () => {
		let current = Graphics2.current;
		if (current != null) Graphics2.end(current);

		let text = Context.raw.textToolText;
		let font = Context.raw.font.font;
		let fontSize = 200;
		let textW = Math.floor(Font.width(font, fontSize, text));
		let textH = Math.floor(Font.height(font, fontSize));
		let texW = textW + 32;
		if (texW < 512) texW = 512;
		if (Context.raw.textToolImage != null && Context.raw.textToolImage.width < texW) {
			Image.unload(Context.raw.textToolImage);
			Context.raw.textToolImage = null;
		}
		if (Context.raw.textToolImage == null) {
			///if krom_metal
			Context.raw.textToolImage = Image.createRenderTarget(texW, texW, TextureFormat.RGBA32);
			///else
			Context.raw.textToolImage = Image.createRenderTarget(texW, texW, TextureFormat.R8);
			///end
		}
		let g2 = Context.raw.textToolImage.g2;
		Graphics2.begin(g2, true, 0xff000000);
		g2.font = font;
		g2.fontSize = fontSize;
		g2.color = 0xffffffff;
		Graphics2.drawString(text, texW / 2 - textW / 2, texW / 2 - textH / 2);
		Graphics2.end(g2);

		if (current != null) Graphics2.begin(current, false);
	}

	static makeFontPreview = () => {
		let current = Graphics2.current;
		if (current != null) Graphics2.end(current);

		let text = "Abg";
		let font = Context.raw.font.font;
		let fontSize = 318;
		let textW = Math.floor(Font.width(font, fontSize, text)) + 8;
		let textH = Math.floor(Font.height(font, fontSize)) + 8;
		if (Context.raw.font.image == null) {
			Context.raw.font.image = Image.createRenderTarget(512, 512, TextureFormat.RGBA32);
		}
		let g2 = Context.raw.font.image.g2;
		Graphics2.begin(g2, true, 0x00000000);
		g2.font = font;
		g2.fontSize = fontSize;
		g2.color = 0xffffffff;
		Graphics2.drawString(text, 512 / 2 - textW / 2, 512 / 2 - textH / 2);
		Graphics2.end(g2);
		Context.raw.font.previewReady = true;

		if (current != null) Graphics2.begin(current, false);
	}

	static makeBrushPreview = () => {
		if (RenderPathPaint.liveLayerLocked) return;
		Context.raw.materialPreview = true;

		let current = Graphics2.current;
		if (current != null) Graphics2.end(current);

		// Prepare layers
		if (RenderPathPaint.liveLayer == null) {
			RenderPathPaint.liveLayer = SlotLayer.create("_live");
		}

		let l = RenderPathPaint.liveLayer;
		SlotLayer.clear(l);

		if (Context.raw.brush.image == null) {
			Context.raw.brush.image = Image.createRenderTarget(UtilRender.materialPreviewSize, UtilRender.materialPreviewSize);
			Context.raw.brush.imageIcon = Image.createRenderTarget(50, 50);
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
		RenderPath.renderTargets.set("texpaint_undo" + hid, RenderPath.renderTargets.get("empty_black"));

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

		let cam = Scene.camera;
		Mat4.setFrom(Context.raw.savedCamera, cam.base.transform.local);
		let savedFov = cam.data.fov;
		Viewport.updateCameraType(CameraType.CameraPerspective);
		let m = Mat4.identity();
		Mat4.translate(m, 0, 0, 0.5);
		Transform.setMatrix(cam.base.transform, m);
		cam.data.fov = 0.92;
		CameraObject.buildProjection(cam);
		CameraObject.buildMatrix(cam);
		Mat4.getInverse(m, Scene.camera.VP);

		let planeo: TMeshObject = Scene.getChild(".Plane").ext;
		planeo.base.visible = true;
		Context.raw.paintObject = planeo;

		let v = Vec4.create();
		let sx = Vec4.vec4_length(Vec4.set(v, m._00, m._01, m._02));
		Quat.fromEuler(planeo.base.transform.rot, -Math.PI / 2, 0, 0);
		Vec4.set(planeo.base.transform.scale, sx, 1.0, sx);
		Vec4.set(planeo.base.transform.loc, m._30, -m._31, 0.0);
		Transform.buildMatrix(planeo.base.transform);

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
		App.notifyOnInit(_init);

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
		Transform.setMatrix(Scene.camera.base.transform, Context.raw.savedCamera);
		Scene.camera.data.fov = savedFov;
		Viewport.updateCameraType(Context.raw.cameraType);
		CameraObject.buildProjection(Scene.camera);
		CameraObject.buildMatrix(Scene.camera);

		// Scale layer down to to image preview
		if (Base.pipeMerge == null) Base.makePipe();
		l = RenderPathPaint.liveLayer;
		let target = Context.raw.brush.image;
		Graphics2.begin(target.g2, true, 0x00000000);
		target.g2.pipeline = Base.pipeCopy;
		Graphics2.drawScaledImage(l.texpaint, 0, 0, target.width, target.height);
		target.g2.pipeline = null;
		Graphics2.end(target.g2);

		// Scale image preview down to to icon
		RenderPath.renderTargets.get("texpreview").image = Context.raw.brush.image;
		RenderPath.renderTargets.get("texpreview_icon").image = Context.raw.brush.imageIcon;
		RenderPath.setTarget("texpreview_icon");
		RenderPath.bindTarget("texpreview", "tex");
		RenderPath.drawShader("shader_datas/supersample_resolve/supersample_resolve");

		Context.raw.brush.previewReady = true;
		Context.raw.brushBlendDirty = true;

		if (current != null) Graphics2.begin(current, false);
	}

	static makeNodePreview = (canvas: TNodeCanvas, node: TNode, image: ImageRaw, group: TNodeCanvas = null, parents: TNode[] = null) => {
		let res = MakeMaterial.parseNodePreviewMaterial(node, group, parents);
		if (res == null || res.scon == null) return;

		let g4 = image.g4;
		if (UtilRender.screenAlignedFullVB == null) {
			UtilRender.createScreenAlignedFullData();
		}

		let _scaleWorld = Context.raw.paintObject.base.transform.scaleWorld;
		Context.raw.paintObject.base.transform.scaleWorld = 3.0;
		Transform.buildMatrix(Context.raw.paintObject.base.transform);

		Graphics4.begin(g4);
		Graphics4.setPipeline(res.scon._pipeState);
		Uniforms.setContextConstants(g4, res.scon, [""]);
		Uniforms.setObjectConstants(g4, res.scon, Context.raw.paintObject.base);
		Uniforms.setMaterialConstants(g4, res.scon, res.mcon);
		Graphics4.setVertexBuffer(UtilRender.screenAlignedFullVB);
		Graphics4.setIndexBuffer(UtilRender.screenAlignedFullIB);
		Graphics4.drawIndexedVertices();
		Graphics4.end();

		Context.raw.paintObject.base.transform.scaleWorld = _scaleWorld;
		Transform.buildMatrix(Context.raw.paintObject.base.transform);
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

	static getDecalMat = (): TMat4 => {
		UtilRender.pickPosNorTex();
		let decalMat = Mat4.identity();
		let loc = Vec4.create(Context.raw.posXPicked, Context.raw.posYPicked, Context.raw.posZPicked);
		let rot = Quat.fromTo(Quat.create(), Vec4.create(0.0, 0.0, -1.0), Vec4.create(Context.raw.norXPicked, Context.raw.norYPicked, Context.raw.norZPicked));
		let scale = Vec4.create(Context.raw.brushRadius * 0.5, Context.raw.brushRadius * 0.5, Context.raw.brushRadius * 0.5);
		Mat4.compose(decalMat, loc, rot, scale);
		return decalMat;
	}

	static createScreenAlignedFullData = () => {
		// Over-sized triangle
		let data = [-Math.floor(32767 / 3), -Math.floor(32767 / 3), 0, 32767, 0, 0, 0, 0, 0, 0, 0, 0,
					 32767,              -Math.floor(32767 / 3), 0, 32767, 0, 0, 0, 0, 0, 0, 0, 0,
					-Math.floor(32767 / 3),  32767,              0, 32767, 0, 0, 0, 0, 0, 0, 0, 0];
		let indices = [0, 1, 2];

		// Mandatory vertex data names and sizes
		let structure = VertexStructure.create();
		VertexStructure.add(structure, "pos", VertexData.I16_4X_Normalized);
		VertexStructure.add(structure, "nor", VertexData.I16_2X_Normalized);
		VertexStructure.add(structure, "tex", VertexData.I16_2X_Normalized);
		VertexStructure.add(structure, "col", VertexData.I16_4X_Normalized);
		UtilRender.screenAlignedFullVB = VertexBuffer.create(Math.floor(data.length / Math.floor(VertexStructure.byteSize(structure) / 4)), structure, Usage.StaticUsage);
		let vertices = VertexBuffer.lock(UtilRender.screenAlignedFullVB);
		for (let i = 0; i < Math.floor(vertices.byteLength / 2); ++i) vertices.setInt16(i * 2, data[i], true);
		VertexBuffer.unlock(UtilRender.screenAlignedFullVB);

		UtilRender.screenAlignedFullIB = IndexBuffer.create(indices.length, Usage.StaticUsage);
		let id = IndexBuffer.lock(UtilRender.screenAlignedFullIB);
		for (let i = 0; i < id.length; ++i) id[i] = indices[i];
		IndexBuffer.unlock(UtilRender.screenAlignedFullIB);
	}
}

///end
