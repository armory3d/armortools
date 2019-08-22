#version 450
layout(triangles) in;
layout(triangle_strip) out;
layout(max_vertices=3) out;
in vec3 voxpositionGeom[];
out vec3 voxposition;
void main() {
	vec3 p1 = voxpositionGeom[1] - voxpositionGeom[0];
	vec3 p2 = voxpositionGeom[2] - voxpositionGeom[0];
	vec3 p = abs(cross(p1, p2));
	for (uint i = 0; i < 3; ++i) {
	    voxposition = voxpositionGeom[i];
	    if (p.z > p.x && p.z > p.y) {
	        gl_Position = vec4(voxposition.x, voxposition.y, 0.0, 1.0);
	    }
	    else if (p.x > p.y && p.x > p.z) {
	        gl_Position = vec4(voxposition.y, voxposition.z, 0.0, 1.0);
	    }
	    else {
	        gl_Position = vec4(voxposition.x, voxposition.z, 0.0, 1.0);
	    }
	    EmitVertex();
	}
	EndPrimitive();
}
