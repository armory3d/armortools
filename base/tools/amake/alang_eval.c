#include "alang.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

typedef union value {
	float f;
	int i;
	bool b;
} value;

typedef struct variable_storage {
	value val;
	type_id type;
} variable_storage;

typedef struct interpreter_state {
	variable_storage vars[1024];
	size_t pos;
} interpreter_state;

static value get_variable_value(interpreter_state *state, variable var) {
	return state->vars[var.index].val;
}

static void set_variable_value(interpreter_state *state, variable var, value v) {
	state->vars[var.index].val = v;
	state->vars[var.index].type = var.type.type;
}

static float eval() {
	interpreter_state state = {0};
	function *main_func = NULL;

	for (function_id i = 0; _get_function(i) != NULL; ++i) {
		function *f = _get_function(i);
		if (strcmp(_get_name(f->name), "main") == 0) {
			main_func = f;
			break;
		}
	}

	for (size_t i = 0; i < __allocated_globals_size; ++i) {
		global *g = __allocated_globals[i].g;
		uint64_t var_index = __allocated_globals[i].variable_id;
		if (g->value.kind == GLOBAL_VALUE_FLOAT) {
			state.vars[var_index].val.f = g->value.value.floats[0];
			state.vars[var_index].type = g->type;
		}
		else if (g->value.kind == GLOBAL_VALUE_INT) {
			state.vars[var_index].val.i = g->value.value.ints[0];
			state.vars[var_index].type = g->type;
		}
		else if (g->value.kind == GLOBAL_VALUE_BOOL) {
			state.vars[var_index].val.b = g->value.value.b;
			state.vars[var_index].type = g->type;
		}
	}

	state.pos = 0;
	opcodes *code = &main_func->code;

	while (state.pos < code->size) {
		opcode *op = (opcode *)&code->o[state.pos];
		state.pos += op->size;

		switch (op->type) {
		case OPCODE_VAR: {
			break;
		}
		case OPCODE_LOAD_FLOAT_CONSTANT: {
			value v;
			v.f = op->op_load_float_constant.number;
			set_variable_value(&state, op->op_load_float_constant.to, v);
			break;
		}
		case OPCODE_LOAD_INT_CONSTANT: {
			value v;
			v.i = op->op_load_int_constant.number;
			set_variable_value(&state, op->op_load_int_constant.to, v);
			break;
		}
		case OPCODE_LOAD_BOOL_CONSTANT: {
			value v;
			v.b = op->op_load_bool_constant.boolean;
			set_variable_value(&state, op->op_load_bool_constant.to, v);
			break;
		}
		case OPCODE_STORE_VARIABLE: {
			value v = get_variable_value(&state, op->op_store_var.from);
			set_variable_value(&state, op->op_store_var.to, v);
			break;
		}
		case OPCODE_ADD: {
			value left = get_variable_value(&state, op->op_binary.left);
			value right = get_variable_value(&state, op->op_binary.right);
			value result;
			type_id result_type = op->op_binary.result.type.type;
			if (result_type == _float_id) {
				float l = (op->op_binary.left.type.type == _float_id) ? left.f : (float)left.i;
				float r = (op->op_binary.right.type.type == _float_id) ? right.f : (float)right.i;
				result.f = l + r;
			}
			else {
				result.i = left.i + right.i;
			}
			set_variable_value(&state, op->op_binary.result, result);
			break;
		}
		case OPCODE_SUB: {
			value left = get_variable_value(&state, op->op_binary.left);
			value right = get_variable_value(&state, op->op_binary.right);
			value result;
			type_id result_type = op->op_binary.result.type.type;
			if (result_type == _float_id) {
				float l = (op->op_binary.left.type.type == _float_id) ? left.f : (float)left.i;
				float r = (op->op_binary.right.type.type == _float_id) ? right.f : (float)right.i;
				result.f = l - r;
			}
			else {
				result.i = left.i - right.i;
			}
			set_variable_value(&state, op->op_binary.result, result);
			break;
		}
		case OPCODE_MULTIPLY: {
			value left = get_variable_value(&state, op->op_binary.left);
			value right = get_variable_value(&state, op->op_binary.right);
			value result;
			type_id result_type = op->op_binary.result.type.type;
			if (result_type == _float_id) {
				float l = (op->op_binary.left.type.type == _float_id) ? left.f : (float)left.i;
				float r = (op->op_binary.right.type.type == _float_id) ? right.f : (float)right.i;
				result.f = l * r;
			}
			else {
				result.i = left.i * right.i;
			}
			set_variable_value(&state, op->op_binary.result, result);
			break;
		}
		case OPCODE_DIVIDE: {
			value left = get_variable_value(&state, op->op_binary.left);
			value right = get_variable_value(&state, op->op_binary.right);
			value result;
			type_id result_type = op->op_binary.result.type.type;
			if (result_type == _float_id) {
				float l = (op->op_binary.left.type.type == _float_id) ? left.f : (float)left.i;
				float r = (op->op_binary.right.type.type == _float_id) ? right.f : (float)right.i;
				result.f = l / r;
			}
			else {
				result.i = left.i / right.i;
			}
			set_variable_value(&state, op->op_binary.result, result);
			break;
		}
		case OPCODE_MOD: {
			value left = get_variable_value(&state, op->op_binary.left);
			value right = get_variable_value(&state, op->op_binary.right);
			value result;
			result.i = left.i % right.i;
			set_variable_value(&state, op->op_binary.result, result);
			break;
		}
		case OPCODE_EQUALS: {
			value left = get_variable_value(&state, op->op_binary.left);
			value right = get_variable_value(&state, op->op_binary.right);
			value result;
			type_id left_type = op->op_binary.left.type.type;
			type_id right_type = op->op_binary.right.type.type;
			if (left_type == _float_id || right_type == _float_id) {
				float l = (left_type == _float_id) ? left.f : (float)left.i;
				float r = (right_type == _float_id) ? right.f : (float)right.i;
				result.b = l == r;
			}
			else if (left_type == _bool_id && right_type == _bool_id) {
				result.b = left.b == right.b;
			}
			else {
				result.b = left.i == right.i;
			}
			set_variable_value(&state, op->op_binary.result, result);
			break;
		}
		case OPCODE_NOT_EQUALS: {
			value left = get_variable_value(&state, op->op_binary.left);
			value right = get_variable_value(&state, op->op_binary.right);
			value result;
			type_id left_type = op->op_binary.left.type.type;
			type_id right_type = op->op_binary.right.type.type;
			if (left_type == _float_id || right_type == _float_id) {
				float l = (left_type == _float_id) ? left.f : (float)left.i;
				float r = (right_type == _float_id) ? right.f : (float)right.i;
				result.b = l != r;
			}
			else if (left_type == _bool_id && right_type == _bool_id) {
				result.b = left.b != right.b;
			}
			else {
				result.b = left.i != right.i;
			}
			set_variable_value(&state, op->op_binary.result, result);
			break;
		}
		case OPCODE_GREATER: {
			value left = get_variable_value(&state, op->op_binary.left);
			value right = get_variable_value(&state, op->op_binary.right);
			value result;
			type_id left_type = op->op_binary.left.type.type;
			type_id right_type = op->op_binary.right.type.type;
			if (left_type == _float_id || right_type == _float_id) {
				float l = (left_type == _float_id) ? left.f : (float)left.i;
				float r = (right_type == _float_id) ? right.f : (float)right.i;
				result.b = l > r;
			}
			else {
				result.b = left.i > right.i;
			}
			set_variable_value(&state, op->op_binary.result, result);
			break;
		}
		case OPCODE_GREATER_EQUAL: {
			value left = get_variable_value(&state, op->op_binary.left);
			value right = get_variable_value(&state, op->op_binary.right);
			value result;
			type_id left_type = op->op_binary.left.type.type;
			type_id right_type = op->op_binary.right.type.type;
			if (left_type == _float_id || right_type == _float_id) {
				float l = (left_type == _float_id) ? left.f : (float)left.i;
				float r = (right_type == _float_id) ? right.f : (float)right.i;
				result.b = l >= r;
			}
			else {
				result.b = left.i >= right.i;
			}
			set_variable_value(&state, op->op_binary.result, result);
			break;
		}
		case OPCODE_LESS: {
			value left = get_variable_value(&state, op->op_binary.left);
			value right = get_variable_value(&state, op->op_binary.right);
			value result;
			type_id left_type = op->op_binary.left.type.type;
			type_id right_type = op->op_binary.right.type.type;
			if (left_type == _float_id || right_type == _float_id) {
				float l = (left_type == _float_id) ? left.f : (float)left.i;
				float r = (right_type == _float_id) ? right.f : (float)right.i;
				result.b = l < r;
			}
			else {
				result.b = left.i < right.i;
			}
			set_variable_value(&state, op->op_binary.result, result);
			break;
		}
		case OPCODE_LESS_EQUAL: {
			value left = get_variable_value(&state, op->op_binary.left);
			value right = get_variable_value(&state, op->op_binary.right);
			value result;
			type_id left_type = op->op_binary.left.type.type;
			type_id right_type = op->op_binary.right.type.type;
			if (left_type == _float_id || right_type == _float_id) {
				float l = (left_type == _float_id) ? left.f : (float)left.i;
				float r = (right_type == _float_id) ? right.f : (float)right.i;
				result.b = l <= r;
			}
			else {
				result.b = left.i <= right.i;
			}
			set_variable_value(&state, op->op_binary.result, result);
			break;
		}
		case OPCODE_AND: {
			value left = get_variable_value(&state, op->op_binary.left);
			value right = get_variable_value(&state, op->op_binary.right);
			value result;
			result.b = left.b && right.b;
			set_variable_value(&state, op->op_binary.result, result);
			break;
		}
		case OPCODE_OR: {
			value left = get_variable_value(&state, op->op_binary.left);
			value right = get_variable_value(&state, op->op_binary.right);
			value result;
			result.b = left.b || right.b;
			set_variable_value(&state, op->op_binary.result, result);
			break;
		}
		case OPCODE_BITWISE_XOR: {
			value left = get_variable_value(&state, op->op_binary.left);
			value right = get_variable_value(&state, op->op_binary.right);
			value result;
			result.i = left.i ^ right.i;
			set_variable_value(&state, op->op_binary.result, result);
			break;
		}
		case OPCODE_BITWISE_AND: {
			value left = get_variable_value(&state, op->op_binary.left);
			value right = get_variable_value(&state, op->op_binary.right);
			value result;
			result.i = left.i & right.i;
			set_variable_value(&state, op->op_binary.result, result);
			break;
		}
		case OPCODE_BITWISE_OR: {
			value left = get_variable_value(&state, op->op_binary.left);
			value right = get_variable_value(&state, op->op_binary.right);
			value result;
			result.i = left.i | right.i;
			set_variable_value(&state, op->op_binary.result, result);
			break;
		}
		case OPCODE_LEFT_SHIFT: {
			value left = get_variable_value(&state, op->op_binary.left);
			value right = get_variable_value(&state, op->op_binary.right);
			value result;
			result.i = left.i << right.i;
			set_variable_value(&state, op->op_binary.result, result);
			break;
		}
		case OPCODE_RIGHT_SHIFT: {
			value left = get_variable_value(&state, op->op_binary.left);
			value right = get_variable_value(&state, op->op_binary.right);
			value result;
			result.i = left.i >> right.i;
			set_variable_value(&state, op->op_binary.result, result);
			break;
		}
		case OPCODE_NOT: {
			value from = get_variable_value(&state, op->op_not.from);
			value result;
			result.b = !from.b;
			set_variable_value(&state, op->op_not.to, result);
			break;
		}
		case OPCODE_NEGATE: {
			value from = get_variable_value(&state, op->op_negate.from);
			value result;
			if (op->op_negate.from.type.type == _float_id) {
				result.f = -from.f;
			}
			else {
				result.i = -from.i;
			}
			set_variable_value(&state, op->op_negate.to, result);
			break;
		}
		case OPCODE_IF: {
			value condition = get_variable_value(&state, op->op_if.condition);
			if (!condition.b) {
				state.pos = op->op_if.end_id;
			}
			break;
		}
		case OPCODE_WHILE_START: {
			break;
		}
		case OPCODE_WHILE_CONDITION: {
			value condition = get_variable_value(&state, op->op_while.condition);
			if (!condition.b) {
				state.pos = op->op_while.end_id;
			}
			break;
		}
		case OPCODE_WHILE_END: {
			state.pos = op->op_while_end.start_id;
			break;
		}
		case OPCODE_BLOCK_START:
		case OPCODE_BLOCK_END: {
			break;
		}
		case OPCODE_RETURN: {
			if (op->op_return.var.index != 0) {
				value return_val = get_variable_value(&state, op->op_return.var);
				return return_val.f;
			}
			return 0.0;
		}
		case OPCODE_CALL: {
			if (strcmp(_get_name(op->op_call.func), "print") == 0) {
				value param = get_variable_value(&state, op->op_call.parameters[0]);
				value result;
				printf("%f\n", param.f);
				set_variable_value(&state, op->op_call.var, result);
			}
			break;
		}
		default: {
			break;
		}
		}
	}

	return 0.0;
}

float alang_eval(char *data) {
	_names_init();
	_types_init();
	_functions_init();
	_globals_init();

	char buffer[2048];
	strcpy(buffer, "fun main(): float { return ");
	strcat(buffer, data);
	strcat(buffer, "; }");

	char *filename = "main.kong";
	tokens tokens = _tokenize(filename, buffer);
	_parse(filename, &tokens);
	_resolve_types();
	// allocate_globals();
	for (function_id i = 0; _get_function(i) != NULL; ++i) {
		_compile_function_block(&_get_function(i)->code, _get_function(i)->block);
	}

	return eval(buffer);
}
