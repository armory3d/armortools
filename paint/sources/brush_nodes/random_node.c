
#include "../global.h"

i32 random_node_a;
i32 random_node_b;
i32 random_node_c;
i32 random_node_d = -1;

void random_node_set_seed(i32 seed);

i32 random_node_get_int() {
	if (random_node_d == -1) {
		random_node_set_seed(352124);
	}

	// Courtesy of https://github.com/Kode/Kha/blob/main/Sources/kha/math/Random.hx
	i32 t              = (random_node_a + random_node_b | 0) + random_node_d | 0;
	random_node_d      = random_node_d + 1 | 0;
	u32 urandom_node_b = random_node_b;
	random_node_a      = random_node_b ^ urandom_node_b >> 9;
	random_node_b      = random_node_c + (random_node_c << 3) | 0;
	u32 urandom_node_c = random_node_c;
	random_node_c      = random_node_c << 21 | urandom_node_c >> 11;
	random_node_c      = random_node_c + t | 0;
	return t & 0x7fffffff;
}

void random_node_set_seed(i32 seed) {
	random_node_d = seed;
	random_node_a = 0x36aef51a;
	random_node_b = 0x21d4b3eb;
	random_node_c = 0xf2517abf;
	// Immediately skip a few possibly poor results the easy way
	for (i32 i = 0; i < 15; ++i) {
		random_node_get_int();
	}
}

i32 random_node_get_seed() {
	return random_node_d;
}

f32 random_node_get_float() {
	return random_node_get_int() / (float)0x7fffffff;
}

logic_node_value_t *random_node_get(random_node_t *self, i32 from) {
	f32                 min = logic_node_input_get(self->base->inputs->buffer[0])->_f32;
	f32                 max = logic_node_input_get(self->base->inputs->buffer[1])->_f32;
	logic_node_value_t *v   = GC_ALLOC_INIT(logic_node_value_t, {._f32 = min + random_node_get_float() * (max - min)});
	return v;
}

random_node_t *random_node_create(ui_node_t *raw, f32_array_t *args) {
	random_node_t *n = GC_ALLOC_INIT(random_node_t, {0});
	n->base          = logic_node_create(n);
	n->base->get     = random_node_get;
	return n;
}
