#pragma once

#include "names.h"
#include "types.h"

#include <stddef.h>
#include <stdint.h>

typedef enum variable_kind { VARIABLE_GLOBAL, VARIABLE_LOCAL } variable_kind;

typedef struct variable {
	variable_kind kind;
	uint64_t      index;
	type_ref      type;
} variable;

typedef struct opcode {
	enum {
		OPCODE_VAR,
		OPCODE_NOT,
		OPCODE_NEGATE,
		OPCODE_STORE_VARIABLE,
		OPCODE_SUB_AND_STORE_VARIABLE,
		OPCODE_ADD_AND_STORE_VARIABLE,
		OPCODE_DIVIDE_AND_STORE_VARIABLE,
		OPCODE_MULTIPLY_AND_STORE_VARIABLE,
		OPCODE_STORE_MEMBER,
		OPCODE_SUB_AND_STORE_MEMBER,
		OPCODE_ADD_AND_STORE_MEMBER,
		OPCODE_DIVIDE_AND_STORE_MEMBER,
		OPCODE_MULTIPLY_AND_STORE_MEMBER,
		OPCODE_LOAD_FLOAT_CONSTANT,
		OPCODE_LOAD_INT_CONSTANT,
		OPCODE_LOAD_BOOL_CONSTANT,
		OPCODE_LOAD_MEMBER,
		OPCODE_RETURN,
		OPCODE_CALL,
		OPCODE_MULTIPLY,
		OPCODE_DIVIDE,
		OPCODE_MOD,
		OPCODE_ADD,
		OPCODE_SUB,
		OPCODE_EQUALS,
		OPCODE_NOT_EQUALS,
		OPCODE_GREATER,
		OPCODE_GREATER_EQUAL,
		OPCODE_LESS,
		OPCODE_LESS_EQUAL,
		OPCODE_AND,
		OPCODE_OR,
		OPCODE_XOR,
		OPCODE_IF,
		OPCODE_WHILE_START,
		OPCODE_WHILE_CONDITION,
		OPCODE_WHILE_END,
		OPCODE_WHILE_BODY,
		OPCODE_BLOCK_START,
		OPCODE_BLOCK_END
	} type;
	uint32_t size;

	union {
		struct {
			variable var;
		} op_var;
		struct {
			variable from;
			variable to;
		} op_negate;
		struct {
			variable from;
			variable to;
		} op_not;
		struct {
			variable from;
			variable to;
		} op_store_var;
		struct {
			variable from;
			variable to;

			bool     dynamic_member[64];
			variable dynamic_member_indices[64];

			uint32_t static_member_indices[64];

			uint8_t member_indices_size;
		} op_store_member;
		struct {
			float    number;
			variable to;
		} op_load_float_constant;
		struct {
			int      number;
			variable to;
		} op_load_int_constant;
		struct {
			bool     boolean;
			variable to;
		} op_load_bool_constant;
		struct {
			variable from;
			variable to;

			bool     dynamic_member[64];
			variable dynamic_member_indices[64];

			uint32_t static_member_indices[64];
			type_id  member_parent_type;
			bool     member_parent_array;

			uint8_t member_indices_size;
		} op_load_member;
		struct {
			variable var;
		} op_return;
		struct {
			variable var;
			name_id  func;
			variable parameters[64];
			uint8_t  parameters_size;
		} op_call;
		struct {
			variable right;
			variable left;
			variable result;
		} op_binary;
		struct {
			variable condition;
			uint64_t start_id;
			uint64_t end_id;
		} op_if;
		struct {
			uint64_t start_id;
			uint64_t continue_id;
			uint64_t end_id;
		} op_while_start;
		struct {
			uint64_t start_id;
			uint64_t continue_id;
			uint64_t end_id;
		} op_while_end;
		struct {
			variable condition;
			uint64_t end_id;
		} op_while;
		struct {
			uint64_t id;
		} op_block;
		struct {
			uint8_t nothing;
		} op_nothing;
	};
} opcode;

#define OPCODES_SIZE (256 * 1024)

typedef struct opcodes {
	uint8_t o[OPCODES_SIZE];
	size_t  size;
} opcodes;

void allocate_globals(void);

struct statement;

void compile_function_block(opcodes *code, struct statement *block);
