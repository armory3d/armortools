#include "texture.h"

bool kinc_g5_texture_unit_equals(kinc_g5_texture_unit_t *unit1, kinc_g5_texture_unit_t *unit2) {
	for (int i = 0; i < KINC_G5_SHADER_TYPE_COUNT; ++i) {
		if (unit1->stages[i] != unit2->stages[i]) {
			return false;
		}
	}
	return true;
}
