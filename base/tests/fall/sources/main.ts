// ../../../make --graphics vulkan --run

///include "../../sources/libs/asim.h"

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
		use_depth: true
	};
	sys_start(ops);
	app_init();
	app_ready();
}

function render_commands() {
	render_path_set_target("");
	render_path_clear_target(0xff6495ed, 1.0, clear_flag_t.COLOR | clear_flag_t.DEPTH);
	render_path_draw_meshes("mesh");
}

function app_ready() {
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

	app_notify_on_update(scene_update);

	let cube: object_t = scene_get_child("Cube");
	let mesh: mesh_object_t = cube.ext;

	asim_init(mesh.data.vertex_arrays[0].values, mesh.data.index_arrays[0].values, mesh.data.scale_pos);
}

function scene_update() {
	asim_update();
	camera_update();

	if (keyboard_started("space")) {
		asim_set_sphere(0, 0, 5);
	}

	let sphere: object_t = scene_get_child("Sphere");
	let t: transform_t = sphere.transform;
	t.loc.x = asim_get_sphere_x();
	t.loc.y = asim_get_sphere_y();
	t.loc.z = asim_get_sphere_z();
	transform_build_matrix(t);
}
