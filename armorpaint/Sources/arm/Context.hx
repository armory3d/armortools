package arm;

import kha.System;
import iron.RenderPath;
import iron.object.MeshObject;
import iron.system.Input;
import arm.data.MaterialSlot;
import arm.data.LayerSlot;
import arm.data.BrushSlot;
import arm.data.FontSlot;
import arm.shader.NodeShader;
import arm.shader.MakeMaterial;
import arm.util.UVUtil;
import arm.util.RenderUtil;
import arm.util.ParticleUtil;
import arm.render.RenderPathDeferred;
import arm.render.RenderPathForward;
import arm.ui.UIBase;
import arm.ui.UIToolbar;
import arm.ui.UINodes;
import arm.ui.UIView2D;
import arm.ui.UIHeader;
import arm.ui.UIStatus;
import arm.ui.BoxPreferences;
import arm.ProjectBaseFormat;
import arm.ContextFormat;

class Context {

	public static var raw: TContext = {};

	public static function selectMaterial(i: Int) {
		if (Project.materials.length <= i) return;
		setMaterial(Project.materials[i]);
	}

	public static function setViewportMode(mode: ViewportMode) {
		if (mode == raw.viewportMode) return;

		raw.viewportMode = mode;
		var deferred = raw.renderMode != RenderForward && (raw.viewportMode == ViewLit || raw.viewportMode == ViewPathTrace) && raw.tool != ToolColorId;
		if (deferred) {
			RenderPath.active.commands = RenderPathDeferred.commands;
		}
		// else if (raw.viewportMode == ViewPathTrace) {
		// }
		else {
			if (RenderPathForward.path == null) {
				RenderPathForward.init(RenderPath.active);
			}
			RenderPath.active.commands = RenderPathForward.commands;
		}
		var _workspace = UIHeader.inst.worktab.position;
		UIHeader.inst.worktab.position = SpacePaint;
		MakeMaterial.parseMeshMaterial();
		UIHeader.inst.worktab.position = _workspace;
	}

	public static function setMaterial(m: MaterialSlot) {
		if (Project.materials.indexOf(m) == -1) return;
		raw.material = m;
		MakeMaterial.parsePaintMaterial();
		UIBase.inst.hwnd1.redraws = 2;
		UIHeader.inst.headerHandle.redraws = 2;
		UINodes.inst.hwnd.redraws = 2;
		UINodes.inst.groupStack = [];

		var decal = raw.tool == ToolDecal || raw.tool == ToolText;
		if (decal) {
			function _next() {
				RenderUtil.makeDecalPreview();
			}
			App.notifyOnNextFrame(_next);
		}
	}

	public static function selectBrush(i: Int) {
		if (Project.brushes.length <= i) return;
		setBrush(Project.brushes[i]);
	}

	public static function setBrush(b: BrushSlot) {
		if (Project.brushes.indexOf(b) == -1) return;
		raw.brush = b;
		MakeMaterial.parseBrush();
		UIBase.inst.hwnd1.redraws = 2;
		UINodes.inst.hwnd.redraws = 2;
	}

	public static function selectFont(i: Int) {
		if (Project.fonts.length <= i) return;
		setFont(Project.fonts[i]);
	}

	public static function setFont(f: FontSlot) {
		if (Project.fonts.indexOf(f) == -1) return;
		raw.font = f;
		RenderUtil.makeTextPreview();
		RenderUtil.makeDecalPreview();
		UIStatus.inst.statusHandle.redraws = 2;
		UIView2D.inst.hwnd.redraws = 2;
	}

	public static function setSwatch(s: TSwatchColor) {
		raw.swatch = s;
		App.notifyOnNextFrame(function() {
			MakeMaterial.parsePaintMaterial();
			RenderUtil.makeMaterialPreview();
			UIBase.inst.hwnd1.redraws = 2;
		});
	}

	public static function selectLayer(i: Int) {
		if (Project.layers.length <= i) return;
		setLayer(Project.layers[i]);
	}

	public static function setLayer(l: LayerSlot) {
		if (l == raw.layer) return;
		raw.layer = l;
		UIHeader.inst.headerHandle.redraws = 2;

		var current = @:privateAccess kha.graphics2.Graphics.current;
		if (current != null) current.end();

		App.setObjectMask();
		MakeMaterial.parseMeshMaterial();
		MakeMaterial.parsePaintMaterial();

		if (current != null) current.begin(false);

		UIBase.inst.hwnd0.redraws = 2;
		UIView2D.inst.hwnd.redraws = 2;
	}

	public static function selectTool(i: Int) {
		raw.tool = i;
		MakeMaterial.parsePaintMaterial();
		MakeMaterial.parseMeshMaterial();
		UIHeader.inst.headerHandle.redraws = 2;
		UIToolbar.inst.toolbarHandle.redraws = 2;
		raw.ddirty = 3;
		initTool();
		var _viewportMode = raw.viewportMode;
		raw.viewportMode = -1;
		setViewportMode(_viewportMode);
	}

	public static function initTool() {
		var decal = raw.tool == ToolDecal || raw.tool == ToolText;
		if (decal) {
			if (raw.tool == ToolText) {
				RenderUtil.makeTextPreview();
			}
			RenderUtil.makeDecalPreview();
		}
		if (raw.tool == ToolParticle) {
			ParticleUtil.initParticle();
			MakeMaterial.parseParticleMaterial();
		}

		#if krom_ios
		// No hover on iPad, decals are painted by pen release
		Config.raw.brush_live = decal;
		#end
	}

	public static function selectPaintObject(o: MeshObject) {
		UIHeader.inst.headerHandle.redraws = 2;
		for (p in Project.paintObjects) p.skip_context = "paint";
		raw.paintObject = o;

		var mask = raw.layer.getObjectMask();
		if (Context.layerFilterUsed()) mask = raw.layerFilter;

		if (raw.mergedObject == null || mask > 0) {
			raw.paintObject.skip_context = "";
		}
		UVUtil.uvmapCached = false;
		UVUtil.trianglemapCached = false;
		UVUtil.dilatemapCached = false;
	}

	public static function mainObject(): MeshObject {
		for (po in Project.paintObjects) if (po.children.length > 0) return po;
		return Project.paintObjects[0];
	}

	public static function loadEnvmap() {
		if (!raw.envmapLoaded) {
			// TODO: Unable to share texture for both radiance and envmap - reload image
			raw.envmapLoaded = true;
			iron.data.Data.cachedImages.remove("World_radiance.k");
		}
		iron.Scene.active.world.loadEnvmap(function(_) {});
		if (raw.savedEnvmap == null) raw.savedEnvmap = iron.Scene.active.world.envmap;
	}

	@:keep
	public static function setViewportShader(viewportShader: NodeShader->String) {
		raw.viewportShader = viewportShader;
		setRenderPath();
	}

	public static function setRenderPath() {
		if (raw.renderMode == RenderForward || raw.viewportShader != null) {
			if (RenderPathForward.path == null) {
				RenderPathForward.init(RenderPath.active);
			}
			RenderPath.active.commands = RenderPathForward.commands;
		}
		else {
			RenderPath.active.commands = RenderPathDeferred.commands;
		}
		iron.App.notifyOnInit(function() {
			MakeMaterial.parseMeshMaterial();
		});
	}

	public static function enableImportPlugin(file: String): Bool {
		// Return plugin name suitable for importing the specified file
		if (BoxPreferences.filesPlugin == null) {
			BoxPreferences.fetchPlugins();
		}
		var ext = file.substr(file.lastIndexOf(".") + 1);
		for (f in BoxPreferences.filesPlugin) {
			if (f.startsWith("import_") && f.indexOf(ext) >= 0) {
				Config.enablePlugin(f);
				Console.info(f + " " + tr("plugin enabled"));
				return true;
			}
		}
		return false;
	}

	public static function layerFilterUsed(): Bool {
		return raw.layerFilter > 0 && raw.layerFilter <= Project.paintObjects.length;
	}

	public static function objectMaskUsed(): Bool {
		return raw.layer.getObjectMask() > 0 && raw.layer.getObjectMask() <= Project.paintObjects.length;
	}

	public static function inViewport(): Bool {
		return raw.paintVec.x < 1 && raw.paintVec.x > 0 &&
			   raw.paintVec.y < 1 && raw.paintVec.y > 0;
	}

	public static function inPaintArea(): Bool {
		var mouse = Input.getMouse();
		var right = iron.App.w();
		if (UIView2D.inst.show) right += UIView2D.inst.ww;
		return mouse.viewX > 0 && mouse.viewX < right &&
			   mouse.viewY > 0 && mouse.viewY < iron.App.h();
	}

	public static function inLayers(): Bool {
		var mouse = Input.getMouse();
		return UIBase.inst.htab0.position == 0 &&
			   mouse.x > UIBase.inst.tabx && mouse.y < Config.raw.layout[LayoutSidebarH0];
	}

	public static function inMaterials(): Bool {
		var mouse = Input.getMouse();
		return UIBase.inst.htab1.position == 0 &&
			   mouse.x > UIBase.inst.tabx &&
			   mouse.y > Config.raw.layout[LayoutSidebarH0] &&
			   mouse.y < Config.raw.layout[LayoutSidebarH1] + Config.raw.layout[LayoutSidebarH0];
	}

	public static function in2dView(): Bool {
		var mouse = Input.getMouse();
		return UIView2D.inst.show && UIView2D.inst.type == View2DLayer &&
			   mouse.x > UIView2D.inst.wx && mouse.x < UIView2D.inst.wx + UIView2D.inst.ww &&
			   mouse.y > UIView2D.inst.wy && mouse.y < UIView2D.inst.wy + UIView2D.inst.wh;
	}

	public static function inNodes(): Bool {
		var mouse = Input.getMouse();
		return UINodes.inst.show &&
			   mouse.x > UINodes.inst.wx && mouse.x < UINodes.inst.wx + UINodes.inst.ww &&
			   mouse.y > UINodes.inst.wy && mouse.y < UINodes.inst.wy + UINodes.inst.wh;
	}

	public static function inSwatches(): Bool {
		var mouse = Input.getMouse();
		return UIStatus.inst.statustab.position == 4 &&
			   mouse.x > iron.App.x() &&
			   mouse.x < iron.App.x() + System.windowWidth() - UIToolbar.inst.toolbarw - Config.raw.layout[LayoutSidebarW] &&
			   mouse.y > System.windowHeight() - Config.raw.layout[LayoutStatusH];
	}

	public static function inBrowser(): Bool {
		var mouse = Input.getMouse();
		return mouse.x > iron.App.x() &&
			   mouse.x < iron.App.x() + (System.windowWidth() - UIToolbar.inst.toolbarw - Config.raw.layout[LayoutSidebarW]) &&
			   mouse.y > System.windowHeight() - Config.raw.layout[LayoutStatusH];
	}

	public static function getAreaType(): AreaType {
		if (inViewport()) return AreaViewport;
		if (in2dView()) return Area2DView;
		if (inLayers()) return AreaLayers;
		if (inMaterials()) return AreaMaterials;
		if (inNodes()) return AreaNodes;
		if (inBrowser()) return AreaBrowser;
		return -1;
	}
}
