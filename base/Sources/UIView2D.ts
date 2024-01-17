
class UIView2D {

	///if (is_paint || is_sculpt)
	static pipe: PipelineState;
	static channelLocation: ConstantLocation;
	static textInputHover = false;
	uvmapShow = false;
	texType = PaintTex.TexBase;
	layerMode = View2DLayerMode.View2DSelected;
	///end

	///if (is_paint || is_sculpt)
	type = View2DType.View2DLayer;
	///else
	type = View2DType.View2DAsset;
	///end

	static inst: UIView2D;
	show = false;
	wx: i32;
	wy: i32;
	ww: i32;
	wh: i32;
	ui: Zui;
	hwnd = new Handle();
	panX = 0.0;
	panY = 0.0;
	panScale = 1.0;
	tiledShow = false;
	controlsDown = false;

	constructor() {
		UIView2D.inst = this;

		///if (is_paint || is_sculpt)
		UIView2D.pipe = new PipelineState();
		UIView2D.pipe.vertexShader = System.getShader("layer_view.vert");
		UIView2D.pipe.fragmentShader = System.getShader("layer_view.frag");
		let vs = new VertexStructure();
		vs.add("pos", VertexData.F32_3X);
		vs.add("tex", VertexData.F32_2X);
		vs.add("col", VertexData.U8_4X_Normalized);
		UIView2D.pipe.inputLayout = [vs];
		UIView2D.pipe.blendSource = BlendingFactor.BlendOne;
		UIView2D.pipe.blendDestination = BlendingFactor.BlendZero;
		UIView2D.pipe.colorWriteMasksAlpha[0] = false;
		UIView2D.pipe.compile();
		UIView2D.channelLocation = UIView2D.pipe.getConstantLocation("channel");
		///end

		let scale = Config.raw.window_scale;
		this.ui = new Zui({ theme: Base.theme, font: Base.font, color_wheel: Base.colorWheel, black_white_gradient: Base.colorWheelGradient, scaleFactor: scale });
		this.ui.scrollEnabled = false;
	}

	render = (g: Graphics2) => {

		this.ww = Config.raw.layout[LayoutSize.LayoutNodesW];

		///if (is_paint || is_sculpt)
		this.wx = Math.floor(App.w()) + UIToolbar.inst.toolbarw;
		///else
		this.wx = Math.floor(App.w());
		///end

		this.wy = 0;

		///if (is_paint || is_sculpt)
		if (!UIBase.inst.show) {
			this.ww += Config.raw.layout[LayoutSize.LayoutSidebarW] + UIToolbar.inst.toolbarw;
			this.wx -= UIToolbar.inst.toolbarw;
		}
		///end

		if (!this.show) return;
		if (System.width == 0 || System.height == 0) return;

		if (Context.raw.pdirty >= 0) this.hwnd.redraws = 2; // Paint was active

		g.end();

		// Cache grid
		if (UINodes.inst.grid == null) UINodes.inst.drawGrid();

		// Ensure UV map is drawn
		///if (is_paint || is_sculpt)
		if (this.uvmapShow) UtilUV.cacheUVMap();
		///end

		// Ensure font image is drawn
		///if (is_paint || is_sculpt)
		if (Context.raw.font.image == null) UtilRender.makeFontPreview();
		///end

		this.ui.begin(g);

		let headerh = Config.raw.layout[LayoutSize.LayoutHeader] == 1 ? UIHeader.headerh * 2 : UIHeader.headerh;
		let apph = System.height - Config.raw.layout[LayoutSize.LayoutStatusH] + headerh;
		this.wh = System.height - Config.raw.layout[LayoutSize.LayoutStatusH];

		if (UINodes.inst.show) {
			this.wh -= Config.raw.layout[LayoutSize.LayoutNodesH];
			if (Config.raw.touch_ui) this.wh += UIHeader.headerh;
		}

		if (this.ui.window(this.hwnd, this.wx, this.wy, this.ww, this.wh)) {

			this.ui.tab(Zui.handle("uiview2d_0"), tr("2D View"));

			// Grid
			this.ui.g.color = 0xffffffff;
			this.ui.g.drawImage(UINodes.inst.grid, (this.panX * this.panScale) % 100 - 100, (this.panY * this.panScale) % 100 - 100);

			// Texture
			let tex: Image = null;

			///if (is_paint || is_sculpt)
			let l = Context.raw.layer;
			let channel = 0;
			///end

			let tw = this.ww * 0.95 * this.panScale;
			let tx = this.ww / 2 - tw / 2 + this.panX;
			let ty = apph / 2 - tw / 2 + this.panY;

			if (this.type == View2DType.View2DAsset) {
				tex = Project.getImage(Context.raw.texture);
			}
			else if (this.type == View2DType.View2DNode) {
				///if (is_paint || is_sculpt)

				tex = Context.raw.nodePreview;

				///else

				let nodes = UINodes.inst.getNodes();
				if (nodes.nodesSelectedId.length > 0) {
					let sel = nodes.getNode(UINodes.inst.getCanvas(true).nodes, nodes.nodesSelectedId[0]);
					let brushNode = ParserLogic.getLogicNode(sel);
					if (brushNode != null) {
						tex = brushNode.getCachedImage();
					}
				}

				///end
			}
			///if is_paint
			else if (this.type == View2DType.View2DLayer) {
				let layer = l;

				if (Config.raw.brush_live && RenderPathPaint.liveLayerDrawn > 0) {
					layer = RenderPathPaint.liveLayer;
				}

				if (this.layerMode == View2DLayerMode.View2DVisible) {
					let current = Graphics2.current;
					if (current != null) current.end();
					layer = Base.flatten();
					if (current != null) current.begin(false);
				}
				else if (layer.isGroup()) {
					let current = Graphics2.current;
					if (current != null) current.end();
					layer = Base.flatten(false, layer.getChildren());
					if (current != null) current.begin(false);
				}

				tex =
					Context.raw.layer.isMask() ? layer.texpaint :
					this.texType == PaintTex.TexBase     ? layer.texpaint :
					this.texType == PaintTex.TexOpacity  ? layer.texpaint :
					this.texType == PaintTex.TexNormal   ? layer.texpaint_nor :
														   layer.texpaint_pack;

				channel =
					Context.raw.layer.isMask()  ? 1 :
					this.texType == PaintTex.TexOcclusion ? 1 :
					this.texType == PaintTex.TexRoughness ? 2 :
					this.texType == PaintTex.TexMetallic  ? 3 :
					this.texType == PaintTex.TexOpacity   ? 4 :
					this.texType == PaintTex.TexHeight    ? 4 :
					this.texType == PaintTex.TexNormal    ? 5 :
															0;
			}
			else if (this.type == View2DType.View2DFont) {
				tex = Context.raw.font.image;
			}
			///end

			let th = tw;
			if (tex != null) {
				th = tw * (tex.height / tex.width);
				ty = apph / 2 - th / 2 + this.panY;

				///if (is_paint || is_sculpt)
				if (this.type == View2DType.View2DLayer) {
					///if (!krom_opengl)
					this.ui.g.pipeline = UIView2D.pipe;
					///end
					if (!Context.raw.textureFilter) {
						this.ui.g.imageScaleQuality = ImageScaleQuality.Low;
					}
					///if krom_opengl
					Krom.setPipeline(UIView2D.pipe.pipeline_);
					///end
					Krom.setInt(UIView2D.channelLocation, channel);
				}
				///end

				this.ui.g.drawScaledImage(tex, tx, ty, tw, th);

				if (this.tiledShow) {
					this.ui.g.drawScaledImage(tex, tx - tw, ty, tw, th);
					this.ui.g.drawScaledImage(tex, tx - tw, ty - th, tw, th);
					this.ui.g.drawScaledImage(tex, tx - tw, ty + th, tw, th);
					this.ui.g.drawScaledImage(tex, tx + tw, ty, tw, th);
					this.ui.g.drawScaledImage(tex, tx + tw, ty - th, tw, th);
					this.ui.g.drawScaledImage(tex, tx + tw, ty + th, tw, th);
					this.ui.g.drawScaledImage(tex, tx, ty - th, tw, th);
					this.ui.g.drawScaledImage(tex, tx, ty + th, tw, th);
				}

				///if (is_paint || is_sculpt)
				if (this.type == View2DType.View2DLayer) {
					this.ui.g.pipeline = null;
					if (!Context.raw.textureFilter) {
						this.ui.g.imageScaleQuality = ImageScaleQuality.High;
					}
				}

				// Texture and node preview color picking
				if ((Context.in2dView(View2DType.View2DAsset) || Context.in2dView(View2DType.View2DNode)) && Context.raw.tool == WorkspaceTool.ToolPicker && this.ui.inputDown) {
					let x = this.ui.inputX - tx - this.wx;
					let y = this.ui.inputY - ty - this.wy;
					Base.notifyOnNextFrame(() => {
						let path = RenderPath.active;
						let texpaint_picker = path.renderTargets.get("texpaint_picker").image;
						let g2 = texpaint_picker.g2;
						g2.begin(false);
						g2.drawScaledImage(tex, -x, -y, tw, th);
						g2.end();
						let a = new DataView(texpaint_picker.getPixels());
						///if (krom_metal || krom_vulkan)
						let i0 = 2;
						let i1 = 1;
						let i2 = 0;
						///else
						let i0 = 0;
						let i1 = 1;
						let i2 = 2;
						///end

						Context.raw.pickedColor.base = color_set_rb(Context.raw.pickedColor.base, a.getUint8(i0));
						Context.raw.pickedColor.base = color_set_gb(Context.raw.pickedColor.base, a.getUint8(i1));
						Context.raw.pickedColor.base = color_set_bb(Context.raw.pickedColor.base, a.getUint8(i2));
						UIHeader.inst.headerHandle.redraws = 2;
					});
				}
				///end
			}

			///if (is_paint || is_sculpt)
			// UV map
			if (this.type == View2DType.View2DLayer && this.uvmapShow) {
				this.ui.g.drawScaledImage(UtilUV.uvmap, tx, ty, tw, th);
			}
			///end

			// Menu
			let ew = Math.floor(this.ui.ELEMENT_W());
			this.ui.g.color = this.ui.t.SEPARATOR_COL;
			this.ui.g.fillRect(0, this.ui.ELEMENT_H(), this.ww, this.ui.ELEMENT_H() + this.ui.ELEMENT_OFFSET() * 2);
			this.ui.g.color = 0xffffffff;

			let startY = this.ui.ELEMENT_H() + this.ui.ELEMENT_OFFSET();
			this.ui._x = 2;
			this.ui._y = 2 + startY;
			this.ui._w = ew;

			// Editable layer name
			let h = Zui.handle("uiview2d_1");

			///if (is_paint || is_sculpt)
			let text = this.type == View2DType.View2DNode ? Context.raw.nodePreviewName : h.text;
			///else
			let text = h.text;
			///end

			this.ui._w = Math.floor(Math.min(this.ui.font.width(this.ui.fontSize, text) + 15 * this.ui.SCALE(), 100 * this.ui.SCALE()));

			if (this.type == View2DType.View2DAsset) {
				let asset = Context.raw.texture;
				if (asset != null) {
					let assetNames = Project.assetNames;
					let i = assetNames.indexOf(asset.name);
					h.text = asset.name;
					asset.name = this.ui.textInput(h, "");
					assetNames[i] = asset.name;
				}
			}
			else if (this.type == View2DType.View2DNode) {
				///if (is_paint || is_sculpt)

				this.ui.text(Context.raw.nodePreviewName);

				///else

				let nodes = UINodes.inst.getNodes();
				if (nodes.nodesSelectedId.length > 0) {
					this.ui.text(nodes.getNode(UINodes.inst.getCanvas(true).nodes, nodes.nodesSelectedId[0]).name);
				}

				///end
			}
			///if (is_paint || is_sculpt)
			else if (this.type == View2DType.View2DLayer) {
				h.text = l.name;
				l.name = this.ui.textInput(h, "");
				UIView2D.textInputHover = this.ui.isHovered;
			}
			else if (this.type == View2DType.View2DFont) {
				h.text = Context.raw.font.name;
				Context.raw.font.name = this.ui.textInput(h, "");
			}
			///end

			if (h.changed) UIBase.inst.hwnds[0].redraws = 2;
			this.ui._x += this.ui._w + 3;
			this.ui._y = 2 + startY;
			this.ui._w = ew;

			///if (is_paint || is_sculpt)
			if (this.type == View2DType.View2DLayer) {
				this.layerMode = this.ui.combo(Zui.handle("uiview2d_2", { position: this.layerMode }), [
					tr("Visible"),
					tr("Selected"),
				], tr("Layers"));
				this.ui._x += ew + 3;
				this.ui._y = 2 + startY;

				if (!Context.raw.layer.isMask()) {
					this.texType = this.ui.combo(Zui.handle("uiview2d_3", { position: this.texType }), [
						tr("Base Color"),
						tr("Normal Map"),
						tr("Occlusion"),
						tr("Roughness"),
						tr("Metallic"),
						tr("Opacity"),
						tr("Height"),
					], tr("Texture"));
					this.ui._x += ew + 3;
					this.ui._y = 2 + startY;
				}

				this.ui._w = Math.floor(ew * 0.7 + 3);
				this.uvmapShow = this.ui.check(Zui.handle("uiview2d_4", { selected: this.uvmapShow }), tr("UV Map"));
				this.ui._x += ew * 0.7 + 3;
				this.ui._y = 2 + startY;
			}
			///end

			this.tiledShow = this.ui.check(Zui.handle("uiview2d_5", { selected: this.tiledShow }), tr("Tiled"));
			this.ui._x += ew * 0.7 + 3;
			this.ui._y = 2 + startY;

			if (this.type == View2DType.View2DAsset && tex != null) { // Texture resolution
				this.ui.text(tex.width + "x" + tex.height);
			}

			// Picked position
			///if (is_paint || is_sculpt)
			if (Context.raw.tool == WorkspaceTool.ToolPicker && (this.type == View2DType.View2DLayer || this.type == View2DType.View2DAsset)) {
				let cursorImg = Res.get("cursor.k");
				let hsize = 16 * this.ui.SCALE();
				let size = hsize * 2;
				this.ui.g.drawScaledImage(cursorImg, tx + tw * Context.raw.uvxPicked - hsize, ty + th * Context.raw.uvyPicked - hsize, size, size);
			}
			///end
		}
		this.ui.end();
		g.begin(false);
	}

	update = () => {
		let mouse = Input.getMouse();
		let kb = Input.getKeyboard();

		let headerh = this.ui.ELEMENT_H() * 1.4;

		///if (is_paint || is_sculpt)
		Context.raw.paint2d = false;
		///end

		if (!Base.uiEnabled ||
			!this.show ||
			mouse.x < this.wx ||
			mouse.x > this.wx + this.ww ||
			mouse.y < this.wy + headerh ||
			mouse.y > this.wy + this.wh) {
			if (UIView2D.inst.controlsDown) {
				UINodes.getCanvasControl(this.ui, UIView2D.inst);
			}
			return;
		}

		let control = UINodes.getCanvasControl(this.ui, UIView2D.inst);
		this.panX += control.panX;
		this.panY += control.panY;
		if (control.zoom != 0) {
			let _panX = this.panX / this.panScale;
			let _panY = this.panY / this.panScale;
			this.panScale += control.zoom;
			if (this.panScale < 0.1) this.panScale = 0.1;
			if (this.panScale > 6.0) this.panScale = 6.0;
			this.panX = _panX * this.panScale;
			this.panY = _panY * this.panScale;

			if (Zui.touchScroll) {
				// Zoom to finger location
				this.panX -= (this.ui.inputX - this.ui._windowX - this.ui._windowW / 2) * control.zoom;
				this.panY -= (this.ui.inputY - this.ui._windowY - this.ui._windowH / 2) * control.zoom;
			}
		}

		///if (is_paint || is_sculpt)
		let decal = Context.raw.tool == WorkspaceTool.ToolDecal || Context.raw.tool == WorkspaceTool.ToolText;
		let decalMask = decal && Operator.shortcut(Config.keymap.decal_mask + "+" + Config.keymap.action_paint, ShortcutType.ShortcutDown);
		let setCloneSource = Context.raw.tool == WorkspaceTool.ToolClone && Operator.shortcut(Config.keymap.set_clone_source + "+" + Config.keymap.action_paint, ShortcutType.ShortcutDown);

		if (this.type == View2DType.View2DLayer &&
			!UIView2D.textInputHover &&
			(Operator.shortcut(Config.keymap.action_paint, ShortcutType.ShortcutDown) ||
			 Operator.shortcut(Config.keymap.brush_ruler + "+" + Config.keymap.action_paint, ShortcutType.ShortcutDown) ||
			 decalMask ||
			 setCloneSource ||
			 Config.raw.brush_live)) {
			Context.raw.paint2d = true;
		}
		///end

		if (this.ui.isTyping) return;

		if (kb.started("left")) this.panX -= 5;
		else if (kb.started("right")) this.panX += 5;
		if (kb.started("up")) this.panY -= 5;
		else if (kb.started("down")) this.panY += 5;

		// Limit panning to keep texture in viewport
		let border = 32;
		let tw = this.ww * 0.95 * this.panScale;
		let tx = this.ww / 2 - tw / 2 + this.panX;
		let hh = App.h();
		let ty = hh / 2 - tw / 2 + this.panY;

		if      (tx + border >  this.ww) this.panX =  this.ww / 2 + tw / 2 - border;
		else if (tx - border < -tw) this.panX = -tw / 2 - this.ww / 2 + border;
		if      (ty + border >  hh) this.panY =  hh / 2 + tw / 2 - border;
		else if (ty - border < -tw) this.panY = -tw / 2 - hh / 2 + border;

		if (Operator.shortcut(Config.keymap.view_reset)) {
			this.panX = 0.0;
			this.panY = 0.0;
			this.panScale = 1.0;
		}
	}
}
