
class LineDraw {

	static color: Color = 0xffff0000;
	static strength = 0.005;
	static mat: TMat4 = null;
	static dim: TVec4 = null;

	static vertexBuffer: VertexBufferRaw;
	static indexBuffer: IndexBufferRaw;
	static pipeline: PipelineStateRaw = null;

	static vp: TMat4;
	static vpID: ConstantLocation;

	static vbData: DataView;
	static ibData: Uint32Array;

	static maxLines = 300;
	static maxVertices = LineDraw.maxLines * 4;
	static maxIndices = LineDraw.maxLines * 6;
	static lines = 0;

	static g: Graphics4Raw;

	static render = (g4: Graphics4Raw, matrix: TMat4) => {
		LineDraw.g = g4;
		LineDraw.mat = matrix;
		LineDraw.dim = Mat4.getScale(matrix);

		if (LineDraw.pipeline == null) {
			let structure = VertexStructure.create();
			VertexStructure.add(structure, "pos", VertexData.F32_3X);
			VertexStructure.add(structure, "col", VertexData.F32_3X);
			LineDraw.pipeline = PipelineState.create();
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
			PipelineState.compile(LineDraw.pipeline);
			LineDraw.vpID = PipelineState.getConstantLocation(LineDraw.pipeline, "VP");
			LineDraw.vp = Mat4.identity();
			LineDraw.vertexBuffer = VertexBuffer.create(LineDraw.maxVertices, structure, Usage.DynamicUsage);
			LineDraw.indexBuffer = IndexBuffer.create(LineDraw.maxIndices, Usage.DynamicUsage);
		}

		LineDraw.begin();
		LineDraw.bounds(LineDraw.mat, LineDraw.dim);
		LineDraw.end();
	}

	static wpos: TVec4;
	static vx = Vec4.create();
	static vy = Vec4.create();
	static vz = Vec4.create();

	static bounds = (mat: TMat4, dim: TVec4) => {
		LineDraw.wpos = Mat4.getLoc(mat);
		let dx = dim.x / 2;
		let dy = dim.y / 2;
		let dz = dim.z / 2;

		let up = Mat4.up(mat);
		let look = Mat4.look(mat);
		let right = Mat4.right(mat);
		Vec4.normalize(up);
		Vec4.normalize(look);
		Vec4.normalize(right);

		Vec4.setFrom(LineDraw.vx, right);
		Vec4.mult(LineDraw.vx, dx);
		Vec4.setFrom(LineDraw.vy, look);
		Vec4.mult(LineDraw.vy, dy);
		Vec4.setFrom(LineDraw.vz, up);
		Vec4.mult(LineDraw.vz, dz);

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

	static v1 = Vec4.create();
	static v2 = Vec4.create();
	static t = Vec4.create();

	static lineb = (a: i32, b: i32, c: i32, d: i32, e: i32, f: i32) => {
		Vec4.setFrom(LineDraw.v1, LineDraw.wpos);
		Vec4.setFrom(LineDraw.t, LineDraw.vx); Vec4.mult(LineDraw.t, a); Vec4.add(LineDraw.v1, LineDraw.t);
		Vec4.setFrom(LineDraw.t, LineDraw.vy); Vec4.mult(LineDraw.t, b); Vec4.add(LineDraw.v1, LineDraw.t);
		Vec4.setFrom(LineDraw.t, LineDraw.vz); Vec4.mult(LineDraw.t, c); Vec4.add(LineDraw.v1, LineDraw.t);

		Vec4.setFrom(LineDraw.v2, LineDraw.wpos);
		Vec4.setFrom(LineDraw.t, LineDraw.vx); Vec4.mult(LineDraw.t, d); Vec4.add(LineDraw.v2, LineDraw.t);
		Vec4.setFrom(LineDraw.t, LineDraw.vy); Vec4.mult(LineDraw.t, e); Vec4.add(LineDraw.v2, LineDraw.t);
		Vec4.setFrom(LineDraw.t, LineDraw.vz); Vec4.mult(LineDraw.t, f); Vec4.add(LineDraw.v2, LineDraw.t);

		LineDraw.line(LineDraw.v1.x, LineDraw.v1.y, LineDraw.v1.z, LineDraw.v2.x, LineDraw.v2.y, LineDraw.v2.z);
	}

	static midPoint = Vec4.create();
	static midLine = Vec4.create();
	static corner1 = Vec4.create();
	static corner2 = Vec4.create();
	static corner3 = Vec4.create();
	static corner4 = Vec4.create();
	static cameraLook = Vec4.create();

	static line = (x1: f32, y1: f32, z1: f32, x2: f32, y2: f32, z2: f32) => {
		if (LineDraw.lines >= LineDraw.maxLines) {
			LineDraw.end();
			LineDraw.begin();
		}

		Vec4.set(LineDraw.midPoint, x1 + x2, y1 + y2, z1 + z2);
		Vec4.mult(LineDraw.midPoint, 0.5);

		Vec4.set(LineDraw.midLine, x1, y1, z1);
		Vec4.sub(LineDraw.midLine, LineDraw.midPoint);

		let camera = Scene.camera;
		LineDraw.cameraLook = Mat4.getLoc(camera.base.transform.world);
		Vec4.sub(LineDraw.cameraLook, LineDraw.midPoint);

		let lineWidth = Vec4.cross(LineDraw.cameraLook, LineDraw.midLine);
		Vec4.normalize(lineWidth, );
		Vec4.mult(lineWidth, LineDraw.strength);

		Vec4.add(Vec4.set(LineDraw.corner1, x1, y1, z1), lineWidth);
		Vec4.sub(Vec4.set(LineDraw.corner2, x1, y1, z1), lineWidth);
		Vec4.sub(Vec4.set(LineDraw.corner3, x2, y2, z2), lineWidth);
		Vec4.add(Vec4.set(LineDraw.corner4, x2, y2, z2), lineWidth);

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
		LineDraw.vbData = VertexBuffer.lock(LineDraw.vertexBuffer);
		LineDraw.ibData = IndexBuffer.lock(LineDraw.indexBuffer);
	}

	static end = () => {
		VertexBuffer.unlock(LineDraw.vertexBuffer);
		IndexBuffer.unlock(LineDraw.indexBuffer);

		Graphics4.setVertexBuffer(LineDraw.vertexBuffer);
		Graphics4.setIndexBuffer(LineDraw.indexBuffer);
		Graphics4.setPipeline(LineDraw.pipeline);
		let camera = Scene.camera;
		Mat4.setFrom(LineDraw.vp, camera.V);
		Mat4.multmat(LineDraw.vp, camera.P);
		Graphics4.setMatrix(LineDraw.vpID, LineDraw.vp);
		Graphics4.drawIndexedVertices(0, LineDraw.lines * 6);
	}

	static addVbData = (i: i32, data: f32[]) => {
		for (let offset = 0; offset < 6; ++offset) {
			LineDraw.vbData.setFloat32((i + offset) * 4, data[offset], true);
		}
	}
}
