#include "iron_path.h"

#include <string.h>

#include "iron_array.h"
#include "iron_file.h"
#include "iron_map.h"
#include "iron_string.h"
#include "iron_gc.h"

#ifdef IRON_LINUX
bool path_is_protected_linux = false;
#endif

static char_ptr_array_t *_path_mesh_formats     = NULL;
static char_ptr_array_t *_path_texture_formats  = NULL;
static char_ptr_array_t *_path_base_color_ext   = NULL;
static char_ptr_array_t *_path_opacity_ext      = NULL;
static char_ptr_array_t *_path_normal_map_ext   = NULL;
static char_ptr_array_t *_path_occlusion_ext    = NULL;
static char_ptr_array_t *_path_roughness_ext    = NULL;
static char_ptr_array_t *_path_metallic_ext     = NULL;
static char_ptr_array_t *_path_displacement_ext = NULL;

char_ptr_array_t *path_mesh_formats(void) {
	if (_path_mesh_formats == NULL) {
		_path_mesh_formats = char_ptr_array_create(0);
        gc_root(_path_mesh_formats);
		char_ptr_array_push(_path_mesh_formats, "obj");
		char_ptr_array_push(_path_mesh_formats, "blend");
	}
	return _path_mesh_formats;
}

char_ptr_array_t *path_texture_formats(void) {
	if (_path_texture_formats == NULL) {
		_path_texture_formats = char_ptr_array_create(0);
        gc_root(_path_texture_formats);
		char_ptr_array_push(_path_texture_formats, "jpg");
		char_ptr_array_push(_path_texture_formats, "jpeg");
		char_ptr_array_push(_path_texture_formats, "png");
		char_ptr_array_push(_path_texture_formats, "tga");
		char_ptr_array_push(_path_texture_formats, "bmp");
		char_ptr_array_push(_path_texture_formats, "psd");
		char_ptr_array_push(_path_texture_formats, "gif");
		char_ptr_array_push(_path_texture_formats, "hdr");
		char_ptr_array_push(_path_texture_formats, "k");
	}
	return _path_texture_formats;
}

char_ptr_array_t *path_base_color_ext(void) {
	if (_path_base_color_ext == NULL) {
		_path_base_color_ext = char_ptr_array_create(0);
        gc_root(_path_base_color_ext);
		char_ptr_array_push(_path_base_color_ext, "albedo");
		char_ptr_array_push(_path_base_color_ext, "alb");
		char_ptr_array_push(_path_base_color_ext, "basecol");
		char_ptr_array_push(_path_base_color_ext, "basecolor");
		char_ptr_array_push(_path_base_color_ext, "diffuse");
		char_ptr_array_push(_path_base_color_ext, "diff");
		char_ptr_array_push(_path_base_color_ext, "base");
		char_ptr_array_push(_path_base_color_ext, "bc");
		char_ptr_array_push(_path_base_color_ext, "d");
		char_ptr_array_push(_path_base_color_ext, "color");
		char_ptr_array_push(_path_base_color_ext, "col");
	}
	return _path_base_color_ext;
}

char_ptr_array_t *path_opacity_ext(void) {
	if (_path_opacity_ext == NULL) {
		_path_opacity_ext = char_ptr_array_create(0);
        gc_root(_path_opacity_ext);
		char_ptr_array_push(_path_opacity_ext, "opac");
		char_ptr_array_push(_path_opacity_ext, "opacity");
		char_ptr_array_push(_path_opacity_ext, "alpha");
	}
	return _path_opacity_ext;
}

char_ptr_array_t *path_normal_map_ext(void) {
	if (_path_normal_map_ext == NULL) {
		_path_normal_map_ext = char_ptr_array_create(0);
        gc_root(_path_normal_map_ext);
		char_ptr_array_push(_path_normal_map_ext, "normal");
		char_ptr_array_push(_path_normal_map_ext, "normals");
		char_ptr_array_push(_path_normal_map_ext, "nor");
		char_ptr_array_push(_path_normal_map_ext, "n");
		char_ptr_array_push(_path_normal_map_ext, "nrm");
		char_ptr_array_push(_path_normal_map_ext, "normalgl");
	}
	return _path_normal_map_ext;
}

char_ptr_array_t *path_occlusion_ext(void) {
	if (_path_occlusion_ext == NULL) {
		_path_occlusion_ext = char_ptr_array_create(0);
        gc_root(_path_occlusion_ext);
		char_ptr_array_push(_path_occlusion_ext, "ao");
		char_ptr_array_push(_path_occlusion_ext, "occlusion");
		char_ptr_array_push(_path_occlusion_ext, "ambientOcclusion");
		char_ptr_array_push(_path_occlusion_ext, "o");
		char_ptr_array_push(_path_occlusion_ext, "occ");
	}
	return _path_occlusion_ext;
}

char_ptr_array_t *path_roughness_ext(void) {
	if (_path_roughness_ext == NULL) {
		_path_roughness_ext = char_ptr_array_create(0);
        gc_root(_path_roughness_ext);
		char_ptr_array_push(_path_roughness_ext, "roughness");
		char_ptr_array_push(_path_roughness_ext, "rough");
		char_ptr_array_push(_path_roughness_ext, "r");
		char_ptr_array_push(_path_roughness_ext, "rgh");
	}
	return _path_roughness_ext;
}

char_ptr_array_t *path_metallic_ext(void) {
	if (_path_metallic_ext == NULL) {
		_path_metallic_ext = char_ptr_array_create(0);
        gc_root(_path_metallic_ext);
		char_ptr_array_push(_path_metallic_ext, "metallic");
		char_ptr_array_push(_path_metallic_ext, "metal");
		char_ptr_array_push(_path_metallic_ext, "metalness");
		char_ptr_array_push(_path_metallic_ext, "m");
		char_ptr_array_push(_path_metallic_ext, "met");
	}
	return _path_metallic_ext;
}

char_ptr_array_t *path_displacement_ext(void) {
	if (_path_displacement_ext == NULL) {
		_path_displacement_ext = char_ptr_array_create(0);
        gc_root(_path_displacement_ext);
		char_ptr_array_push(_path_displacement_ext, "displacement");
		char_ptr_array_push(_path_displacement_ext, "height");
		char_ptr_array_push(_path_displacement_ext, "h");
		char_ptr_array_push(_path_displacement_ext, "disp");
	}
	return _path_displacement_ext;
}

static char *data_path(void) {
#ifdef IRON_ANDROID
	return "data" PATH_SEP;
#else
	return "." PATH_SEP "data" PATH_SEP;
#endif
}

char *path_data(void) {
	return string_join(iron_internal_files_location(), string_join(PATH_SEP, data_path()));
}

char *path_to_relative(char *from, char *to) {
	any_array_t *a = string_split(from, PATH_SEP);
	any_array_t *b = string_split(to, PATH_SEP);
	while (a->length > 0 && b->length > 0) {
		char *a0 = a->buffer[0];
		char *b0 = b->buffer[0];
		if (string_equals(a0, b0)) {
			array_shift(a);
			array_shift(b);
		}
		else {
			break;
		}
	}
	char *p = "";
	for (uint32_t i = 0; i < a->length; ++i) {
		p = string_join(p, ".." PATH_SEP);
	}
	p = string_join(p, string_array_join(b, PATH_SEP));
	return p;
}

char *path_normalize(char *path) {
	size_t path_len = strlen(path);
	if (path_len > 0 && ends_with(path, PATH_SEP)) {
		path = substring(path, 0, path_len - 1);
	}
	any_array_t *ar = string_split(path, PATH_SEP);
	uint32_t     i  = 0;
	while (i < ar->length) {
		if (i > 0) {
			char *ar_i   = ar->buffer[i];
			char *ar_i_1 = ar->buffer[i - 1];
			if (string_equals(ar_i, "..") && !string_equals(ar_i_1, "..")) {
				array_splice(ar, i - 1, 2);
				i--;
			}
			else {
				i++;
			}
		}
		else {
			i++;
		}
	}
	return string_array_join(ar, PATH_SEP);
}

char *path_base_dir(char *path) {
	int32_t last = string_last_index_of(path, PATH_SEP);
	return substring(path, 0, last + 1);
}

char *path_base_name(char *path) {
	int32_t last_sep = string_last_index_of(path, PATH_SEP);
	int32_t last_dot = string_last_index_of(path, ".");
	return substring(path, last_sep + 1, last_dot);
}

bool path_is_mesh(char *path) {
	char             *p       = to_lower_case(path);
	char_ptr_array_t *formats = path_mesh_formats();
	for (uint32_t i = 0; i < formats->length; ++i) {
		char *s   = formats->buffer[i];
		char *ext = string_join(".", s);
		if (ends_with(p, ext)) {
			return true;
		}
	}
	return false;
}

bool path_is_texture(char *path) {
	char             *p       = to_lower_case(path);
	char_ptr_array_t *formats = path_texture_formats();
	for (uint32_t i = 0; i < formats->length; ++i) {
		char *s   = formats->buffer[i];
		char *ext = string_join(".", s);
		if (ends_with(p, ext)) {
			return true;
		}
	}
	return false;
}

bool path_is_font(char *path) {
	char *p = to_lower_case(path);
	return ends_with(p, ".ttf") || ends_with(p, ".ttc") || ends_with(p, ".otf");
}

bool path_is_project(char *path) {
	char *p = to_lower_case(path);
	return ends_with(p, ".arm");
}

bool path_is_plugin(char *path) {
	char *p = to_lower_case(path);
	return ends_with(p, ".js");
}

bool path_is_json(char *path) {
	char *p = to_lower_case(path);
	return ends_with(p, ".json");
}

bool path_is_text(char *path) {
	char *p = to_lower_case(path);
	return ends_with(p, ".txt");
}

bool path_is_gimp_color_palette(char *path) {
	char *p = to_lower_case(path);
	return ends_with(p, ".gpl");
}

bool path_is_ext_format(char *path) {
	char *p = to_lower_case(path);
	return ends_with(p, ".fbx") || ends_with(p, ".gltf") || ends_with(p, ".glb") || ends_with(p, ".stl") || ends_with(p, ".svg");
}

bool path_is_known(char *path) {
	return path_is_mesh(path) || path_is_texture(path) || path_is_font(path) || path_is_project(path) || path_is_plugin(path) || path_is_text(path) ||
	       path_is_gimp_color_palette(path) || path_is_ext_format(path);
}

bool path_check_ext(char *p, char_ptr_array_t *exts) {
	p = string_replace_all(p, "-", "_");
	for (uint32_t i = 0; i < exts->length; ++i) {
		char *ext            = exts->buffer[i];
		char *ext_underscore = string_join("_", ext);
		if (ends_with(p, ext_underscore)) {
			return true;
		}
		char *ext_underscore_mid = string_join("_", string_join(ext, "_"));
		if (string_index_of(p, ext_underscore_mid) >= 0 && !ends_with(p, "_preview") && !ends_with(p, "_icon")) {
			return true;
		}
	}
	return false;
}

bool path_is_base_color_tex(char *p) {
	return path_check_ext(p, path_base_color_ext());
}

bool path_is_opacity_tex(char *p) {
	return path_check_ext(p, path_opacity_ext());
}

bool path_is_normal_map_tex(char *p) {
	return path_check_ext(p, path_normal_map_ext());
}

bool path_is_occlusion_tex(char *p) {
	return path_check_ext(p, path_occlusion_ext());
}

bool path_is_roughness_tex(char *p) {
	return path_check_ext(p, path_roughness_ext());
}

bool path_is_metallic_tex(char *p) {
	return path_check_ext(p, path_metallic_ext());
}

bool path_is_displacement_tex(char *p) {
	return path_check_ext(p, path_displacement_ext());
}

bool path_is_folder(char *p) {
	char        *p_slash = string_replace_all(p, "\\", "/");
	any_array_t *split   = string_split(p_slash, "/");
	char        *last    = array_pop(split);
	return string_index_of(last, ".") < 0;
}

bool path_is_protected(void) {
#ifdef IRON_WINDOWS
	return string_index_of(iron_internal_files_location(), "Program Files") >= 0;
#elif defined(IRON_LINUX)
	return path_is_protected_linux;
#elif defined(IRON_ANDROID)
	return true;
#elif defined(IRON_IOS)
	return true;
#else
	return false;
#endif
}

char *path_join(char *a, char *b) {
	char *path = a;
	if (!ends_with(path, PATH_SEP)) {
		path = string_join(path, PATH_SEP);
	}
	path = string_join(path, b);
	return path;
}
