
class LineDraw {

	static color: Color = 0xffff0000;
	static strength = 0.005;
	static mat: Mat4 = null;
	static dim: Vec4 = null;

	static vertexBuffer: VertexBuffer;
	static indexBuffer: IndexBuffer;
	static pipeline: PipelineState = null;

	static vp: Mat4;
	static vpID: ConstantLocation;

	static vbData: DataView;
	static ibData: Uint32Array;

	static maxLines = 300;
	static maxVertices = LineDraw.maxLines * 4;
	static maxIndices = LineDraw.maxLines * 6;
	static lines = 0;

	static g: Graphics4;

	static render = (g4: Graphics4, matrix: Mat4) => {
		LineDraw.g = g4;
		LineDraw.mat = matrix;
		LineDraw.dim = matrix.getScale();

		if (LineDraw.pipeline == null) {
			let structure = new VertexStructure();
			structure.add("pos", VertexData.F32_3X);
			structure.add("col", VertexData.F32_3X);
			LineDraw.pipeline = new PipelineState();
			LineDraw.pipeline.inputLayout = [structure];
			LineDraw.pipeline.fragmentShader = System.getShader("line.frag");
			LineDraw.pipeline.vertexShader = System.getShader("line.vert");
			LineDraw.pipeline.depthWrite = true;
			LineDraw.pipeline.depthMode = CompareMode.Less;
			LineDraw.pipeline.cullMode = CullMode.None;
			LineDraw.pipeline.colorAttachmentCount = 3;
			LineDraw.pipeline.colorAttachments[0] = TextureFormat.RGBA64;
			LineDraw.pipeline.colorAttachments[1] = TextureFormat.RGBA64;
			LineDraw.pipeline.colorAttachments[2] = TextureFormat.RGBA64;
			LineDraw.pipeline.depthStencilAttachment = DepthStencilFormat.DepthOnly;
			LineDraw.pipeline.compile();
			LineDraw.vpID = LineDraw.pipeline.getConstantLocation("VP");
			LineDraw.vp = Mat4.identity();
			LineDraw.vertexBuffer = new VertexBuffer(LineDraw.maxVertices, structure, Usage.DynamicUsage);
			LineDraw.indexBuffer = new IndexBuffer(LineDraw.maxIndices, Usage.DynamicUsage);
		}

		LineDraw.begin();
		LineDraw.bounds(LineDraw.mat, LineDraw.dim);
		LineDraw.end();
	}

	static wpos: Vec4;
	static vx = new Vec4();
	static vy = new Vec4();
	static vz = new Vec4();

	static bounds = (mat: Mat4, dim: Vec4) => {
		LineDraw.wpos = mat.getLoc();
		let dx = dim.x / 2;
		let dy = dim.y / 2;
		let dz = dim.z / 2;

		let up = mat.up();
		let look = mat.look();
		let right = mat.right();
		up.normalize();
		look.normalize();
		right.normalize();

		LineDraw.vx.setFrom(right);
		LineDraw.vx.mult(dx);
		LineDraw.vy.setFrom(look);
		LineDraw.vy.mult(dy);
		LineDraw.vz.setFrom(up);
		LineDraw.vz.mult(dz);

		LineDraw.lineb(-1, -1, -1,  1, -1, -1);
		LineDraw.lineb(-1,  1, -1,  1,  1, -1);
		LineDraw.lineb(-1, -1,  1,  1, -1,  1);
		LineDraw.lineb(-1,  1,  1,  1,  1,  1);

		LineDraw.lineb(-1, -1, -1, -1,  1, -1);
		LineDraw.lineb(-1, -1,  1, -1,  1,  1);
		LineDraw.lineb( 1, -1, -1,  1,  1, -1);
		LineDraw.lineb( 1, -1,  1,  1,  1,  1);

		LineDraw.lineb(-1, -1, -1, -1, -1,  1);
		LineDraw.lineb(-1,  1, -1, -1,  1,  1);
		LineDraw.lineb( 1, -1, -1,  1, -1,  1);
		LineDraw.lineb( 1,  1, -1,  1,  1,  1);
	}

	static v1 = new Vec4();
	static v2 = new Vec4();
	static t = new Vec4();

	static lineb = (a: i32, b: i32, c: i32, d: i32, e: i32, f: i32) => {
		LineDraw.v1.setFrom(LineDraw.wpos);
		LineDraw.t.setFrom(LineDraw.vx); LineDraw.t.mult(a); LineDraw.v1.add(LineDraw.t);
		LineDraw.t.setFrom(LineDraw.vy); LineDraw.t.mult(b); LineDraw.v1.add(LineDraw.t);
		LineDraw.t.setFrom(LineDraw.vz); LineDraw.t.mult(c); LineDraw.v1.add(LineDraw.t);

		LineDraw.v2.setFrom(LineDraw.wpos);
		LineDraw.t.setFrom(LineDraw.vx); LineDraw.t.mult(d); LineDraw.v2.add(LineDraw.t);
		LineDraw.t.setFrom(LineDraw.vy); LineDraw.t.mult(e); LineDraw.v2.add(LineDraw.t);
		LineDraw.t.setFrom(LineDraw.vz); LineDraw.t.mult(f); LineDraw.v2.add(LineDraw.t);

		LineDraw.line(LineDraw.v1.x, LineDraw.v1.y, LineDraw.v1.z, LineDraw.v2.x, LineDraw.v2.y, LineDraw.v2.z);
	}

	static midPoint = new Vec4();
	static midLine = new Vec4();
	static corner1 = new Vec4();
	static corner2 = new Vec4();
	static corner3 = new Vec4();
	static corner4 = new Vec4();
	static cameraLook = new Vec4();

	static line = (x1: f32, y1: f32, z1: f32, x2: f32, y2: f32, z2: f32) => {
		if (LineDraw.lines >= LineDraw.maxLines) {
			LineDraw.end();
			LineDraw.begin();
		}

		LineDraw.midPoint.set(x1 + x2, y1 + y2, z1 + z2);
		LineDraw.midPoint.mult(0.5);

		LineDraw.midLine.set(x1, y1, z1);
		LineDraw.midLine.sub(LineDraw.midPoint);

		let camera = Scene.active.camera;
		LineDraw.cameraLook = camera.transform.world.getLoc();
		LineDraw.cameraLook.sub(LineDraw.midPoint);

		let lineWidth = LineDraw.cameraLook.cross(LineDraw.midLine);
		lineWidth.normalize();
		lineWidth.mult(LineDraw.strength);

		LineDraw.corner1.set(x1, y1, z1).add(lineWidth);
		LineDraw.corner2.set(x1, y1, z1).sub(lineWidth);
		LineDraw.corner3.set(x2, y2, z2).sub(lineWidth);
		LineDraw.corner4.set(x2, y2, z2).add(lineWidth);

		let i = LineDraw.lines * 24; // 4 * 6 (structure len)
		LineDraw.addVbData(i, [LineDraw.corner1.x, LineDraw.corner1.y, LineDraw.corner1.z, color_get_rb(LineDraw.color) / 255, color_get_gb(LineDraw.color) / 255, color_get_ab(LineDraw.color) / 255]);
		i += 6;
		LineDraw.addVbData(i, [LineDraw.corner2.x, LineDraw.corner2.y, LineDraw.corner2.z, color_get_rb(LineDraw.color) / 255, color_get_gb(LineDraw.color) / 255, color_get_ab(LineDraw.color) / 255]);
		i += 6;
		LineDraw.addVbData(i, [LineDraw.corner3.x, LineDraw.corner3.y, LineDraw.corner3.z, color_get_rb(LineDraw.color) / 255, color_get_gb(LineDraw.color) / 255, color_get_ab(LineDraw.color) / 255]);
		i += 6;
		LineDraw.addVbData(i, [LineDraw.corner4.x, LineDraw.corner4.y, LineDraw.corner4.z, color_get_rb(LineDraw.color) / 255, color_get_gb(LineDraw.color) / 255, color_get_ab(LineDraw.color) / 255]);

		i = LineDraw.lines * 6;
		LineDraw.ibData[i    ] = LineDraw.lines * 4;
		LineDraw.ibData[i + 1] = LineDraw.lines * 4 + 1;
		LineDraw.ibData[i + 2] = LineDraw.lines * 4 + 2;
		LineDraw.ibData[i + 3] = LineDraw.lines * 4 + 2;
		LineDraw.ibData[i + 4] = LineDraw.lines * 4 + 3;
		LineDraw.ibData[i + 5] = LineDraw.lines * 4;

		LineDraw.lines++;
	}

	static begin = () => {
		LineDraw.lines = 0;
		LineDraw.vbData = LineDraw.vertexBuffer.lock();
		LineDraw.ibData = LineDraw.indexBuffer.lock();
	}

	static end = () => {
		LineDraw.vertexBuffer.unlock();
		LineDraw.indexBuffer.unlock();

		LineDraw.g.setVertexBuffer(LineDraw.vertexBuffer);
		LineDraw.g.setIndexBuffer(LineDraw.indexBuffer);
		LineDraw.g.setPipeline(LineDraw.pipeline);
		let camera = Scene.active.camera;
		LineDraw.vp.setFrom(camera.V);
		LineDraw.vp.multmat(camera.P);
		LineDraw.g.setMatrix(LineDraw.vpID, LineDraw.vp);
		LineDraw.g.drawIndexedVertices(0, LineDraw.lines * 6);
	}

	static addVbData = (i: i32, data: f32[]) => {
		for (let offset = 0; offset < 6; ++offset) {
			LineDraw.vbData.setFloat32((i + offset) * 4, data[offset], true);
		}
	}
}
