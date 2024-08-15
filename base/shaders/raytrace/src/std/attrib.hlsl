
#ifndef _ATTRIB_HLSL_
#define _ATTRIB_HLSL_

float2 s16_to_f32(uint val) {
	int a = (int)(val << 16) >> 16;
	int b = (int)(val & 0xffff0000) >> 16;
	return float2(a, b) / 32767.0f;
}

float3 hit_world_position() {
	return WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
}

float3 hit_attribute(float3 vertex_attribute[3], BuiltInTriangleIntersectionAttributes attr) {
	return vertex_attribute[0] +
		attr.barycentrics.x * (vertex_attribute[1] - vertex_attribute[0]) +
		attr.barycentrics.y * (vertex_attribute[2] - vertex_attribute[0]);
}

float2 hit_attribute2d(float2 vertex_attribute[3], BuiltInTriangleIntersectionAttributes attr) {
	return vertex_attribute[0] +
		attr.barycentrics.x * (vertex_attribute[1] - vertex_attribute[0]) +
		attr.barycentrics.y * (vertex_attribute[2] - vertex_attribute[0]);
}

#endif
