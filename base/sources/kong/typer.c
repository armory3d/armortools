#include "compiler.h"
#include "disasm.h"
#include "errors.h"
#include "functions.h"
#include "globals.h"
#include "log.h"
#include "names.h"
#include "parser.h"
#include "tokenizer.h"
#include "types.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

type_ref find_local_var_type(block *b, name_id name) {
	if (b == NULL) {
		type_ref t;
		init_type_ref(&t, NO_NAME);
		return t;
	}

	for (size_t i = 0; i < b->vars.size; ++i) {
		if (b->vars.v[i].name == name) {
			debug_context context = {0};
			check(b->vars.v[i].type.type != NO_TYPE, context, "Local var has no type");
			return b->vars.v[i].type;
		}
	}

	return find_local_var_type(b->parent, name);
}

void resolve_types_in_expression(statement *parent, expression *e);

static void resolve_types_in_element(statement *parent_block, expression *element) {
	resolve_types_in_expression(parent_block, element->element.of);
	resolve_types_in_expression(parent_block, element->element.element_index);

	type_id of_type = element->element.of->type.type;

	assert(of_type != NO_TYPE);

	type *of = get_type(of_type);

	if (of->tex_kind != TEXTURE_KIND_NONE) {
		if (of->tex_format == TEXTURE_FORMAT_UNDEFINED) {
			element->type.type = float4_id;
		}
		else if (of->tex_format == TEXTURE_FORMAT_FRAMEBUFFER) {
			element->type.type = float4_id;
		}
		else if (of->tex_format == TEXTURE_FORMAT_RGBA32_FLOAT) {
			element->type.type = float4_id;
		}
		else if (of->tex_format == TEXTURE_FORMAT_RGBA8_UNORM) {
			element->type.type = float4_id;
		}
		else if (of->tex_format == TEXTURE_FORMAT_DEPTH) {
			element->type.type = float_id;
		}
		else {
			// TODO
			assert(false);
		}
	}
	else {
		if (of->array_size > 0) {
			element->type.type = of->base;
		}
		else {
			debug_context context = {0};
			error(context, "Indexed non-array %s", get_name(of->name));
		}
	}
}

static void resolve_types_in_member(statement *parent_block, expression *member) {
	resolve_types_in_expression(parent_block, member->member.of);

	type_id of_type     = member->member.of->type.type;
	name_id member_name = member->member.member_name;

	assert(of_type != NO_TYPE);

	if (is_vector_or_scalar(of_type)) {
		expression *of = member->member.of;

		member->kind       = EXPRESSION_SWIZZLE;
		member->swizzle.of = of;

		char    *name         = get_name(member_name);
		uint32_t swizzle_size = (uint32_t)strlen(name);

		if (swizzle_size > 4) {
			debug_context context = {0};
			error(context, "Swizzle size can not be more than four", get_name(member_name));
		}

		member->swizzle.swizz.size = swizzle_size;

		for (uint32_t swizzle_index = 0; swizzle_index < swizzle_size; ++swizzle_index) {
			switch (name[swizzle_index]) {
			case 'r':
			case 'x':
				member->swizzle.swizz.indices[swizzle_index] = 0;
				break;
			case 'g':
			case 'y':
				if (1 >= vector_size(of_type)) {
					debug_context context = {0};
					error(context, "Swizzle out of bounds", get_name(member_name));
				}
				member->swizzle.swizz.indices[swizzle_index] = 1;
				break;
			case 'b':
			case 'z':
				if (2 >= vector_size(of_type)) {
					debug_context context = {0};
					error(context, "Swizzle out of bounds", get_name(member_name));
				}
				member->swizzle.swizz.indices[swizzle_index] = 2;
				break;
			case 'a':
			case 'w':
				if (3 >= vector_size(of_type)) {
					debug_context context = {0};
					error(context, "Swizzle out of bounds", get_name(member_name));
				}
				member->swizzle.swizz.indices[swizzle_index] = 3;
				break;
			}
		}

		if (of_type == float_id || of_type == float2_id || of_type == float3_id || of_type == float4_id) {
			switch (swizzle_size) {
			case 1:
				member->type.type = float_id;
				break;
			case 2:
				member->type.type = float2_id;
				break;
			case 3:
				member->type.type = float3_id;
				break;
			case 4:
				member->type.type = float4_id;
				break;
			default:
				assert(false);
				break;
			}
		}
		else if (of_type == int_id || of_type == int2_id || of_type == int3_id || of_type == int4_id) {
			switch (swizzle_size) {
			case 1:
				member->type.type = int_id;
				break;
			case 2:
				member->type.type = int2_id;
				break;
			case 3:
				member->type.type = int3_id;
				break;
			case 4:
				member->type.type = int4_id;
				break;
			default:
				assert(false);
				break;
			}
		}
		else if (of_type == uint_id || of_type == uint2_id || of_type == uint3_id || of_type == uint4_id) {
			switch (swizzle_size) {
			case 1:
				member->type.type = uint_id;
				break;
			case 2:
				member->type.type = uint2_id;
				break;
			case 3:
				member->type.type = uint3_id;
				break;
			case 4:
				member->type.type = uint4_id;
				break;
			default:
				assert(false);
				break;
			}
		}
		else if (of_type == bool_id || of_type == bool2_id || of_type == bool3_id || of_type == bool4_id) {
			switch (swizzle_size) {
			case 1:
				member->type.type = bool_id;
				break;
			case 2:
				member->type.type = bool2_id;
				break;
			case 3:
				member->type.type = bool3_id;
				break;
			case 4:
				member->type.type = bool4_id;
				break;
			default:
				assert(false);
				break;
			}
		}
		else {
			assert(false);
		}
	}
	else {
		type *of_struct = get_type(of_type);

		for (size_t i = 0; i < of_struct->members.size; ++i) {
			if (of_struct->members.m[i].name == member_name) {
				member->type = of_struct->members.m[i].type;
				return;
			}
		}

		debug_context context = {0};
		error(context, "Member %s not found", get_name(member_name));
	}
}

static bool types_compatible(type_id left, type_id right) {
	if (left == right) {
		return true;
	}

	if ((left == int_id && right == float_id) || (left == float_id && right == int_id)) {
		return true;
	}
	if ((left == int2_id && right == float2_id) || (left == float2_id && right == int2_id)) {
		return true;
	}
	if ((left == int3_id && right == float3_id) || (left == float3_id && right == int3_id)) {
		return true;
	}
	if ((left == int4_id && right == float4_id) || (left == float4_id && right == int4_id)) {
		return true;
	}

	if ((left == uint_id && right == float_id) || (left == float_id && right == uint_id)) {
		return true;
	}
	if ((left == uint2_id && right == float2_id) || (left == float2_id && right == uint2_id)) {
		return true;
	}
	if ((left == uint3_id && right == float3_id) || (left == float3_id && right == uint3_id)) {
		return true;
	}
	if ((left == uint4_id && right == float4_id) || (left == float4_id && right == uint4_id)) {
		return true;
	}

	if ((left == uint_id && right == int_id) || (left == int_id && right == uint_id)) {
		return true;
	}
	if ((left == uint2_id && right == int2_id) || (left == int2_id && right == uint2_id)) {
		return true;
	}
	if ((left == uint3_id && right == int3_id) || (left == int3_id && right == uint3_id)) {
		return true;
	}
	if ((left == uint4_id && right == int4_id) || (left == int4_id && right == uint4_id)) {
		return true;
	}

	if ((left == float_id && right == float2_id) || (left == float_id && right == float3_id) || (left == float_id && right == float4_id) ||
	    (left == float2_id && right == float_id) || (left == float3_id && right == float_id) || (left == float4_id && right == float_id)) {
		return true;
	}

	if ((left == int_id && right == int2_id) || (left == int_id && right == int3_id) || (left == int_id && right == int4_id) ||
	    (left == int2_id && right == int_id) || (left == int3_id && right == int_id) || (left == int4_id && right == int_id)) {
		return true;
	}

	if ((left == uint_id && right == uint2_id) || (left == uint_id && right == uint3_id) || (left == uint_id && right == uint4_id) ||
	    (left == uint2_id && right == uint_id) || (left == uint3_id && right == uint_id) || (left == uint4_id && right == uint_id)) {
		return true;
	}

	if ((left == uint_id && right == int2_id) || (left == uint_id && right == int3_id) || (left == uint_id && right == int4_id) ||
	    (left == int2_id && right == uint_id) || (left == int3_id && right == uint_id) || (left == int4_id && right == uint_id)) {
		return true;
	}

	if ((left == int_id && right == uint2_id) || (left == int_id && right == uint3_id) || (left == int_id && right == uint4_id) ||
	    (left == uint2_id && right == int_id) || (left == uint3_id && right == int_id) || (left == uint4_id && right == int_id)) {
		return true;
	}

	return false;
}

static type_ref upgrade_type(type_ref left_type, type_ref right_type) {
	type_id left  = left_type.type;
	type_id right = right_type.type;

	if (left == right) {
		return left_type;
	}

	if (left == int_id && right == float_id) {
		return right_type;
	}
	if (left == float_id && right == int_id) {
		return left_type;
	}
	if (left == int2_id && right == float2_id) {
		return right_type;
	}
	if (left == float2_id && right == int2_id) {
		return left_type;
	}
	if (left == int3_id && right == float3_id) {
		return right_type;
	}
	if (left == float3_id && right == int3_id) {
		return left_type;
	}
	if (left == int4_id && right == float4_id) {
		return right_type;
	}
	if (left == float4_id && right == int4_id) {
		return left_type;
	}

	if (left == uint_id && right == float_id) {
		return right_type;
	}
	if (left == float_id && right == uint_id) {
		return left_type;
	}
	if (left == uint2_id && right == float2_id) {
		return right_type;
	}
	if (left == float2_id && right == uint2_id) {
		return left_type;
	}
	if (left == uint3_id && right == float3_id) {
		return right_type;
	}
	if (left == float3_id && right == uint3_id) {
		return left_type;
	}
	if (left == uint4_id && right == float4_id) {
		return right_type;
	}
	if (left == float4_id && right == uint4_id) {
		return left_type;
	}

	if (left == uint_id && right == int_id) {
		return right_type;
	}
	if (left == int_id && right == uint_id) {
		return left_type;
	}
	if (left == uint2_id && right == int2_id) {
		return right_type;
	}
	if (left == int2_id && right == uint2_id) {
		return left_type;
	}
	if (left == uint3_id && right == int3_id) {
		return right_type;
	}
	if (left == int3_id && right == uint3_id) {
		return left_type;
	}
	if (left == uint4_id && right == int4_id) {
		return right_type;
	}
	if (left == int4_id && right == uint4_id) {
		return left_type;
	}

	if ((left == float2_id && right == float_id) || (left == float3_id && right == float_id) || (left == float4_id && right == float_id)) {
		return left_type;
	}

	if ((left == float_id && right == float2_id) || (left == float_id && right == float3_id) || (left == float_id && right == float4_id)) {
		return right_type;
	}

	if ((left == int2_id && right == int_id) || (left == int3_id && right == int_id) || (left == int4_id && right == int_id)) {
		return left_type;
	}

	if ((left == int_id && right == int2_id) || (left == int_id && right == int3_id) || (left == int_id && right == int4_id)) {
		return right_type;
	}

	if ((left == uint2_id && right == uint_id) || (left == uint3_id && right == uint_id) || (left == uint4_id && right == uint_id)) {
		return left_type;
	}

	if ((left == uint_id && right == uint2_id) || (left == uint_id && right == uint3_id) || (left == uint_id && right == uint4_id)) {
		return right_type;
	}

	if ((left == uint2_id && right == int_id) || (left == uint3_id && right == int_id) || (left == uint4_id && right == int_id)) {
		return left_type;
	}

	if ((left == int_id && right == uint2_id) || (left == int_id && right == uint3_id) || (left == int_id && right == uint4_id)) {
		return right_type;
	}

	if ((left == int2_id && right == uint_id) || (left == int3_id && right == uint_id) || (left == int4_id && right == uint_id)) {
		return left_type;
	}

	if ((left == uint_id && right == int2_id) || (left == uint_id && right == int3_id) || (left == uint_id && right == int4_id)) {
		return right_type;
	}

	kong_log(LOG_LEVEL_WARNING, "Suspicious type upgrade");
	return left_type;
}

void resolve_types_in_expression(statement *parent, expression *e) {
	switch (e->kind) {
	case EXPRESSION_BINARY: {
		resolve_types_in_expression(parent, e->binary.left);
		resolve_types_in_expression(parent, e->binary.right);
		switch (e->binary.op) {
		case OPERATOR_EQUALS:
		case OPERATOR_NOT_EQUALS:
		case OPERATOR_GREATER:
		case OPERATOR_GREATER_EQUAL:
		case OPERATOR_LESS:
		case OPERATOR_LESS_EQUAL:
		case OPERATOR_OR:
		case OPERATOR_AND: {
			e->type.type = bool_id;
			break;
		}
		case OPERATOR_BITWISE_XOR:
		case OPERATOR_BITWISE_AND:
		case OPERATOR_BITWISE_OR:
		case OPERATOR_LEFT_SHIFT:
		case OPERATOR_RIGHT_SHIFT: {
			e->type = e->binary.left->type;
			break;
		}
		case OPERATOR_MULTIPLY:
		case OPERATOR_MULTIPLY_ASSIGN: {
			type_id left_type  = e->binary.left->type.type;
			type_id right_type = e->binary.right->type.type;
			if ((left_type == float4x4_id && right_type == float4_id) || (left_type == float3x3_id && right_type == float3_id)) {
				e->type = e->binary.right->type;
			}
			else if (right_type == float_id && (left_type == float2_id || left_type == float3_id || left_type == float4_id)) {
				e->type = e->binary.left->type;
			}
			else if (types_compatible(left_type, right_type)) {
				e->type = upgrade_type(e->binary.left->type, e->binary.right->type);
			}
			else {
				debug_context context = {0};
				error(context, "Type mismatch %s vs %s", get_name(get_type(left_type)->name), get_name(get_type(right_type)->name));
			}
			break;
		}
		case OPERATOR_MINUS:
		case OPERATOR_PLUS:
		case OPERATOR_DIVIDE:
		case OPERATOR_MOD: {
			type_id left_type  = e->binary.left->type.type;
			type_id right_type = e->binary.right->type.type;
			if (!types_compatible(left_type, right_type)) {
				debug_context context = {0};
				error(context, "Type mismatch %s vs %s", get_name(get_type(left_type)->name), get_name(get_type(right_type)->name));
			}
			e->type = upgrade_type(e->binary.left->type, e->binary.right->type);
			break;
		}
		case OPERATOR_ASSIGN:
		case OPERATOR_DIVIDE_ASSIGN:
		case OPERATOR_MINUS_ASSIGN:
		case OPERATOR_PLUS_ASSIGN: {
			type_id left_type  = e->binary.left->type.type;
			type_id right_type = e->binary.right->type.type;
			if (!types_compatible(left_type, right_type)) {
				debug_context context = {0};
				error(context, "Type mismatch %s vs %s", get_name(get_type(left_type)->name), get_name(get_type(right_type)->name));
			}
			e->type = e->binary.left->type;
			break;
		}
		case OPERATOR_NOT: {
			debug_context context = {0};
			error(context, "Weird binary operator");
			break;
		}
		}
		break;
	}
	case EXPRESSION_UNARY: {
		resolve_types_in_expression(parent, e->unary.right);
		switch (e->unary.op) {
		case OPERATOR_MINUS:
		case OPERATOR_PLUS: {
			e->type = e->unary.right->type;
			break;
		}
		case OPERATOR_NOT: {
			e->type.type = bool_id;
			break;
		}
		case OPERATOR_EQUALS:
		case OPERATOR_NOT_EQUALS:
		case OPERATOR_GREATER:
		case OPERATOR_GREATER_EQUAL:
		case OPERATOR_LESS:
		case OPERATOR_LESS_EQUAL:
		case OPERATOR_DIVIDE:
		case OPERATOR_MULTIPLY:
		case OPERATOR_OR:
		case OPERATOR_BITWISE_XOR:
		case OPERATOR_BITWISE_AND:
		case OPERATOR_BITWISE_OR:
		case OPERATOR_LEFT_SHIFT:
		case OPERATOR_RIGHT_SHIFT:
		case OPERATOR_AND:
		case OPERATOR_MOD:
		case OPERATOR_ASSIGN:
		default: {
			debug_context context = {0};
			error(context, "Weird unary operator");
			break;
		}
		}
		break;
	}
	case EXPRESSION_BOOLEAN: {
		e->type.type = bool_id;
		break;
	}
	case EXPRESSION_FLOAT: {
		e->type.type = float_id;
		break;
	}
	case EXPRESSION_INT: {
		e->type.type = int_id;
		break;
	}
	case EXPRESSION_VARIABLE: {
		global *g = find_global(e->variable);
		if (g != NULL && g->type != NO_TYPE) {
			e->type.type = g->type;
		}
		else {
			type_ref type = find_local_var_type(&parent->block, e->variable);
			if (type.type == NO_TYPE) {
				type                  = find_local_var_type(&parent->block, e->variable);
				debug_context context = {0};
				error(context, "Variable %s not found", get_name(e->variable));
			}
			e->type = type;
		}
		break;
	}
	case EXPRESSION_GROUPING: {
		resolve_types_in_expression(parent, e->grouping);
		e->type = e->grouping->type;
		break;
	}
	case EXPRESSION_CALL: {
		if (e->call.func_name == add_name("sample") || e->call.func_name == add_name("sample_lod")) {
			if (e->call.parameters.e[0]->kind == EXPRESSION_VARIABLE) {
				global *g = find_global(e->call.parameters.e[0]->variable);
				assert(g != NULL);
				if (is_depth(get_type(g->type)->tex_format)) {
					e->type.type = float_id;
				}
				else {
					e->type.type = float4_id;
				}
			}
			else {
				e->type.type = float4_id;
			}
		}
		else {
			for (function_id i = 0; get_function(i) != NULL; ++i) {
				function *f = get_function(i);
				if (f->name == e->call.func_name) {
					e->type = f->return_type;
					break;
				}
			}
		}

		for (size_t i = 0; i < e->call.parameters.size; ++i) {
			resolve_types_in_expression(parent, e->call.parameters.e[i]);
		}
		break;
	}
	case EXPRESSION_MEMBER: {
		resolve_types_in_member(parent, e);
		break;
	}
	case EXPRESSION_ELEMENT: {
		resolve_types_in_element(parent, e);
		break;
	}
	case EXPRESSION_SWIZZLE:
		assert(false); // swizzle is created in the typer
		break;
	}

	if (e->type.type == NO_TYPE) {
		debug_context context = {0};
		// const char *n = get_name(e->call.func_name);
		error(context, "Could not resolve type");
	}
}

void resolve_types_in_block(statement *parent, statement *block) {
	debug_context context = {0};
	check(block->kind == STATEMENT_BLOCK, context, "Malformed block");

	for (size_t i = 0; i < block->block.statements.size; ++i) {
		statement *s = block->block.statements.s[i];
		switch (s->kind) {
		case STATEMENT_DISCARD:
			break;
		case STATEMENT_EXPRESSION: {
			resolve_types_in_expression(block, s->expression);
			break;
		}
		case STATEMENT_RETURN_EXPRESSION: {
			resolve_types_in_expression(block, s->expression);
			break;
		}
		case STATEMENT_IF: {
			resolve_types_in_expression(block, s->iffy.test);
			resolve_types_in_block(block, s->iffy.if_block);
			for (uint16_t i = 0; i < s->iffy.else_size; ++i) {
				if (s->iffy.else_tests[i] != NULL) {
					resolve_types_in_expression(block, s->iffy.else_tests[i]);
				}
				resolve_types_in_block(block, s->iffy.else_blocks[i]);
			}
			break;
		}
		case STATEMENT_WHILE:
		case STATEMENT_DO_WHILE: {
			resolve_types_in_expression(block, s->whiley.test);
			resolve_types_in_block(block, s->whiley.while_block);
			break;
		}
		case STATEMENT_BLOCK: {
			resolve_types_in_block(block, s);
			break;
		}
		case STATEMENT_LOCAL_VARIABLE: {
			name_id var_name      = s->local_variable.var.name;
			name_id var_type_name = s->local_variable.var.type.unresolved.name;

			if (s->local_variable.var.type.type == NO_TYPE && var_type_name != NO_NAME) {
				s->local_variable.var.type.type = find_type_by_ref(&s->local_variable.var.type);
			}

			if (s->local_variable.var.type.type == NO_TYPE) {
				debug_context context = {0};
				error(context, "Could not find type %s for %s", get_name(var_type_name), get_name(var_name));
			}

			if (s->local_variable.init != NULL) {
				resolve_types_in_expression(block, s->local_variable.init);
			}

			block->block.vars.v[block->block.vars.size].name = var_name;
			block->block.vars.v[block->block.vars.size].type = s->local_variable.var.type;
			++block->block.vars.size;
			break;
		}
		}
	}
}

void resolve_types(void) {
	for (type_id i = 0; get_type(i) != NULL; ++i) {
		type *s = get_type(i);
		for (size_t j = 0; j < s->members.size; ++j) {
			if (s->members.m[j].type.type == NO_TYPE) {
				name_id name              = s->members.m[j].type.unresolved.name;
				s->members.m[j].type.type = find_type_by_name(name);
				if (s->members.m[j].type.type == NO_TYPE) {
					debug_context context = {0};
					error(context, "Could not find type %s in %s", get_name(name), get_name(s->name));
				}
			}
		}
	}

	for (function_id i = 0; get_function(i) != NULL; ++i) {
		function *f = get_function(i);

		for (uint8_t parameter_index = 0; parameter_index < f->parameters_size; ++parameter_index) {
			if (f->parameter_types[parameter_index].type == NO_TYPE) {
				name_id parameter_type_name              = f->parameter_types[parameter_index].unresolved.name;
				f->parameter_types[parameter_index].type = find_type_by_name(parameter_type_name);
				if (f->parameter_types[parameter_index].type == NO_TYPE) {
					debug_context context = {0};
					error(context, "Could not find type %s for %s", get_name(parameter_type_name), get_name(f->name));
				}
			}
		}

		if (f->return_type.type == NO_TYPE) {
			f->return_type.type = find_type_by_ref(&f->return_type);

			if (f->return_type.type == NO_TYPE) {
				error_no_context("Could not find type %s for %s", get_name(f->return_type.unresolved.name), get_name(f->name));
			}
		}
	}

	for (function_id i = 0; get_function(i) != NULL; ++i) {
		function *f = get_function(i);

		if (f->block == NULL) {
			// built in
			continue;
		}

		for (uint8_t parameter_index = 0; parameter_index < f->parameters_size; ++parameter_index) {
			f->block->block.vars.v[f->block->block.vars.size].name        = f->parameter_names[parameter_index];
			f->block->block.vars.v[f->block->block.vars.size].type        = f->parameter_types[parameter_index];
			f->block->block.vars.v[f->block->block.vars.size].variable_id = 0;
			++f->block->block.vars.size;
		}

		resolve_types_in_block(NULL, f->block);
	}
}
