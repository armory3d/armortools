#pragma once

#include "functions.h"
#include "globals.h"
#include "names.h"
#include "tokenizer.h"
#include "types.h"

#include <stdbool.h>
#include <stddef.h>

struct expression;

typedef struct expressions {
	struct expression *e[256];
	size_t             size;
} expressions;

typedef struct expression {
	enum {
		EXPRESSION_BINARY,
		EXPRESSION_UNARY,
		EXPRESSION_BOOLEAN,
		EXPRESSION_FLOAT,
		EXPRESSION_INT,
		// EXPRESSION_STRING,
		EXPRESSION_VARIABLE,
		EXPRESSION_GROUPING,
		EXPRESSION_CALL,
		EXPRESSION_MEMBER,
		EXPRESSION_ELEMENT,
		EXPRESSION_SWIZZLE
	} kind;

	type_ref type;

	union {
		struct {
			struct expression *left;
			operatorr          op;
			struct expression *right;
		} binary;
		struct {
			operatorr          op;
			struct expression *right;
		} unary;
		bool   boolean;
		double number;
		// char string[MAX_IDENTIFIER_SIZE];
		name_id            variable;
		uint32_t           index;
		struct expression *grouping;
		struct {
			name_id     func_name;
			expressions parameters;
		} call;
		struct {
			struct expression *of;
			name_id            member_name;
		} member;
		struct {
			struct expression *of;
			struct expression *element_index;
		} element;
		struct {
			struct expression *of;
			struct swizzle     swizz;
		} swizzle;
	};
} expression;

struct statement;

typedef struct statements {
	struct statement *s[256];
	size_t            size;
} statements;

typedef struct local_variable {
	name_id  name;
	type_ref type;
	uint64_t variable_id;
} local_variable;

typedef struct local_variables {
	local_variable v[256];
	size_t         size;
} local_variables;

typedef struct block {
	struct block   *parent;
	local_variables vars;
	statements      statements;
} block;

typedef struct statement {
	enum {
		STATEMENT_EXPRESSION,
		STATEMENT_RETURN_EXPRESSION,
		STATEMENT_DISCARD,
		STATEMENT_IF,
		STATEMENT_WHILE,
		STATEMENT_DO_WHILE,
		STATEMENT_BLOCK,
		STATEMENT_LOCAL_VARIABLE
	} kind;

	union {
		expression *expression;
		struct {
			expression       *test;
			struct statement *if_block;
			expression       *else_tests[64];
			struct statement *else_blocks[64];
			uint16_t          else_size;
		} iffy;
		struct {
			expression       *test;
			struct statement *while_block;
		} whiley;
		block block;
		struct {
			local_variable var;
			expression    *init;
		} local_variable;
	};
} statement;

typedef struct definition {
	enum {
		DEFINITION_FUNCTION,
		DEFINITION_STRUCT,
		DEFINITION_TEX2D,
		DEFINITION_TEX2DARRAY,
		DEFINITION_TEXCUBE,
		DEFINITION_SAMPLER,
		DEFINITION_CONST_CUSTOM,
		DEFINITION_CONST_BASIC,
		DEFINITION_BVH
	} kind;

	union {
		function_id function;
		global_id   global;
		type_id     type;
	};
} definition;

void parse(const char *filename, tokens *tokens);
