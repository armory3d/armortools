package arm.ui;

import kha.System;
import kha.Image;
import kha.graphics4.PipelineState;
import kha.graphics4.VertexStructure;
import kha.graphics4.VertexData;
import kha.graphics4.BlendingFactor;
import kha.graphics4.ConstantLocation;
import zui.Zui;
import zui.Id;
import iron.system.Input;
import arm.util.UVUtil;
import arm.util.RenderUtil;
import arm.render.RenderPathPaint;
import arm.Enums;

class UIView2D {

	public static var inst: UIView2D;
	public static var pipe: PipelineState;
	public static var channelLocation: ConstantLocation;
	public static var textInputHover = false;
	public var show = false;
	public var type = View2DLayer;
	public var wx: Int;
	public var wy: Int;
	public var ww: Int;
	public var wh: Int;
	public var ui: Zui;
	public var hwnd = Id.handle();
	public var panX = 0.0;
	public var panY = 0.0;
	public var panScale = 1.0;
	public var uvmapShow = false;
	public var tiledShow = false;
	public var controlsDown = false;
	var texType = TexBase;
	var layerMode = View2DSelected;

	public function new() {
		inst = this;

		pipe = new PipelineState();
		pipe.vertexShader = kha.Shaders.getVertex("layer_view.vert");
		pipe.fragmentShader = kha.Shaders.getFragment("layer_view.frag");
		var vs = new VertexStructure();
		vs.add("pos", VertexData.Float3);
		vs.add("tex", VertexData.Float2);
		vs.add("col", VertexData.UInt8_4X_Normalized);
		pipe.inputLayout = [vs];
		pipe.blendSource = BlendingFactor.BlendOne;
		pipe.blendDestination = BlendingFactor.BlendZero;
		pipe.colorWriteMaskAlpha = false;
		pipe.compile();
		channelLocation = pipe.getConstantLocation("channel");

		var scale = Config.raw.window_scale;
		ui = new Zui({ theme: App.theme, font: App.font, color_wheel: App.colorWheel, black_white_gradient: App.blackWhiteGradient, scaleFactor: scale });
		ui.scrollEnabled = false;
	}

	@:access(zui.Zui)
	public function render(g: kha.graphics2.Graphics) {
		ww = Config.raw.layout[LayoutNodesW];
		wx = Std.int(iron.App.w()) + UIToolbar.inst.toolbarw;
		wy = UIHeader.inst.headerh * 2;
		if (!UISidebar.inst.show) {
			ww += Config.raw.layout[LayoutSidebarW] + UIToolbar.inst.toolbarw;
			wx -= UIToolbar.inst.toolbarw;
			wy = 0;
		}

		if (!show) return;
		if (System.windowWidth() == 0 || System.windowHeight() == 0) return;

		if (Context.pdirty >= 0) hwnd.redraws = 2; // Paint was active

		g.end();

		// Cache grid
		if (UINodes.inst.grid == null) UINodes.inst.drawGrid();

		// Ensure UV map is drawn
		if (uvmapShow) UVUtil.cacheUVMap();

		// Ensure font image is drawn
		if (Context.font.image == null) RenderUtil.makeFontPreview();

		ui.begin(g);
		wh = iron.App.h();
		if (UINodes.inst.show) {
			wh -= Config.raw.layout[LayoutNodesH];
		}
		if (ui.window(hwnd, wx, wy, ww, wh)) {

			// Grid
			ui.g.color = 0xffffffff;
			ui.g.drawImage(UINodes.inst.grid, (panX * panScale) % 40 - 40, (panY * panScale) % 40 - 40);

			// Texture
			var l = Context.layer;
			var tex: Image = null;
			var channel = 0;

			var tw = ww * 0.95 * panScale;
			var tx = ww / 2 - tw / 2 + panX;
			var ty = iron.App.h() / 2 - tw / 2 + panY;

			if (type == View2DLayer) {
				var layer = l;

				if (Config.raw.brush_live && RenderPathPaint.liveLayerDrawn > 0) {
					layer = RenderPathPaint.liveLayer;
				}

				if (layerMode == View2DVisible) {
					var current = @:privateAccess kha.graphics2.Graphics.current;
					if (current != null) current.end();
					layer = untyped Layers.flatten();
					if (current != null) current.begin(false);
				}
				else if (layer.isGroup()) {
					var current = @:privateAccess kha.graphics2.Graphics.current;
					if (current != null) current.end();
					layer = untyped Layers.flatten(false, layer.getChildren());
					if (current != null) current.begin(false);
				}

				tex =
					Context.layer.isMask() ? layer.texpaint :
					texType == TexBase     ? layer.texpaint :
					texType == TexOpacity  ? layer.texpaint :
					texType == TexNormal   ? layer.texpaint_nor :
										     layer.texpaint_pack;

				channel =
					Context.layer.isMask()  ? 1 :
					texType == TexOcclusion ? 1 :
					texType == TexRoughness ? 2 :
					texType == TexMetallic  ? 3 :
					texType == TexOpacity   ? 4 :
					texType == TexHeight    ? 4 :
					texType == TexNormal    ? 5 :
											  0;
			}
			else if (type == View2DAsset) {
				tex = Project.getImage(Context.texture);
			}
			else if (type == View2DFont) {
				tex = Context.font.image;
			}
			else { // View2DNode
				tex = Context.nodePreview;
			}

			var th = tw;
			if (tex != null) {
				th = tw * (tex.height / tex.width);
				ty = iron.App.h() / 2 - th / 2 + panY;

				if (type == View2DLayer) {
					ui.g.pipeline = pipe;
					if (!Context.textureFilter) {
						ui.g.imageScaleQuality = kha.graphics2.ImageScaleQuality.Low;
					}
					#if kha_opengl
					ui.currentWindow.texture.g4.setPipeline(pipe);
					#end
					ui.currentWindow.texture.g4.setInt(channelLocation, channel);
				}

				ui.g.drawScaledImage(tex, tx, ty, tw, th);

				if (tiledShow) {
					ui.g.drawScaledImage(tex, tx - tw, ty, tw, th);
					ui.g.drawScaledImage(tex, tx - tw, ty - th, tw, th);
					ui.g.drawScaledImage(tex, tx - tw, ty + th, tw, th);
					ui.g.drawScaledImage(tex, tx + tw, ty, tw, th);
					ui.g.drawScaledImage(tex, tx + tw, ty - th, tw, th);
					ui.g.drawScaledImage(tex, tx + tw, ty + th, tw, th);
					ui.g.drawScaledImage(tex, tx, ty - th, tw, th);
					ui.g.drawScaledImage(tex, tx, ty + th, tw, th);
				}

				if (type == View2DLayer) {
					ui.g.pipeline = null;
					if (!Context.textureFilter) {
						ui.g.imageScaleQuality = kha.graphics2.ImageScaleQuality.High;
					}
				}
			}

			// UV map
			if (type == View2DLayer && uvmapShow) {
				ui.g.drawScaledImage(UVUtil.uvmap, tx, ty, tw, th);
			}

			// Editable layer name
			var ACCENT_COL = ui.t.ACCENT_COL;
			var BUTTON_H = ui.t.BUTTON_H;
			var ELEMENT_H = ui.t.ELEMENT_H;
			var FONT_SIZE = ui.fontSize;
			ui.t.ACCENT_COL = 0x00000000;
			ui.t.BUTTON_H = 30;
			ui.t.ELEMENT_H = 30;
			ui.fontSize = Std.int(22 * ui.SCALE());
			ui._x = ww - ui.ELEMENT_W() * 1.4;
			ui._y = wh - ui.ELEMENT_H() * 1.2;
			ui._w = Std.int(ui.ELEMENT_W() * 1.4);
			var h = Id.handle();

			if (type == View2DLayer) {
				h.text = l.name;
				l.name = ui.textInput(h, "", Right);
				textInputHover = ui.isHovered;
			}
			else if (type == View2DAsset) {
				var asset = Context.texture;
				if (asset != null) {
					var assetNames = Project.assetNames;
					var i = assetNames.indexOf(asset.name);
					h.text = asset.name;
					asset.name = ui.textInput(h, "", Right);
					assetNames[i] = asset.name;
				}
			}
			else if (type == View2DFont) {
				h.text = Context.font.name;
				Context.font.name = ui.textInput(h, "", Right);
			}
			// else { // View2DNode
			// }

			if (h.changed) UISidebar.inst.hwnd0.redraws = 2;
			ui.t.ACCENT_COL = ACCENT_COL;
			ui.t.BUTTON_H = BUTTON_H;
			ui.t.ELEMENT_H = ELEMENT_H;
			ui.fontSize = FONT_SIZE;

			// Controls
			var ew = Std.int(ui.ELEMENT_W());
			ui.g.color = ui.t.SEPARATOR_COL;
			ui.g.fillRect(0, 0, ww, ui.ELEMENT_H() + ui.ELEMENT_OFFSET());
			ui.g.color = 0xffffffff;
			ui._x = 2;
			ui._y = 2;
			ui._w = ew;

			if (type == View2DLayer) {
				layerMode = ui.combo(Id.handle({ position: layerMode }), [
					tr("Visible"),
					tr("Selected"),
				], tr("Layers"));
				ui._x += ew + 3;
				ui._y = 2;

				if (!Context.layer.isMask()) {
					texType = ui.combo(Id.handle({ position: texType }), [
						tr("Base Color"),
						tr("Normal Map"),
						tr("Occlusion"),
						tr("Roughness"),
						tr("Metallic"),
						tr("Opacity"),
						tr("Height"),
					], tr("Texture"));
					ui._x += ew + 3;
					ui._y = 2;
				}

				uvmapShow = ui.check(Id.handle({ selected: uvmapShow }), tr("UV Map"));
				ui._x += ew + 3;
				ui._y = 2;
			}

			tiledShow = ui.check(Id.handle({ selected: tiledShow }), tr("Tiled"));

			if (type == View2DAsset && tex != null) { // Texture resolution
				ui._x += ew + 3;
				ui._y = 2;
				ui.text(tex.width + "x" + tex.height);
			}

			if (Context.tool == ToolPicker && (type == View2DLayer || type == View2DAsset)) {
				var cursorImg = Res.get("cursor.k");
				var hsize = 16 * ui.SCALE();
				var size = hsize * 2;
				ui.g.drawScaledImage(cursorImg, tx + tw * Context.uvxPicked - hsize, ty + th * Context.uvyPicked - hsize, size, size);
			}
		}
		ui.end();
		g.begin(false);
	}

	public function update() {
		var mouse = Input.getMouse();
		var kb = Input.getKeyboard();

		var headerh = ui.ELEMENT_H() * 1.4;
		Context.paint2d = false;

		if (!App.uiEnabled ||
			!show ||
			mouse.x < wx ||
			mouse.x > wx + ww ||
			mouse.y < wy + headerh ||
			mouse.y > wy + wh) {
			if (UIView2D.inst.controlsDown) {
				UINodes.getCanvasControl(ui, inst);
			}
			return;
		}

		var control = UINodes.getCanvasControl(ui, inst);
		panX += control.panX;
		panY += control.panY;
		if (control.zoom != 0) {
			var _panX = panX / panScale;
			var _panY = panY / panScale;
			panScale += control.zoom;
			if (panScale < 0.1) panScale = 0.1;
			if (panScale > 6.0) panScale = 6.0;
			panX = _panX * panScale;
			panY = _panY * panScale;
		}

		var decal = Context.tool == ToolDecal || Context.tool == ToolText;
		var decalMask = decal && Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.action_paint, ShortcutDown);
		var setCloneSource = Context.tool == ToolClone && Operator.shortcut(Config.keymap.set_clone_source + "+" + Config.keymap.action_paint, ShortcutDown);

		if (type == View2DLayer &&
			!textInputHover &&
			(Operator.shortcut(Config.keymap.action_paint, ShortcutDown) ||
			 Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint, ShortcutDown) ||
			 decalMask ||
			 setCloneSource ||
			 Config.raw.brush_live)) {
			Context.paint2d = true;
		}

		if (ui.isTyping) return;

		if (kb.started("left")) panX -= 5;
		else if (kb.started("right")) panX += 5;
		if (kb.started("up")) panY -= 5;
		else if (kb.started("down")) panY += 5;

		// Limit panning to keep texture in viewport
		var border = 32;
		var tw = ww * 0.95 * panScale;
		var tx = ww / 2 - tw / 2 + panX;
		var hh = iron.App.h();
		var ty = hh / 2 - tw / 2 + panY;

		if      (tx + border >  ww) panX =  ww / 2 + tw / 2 - border;
		else if (tx - border < -tw) panX = -tw / 2 - ww / 2 + border;
		if      (ty + border >  hh) panY =  hh / 2 + tw / 2 - border;
		else if (ty - border < -tw) panY = -tw / 2 - hh / 2 + border;

		if (Operator.shortcut(Config.keymap.view_reset)) {
			panX = 0.0;
			panY = 0.0;
			panScale = 1.0;
		}
	}
}
