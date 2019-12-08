// An Analytic Model for Full Spectral Sky-Dome Radiance
// Lukas Hosek and Alexander Wilkie
// Based on https://github.com/ddiakopoulos/sandbox
package arm.data;

import kha.math.FastVector3;
import iron.data.WorldData;

class HosekWilkieRadianceData {

	public var hosekA = new FastVector3();
	public var hosekB = new FastVector3();
	public var hosekC = new FastVector3();
	public var hosekD = new FastVector3();
	public var hosekE = new FastVector3();
	public var hosekF = new FastVector3();
	public var hosekG = new FastVector3();
	public var hosekH = new FastVector3();
	public var hosekI = new FastVector3();
	public var hosekZ = new FastVector3();

	function evaluateSpline(spline: Array<Float>, index: Int, stride: Int, value: Float): Float {
		return
		1  * Math.pow(1 - value, 5) *                      spline[index             ] +
		5  * Math.pow(1 - value, 4) * Math.pow(value, 1) * spline[index + 1 * stride] +
		10 * Math.pow(1 - value, 3) * Math.pow(value, 2) * spline[index + 2 * stride] +
		10 * Math.pow(1 - value, 2) * Math.pow(value, 3) * spline[index + 3 * stride] +
		5  * Math.pow(1 - value, 1) * Math.pow(value, 4) * spline[index + 4 * stride] +
		1  *                          Math.pow(value, 5) * spline[index + 5 * stride];
	}

	function clamp(n: Int, lower: Int, upper: Int): Int {
		return n <= lower ? lower : n >= upper ? upper : n;
	}

	function clampF(n: Float, lower: Float, upper: Float): Float {
		return n <= lower ? lower : n >= upper ? upper : n;
	}

	function evaluate(dataset: Array<Float>, index: Int, stride: Int, turbidity: Float, albedo: Float, sunTheta: Float): Float {
		// Splines are functions of elevation^1/3
		var elevationK = Math.pow(Math.max(0.0, 1.0 - sunTheta / (Math.PI / 2.0)), 1.0 / 3.0);

		// Table has values for turbidity 1..10
		var turbidity0 = clamp(Std.int(turbidity), 1, 10);
		var turbidity1 = Std.int(Math.min(turbidity0 + 1, 10));
		var turbidityK = clampF(turbidity - turbidity0, 0.0, 1.0);

		var datasetA0Index = index;
		var datasetA1Index = index + stride * 6 * 10;

		var a0t0 = evaluateSpline(dataset, datasetA0Index + stride * 6 * (turbidity0 - 1), stride, elevationK);
		var a1t0 = evaluateSpline(dataset, datasetA1Index + stride * 6 * (turbidity0 - 1), stride, elevationK);
		var a0t1 = evaluateSpline(dataset, datasetA0Index + stride * 6 * (turbidity1 - 1), stride, elevationK);
		var a1t1 = evaluateSpline(dataset, datasetA1Index + stride * 6 * (turbidity1 - 1), stride, elevationK);

		return a0t0 * (1 - albedo) * (1 - turbidityK) + a1t0 * albedo * (1 - turbidityK) + a0t1 * (1 - albedo) * turbidityK + a1t1 * albedo * turbidityK;
	}

	function hosekWilkie(cosTheta: Float, gamma: Float, cosGamma: Float, hosekA: FastVector3, hosekB: FastVector3, hosekC: FastVector3, hosekD: FastVector3, hosekE: FastVector3, hosekF: FastVector3, hosekG: FastVector3, hosekH: FastVector3, hosekI: FastVector3): FastVector3 {
		var val = (1.0 + cosGamma * cosGamma);
		var chix = val / Math.pow(1.0 + hosekH.x * hosekH.x - 2.0 * cosGamma * hosekH.x, 1.5);
		var chiy = val / Math.pow(1.0 + hosekH.y * hosekH.y - 2.0 * cosGamma * hosekH.y, 1.5);
		var chiz = val / Math.pow(1.0 + hosekH.z * hosekH.z - 2.0 * cosGamma * hosekH.z, 1.5);
		var chi = new FastVector3(chix, chiy, chiz);

		var vx = (1.0 + hosekA.x * Math.exp(hosekB.x / (cosTheta + 0.01))) * (hosekC.x + hosekD.x * Math.exp(hosekE.x * gamma) + hosekF.x * (cosGamma * cosGamma) + hosekG.x * chi.x + hosekI.x * Math.sqrt(Math.max(0.0, cosTheta)));
		var vy = (1.0 + hosekA.y * Math.exp(hosekB.y / (cosTheta + 0.01))) * (hosekC.y + hosekD.y * Math.exp(hosekE.y * gamma) + hosekF.y * (cosGamma * cosGamma) + hosekG.y * chi.y + hosekI.y * Math.sqrt(Math.max(0.0, cosTheta)));
		var vz = (1.0 + hosekA.z * Math.exp(hosekB.z / (cosTheta + 0.01))) * (hosekC.z + hosekD.z * Math.exp(hosekE.z * gamma) + hosekF.z * (cosGamma * cosGamma) + hosekG.z * chi.z + hosekI.z * Math.sqrt(Math.max(0.0, cosTheta)));
		return new FastVector3(vx, vy, vz);
	}

	function setVector(v: FastVector3, index: Int, f: Float) {
		index == 0 ? v.x = f : index == 1 ? v.y = f : v.z = f;
	}

	public function new() {}

	public function recompute(sunTheta: Float, turbidity: kha.FastFloat, albedo: kha.FastFloat, normalizedSunY: Float) {
		for (i in 0...3) {
			setVector(hosekA, i, evaluate(HosekWilkieData.datasetsRGB[i], 0, 9, turbidity, albedo, sunTheta));
			setVector(hosekB, i, evaluate(HosekWilkieData.datasetsRGB[i], 1, 9, turbidity, albedo, sunTheta));
			setVector(hosekC, i, evaluate(HosekWilkieData.datasetsRGB[i], 2, 9, turbidity, albedo, sunTheta));
			setVector(hosekD, i, evaluate(HosekWilkieData.datasetsRGB[i], 3, 9, turbidity, albedo, sunTheta));
			setVector(hosekE, i, evaluate(HosekWilkieData.datasetsRGB[i], 4, 9, turbidity, albedo, sunTheta));
			setVector(hosekF, i, evaluate(HosekWilkieData.datasetsRGB[i], 5, 9, turbidity, albedo, sunTheta));
			setVector(hosekG, i, evaluate(HosekWilkieData.datasetsRGB[i], 6, 9, turbidity, albedo, sunTheta));

			// Swapped in the dataset
			setVector(hosekH, i, evaluate(HosekWilkieData.datasetsRGB[i], 8, 9, turbidity, albedo, sunTheta));
			setVector(hosekI, i, evaluate(HosekWilkieData.datasetsRGB[i], 7, 9, turbidity, albedo, sunTheta));

			setVector(hosekZ, i, evaluate(HosekWilkieData.datasetsRGBRad[i], 0, 1, turbidity, albedo, sunTheta));
		}

		if (normalizedSunY != 0.0) {
			var hosekS: FastVector3 = hosekWilkie(Math.cos(sunTheta), 0, 1.0, hosekA, hosekB, hosekC, hosekD, hosekE, hosekF, hosekG, hosekH, hosekI);
			hosekS.x *= hosekZ.x;
			hosekS.y *= hosekZ.y;
			hosekS.z *= hosekZ.z;
			var dotS = hosekS.dot(new FastVector3(0.2126, 0.7152, 0.0722));
			hosekZ.x /= dotS;
			hosekZ.y /= dotS;
			hosekZ.z /= dotS;
			hosekZ = hosekZ.mult(normalizedSunY);
		}
	}
}

class HosekWilkie {
	public static var data: HosekWilkieRadianceData = null;

	public static function recompute(world: WorldData) {
		if (world == null || world.raw.sun_direction == null) return;
		if (data == null) data = new HosekWilkieRadianceData();
		// Clamp Z for night cycle
		var sunZ = world.raw.sun_direction[2] > 0 ? world.raw.sun_direction[2] : 0;
		var sunPositionX = Math.acos(sunZ);
		var normalizedSunY: kha.FastFloat = 1.15;
		data.recompute(sunPositionX, world.raw.turbidity, world.raw.ground_albedo, normalizedSunY);
	}
}
