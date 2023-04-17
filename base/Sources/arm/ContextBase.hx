package arm;

import iron.RenderPath;
import arm.render.RenderPathDeferred;
import arm.render.RenderPathForward;
import arm.shader.MakeMaterial;
import arm.shader.NodeShader;
import arm.ui.BoxPreferences;
import arm.ui.UIHeader;
import arm.ProjectBaseFormat;

class ContextBase {

	public static function setViewportMode(mode: ViewportMode) {
		if (mode == Context.raw.viewportMode) return;

		Context.raw.viewportMode = mode;
		if (Context.useDeferred()) {
			RenderPath.active.commands = RenderPathDeferred.commands;
		}
		else {
			if (RenderPathForward.path == null) {
				RenderPathForward.init(RenderPath.active);
			}
			RenderPath.active.commands = RenderPathForward.commands;
		}
		var _workspace = UIHeader.inst.worktab.position;
		UIHeader.inst.worktab.position = 0;
		MakeMaterial.parseMeshMaterial();
		UIHeader.inst.worktab.position = _workspace;
	}

	public static function loadEnvmap() {
		if (!Context.raw.envmapLoaded) {
			// TODO: Unable to share texture for both radiance and envmap - reload image
			Context.raw.envmapLoaded = true;
			iron.data.Data.cachedImages.remove("World_radiance.k");
		}
		iron.Scene.active.world.loadEnvmap(function(_) {});
		if (Context.raw.savedEnvmap == null) Context.raw.savedEnvmap = iron.Scene.active.world.envmap;
	}

	@:keep
	public static function setViewportShader(viewportShader: NodeShader->String) {
		Context.raw.viewportShader = viewportShader;
		setRenderPath();
	}

	public static function setRenderPath() {
		if (Context.raw.renderMode == RenderForward || Context.raw.viewportShader != null) {
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

	public static function setSwatch(s: TSwatchColor) {
		Context.raw.swatch = s;
	}

	static function selectTool(i: Int) {
		Context.raw.tool = i;
		MakeMaterial.parsePaintMaterial();
		MakeMaterial.parseMeshMaterial();
		Context.raw.ddirty = 3;
		var _viewportMode = Context.raw.viewportMode;
		Context.raw.viewportMode = -1;
		ContextBase.setViewportMode(_viewportMode);
	}
}
