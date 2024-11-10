#include "texture.h"

/*
Graphics5::Texture::Texture(Kore::Reader& reader, const char* format, bool readable) : Image(reader, format, readable) {
    _init(format, readable);
}

Graphics5::Texture::Texture(const char* filename, bool readable) {
    FileReader reader(filename);
    Image::init(reader, filename, readable);
    _init(filename, readable);
}

Graphics5::Texture::Texture(void* data, int size, const char* format, bool readable) {
    BufferReader reader(data, size);
    Image::init(reader, format, readable);
    _init(format, readable);
}

Graphics5::Texture::Texture(void* data, int width, int height, int format, bool readable) : Image(data, width, height, Image::Format(format), readable) {
    _init("", readable);
}
*/

bool kinc_g5_texture_unit_equals(kinc_g5_texture_unit_t *unit1, kinc_g5_texture_unit_t *unit2) {
	for (int i = 0; i < KINC_G5_SHADER_TYPE_COUNT; ++i) {
		if (unit1->stages[i] != unit2->stages[i]) {
			return false;
		}
	}
	return true;
}
