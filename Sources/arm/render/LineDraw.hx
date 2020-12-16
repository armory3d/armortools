package arm.render;

import kha.graphics4.PipelineState;
import kha.graphics4.VertexStructure;
import kha.graphics4.VertexBuffer;
import kha.graphics4.IndexBuffer;
import kha.graphics4.VertexData;
import kha.graphics4.Usage;
import kha.graphics4.ConstantLocation;
import kha.graphics4.CompareMode;
import kha.graphics4.CullMode;
import kha.graphics4.TextureFormat;
import kha.graphics4.DepthStencilFormat;
import iron.math.Vec4;
import iron.math.Mat4;
import arm.ui.UIHeader;
import arm.Enums;

class LineDraw {

	public static var color: kha.Color = 0xffff0000;
	public static var strength = 0.005;
	public static var mat: iron.math.Mat4 = null;
	public static var dim: iron.math.Vec4 = null;

	static var vertexBuffer: VertexBuffer;
	static var indexBuffer: IndexBuffer;
	static var pipeline: PipelineState = null;

	static var vp: Mat4;
	static var vpID: ConstantLocation;

	static var vbData: kha.arrays.Float32Array;
	static var ibData: kha.arrays.Uint32Array;

	static inline var maxLines = 300;
	static inline var maxVertices = maxLines * 4;
	static inline var maxIndices = maxLines * 6;
	static var lines = 0;

	static var g: kha.graphics4.Graphics;

	public static function render(g4: kha.graphics4.Graphics) {

		var hide = Operator.shortcut(Config.keymap.stencil_hide, ShortcutDown) || iron.system.Input.getKeyboard().down("control");
		var isPaint = UIHeader.inst.worktab.position == SpacePaint;
		var isDecal = isPaint && Context.layer.fill_layer != null && Context.layer.uvType == UVProject && !Context.layerIsMask;
		if (!isDecal || hide) return;

		mat = Context.layer.decalMat;
		dim = Context.layer.decalMat.getScale();

		g = g4;

		if (pipeline == null) {
			var structure = new VertexStructure();
			structure.add("pos", VertexData.Float3);
			structure.add("col", VertexData.Float3);
			pipeline = new PipelineState();
			pipeline.inputLayout = [structure];
			pipeline.fragmentShader = kha.Shaders.getFragment("line.frag");
			pipeline.vertexShader = kha.Shaders.getVertex("line.vert");
			pipeline.depthWrite = true;
			pipeline.depthMode = CompareMode.Less;
			pipeline.cullMode = CullMode.None;
			pipeline.colorAttachmentCount = 3;
			pipeline.colorAttachments[0] = TextureFormat.RGBA64;
			pipeline.colorAttachments[1] = TextureFormat.RGBA64;
			pipeline.colorAttachments[2] = TextureFormat.RGBA64;
			pipeline.depthStencilAttachment = DepthStencilFormat.DepthOnly;
			pipeline.compile();
			vpID = pipeline.getConstantLocation("VP");
			vp = Mat4.identity();
			vertexBuffer = new VertexBuffer(maxVertices, structure, Usage.DynamicUsage);
			indexBuffer = new IndexBuffer(maxIndices, Usage.DynamicUsage);
		}

		begin();
		bounds(mat, dim);
		end();
	}

	static var wpos: Vec4;
	static var vx = new Vec4();
	static var vy = new Vec4();
	static var vz = new Vec4();
	public static function bounds(mat: iron.math.Mat4, dim: iron.math.Vec4) {
		wpos = mat.getLoc();
		var dx = dim.x / 2;
		var dy = dim.y / 2;
		var dz = dim.z / 2;

		var up = mat.up();
		var look = mat.look();
		var right = mat.right();
		up.normalize();
		look.normalize();
		right.normalize();

		vx.setFrom(right);
		vx.mult(dx);
		vy.setFrom(look);
		vy.mult(dy);
		vz.setFrom(up);
		vz.mult(dz);

		lineb(-1, -1, -1,  1, -1, -1);
		lineb(-1,  1, -1,  1,  1, -1);
		lineb(-1, -1,  1,  1, -1,  1);
		lineb(-1,  1,  1,  1,  1,  1);

		lineb(-1, -1, -1, -1,  1, -1);
		lineb(-1, -1,  1, -1,  1,  1);
		lineb( 1, -1, -1,  1,  1, -1);
		lineb( 1, -1,  1,  1,  1,  1);

		lineb(-1, -1, -1, -1, -1,  1);
		lineb(-1,  1, -1, -1,  1,  1);
		lineb( 1, -1, -1,  1, -1,  1);
		lineb( 1,  1, -1,  1,  1,  1);
	}

	static var v1 = new Vec4();
	static var v2 = new Vec4();
	static var t = new Vec4();
	static function lineb(a: Int, b: Int, c: Int, d: Int, e: Int, f: Int) {
		v1.setFrom(wpos);
		t.setFrom(vx); t.mult(a); v1.add(t);
		t.setFrom(vy); t.mult(b); v1.add(t);
		t.setFrom(vz); t.mult(c); v1.add(t);

		v2.setFrom(wpos);
		t.setFrom(vx); t.mult(d); v2.add(t);
		t.setFrom(vy); t.mult(e); v2.add(t);
		t.setFrom(vz); t.mult(f); v2.add(t);

		line(v1.x, v1.y, v1.z, v2.x, v2.y, v2.z);
	}

	static var midPoint = new Vec4();
	static var midLine = new Vec4();
	static var corner1 = new Vec4();
	static var corner2 = new Vec4();
	static var corner3 = new Vec4();
	static var corner4 = new Vec4();
	static var cameraLook = new Vec4();
	public static function line(x1: Float, y1: Float, z1: Float, x2: Float, y2: Float, z2: Float) {

		if (lines >= maxLines) { end(); begin(); }

		midPoint.set(x1 + x2, y1 + y2, z1 + z2);
		midPoint.mult(0.5);

		midLine.set(x1, y1, z1);
		midLine.sub(midPoint);

		var camera = iron.Scene.active.camera;
		cameraLook = camera.transform.world.getLoc();
		cameraLook.sub(midPoint);

		var lineWidth = cameraLook.cross(midLine);
		lineWidth.normalize();
		lineWidth.mult(strength);

		corner1.set(x1, y1, z1).add(lineWidth);
		corner2.set(x1, y1, z1).sub(lineWidth);
		corner3.set(x2, y2, z2).sub(lineWidth);
		corner4.set(x2, y2, z2).add(lineWidth);

		var i = lines * 24; // 4 * 6 (structure len)
		addVbData(i, [corner1.x, corner1.y, corner1.z, color.R, color.G, color.B]);
		i += 6;
		addVbData(i, [corner2.x, corner2.y, corner2.z, color.R, color.G, color.B]);
		i += 6;
		addVbData(i, [corner3.x, corner3.y, corner3.z, color.R, color.G, color.B]);
		i += 6;
		addVbData(i, [corner4.x, corner4.y, corner4.z, color.R, color.G, color.B]);

		i = lines * 6;
		ibData[i    ] = lines * 4;
		ibData[i + 1] = lines * 4 + 1;
		ibData[i + 2] = lines * 4 + 2;
		ibData[i + 3] = lines * 4 + 2;
		ibData[i + 4] = lines * 4 + 3;
		ibData[i + 5] = lines * 4;

		lines++;
	}

	static function begin() {
		lines = 0;
		vbData = vertexBuffer.lock();
		ibData = indexBuffer.lock();
	}

	static function end() {
		vertexBuffer.unlock();
		indexBuffer.unlock();

		g.setVertexBuffer(vertexBuffer);
		g.setIndexBuffer(indexBuffer);
		g.setPipeline(pipeline);
		var camera = iron.Scene.active.camera;
		vp.setFrom(camera.V);
		vp.multmat(camera.P);
		g.setMatrix(vpID, vp.self);
		g.drawIndexedVertices(0, lines * 6);
	}

	inline static function addVbData(i: Int, data: Array<Float>) {
		for (offset in 0...6) {
			vbData.set(i + offset, data[offset]);
		}
	}
}
