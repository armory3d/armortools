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
#if (is_paint || is_sculpt)
import arm.util.RenderUtil;
import arm.util.UVUtil;
import arm.render.RenderPathPaint;
#end

class UIView2D {

	#if (is_paint || is_sculpt)
	public static var pipe: PipelineState;
	public static var channelLocation: ConstantLocation;
	public static var textInputHover = false;
	public var uvmapShow = false;
	var texType = TexBase;
	var layerMode = View2DSelected;
	#end

	#if (is_paint || is_sculpt)
	public var type = View2DLayer;
	#else
	public var type = View2DAsset;
	#end

	public static var inst: UIView2D;
	public var show = false;
	public var wx: Int;
	public var wy: Int;
	public var ww: Int;
	public var wh: Int;
	public var ui: Zui;
	public var hwnd = Id.handle();
	public var panX = 0.0;
	public var panY = 0.0;
	public var panScale = 1.0;
	public var tiledShow = false;
	public var controlsDown = false;

	public function new() {
		inst = this;

		#if (is_paint || is_sculpt)
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
		#end

		var scale = Config.raw.window_scale;
		ui = new Zui({ theme: App.theme, font: App.font, color_wheel: App.colorWheel, black_white_gradient: App.colorWheelGradient, scaleFactor: scale });
		ui.scrollEnabled = false;
	}

	@:access(zui.Zui)
	public function render(g: kha.graphics2.Graphics) {

		ww = Config.raw.layout[LayoutNodesW];

		#if (is_paint || is_sculpt)
		wx = Std.int(iron.App.w()) + UIToolbar.inst.toolbarw;
		#else
		wx = Std.int(iron.App.w());
		#end

		wy = 0;

		#if (is_paint || is_sculpt)
		if (!UIBase.inst.show) {
			ww += Config.raw.layout[LayoutSidebarW] + UIToolbar.inst.toolbarw;
			wx -= UIToolbar.inst.toolbarw;
		}
		#end

		if (!show) return;
		if (System.windowWidth() == 0 || System.windowHeight() == 0) return;

		if (Context.raw.pdirty >= 0) hwnd.redraws = 2; // Paint was active

		g.end();

		// Cache grid
		if (UINodes.inst.grid == null) UINodes.inst.drawGrid();

		// Ensure UV map is drawn
		#if (is_paint || is_sculpt)
		if (uvmapShow) UVUtil.cacheUVMap();
		#end

		// Ensure font image is drawn
		#if (is_paint || is_sculpt)
		if (Context.raw.font.image == null) RenderUtil.makeFontPreview();
		#end

		ui.begin(g);

		var headerh = Config.raw.layout[LayoutHeader] == 1 ? UIHeader.headerh * 2 : UIHeader.headerh;
		var apph = System.windowHeight() - Config.raw.layout[LayoutStatusH] + headerh;
		wh = System.windowHeight() - Config.raw.layout[LayoutStatusH];

		if (UINodes.inst.show) {
			wh -= Config.raw.layout[LayoutNodesH];
			if (Config.raw.touch_ui) wh += UIHeader.headerh;
		}

		if (ui.window(hwnd, wx, wy, ww, wh)) {

			ui.tab(Id.handle(), tr("2D View"));

			// Grid
			ui.g.color = 0xffffffff;
			ui.g.drawImage(UINodes.inst.grid, (panX * panScale) % 100 - 100, (panY * panScale) % 100 - 100);

			// Texture
			var tex: Image = null;

			#if (is_paint || is_sculpt)
			var l = Context.raw.layer;
			var channel = 0;
			#end

			var tw = ww * 0.95 * panScale;
			var tx = ww / 2 - tw / 2 + panX;
			var ty = apph / 2 - tw / 2 + panY;

			if (type == View2DAsset) {
				tex = Project.getImage(Context.raw.texture);
			}
			else if (type == View2DNode) {
				#if (is_paint || is_sculpt)

				tex = Context.raw.nodePreview;

				#else

				var nodes = UINodes.inst.getNodes();
				if (nodes.nodesSelected.length > 0) {
					var sel = nodes.nodesSelected[0];
					var brushNode = arm.logic.LogicParser.getLogicNode(sel);
					if (brushNode != null) {
						tex = brushNode.getCachedImage();
					}
				}

				#end
			}
			#if (is_paint || is_sculpt)
			else if (type == View2DLayer) {
				var layer = l;

				if (Config.raw.brush_live && RenderPathPaint.liveLayerDrawn > 0) {
					layer = RenderPathPaint.liveLayer;
				}

				if (layerMode == View2DVisible) {
					var current = @:privateAccess kha.graphics2.Graphics.current;
					if (current != null) current.end();
					layer = untyped App.flatten();
					if (current != null) current.begin(false);
				}
				else if (layer.isGroup()) {
					var current = @:privateAccess kha.graphics2.Graphics.current;
					if (current != null) current.end();
					layer = untyped App.flatten(false, layer.getChildren());
					if (current != null) current.begin(false);
				}

				tex =
					Context.raw.layer.isMask() ? layer.texpaint :
					texType == TexBase     ? layer.texpaint :
					texType == TexOpacity  ? layer.texpaint :
					texType == TexNormal   ? layer.texpaint_nor :
										     layer.texpaint_pack;

				channel =
					Context.raw.layer.isMask()  ? 1 :
					texType == TexOcclusion ? 1 :
					texType == TexRoughness ? 2 :
					texType == TexMetallic  ? 3 :
					texType == TexOpacity   ? 4 :
					texType == TexHeight    ? 4 :
					texType == TexNormal    ? 5 :
											  0;
			}
			else if (type == View2DFont) {
				tex = Context.raw.font.image;
			}
			#end

			var th = tw;
			if (tex != null) {
				th = tw * (tex.height / tex.width);
				ty = apph / 2 - th / 2 + panY;

				#if (is_paint || is_sculpt)
				if (type == View2DLayer) {
					ui.g.pipeline = pipe;
					if (!Context.raw.textureFilter) {
						ui.g.imageScaleQuality = kha.graphics2.ImageScaleQuality.Low;
					}
					#if kha_opengl
					ui.currentWindow.texture.g4.setPipeline(pipe);
					#end
					ui.currentWindow.texture.g4.setInt(channelLocation, channel);
				}
				#end

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

				#if (is_paint || is_sculpt)
				if (type == View2DLayer) {
					ui.g.pipeline = null;
					if (!Context.raw.textureFilter) {
						ui.g.imageScaleQuality = kha.graphics2.ImageScaleQuality.High;
					}
				}

				// Texture and node preview color picking
				if ((Context.in2dView(View2DAsset) || Context.in2dView(View2DNode)) && Context.raw.tool == ToolPicker && ui.inputDown) {
					var x = ui.inputX - tx - wx;
					var y = ui.inputY - ty - wy;
					App.notifyOnNextFrame(function() {
						var path = iron.RenderPath.active;
						var texpaint_picker = path.renderTargets.get("texpaint_picker").image;
						var g2 = texpaint_picker.g2;
						g2.begin(false);
						g2.drawScaledImage(tex, -x, -y, tw, th);
						g2.end();
						var a = texpaint_picker.getPixels();
						#if (kha_metal || kha_vulkan)
						var i0 = 2;
						var i1 = 1;
						var i2 = 0;
						#else
						var i0 = 0;
						var i1 = 1;
						var i2 = 2;
						#end
						Context.raw.pickedColor.base.Rb = a.get(i0);
						Context.raw.pickedColor.base.Gb = a.get(i1);
						Context.raw.pickedColor.base.Bb = a.get(i2);
						UIHeader.inst.headerHandle.redraws = 2;
					});
				}
				#end
			}

			#if (is_paint || is_sculpt)
			// UV map
			if (type == View2DLayer && uvmapShow) {
				ui.g.drawScaledImage(UVUtil.uvmap, tx, ty, tw, th);
			}
			#end

			// Menu
			var ew = Std.int(ui.ELEMENT_W());
			ui.g.color = ui.t.SEPARATOR_COL;
			ui.g.fillRect(0, ui.ELEMENT_H(), ww, ui.ELEMENT_H() + ui.ELEMENT_OFFSET() * 2);
			ui.g.color = 0xffffffff;

			var startY = ui.ELEMENT_H() + ui.ELEMENT_OFFSET();
			ui._x = 2;
			ui._y = 2 + startY;
			ui._w = ew;

			// Editable layer name
			var h = Id.handle();

			#if (is_paint || is_sculpt)
			var text = type == View2DNode ? Context.raw.nodePreviewName : h.text;
			#else
			var text = h.text;
			#end

			ui._w = Std.int(Math.min(ui.ops.font.width(ui.fontSize, text) + 15 * ui.SCALE(), 100 * ui.SCALE()));

			if (type == View2DAsset) {
				var asset = Context.raw.texture;
				if (asset != null) {
					var assetNames = Project.assetNames;
					var i = assetNames.indexOf(asset.name);
					h.text = asset.name;
					asset.name = ui.textInput(h, "");
					assetNames[i] = asset.name;
				}
			}
			else if (type == View2DNode) {
				#if (is_paint || is_sculpt)

				ui.text(Context.raw.nodePreviewName);

				#else

				var nodes = UINodes.inst.getNodes();
				if (nodes.nodesSelected.length > 0) {
					ui.text(nodes.nodesSelected[0].name);
				}

				#end
			}
			#if (is_paint || is_sculpt)
			else if (type == View2DLayer) {
				h.text = l.name;
				l.name = ui.textInput(h, "");
				textInputHover = ui.isHovered;
			}
			else if (type == View2DFont) {
				h.text = Context.raw.font.name;
				Context.raw.font.name = ui.textInput(h, "");
			}
			#end

			if (h.changed) UIBase.inst.hwnds[0].redraws = 2;
			ui._x += ui._w + 3;
			ui._y = 2 + startY;
			ui._w = ew;

			#if (is_paint || is_sculpt)
			if (type == View2DLayer) {
				layerMode = ui.combo(Id.handle({ position: layerMode }), [
					tr("Visible"),
					tr("Selected"),
				], tr("Layers"));
				ui._x += ew + 3;
				ui._y = 2 + startY;

				if (!Context.raw.layer.isMask()) {
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
					ui._y = 2 + startY;
				}

				ui._w = Std.int(ew * 0.7 + 3);
				uvmapShow = ui.check(Id.handle({ selected: uvmapShow }), tr("UV Map"));
				ui._x += ew * 0.7 + 3;
				ui._y = 2 + startY;
			}
			#end

			tiledShow = ui.check(Id.handle({ selected: tiledShow }), tr("Tiled"));
			ui._x += ew * 0.7 + 3;
			ui._y = 2 + startY;

			if (type == View2DAsset && tex != null) { // Texture resolution
				ui.text(tex.width + "x" + tex.height);
			}

			// Picked position
			#if (is_paint || is_sculpt)
			if (Context.raw.tool == ToolPicker && (type == View2DLayer || type == View2DAsset)) {
				var cursorImg = Res.get("cursor.k");
				var hsize = 16 * ui.SCALE();
				var size = hsize * 2;
				ui.g.drawScaledImage(cursorImg, tx + tw * Context.raw.uvxPicked - hsize, ty + th * Context.raw.uvyPicked - hsize, size, size);
			}
			#end
		}
		ui.end();
		g.begin(false);
	}

	@:access(zui.Zui)
	public function update() {

		var mouse = Input.getMouse();
		var kb = Input.getKeyboard();

		var headerh = ui.ELEMENT_H() * 1.4;

		#if (is_paint || is_sculpt)
		Context.raw.paint2d = false;
		#end

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

			if (Zui.touchScroll) {
				// Zoom to finger location
				panX -= (ui.inputX - ui._windowX - ui._windowW / 2) * control.zoom;
				panY -= (ui.inputY - ui._windowY - ui._windowH / 2) * control.zoom;
			}
		}

		#if (is_paint || is_sculpt)
		var decal = Context.raw.tool == ToolDecal || Context.raw.tool == ToolText;
		var decalMask = decal && Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.action_paint, ShortcutDown);
		var setCloneSource = Context.raw.tool == ToolClone && Operator.shortcut(Config.keymap.set_clone_source + "+" + Config.keymap.action_paint, ShortcutDown);

		if (type == View2DLayer &&
			!textInputHover &&
			(Operator.shortcut(Config.keymap.action_paint, ShortcutDown) ||
			 Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint, ShortcutDown) ||
			 decalMask ||
			 setCloneSource ||
			 Config.raw.brush_live)) {
			Context.raw.paint2d = true;
		}
		#end

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
