#include "compiler.h"

#include "errors.h"
#include "parser.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

typedef struct allocated_global {
	global  *g;
	uint64_t variable_id;
} allocated_global;

static allocated_global allocated_globals[1024];
////
// static size_t           allocated_globals_size = 0;
size_t                  allocated_globals_size = 0;
////

allocated_global find_allocated_global(name_id name) {
	for (size_t i = 0; i < allocated_globals_size; ++i) {
		if (name == allocated_globals[i].g->name) {
			return allocated_globals[i];
		}
	}

	allocated_global a;
	a.g           = NULL;
	a.variable_id = 0;
	return a;
}

variable find_local_var(block *b, name_id name) {
	if (b == NULL) {
		variable var;
		var.index = 0;
		init_type_ref(&var.type, NO_NAME);
		return var;
	}

	for (size_t i = 0; i < b->vars.size; ++i) {
		if (b->vars.v[i].name == name) {
			debug_context context = {0};
			check(b->vars.v[i].type.type != NO_TYPE, context, "Local variable does not have a type");
			variable var;
			var.index = b->vars.v[i].variable_id;
			var.type  = b->vars.v[i].type;
			var.kind  = VARIABLE_LOCAL;
			return var;
		}
	}

	return find_local_var(b->parent, name);
}

variable find_variable(block *parent, name_id name) {
	variable local_var = find_local_var(parent, name);
	if (local_var.index == 0) {
		allocated_global global = find_allocated_global(name);
		if (global.g->type != NO_TYPE && global.variable_id != 0) {
			variable v;
			init_type_ref(&v.type, NO_NAME);
			v.type.type = global.g->type;
			v.index     = global.variable_id;
			v.kind      = VARIABLE_GLOBAL;
			return v;
		}
		else {
			debug_context context = {0};
			error(context, "Variable %s not found", get_name(name));

			variable v;
			v.index = 0;
			return v;
		}
	}
	else {
		return local_var;
	}
}

const char all_names[1024 * 1024];

////
// static uint64_t next_variable_id = 1;
uint64_t next_variable_id = 1;
////

variable all_variables[1024 * 1024];

variable allocate_variable(type_ref type, variable_kind kind) {
	variable v;
	v.index                = next_variable_id;
	v.type                 = type;
	v.kind                 = kind;
	all_variables[v.index] = v;
	++next_variable_id;
	return v;
}

opcode *emit_op(opcodes *code, opcode *o) {
	assert(code->size + o->size < OPCODES_SIZE);

	uint8_t *location = &code->o[code->size];

	memcpy(&code->o[code->size], o, o->size);

	code->size += o->size;

	return (opcode *)location;
}

variable emit_expression(opcodes *code, block *parent, expression *e) {
	switch (e->kind) {
	case EXPRESSION_BINARY: {
		expression *left  = e->binary.left;
		expression *right = e->binary.right;

		debug_context context = {0};

		switch (e->binary.op) {
		case OPERATOR_EQUALS:
		case OPERATOR_NOT_EQUALS:
		case OPERATOR_GREATER:
		case OPERATOR_GREATER_EQUAL:
		case OPERATOR_LESS:
		case OPERATOR_LESS_EQUAL:
		case OPERATOR_AND:
		case OPERATOR_OR: {
			variable right_var = emit_expression(code, parent, right);
			variable left_var  = emit_expression(code, parent, left);
			type_ref t;
			init_type_ref(&t, NO_NAME);
			t.type              = bool_id;
			variable result_var = allocate_variable(t, VARIABLE_INTERNAL);

			opcode o;
			switch (e->binary.op) {
			case OPERATOR_EQUALS:
				o.type = OPCODE_EQUALS;
				break;
			case OPERATOR_NOT_EQUALS:
				o.type = OPCODE_NOT_EQUALS;
				break;
			case OPERATOR_GREATER:
				o.type = OPCODE_GREATER;
				break;
			case OPERATOR_GREATER_EQUAL:
				o.type = OPCODE_GREATER_EQUAL;
				break;
			case OPERATOR_LESS:
				o.type = OPCODE_LESS;
				break;
			case OPERATOR_LESS_EQUAL:
				o.type = OPCODE_LESS_EQUAL;
				break;
			case OPERATOR_AND:
				o.type = OPCODE_AND;
				break;
			case OPERATOR_OR:
				o.type = OPCODE_OR;
				break;
			default: {
				error(context, "Unexpected operator");
			}
			}
			o.size             = OP_SIZE(o, op_binary);
			o.op_binary.right  = right_var;
			o.op_binary.left   = left_var;
			o.op_binary.result = result_var;
			emit_op(code, &o);

			return result_var;
		}
		case OPERATOR_MINUS:
		case OPERATOR_PLUS:
		case OPERATOR_DIVIDE:
		case OPERATOR_MULTIPLY:
		case OPERATOR_MOD:
		case OPERATOR_BITWISE_XOR:
		case OPERATOR_BITWISE_AND:
		case OPERATOR_BITWISE_OR:
		case OPERATOR_LEFT_SHIFT:
		case OPERATOR_RIGHT_SHIFT: {
			variable right_var  = emit_expression(code, parent, right);
			variable left_var   = emit_expression(code, parent, left);
			variable result_var = allocate_variable(e->type, VARIABLE_INTERNAL);

			opcode o;
			switch (e->binary.op) {
			case OPERATOR_MINUS:
				o.type = OPCODE_SUB;
				break;
			case OPERATOR_PLUS:
				o.type = OPCODE_ADD;
				break;
			case OPERATOR_DIVIDE:
				o.type = OPCODE_DIVIDE;
				break;
			case OPERATOR_MULTIPLY:
				o.type = OPCODE_MULTIPLY;
				break;
			case OPERATOR_MOD:
				o.type = OPCODE_MOD;
				break;
			case OPERATOR_BITWISE_XOR:
				o.type = OPCODE_BITWISE_XOR;
				break;
			case OPERATOR_BITWISE_AND:
				o.type = OPCODE_BITWISE_AND;
				break;
			case OPERATOR_BITWISE_OR:
				o.type = OPCODE_BITWISE_OR;
				break;
			case OPERATOR_LEFT_SHIFT:
				o.type = OPCODE_LEFT_SHIFT;
				break;
			case OPERATOR_RIGHT_SHIFT:
				o.type = OPCODE_RIGHT_SHIFT;
				break;
			default: {
				error(context, "Unexpected operator");
			}
			}
			o.size             = OP_SIZE(o, op_binary);
			o.op_binary.right  = right_var;
			o.op_binary.left   = left_var;
			o.op_binary.result = result_var;
			emit_op(code, &o);

			return result_var;
		}
		case OPERATOR_NOT: {
			error(context, "! is not a binary operator");
		}
		case OPERATOR_ASSIGN:
		case OPERATOR_MINUS_ASSIGN:
		case OPERATOR_PLUS_ASSIGN:
		case OPERATOR_DIVIDE_ASSIGN:
		case OPERATOR_MULTIPLY_ASSIGN: {
			variable v = emit_expression(code, parent, right);

			switch (left->kind) {
			case EXPRESSION_VARIABLE: {
				opcode o;
				switch (e->binary.op) {
				case OPERATOR_ASSIGN:
					o.type = OPCODE_STORE_VARIABLE;
					break;
				case OPERATOR_MINUS_ASSIGN:
					o.type = OPCODE_SUB_AND_STORE_VARIABLE;
					break;
				case OPERATOR_PLUS_ASSIGN:
					o.type = OPCODE_ADD_AND_STORE_VARIABLE;
					break;
				case OPERATOR_DIVIDE_ASSIGN:
					o.type = OPCODE_DIVIDE_AND_STORE_VARIABLE;
					break;
				case OPERATOR_MULTIPLY_ASSIGN:
					o.type = OPCODE_MULTIPLY_AND_STORE_VARIABLE;
					break;
				default: {
					error(context, "Unexpected operator");
				}
				}
				o.size              = OP_SIZE(o, op_store_var);
				o.op_store_var.from = v;
				o.op_store_var.to   = find_variable(parent, left->variable);
				emit_op(code, &o);
				break;
			}
			case EXPRESSION_ELEMENT:
			case EXPRESSION_MEMBER:
			case EXPRESSION_SWIZZLE: {
				opcode o;
				switch (e->binary.op) {
				case OPERATOR_ASSIGN:
					o.type = OPCODE_STORE_ACCESS_LIST;
					break;
				case OPERATOR_MINUS_ASSIGN:
					o.type = OPCODE_SUB_AND_STORE_ACCESS_LIST;
					break;
				case OPERATOR_PLUS_ASSIGN:
					o.type = OPCODE_ADD_AND_STORE_ACCESS_LIST;
					break;
				case OPERATOR_DIVIDE_ASSIGN:
					o.type = OPCODE_DIVIDE_AND_STORE_ACCESS_LIST;
					break;
				case OPERATOR_MULTIPLY_ASSIGN:
					o.type = OPCODE_MULTIPLY_AND_STORE_ACCESS_LIST;
					break;
				default: {
					error(context, "Unexpected operator");
				}
				}
				o.size                      = OP_SIZE(o, op_store_access_list);
				o.op_store_access_list.from = v;

				expression *of = left;

				access   access_list[64];
				uint32_t access_list_size = 0;

				while (of->kind == EXPRESSION_ELEMENT || of->kind == EXPRESSION_MEMBER || of->kind == EXPRESSION_SWIZZLE) {
					access *a = &access_list[access_list_size];
					a->type   = of->type.type;

					switch (of->kind) {
					case EXPRESSION_ELEMENT:
						a->kind                 = ACCESS_ELEMENT;
						a->access_element.index = emit_expression(code, parent, of->element.element_index);

						of = of->element.of;

						break;
					case EXPRESSION_MEMBER:
						a->kind               = ACCESS_MEMBER;
						a->access_member.name = of->member.member_name;

						of = of->member.of;

						break;
					case EXPRESSION_SWIZZLE:
						a->kind                   = ACCESS_SWIZZLE;
						a->access_swizzle.swizzle = of->swizzle.swizz;

						of = of->swizzle.of;

						break;
					default:
						assert(false);
						break;
					}

					access_list_size += 1;
				}

				o.op_store_access_list.access_list_size = access_list_size;

				for (uint32_t access_index = 0; access_index < access_list_size; ++access_index) {
					o.op_store_access_list.access_list[access_list_size - access_index - 1] = access_list[access_index];
				}

				o.op_store_access_list.to = emit_expression(code, parent, of);

				emit_op(code, &o);
				break;
			}
			default: {
				debug_context context = {0};
				error(context, "Expected a variable or an access");
			}
			}

			return v;
		}
		}
		break;
	}
	case EXPRESSION_UNARY: {
		debug_context context = {0};
		switch (e->unary.op) {
		case OPERATOR_EQUALS:
			error(context, "not implemented");
		case OPERATOR_NOT_EQUALS:
			error(context, "not implemented");
		case OPERATOR_GREATER:
			error(context, "not implemented");
		case OPERATOR_GREATER_EQUAL:
			error(context, "not implemented");
		case OPERATOR_LESS:
			error(context, "not implemented");
		case OPERATOR_LESS_EQUAL:
			error(context, "not implemented");
		case OPERATOR_MINUS: {
			variable v = emit_expression(code, parent, e->unary.right);
			opcode   o;
			o.type           = OPCODE_NEGATE;
			o.size           = OP_SIZE(o, op_negate);
			o.op_negate.from = v;
			o.op_negate.to   = allocate_variable(v.type, VARIABLE_INTERNAL);
			emit_op(code, &o);
			return o.op_negate.to;
		}
		case OPERATOR_PLUS:
			error(context, "not implemented");
		case OPERATOR_DIVIDE:
			error(context, "not implemented");
		case OPERATOR_MULTIPLY:
			error(context, "not implemented");
		case OPERATOR_NOT: {
			variable v = emit_expression(code, parent, e->unary.right);
			opcode   o;
			o.type        = OPCODE_NOT;
			o.size        = OP_SIZE(o, op_not);
			o.op_not.from = v;
			o.op_not.to   = allocate_variable(v.type, VARIABLE_INTERNAL);
			emit_op(code, &o);
			return o.op_not.to;
		}
		case OPERATOR_OR:
			error(context, "not implemented");
		case OPERATOR_BITWISE_XOR:
			error(context, "not implemented");
		case OPERATOR_BITWISE_AND:
			error(context, "not implemented");
		case OPERATOR_BITWISE_OR:
			error(context, "not implemented");
		case OPERATOR_LEFT_SHIFT:
			error(context, "not implemented");
		case OPERATOR_RIGHT_SHIFT:
			error(context, "not implemented");
		case OPERATOR_AND:
			error(context, "not implemented");
		case OPERATOR_MOD:
			error(context, "not implemented");
		case OPERATOR_ASSIGN:
			error(context, "not implemented");
		case OPERATOR_PLUS_ASSIGN:
		case OPERATOR_MINUS_ASSIGN:
		case OPERATOR_MULTIPLY_ASSIGN:
		case OPERATOR_DIVIDE_ASSIGN:
			error(context, "not implemented");
		}
	}
	case EXPRESSION_BOOLEAN: {
		type_ref t;
		init_type_ref(&t, NO_NAME);
		t.type     = float_id;
		variable v = allocate_variable(t, VARIABLE_INTERNAL);

		opcode o;
		o.type                          = OPCODE_LOAD_BOOL_CONSTANT;
		o.size                          = OP_SIZE(o, op_load_bool_constant);
		o.op_load_bool_constant.boolean = e->boolean;
		o.op_load_bool_constant.to      = v;
		emit_op(code, &o);

		return v;
	}
	case EXPRESSION_FLOAT: {
		type_ref t;
		init_type_ref(&t, NO_NAME);
		t.type     = float_id;
		variable v = allocate_variable(t, VARIABLE_INTERNAL);

		opcode o;
		o.type                          = OPCODE_LOAD_FLOAT_CONSTANT;
		o.size                          = OP_SIZE(o, op_load_float_constant);
		o.op_load_float_constant.number = (float)e->number;
		o.op_load_float_constant.to     = v;
		emit_op(code, &o);

		return v;
	}
	case EXPRESSION_INT: {
		type_ref t;
		init_type_ref(&t, NO_NAME);
		t.type     = int_id;
		variable v = allocate_variable(t, VARIABLE_INTERNAL);

		opcode o;
		o.type                        = OPCODE_LOAD_INT_CONSTANT;
		o.size                        = OP_SIZE(o, op_load_float_constant);
		o.op_load_int_constant.number = (int)e->number;
		o.op_load_int_constant.to     = v;
		emit_op(code, &o);

		return v;
	}
	// case EXPRESSION_STRING:
	//	error("not implemented", 0, 0);
	case EXPRESSION_VARIABLE: {
		return find_variable(parent, e->variable);
	}
	case EXPRESSION_GROUPING: {
		return emit_expression(code, parent, e->grouping);
	}
	case EXPRESSION_CALL: {
		type_ref t;
		init_type_ref(&t, NO_NAME);
		t.type     = e->type.type;
		variable v = allocate_variable(t, VARIABLE_INTERNAL);

		opcode o;
		o.type         = OPCODE_CALL;
		o.size         = OP_SIZE(o, op_call);
		o.op_call.func = e->call.func_name;
		o.op_call.var  = v;

		debug_context context = {0};
		check(e->call.parameters.size <= sizeof(o.op_call.parameters) / sizeof(variable), context, "Call parameters missized");
		for (size_t i = 0; i < e->call.parameters.size; ++i) {
			o.op_call.parameters[i] = emit_expression(code, parent, e->call.parameters.e[i]);
		}
		o.op_call.parameters_size = (uint8_t)e->call.parameters.size;

		emit_op(code, &o);

		return v;
	}
	case EXPRESSION_ELEMENT:
	case EXPRESSION_MEMBER:
	case EXPRESSION_SWIZZLE: {
		opcode o;
		o.type = OPCODE_LOAD_ACCESS_LIST;
		o.size = OP_SIZE(o, op_load_access_list);

		variable v               = allocate_variable(e->type, VARIABLE_INTERNAL);
		o.op_load_access_list.to = v;

		expression *of = e;

		access   access_list[64];
		uint32_t access_list_size = 0;

		while (of->kind == EXPRESSION_ELEMENT || of->kind == EXPRESSION_MEMBER || of->kind == EXPRESSION_SWIZZLE) {
			access *a = &access_list[access_list_size];
			a->type   = of->type.type;

			switch (of->kind) {
			case EXPRESSION_ELEMENT:
				a->kind                 = ACCESS_ELEMENT;
				a->access_element.index = emit_expression(code, parent, of->element.element_index);

				of = of->element.of;

				break;
			case EXPRESSION_MEMBER:
				a->kind               = ACCESS_MEMBER;
				a->access_member.name = of->member.member_name;

				of = of->member.of;

				break;
			case EXPRESSION_SWIZZLE:
				a->kind                   = ACCESS_SWIZZLE;
				a->access_swizzle.swizzle = of->swizzle.swizz;

				of = of->swizzle.of;

				break;
			default:
				assert(false);
				break;
			}

			access_list_size += 1;
		}

		o.op_load_access_list.access_list_size = access_list_size;

		for (uint32_t access_index = 0; access_index < access_list_size; ++access_index) {
			o.op_load_access_list.access_list[access_list_size - access_index - 1] = access_list[access_index];
		}

		o.op_load_access_list.from = emit_expression(code, parent, of);

		emit_op(code, &o);

		return v;
	}
	default: {
		debug_context context = {0};
		error(context, "not implemented");
	}
	}

	{
		debug_context context = {0};
		error(context, "Supposedly unreachable code reached");
		variable v;
		v.index = 0;
		return v;
	}
}

typedef struct block_ids {
	uint64_t start;
	uint64_t end;
} block_ids;

static block_ids emit_statement(opcodes *code, block *parent, statement *statement) {
	switch (statement->kind) {
	case STATEMENT_EXPRESSION:
		emit_expression(code, parent, statement->expression);
		break;
	case STATEMENT_RETURN_EXPRESSION: {
		opcode o;
		o.type     = OPCODE_RETURN;
		variable v = emit_expression(code, parent, statement->expression);
		if (v.index == 0) {
			o.size = offsetof(opcode, op_return);
		}
		else {
			o.size          = OP_SIZE(o, op_return);
			o.op_return.var = v;
		}
		emit_op(code, &o);
		break;
	}
	case STATEMENT_DISCARD: {
		opcode o;
		o.type = OPCODE_DISCARD;
		o.size = offsetof(opcode, op_nothing);

		emit_op(code, &o);
		break;
	}
	case STATEMENT_IF: {
		struct previous_condition {
			variable condition;
			variable summed_condition;
		};
		struct previous_condition previous_conditions[64]  = {0};
		uint8_t                   previous_conditions_size = 0;

		{
			opcode o;
			o.type = OPCODE_IF;
			o.size = OP_SIZE(o, op_if);

			variable initial_condition = emit_expression(code, parent, statement->iffy.test);

			o.op_if.condition = initial_condition;

			opcode *written_opcode = emit_op(code, &o);

			previous_conditions[previous_conditions_size].condition = initial_condition;
			previous_conditions_size += 1;

			block_ids ids = emit_statement(code, parent, statement->iffy.if_block);

			written_opcode->op_if.start_id = ids.start;
			written_opcode->op_if.end_id   = ids.end;
		}

		for (uint16_t i = 0; i < statement->iffy.else_size; ++i) {
			variable current_condition;
			{
				opcode o;
				o.type        = OPCODE_NOT;
				o.size        = OP_SIZE(o, op_not);
				o.op_not.from = previous_conditions[previous_conditions_size - 1].condition;
				type_ref t;
				init_type_ref(&t, NO_NAME);
				t.type            = bool_id;
				current_condition = allocate_variable(t, VARIABLE_INTERNAL);
				o.op_not.to       = current_condition;
				emit_op(code, &o);
			}

			variable summed_condition;
			if (previous_conditions_size == 1) {
				summed_condition = previous_conditions[0].summed_condition = current_condition;
			}
			else {
				opcode o;
				o.type            = OPCODE_AND;
				o.size            = OP_SIZE(o, op_binary);
				o.op_binary.left  = previous_conditions[previous_conditions_size - 2].summed_condition;
				o.op_binary.right = current_condition;
				type_ref t;
				init_type_ref(&t, NO_NAME);
				t.type             = bool_id;
				summed_condition   = allocate_variable(t, VARIABLE_INTERNAL);
				o.op_binary.result = summed_condition;
				emit_op(code, &o);
			}

			opcode o;
			o.type = OPCODE_IF;
			o.size = OP_SIZE(o, op_if);

			if (statement->iffy.else_tests[i] != NULL) {
				variable v = emit_expression(code, parent, statement->iffy.else_tests[i]);

				variable else_test;
				{
					opcode o;
					o.type            = OPCODE_AND;
					o.size            = OP_SIZE(o, op_binary);
					o.op_binary.left  = summed_condition;
					o.op_binary.right = v;
					type_ref t;
					init_type_ref(&t, NO_NAME);
					t.type             = bool_id;
					else_test          = allocate_variable(t, VARIABLE_INTERNAL);
					o.op_binary.result = else_test;
					emit_op(code, &o);
				}

				o.op_if.condition = else_test;

				previous_conditions[previous_conditions_size].condition = v;
				previous_conditions_size += 1;
			}
			else {
				o.op_if.condition = summed_condition;
			}

			{
				opcode *written_opcode = emit_op(code, &o);

				block_ids ids = emit_statement(code, parent, statement->iffy.else_blocks[i]);

				written_opcode->op_if.start_id = ids.start;
				written_opcode->op_if.end_id   = ids.end;
			}
		}

		break;
	}
	case STATEMENT_WHILE: {
		uint64_t start_id = next_variable_id;
		++next_variable_id;
		uint64_t continue_id = next_variable_id;
		++next_variable_id;
		uint64_t end_id = next_variable_id;
		++next_variable_id;

		{
			opcode o;
			o.type                       = OPCODE_WHILE_START;
			o.op_while_start.start_id    = start_id;
			o.op_while_start.continue_id = continue_id;
			o.op_while_start.end_id      = end_id;
			o.size                       = OP_SIZE(o, op_while_start);
			emit_op(code, &o);
		}

		{
			opcode o;
			o.type = OPCODE_WHILE_CONDITION;
			o.size = OP_SIZE(o, op_while);

			variable v = emit_expression(code, parent, statement->whiley.test);

			o.op_while.condition = v;
			o.op_while.end_id    = end_id;

			emit_op(code, &o);
		}

		emit_statement(code, parent, statement->whiley.while_block);

		{
			opcode o;
			o.type                     = OPCODE_WHILE_END;
			o.op_while_end.start_id    = start_id;
			o.op_while_end.continue_id = continue_id;
			o.op_while_end.end_id      = end_id;
			o.size                     = OP_SIZE(o, op_while_end);
			emit_op(code, &o);
		}

		break;
	}
	case STATEMENT_DO_WHILE: {
		uint64_t start_id = next_variable_id;
		++next_variable_id;
		uint64_t continue_id = next_variable_id;
		++next_variable_id;
		uint64_t end_id = next_variable_id;
		++next_variable_id;

		{
			opcode o;
			o.type                       = OPCODE_WHILE_START;
			o.op_while_start.start_id    = start_id;
			o.op_while_start.continue_id = continue_id;
			o.op_while_start.end_id      = end_id;
			o.size                       = OP_SIZE(o, op_while_start);
			emit_op(code, &o);
		}

		emit_statement(code, parent, statement->whiley.while_block);

		{
			opcode o;
			o.type = OPCODE_WHILE_CONDITION;
			o.size = OP_SIZE(o, op_while);

			variable v = emit_expression(code, parent, statement->whiley.test);

			o.op_while.condition = v;
			o.op_while.end_id    = end_id;

			emit_op(code, &o);
		}

		{
			opcode o;
			o.type                     = OPCODE_WHILE_END;
			o.op_while_end.start_id    = start_id;
			o.op_while_end.continue_id = continue_id;
			o.op_while_end.end_id      = end_id;
			o.size                     = OP_SIZE(o, op_while_end);
			emit_op(code, &o);
		}

		break;
	}
	case STATEMENT_BLOCK: {
		for (size_t i = 0; i < statement->block.vars.size; ++i) {
			variable var                           = allocate_variable(statement->block.vars.v[i].type, VARIABLE_LOCAL);
			statement->block.vars.v[i].variable_id = var.index;
		}

		uint64_t start_block_id = next_variable_id;
		++next_variable_id;

		uint64_t end_block_id = next_variable_id;
		++next_variable_id;

		{
			opcode o;
			o.type        = OPCODE_BLOCK_START;
			o.op_block.id = start_block_id;
			o.size        = OP_SIZE(o, op_block);
			emit_op(code, &o);
		}

		for (size_t i = 0; i < statement->block.statements.size; ++i) {
			emit_statement(code, &statement->block, statement->block.statements.s[i]);
		}

		{
			opcode o;
			o.type        = OPCODE_BLOCK_END;
			o.op_block.id = end_block_id;
			o.size        = OP_SIZE(o, op_block);
			emit_op(code, &o);
		}

		block_ids ids;
		ids.start = start_block_id;
		ids.end   = end_block_id;
		return ids;
	}
	case STATEMENT_LOCAL_VARIABLE: {
		opcode o;
		o.type = OPCODE_VAR;
		o.size = OP_SIZE(o, op_var);

		variable init_var = {0};
		if (statement->local_variable.init != NULL) {
			init_var = emit_expression(code, parent, statement->local_variable.init);
		}

		variable local_var                        = find_local_var(parent, statement->local_variable.var.name);
		statement->local_variable.var.variable_id = local_var.index;
		o.op_var.var.index                        = statement->local_variable.var.variable_id;
		debug_context context                     = {0};
		check(statement->local_variable.var.type.type != NO_TYPE, context, "Local var has no type");
		o.op_var.var.type = statement->local_variable.var.type;
		emit_op(code, &o);

		if (statement->local_variable.init != NULL) {
			opcode o;
			o.type = OPCODE_STORE_VARIABLE;
			o.size = OP_SIZE(o, op_store_var);

			o.op_store_var.from = init_var;
			o.op_store_var.to   = local_var;

			emit_op(code, &o);
		}

		break;
	}
	}

	block_ids ids;
	ids.start = 0;
	ids.end   = 0;
	return ids;
}

void allocate_globals(void) {
	for (global_id i = 0; get_global(i) != NULL && get_global(i)->type != NO_TYPE; ++i) {
		global *g = get_global(i);

		type_ref t;
		init_type_ref(&t, NO_NAME);
		t.type                                                = g->type;
		variable v                                            = allocate_variable(t, VARIABLE_GLOBAL);
		allocated_globals[allocated_globals_size].g           = g;
		allocated_globals[allocated_globals_size].variable_id = v.index;
		allocated_globals_size += 1;

		assign_global_var(i, v.index);
	}
}

void compile_function_block(opcodes *code, struct statement *block) {
	if (block == NULL) {
		// built-in
		return;
	}

	if (block->kind != STATEMENT_BLOCK) {
		debug_context context = {0};
		error(context, "Expected a block");
	}
	for (size_t i = 0; i < block->block.vars.size; ++i) {
		variable var                       = allocate_variable(block->block.vars.v[i].type, VARIABLE_LOCAL);
		block->block.vars.v[i].variable_id = var.index;
	}
	for (size_t i = 0; i < block->block.statements.size; ++i) {
		emit_statement(code, &block->block, block->block.statements.s[i]);
	}
}
