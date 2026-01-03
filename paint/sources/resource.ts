
let resource_bundled: map_t<string, gpu_texture_t> = map_create();

function resource_load(names: string[]) {
	for (let i: i32 = 0; i < names.length; ++i) {
		let s: string            = names[i];
		let image: gpu_texture_t = data_get_image(s);
		map_set(resource_bundled, s, image);
	}
}

function resource_get(name: string): gpu_texture_t {
	return map_get(resource_bundled, name);
}

function resource_tile50(img: gpu_texture_t, i: i32): rect_t {
	let x: i32 = i % 12;
	let y: i32 = i / 12;
	let size: i32 = config_raw.window_scale > 1 ? 100 : 50;
	let r: rect_t = {x : x * size, y : y * size, w : size, h : size};
	return r;
}

function resource_tile18(img: gpu_texture_t, i: i32): rect_t {
	let size: i32 = config_raw.window_scale > 1 ? 36 : 18;
	let r: rect_t = {x : i * size, y : img.height - size, w : size, h : size};
	return r;
}

type rect_t = {
	x?: i32;
	y?: i32;
	w?: i32;
	h?: i32;
};

enum icon18_t {
	EYE_ON  = 0,
	EYE_OFF = 1,
}

enum icon_t {
	BRUSH    = 0,
	ERASER   = 1,
	FILL     = 2,
	DECAL    = 3,
	TEXT     = 4,
	CLONE    = 5,
	BLUR     = 6,
	SMUDGE   = 7,
	PARTICLE = 8,
	COLOR_ID = 9,
	PICKER   = 10,
	BAKE     = 11,

	DROP             = 12,
	MATERIAL_PREVIEW = 13,
	FOLDER_FULL      = 14,
	FILE             = 15,
	CHECKER          = 16,
	BRUSH_PREVIEW    = 17,
	TEXT_PREVIEW     = 18,
	PROPERTIES       = 19,
	FOLDER_OPEN      = 20,
	EMPRT            = 21,
	GIZMO            = 22,
	MATERIAL         = 23,

	MENU     = 24,
	FILE_NEW = 25,
	FOLDER   = 26,
	SAVE     = 27,
	IMPORT   = 28,
	EXPORT   = 29,
	UNDO     = 30,
	REDO     = 31,
	VIEWPORT = 32,
	MODE     = 33,
	CAMERA   = 34,
	HELP     = 35,

	PROJECTS      = 36,
	DELETE        = 37,
	SEARCH        = 38,
	HOME          = 39,
	CLOSE         = 40,
	WRENCH        = 41,
	HEART         = 42,
	PLUS          = 43,
	CHEVRON_LEFT  = 44,
	CHEVRON_RIGHT = 45,
	ARROW_LEFT    = 46,
	ARROW_RIGHT   = 47,

	ARROW_UP   = 48,
	ARROW_DOWN = 49,
	DOWN       = 50,
	UP         = 51,
	CHECK      = 52,
	REFRESH    = 53,
	MINUS      = 54,
	NEW        = 55,
	CLEAR      = 56,
	RESIZE     = 57,
	FULLSCREEN = 58,
	DUPLICATE  = 59,

	CHEVRON_UP   = 60,
	CHEVRON_DOWN = 61,
	LEFT         = 62,
	RIGHT        = 63,
	MOVE         = 64,
	FILE_OPEN    = 65,
	FOLDER_NEW   = 66,
	DOWNLOADING  = 67,
	PUBLISH      = 68,
	ZOOM_OUT     = 69,
	ZOOM_IN      = 70,
	DRAFT        = 71,

	STAR      = 72,
	CUBE      = 73,
	TWOD      = 74,
	THREED    = 75,
	LAYER     = 76,
	LAYER_NEW = 77,
	WINDOW    = 78,
	BOOKMARK  = 79,
	FLAG      = 80,
	PIN       = 81,
	LABEL     = 82,
	ACCOUNT   = 83,

	ARROW_UP_LEFT = 84,
	ATTACHMENT    = 85,
	BLOCK         = 86,
	CAPTURE       = 87,
	CHAT          = 88,
	HORSE         = 89,
	CLOUD         = 90,
	PICKER2       = 91,
	COPY          = 92,
	CUT           = 93,
	PASTE         = 94,
	CROP          = 95,

	EDIT      = 96,
	EXIT      = 97,
	FILTER    = 98,
	PLUGIN    = 99,
	EMPTY2    = 100,
	FILE_SAVE = 101,
	QUESTION  = 102,
	HISTORY   = 103,
	IMAGE     = 104,
	INFO      = 105,
	KEY       = 106,
	WEB       = 107,

	LAYERS  = 108,
	LINK    = 109,
	LOCK    = 110,
	MAIL    = 111,
	MASK    = 112,
	DISPLAY = 113,
	MOVIE   = 114,
	MUSIC   = 115,
	NEW2    = 116,
	OPACITY = 117,
	HAND    = 118,
	PAUSE   = 119,

	PHOTO     = 120,
	PLAY      = 121,
	REPLAY    = 122,
	SAVE_AS   = 123,
	TIMER     = 124,
	BAR       = 125,
	RULER     = 126,
	STOP      = 127,
	STORAGE   = 128,
	TERMINAL  = 129,
	TRANSLATE = 130,
	UPLOAD    = 131,
}
