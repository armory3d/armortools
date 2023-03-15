package arm;

import kha.Image;
import zui.Zui;
import zui.Id;
import iron.math.Vec4;
import iron.object.MeshObject;
import arm.shader.NodeShader;
import arm.ProjectBaseFormat;
import arm.ContextBaseFormat;

@:structInit class TContextBase {
	@:optional public var texture: TAsset = null;
	@:optional public var paintObject: MeshObject;
	@:optional public var mergedObject: MeshObject = null; // For object mask
	@:optional public var mergedObjectIsAtlas = false; // Only objects referenced by atlas are merged

	@:optional public var ddirty = 0; // depth
	@:optional public var pdirty = 0; // paint
	@:optional public var rdirty = 0; // render
	@:optional public var brushBlendDirty = true;
	@:optional public var nodePreviewSocket = 0;

	@:optional public var splitView = false;
	@:optional public var viewIndex = -1;
	@:optional public var viewIndexLast = -1;

	@:optional public var swatch: TSwatchColor;
	@:optional public var pickedColor: TSwatchColor = Project.makeSwatch();
	@:optional public var colorPickerCallback: TSwatchColor->Void = null;

	@:optional public var defaultIrradiance: kha.arrays.Float32Array = null;
	@:optional public var defaultRadiance: Image = null;
	@:optional public var defaultRadianceMipmaps: Array<Image> = null;
	@:optional public var savedEnvmap: Image = null;
	@:optional public var emptyEnvmap: Image = null;
	@:optional public var previewEnvmap: Image = null;
	@:optional public var envmapLoaded = false;
	@:optional public var showEnvmap = false;
	@:optional public var showEnvmapHandle = new Handle({ selected: false });
	@:optional public var showEnvmapBlur = false;
	@:optional public var showEnvmapBlurHandle = new Handle({ selected: false });
	@:optional public var envmapAngle = 0.0;
	@:optional public var lightAngle = 0.0;
	@:optional public var cullBackfaces = true;
	@:optional public var textureFilter = true;

	@:optional public var formatType = FormatPng;
	@:optional public var formatQuality = 100.0;
	@:optional public var layersDestination = DestinationDisk;
	@:optional public var splitBy = SplitObject;
	@:optional public var parseTransform = true;
	@:optional public var parseVCols = false;

	@:optional public var selectTime = 0.0;
	#if (kha_direct3d12 || kha_vulkan)
	@:optional public var pathTraceMode = TraceCore;
	#end
	#if (kha_direct3d12 || kha_vulkan)
	@:optional public var viewportMode = ViewPathTrace;
	#else
	@:optional public var viewportMode = ViewLit;
	#end
	#if (krom_android || krom_ios || arm_vr)
	@:optional public var renderMode = RenderForward;
	#else
	@:optional public var renderMode = RenderDeferred;
	#end

	@:optional public var viewportShader: NodeShader->String = null;
	@:optional public var hscaleWasChanged = false;
	@:optional public var exportMeshFormat = FormatObj;
	@:optional public var exportMeshIndex = 0;
	@:optional public var packAssetsOnExport = true;

	@:optional public var paintVec = new Vec4();
	@:optional public var lastPaintX = -1.0;
	@:optional public var lastPaintY = -1.0;
	@:optional public var foregroundEvent = false;
	@:optional public var painted = 0;
	@:optional public var brushTime = 0.0;
	@:optional public var cloneStartX = -1.0;
	@:optional public var cloneStartY = -1.0;
	@:optional public var cloneDeltaX = 0.0;
	@:optional public var cloneDeltaY = 0.0;

	@:optional public var showCompass = true;
	@:optional public var projectType = ModelRoundedCube;
	@:optional public var projectAspectRatio = 0; // 1:1, 2:1, 1:2
	@:optional public var projectObjects: Array<MeshObject>;

	@:optional public var lastPaintVecX = -1.0;
	@:optional public var lastPaintVecY = -1.0;
	@:optional public var prevPaintVecX = -1.0;
	@:optional public var prevPaintVecY = -1.0;
	@:optional public var frame = 0;
	@:optional public var paint2dView = false;

	@:optional public var lockStartedX = -1.0;
	@:optional public var lockStartedY = -1.0;
	@:optional public var brushLocked = false;
	@:optional public var brushCanLock = false;
	@:optional public var brushCanUnlock = false;
	@:optional public var cameraType = CameraPerspective;
	@:optional public var camHandle = new Handle();
	@:optional public var fovHandle: Handle = null;
	@:optional public var undoHandle: Handle = null;
	@:optional public var hssao: Handle = null;
	@:optional public var hssr: Handle = null;
	@:optional public var hbloom: Handle = null;
	@:optional public var hsupersample: Handle = null;
	@:optional public var hvxao: Handle = null;
	@:optional public var vxaoExt = 1.0;
	@:optional public var vxaoOffset = 1.5;
	@:optional public var vxaoAperture = 1.2;
	@:optional public var textureExportPath = "";
	@:optional public var lastStatusPosition = 0;
	@:optional public var cameraControls = ControlsOrbit;
	@:optional public var penPaintingOnly = false; // Reject painting with finger when using pen
}
