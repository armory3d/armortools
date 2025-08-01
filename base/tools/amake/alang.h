// Based on https://github.com/Kode/Kongruent by RobDangerous
#pragma once

#include <stdint.h>
#include <stdbool.h>

#define OPCODES_SIZE (1024 * 1024)

typedef enum variable_kind { VARIABLE_GLOBAL, VARIABLE_LOCAL, VARIABLE_INTERNAL } variable_kind;

typedef enum access_kind { ACCESS_MEMBER, ACCESS_ELEMENT } access_kind;

typedef enum operatorr {
	OPERATOR_EQUALS,
	OPERATOR_NOT_EQUALS,
	OPERATOR_GREATER,
	OPERATOR_GREATER_EQUAL,
	OPERATOR_LESS,
	OPERATOR_LESS_EQUAL,
	OPERATOR_MINUS,
	OPERATOR_PLUS,
	OPERATOR_DIVIDE,
	OPERATOR_MULTIPLY,
	OPERATOR_NOT,
	OPERATOR_OR,
	OPERATOR_BITWISE_XOR,
	OPERATOR_BITWISE_AND,
	OPERATOR_BITWISE_OR,
	OPERATOR_LEFT_SHIFT,
	OPERATOR_RIGHT_SHIFT,
	OPERATOR_AND,
	OPERATOR_MOD,
	OPERATOR_ASSIGN,
	OPERATOR_MINUS_ASSIGN,
	OPERATOR_PLUS_ASSIGN,
	OPERATOR_DIVIDE_ASSIGN,
	OPERATOR_MULTIPLY_ASSIGN
} operatorr;

typedef uint32_t type_id;
typedef size_t name_id;

typedef struct attribute {
	name_id name;
	double  parameters[16];
	uint8_t paramters_count;
} attribute;

typedef struct attributes {
	attribute attributes[64];
	uint8_t   attributes_count;
} attribute_list;

typedef struct unresolved_type_ref {
	name_id  name;
	uint32_t array_size;
} unresolved_type_ref;

typedef struct type_ref {
	type_id             type;
	unresolved_type_ref unresolved;
} type_ref;

typedef struct opcodes {
	uint8_t o[OPCODES_SIZE];
	size_t  size;
} opcodes;

typedef struct function {
	attribute_list    attributes;
	name_id           name;
	type_ref          return_type;
	name_id           parameter_names[256];
	type_ref          parameter_types[256];
	name_id           parameter_attributes[256];
	uint8_t           parameters_size;
	struct statement *block;
	opcodes code;
} function;

typedef uint32_t function_id;

typedef struct global_value {
	enum {
		GLOBAL_VALUE_FLOAT,
		GLOBAL_VALUE_INT,
		GLOBAL_VALUE_UINT,
		GLOBAL_VALUE_BOOL,
		GLOBAL_VALUE_NONE
	} kind;
	union {
		float    floats[1];
		int      ints[1];
		unsigned uints[1];
		bool     b;
	} value;
} global_value;

typedef struct global {
	name_id                name;
	type_id                type;
	uint64_t               var_index;
	global_value           value;
	attribute_list         attributes;
	uint32_t               usage;
} global;

typedef struct variable {
	variable_kind kind;
	uint64_t      index;
	type_ref      type;
} variable;

typedef struct access {
	access_kind kind;
	type_id     type;

	union {
		struct {
			name_id name;
		} access_member;

		struct {
			variable index;
		} access_element;
	};
} access;

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
		OPCODE_STORE_ACCESS_LIST,
		OPCODE_SUB_AND_STORE_ACCESS_LIST,
		OPCODE_ADD_AND_STORE_ACCESS_LIST,
		OPCODE_DIVIDE_AND_STORE_ACCESS_LIST,
		OPCODE_MULTIPLY_AND_STORE_ACCESS_LIST,
		OPCODE_LOAD_FLOAT_CONSTANT,
		OPCODE_LOAD_INT_CONSTANT,
		OPCODE_LOAD_BOOL_CONSTANT,
		OPCODE_LOAD_ACCESS_LIST,
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
		OPCODE_BITWISE_XOR,
		OPCODE_BITWISE_AND,
		OPCODE_BITWISE_OR,
		OPCODE_LEFT_SHIFT,
		OPCODE_RIGHT_SHIFT,
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

			access  access_list[64];
			uint8_t access_list_size;
		} op_store_access_list;
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

			access  access_list[64];
			uint8_t access_list_size;
		} op_load_access_list;
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

typedef struct token {
	int line, column;

	enum {
		TOKEN_NONE,
		TOKEN_BOOLEAN,
		TOKEN_FLOAT,
		TOKEN_INT,
		// TOKEN_STRING,
		TOKEN_IDENTIFIER,
		TOKEN_LEFT_PAREN,
		TOKEN_RIGHT_PAREN,
		TOKEN_LEFT_CURLY,
		TOKEN_RIGHT_CURLY,
		TOKEN_LEFT_SQUARE,
		TOKEN_RIGHT_SQUARE,
		TOKEN_HASH,
		TOKEN_IF,
		TOKEN_ELSE,
		TOKEN_WHILE,
		TOKEN_DO,
		TOKEN_FOR,
		TOKEN_SEMICOLON,
		TOKEN_COLON,
		TOKEN_DOT,
		TOKEN_COMMA,
		TOKEN_OPERATOR,
		TOKEN_IN,
		TOKEN_STRUCT,
		TOKEN_FUNCTION,
		TOKEN_VAR,
		TOKEN_CONST,
		TOKEN_RETURN
	} kind;

	union {
		bool   boolean;
		double number;
		// char string[MAX_IDENTIFIER_SIZE];
		name_id   identifier;
		operatorr op;
	};
} token;

typedef struct tokens {
	token *t;
	size_t current_size;
	size_t max_size;
} tokens;

typedef struct allocated_global {
	global  *g;
	uint64_t variable_id;
} allocated_global;

function *_get_function(function_id function);
void _names_init(void);
void _types_init(void);
void _functions_init(void);
void _globals_init(void);
void _parse(const char *filename, tokens *tokens);
void _resolve_types(void);
void _compile_function_block(opcodes *code, struct statement *block);
tokens _tokenize(const char *filename, const char *source);
char *_get_name(name_id index);

extern type_id _void_id;
extern type_id _float_id;
extern type_id _int_id;
extern type_id _uint_id;
extern type_id _bool_id;

extern allocated_global __allocated_globals[1024];
extern size_t           __allocated_globals_size;
