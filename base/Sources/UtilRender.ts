
///if (is_paint || is_sculpt)

class UtilRender {

	static materialPreviewSize = 256;
	static decalPreviewSize = 512;
	static layerPreviewSize = 200;
	static screenAlignedFullVB: VertexBuffer = null;
	static screenAlignedFullIB: IndexBuffer = null;

	static makeMaterialPreview = () => {
		Context.raw.materialPreview = true;

		let sphere: MeshObject = Scene.active.getChild(".Sphere") as MeshObject;
		sphere.visible = true;
		let meshes = Scene.active.meshes;
		Scene.active.meshes = [sphere];
		let painto = Context.raw.paintObject;
		Context.raw.paintObject = sphere;

		sphere.materials[0] = Project.materials[0].data;
		Context.raw.material.previewReady = true;

		Context.raw.savedCamera.setFrom(Scene.active.camera.transform.local);
		let m = new Mat4(0.9146286343879498, -0.0032648027153306235, 0.404281837254303, 0.4659988049397712, 0.404295023959927, 0.007367569133732468, -0.9145989516155143, -1.0687517188018691, 0.000007410128652369705, 0.9999675337275382, 0.008058532943908717, 0.015935682577325486, 0, 0, 0, 1);
		Scene.active.camera.transform.setMatrix(m);
		let savedFov = Scene.active.camera.data.raw.fov;
		Scene.active.camera.data.raw.fov = 0.92;
		Viewport.updateCameraType(CameraType.CameraPerspective);
		let light = Scene.active.lights[0];
		let _lightStrength = light.data.raw.strength;
		let probe = Scene.active.world.probe;
		let _probeStrength = probe.raw.strength;
		light.data.raw.strength = 0;
		probe.raw.strength = 7;
		let _envmapAngle = Context.raw.envmapAngle;
		Context.raw.envmapAngle = 6.0;
		let _brushScale = Context.raw.brushScale;
		Context.raw.brushScale = 1.5;
		let _brushNodesScale = Context.raw.brushNodesScale;
		Context.raw.brushNodesScale = 1.0;

		Scene.active.world.envmap = Context.raw.previewEnvmap;
		// No resize
		RenderPath.active.lastW = UtilRender.materialPreviewSize;
		RenderPath.active.lastH = UtilRender.materialPreviewSize;
		Scene.active.camera.buildProjection();
		Scene.active.camera.buildMatrix();

		MakeMaterial.parseMeshPreviewMaterial();
		let _commands = RenderPath.active.commands;
		RenderPath.active.commands = RenderPathPreview.commandsPreview;
		RenderPath.active.renderFrame(RenderPath.active.frameG);
		RenderPath.active.commands = _commands;

		Context.raw.materialPreview = false;
		RenderPath.active.lastW = App.w();
		RenderPath.active.lastH = App.h();

		// Restore
		sphere.visible = false;
		Scene.active.meshes = meshes;
		Context.raw.paintObject = painto;

		Scene.active.camera.transform.setMatrix(Context.raw.savedCamera);
		Viewport.updateCameraType(Context.raw.cameraType);
		Scene.active.camera.data.raw.fov = savedFov;
		Scene.active.camera.buildProjection();
		Scene.active.camera.buildMatrix();
		light.data.raw.strength = _lightStrength;
		probe.raw.strength = _probeStrength;
		Context.raw.envmapAngle = _envmapAngle;
		Context.raw.brushScale = _brushScale;
		Context.raw.brushNodesScale = _brushNodesScale;
		Scene.active.world.envmap = Context.raw.showEnvmap ? Context.raw.savedEnvmap : Context.raw.emptyEnvmap;
		MakeMaterial.parseMeshMaterial();
		Context.raw.ddirty = 0;
	}

	static makeDecalPreview = () => {
		let current = Graphics2.current;
		if (current != null) current.end();

		if (Context.raw.decalImage == null) {
			Context.raw.decalImage = Image.createRenderTarget(UtilRender.decalPreviewSize, UtilRender.decalPreviewSize);
		}
		Context.raw.decalPreview = true;

		let plane: MeshObject = Scene.active.getChild(".Plane") as MeshObject;
		plane.transform.scale.set(1, 1, 1);
		plane.transform.rot.fromEuler(-Math.PI / 2, 0, 0);
		plane.transform.buildMatrix();
		plane.visible = true;
		let meshes = Scene.active.meshes;
		Scene.active.meshes = [plane];
		let painto = Context.raw.paintObject;
		Context.raw.paintObject = plane;

		Context.raw.savedCamera.setFrom(Scene.active.camera.transform.local);
		let m = Mat4.identity();
		m.translate(0, 0, 1);
		Scene.active.camera.transform.setMatrix(m);
		let savedFov = Scene.active.camera.data.raw.fov;
		Scene.active.camera.data.raw.fov = 0.92;
		Viewport.updateCameraType(CameraType.CameraPerspective);
		let light = Scene.active.lights[0];
		light.visible = false;
		Scene.active.world.envmap = Context.raw.previewEnvmap;

		// No resize
		RenderPath.active.lastW = UtilRender.decalPreviewSize;
		RenderPath.active.lastH = UtilRender.decalPreviewSize;
		Scene.active.camera.buildProjection();
		Scene.active.camera.buildMatrix();

		MakeMaterial.parseMeshPreviewMaterial();
		let _commands = RenderPath.active.commands;
		RenderPath.active.commands = RenderPathPreview.commandsDecal;
		RenderPath.active.renderFrame(RenderPath.active.frameG);
		RenderPath.active.commands = _commands;

		Context.raw.decalPreview = false;
		RenderPath.active.lastW = App.w();
		RenderPath.active.lastH = App.h();

		// Restore
		plane.visible = false;
		Scene.active.meshes = meshes;
		Context.raw.paintObject = painto;

		Scene.active.camera.transform.setMatrix(Context.raw.savedCamera);
		Scene.active.camera.data.raw.fov = savedFov;
		Viewport.updateCameraType(Context.raw.cameraType);
		Scene.active.camera.buildProjection();
		Scene.active.camera.buildMatrix();
		light = Scene.active.lights[0];
		light.visible = true;
		Scene.active.world.envmap = Context.raw.showEnvmap ? Context.raw.savedEnvmap : Context.raw.emptyEnvmap;

		MakeMaterial.parseMeshMaterial();
		Context.raw.ddirty = 1; // Refresh depth for decal paint

		if (current != null) current.begin(false);
	}

	static makeTextPreview = () => {
		let current = Graphics2.current;
		if (current != null) current.end();

		let text = Context.raw.textToolText;
		let font = Context.raw.font.font;
		let fontSize = 200;
		let textW = Math.floor(font.width(fontSize, text));
		let textH = Math.floor(font.height(fontSize));
		let texW = textW + 32;
		if (texW < 512) texW = 512;
		if (Context.raw.textToolImage != null && Context.raw.textToolImage.width < texW) {
			Context.raw.textToolImage.unload();
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
		g2.begin(true, 0xff000000);
		g2.font = font;
		g2.fontSize = fontSize;
		g2.color = 0xffffffff;
		g2.drawString(text, texW / 2 - textW / 2, texW / 2 - textH / 2);
		g2.end();

		if (current != null) current.begin(false);
	}

	static makeFontPreview = () => {
		let current = Graphics2.current;
		if (current != null) current.end();

		let text = "Abg";
		let font = Context.raw.font.font;
		let fontSize = 318;
		let textW = Math.floor(font.width(fontSize, text)) + 8;
		let textH = Math.floor(font.height(fontSize)) + 8;
		if (Context.raw.font.image == null) {
			Context.raw.font.image = Image.createRenderTarget(512, 512, TextureFormat.RGBA32);
		}
		let g2 = Context.raw.font.image.g2;
		g2.begin(true, 0x00000000);
		g2.font = font;
		g2.fontSize = fontSize;
		g2.color = 0xffffffff;
		g2.drawString(text, 512 / 2 - textW / 2, 512 / 2 - textH / 2);
		g2.end();
		Context.raw.font.previewReady = true;

		if (current != null) current.begin(false);
	}

	static makeBrushPreview = () => {
		if (RenderPathPaint.liveLayerLocked) return;
		Context.raw.materialPreview = true;

		let current = Graphics2.current;
		if (current != null) current.end();

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

		let path = RenderPath.active;
		let hid = History.undoI - 1 < 0 ? Config.raw.undo_steps - 1 : History.undoI - 1;
		path.renderTargets.set("texpaint_undo" + hid, path.renderTargets.get("empty_black"));

		// Set plane mesh
		let painto = Context.raw.paintObject;
		let visibles: bool[] = [];
		for (let p of Project.paintObjects) {
			visibles.push(p.visible);
			p.visible = false;
		}
		let mergedObjectVisible = false;
		if (Context.raw.mergedObject != null) {
			mergedObjectVisible = Context.raw.mergedObject.visible;
			Context.raw.mergedObject.visible = false;
		}

		let cam = Scene.active.camera;
		Context.raw.savedCamera.setFrom(cam.transform.local);
		let savedFov = cam.data.raw.fov;
		Viewport.updateCameraType(CameraType.CameraPerspective);
		let m = Mat4.identity();
		m.translate(0, 0, 0.5);
		cam.transform.setMatrix(m);
		cam.data.raw.fov = 0.92;
		cam.buildProjection();
		cam.buildMatrix();
		m.getInverse(Scene.active.camera.VP);

		let planeo: MeshObject = Scene.active.getChild(".Plane") as MeshObject;
		planeo.visible = true;
		Context.raw.paintObject = planeo;

		let v = new Vec4();
		let sx = v.set(m._00, m._01, m._02).length();
		planeo.transform.rot.fromEuler(-Math.PI / 2, 0, 0);
		planeo.transform.scale.set(sx, 1.0, sx);
		planeo.transform.loc.set(m._30, -m._31, 0.0);
		planeo.transform.buildMatrix();

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
		planeo.visible = false;
		for (let i = 0; i < Project.paintObjects.length; ++i) {
			Project.paintObjects[i].visible = visibles[i];
		}
		if (Context.raw.mergedObject != null) {
			Context.raw.mergedObject.visible = mergedObjectVisible;
		}
		Context.raw.paintObject = painto;
		Scene.active.camera.transform.setMatrix(Context.raw.savedCamera);
		Scene.active.camera.data.raw.fov = savedFov;
		Viewport.updateCameraType(Context.raw.cameraType);
		Scene.active.camera.buildProjection();
		Scene.active.camera.buildMatrix();

		// Scale layer down to to image preview
		if (Base.pipeMerge == null) Base.makePipe();
		l = RenderPathPaint.liveLayer;
		let target = Context.raw.brush.image;
		target.g2.begin(true, 0x00000000);
		target.g2.pipeline = Base.pipeCopy;
		target.g2.drawScaledImage(l.texpaint, 0, 0, target.width, target.height);
		target.g2.pipeline = null;
		target.g2.end();

		// Scale image preview down to to icon
		path.renderTargets.get("texpreview").image = Context.raw.brush.image;
		path.renderTargets.get("texpreview_icon").image = Context.raw.brush.imageIcon;
		path.setTarget("texpreview_icon");
		path.bindTarget("texpreview", "tex");
		path.drawShader("shader_datas/supersample_resolve/supersample_resolve");

		Context.raw.brush.previewReady = true;
		Context.raw.brushBlendDirty = true;

		if (current != null) current.begin(false);
	}

	static makeNodePreview = (canvas: TNodeCanvas, node: TNode, image: Image, group: TNodeCanvas = null, parents: TNode[] = null) => {
		let res = MakeMaterial.parseNodePreviewMaterial(node, group, parents);
		if (res == null || res.scon == null) return;

		let g4 = image.g4;
		if (UtilRender.screenAlignedFullVB == null) {
			UtilRender.createScreenAlignedFullData();
		}

		let _scaleWorld = Context.raw.paintObject.transform.scaleWorld;
		Context.raw.paintObject.transform.scaleWorld = 3.0;
		Context.raw.paintObject.transform.buildMatrix();

		g4.begin();
		g4.setPipeline(res.scon.pipeState);
		Uniforms.setContextConstants(g4, res.scon, [""]);
		Uniforms.setObjectConstants(g4, res.scon, Context.raw.paintObject);
		Uniforms.setMaterialConstants(g4, res.scon, res.mcon);
		g4.setVertexBuffer(UtilRender.screenAlignedFullVB);
		g4.setIndexBuffer(UtilRender.screenAlignedFullIB);
		g4.drawIndexedVertices();
		g4.end();

		Context.raw.paintObject.transform.scaleWorld = _scaleWorld;
		Context.raw.paintObject.transform.buildMatrix();
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

	static getDecalMat = (): Mat4 => {
		UtilRender.pickPosNorTex();
		let decalMat = Mat4.identity();
		let loc = new Vec4(Context.raw.posXPicked, Context.raw.posYPicked, Context.raw.posZPicked);
		let rot = new Quat().fromTo(new Vec4(0.0, 0.0, -1.0), new Vec4(Context.raw.norXPicked, Context.raw.norYPicked, Context.raw.norZPicked));
		let scale = new Vec4(Context.raw.brushRadius * 0.5, Context.raw.brushRadius * 0.5, Context.raw.brushRadius * 0.5);
		decalMat.compose(loc, rot, scale);
		return decalMat;
	}

	static createScreenAlignedFullData = () => {
		// Over-sized triangle
		let data = [-Math.floor(32767 / 3), -Math.floor(32767 / 3), 0, 32767, 0, 0, 0, 0, 0, 0, 0, 0,
					 32767,              -Math.floor(32767 / 3), 0, 32767, 0, 0, 0, 0, 0, 0, 0, 0,
					-Math.floor(32767 / 3),  32767,              0, 32767, 0, 0, 0, 0, 0, 0, 0, 0];
		let indices = [0, 1, 2];

		// Mandatory vertex data names and sizes
		let structure = new VertexStructure();
		structure.add("pos", VertexData.I16_4X_Normalized);
		structure.add("nor", VertexData.I16_2X_Normalized);
		structure.add("tex", VertexData.I16_2X_Normalized);
		structure.add("col", VertexData.I16_4X_Normalized);
		UtilRender.screenAlignedFullVB = new VertexBuffer(Math.floor(data.length / Math.floor(structure.byteSize() / 4)), structure, Usage.StaticUsage);
		let vertices = UtilRender.screenAlignedFullVB.lock();
		for (let i = 0; i < Math.floor(vertices.byteLength / 2); ++i) vertices.setInt16(i * 2, data[i], true);
		UtilRender.screenAlignedFullVB.unlock();

		UtilRender.screenAlignedFullIB = new IndexBuffer(indices.length, Usage.StaticUsage);
		let id = UtilRender.screenAlignedFullIB.lock();
		for (let i = 0; i < id.length; ++i) id[i] = indices[i];
		UtilRender.screenAlignedFullIB.unlock();
	}
}

///end
