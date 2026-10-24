// Standalone: cc -std=c11 normal-bake-core.c -lm -o normal-bake-core
#include "../sources/util/bake_normal_core.h"
#include <assert.h>
#include <stdio.h>

static bn_low     low     = {.face = {.p = {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}}, .n = {{0, 0, 1}, {0, 0, 1}, {0, 0, 1}}},
                             .uv   = {{0.25f, 0.25f}, {0.75f, 0.25f}, {0.25f, 0.75f}}};
static bn_options options = {.size = 64, .padding = 0, .samples = 1, .outer = 1, .inner = 1, .backfaces = true};

static bn_face surface(float height, bn_v3 normal) {
	bn_face f = {.p = {{-2, -2, height}, {4, -2, height}, {-2, 4, height}}};
	for (int k = 0; k < 3; k++)
		f.n[k] = bn_norm(normal);
	return f;
}

static bool cancel(void *user, int progress) {
	return progress < *(int *)user;
}

static void near_byte(int actual, int expected) {
	assert(abs(actual - expected) <= 1);
}

int main(void) {
	bn_result r    = {0};
	bn_face   high = surface(0.1f, (bn_v3){-0.2f, -0.1f, 1});
	assert(bn_bake(&low, 1, &high, 1, options, &r, NULL, NULL));
	assert(r.covered > 400 && r.missed == 0);
	int   sample = (24 * 64 + 24) * 4;
	bn_v3 n      = bn_norm((bn_v3){-0.2f, 0.1f, 1});
	near_byte(r.rgba[sample], bn_encode(n.x));
	near_byte(r.rgba[sample + 1], bn_encode(n.y));
	near_byte(r.rgba[sample + 2], bn_encode(n.z));
	int green = r.rgba[sample + 1];
	bn_result_free(&r);
	options.directx = true;
	assert(bn_bake(&low, 1, &high, 1, options, &r, NULL, NULL));
	near_byte(r.rgba[sample + 1], 255 - green);
	bn_result_free(&r);
	options.directx = false;

	// A skewed chart must decode through the renderer's shared-scale cotangent frame.
	bn_low skewed      = low;
	skewed.face.p[2].x = 1;
	assert(bn_bake(&skewed, 1, &high, 1, options, &r, NULL, NULL));
	bn_v3 skew_normal = bn_norm((bn_v3){-0.2f * sqrtf(2), 0.3f * sqrtf(2), 1});
	near_byte(r.rgba[sample], bn_encode(skew_normal.x));
	near_byte(r.rgba[sample + 1], bn_encode(skew_normal.y));
	near_byte(r.rgba[sample + 2], bn_encode(skew_normal.z));
	bn_result_free(&r);

	// Mirrored U flips the tangent component, not the geometric projection.
	bn_low mirrored = low;
	for (int k = 0; k < 3; k++)
		mirrored.uv[k][0] = 1 - low.uv[k][0];
	assert(bn_bake(&mirrored, 1, &high, 1, options, &r, NULL, NULL));
	near_byte(r.rgba[(24 * 64 + 40) * 4], bn_encode(-n.x));
	bn_result_free(&r);

	options.outer = 0.05f;
	assert(bn_bake(&low, 1, &high, 1, options, &r, NULL, NULL));
	assert(r.covered == r.missed);
	bn_result_free(&r);
	high          = surface(-0.2f, (bn_v3){0, 0, 1});
	options.inner = 0.3f;
	assert(bn_bake(&low, 1, &high, 1, options, &r, NULL, NULL));
	assert(r.missed == 0);
	bn_result_free(&r);
	options.inner = 0.1f;
	assert(bn_bake(&low, 1, &high, 1, options, &r, NULL, NULL));
	assert(r.covered == r.missed);
	bn_result_free(&r);

	options.outer = options.inner = 1;
	high                          = surface(0.1f, (bn_v3){0, 0, -1});
	bn_v3 p                       = high.p[1];
	high.p[1]                     = high.p[2];
	high.p[2]                     = p;
	options.backfaces             = false;
	assert(bn_bake(&low, 1, &high, 1, options, &r, NULL, NULL));
	assert(r.covered == r.missed);
	bn_result_free(&r);
	options.backfaces = true;
	assert(bn_bake(&low, 1, &high, 1, options, &r, NULL, NULL));
	assert(r.missed == 0);
	bn_result_free(&r);

	// Closest hit must win even when the hierarchy partitions triangles.
	bn_face many[65];
	for (int i = 0; i < 65; i++)
		many[i] = surface(-0.5f, (bn_v3){0, 0, 1});
	many[31] = surface(0.5f, (bn_v3){0.3f, 0, 1});
	assert(bn_bake(&low, 1, many, 65, options, &r, NULL, NULL));
	assert(r.rgba[sample] > 160);
	bn_result_free(&r);

	options.padding = 3;
	options.samples = 4;
	high            = surface(0.1f, (bn_v3){0.3f, 0, 1});
	assert(bn_bake(&low, 1, &high, 1, options, &r, NULL, NULL));
	assert(r.mask[20 * 64 + 14] == 0 && r.rgba[(20 * 64 + 14) * 4] > 160);
	assert(r.rgba[(20 * 64 + 11) * 4] == 128);
	bn_result_free(&r);
	for (int stop = 0; stop <= 100; stop += 5) {
		assert(!bn_bake(&low, 1, &high, 1, options, &r, cancel, &stop));
		assert(!r.rgba && !r.mask);
	}
	bn_low degenerate = low;
	memset(degenerate.uv, 0, sizeof(degenerate.uv));
	assert(!bn_bake(&degenerate, 1, &high, 1, options, &r, NULL, NULL));
	high.p[0].x = NAN;
	assert(!bn_bake(&low, 1, &high, 1, options, &r, NULL, NULL));

	options    = (bn_options){.size = 32, .samples = 1, .outer = .2f, .inner = 1, .backfaces = true, .type = BN_HEIGHT, .rays = 16, .radius = 1, .strength = 4};
	high       = surface(.1f, (bn_v3){0, 0, 1});
	int center = (12 * 32 + 12) * 4;
	assert(bn_bake(&low, 1, &high, 1, options, &r, NULL, NULL));
	near_byte(r.rgba[center], 140);
	bn_result_free(&r);
	high = surface(-.2f, (bn_v3){0, 0, 1});
	assert(bn_bake(&low, 1, &high, 1, options, &r, NULL, NULL));
	near_byte(r.rgba[center], 102);
	bn_result_free(&r);
	options.type = BN_AO;
	high         = surface(.1f, (bn_v3){0, 0, 1});
	assert(bn_bake(&low, 1, &high, 1, options, &r, NULL, NULL));
	near_byte(r.rgba[center], 255);
	bn_result_free(&r);
	bn_face occluded[2] = {high, surface(.5f, (bn_v3){0, 0, -1})};
	assert(bn_bake(&low, 1, occluded, 2, options, &r, NULL, NULL));
	assert(r.rgba[center] < 180);
	bn_result_free(&r);
	options.type   = BN_CURVATURE;
	options.radius = .1f;
	assert(bn_bake(&low, 1, &high, 1, options, &r, NULL, NULL));
	near_byte(r.rgba[center], 128);
	bn_result_free(&r);
	// Analytic convex/concave normal fields on a dense curved surface.
	bn_face curved[2048];
	for (int sign = -1; sign <= 1; sign += 2) {
		int used = 0;
		for (int y = 0; y < 32; y++)
			for (int x = 0; x < 32; x++) {
				float x0 = -2 + x * .125f, y0 = -2 + y * .125f;
				bn_v3 p[4]       = {{x0, y0, 0}, {x0 + .125f, y0, 0}, {x0 + .125f, y0 + .125f, 0}, {x0, y0 + .125f, 0}};
				int   indices[6] = {0, 1, 2, 0, 2, 3};
				for (int t = 0; t < 2; t++) {
					bn_face *f = &curved[used++];
					for (int k = 0; k < 3; k++) {
						bn_v3 v = p[indices[t * 3 + k]];
						v.z     = sign * .2f * (v.x * v.x + v.y * v.y);
						f->p[k] = v;
						f->n[k] = bn_norm((bn_v3){-sign * .4f * v.x, -sign * .4f * v.y, 1});
					}
				}
			}
		options.outer = options.inner = 1;
		assert(bn_bake(&low, 1, curved, 2048, options, &r, NULL, NULL));
		assert(sign < 0 ? r.rgba[center] > 130 : r.rgba[center] < 125);
		bn_result_free(&r);
	}
	assert(bn_risk(false, 0, 1, options) == 3);
	assert(bn_risk(true, .95f, 1, options) == 1);
	assert(bn_risk(true, 0, .2f, options) == 2);
	assert(bn_risk(true, 0, 1, options) == 0);
	bn_tree tree;
	high = surface(2, (bn_v3){0, 0, 1});
	assert(bn_tree_init(&tree, &high, 1, NULL, NULL));
	uint8_t risk = 0;
	assert(bn_analyze(&low, 1, &tree, options, &risk, NULL, NULL));
	assert(risk == 3);
	free(tree.order);
	free(tree.nodes);
	puts("PASS: projection, range, mirrored UVs, DirectX, backfaces, BVH, padding and cancellation");
	puts("PASS: signed height, ambient occlusion, convex/concave curvature and projection diagnostics");
	return 0;
}
