package iron.format.obj;

class Loader {

	public var indexedVertices:Array<Float> = null;
	public var indexedUVs:Array<Float> = null;
	public var indexedNormals:Array<Float> = null;
	public var index:Int;

	// public var data:Array<Float>;
	public var indices:Array<Int>;

	public function new(objData:String) {

		var vertices:Array<Float> = [];
		var uvs:Array<Float> = [];
		var normals:Array<Float> = [];

		var vertexIndices:Array<Int> = [];
		var uvIndices:Array<Int> = [];
		var normalIndices:Array<Int> = [];

		var tempVertices:Array<Array<Float>> = [];
		var tempUVs:Array<Array<Float>> = [];
		var tempNormals:Array<Array<Float>> = [];

		var lines:Array<String> = objData.split("\n");

		for (i in 0...lines.length) {
			var words:Array<String> = lines[i].split(" ");

			if (words[0] == "v") {
				var vector:Array<Float> = [];
				vector.push(Std.parseFloat(words[1]));
				vector.push(Std.parseFloat(words[2]));
				vector.push(Std.parseFloat(words[3]));
				tempVertices.push(vector);
			}
			else if (words[0] == "vt") {
				var vector:Array<Float> = [];
				vector.push(Std.parseFloat(words[1]));
				vector.push(Std.parseFloat(words[2]));
				tempUVs.push(vector);
			}
			else if (words[0] == "vn") {
				var vector:Array<Float> = [];
				vector.push(Std.parseFloat(words[1]));
				vector.push(Std.parseFloat(words[2]));
				vector.push(Std.parseFloat(words[3]));
				tempNormals.push(vector);
			}
			else if (words[0] == "f") {
				var sec1:Array<String> = words[1].split("/");
				var sec2:Array<String> = words[2].split("/");
				var sec3:Array<String> = words[3].split("/");

				var vi0 = Std.int(Std.parseFloat(sec1[0]));
				var vi1 = Std.int(Std.parseFloat(sec2[0]));
				var vi2 = Std.int(Std.parseFloat(sec3[0]));
				vertexIndices.push(vi0);
				vertexIndices.push(vi1);
				vertexIndices.push(vi2);

				var vuv0 = Std.int(Std.parseFloat(sec1[1]));
				var vuv1 = Std.int(Std.parseFloat(sec2[1]));
				var vuv2 = Std.int(Std.parseFloat(sec3[1]));
				uvIndices.push(vuv0);
				uvIndices.push(vuv1);
				uvIndices.push(vuv2);
				
				var vn0 = Std.int(Std.parseFloat(sec1[2]));
				var vn1 = Std.int(Std.parseFloat(sec2[2]));
				var vn2 = Std.int(Std.parseFloat(sec3[2]));
				normalIndices.push(vn0);
				normalIndices.push(vn1);
				normalIndices.push(vn2);

				if (words.length > 4) {
					var sec4:Array<String> = words[4].split("/");

					vertexIndices.push(vi2);
					vertexIndices.push(Std.int(Std.parseFloat(sec4[0])));
					vertexIndices.push(vi0);

					uvIndices.push(vuv2);
					uvIndices.push(Std.int(Std.parseFloat(sec4[1])));
					uvIndices.push(vuv0);

					normalIndices.push(vn2);
					normalIndices.push(Std.int(Std.parseFloat(sec4[2])));
					normalIndices.push(vn0);
				}
			}
		}

		// UVs not found
		if (tempUVs.length == 0) {
			return;
		}

		for (i in 0...vertexIndices.length) {
			var vertex:Array<Float> = tempVertices[vertexIndices[i] - 1];
			var uv:Array<Float> = tempUVs[uvIndices[i] - 1];
			var normal:Array<Float> = tempNormals[normalIndices[i] - 1];

			vertices.push(vertex[0]);
			vertices.push(vertex[1]);
			vertices.push(vertex[2]);
			uvs.push(uv[0]);
			uvs.push(uv[1]);
			normals.push(normal[0]);
			normals.push(normal[1]);
			normals.push(normal[2]);
		}

		build(vertices, uvs, normals);

		// data = [];
		// for (i in 0...Std.int(vertices.length / 3)) {
		// 	data.push(indexedVertices[i * 3]);
		// 	data.push(indexedVertices[i * 3 + 1]);
		// 	data.push(indexedVertices[i * 3 + 2]);
		// 	data.push(indexedUVs[i * 2]);
		// 	data.push(1-indexedUVs[i * 2 + 1]);
		// 	data.push(indexedNormals[i * 3]);
		// 	data.push(indexedNormals[i * 3 + 1]);
		// 	data.push(indexedNormals[i * 3 + 2]);
		// }
	}

	function build(vertices:Array<Float>, uvs:Array<Float>, normals:Array<Float>) {
		indexedVertices = [];
		indexedUVs = [];
		indexedNormals = [];
		indices = [];

		// For each input vertex
		for (i in 0...Std.int(vertices.length / 3)) {

			// Try to find a similar vertex in out_XXXX
			// var found:Bool = getSimilarVertexIndex(
				// vertices[i * 3], vertices[i * 3 + 1], vertices[i * 3 + 2],
				// uvs[i * 2], uvs[i * 2 + 1],
				// normals[i * 3], normals[i * 3 + 1], normals[i * 3 + 2]);

			// if (found) { // A similar vertex is already in the VBO, use it instead !
				// indices.push(index);
			// }
			// else { // If not, it needs to be added in the output data.
				indexedVertices.push(vertices[i * 3]);
				indexedVertices.push(vertices[i * 3 + 1]);
				indexedVertices.push(vertices[i * 3 + 2]);
				indexedUVs.push(uvs[i * 2 ]);
				indexedUVs.push(1-uvs[i * 2 + 1]);
				indexedNormals.push(normals[i * 3]);
				indexedNormals.push(normals[i * 3 + 1]);
				indexedNormals.push(normals[i * 3 + 2]);
				indices.push(Std.int(indexedVertices.length / 3) - 1);
			// }
		}
	}

	// Returns true if v1 can be considered equal to v2
	// function isNear(v1:Float, v2:Float):Bool {
	// 	return Math.abs(v1 - v2) < 0.001;
	// }

	// // Searches through all already-exported vertices for a similar one.
	// // Similar = same position + same UVs + same normal
	// function getSimilarVertexIndex( 
	// 	vertexX:Float, vertexY:Float, vertexZ:Float,
	// 	uvX:Float, uvY:Float,
	// 	normalX:Float, normalY:Float, normalZ:Float
	// ):Bool {
	// 	// Lame linear search
	// 	for (i in 0...Std.int(indexedVertices.length / 3)) {
	// 		if (
	// 			isNear(vertexX, indexedVertices[i * 3]) &&
	// 			isNear(vertexY, indexedVertices[i * 3 + 1]) &&
	// 			isNear(vertexZ, indexedVertices[i * 3 + 2]) &&
	// 			isNear(uvX    , indexedUVs     [i * 2]) &&
	// 			isNear(uvY    , indexedUVs     [i * 2 + 1]) &&
	// 			isNear(normalX, indexedNormals [i * 3]) &&
	// 			isNear(normalY, indexedNormals [i * 3 + 1]) &&
	// 			isNear(normalZ, indexedNormals [i * 3 + 2])
	// 		) {
	// 			index = i;
	// 			return true;
	// 		}
	// 	}
	// 	// No other vertex could be used instead.
	// 	// Looks like we'll have to add it to the VBO.
	// 	return false;
	// }
}
