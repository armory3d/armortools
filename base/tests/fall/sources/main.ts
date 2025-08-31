// ../../make --run

///include "../../sources/libs/asim.h"

let body: any;

function main() {
	let ops: iron_window_options_t = {
		title: "Empty",
		width: 1280,
		height: 720,
		x: -1,
		y: -1,
		features: window_features_t.RESIZABLE | window_features_t.MINIMIZABLE | window_features_t.MAXIMIZABLE,
		mode: window_mode_t.WINDOWED,
		frequency: 60,
		vsync: true,
		depth_bits: 32
	};
	sys_start(ops);
	ready();
}

function render_commands() {
	render_path_set_target("", null, null, clear_flag_t.COLOR | clear_flag_t.DEPTH, 0xff6495ed, 1.0);
	render_path_draw_meshes("mesh");
}

function ready() {
	render_path_commands = render_commands;

	let scene: scene_t = {
		name: "Scene",
		objects: [
			{
				name: "Cube",
				type: "mesh_object",
				data_ref: "cube.arm/Cube",
				material_refs: ["MyMaterial"],
				visible: true,
				spawn: true
			},
			{
				name: "Sphere",
				type: "mesh_object",
				data_ref: "sphere.arm/Sphere",
				material_refs: ["MyMaterial"],
				visible: true,
				spawn: true
			},
			{
				name: "Camera",
				type: "camera_object",
				data_ref: "MyCamera",
				visible: true,
				spawn: true
			}
		],
		camera_datas: [
			{
				name: "MyCamera",
				near_plane: 0.1,
				far_plane: 100.0,
				fov: 0.85
			}
		],
		camera_ref: "Camera",
		material_datas: [
			{
				name: "MyMaterial",
				shader: "MyShader",
				contexts: [
					{
						name: "mesh",
						bind_textures: [
							{
								name: "my_texture",
								file: "texture.k"
							}
						]
					}
				]
			}
		],
		shader_datas: [
			{
				name: "MyShader",
				contexts: [
					{
						name: "mesh",
						vertex_shader: "mesh.vert",
						fragment_shader: "mesh.frag",
						compare_mode: "less",
						cull_mode: "clockwise",
						depth_write: true,
						vertex_elements: [
							{
								name: "pos",
								data: "short4norm"
							},
							{
								name: "nor",
								data: "short2norm"
							},
							{
								name: "tex",
								data: "short2norm"
							}
						],
						constants: [
							{
								name: "WVP",
								type: "mat4",
								link: "_world_view_proj_matrix"
							}
						],
						texture_units: [
							{
								name: "my_texture"
							}
						]
					}
				]
			}
		]
	};
	map_set(data_cached_scene_raws, scene.name, scene);

	// Instantiate scene
	scene_create(scene);
	scene_ready();
}

function scene_ready() {
	// Set camera
	let t: transform_t = scene_camera.base.transform;
	t.loc = vec4_create(0, -10, 0);
	t.rot = quat_from_to(vec4_create(0, 0, 1), vec4_create(0, -1, 0));
	transform_build_matrix(t);

	sys_notify_on_update(scene_update);

	let cube: object_t = scene_get_child("Cube");
	let mesh: mesh_object_t = cube.ext;

	asim_world_create();
	body = asim_body_create(1, 1, 1, 1, 1, 0, 0, 5, null, null, 1);
	asim_body_create(0, 1, 1, 1, 1, 0, 0, 0, mesh.data.vertex_arrays[0].values, mesh.data.index_array, mesh.data.scale_pos);
}

function scene_update() {
	asim_world_update(sys_delta());
	camera_update();

	if (keyboard_started("space")) {
		asim_body_sync_transform(body, vec4_create(0, 0, 5), quat_create(0, 0, 0, 1));
	}

	let sphere: object_t = scene_get_child("Sphere");
	let t: transform_t = sphere.transform;
	asim_body_get_pos(body, ADDRESS(t.loc));
	transform_build_matrix(t);
}

function tr(id: string, vars: map_t<string, string> = null): string {}
