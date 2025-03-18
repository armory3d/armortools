#include "analyzer.h"

#include "array.h"
#include "errors.h"

#include <string.h>

static render_pipelines all_render_pipelines;
// a pipeline group is a collection of pipelines that share shaders
static render_pipeline_groups all_render_pipeline_groups;

static compute_shaders all_compute_shaders;

static raytracing_pipelines all_raytracing_pipelines;
// a pipeline group is a collection of pipelines that share shaders
static raytracing_pipeline_groups all_raytracing_pipeline_groups;

static void find_referenced_global_for_var(variable v, global_id *globals, size_t *globals_size) {
	for (global_id j = 0; get_global(j) != NULL && get_global(j)->type != NO_TYPE; ++j) {
		global *g = get_global(j);
		if (v.index == g->var_index) {
			bool found = false;
			for (size_t k = 0; k < *globals_size; ++k) {
				if (globals[k] == j) {
					found = true;
					break;
				}
			}
			if (!found) {
				globals[*globals_size] = j;
				*globals_size += 1;
			}
			return;
		}
	}
}

void find_referenced_globals(function *f, global_id *globals, size_t *globals_size) {
	if (f->block == NULL) {
		// built-in
		return;
	}

	function *functions[256];
	size_t    functions_size = 0;

	functions[functions_size] = f;
	functions_size += 1;

	find_referenced_functions(f, functions, &functions_size);

	for (size_t l = 0; l < functions_size; ++l) {
		uint8_t *data = functions[l]->code.o;
		size_t   size = functions[l]->code.size;

		size_t index = 0;
		while (index < size) {
			opcode *o = (opcode *)&data[index];
			switch (o->type) {
			case OPCODE_MULTIPLY:
			case OPCODE_DIVIDE:
			case OPCODE_ADD:
			case OPCODE_SUB:
			case OPCODE_EQUALS:
			case OPCODE_NOT_EQUALS:
			case OPCODE_GREATER:
			case OPCODE_GREATER_EQUAL:
			case OPCODE_LESS:
			case OPCODE_LESS_EQUAL: {
				find_referenced_global_for_var(o->op_binary.left, globals, globals_size);
				find_referenced_global_for_var(o->op_binary.right, globals, globals_size);
				break;
			}
			case OPCODE_LOAD_MEMBER: {
				find_referenced_global_for_var(o->op_load_member.from, globals, globals_size);
				break;
			}
			case OPCODE_STORE_MEMBER:
			case OPCODE_SUB_AND_STORE_MEMBER:
			case OPCODE_ADD_AND_STORE_MEMBER:
			case OPCODE_DIVIDE_AND_STORE_MEMBER:
			case OPCODE_MULTIPLY_AND_STORE_MEMBER: {
				find_referenced_global_for_var(o->op_store_member.to, globals, globals_size);
				break;
			}
			case OPCODE_CALL: {
				for (uint8_t i = 0; i < o->op_call.parameters_size; ++i) {
					find_referenced_global_for_var(o->op_call.parameters[i], globals, globals_size);
				}
				break;
			}
			default:
				break;
			}

			index += o->size;
		}
	}
}

void find_referenced_functions(function *f, function **functions, size_t *functions_size) {
	if (f->block == NULL) {
		// built-in
		return;
	}

	uint8_t *data = f->code.o;
	size_t   size = f->code.size;

	size_t index = 0;
	while (index < size) {
		opcode *o = (opcode *)&data[index];
		switch (o->type) {
		case OPCODE_CALL: {
			for (function_id i = 0; get_function(i) != NULL; ++i) {
				function *f = get_function(i);
				if (f->name == o->op_call.func) {
					if (f->block == NULL) {
						// built-in
						break;
					}

					bool found = false;
					for (size_t j = 0; j < *functions_size; ++j) {
						if (functions[j]->name == o->op_call.func) {
							found = true;
							break;
						}
					}
					if (!found) {
						functions[*functions_size] = f;
						*functions_size += 1;
						find_referenced_functions(f, functions, functions_size);
					}
					break;
				}
			}
			break;
		}
		default:
			break;
		}

		index += o->size;
	}
}

static void add_found_type(type_id t, type_id *types, size_t *types_size) {
	for (size_t i = 0; i < *types_size; ++i) {
		if (types[i] == t) {
			return;
		}
	}

	types[*types_size] = t;
	*types_size += 1;
}

void find_referenced_types(function *f, type_id *types, size_t *types_size) {
	if (f->block == NULL) {
		// built-in
		return;
	}

	function *functions[256];
	size_t    functions_size = 0;

	functions[functions_size] = f;
	functions_size += 1;

	find_referenced_functions(f, functions, &functions_size);

	for (size_t function_index = 0; function_index < functions_size; ++function_index) {
		function     *func    = functions[function_index];
		debug_context context = {0};
		for (uint8_t parameter_index = 0; parameter_index < func->parameters_size; ++parameter_index) {
			check(func->parameter_types[parameter_index].type != NO_TYPE, context, "Function parameter type not found");
			add_found_type(func->parameter_types[parameter_index].type, types, types_size);
		}
		check(func->return_type.type != NO_TYPE, context, "Function return type missing");
		add_found_type(func->return_type.type, types, types_size);

		uint8_t *data = functions[function_index]->code.o;
		size_t   size = functions[function_index]->code.size;

		size_t index = 0;
		while (index < size) {
			opcode *o = (opcode *)&data[index];
			switch (o->type) {
			case OPCODE_VAR:
				add_found_type(o->op_var.var.type.type, types, types_size);
				break;
			default:
				break;
			}

			index += o->size;
		}
	}
}

static bool has_set(descriptor_sets *sets, descriptor_set *set) {
	for (size_t set_index = 0; set_index < sets->size; ++set_index) {
		if (sets->values[set_index] == set) {
			return true;
		}
	}

	return false;
}

static void add_set(descriptor_sets *sets, descriptor_set *set) {
	if (has_set(sets, set)) {
		return;
	}

	static_array_push_p(sets, set);
}

static void find_referenced_sets(global_id *globals, size_t globals_size, descriptor_sets *sets) {
	for (size_t global_index = 0; global_index < globals_size; ++global_index) {
		global *g = get_global(globals[global_index]);

		if (g->sets_count == 0) {
			continue;
		}

		if (g->sets_count == 1) {
			add_set(sets, g->sets[0]);
			continue;
		}
	}

	for (size_t global_index = 0; global_index < globals_size; ++global_index) {
		global *g = get_global(globals[global_index]);

		if (g->sets_count < 2) {
			continue;
		}

		bool found = false;

		for (size_t set_index = 0; set_index < g->sets_count; ++set_index) {
			descriptor_set *set = g->sets[set_index];

			if (has_set(sets, set)) {
				found = true;
				break;
			}
		}

		if (!found) {
			debug_context context = {0};
			error(context, "Global %s could be used from multiple descriptor sets.", get_name(g->name));
		}
	}
}

static render_pipeline extract_render_pipeline_from_type(type *t) {
	name_id vertex_shader_name        = NO_NAME;
	name_id amplification_shader_name = NO_NAME;
	name_id mesh_shader_name          = NO_NAME;
	name_id fragment_shader_name      = NO_NAME;

	for (size_t j = 0; j < t->members.size; ++j) {
		if (t->members.m[j].name == add_name("vertex")) {
			vertex_shader_name = t->members.m[j].value.identifier;
		}
		else if (t->members.m[j].name == add_name("amplification")) {
			amplification_shader_name = t->members.m[j].value.identifier;
		}
		else if (t->members.m[j].name == add_name("mesh")) {
			mesh_shader_name = t->members.m[j].value.identifier;
		}
		else if (t->members.m[j].name == add_name("fragment")) {
			fragment_shader_name = t->members.m[j].value.identifier;
		}
	}

	debug_context context = {0};
	check(vertex_shader_name != NO_NAME || mesh_shader_name != NO_NAME, context, "vertex or mesh shader missing");
	check(fragment_shader_name != NO_NAME, context, "fragment shader missing");

	render_pipeline pipeline = {0};

	for (function_id i = 0; get_function(i) != NULL; ++i) {
		function *f = get_function(i);
		if (vertex_shader_name != NO_NAME && f->name == vertex_shader_name) {
			pipeline.vertex_shader = f;
		}
		if (amplification_shader_name != NO_NAME && f->name == amplification_shader_name) {
			pipeline.amplification_shader = f;
		}
		if (mesh_shader_name != NO_NAME && f->name == mesh_shader_name) {
			pipeline.mesh_shader = f;
		}
		if (f->name == fragment_shader_name) {
			pipeline.fragment_shader = f;
		}
	}

	return pipeline;
}

static void find_all_render_pipelines(void) {
	static_array_init(all_render_pipelines);

	for (type_id i = 0; get_type(i) != NULL; ++i) {
		type *t = get_type(i);
		if (!t->built_in && has_attribute(&t->attributes, add_name("pipe"))) {
			static_array_push(all_render_pipelines, extract_render_pipeline_from_type(t));
		}
	}
}

static void find_render_pipeline_groups(void) {
	static_array_init(all_render_pipeline_groups);

	render_pipeline_indices remaining_pipelines;
	static_array_init(remaining_pipelines);

	for (uint32_t index = 0; index < all_render_pipelines.size; ++index) {
		static_array_push(remaining_pipelines, index);
	}

	while (remaining_pipelines.size > 0) {
		render_pipeline_indices next_remaining_pipelines;
		static_array_init(next_remaining_pipelines);

		render_pipeline_group group;
		static_array_init(group);

		static_array_push(group, remaining_pipelines.values[0]);

		for (size_t index = 1; index < remaining_pipelines.size; ++index) {
			uint32_t         pipeline_index = remaining_pipelines.values[index];
			render_pipeline *pipeline       = &all_render_pipelines.values[pipeline_index];

			bool found = false;

			for (size_t index_in_bucket = 0; index_in_bucket < group.size; ++index_in_bucket) {
				render_pipeline *pipeline_in_group = &all_render_pipelines.values[group.values[index_in_bucket]];
				if (pipeline->vertex_shader == pipeline_in_group->vertex_shader || pipeline->amplification_shader == pipeline_in_group->amplification_shader ||
				    pipeline->mesh_shader == pipeline_in_group->mesh_shader || pipeline->fragment_shader == pipeline_in_group->fragment_shader) {
					found = true;
					break;
				}
			}

			if (found) {
				static_array_push(group, pipeline_index);
			}
			else {
				static_array_push(next_remaining_pipelines, pipeline_index);
			}
		}

		remaining_pipelines = next_remaining_pipelines;
		static_array_push(all_render_pipeline_groups, group);
	}
}

static void find_all_compute_shaders(void) {
	static_array_init(all_compute_shaders);

	for (function_id i = 0; get_function(i) != NULL; ++i) {
		function *f = get_function(i);
		if (has_attribute(&f->attributes, add_name("compute"))) {
			static_array_push(all_compute_shaders, f);
		}
	}
}

static raytracing_pipeline extract_raytracing_pipeline_from_type(type *t) {
	name_id gen_shader_name          = NO_NAME;
	name_id miss_shader_name         = NO_NAME;
	name_id closest_shader_name      = NO_NAME;
	name_id intersection_shader_name = NO_NAME;
	name_id any_shader_name          = NO_NAME;

	for (size_t j = 0; j < t->members.size; ++j) {
		if (t->members.m[j].name == add_name("gen")) {
			gen_shader_name = t->members.m[j].value.identifier;
		}
		else if (t->members.m[j].name == add_name("miss")) {
			miss_shader_name = t->members.m[j].value.identifier;
		}
		else if (t->members.m[j].name == add_name("closest")) {
			closest_shader_name = t->members.m[j].value.identifier;
		}
		else if (t->members.m[j].name == add_name("intersection")) {
			intersection_shader_name = t->members.m[j].value.identifier;
		}
		else if (t->members.m[j].name == add_name("any")) {
			any_shader_name = t->members.m[j].value.identifier;
		}
	}

	raytracing_pipeline pipeline = {0};

	for (function_id i = 0; get_function(i) != NULL; ++i) {
		function *f = get_function(i);
		if (gen_shader_name != NO_NAME && f->name == gen_shader_name) {
			pipeline.gen_shader = f;
		}
		if (miss_shader_name != NO_NAME && f->name == miss_shader_name) {
			pipeline.miss_shader = f;
		}
		if (closest_shader_name != NO_NAME && f->name == closest_shader_name) {
			pipeline.closest_shader = f;
		}
		if (intersection_shader_name != NO_NAME && f->name == intersection_shader_name) {
			pipeline.intersection_shader = f;
		}
		if (any_shader_name != NO_NAME && f->name == any_shader_name) {
			pipeline.any_shader = f;
		}
	}

	return pipeline;
}

static void find_all_raytracing_pipelines(void) {
	static_array_init(all_raytracing_pipelines);

	for (type_id i = 0; get_type(i) != NULL; ++i) {
		type *t = get_type(i);
		if (!t->built_in && has_attribute(&t->attributes, add_name("raypipe"))) {

			static_array_push(all_raytracing_pipelines, extract_raytracing_pipeline_from_type(t));
		}
	}
}

static void find_raytracing_pipeline_groups(void) {
	static_array_init(all_raytracing_pipeline_groups);

	raytracing_pipeline_indices remaining_pipelines;
	static_array_init(remaining_pipelines);

	for (uint32_t index = 0; index < all_raytracing_pipelines.size; ++index) {
		static_array_push(remaining_pipelines, index);
	}

	while (remaining_pipelines.size > 0) {
		raytracing_pipeline_indices next_remaining_pipelines;
		static_array_init(next_remaining_pipelines);

		raytracing_pipeline_group group;
		static_array_init(group);

		static_array_push(group, remaining_pipelines.values[0]);

		for (size_t index = 1; index < remaining_pipelines.size; ++index) {
			uint32_t             pipeline_index = remaining_pipelines.values[index];
			raytracing_pipeline *pipeline       = &all_raytracing_pipelines.values[pipeline_index];

			bool found = false;

			for (size_t index_in_bucket = 0; index_in_bucket < group.size; ++index_in_bucket) {
				raytracing_pipeline *pipeline_in_group = &all_raytracing_pipelines.values[group.values[index_in_bucket]];
				if (pipeline->gen_shader == pipeline_in_group->gen_shader || pipeline->miss_shader == pipeline_in_group->miss_shader ||
				    pipeline->closest_shader == pipeline_in_group->closest_shader || pipeline->intersection_shader == pipeline_in_group->intersection_shader ||
				    pipeline->any_shader == pipeline_in_group->any_shader) {
					found = true;
					break;
				}
			}

			if (found) {
				static_array_push(group, pipeline_index);
			}
			else {
				static_array_push(next_remaining_pipelines, pipeline_index);
			}
		}

		remaining_pipelines = next_remaining_pipelines;
		static_array_push(all_raytracing_pipeline_groups, group);
	}
}

static void check_globals_in_descriptor_set_group(descriptor_set_group *group) {
	static_array(global_id, globals, 256);

	globals set_globals;
	static_array_init(set_globals);

	for (size_t set_index = 0; set_index < group->size; ++set_index) {
		descriptor_set *set = group->values[set_index];
		for (size_t definition_index = 0; definition_index < set->definitions_count; ++definition_index) {
			global_id g = set->definitions[definition_index].global;

			for (size_t global_index = 0; global_index < set_globals.size; ++global_index) {
				if (set_globals.values[global_index] == g) {
					debug_context context = {0};
					error(context, "Global used from more than one descriptor set in one descriptor set group");
				}
			}

			static_array_push(set_globals, g);
		}
	}
}

static descriptor_set_groups all_descriptor_set_groups;

descriptor_set_group *get_descriptor_set_group(uint32_t descriptor_set_group_index) {
	assert(descriptor_set_group_index < all_descriptor_set_groups.size);
	return &all_descriptor_set_groups.values[descriptor_set_group_index];
}

static void assign_descriptor_set_group_index(function *f, uint32_t descriptor_set_group_index) {
	assert(f->descriptor_set_group_index == UINT32_MAX || f->descriptor_set_group_index == descriptor_set_group_index);
	f->descriptor_set_group_index = descriptor_set_group_index;
}

static void find_descriptor_set_groups(void) {
	static_array_init(all_descriptor_set_groups);

	for (size_t pipeline_group_index = 0; pipeline_group_index < all_render_pipeline_groups.size; ++pipeline_group_index) {
		descriptor_set_group group;
		static_array_init(group);

		global_id function_globals[256];
		size_t    function_globals_size = 0;

		render_pipeline_group *pipeline_group = &all_render_pipeline_groups.values[pipeline_group_index];
		for (size_t pipeline_index = 0; pipeline_index < pipeline_group->size; ++pipeline_index) {
			render_pipeline *pipeline = &all_render_pipelines.values[pipeline_group->values[pipeline_index]];

			if (pipeline->vertex_shader != NULL) {
				find_referenced_globals(pipeline->vertex_shader, function_globals, &function_globals_size);
			}
			if (pipeline->amplification_shader != NULL) {
				find_referenced_globals(pipeline->amplification_shader, function_globals, &function_globals_size);
			}
			if (pipeline->mesh_shader != NULL) {
				find_referenced_globals(pipeline->mesh_shader, function_globals, &function_globals_size);
			}
			if (pipeline->fragment_shader != NULL) {
				find_referenced_globals(pipeline->fragment_shader, function_globals, &function_globals_size);
			}
		}

		find_referenced_sets(function_globals, function_globals_size, &group);

		check_globals_in_descriptor_set_group(&group);

		uint32_t descriptor_set_group_index = (uint32_t)all_descriptor_set_groups.size;
		static_array_push(all_descriptor_set_groups, group);

		for (size_t pipeline_index = 0; pipeline_index < pipeline_group->size; ++pipeline_index) {
			render_pipeline *pipeline = &all_render_pipelines.values[pipeline_group->values[pipeline_index]];

			if (pipeline->vertex_shader != NULL) {
				assign_descriptor_set_group_index(pipeline->vertex_shader, descriptor_set_group_index);
			}
			if (pipeline->amplification_shader != NULL) {
				assign_descriptor_set_group_index(pipeline->amplification_shader, descriptor_set_group_index);
			}
			if (pipeline->mesh_shader != NULL) {
				assign_descriptor_set_group_index(pipeline->mesh_shader, descriptor_set_group_index);
			}
			if (pipeline->fragment_shader != NULL) {
				assign_descriptor_set_group_index(pipeline->fragment_shader, descriptor_set_group_index);
			}
		}
	}

	for (size_t compute_shader_index = 0; compute_shader_index < all_compute_shaders.size; ++compute_shader_index) {
		descriptor_set_group group;
		static_array_init(group);

		global_id function_globals[256];
		size_t    function_globals_size = 0;

		find_referenced_globals(all_compute_shaders.values[compute_shader_index], function_globals, &function_globals_size);

		find_referenced_sets(function_globals, function_globals_size, &group);

		check_globals_in_descriptor_set_group(&group);

		uint32_t descriptor_set_group_index = (uint32_t)all_descriptor_set_groups.size;
		static_array_push(all_descriptor_set_groups, group);

		for (size_t compute_shader_index = 0; compute_shader_index < all_compute_shaders.size; ++compute_shader_index) {
			assign_descriptor_set_group_index(all_compute_shaders.values[compute_shader_index], descriptor_set_group_index);
		}
	}

	for (size_t pipeline_group_index = 0; pipeline_group_index < all_raytracing_pipeline_groups.size; ++pipeline_group_index) {
		descriptor_set_group group;
		static_array_init(group);

		global_id function_globals[256];
		size_t    function_globals_size = 0;

		raytracing_pipeline_group *pipeline_group = &all_raytracing_pipeline_groups.values[pipeline_group_index];
		for (size_t pipeline_index = 0; pipeline_index < pipeline_group->size; ++pipeline_index) {
			raytracing_pipeline *pipeline = &all_raytracing_pipelines.values[pipeline_group->values[pipeline_index]];

			if (pipeline->gen_shader != NULL) {
				find_referenced_globals(pipeline->gen_shader, function_globals, &function_globals_size);
			}
			if (pipeline->miss_shader != NULL) {
				find_referenced_globals(pipeline->miss_shader, function_globals, &function_globals_size);
			}
			if (pipeline->closest_shader != NULL) {
				find_referenced_globals(pipeline->closest_shader, function_globals, &function_globals_size);
			}
			if (pipeline->intersection_shader != NULL) {
				find_referenced_globals(pipeline->intersection_shader, function_globals, &function_globals_size);
			}
			if (pipeline->any_shader != NULL) {
				find_referenced_globals(pipeline->any_shader, function_globals, &function_globals_size);
			}
		}

		find_referenced_sets(function_globals, function_globals_size, &group);

		check_globals_in_descriptor_set_group(&group);

		uint32_t descriptor_set_group_index = (uint32_t)all_descriptor_set_groups.size;
		static_array_push(all_descriptor_set_groups, group);

		for (size_t pipeline_index = 0; pipeline_index < pipeline_group->size; ++pipeline_index) {
			raytracing_pipeline *pipeline = &all_raytracing_pipelines.values[pipeline_group->values[pipeline_index]];

			if (pipeline->gen_shader != NULL) {
				assign_descriptor_set_group_index(pipeline->gen_shader, descriptor_set_group_index);
			}
			if (pipeline->miss_shader != NULL) {
				assign_descriptor_set_group_index(pipeline->miss_shader, descriptor_set_group_index);
			}
			if (pipeline->closest_shader != NULL) {
				assign_descriptor_set_group_index(pipeline->closest_shader, descriptor_set_group_index);
			}
			if (pipeline->intersection_shader != NULL) {
				assign_descriptor_set_group_index(pipeline->intersection_shader, descriptor_set_group_index);
			}
			if (pipeline->any_shader != NULL) {
				assign_descriptor_set_group_index(pipeline->any_shader, descriptor_set_group_index);
			}
		}
	}
}

descriptor_set_group *find_descriptor_set_group_for_type(type *t) {
	if (!t->built_in && has_attribute(&t->attributes, add_name("pipe"))) {
		render_pipeline pipeline = extract_render_pipeline_from_type(t);

		if (pipeline.vertex_shader->descriptor_set_group_index != UINT32_MAX) {
			return &all_descriptor_set_groups.values[pipeline.vertex_shader->descriptor_set_group_index];
		}

		if (pipeline.amplification_shader->descriptor_set_group_index != UINT32_MAX) {
			return &all_descriptor_set_groups.values[pipeline.amplification_shader->descriptor_set_group_index];
		}

		if (pipeline.mesh_shader->descriptor_set_group_index != UINT32_MAX) {
			return &all_descriptor_set_groups.values[pipeline.mesh_shader->descriptor_set_group_index];
		}

		if (pipeline.fragment_shader->descriptor_set_group_index != UINT32_MAX) {
			return &all_descriptor_set_groups.values[pipeline.fragment_shader->descriptor_set_group_index];
		}

		return NULL;
	}

	if (!t->built_in && has_attribute(&t->attributes, add_name("raypipe"))) {
		raytracing_pipeline pipeline = extract_raytracing_pipeline_from_type(t);

		if (pipeline.gen_shader->descriptor_set_group_index != UINT32_MAX) {
			return &all_descriptor_set_groups.values[pipeline.gen_shader->descriptor_set_group_index];
		}

		if (pipeline.miss_shader->descriptor_set_group_index != UINT32_MAX) {
			return &all_descriptor_set_groups.values[pipeline.miss_shader->descriptor_set_group_index];
		}

		if (pipeline.closest_shader->descriptor_set_group_index != UINT32_MAX) {
			return &all_descriptor_set_groups.values[pipeline.closest_shader->descriptor_set_group_index];
		}

		if (pipeline.intersection_shader->descriptor_set_group_index != UINT32_MAX) {
			return &all_descriptor_set_groups.values[pipeline.intersection_shader->descriptor_set_group_index];
		}

		if (pipeline.any_shader->descriptor_set_group_index != UINT32_MAX) {
			return &all_descriptor_set_groups.values[pipeline.any_shader->descriptor_set_group_index];
		}

		return NULL;
	}

	return NULL;
}

descriptor_set_group *find_descriptor_set_group_for_function(function *f) {
	if (f->descriptor_set_group_index != UINT32_MAX) {
		return &all_descriptor_set_groups.values[f->descriptor_set_group_index];
	}
	else {
		return NULL;
	}
}

void analyze(void) {
	find_all_render_pipelines();
	find_render_pipeline_groups();

	find_all_compute_shaders();

	find_all_raytracing_pipelines();
	find_raytracing_pipeline_groups();

	find_descriptor_set_groups();
}
