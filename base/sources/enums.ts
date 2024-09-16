
enum dilate_type_t {
	INSTANT,
	DELAYED,
}

enum bake_type_t {
	INIT = -1,
	AO = 0,
	CURVATURE = 1,
	NORMAL = 2,
	NORMAL_OBJECT = 3,
	HEIGHT = 4,
	DERIVATIVE = 5,
	POSITION = 6,
	TEXCOORD = 7,
	MATERIALID = 8,
	OBJECTID = 9,
	VERTEX_COLOR = 10,
	LIGHTMAP = 11,
	BENT_NORMAL = 12,
	THICKNESS = 13,
}

enum split_type_t {
	OBJECT = 0,
	GROUP = 1,
	MATERIAL = 2,
	UDIM = 3,
}

enum bake_axis_t {
	XYZ = 0,
	X = 1,
	Y = 2,
	Z = 3,
	MX = 4,
	MY = 5,
	MZ = 6,
}

enum bake_up_axis_t {
	Z = 0,
	Y = 1,
	X = 2,
}

enum viewport_mode_t {
	MINUS_ONE = -1,
	LIT = 0,
	BASE_COLOR = 1,
	NORMAL_MAP = 2,
	OCCLUSION = 3,
	ROUGHNESS = 4,
	METALLIC = 5,
	OPACITY = 6,
	HEIGHT = 7,
	EMISSION = 8,
	SUBSURFACE = 9,
	TEXCOORD = 10,
	OBJECT_NORMAL = 11,
	MATERIAL_ID = 12,
	OBJECT_ID = 13,
	MASK = 14,
	PATH_TRACE = 15,
}

enum channel_type_t {
	BASE_COLOR = 0,
	OCCLUSION = 1,
	ROUGHNESS = 2,
	METALLIC = 3,
	NORMAL_MAP = 4,
	HEIGHT = 5,
}

enum render_mode_t {
	DEFERRED = 0,
	FORWARD = 1,
	PATH_TRACE = 2,
}

enum export_mode_t {
	VISIBLE = 0,
	SELECTED = 1,
	PER_OBJECT = 2,
	PER_UDIM_TILE = 3,
}

enum export_destination_t {
	DISK = 0,
	PACKED = 1,
}

enum path_trace_mode_t {
	CORE = 0,
	FULL = 1,
}

enum fill_type_t {
	OBJECT = 0,
	FACE = 1,
	ANGLE = 2,
	UV_ISLAND = 3,
}

enum uv_type_t {
	UVMAP = 0,
	TRIPLANAR = 1,
	PROJECT = 2,
}

enum picker_mask_t {
	NONE = 0,
	MATERIAL = 1,
}

enum blend_type_t {
	MIX = 0,
	DARKEN = 1,
	MULTIPLY = 2,
	BURN = 3,
	LIGHTEN = 4,
	SCREEN = 5,
	DODGE = 6,
	ADD = 7,
	OVERLAY = 8,
	SOFT_LIGHT = 9,
	LINEAR_LIGHT = 10,
	DIFFERENCE = 11,
	SUBTRACT = 12,
	DIVIDE = 13,
	HUE = 14,
	SATURATION = 15,
	COLOR = 16,
	VALUE = 17,
}

enum camera_controls_t {
	ORBIT = 0,
	ROTATE = 1,
	FLY = 2,
}

enum camera_type_t {
	PERSPECTIVE = 0,
	ORTHOGRAPHIC = 1,
}

enum texture_bits_t {
	BITS8 = 0,
	BITS16 = 1,
	BITS32 = 2,
}

enum texture_ldr_format_t {
	PNG = 0,
	JPG = 1,
}

enum texture_hdr_format_t {
	EXR = 0,
}

enum mesh_format_t {
	OBJ = 0,
	ARM = 1,
}

enum menu_category_t {
	FILE = 0,
	EDIT = 1,
	VIEWPORT = 2,
	MODE = 3,
	CAMERA = 4,
	HELP = 5,
}

enum canvas_type_t {
	MATERIAL = 0,
	BRUSH = 1,
}

enum view_2d_type_t {
	ASSET = 0,
	NODE = 1,
	FONT = 2,
	LAYER = 3,
}

enum view_2d_layer_mode_t {
	VISIBLE = 0,
	SELECTED = 1,
}

enum border_side_t {
	LEFT = 0,
	RIGHT = 1,
	TOP = 2,
	BOTTOm = 3,
}

enum paint_tex_t {
	BASE = 0,
	NORMAL = 1,
	OCCLUSION = 2,
	ROUGHNESS = 3,
	METALLIC = 4,
	OPACITY = 5,
	HEIGHT = 6,
}

enum project_model_t {
	ROUNDED_CUBE = 0,
	SPHERE = 1,
	TESSELLATED_PLANE = 2,
	CUSTOM = 3,
}

enum zoom_direction_t {
	VERTICAL = 0,
	VERTICAL_INVERTED = 1,
	HORIZONTAL = 2,
	HORIZONTAL_INVERTED = 3,
	VERTICAL_HORIZONTAL = 4,
	VERTICAL_HORIZONTAL_INVERTED = 5,
}

enum layer_slot_type_t {
	LAYER = 0,
	MASK = 1,
	GROUP = 2,
}

enum space_type_t {
	SPACE3D = 0,
	SPACE2D = 1,
}

enum workspace_tool_t {
	BRUSH = 0,
	ERASER = 1,
	FILL = 2,
	DECAL = 3,
	TEXT = 4,
	CLONE = 5,
	BLUR = 6,
	SMUDGE = 7,
	PARTICLE = 8,
	COLORID = 9,
	PICKER = 10,
	BAKE = 11,
	GIZMO = 12,
	MATERIAL = 13,
}

enum area_type_t {
	MINUS_ONE = -1,
	VIEWPORT = 0,
	VIEW2D = 1,
	LAYERS = 2,
	MATERIALS = 3,
	NODES = 4,
	BROWSER = 5,
}

enum tab_area_t {
	SIDEBAR0 = 0,
	SIDEBAR1 = 1,
	STATUS = 2,
}

enum texture_res_t {
	RES128 = 0,
	RES256 = 1,
	RES512 = 2,
	RES1024 = 3,
	RES2048 = 4,
	RES4096 = 5,
	RES8192 = 6,
	RES16384 = 7,
}

enum layout_size_t {
	SIDEBAR_W = 0,
	SIDEBAR_H0 = 1,
	SIDEBAR_H1 = 2,
	NODES_W = 3,
	NODES_H = 4,
	STATUS_H = 5,
	HEADER = 6, // 0 - hidden, 1 - visible
}
