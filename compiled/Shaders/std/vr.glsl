uniform mat4 U; // Undistortion
uniform float maxRadSq;

// GoogleVR Distortion using Vertex Displacement
float distortionFactor(const float rSquared) {
	float ret = 0.0;
	ret = rSquared * (ret + U[1][1]);
	ret = rSquared * (ret + U[0][1]);
	ret = rSquared * (ret + U[3][0]);
	ret = rSquared * (ret + U[2][0]);
	ret = rSquared * (ret + U[1][0]);
	ret = rSquared * (ret + U[0][0]);
	return ret + 1.0;
}
// Convert point from world space to undistorted camera space
vec4 undistort(const mat4 WV, vec4 pos) {
	// Go to camera space
	pos = WV * pos;
	const float nearClip = 0.1;
	if (pos.z <= -nearClip) {  // Reminder: Forward is -Z
		// Undistort the point's coordinates in XY
		float r2 = clamp(dot(pos.xy, pos.xy) / (pos.z * pos.z), 0.0, maxRadSq);
		pos.xy *= distortionFactor(r2);
	}
	return pos;
}
