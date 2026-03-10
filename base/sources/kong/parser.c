#include "parser.h"
#include "errors.h"
#include "functions.h"
#include "sets.h"
#include "tokenizer.h"
#include "types.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

////
static statement statements_buffer[8192];
int statement_index = 0;
////

static statement *statement_allocate(void) {
	////
	// statement    *s       = (statement *)malloc(sizeof(statement));
	statement   *s = &statements_buffer[statement_index];
	statement_index++;
	////
	debug_context context = {0};
	check(s != NULL, context, "Could not allocate statement");
	return s;
}

// static void statement_free(statement *statement) {
//	free(statement);
// }

static void statements_init(statements *statements) {
	statements->size = 0;
}

static void statements_add(statements *statements, statement *statement) {
	statements->s[statements->size] = statement;
	statements->size += 1;
}

////
static expression experessions_buffer[8192];
int expression_index = 0;
////

static expression *expression_allocate(void) {
	////
	//expression   *e       = (expression *)malloc(sizeof(expression));
	expression   *e = &experessions_buffer[expression_index];
	expression_index++;
	////
	debug_context context = {0};
	check(e != NULL, context, "Could not allocate expression");
	init_type_ref(&e->type, NO_NAME);
	return e;
}

// static void expression_free(expression *expression) {
//	free(expression);
// }

typedef struct state {
	tokens       *tokens;
	size_t        index;
	debug_context context;
} state_t;

static token current(state_t *state) {
	token token = tokens_get(state->tokens, state->index);
	return token;
}

static void update_debug_context(state_t *state) {
	state->context.column = current(state).column;
	state->context.line   = current(state).line;
}

static void advance_state(state_t *state) {
	state->index += 1;
	update_debug_context(state);
}

static void match_token(state_t *state, int token, const char *error_message) {
	int current_token = current(state).kind;
	if (current_token != token) {
		error(state->context, error_message);
	}
}

static void match_token_identifier(state_t *state) {
	if (current(state).kind != TOKEN_IDENTIFIER) {
		error(state->context, "Expected an identifier");
	}
}

static definition  parse_definition(state_t *state);
static statement  *parse_statement(state_t *state, block *parent_block);
static expression *parse_expression(state_t *state);

void parse(const char *filename, tokens *tokens) {
	state_t state          = {0};
	state.context.filename = filename;
	state.tokens           = tokens;
	state.index            = 0;

	for (;;) {
		token token = current(&state);
		if (token.kind == TOKEN_NONE) {
			return;
		}
		else {
			parse_definition(&state);
		}
	}
}

static statement *parse_block(state_t *state, block *parent_block) {
	match_token(state, TOKEN_LEFT_CURLY, "Expected an opening curly bracket");
	advance_state(state);

	statements statements;
	statements_init(&statements);

	statement *new_block       = statement_allocate();
	new_block->kind            = STATEMENT_BLOCK;
	new_block->block.parent    = parent_block;
	new_block->block.vars.size = 0;

	for (;;) {
		switch (current(state).kind) {
		case TOKEN_RIGHT_CURLY: {
			advance_state(state);
			new_block->block.statements = statements;
			return new_block;
		}
		case TOKEN_NONE: {
			update_debug_context(state);
			error(state->context, "File ended before a block ended");
			return NULL;
		}
		default:
			statements_add(&statements, parse_statement(state, &new_block->block));
			break;
		}
	}
}

typedef enum modifier {
	MODIFIER_IN,
	// Out,
} modifier_t;

typedef struct modifiers {
	modifier_t m[16];
	size_t     size;
} modifiers_t;

// static void modifiers_init(modifiers_t *modifiers) {
//	modifiers->size = 0;
// }

// static void modifiers_add(modifiers_t *modifiers, modifier_t modifier) {
//	modifiers->m[modifiers->size] = modifier;
//	modifiers->size += 1;
// }

static definition parse_struct(state_t *state);
static definition parse_function(state_t *state);
static definition parse_const(state_t *state, attribute_list attributes);

static double attribute_parameter_to_number(name_id attribute_name, name_id parameter_name) {
	if (attribute_name == add_name("topology") && parameter_name == add_name("triangle")) {
		return 0;
	}

	type_id type = find_type_by_name(parameter_name);
	if (type != NO_TYPE) {
		return (double)type;
	}

	debug_context context = {0};
	error(context, "Unknown attribute parameter %s", get_name(parameter_name));
	return 0;
}

static definition parse_definition(state_t *state) {
	attribute_list  attributes = {0};
	descriptor_set *current_sets[64];
	size_t          current_sets_count = 0;

	if (current(state).kind == TOKEN_HASH) {
		advance_state(state);
		match_token(state, TOKEN_LEFT_SQUARE, "Expected left square");
		advance_state(state);

		while (current(state).kind != TOKEN_RIGHT_SQUARE) {
			attribute current_attribute = {0};

			match_token(state, TOKEN_IDENTIFIER, "Expected an identifier");
			current_attribute.name = current(state).identifier;

			if (current_attribute.name == add_name("root_constants")) {
				current_sets[current_sets_count]                                = create_set(current_attribute.name);
				current_attribute.parameters[current_attribute.paramters_count] = current_sets[current_sets_count]->index;
				current_sets_count += 1;
			}

			advance_state(state);

			if (current(state).kind == TOKEN_LEFT_PAREN) {
				advance_state(state);

				while (current(state).kind != TOKEN_RIGHT_PAREN) {
					if (current(state).kind == TOKEN_IDENTIFIER) {
						if (current_attribute.name == add_name("set")) {
							if (current(state).identifier == add_name("root_constants")) {
								debug_context context = {0};
								error(context, "Descriptor set can not be called root_constants");
							}
							current_sets[current_sets_count]                                = create_set(current(state).identifier);
							current_attribute.parameters[current_attribute.paramters_count] = current_sets[current_sets_count]->index;
							current_sets_count += 1;
						}
						else {
							current_attribute.parameters[current_attribute.paramters_count] =
							    attribute_parameter_to_number(current_attribute.name, current(state).identifier);
						}
						current_attribute.paramters_count += 1;
						advance_state(state);
					}
					else if (current(state).kind == TOKEN_FLOAT || current(state).kind == TOKEN_INT) {
						current_attribute.parameters[current_attribute.paramters_count] = current(state).number;
						current_attribute.paramters_count += 1;
						advance_state(state);
					}
					else {
						debug_context context = {0};
						error(context, "Expected an identifier or a number");
					}

					if (current(state).kind != TOKEN_RIGHT_PAREN) {
						match_token(state, TOKEN_COMMA, "Expected a comma");
						advance_state(state);
					}
				}
				advance_state(state);
			}

			attributes.attributes[attributes.attributes_count] = current_attribute;
			attributes.attributes_count += 1;

			if (current(state).kind != TOKEN_RIGHT_SQUARE) {
				match_token(state, TOKEN_COMMA, "Expected a comma");
				advance_state(state);
			}
		}
		advance_state(state);
	}

	switch (current(state).kind) {
	case TOKEN_STRUCT: {
		if (current_sets_count != 0) {
			debug_context context = {0};
			error(context, "A struct can not be assigned to a set");
		}

		definition structy                 = parse_struct(state);
		get_type(structy.type)->attributes = attributes;
		return structy;
	}
	case TOKEN_FUNCTION: {
		if (current_sets_count != 0) {
			debug_context context = {0};
			error(context, "A function can not be assigned to a set");
		}

		definition d  = parse_function(state);
		function  *f  = get_function(d.function);
		f->attributes = attributes;
		return d;
	}
	case TOKEN_CONST: {
		definition d = parse_const(state, attributes);

		for (size_t set_index = 0; set_index < current_sets_count; ++set_index) {
			add_definition_to_set(current_sets[set_index], d);
		}

		return d;
	}
	default: {
		update_debug_context(state);
		error(state->context, "Expected a struct, a function or a const");

		definition d = {0};
		return d;
	}
	}
}

static type_ref parse_type_ref(state_t *state) {
	match_token(state, TOKEN_IDENTIFIER, "Expected an identifier");
	token type_name = current(state);
	advance_state(state);

	uint32_t array_size = 0;
	if (current(state).kind == TOKEN_LEFT_SQUARE) {
		advance_state(state);
		if (current(state).kind == TOKEN_INT) {
			array_size = (uint32_t)current(state).number;
			if (array_size == 0) {
				error(state->context, "Array size of 0 is not allowed");
			}
			advance_state(state);
		}
		else {
			array_size = UINT32_MAX;
		}
		match_token(state, TOKEN_RIGHT_SQUARE, "Expected a closing square bracket");
		advance_state(state);
	}

	type_ref t;
	init_type_ref(&t, type_name.identifier);
	t.unresolved.array_size = array_size;
	return t;
}

static statement *parse_statement(state_t *state, block *parent_block) {
	switch (current(state).kind) {
	case TOKEN_IF: {
		advance_state(state);
		match_token(state, TOKEN_LEFT_PAREN, "Expected an opening bracket");
		advance_state(state);

		expression *test = parse_expression(state);
		match_token(state, TOKEN_RIGHT_PAREN, "Expected a closing bracket");
		advance_state(state);

		statement *if_block = parse_statement(state, parent_block);
		statement *s        = statement_allocate();
		s->kind             = STATEMENT_IF;
		s->iffy.test        = test;
		s->iffy.if_block    = if_block;

		s->iffy.else_size = 0;

		while (current(state).kind == TOKEN_ELSE) {
			advance_state(state);

			if (current(state).kind == TOKEN_IF) {
				advance_state(state);
				match_token(state, TOKEN_LEFT_PAREN, "Expected an opening bracket");
				advance_state(state);

				expression *test = parse_expression(state);
				match_token(state, TOKEN_RIGHT_PAREN, "Expected a closing bracket");
				advance_state(state);

				statement *if_block = parse_statement(state, parent_block);

				s->iffy.else_tests[s->iffy.else_size]  = test;
				s->iffy.else_blocks[s->iffy.else_size] = if_block;
			}
			else {
				statement *else_block                  = parse_statement(state, parent_block);
				s->iffy.else_tests[s->iffy.else_size]  = NULL;
				s->iffy.else_blocks[s->iffy.else_size] = else_block;
			}

			s->iffy.else_size += 1;
			assert(s->iffy.else_size < 64);
		}

		return s;
	}
	case TOKEN_WHILE: {
		advance_state(state);
		match_token(state, TOKEN_LEFT_PAREN, "Expected an opening bracket");
		advance_state(state);

		expression *test = parse_expression(state);
		match_token(state, TOKEN_RIGHT_PAREN, "Expected a closing bracket");
		advance_state(state);

		statement *while_block = parse_statement(state, parent_block);

		statement *s          = statement_allocate();
		s->kind               = STATEMENT_WHILE;
		s->whiley.test        = test;
		s->whiley.while_block = while_block;

		return s;
	}
	case TOKEN_DO: {
		advance_state(state);

		statement *do_block = parse_statement(state, parent_block);

		statement *s          = statement_allocate();
		s->kind               = STATEMENT_DO_WHILE;
		s->whiley.while_block = do_block;

		match_token(state, TOKEN_WHILE, "Expected \"while\"");
		advance_state(state);
		match_token(state, TOKEN_LEFT_PAREN, "Expected an opening bracket");
		advance_state(state);

		expression *test = parse_expression(state);
		match_token(state, TOKEN_RIGHT_PAREN, "Expected a closing bracket");
		advance_state(state);

		s->whiley.test = test;

		match_token(state, TOKEN_SEMICOLON, "Expected a semicolon");
		advance_state(state);

		return s;
	}
	case TOKEN_FOR: {
		statements outer_block_statements;
		statements_init(&outer_block_statements);

		statement *outer_block        = statement_allocate();
		outer_block->kind             = STATEMENT_BLOCK;
		outer_block->block.parent     = parent_block;
		outer_block->block.vars.size  = 0;
		outer_block->block.statements = outer_block_statements;

		advance_state(state);

		match_token(state, TOKEN_LEFT_PAREN, "Expected an opening bracket");
		advance_state(state);

		statement *pre = parse_statement(state, &outer_block->block);
		statements_add(&outer_block->block.statements, pre);

		expression *test = parse_expression(state);
		match_token(state, TOKEN_SEMICOLON, "Expected a semicolon");
		advance_state(state);

		expression *post_expression = parse_expression(state);

		match_token(state, TOKEN_RIGHT_PAREN, "Expected a closing bracket");
		advance_state(state);

		statement *inner_block = parse_statement(state, &outer_block->block);

		statement *post_statement  = statement_allocate();
		post_statement->kind       = STATEMENT_EXPRESSION;
		post_statement->expression = post_expression;

		statements_add(&inner_block->block.statements, post_statement);

		statement *s          = statement_allocate();
		s->kind               = STATEMENT_WHILE;
		s->whiley.test        = test;
		s->whiley.while_block = inner_block;

		statements_add(&outer_block->block.statements, s);

		return outer_block;
	}
	case TOKEN_LEFT_CURLY: {
		return parse_block(state, parent_block);
	}
	case TOKEN_VAR: {
		advance_state(state);

		match_token_identifier(state);
		token name = current(state);
		advance_state(state);

		match_token(state, TOKEN_COLON, "Expected a colon");
		advance_state(state);

		type_ref type = parse_type_ref(state);

		expression *init = NULL;

		if (current(state).kind == TOKEN_OPERATOR) {
			check(current(state).op == OPERATOR_ASSIGN, state->context, "Expected an assign");
			advance_state(state);
			init = parse_expression(state);
		}

		match_token(state, TOKEN_SEMICOLON, "Expected a semicolon");
		advance_state(state);

		statement *statement                      = statement_allocate();
		statement->kind                           = STATEMENT_LOCAL_VARIABLE;
		statement->local_variable.var.name        = name.identifier;
		statement->local_variable.var.type        = type;
		statement->local_variable.var.variable_id = 0;
		statement->local_variable.init            = init;
		return statement;
	}
	case TOKEN_RETURN: {
		advance_state(state);

		expression *expr = parse_expression(state);
		match_token(state, TOKEN_SEMICOLON, "Expected a semicolon");
		advance_state(state);

		statement *statement  = statement_allocate();
		statement->kind       = STATEMENT_RETURN_EXPRESSION;
		statement->expression = expr;
		return statement;
	}
	case TOKEN_DISCARD: {
		advance_state(state);

		match_token(state, TOKEN_SEMICOLON, "Expected a semicolon");
		advance_state(state);

		statement *statement = statement_allocate();
		statement->kind      = STATEMENT_DISCARD;

		return statement;
	}
	default: {
		expression *expr = parse_expression(state);
		match_token(state, TOKEN_SEMICOLON, "Expected a semicolon");
		advance_state(state);

		statement *statement  = statement_allocate();
		statement->kind       = STATEMENT_EXPRESSION;
		statement->expression = expr;
		return statement;
	}
	}
}

static expression *parse_assign(state_t *state);

static expression *parse_expression(state_t *state) {
	return parse_assign(state);
}

static expression *parse_logical(state_t *state);

static expression *parse_assign(state_t *state) {
	expression *expr = parse_logical(state);
	bool        done = false;
	while (!done) {
		if (current(state).kind == TOKEN_OPERATOR) {
			operatorr op = current(state).op;
			if (op == OPERATOR_ASSIGN || op == OPERATOR_MINUS_ASSIGN || op == OPERATOR_PLUS_ASSIGN || op == OPERATOR_DIVIDE_ASSIGN ||
			    op == OPERATOR_MULTIPLY_ASSIGN) {
				advance_state(state);
				expression *right        = parse_logical(state);
				expression *expression   = expression_allocate();
				expression->kind         = EXPRESSION_BINARY;
				expression->binary.left  = expr;
				expression->binary.op    = op;
				expression->binary.right = right;
				expr                     = expression;
			}
			else {
				done = true;
			}
		}
		else {
			done = true;
		}
	}
	return expr;
}

static expression *parse_bitwise(state_t *state);

static expression *parse_logical(state_t *state) {
	expression *expr = parse_bitwise(state);
	bool        done = false;
	while (!done) {
		if (current(state).kind == TOKEN_OPERATOR) {
			operatorr op = current(state).op;
			if (op == OPERATOR_OR || op == OPERATOR_AND) {
				advance_state(state);
				expression *right        = parse_bitwise(state);
				expression *expression   = expression_allocate();
				expression->kind         = EXPRESSION_BINARY;
				expression->binary.left  = expr;
				expression->binary.op    = op;
				expression->binary.right = right;
				expr                     = expression;
			}
			else {
				done = true;
			}
		}
		else {
			done = true;
		}
	}
	return expr;
}

static expression *parse_equality(state_t *state);

static expression *parse_bitwise(state_t *state) {
	expression *expr = parse_equality(state);
	bool        done = false;
	while (!done) {
		if (current(state).kind == TOKEN_OPERATOR) {
			operatorr op = current(state).op;
			if (op == OPERATOR_BITWISE_XOR || op == OPERATOR_BITWISE_OR || op == OPERATOR_BITWISE_AND) {
				advance_state(state);
				expression *right        = parse_equality(state);
				expression *expression   = expression_allocate();
				expression->kind         = EXPRESSION_BINARY;
				expression->binary.left  = expr;
				expression->binary.op    = op;
				expression->binary.right = right;
				expr                     = expression;
			}
			else {
				done = true;
			}
		}
		else {
			done = true;
		}
	}
	return expr;
}

static expression *parse_comparison(state_t *state);

static expression *parse_equality(state_t *state) {
	expression *expr = parse_comparison(state);
	bool        done = false;
	while (!done) {
		if (current(state).kind == TOKEN_OPERATOR) {
			operatorr op = current(state).op;
			if (op == OPERATOR_EQUALS || op == OPERATOR_NOT_EQUALS) {
				advance_state(state);
				expression *right        = parse_comparison(state);
				expression *expression   = expression_allocate();
				expression->kind         = EXPRESSION_BINARY;
				expression->binary.left  = expr;
				expression->binary.op    = op;
				expression->binary.right = right;
				expr                     = expression;
			}
			else {
				done = true;
			}
		}
		else {
			done = true;
		}
	}
	return expr;
}

static expression *parse_shift(state_t *state);

static expression *parse_comparison(state_t *state) {
	expression *expr = parse_shift(state);
	bool        done = false;
	while (!done) {
		if (current(state).kind == TOKEN_OPERATOR) {
			operatorr op = current(state).op;
			if (op == OPERATOR_GREATER || op == OPERATOR_GREATER_EQUAL || op == OPERATOR_LESS || op == OPERATOR_LESS_EQUAL) {
				advance_state(state);
				expression *right        = parse_shift(state);
				expression *expression   = expression_allocate();
				expression->kind         = EXPRESSION_BINARY;
				expression->binary.left  = expr;
				expression->binary.op    = op;
				expression->binary.right = right;
				expr                     = expression;
			}
			else {
				done = true;
			}
		}
		else {
			done = true;
		}
	}
	return expr;
}

static expression *parse_addition(state_t *state);

static expression *parse_shift(state_t *state) {
	expression *expr = parse_addition(state);
	bool        done = false;
	while (!done) {
		if (current(state).kind == TOKEN_OPERATOR) {
			operatorr op = current(state).op;
			if (op == OPERATOR_LEFT_SHIFT || op == OPERATOR_RIGHT_SHIFT) {
				advance_state(state);
				expression *right        = parse_addition(state);
				expression *expression   = expression_allocate();
				expression->kind         = EXPRESSION_BINARY;
				expression->binary.left  = expr;
				expression->binary.op    = op;
				expression->binary.right = right;
				expr                     = expression;
			}
			else {
				done = true;
			}
		}
		else {
			done = true;
		}
	}
	return expr;
}

static expression *parse_multiplication(state_t *state);

static expression *parse_addition(state_t *state) {
	expression *expr = parse_multiplication(state);
	bool        done = false;
	while (!done) {
		if (current(state).kind == TOKEN_OPERATOR) {
			operatorr op = current(state).op;
			if (op == OPERATOR_MINUS || op == OPERATOR_PLUS) {
				advance_state(state);
				expression *right        = parse_multiplication(state);
				expression *expression   = expression_allocate();
				expression->kind         = EXPRESSION_BINARY;
				expression->binary.left  = expr;
				expression->binary.op    = op;
				expression->binary.right = right;
				expr                     = expression;
			}
			else {
				done = true;
			}
		}
		else {
			done = true;
		}
	}
	return expr;
}

static expression *parse_unary(state_t *state);

static expression *parse_multiplication(state_t *state) {
	expression *expr = parse_unary(state);
	bool        done = false;
	while (!done) {
		if (current(state).kind == TOKEN_OPERATOR) {
			operatorr op = current(state).op;
			if (op == OPERATOR_DIVIDE || op == OPERATOR_MULTIPLY || op == OPERATOR_MOD) {
				advance_state(state);
				expression *right        = parse_unary(state);
				expression *expression   = expression_allocate();
				expression->kind         = EXPRESSION_BINARY;
				expression->binary.left  = expr;
				expression->binary.op    = op;
				expression->binary.right = right;
				expr                     = expression;
			}
			else {
				done = true;
			}
		}
		else {
			done = true;
		}
	}
	return expr;
}

static expression *parse_primary(state_t *state);

static expression *parse_unary(state_t *state) {
	bool done = false;
	while (!done) {
		if (current(state).kind == TOKEN_OPERATOR) {
			operatorr op = current(state).op;
			if (op == OPERATOR_NOT || op == OPERATOR_MINUS) {
				advance_state(state);
				expression *right       = parse_unary(state);
				expression *expression  = expression_allocate();
				expression->kind        = EXPRESSION_UNARY;
				expression->unary.op    = op;
				expression->unary.right = right;
				return expression;
			}
			else {
				done = true;
			}
		}
		else {
			done = true;
		}
	}
	return parse_primary(state);
}

static expression *parse_member_or_element_access(state_t *state, expression *of) {
	if (current(state).kind == TOKEN_DOT) {
		advance_state(state);

		match_token(state, TOKEN_IDENTIFIER, "Expected an identifier");

		token token = current(state);

		advance_state(state);

		expression *member         = expression_allocate();
		member->kind               = EXPRESSION_MEMBER;
		member->member.of          = of;
		member->member.member_name = token.identifier;

		expression *sub = parse_member_or_element_access(state, member);

		return sub;
	}
	else if (current(state).kind == TOKEN_LEFT_SQUARE) {
		advance_state(state);

		expression *index = parse_expression(state);

		match_token(state, TOKEN_RIGHT_SQUARE, "Expected a closing square bracket");

		advance_state(state);

		expression *element            = expression_allocate();
		element->kind                  = EXPRESSION_ELEMENT;
		element->element.of            = of;
		element->element.element_index = index;

		expression *sub = parse_member_or_element_access(state, element);

		return sub;
	}

	if (current(state).kind == TOKEN_LEFT_PAREN) {
		error(state->context, "Function members not supported.");
	}

	return of;
}

static expression *parse_call(state_t *state, name_id func_name);

static expression *parse_primary(state_t *state) {
	expression *left = NULL;

	switch (current(state).kind) {
	case TOKEN_BOOLEAN: {
		bool value = current(state).boolean;
		advance_state(state);
		left          = expression_allocate();
		left->kind    = EXPRESSION_BOOLEAN;
		left->boolean = value;
		break;
	}
	case TOKEN_FLOAT: {
		double value = current(state).number;
		advance_state(state);
		left         = expression_allocate();
		left->kind   = EXPRESSION_FLOAT;
		left->number = value;
		break;
	}
	case TOKEN_INT: {
		double value = current(state).number;
		advance_state(state);
		left         = expression_allocate();
		left->kind   = EXPRESSION_INT;
		left->number = value;
		break;
	}
	/*case TOKEN_STRING: {
		token token = current(state);
		advance_state(state);
		left = expression_allocate();
		left->kind = EXPRESSION_STRING;
		left->string = add_name(token.string);
		break;
	}*/
	case TOKEN_IDENTIFIER: {
		token token = current(state);
		advance_state(state);
		if (current(state).kind == TOKEN_LEFT_PAREN) {
			left = parse_call(state, token.identifier);
		}
		else {
			expression *var = expression_allocate();
			var->kind       = EXPRESSION_VARIABLE;
			var->variable   = token.identifier;
			left            = var;
		}
		break;
	}
	case TOKEN_LEFT_PAREN: {
		advance_state(state);
		expression *expr = parse_expression(state);
		match_token(state, TOKEN_RIGHT_PAREN, "Expected a closing bracket");
		advance_state(state);
		left           = expression_allocate();
		left->kind     = EXPRESSION_GROUPING;
		left->grouping = expr;
		break;
	}
	default:
		error(state->context, "Unexpected token");
		return NULL;
	}

	return parse_member_or_element_access(state, left);
}

static expressions parse_parameters(state_t *state) {
	expressions e;
	e.size = 0;

	if (current(state).kind == TOKEN_RIGHT_PAREN) {
		advance_state(state);
		return e;
	}

	for (;;) {
		e.e[e.size] = parse_expression(state);
		e.size += 1;

		if (current(state).kind == TOKEN_COMMA) {
			advance_state(state);
		}
		else {
			match_token(state, TOKEN_RIGHT_PAREN, "Expected a closing bracket");
			advance_state(state);
			return e;
		}
	}
}

static expression *parse_call(state_t *state, name_id func_name) {
	match_token(state, TOKEN_LEFT_PAREN, "Expected an opening bracket");
	advance_state(state);

	expression *call = NULL;

	call                  = expression_allocate();
	call->kind            = EXPRESSION_CALL;
	call->call.func_name  = func_name;
	call->call.parameters = parse_parameters(state);

	return parse_member_or_element_access(state, call);
}

static definition parse_struct_inner(state_t *state, name_id name) {
	match_token(state, TOKEN_LEFT_CURLY, "Expected an opening curly bracket");
	advance_state(state);

	token    member_names[MAX_MEMBERS];
	type_ref type_refs[MAX_MEMBERS];
	token    member_values[MAX_MEMBERS];
	size_t   count = 0;

	while (current(state).kind != TOKEN_RIGHT_CURLY) {
		debug_context context = {0};
		check(count < MAX_MEMBERS, context, "Out of members");

		match_token(state, TOKEN_IDENTIFIER, "Expected an identifier");
		member_names[count] = current(state);

		advance_state(state);

		if (current(state).kind == TOKEN_COLON) {
			advance_state(state);
			type_refs[count] = parse_type_ref(state);
		}
		else {
			type_ref t;
			t.type                  = NO_TYPE;
			t.unresolved.name       = NO_NAME;
			t.unresolved.array_size = 0;
			type_refs[count]        = t;
		}

		if (current(state).kind == TOKEN_OPERATOR && current(state).op == OPERATOR_ASSIGN) {
			advance_state(state);
			if (current(state).kind == TOKEN_BOOLEAN || current(state).kind == TOKEN_FLOAT || current(state).kind == TOKEN_INT ||
			    current(state).kind == TOKEN_IDENTIFIER) {
				member_values[count] = current(state);
				advance_state(state);

				if (current(state).kind == TOKEN_LEFT_PAREN) {
					advance_state(state);
					match_token(state, TOKEN_RIGHT_PAREN, "Expected a right paren");
					advance_state(state);
				}
			}
			else {
				debug_context context = {0};
				error(context, "Unsupported assign in struct");
			}
		}
		else {
			member_values[count].kind       = TOKEN_NONE;
			member_values[count].identifier = NO_NAME;
		}

		match_token(state, TOKEN_SEMICOLON, "Expected a semicolon");

		advance_state(state);

		++count;
	}

	advance_state(state);

	definition definition;
	definition.kind = DEFINITION_STRUCT;

	definition.type = add_type(name);

	type *s = get_type(definition.type);

	for (size_t i = 0; i < count; ++i) {
		member member;
		member.name  = member_names[i].identifier;
		member.value = member_values[i];
		if (member.value.kind != TOKEN_NONE) {
			if (member.value.kind == TOKEN_BOOLEAN) {
				init_type_ref(&member.type, add_name("bool"));
			}
			else if (member.value.kind == TOKEN_FLOAT) {
				init_type_ref(&member.type, add_name("float"));
			}
			else if (member.value.kind == TOKEN_INT) {
				init_type_ref(&member.type, add_name("int"));
			}
			else if (member.value.kind == TOKEN_IDENTIFIER) {
				global *g = find_global(member.value.identifier);
				if (g != NULL && g->name != NO_NAME) {
					init_type_ref(&member.type, get_type(g->type)->name);
				}
				else {
					init_type_ref(&member.type, add_name("fun"));
				}
			}
			else {
				debug_context context = {0};
				error(context, "Unsupported value in struct");
			}
		}
		else {
			member.type = type_refs[i];
		}

		s->members.m[i] = member;
	}
	s->members.size = count;

	return definition;
}

static definition parse_struct(state_t *state) {
	advance_state(state);

	match_token(state, TOKEN_IDENTIFIER, "Expected an identifier");
	token name = current(state);
	advance_state(state);

	return parse_struct_inner(state, name.identifier);
}

static definition parse_function(state_t *state) {
	advance_state(state);
	match_token(state, TOKEN_IDENTIFIER, "Expected an identifier");

	token name = current(state);
	advance_state(state);
	match_token(state, TOKEN_LEFT_PAREN, "Expected an opening bracket");
	advance_state(state);

	uint8_t  parameters_size       = 0;
	name_id  param_names[256]      = {0};
	type_ref param_types[256]      = {0};
	name_id  param_attributes[256] = {0};

	while (current(state).kind != TOKEN_RIGHT_PAREN) {
		if (current(state).kind == TOKEN_HASH) {
			advance_state(state);
			match_token(state, TOKEN_LEFT_SQUARE, "Expected an opening square bracket");
			advance_state(state);
			match_token(state, TOKEN_IDENTIFIER, "Expected an identifier");
			token attribute_name              = current(state);
			param_attributes[parameters_size] = attribute_name.identifier;
			advance_state(state);
			match_token(state, TOKEN_RIGHT_SQUARE, "Expected a closing square bracket");
			advance_state(state);
		}

		match_token(state, TOKEN_IDENTIFIER, "Expected an identifier");
		param_names[parameters_size] = current(state).identifier;
		advance_state(state);
		match_token(state, TOKEN_COLON, "Expected a colon");
		advance_state(state);
		param_types[parameters_size] = parse_type_ref(state);
		if (current(state).kind == TOKEN_COMMA) {
			advance_state(state);
		}
		parameters_size += 1;
	}

	match_token(state, TOKEN_RIGHT_PAREN, "Expected a closing bracket");
	advance_state(state);
	match_token(state, TOKEN_COLON, "Expected a colon");
	advance_state(state);

	type_ref return_type = parse_type_ref(state);

	statement *block = parse_block(state, NULL);

	definition d;
	d.kind             = DEFINITION_FUNCTION;
	d.function         = add_function(name.identifier);
	function *f        = get_function(d.function);
	f->return_type     = return_type;
	f->parameters_size = parameters_size;
	for (uint8_t parameter_index = 0; parameter_index < parameters_size; ++parameter_index) {
		f->parameter_names[parameter_index]      = param_names[parameter_index];
		f->parameter_types[parameter_index]      = param_types[parameter_index];
		f->parameter_attributes[parameter_index] = param_attributes[parameter_index];
	}
	f->block = block;

	return d;
}

static texture_format convert_texture_format(state_t *state, name_id format_name) {
	if (format_name == NO_NAME) {
		return TEXTURE_FORMAT_UNDEFINED;
	}
	else if (format_name == add_name("framebuffer_format")) {
		return TEXTURE_FORMAT_FRAMEBUFFER;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_DEPTH")) {
		return TEXTURE_FORMAT_DEPTH;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_R8_UNORM")) {
		return TEXTURE_FORMAT_R8_UNORM;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_R8_SNORM")) {
		return TEXTURE_FORMAT_R8_SNORM;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_R8_UINT")) {
		return TEXTURE_FORMAT_R8_UINT;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_R8_SINT")) {
		return TEXTURE_FORMAT_R8_SINT;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_R16_UINT")) {
		return TEXTURE_FORMAT_R16_UINT;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_R16_SINT")) {
		return TEXTURE_FORMAT_R16_SINT;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_R16_FLOAT")) {
		return TEXTURE_FORMAT_R16_FLOAT;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_RG8_UNORM")) {
		return TEXTURE_FORMAT_RG8_UNORM;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_RG8_SNORM")) {
		return TEXTURE_FORMAT_RG8_SNORM;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_RG8_UINT")) {
		return TEXTURE_FORMAT_RG8_UINT;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_RG8_SINT")) {
		return TEXTURE_FORMAT_RG8_SINT;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_R32_UINT")) {
		return TEXTURE_FORMAT_R32_UINT;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_R32_SINT")) {
		return TEXTURE_FORMAT_R32_SINT;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_R32_FLOAT")) {
		return TEXTURE_FORMAT_R32_FLOAT;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_RG16_UINT")) {
		return TEXTURE_FORMAT_RG16_UINT;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_RG16_SINT")) {
		return TEXTURE_FORMAT_RG16_SINT;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_RG16_FLOAT")) {
		return TEXTURE_FORMAT_RG16_FLOAT;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_RGBA8_UNORM")) {
		return TEXTURE_FORMAT_RGBA8_UNORM;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_RGBA8_UNORM_SRGB")) {
		return TEXTURE_FORMAT_RGBA8_UNORM_SRGB;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_RGBA8_SNORM")) {
		return TEXTURE_FORMAT_RGBA8_SNORM;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_RGBA8_UINT")) {
		return TEXTURE_FORMAT_RGBA8_UINT;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_RGBA8_SINT")) {
		return TEXTURE_FORMAT_RGBA8_SINT;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_BGRA8_UNORM")) {
		return TEXTURE_FORMAT_BGRA8_UNORM;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_BGRA8_UNORM_SRGB")) {
		return TEXTURE_FORMAT_BGRA8_UNORM_SRGB;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_RGB9E5U_FLOAT")) {
		return TEXTURE_FORMAT_RGB9E5U_FLOAT;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_RGB10A2_UINT")) {
		return TEXTURE_FORMAT_RGB10A2_UINT;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_RGB10A2_UNORM")) {
		return TEXTURE_FORMAT_RGB10A2_UNORM;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_RG11B10U_FLOAT")) {
		return TEXTURE_FORMAT_RG11B10U_FLOAT;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_RG32_UINT")) {
		return TEXTURE_FORMAT_RG32_UINT;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_RG32_SINT")) {
		return TEXTURE_FORMAT_RG32_SINT;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_RG32_FLOAT")) {
		return TEXTURE_FORMAT_RG32_FLOAT;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_RGBA16_UINT")) {
		return TEXTURE_FORMAT_RGBA16_UINT;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_RGBA16_SINT")) {
		return TEXTURE_FORMAT_RGBA16_SINT;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_RGBA16_FLOAT")) {
		return TEXTURE_FORMAT_RGBA16_FLOAT;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_RGBA32_UINT")) {
		return TEXTURE_FORMAT_RGBA32_UINT;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_RGBA32_SINT")) {
		return TEXTURE_FORMAT_RGBA32_SINT;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_RGBA32_FLOAT")) {
		return TEXTURE_FORMAT_RGBA32_FLOAT;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_DEPTH16_UNORM")) {
		return TEXTURE_FORMAT_DEPTH16_UNORM;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_DEPTH24_NOTHING8")) {
		return TEXTURE_FORMAT_DEPTH24_NOTHING8;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_DEPTH24_STENCIL8")) {
		return TEXTURE_FORMAT_DEPTH24_STENCIL8;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_DEPTH32_FLOAT")) {
		return TEXTURE_FORMAT_DEPTH32_FLOAT;
	}
	else if (format_name == add_name("TEXTURE_FORMAT_DEPTH32_FLOAT_STENCIL8_NOTHING24")) {
		return TEXTURE_FORMAT_DEPTH32_FLOAT_STENCIL8_NOTHING24;
	}
	else {
		error(state->context, "Unknown texture format %s", format_name);
		return TEXTURE_FORMAT_UNDEFINED;
	}
}

static definition parse_const(state_t *state, attribute_list attributes) {
	advance_state(state);
	match_token(state, TOKEN_IDENTIFIER, "Expected an identifier");

	token name = current(state);
	advance_state(state);
	match_token(state, TOKEN_COLON, "Expected a colon");
	advance_state(state);

	name_id type_name   = NO_NAME;
	type_id type        = NO_TYPE;
	name_id format_name = NO_NAME;

	if (current(state).kind == TOKEN_LEFT_CURLY) {
		type = parse_struct_inner(state, NO_NAME).type;
	}
	else {
		match_token(state, TOKEN_IDENTIFIER, "Expected an identifier");
		type_name = current(state).identifier;
		advance_state(state);
	}

	bool     array      = false;
	uint32_t array_size = UINT32_MAX;

	if (current(state).kind == TOKEN_LEFT_SQUARE) {
		array = true;
		advance_state(state);

		if (current(state).kind == TOKEN_INT) {
			array_size = (uint32_t)current(state).number;
			advance_state(state);
		}

		match_token(state, TOKEN_RIGHT_SQUARE, "Expected a right square bracket");
		advance_state(state);
	}

	expression *value = NULL;
	if (current(state).kind == TOKEN_OPERATOR && current(state).op == OPERATOR_ASSIGN) {
		advance_state(state);
		value = parse_expression(state);
	}

	if (current(state).kind == TOKEN_OPERATOR && current(state).op == OPERATOR_LESS) {
		advance_state(state);
		match_token(state, TOKEN_IDENTIFIER, "Expected an identifier");
		format_name = current(state).identifier;
		advance_state(state);

		if (current(state).kind == TOKEN_LEFT_PAREN) {
			advance_state(state);
			match_token(state, TOKEN_RIGHT_PAREN, "Expected a right paren");
			advance_state(state);
		}

		if (current(state).kind != TOKEN_OPERATOR || current(state).op != OPERATOR_GREATER) {
			error(state->context, "Expected a greater than");
		}
		advance_state(state);
	}

	match_token(state, TOKEN_SEMICOLON, "Expected a semicolon");
	advance_state(state);

	definition d = {0};

	name_id tex1d_name        = add_name("tex1d");
	name_id tex2d_name        = add_name("tex2d");
	name_id tex3d_name        = add_name("tex3d");
	name_id texcube_name      = add_name("texcube");
	name_id tex1darray_name   = add_name("tex1darray");
	name_id tex2darray_name   = add_name("tex2darray");
	name_id texcubearray_name = add_name("texcubearray");

	if (type_name == NO_NAME) {
		debug_context context = {0};
		check(type != NO_TYPE, context, "Const has no type");
		d.kind   = DEFINITION_CONST_CUSTOM;
		d.global = add_global(type, attributes, name.identifier);
	}
	else if (type_name == tex1d_name || type_name == tex2d_name || type_name == tex3d_name || type_name == texcube_name || type_name == tex1darray_name ||
	         type_name == tex2darray_name || type_name == texcubearray_name) {
		struct type tex_type;
		tex_type.name                        = type_name;
		tex_type.attributes.attributes_count = 0;
		tex_type.members.size                = 0;
		tex_type.built_in                    = true;
		tex_type.array_size                  = 0;
		tex_type.base                        = NO_TYPE;

		if (type_name == tex1d_name) {
			d.kind            = DEFINITION_TEX1D;
			tex_type.tex_kind = TEXTURE_KIND_1D;
		}
		else if (type_name == tex2d_name) {
			d.kind            = DEFINITION_TEX2D;
			tex_type.tex_kind = TEXTURE_KIND_2D;
		}
		else if (type_name == tex3d_name) {
			d.kind            = DEFINITION_TEX3D;
			tex_type.tex_kind = TEXTURE_KIND_3D;
		}
		else if (type_name == texcube_name) {
			d.kind            = DEFINITION_TEXCUBE;
			tex_type.tex_kind = TEXTURE_KIND_CUBE;
		}
		else if (type_name == tex1darray_name) {
			d.kind            = DEFINITION_TEX1DARRAY;
			tex_type.tex_kind = TEXTURE_KIND_1D_ARRAY;
		}
		else if (type_name == tex2darray_name) {
			d.kind            = DEFINITION_TEX2DARRAY;
			tex_type.tex_kind = TEXTURE_KIND_2D_ARRAY;
		}
		else if (type_name == texcubearray_name) {
			d.kind            = DEFINITION_TEXCUBEARRAY;
			tex_type.tex_kind = TEXTURE_KIND_CUBE_ARRAY;
		}
		else {
			assert(false);
		}

		tex_type.tex_format = convert_texture_format(state, format_name);

		type_id t_id = add_full_type(&tex_type);

		if (array) {
			type_id array_type_id               = add_type(type_name);
			get_type(array_type_id)->base       = t_id;
			get_type(array_type_id)->built_in   = true;
			get_type(array_type_id)->array_size = array_size;
			t_id                                = array_type_id;
		}

		d.global = add_global(t_id, attributes, name.identifier);
	}
	else if (type_name == add_name("sampler")) {
		d.kind   = DEFINITION_SAMPLER;
		d.global = add_global(sampler_type_id, attributes, name.identifier);
	}
	else if (type_name == add_name("bvh")) {
		d.kind   = DEFINITION_BVH;
		d.global = add_global(bvh_type_id, attributes, name.identifier);
	}
	else if (type_name == add_name("float")) {
		debug_context context = {0};
		check(value != NULL, context, "const float requires an initialization value");
		check(value->kind == EXPRESSION_FLOAT || value->kind == EXPRESSION_INT, context, "const float requires a number");

		global_value float_value;
		float_value.kind = GLOBAL_VALUE_FLOAT;

		float_value.value.floats[0] = (float)value->number;

		d.kind   = DEFINITION_CONST_BASIC;
		d.global = add_global_with_value(float_id, attributes, name.identifier, float_value);
	}
	else if (type_name == add_name("float2")) {
		debug_context context = {0};
		check(value != NULL, context, "const float2 requires an initialization value");
		check(value->kind == EXPRESSION_CALL, context, "const float2 requires a constructor call");
		check(value->call.func_name == add_name("float2"), context, "const float2 requires a float2 call");
		check(value->call.parameters.size == 3, context, "const float2 construtor call requires two parameters");

		global_value float2_value;
		float2_value.kind = GLOBAL_VALUE_FLOAT2;

		for (int i = 0; i < 2; ++i) {
			check(value->call.parameters.e[i]->kind == EXPRESSION_FLOAT || value->call.parameters.e[i]->kind == EXPRESSION_INT, context,
			      "const float2 construtor parameters have to be numbers");
			float2_value.value.floats[i] = (float)value->call.parameters.e[i]->number;
		}

		d.kind   = DEFINITION_CONST_BASIC;
		d.global = add_global_with_value(float2_id, attributes, name.identifier, float2_value);
	}
	else if (type_name == add_name("float3")) {
		debug_context context = {0};
		check(value != NULL, context, "const float3 requires an initialization value");
		check(value->kind == EXPRESSION_CALL, context, "const float3 requires a constructor call");
		check(value->call.func_name == add_name("float3"), context, "const float3 requires a float3 call");
		check(value->call.parameters.size == 3, context, "const float3 construtor call requires three parameters");

		global_value float3_value;
		float3_value.kind = GLOBAL_VALUE_FLOAT3;

		for (int i = 0; i < 3; ++i) {
			check(value->call.parameters.e[i]->kind == EXPRESSION_FLOAT || value->call.parameters.e[i]->kind == EXPRESSION_INT, context,
			      "const float3 construtor parameters have to be numbers");
			float3_value.value.floats[i] = (float)value->call.parameters.e[i]->number;
		}

		d.kind   = DEFINITION_CONST_BASIC;
		d.global = add_global_with_value(float3_id, attributes, name.identifier, float3_value);
	}
	else if (type_name == add_name("float4")) {
		debug_context context = {0};
		if (!array) {
			check(value != NULL, context, "const float4 requires an initialization value");
			check(value->kind == EXPRESSION_CALL, context, "const float4 requires a constructor call");
			check(value->call.func_name == add_name("float4"), context, "const float4 requires a float4 call");
			check(value->call.parameters.size == 4, context, "const float4 construtor call requires four parameters");
		}
		else {
			check(value == NULL, context, "const float4[] does not allow an initialization value");
		}

		global_value float4_value;
		float4_value.kind = GLOBAL_VALUE_FLOAT4;

		if (!array) {
			for (int i = 0; i < 4; ++i) {
				check(value->call.parameters.e[i]->kind == EXPRESSION_FLOAT || value->call.parameters.e[i]->kind == EXPRESSION_INT, context,
				      "const float4 construtor parameters have to be numbers");
				float4_value.value.floats[i] = (float)value->call.parameters.e[i]->number;
			}
		}

		d.kind = DEFINITION_CONST_BASIC;
		if (array) {
			type_id array_type_id               = add_type(get_type(float4_id)->name);
			get_type(array_type_id)->base       = float4_id;
			get_type(array_type_id)->built_in   = true;
			get_type(array_type_id)->array_size = array_size;

			d.global = add_global(array_type_id, attributes, name.identifier);
		}
		else {
			d.global = add_global_with_value(float4_id, attributes, name.identifier, float4_value);
		}
	}
	else {
		debug_context context = {0};
		error(context, "Unsupported global");
	}

	return d;
}
