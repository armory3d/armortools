
const int maxLights = 16;
const int maxLightsCluster = 4; // Ensure fast loop unroll before going higher
const float clusterNear = 3.0;
const vec3 clusterSlices = vec3(16, 16, 16);

int getClusterI(vec2 tc, float viewz, vec2 cameraPlane) {
	int sliceZ = 0;
	float cnear = clusterNear + cameraPlane.x;
	if (viewz >= cnear) {
		float z = log(viewz - cnear + 1.0) / log(cameraPlane.y - cnear + 1.0);
		sliceZ = int(z * (clusterSlices.z - 1)) + 1;
	}
	return int(tc.x * clusterSlices.x) +
		   int(int(tc.y * clusterSlices.y) * clusterSlices.x) +
		   int(sliceZ * clusterSlices.x * clusterSlices.y);
}
