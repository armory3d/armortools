
#ifndef _ATTRIB_HLSL_
#define _ATTRIB_HLSL_

float3 hit_world_position() {
	return WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
}

float3 hit_attribute(float3 vertexAttribute[3], BuiltInTriangleIntersectionAttributes attr) {
	return vertexAttribute[0] +
		attr.barycentrics.x * (vertexAttribute[1] - vertexAttribute[0]) +
		attr.barycentrics.y * (vertexAttribute[2] - vertexAttribute[0]);
}

float2 hit_attribute2d(float2 vertexAttribute[3], BuiltInTriangleIntersectionAttributes attr) {
	return vertexAttribute[0] +
		attr.barycentrics.x * (vertexAttribute[1] - vertexAttribute[0]) +
		attr.barycentrics.y * (vertexAttribute[2] - vertexAttribute[0]);
}

#endif
