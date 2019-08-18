// An Analytic Model for Full Spectral Sky-Dome Radiance
// Lukas Hosek and Alexander Wilkie
// Based on https://github.com/ddiakopoulos/sandbox
package arm;

import kha.math.FastVector3;
import iron.data.WorldData;

class HosekWilkieRadianceData {
	
	public var A = new FastVector3();
	public var B = new FastVector3();
	public var C = new FastVector3();
	public var D = new FastVector3();
	public var E = new FastVector3();
	public var F = new FastVector3();
	public var G = new FastVector3();
	public var H = new FastVector3();
	public var I = new FastVector3();
	public var Z = new FastVector3();

	function evaluateSpline(spline:Array<Float>, index:Int, stride:Int, value:Float):Float {
		return
		1 *  Math.pow(1 - value, 5) *                      spline[index             ] +
		5 *  Math.pow(1 - value, 4) * Math.pow(value, 1) * spline[index + 1 * stride] +
		10 * Math.pow(1 - value, 3) * Math.pow(value, 2) * spline[index + 2 * stride] +
		10 * Math.pow(1 - value, 2) * Math.pow(value, 3) * spline[index + 3 * stride] +
		5 *  Math.pow(1 - value, 1) * Math.pow(value, 4) * spline[index + 4 * stride] +
		1 *                           Math.pow(value, 5) * spline[index + 5 * stride];
	}
	
	function clamp(n:Int, lower:Int, upper:Int) {
		return n <= lower ? lower : n >= upper ? upper : n;
	}
	
	function clampF(n:Float, lower:Float, upper:Float) {
		return n <= lower ? lower : n >= upper ? upper : n;
	}
	
	function evaluate(dataset:Array<Float>, index:Int, stride:Int, turbidity:Float, albedo:Float, sunTheta:Float):Float {
		// Splines are functions of elevation^1/3
		var elevationK:Float = Math.pow(Math.max(0.0, 1.0 - sunTheta / (Math.PI / 2.0)), 1.0 / 3.0);
		
		// Table has values for turbidity 1..10
		var turbidity0:Int = clamp(Std.int(turbidity), 1, 10);
		var turbidity1:Int = Std.int(Math.min(turbidity0 + 1, 10));
		var turbidityK:Float = clampF(turbidity - turbidity0, 0.0, 1.0);
		
		var datasetA0Index = index;
		var datasetA1Index = index + stride * 6 * 10;
		
		var a0t0:Float = evaluateSpline(dataset, datasetA0Index + stride * 6 * (turbidity0 - 1), stride, elevationK);
		var a1t0:Float = evaluateSpline(dataset, datasetA1Index + stride * 6 * (turbidity0 - 1), stride, elevationK);
		var a0t1:Float = evaluateSpline(dataset, datasetA0Index + stride * 6 * (turbidity1 - 1), stride, elevationK);
		var a1t1:Float = evaluateSpline(dataset, datasetA1Index + stride * 6 * (turbidity1 - 1), stride, elevationK);
		
		return a0t0 * (1 - albedo) * (1 - turbidityK) + a1t0 * albedo * (1 - turbidityK) + a0t1 * (1 - albedo) * turbidityK + a1t1 * albedo * turbidityK;
	}
	
	function hosek_wilkie(cos_theta:Float, gamma:Float, cos_gamma:Float, A:FastVector3, B:FastVector3, C:FastVector3, D:FastVector3, E:FastVector3, F:FastVector3, G:FastVector3, H:FastVector3, I:FastVector3):FastVector3 {
		var val = (1.0 + cos_gamma * cos_gamma);
		var chix = val / Math.pow(1.0 + H.x * H.x - 2.0 * cos_gamma * H.x, 1.5);
		var chiy = val / Math.pow(1.0 + H.y * H.y - 2.0 * cos_gamma * H.y, 1.5);
		var chiz = val / Math.pow(1.0 + H.z * H.z - 2.0 * cos_gamma * H.z, 1.5);
		var chi = new FastVector3(chix, chiy, chiz);
		
		var vx = (1.0 + A.x * Math.exp(B.x / (cos_theta + 0.01))) * (C.x + D.x * Math.exp(E.x * gamma) + F.x * (cos_gamma * cos_gamma) + G.x * chi.x + I.x * Math.sqrt(Math.max(0.0, cos_theta)));
		var vy = (1.0 + A.y * Math.exp(B.y / (cos_theta + 0.01))) * (C.y + D.y * Math.exp(E.y * gamma) + F.y * (cos_gamma * cos_gamma) + G.y * chi.y + I.y * Math.sqrt(Math.max(0.0, cos_theta)));
		var vz = (1.0 + A.z * Math.exp(B.z / (cos_theta + 0.01))) * (C.z + D.z * Math.exp(E.z * gamma) + F.z * (cos_gamma * cos_gamma) + G.z * chi.z + I.z * Math.sqrt(Math.max(0.0, cos_theta)));
		return new FastVector3(vx, vy, vz);
	}
	
	function setVector(v:FastVector3, index:Int, f:Float) {
		index == 0 ? v.x = f : index == 1 ? v.y = f : v.z = f;
	}
	
	public function new() {}

	public function recompute(sunTheta:Float, turbidity:kha.FastFloat, albedo:kha.FastFloat, normalizedSunY:Float) {
		for (i in 0...3) {
			setVector(A, i, evaluate(HosekWilkieData.datasetsRGB[i], 0, 9, turbidity, albedo, sunTheta));
			setVector(B, i, evaluate(HosekWilkieData.datasetsRGB[i], 1, 9, turbidity, albedo, sunTheta));
			setVector(C, i, evaluate(HosekWilkieData.datasetsRGB[i], 2, 9, turbidity, albedo, sunTheta));
			setVector(D, i, evaluate(HosekWilkieData.datasetsRGB[i], 3, 9, turbidity, albedo, sunTheta));
			setVector(E, i, evaluate(HosekWilkieData.datasetsRGB[i], 4, 9, turbidity, albedo, sunTheta));
			setVector(F, i, evaluate(HosekWilkieData.datasetsRGB[i], 5, 9, turbidity, albedo, sunTheta));
			setVector(G, i, evaluate(HosekWilkieData.datasetsRGB[i], 6, 9, turbidity, albedo, sunTheta));
			
			// Swapped in the dataset
			setVector(H, i, evaluate(HosekWilkieData.datasetsRGB[i], 8, 9, turbidity, albedo, sunTheta));
			setVector(I, i, evaluate(HosekWilkieData.datasetsRGB[i], 7, 9, turbidity, albedo, sunTheta));
			
			setVector(Z, i, evaluate(HosekWilkieData.datasetsRGBRad[i], 0, 1, turbidity, albedo, sunTheta));
		}
		
		if (normalizedSunY != 0.0) {
			var S:FastVector3 = hosek_wilkie(Math.cos(sunTheta), 0, 1.0, A, B, C, D, E, F, G, H, I);
			S.x *= Z.x;
			S.y *= Z.y;
			S.z *= Z.z;
			var dotS = S.dot(new FastVector3(0.2126, 0.7152, 0.0722));
			Z.x /= dotS;
			Z.y /= dotS;
			Z.z /= dotS;
			Z = Z.mult(normalizedSunY);
		}
	}
}

class HosekWilkie {
	public static var data:HosekWilkieRadianceData = null;

	public static function recompute(world:WorldData) {
		if (world == null || world.raw.sun_direction == null) return;
		if (data == null) data = new HosekWilkieRadianceData();
		// Clamp Z for night cycle
		var sunZ = world.raw.sun_direction[2] > 0 ? world.raw.sun_direction[2] : 0;
		var sunPositionX = Math.acos(sunZ);
		var normalizedSunY:kha.FastFloat = 1.15;
		data.recompute(sunPositionX, world.raw.turbidity, world.raw.ground_albedo, normalizedSunY);
	}
}
