#include "alang.h"

static void _indent(FILE *out, int indent) {
	for (int i = 0; i < indent; ++i) {
		fprintf(out, "	");
	}
}

static const char *to_c_type(type_id type) {
	if (type == float_id) return "f32";
	if (type == int_id) return "i32";
	if (type == uint_id) return "u32";
	if (type == bool_id) return "bool";
	return "void";
}

void alang_c(const char *output) {
	FILE *out = fopen(output, "w");
	fprintf(out, "#include <stdbool.h>\n");
	fprintf(out, "#include <stdio.h>\n");
	fprintf(out, "#include <math.h>\n\n");

	function *main_func = NULL;
	for (function_id i = 0; get_function(i) != NULL; ++i) {
		function *f = get_function(i);
		if (strcmp(get_name(f->name), "main") == 0) {
			main_func = f;
			break;
		}
	}

	for (size_t i = 0; i < allocated_globals_size; ++i) {
		global *g = allocated_globals[i].g;
		fprintf(out, "%s var_%llu = ", to_c_type(g->type), allocated_globals[i].variable_id);
		if (g->value.kind == GLOBAL_VALUE_FLOAT) {
			fprintf(out, "%ff;\n", g->value.value.floats[0]);
		}
		else if (g->value.kind == GLOBAL_VALUE_INT) {
			fprintf(out, "%d;\n", g->value.value.ints[0]);
		}
		else if (g->value.kind == GLOBAL_VALUE_BOOL) {
			fprintf(out, "%s;\n", g->value.value.b ? "true" : "false");
		}
	}
	if (allocated_globals_size > 0) {
		fprintf(out, "\n");
	}

	fprintf(out, "int main() {\n");

	for (size_t i = 0; i < main_func->block->block.vars.size; ++i) {
		local_variable *var = &main_func->block->block.vars.v[i];
		_indent(out, 1);
		fprintf(out, "%s var_%llu;\n", to_c_type(var->type.type), var->variable_id);
	}

	size_t pos = 0;
	int indent = 1;
	while (pos < main_func->code.size) {
		opcode *op = (opcode *)&main_func->code.o[pos];
		pos += op->size;

		_indent(out, indent);
		switch (op->type) {
		case OPCODE_VAR: {
			break;
		}
		case OPCODE_LOAD_FLOAT_CONSTANT: {
			fprintf(out, "%s var_%llu = %ff;\n",
					to_c_type(op->op_load_float_constant.to.type.type),
					op->op_load_float_constant.to.index,
					op->op_load_float_constant.number);
			break;
		}
		case OPCODE_LOAD_INT_CONSTANT: {
			fprintf(out, "%s var_%llu = %d;\n",
					to_c_type(op->op_load_int_constant.to.type.type),
					op->op_load_int_constant.to.index,
					op->op_load_int_constant.number);
			break;
		}
		case OPCODE_LOAD_BOOL_CONSTANT: {
			fprintf(out, "%s var_%llu = %s;\n",
					to_c_type(op->op_load_bool_constant.to.type.type),
					op->op_load_bool_constant.to.index,
					op->op_load_bool_constant.boolean ? "true" : "false");
			break;
		}
		case OPCODE_STORE_VARIABLE: {
			fprintf(out, "var_%llu = ", op->op_store_var.to.index);
			fprintf(out, "var_%llu;\n", op->op_store_var.from.index);
			break;
		}
		case OPCODE_ADD:
		case OPCODE_SUB:
		case OPCODE_MULTIPLY:
		case OPCODE_DIVIDE:
		case OPCODE_MOD:
		case OPCODE_EQUALS:
		case OPCODE_NOT_EQUALS:
		case OPCODE_GREATER:
		case OPCODE_GREATER_EQUAL:
		case OPCODE_LESS:
		case OPCODE_LESS_EQUAL:
		case OPCODE_AND:
		case OPCODE_OR:
		case OPCODE_BITWISE_XOR:
		case OPCODE_BITWISE_AND:
		case OPCODE_BITWISE_OR:
		case OPCODE_LEFT_SHIFT:
		case OPCODE_RIGHT_SHIFT: {
			const char *op_str = "";
			switch (op->type) {
				case OPCODE_ADD: op_str = "+"; break;
				case OPCODE_SUB: op_str = "-"; break;
				case OPCODE_MULTIPLY: op_str = "*"; break;
				case OPCODE_DIVIDE: op_str = "/"; break;
				case OPCODE_MOD: op_str = "%"; break;
				case OPCODE_EQUALS: op_str = "=="; break;
				case OPCODE_NOT_EQUALS: op_str = "!="; break;
				case OPCODE_GREATER: op_str = ">"; break;
				case OPCODE_GREATER_EQUAL: op_str = ">="; break;
				case OPCODE_LESS: op_str = "<"; break;
				case OPCODE_LESS_EQUAL: op_str = "<="; break;
				case OPCODE_AND: op_str = "&&"; break;
				case OPCODE_OR: op_str = "||"; break;
				case OPCODE_BITWISE_XOR: op_str = "^"; break;
				case OPCODE_BITWISE_AND: op_str = "&"; break;
				case OPCODE_BITWISE_OR: op_str = "|"; break;
				case OPCODE_LEFT_SHIFT: op_str = "<<"; break;
				case OPCODE_RIGHT_SHIFT: op_str = ">>"; break;
				default: break;
			}
			fprintf(out, "%s var_%llu = ",
					to_c_type(op->op_binary.result.type.type),
					op->op_binary.result.index);
			fprintf(out, "var_%llu", op->op_binary.left.index);
			fprintf(out, " %s ", op_str);
			fprintf(out, "var_%llu;\n", op->op_binary.right.index);
			break;
		}
		case OPCODE_NOT: {
			fprintf(out, "%s var_%llu = !",
					to_c_type(op->op_not.to.type.type),
					op->op_not.to.index);
			fprintf(out, "var_%llu;\n", op->op_not.from.index);
			break;
		}
		case OPCODE_NEGATE: {
			fprintf(out, "%s var_%llu = -",
					to_c_type(op->op_negate.to.type.type),
					op->op_negate.to.index);
			fprintf(out, "var_%llu;\n", op->op_negate.from.index);
			break;
		}
		case OPCODE_IF: {
			fprintf(out, "if (");
			fprintf(out, "var_%llu) {\n", op->op_if.condition.index);
			indent++;
			break;
		}
		case OPCODE_WHILE_START: {
			fprintf(out, "while_%llu:;\n", op->op_while_start.start_id);
			break;
		}
		case OPCODE_WHILE_CONDITION: {
			fprintf(out, "if (!");
			fprintf(out, "var_%llu) goto while_%llu;\n",
					op->op_while.condition.index, op->op_while.end_id);
			break;
		}
		case OPCODE_WHILE_END: {
			fprintf(out, "goto while_%llu;\n", op->op_while_end.start_id);
			_indent(out, indent);
			fprintf(out, "while_%llu:;\n", op->op_while_end.end_id);
			break;
		}
		case OPCODE_BLOCK_START: {
			fprintf(out, "{\n");
			indent++;
			break;
		}
		case OPCODE_BLOCK_END: {
			indent--;
			_indent(out, indent);
			fprintf(out, "}\n");
			break;
		}
		case OPCODE_RETURN: {
			if (op->op_return.var.index != 0) {
				fprintf(out, "return ");
				fprintf(out, "var_%llu;\n", op->op_return.var.index);
			}
			else {
				fprintf(out, "return 0;\n");
			}
			break;
		}
		case OPCODE_CALL: {
			if (strcmp(get_name(op->op_call.func), "print") == 0) {
				fprintf(out, "%s var_%llu = %s(",
						to_c_type(op->op_call.var.type.type),
						op->op_call.var.index,
						get_name(op->op_call.func));
				fprintf(out, "var_%llu);\n", op->op_call.parameters[0].index);
			}
			break;
		}
		default: {
			break;
		}
		}
	}

	fprintf(out, "	return 0;\n");
	fprintf(out, "}\n");
	fclose(out);
}
