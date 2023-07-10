#define _FULL
#ifdef _FULL
#define _EMISSION
#define _SUBSURFACE
#define _TRANSLUCENCY
#endif
#define _RENDER
// #define _ROULETTE
// #define _TRANSPARENCY

using namespace metal;
using namespace raytracing;

struct Vertex {
	uint posxy;
	uint poszw;
	uint nor;
	uint tex;
};

struct RayGenConstantBuffer {
	float4 eye; // xyz, frame
	float4x4 inv_vp;
	float4 params; // envstr, envangle, uvscale
};

struct RayPayload {
	float4 color; // rgb, frame
	float3 ray_origin;
	float3 ray_dir;
};

constant int SAMPLES = 2; // 64
#ifdef _TRANSLUCENCY
constant int DEPTH = 6;
#else
constant int DEPTH = 3; // Opaque hits
#endif
#ifdef _TRANSPARENCY
constant int DEPTH_TRANSPARENT = 16; // Transparent hits
#endif
#ifdef _ROULETTE
constant int rrStart = 2;
constant float rrProbability = 0.5; // Map to albedo
#endif

void generate_camera_ray(float2 screen_pos, thread float3 & ray_origin, thread float3 & ray_dir, float3 eye, float4x4 inv_vp) {
	screen_pos.y = -screen_pos.y;
	float4 world = inv_vp * float4(screen_pos, 0, 1);
	world.xyz /= world.w;
	ray_origin = eye;
	ray_dir = normalize(world.xyz - ray_origin);
}

float2 equirect(float3 normal, float angle) {
	const float PI = 3.1415926535;
	const float PI2 = PI * 2.0;
	float phi = acos(normal.z);
	float theta = atan2(-normal.y, normal.x) + PI + angle;
	return float2(theta / PI2, phi / PI);
}

float rand(int pixel_i, int pixel_j, int sampleIndex, int sampleDimension, int frame, texture2d<float, access::read> sobol, texture2d<float, access::read> scramble, texture2d<float, access::read> rank) {
	pixel_i += frame * 9;
	pixel_j += frame * 11;
	pixel_i = pixel_i & 127;
	pixel_j = pixel_j & 127;
	sampleIndex = sampleIndex & 255;
	sampleDimension = sampleDimension & 255;

	int i = sampleDimension + (pixel_i + pixel_j * 128) * 8;
	int rankedSampleIndex = sampleIndex ^ int(rank.read(uint2(i % 128, uint(i / 128)), 0).r * 255);

	i = sampleDimension + rankedSampleIndex * 256;
	int value = int(sobol.read(uint2(i % 256, uint(i / 256)), 0).r * 255);

	i = (sampleDimension % 8) + (pixel_i + pixel_j * 128) * 8;
	value = value ^ int(scramble.read(uint2(i % 128, uint(i / 128)), 0).r * 255);

	float v = (0.5f + value) / 256.0f;
	return v;
}

float3 cos_weighted_hemisphere_direction(uint2 tid, float3 n, uint sample, uint seed, int frame, texture2d<float, access::read> sobol, texture2d<float, access::read> scramble, texture2d<float, access::read> rank) {
	const float PI = 3.1415926535;
	const float PI2 = PI * 2.0;
	float f0 = rand(tid.x, tid.y, sample, seed, frame, sobol, scramble, rank);
	float f1 = rand(tid.x, tid.y, sample, seed + 1, frame, sobol, scramble, rank);
	float z = f0 * 2.0f - 1.0f;
	float a = f1 * PI2;
	float r = sqrt(1.0f - z * z);
	float x = r * cos(a);
	float y = r * sin(a);
	return normalize(n + float3(x, y, z));
}

float2 s16_to_f32(uint val) {
	int a = (int)(val << 16) >> 16;
	int b = (int)(val & 0xffff0000) >> 16;
	return float2(a, b) / 32767.0f;
}

float3 hit_world_position(ray ray, typename intersector<triangle_data, instancing>::result_type intersection) {
	return ray.origin + ray.direction * intersection.distance;
}

float3 hit_attribute(float3 vertexAttribute[3], float2 barycentrics) {
	return vertexAttribute[0] +
		barycentrics.x * (vertexAttribute[1] - vertexAttribute[0]) +
		barycentrics.y * (vertexAttribute[2] - vertexAttribute[0]);
}

float2 hit_attribute2d(float2 vertexAttribute[3], float2 barycentrics) {
	return vertexAttribute[0] +
		barycentrics.x * (vertexAttribute[1] - vertexAttribute[0]) +
		barycentrics.y * (vertexAttribute[2] - vertexAttribute[0]);
}

void create_basis(float3 normal, thread float3 & tangent, thread float3 & binormal) {
	float3 v1 = cross(normal, float3(0.0, 0.0, 1.0));
	float3 v2 = cross(normal, float3(0.0, 1.0, 0.0));
	tangent = length(v1) > length(v2) ? v1 : v2;
	binormal = cross(tangent, normal);
}

float3 surfaceAlbedo(const float3 baseColor, const float metalness) {
	return mix(baseColor, float3(0.0, 0.0, 0.0), metalness);
}

float3 surfaceSpecular(const float3 baseColor, const float metalness) {
	return mix(float3(0.04, 0.04, 0.04), baseColor, metalness);
}

float3 envBRDFApprox(float3 specular, float roughness, float dotNV) {
	const float4 c0 = float4(-1, -0.0275, -0.572, 0.022);
	const float4 c1 = float4(1, 0.0425, 1.04, -0.04);
	float4 r = roughness * c0 + c1;
	float a004 = min(r.x * r.x, exp2(-9.28 * dotNV)) * r.x + r.y;
	float2 ab = float2(-1.04, 1.04) * a004 + r.zw;
	return specular * ab.x + ab.y;
}

kernel void raytracingKernel(
	uint2 tid [[thread_position_in_grid]],
	constant RayGenConstantBuffer &constant_buffer [[buffer(0)]],
	texture2d<float, access::read_write> render_target [[texture(0)]],
	texture2d<float, access::read> mytexture0 [[texture(1)]],
	texture2d<float, access::read> mytexture1 [[texture(2)]],
	texture2d<float, access::read> mytexture2 [[texture(3)]],
	texture2d<float, access::read> mytexture_env [[texture(4)]],
	texture2d<float, access::read> mytexture_sobol [[texture(5)]],
	texture2d<float, access::read> mytexture_scramble [[texture(6)]],
	texture2d<float, access::read> mytexture_rank [[texture(7)]],
	instance_acceleration_structure scene [[buffer(1)]],
	device void *indices [[buffer(2)]],
	device void *vertices [[buffer(3)]]
) {
	uint seed = 0;
	float3 accum = float3(0, 0, 0);

	for (int j = 0; j < SAMPLES; ++j) {
		// AA
		float2 xy = float2(tid) + float2(0.5f, 0.5f);
		xy.x += rand(tid.x, tid.y, j, seed, constant_buffer.eye.w, mytexture_sobol, mytexture_scramble, mytexture_rank);
		seed += 1;
		xy.y += rand(tid.x, tid.y, j, seed, constant_buffer.eye.w, mytexture_sobol, mytexture_scramble, mytexture_rank);

		float2 screen_pos = xy / float2(render_target.get_width(), render_target.get_height()) * 2.0 - 1.0;
		ray ray;
		ray.min_distance = 0.0001;
		ray.max_distance = 10.0;
		generate_camera_ray(screen_pos, ray.origin, ray.direction, constant_buffer.eye.xyz, constant_buffer.inv_vp);

		RayPayload payload;
		payload.color = float4(1, 1, 1, j);

		#ifdef _TRANSPARENCY
		int transparentHits = 0;
		#endif

		for (int i = 0; i < DEPTH; ++i) {

			#ifdef _ROULETTE
			float rrFactor = 1.0;
			if (i >= rrStart) {
				float f = rand(tid.x, tid.y, j, seed, constant_buffer.eye.w, mytexture_sobol, mytexture_scramble, mytexture_rank);
				if (f <= rrProbability) {
					break;
				}
				rrFactor = 1.0 / (1.0 - rrProbability);
			}
			#endif

			// #ifdef _SUBSURFACE
			// TraceRay(scene, RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);
			// #else
			// TraceRay(scene, RAY_FLAG_FORCE_OPAQUE, ~0, 0, 1, 0, ray, payload);

			intersector<triangle_data, instancing> in;
			in.assume_geometry_type(geometry_type::triangle);
			in.force_opacity(forced_opacity::opaque);
			in.accept_any_intersection(false);
			// in.set_triangle_cull_mode(triangle_cull_mode::none);

			typename intersector<triangle_data, instancing>::result_type intersection;
			intersection = in.intersect(ray, scene);
			if (intersection.type == intersection_type::none) {
				#ifdef _EMISSION
				if (payload.color.a == -2.0) {
					return;
				}
				#endif

				float2 tex_coord = fract(equirect(ray.direction, constant_buffer.params.y));
				uint2 size = uint2(mytexture_env.get_width(), mytexture_env.get_height());
				float3 texenv = mytexture_env.read(uint2(tex_coord * float2(size)), 0).rgb * abs(constant_buffer.params.x);
				payload.color = float4(payload.color.rgb * texenv.rgb, -1);
			}
			else {
				device uint32_t *inda = (device uint32_t *)(indices);
				uint3 indices_sample = uint3(
					inda[intersection.primitive_id * 3],
					inda[intersection.primitive_id * 3 + 1],
					inda[intersection.primitive_id * 3 + 2]
				);

				device Vertex *verta = (device Vertex *)(vertices);
				float2 vertex_uvs[3] = {
					s16_to_f32(verta[indices_sample[0]].tex),
					s16_to_f32(verta[indices_sample[1]].tex),
					s16_to_f32(verta[indices_sample[2]].tex)
				};
				float2 barycentrics = intersection.triangle_barycentric_coord;
				float2 tex_coord = hit_attribute2d(vertex_uvs, barycentrics) * constant_buffer.params.z;

				uint2 size = uint2(mytexture0.get_width(), mytexture0.get_height());
				uint3 utex_coord = uint3(uint2((tex_coord - float2(uint2(tex_coord))) * float2(size)), 0);
				float4 texpaint0 = mytexture0.read(utex_coord.xy, utex_coord.z);

				#ifdef _TRANSPARENCY
				if (texpaint0.a <= 0.1) {
					payload.ray_dir = ray.direction;
					payload.ray_origin = hit_world_position(ray, intersection) + payload.ray_dir * 0.0001f;
					payload.color.a = -2;
					return;
				}
				#endif

				float3 vertex_normals[3] = {
					float3(s16_to_f32(verta[indices_sample[0]].nor), s16_to_f32(verta[indices_sample[0]].poszw).y),
					float3(s16_to_f32(verta[indices_sample[1]].nor), s16_to_f32(verta[indices_sample[1]].poszw).y),
					float3(s16_to_f32(verta[indices_sample[2]].nor), s16_to_f32(verta[indices_sample[2]].poszw).y)
				};
				float3 n = normalize(hit_attribute(vertex_normals, barycentrics));

				float4 texpaint1 = mytexture1.read(utex_coord.xy, utex_coord.z);
				float4 texpaint2 = mytexture2.read(utex_coord.xy, utex_coord.z);
				float3 texcolor = pow(texpaint0.rgb, float3(2.2, 2.2, 2.2));

				float3 tangent = float3(0, 0, 0);
				float3 binormal = float3(0, 0, 0);
				create_basis(n, tangent, binormal);

				texpaint1.rgb = normalize(texpaint1.rgb * 2.0 - 1.0);
				texpaint1.g = -texpaint1.g;
				n = float3x3(tangent, binormal, n) * texpaint1.rgb;

				float f = rand(tid.x, tid.y, payload.color.a, seed, constant_buffer.eye.w, mytexture_sobol, mytexture_scramble, mytexture_rank);

				#ifdef _TRANSLUCENCY
				float3 diffuseDir = texpaint0.a < f ?
					cos_weighted_hemisphere_direction(tid, ray.direction, payload.color.a, seed, constant_buffer.eye.w, mytexture_sobol, mytexture_scramble, mytexture_rank) :
					cos_weighted_hemisphere_direction(tid, n, payload.color.a, seed, constant_buffer.eye.w, mytexture_sobol, mytexture_scramble, mytexture_rank);
				#else
				float3 diffuseDir = cos_weighted_hemisphere_direction(tid, n, payload.color.a, seed, constant_buffer.eye.w, mytexture_sobol, mytexture_scramble, mytexture_rank);
				#endif

				if (f < 0.5) {
					#ifdef _TRANSLUCENCY
					float3 specularDir = texpaint0.a < f * 2 ? ray.direction : reflect(ray.direction, n);
					#else
					float3 specularDir = reflect(ray.direction, n);
					#endif

					payload.ray_dir = mix(specularDir, diffuseDir, texpaint2.g * texpaint2.g);
					float3 v = normalize(constant_buffer.eye.xyz - hit_world_position(ray, intersection));
					float dotNV = max(dot(n, v), 0.0);
					float3 specular = surfaceSpecular(texcolor, texpaint2.b);
					payload.color.xyz *= envBRDFApprox(specular, texpaint2.g, dotNV);
				}
				else {
					payload.ray_dir = diffuseDir;
					payload.color.xyz *= surfaceAlbedo(texcolor, texpaint2.b);
				}

				payload.ray_origin = hit_world_position(ray, intersection) + payload.ray_dir * 0.0001f;

				#ifdef _EMISSION
				if (int(texpaint1.a * 255.0f) % 3 == 1) { // matid
					payload.color.xyz *= 100.0f;
					payload.color.a = -2.0;
				}
				#endif

				#ifdef _SUBSURFACE
				if (int(texpaint1.a * 255.0f) % 3 == 2) {
					payload.ray_origin += ray.direction * f;
				}
				#endif
			}
			// #endif

			#ifdef _EMISSION
			if (payload.color.a == -2) {
				accum += payload.color.rgb;
				break;
			}
			#endif

			// Miss
			if (payload.color.a < 0) {
				#ifdef _TRANSPARENCY
				if (payload.color.a == -2 && transparentHits < DEPTH_TRANSPARENT) {
					payload.color.a = j;
					transparentHits++;
					i--;
				}
				#endif

				if (i == 0 && constant_buffer.params.x < 0) { // No envmap
					payload.color.rgb = float3(0.032, 0.032, 0.032);
				}

				accum += payload.color.rgb;
				break;
			}

			#ifdef _ROULETTE
			payload.color.rgb *= rrFactor;
			#endif

			ray.origin = payload.ray_origin;
			ray.direction = payload.ray_dir;
		}
	}

	float3 color = render_target.read(tid).xyz;

	#ifdef _RENDER
	float a = 1.0 / (constant_buffer.eye.w + 1);
	float b = 1.0 - a;
	color = color * b + (accum.xyz / SAMPLES) * a;
	render_target.write(float4(color.xyz, 0.0f), tid);
	#else
	if (constant_buffer.eye.w == 0) {
		color = accum.xyz / SAMPLES;
	}
	render_target.write(float4(mix(color.xyz, accum.xyz / SAMPLES, 1.0 / 16.0), 0.0f), tid);
	#endif
}
