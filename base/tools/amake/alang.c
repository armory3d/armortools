// Based on https://github.com/Kode/Kongruent by RobDangerous
#include "alang.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>
// #define STB_DS_IMPLEMENTATION
#include "../../sources/libs/kong/libs/stb_ds.h"
#ifdef WIN32
#include <Windows.h>
#endif
#ifdef __android__
#include <android/log.h>
#endif

#define static_array(type, name, max_size) \
	typedef struct name {                  \
		type   values[max_size];           \
		size_t size;                       \
		size_t max;                        \
	} name;

#define static_array_init(array) \
	array.size = 0;              \
	array.max  = sizeof(array.values) / sizeof(array.values[0])

#define static_array_push(array, value)   \
	if (array.size >= array.max) {        \
		debug_context context = {0};      \
		error(context, "Array overflow"); \
	}                                     \
	array.values[array.size++] = value;

#define static_array_push_p(array, value) \
	if (array->size >= array->max) {      \
		debug_context context = {0};      \
		error(context, "Array overflow"); \
	}                                     \
	array->values[array->size++] = value;

// based on https://valkey.io/blog/new-hash-table/

struct meta {
	uint8_t c        : 1;
	uint8_t presence : 7;
	uint8_t hashes[7];
};

struct container {
	int key;
};

struct bucket {
	struct meta meta;
	void       *entries[7];
};

#define HASH_MAP_SIZE 256

struct hash_map {
	struct bucket buckets[HASH_MAP_SIZE];
};

static inline uint64_t hash(int key) {
	uint64_t primary   = key % HASH_MAP_SIZE;
	uint8_t  secondary = key % HASH_MAP_SIZE;
	return primary | ((uint64_t)secondary << 56);
}

static inline struct bucket *allocate_bucket(void) {
#ifdef _WIN32
	struct bucket *new_bucket = (struct bucket *)_aligned_malloc(sizeof(struct bucket), 64);
#else
	struct bucket *new_bucket = (struct bucket *)aligned_alloc(64, sizeof(struct bucket));
#endif
	assert(new_bucket != NULL);
	memset(new_bucket, 0, sizeof(struct bucket));

	// cache line check
	assert((uint64_t)new_bucket % 64 == 0);
	assert(sizeof(struct bucket) == 64);

	return new_bucket;
}

static inline struct hash_map *hash_map_create(void) {
#ifdef _WIN32
	struct hash_map *map = _aligned_malloc(sizeof(struct hash_map), 64);
#else
	struct hash_map *map = aligned_alloc(64, sizeof(struct hash_map));
#endif
	assert(map != NULL);
	memset(map, 0, sizeof(struct hash_map));

	// cache line check
	assert((uint64_t)map % 64 == 0);

	return map;
}

static inline void hash_map_destroy(struct hash_map *map) {}

static inline void hash_map_add_to_bucket(struct bucket *bucket, struct container *value, uint8_t secondary_hash) {
	for (uint32_t index = 0; index < 7; ++index) {
		if (((bucket->meta.presence >> index) & 1) == 0) {
			bucket->entries[index]     = value;
			bucket->meta.hashes[index] = secondary_hash;

			bucket->meta.presence |= (1 << index);

			return;
		}
	}

	if (bucket->meta.c) {
		hash_map_add_to_bucket((struct bucket *)bucket->entries[6], value, secondary_hash);
	}
	else {
		bucket->meta.c = 1;

		struct container *previous                = (struct container *)bucket->entries[6];
		uint8_t           previous_secondary_hash = bucket->meta.hashes[6];

		struct bucket *new_bucket = allocate_bucket();

		bucket->entries[6] = new_bucket;

		hash_map_add_to_bucket(new_bucket, previous, previous_secondary_hash);
		hash_map_add_to_bucket(new_bucket, value, secondary_hash);
	}
}

static inline void hash_map_add(struct hash_map *map, struct container *value) {
	uint64_t hash_value = hash(value->key);
	uint64_t primary    = hash_value & 0xffffffffffffffu;
	uint8_t  secondary  = (uint8_t)(hash_value << 56);

	hash_map_add_to_bucket(&map->buckets[primary], value, secondary);
}

static inline struct container *hash_map_get_from_bucket(struct bucket *bucket, int key, uint8_t secondary_hash) {
	for (uint32_t index = 0; index < 6; ++index) {
		if (((bucket->meta.presence >> index) & 1) != 0 && bucket->meta.hashes[index] == secondary_hash) {
			struct container *value = (struct container *)bucket->entries[index];

			if (value->key == key) {
				return value;
			}
		}
	}

	if (bucket->meta.c) {
		struct bucket *next_bucket = (struct bucket *)bucket->entries[6];
		return hash_map_get_from_bucket(next_bucket, key, secondary_hash);
	}
	else {
		if (((bucket->meta.presence >> 6) & 1) != 0 && bucket->meta.hashes[6] == secondary_hash) {
			struct container *value = (struct container *)bucket->entries[6];

			if (value->key == key) {
				return value;
			}
		}
	}

	return NULL;
}

static inline struct container *hash_map_get(struct hash_map *map, int key) {
	uint64_t hash_value = hash(key);
	uint64_t primary    = hash_value & 0xffffffffffffffu;
	uint8_t  secondary  = (uint8_t)(hash_value << 56);

	struct bucket *bucket = &map->buckets[primary];

	return hash_map_get_from_bucket(bucket, key, secondary);
}

static inline void hash_map_iterate_in_bucket(struct bucket *bucket, void (*callback)(struct container *, void *data), void *data) {
	for (uint32_t index = 0; index < 6; ++index) {
		if (((bucket->meta.presence >> index) & 1) != 0) {
			struct container *value = (struct container *)bucket->entries[index];
			callback(value, data);
		}
	}

	if (bucket->meta.c) {
		struct bucket *next_bucket = (struct bucket *)bucket->entries[6];
		hash_map_iterate_in_bucket(next_bucket, callback, data);
	}
	else {
		if (((bucket->meta.presence >> 6) & 1) != 0) {
			struct container *value = (struct container *)bucket->entries[6];
			callback(value, data);
		}
	}
}

static inline void hash_map_iterate(struct hash_map *map, void (*callback)(struct container *, void *data), void *data) {
	for (uint32_t bucket_index = 0; bucket_index < HASH_MAP_SIZE; ++bucket_index) {
		struct bucket *bucket = &map->buckets[bucket_index];
		hash_map_iterate_in_bucket(bucket, callback, data);
	}
}

typedef struct debug_context {
	const char *filename;
	uint32_t    column;
	uint32_t    line;
} debug_context;

static void error(debug_context context, const char *message, ...);
static void error_no_context(const char *message, ...);
static void error_args(debug_context context, const char *message, va_list args);
static void error_args_no_context(const char *message, va_list args);
static void          check_function(bool test, debug_context context, const char *message, ...);
#define check(test, context, message, ...) \
	assert(test);                          \
	check_function(test, context, message, ##__VA_ARGS__)

typedef enum { LOG_LEVEL_INFO, LOG_LEVEL_WARNING, LOG_LEVEL_ERROR } log_level_t;
static void kong_log(log_level_t log_level, const char *format, ...);
static void kong_log_args(log_level_t log_level, const char *format, va_list args);

////

#define OP_SIZE(op, opmember) offsetof(opcode, opmember) + sizeof(op.opmember)
#define NO_FUNCTION 0xFFFFFFFF
#define NO_NAME 0
#define NO_TYPE 0xFFFFFFFF
#define MAX_MEMBERS 1024

struct statement;

typedef uint32_t global_id;

typedef struct global_array {
	global_id globals[256];
	bool      readable[256];
	bool      writable[256];
	size_t    size;
} global_array;

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
		EXPRESSION_ELEMENT
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
		DEFINITION_CONST_CUSTOM,
		DEFINITION_CONST_BASIC
	} kind;

	union {
		function_id function;
		global_id   global;
		type_id     type;
	};
} definition;

typedef struct member {
	name_id  name;
	type_ref type;
	token    value;
} member;

typedef struct members {
	member m[MAX_MEMBERS];
	size_t size;
} members;

typedef struct type {
	attribute_list attributes;
	name_id        name;
	bool           built_in;
	members members;
	type_id  base;
	uint32_t array_size;
} type;

static void allocate_globals(void);
void _compile_function_block(opcodes *code, struct statement *block);
static variable allocate_variable(type_ref type, variable_kind kind);

void _functions_init(void);
static function_id add_function(name_id name);
static function_id find_function(name_id name);
function *_get_function(function_id function);

void _globals_init(void);
static global_id add_global(type_id type, attribute_list attributes, name_id name);
static global_id add_global_with_value(type_id type, attribute_list attributes, name_id name, global_value value);
static global *find_global(name_id name);
static global *get_global(global_id id);
static void assign_global_var(global_id id, uint64_t var_index);

void _names_init(void);
static name_id add_name(char *name);
char *_get_name(name_id index);

void _parse(const char *filename, tokens *tokens);
static token tokens_get(tokens *arr, size_t index);
tokens _tokenize(const char *filename, const char *source);

void _resolve_types(void);
static void init_type_ref(type_ref *t, name_id name);
void _types_init(void);
static type_id add_type(name_id name);
static type_id add_full_type(type *t);
static type_id find_type_by_name(name_id name);
static type_id find_type_by_ref(type_ref *t);
static type *get_type(type_id t);

static void kong_log(log_level_t level, const char *format, ...) {
	va_list args;
	va_start(args, format);
	kong_log_args(level, format, args);
	va_end(args);
}

static void kong_log_args(log_level_t level, const char *format, va_list args) {
#ifdef WIN32
	{
		char buffer[4096];
		vsnprintf(buffer, 4090, format, args);
		strcat(buffer, "\r\n");
		OutputDebugStringA(buffer);
	}
#endif

	{
		char buffer[4096];
		vsnprintf(buffer, 4090, format, args);
		strcat(buffer, "\n");
		fprintf(level == LOG_LEVEL_INFO ? stdout : stderr, "%s", buffer);
	}
}

static void debug_break(void) {
#ifndef NDEBUG
#if defined(_MSC_VER)
	__debugbreak();
#elif defined(__clang__)
	__builtin_debugtrap();
#else
#if defined(__aarch64__)
	__asm__ volatile(".inst 0xd4200000");
#elif defined(__x86_64__)
	__asm__ volatile("int $0x03");
#else
	kong_log(LOG_LEVEL_WARNING, "Oh no, debug_break is not implemented for the current compiler and CPU.");
#endif
#endif
#endif
}

static void error_args(debug_context context, const char *message, va_list args) {
	char buffer[4096];
	if (context.filename != NULL) {
		sprintf(buffer, "In column %i at line %i in %s: ", context.column + 1, context.line + 1, context.filename);
	}
	else {
		sprintf(buffer, "In column %i at line %i: ", context.column + 1, context.line + 1);
	}
	strcat(buffer, message);
	kong_log_args(LOG_LEVEL_ERROR, buffer, args);
	debug_break();
	exit(1);
}

static void error_args_no_context(const char *message, va_list args) {
	kong_log_args(LOG_LEVEL_ERROR, message, args);
	exit(1);
}

static void error(debug_context context, const char *message, ...) {
	va_list args;
	va_start(args, message);
	error_args(context, message, args);
	va_end(args);
}

static void error_no_context(const char *message, ...) {
	va_list args;
	va_start(args, message);
	error_args_no_context(message, args);
	va_end(args);
}

static void check_function(bool test, debug_context context, const char *message, ...) {
	if (!test) {
		va_list args;
		va_start(args, message);
		error_args(context, message, args);
		va_end(args);
	}
}

////

typedef enum modifier {
	MODIFIER_IN,
	// Out,
} modifier_t;

typedef enum mode {
	MODE_SELECT,
	MODE_NUMBER,
	// MODE_STRING,
	MODE_OPERATOR,
	MODE_IDENTIFIER,
	MODE_LINE_COMMENT,
	MODE_COMMENT
} mode;

typedef struct modifiers {
	modifier_t m[16];
	size_t     size;
} modifiers_t;

typedef struct tokenizer_state {
	const char *iterator;
	char        next;
	char        next_next;
	int         line, column;
	bool        line_end;
} tokenizer_state;

typedef struct block_ids {
	uint64_t start;
	uint64_t end;
} block_ids;

typedef struct state {
	tokens       *tokens;
	size_t        index;
	debug_context context;
} state_t;

typedef struct tokenizer_buffer {
	char  *buf;
	size_t current_size;
	size_t max_size;
	int    column, line;
} tokenizer_buffer;

typedef struct prefix {
	char   str[5];
	size_t size;
} prefix;

struct {
	char   *key;
	name_id value;
} *names_hash = NULL;

allocated_global __allocated_globals[1024];
size_t           __allocated_globals_size = 0;
static const char all_names[1024 * 1024];
static uint64_t next_variable_id = 1;
static variable all_variables[1024 * 1024];
static function   *functions           = NULL;
static function_id functions_size      = 128;
static function_id next_function_index = 0;
static global    globals[1024];
static global_id        globals_size = 0;
static char   *names       = NULL;
static size_t  names_size  = 1024 * 1024;
static name_id        names_index = 1;
static statement statements_buffer[8192];
static int statement_index = 0;
static expression experessions_buffer[8192];
static int expression_index = 0;
static type   *types           = NULL;
static type_id types_size      = 1024;
static type_id        next_type_index = 0;
type_id _void_id;
type_id _float_id;
type_id _int_id;
type_id _uint_id;
type_id _bool_id;
static type_id function_type_id;

static definition  parse_definition(state_t *state);
static statement  *parse_statement(state_t *state, block *parent_block);
static expression *parse_expression(state_t *state);
static definition parse_struct(state_t *state);
static definition parse_function(state_t *state);
static definition parse_const(state_t *state, attribute_list attributes);
static void resolve_types_in_expression(statement *parent, expression *e);

static allocated_global find_allocated_global(name_id name) {
	for (size_t i = 0; i < __allocated_globals_size; ++i) {
		if (name == __allocated_globals[i].g->name) {
			return __allocated_globals[i];
		}
	}

	allocated_global a;
	a.g           = NULL;
	a.variable_id = 0;
	return a;
}

static variable find_local_var(block *b, name_id name) {
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

static variable find_variable(block *parent, name_id name) {
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
			error(context, "Variable %s not found", _get_name(name));

			variable v;
			v.index = 0;
			return v;
		}
	}
	else {
		return local_var;
	}
}

static variable allocate_variable(type_ref type, variable_kind kind) {
	variable v;
	v.index                = next_variable_id;
	v.type                 = type;
	v.kind                 = kind;
	all_variables[v.index] = v;
	++next_variable_id;
	return v;
}

static opcode *emit_op(opcodes *code, opcode *o) {
	assert(code->size + o->size < OPCODES_SIZE);

	uint8_t *location = &code->o[code->size];

	memcpy(&code->o[code->size], o, o->size);

	code->size += o->size;

	return (opcode *)location;
}

static variable emit_expression(opcodes *code, block *parent, expression *e) {
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
			t.type              = _bool_id;
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
			case EXPRESSION_MEMBER: {
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

				while (of->kind == EXPRESSION_ELEMENT || of->kind == EXPRESSION_MEMBER) {
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
		t.type     = _float_id;
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
		t.type     = _float_id;
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
		t.type     = _int_id;
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
	case EXPRESSION_MEMBER: {
		opcode o;
		o.type = OPCODE_LOAD_ACCESS_LIST;
		o.size = OP_SIZE(o, op_load_access_list);

		variable v               = allocate_variable(e->type, VARIABLE_INTERNAL);
		o.op_load_access_list.to = v;

		expression *of = e;

		access   access_list[64];
		uint32_t access_list_size = 0;

		while (of->kind == EXPRESSION_ELEMENT || of->kind == EXPRESSION_MEMBER) {
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
				t.type            = _bool_id;
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
				t.type             = _bool_id;
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
					t.type             = _bool_id;
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

static void allocate_globals(void) {
	for (global_id i = 0; get_global(i) != NULL && get_global(i)->type != NO_TYPE; ++i) {
		global *g = get_global(i);

		type_ref t;
		init_type_ref(&t, NO_NAME);
		t.type                                                = g->type;
		variable v                                            = allocate_variable(t, VARIABLE_GLOBAL);
		__allocated_globals[__allocated_globals_size].g           = g;
		__allocated_globals[__allocated_globals_size].variable_id = v.index;
		__allocated_globals_size += 1;

		assign_global_var(i, v.index);
	}
}

void _compile_function_block(opcodes *code, struct statement *block) {
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

static void add_func_int(char *name) {
	function_id func = add_function(add_name(name));
	function   *f    = _get_function(func);
	init_type_ref(&f->return_type, add_name("int"));
	f->return_type.type = find_type_by_ref(&f->return_type);
	f->parameters_size  = 0;
	f->block            = NULL;
}

static void add_func_float(char *name) {
	function_id func = add_function(add_name(name));
	function   *f    = _get_function(func);
	init_type_ref(&f->return_type, add_name("float"));
	f->return_type.type = find_type_by_ref(&f->return_type);
	f->parameters_size  = 0;
	f->block            = NULL;
}

static void add_func_uint(char *name) {
	function_id func = add_function(add_name(name));
	function   *f    = _get_function(func);
	init_type_ref(&f->return_type, add_name("uint"));
	f->return_type.type = find_type_by_ref(&f->return_type);
	f->parameters_size  = 0;
	f->block            = NULL;
}

static void add_func_float_float(char *name) {
	function_id func = add_function(add_name(name));
	function   *f    = _get_function(func);
	init_type_ref(&f->return_type, add_name("float"));
	f->return_type.type   = find_type_by_ref(&f->return_type);
	f->parameter_names[0] = add_name("a");
	init_type_ref(&f->parameter_types[0], add_name("float"));
	f->parameter_types[0].type = find_type_by_ref(&f->parameter_types[0]);
	f->parameters_size         = 1;
	f->block                   = NULL;
}

void _functions_init(void) {
	function     *new_functions = realloc(functions, functions_size * sizeof(function));
	debug_context context       = {0};
	check(new_functions != NULL, context, "Could not allocate functions");
	functions           = new_functions;
	next_function_index = 0;

	{
		function_id func = add_function(add_name("float"));
		function   *f    = _get_function(func);
		init_type_ref(&f->return_type, add_name("float"));
		f->return_type.type   = find_type_by_ref(&f->return_type);
		f->parameter_names[0] = add_name("x");
		init_type_ref(&f->parameter_types[0], add_name("float"));
		f->parameter_types[0].type = find_type_by_ref(&f->parameter_types[0]);

		f->parameters_size = 1;

		f->block = NULL;
	}

	{
		function_id func = add_function(add_name("int"));
		function   *f    = _get_function(func);
		init_type_ref(&f->return_type, add_name("int"));
		f->return_type.type = find_type_by_ref(&f->return_type);

		f->parameter_names[0] = add_name("x");
		init_type_ref(&f->parameter_types[0], add_name("int"));
		f->parameter_types[0].type = find_type_by_ref(&f->parameter_types[0]);

		f->parameters_size = 1;

		f->block = NULL;
	}

	{
		function_id func = add_function(add_name("uint"));
		function   *f    = _get_function(func);
		init_type_ref(&f->return_type, add_name("uint"));
		f->return_type.type = find_type_by_ref(&f->return_type);

		f->parameter_names[0] = add_name("x");
		init_type_ref(&f->parameter_types[0], add_name("uint"));
		f->parameter_types[0].type = find_type_by_ref(&f->parameter_types[0]);

		f->parameters_size = 1;

		f->block = NULL;
	}

	{
		function_id func = add_function(add_name("bool"));
		function   *f    = _get_function(func);
		init_type_ref(&f->return_type, add_name("bool"));
		f->return_type.type = find_type_by_ref(&f->return_type);

		f->parameter_names[0] = add_name("x");
		init_type_ref(&f->parameter_types[0], add_name("bool"));
		f->parameter_types[0].type = find_type_by_ref(&f->parameter_types[0]);

		f->parameters_size = 1;

		f->block = NULL;
	}

	add_func_float_float("print");
	// add_func_float_float("sin");
	// add_func_float_float("cos");
}

static void grow_functions_if_needed(uint64_t size) {
	while (size >= functions_size) {
		functions_size *= 2;
		function     *new_functions = realloc(functions, functions_size * sizeof(function));
		debug_context context       = {0};
		check(new_functions != NULL, context, "Could not allocate functions");
		functions = new_functions;
	}
}

static function_id add_function(name_id name) {
	grow_functions_if_needed(next_function_index + 1);

	function_id f = next_function_index;
	++next_function_index;

	functions[f].name                        = name;
	functions[f].attributes.attributes_count = 0;
	init_type_ref(&functions[f].return_type, NO_NAME);
	functions[f].parameters_size = 0;
	memset(functions[f].parameter_attributes, 0, sizeof(functions[f].parameter_attributes));
	functions[f].block = NULL;
	memset(functions[f].code.o, 0, sizeof(functions[f].code.o));
	functions[f].code.size                  = 0;

	return f;
}

function *_get_function(function_id function) {
	if (function >= next_function_index) {
		return NULL;
	}
	return &functions[function];
}

void _globals_init(void) {
	global_value int_value;
	int_value.kind = GLOBAL_VALUE_INT;

	attribute_list attributes = {0};

	// int_value.value.ints[0] = 0;
	// add_global_with_value(_float_id, attributes, add_name("COMPARE_ALWAYS"), int_value);

	// global_value uint_value;
	// uint_value.kind = GLOBAL_VALUE_UINT;

	// uint_value.value.uints[0] = 0;
	// add_global_with_value(_uint_id, attributes, add_name("TEXTURE_FORMAT_R8_UNORM"), uint_value);
}

static global_id add_global(type_id type, attribute_list attributes, name_id name) {
	uint32_t index            = globals_size;
	globals[index].name       = name;
	globals[index].type       = type;
	globals[index].var_index  = 0;
	globals[index].value.kind = GLOBAL_VALUE_NONE;
	globals[index].attributes = attributes;
	globals[index].usage      = 0;
	globals_size += 1;
	return index;
}

static global_id add_global_with_value(type_id type, attribute_list attributes, name_id name, global_value value) {
	uint32_t index            = globals_size;
	globals[index].name       = name;
	globals[index].type       = type;
	globals[index].var_index  = 0;
	globals[index].value      = value;
	globals[index].attributes = attributes;
	globals[index].usage      = 0;
	globals_size += 1;
	return index;
}

static global *find_global(name_id name) {
	for (uint32_t i = 0; i < globals_size; ++i) {
		if (globals[i].name == name) {
			return &globals[i];
		}
	}

	return NULL;
}

static global *get_global(global_id id) {
	if (id >= globals_size) {
		return NULL;
	}

	return &globals[id];
}

static void assign_global_var(global_id id, uint64_t var_index) {
	debug_context context = {0};
	check(id < globals_size, context, "Encountered a global with a weird id");
	globals[id].var_index = var_index;
}

void _names_init(void) {
	char         *new_names = realloc(names, names_size);
	debug_context context   = {0};
	check(new_names != NULL, context, "Could not allocate names");
	names    = new_names;
	names[0] = 0; // make NO_NAME a proper string

	sh_new_arena(names_hash); // TODO: Get rid of this by using indices internally in the hash-map so it can survive grow_names_if_needed
}

static void grow_names_if_needed(size_t size) {
	while (size >= names_size) {
		names_size *= 2;
		char         *new_names = realloc(names, names_size);
		debug_context context   = {0};
		check(new_names != NULL, context, "Could not allocate names");
		names = new_names;
	}
}

static name_id add_name(char *name) {
	ptrdiff_t old_id_index = shgeti(names_hash, name);

	if (old_id_index >= 0) {
		return names_hash[old_id_index].value;
	}

	size_t length = strlen(name);

	grow_names_if_needed(names_index + length + 1);

	name_id id = names_index;

	memcpy(&names[id], name, length);
	names[id + length] = 0;

	names_index += length + 1;

	shput(names_hash, &names[id], id);

	return id;
}

char *_get_name(name_id id) {
	debug_context context = {0};
	check(id < names_index, context, "Encountered a weird name id");
	return &names[id];
}

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

void _parse(const char *filename, tokens *tokens) {
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

// static void modifiers_init(modifiers_t *modifiers) {
//	modifiers->size = 0;
// }

// static void modifiers_add(modifiers_t *modifiers, modifier_t modifier) {
//	modifiers->m[modifiers->size] = modifier;
//	modifiers->size += 1;
// }

static definition parse_definition(state_t *state) {
	attribute_list  attributes = {0};

	switch (current(state).kind) {
	case TOKEN_STRUCT: {
		definition structy                 = parse_struct(state);
		get_type(structy.type)->attributes = attributes;
		return structy;
	}
	case TOKEN_FUNCTION: {
		definition d  = parse_function(state);
		function  *f  = _get_function(d.function);
		f->attributes = attributes;
		return d;
	}
	case TOKEN_CONST: {
		definition d = parse_const(state, attributes);
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
	function *f        = _get_function(d.function);
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

	if (type_name == NO_NAME) {
		debug_context context = {0};
		check(type != NO_TYPE, context, "Const has no type");
		d.kind   = DEFINITION_CONST_CUSTOM;
		d.global = add_global(type, attributes, name.identifier);
	}
	else if (type_name == add_name("float")) {
		debug_context context = {0};
		check(value != NULL, context, "const float requires an initialization value");
		check(value->kind == EXPRESSION_FLOAT || value->kind == EXPRESSION_INT, context, "const float requires a number");

		global_value float_value;
		float_value.kind = GLOBAL_VALUE_FLOAT;

		float_value.value.floats[0] = (float)value->number;

		d.kind   = DEFINITION_CONST_BASIC;
		d.global = add_global_with_value(_float_id, attributes, name.identifier, float_value);
	}
	else {
		debug_context context = {0};
		error(context, "Unsupported global");
	}

	return d;
}

static token tokens_get(tokens *tokens, size_t index) {
	debug_context context = {0};
	check(tokens->current_size > index, context, "Token index out of bounds");
	return tokens->t[index];
}

static bool is_num(char ch, char chch) {
	return (ch >= '0' && ch <= '9') || (ch == '-' && chch >= '0' && chch <= '9');
}

static bool is_op(char ch) {
	return ch == '&' || ch == '|' || ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '=' || ch == '!' || ch == '<' || ch == '>' || ch == '%' ||
	       ch == '^';
}

static bool is_whitespace(char ch) {
	return ch == ' ' || (ch >= 9 && ch <= 13);
}

static void tokenizer_state_init(debug_context *context, tokenizer_state *state, const char *source) {
	state->line = state->column = 0;
	state->iterator             = source;
	state->next                 = *state->iterator;
	if (*state->iterator != 0) {
		state->iterator += 1;
	}
	state->next_next = *state->iterator;
	state->line_end  = false;

	context->column = 0;
	context->line   = 0;
}

static void tokenizer_state_advance(debug_context *context, tokenizer_state *state) {
	state->next = state->next_next;
	if (*state->iterator != 0) {
		state->iterator += 1;
	}
	state->next_next = *state->iterator;

	if (state->line_end) {
		state->line_end = false;
		state->line += 1;
		state->column = 0;
	}
	else {
		state->column += 1;
	}

	if (state->next == '\n') {
		state->line_end = true;
	}

	context->column = state->column;
	context->line   = state->line;
}

static void tokenizer_buffer_init(tokenizer_buffer *buffer) {
	buffer->max_size     = 1024 * 1024;
	buffer->buf          = (char *)malloc(buffer->max_size);
	buffer->current_size = 0;
	buffer->column = buffer->line = 0;
}

static void tokenizer_buffer_reset(tokenizer_buffer *buffer, tokenizer_state *state) {
	buffer->current_size = 0;
	buffer->column       = state->column;
	buffer->line         = state->line;
}

static void tokenizer_buffer_add(tokenizer_buffer *buffer, char ch) {
	debug_context context = {0};
	check(buffer->current_size < buffer->max_size, context, "Token buffer is too small");
	buffer->buf[buffer->current_size] = ch;
	buffer->current_size += 1;
}

static bool tokenizer_buffer_equals(tokenizer_buffer *buffer, const char *str) {
	buffer->buf[buffer->current_size] = 0;
	return strcmp(buffer->buf, str) == 0;
}

static name_id tokenizer_buffer_to_name(tokenizer_buffer *buffer) {
	debug_context context = {0};
	check(buffer->current_size < buffer->max_size, context, "Token buffer is too small");
	buffer->buf[buffer->current_size] = 0;
	buffer->current_size += 1;
	return add_name(buffer->buf);
}

static double tokenizer_buffer_parse_number(tokenizer_buffer *buffer) {
	buffer->buf[buffer->current_size] = 0;
	return strtod(buffer->buf, NULL);
}

static token token_create(int kind, tokenizer_state *state) {
	token token;
	token.kind   = kind;
	token.column = state->column;
	token.line   = state->line;
	return token;
}

static void tokens_init(tokens *tokens) {
	tokens->max_size     = 1024 * 1024;
	tokens->t            = malloc(tokens->max_size * sizeof(token));
	tokens->current_size = 0;
}

static void tokens_add(tokens *tokens, token token) {
	tokens->t[tokens->current_size] = token;
	tokens->current_size += 1;
	debug_context context = {0};
	check(tokens->current_size <= tokens->max_size, context, "Out of tokens");
}

static void tokens_add_identifier(tokenizer_state *state, tokens *tokens, tokenizer_buffer *buffer) {
	token token;

	if (tokenizer_buffer_equals(buffer, "true")) {
		token         = token_create(TOKEN_BOOLEAN, state);
		token.boolean = true;
	}
	else if (tokenizer_buffer_equals(buffer, "false")) {
		token         = token_create(TOKEN_BOOLEAN, state);
		token.boolean = false;
	}
	else if (tokenizer_buffer_equals(buffer, "if")) {
		token = token_create(TOKEN_IF, state);
	}
	else if (tokenizer_buffer_equals(buffer, "else")) {
		token = token_create(TOKEN_ELSE, state);
	}
	else if (tokenizer_buffer_equals(buffer, "while")) {
		token = token_create(TOKEN_WHILE, state);
	}
	else if (tokenizer_buffer_equals(buffer, "do")) {
		token = token_create(TOKEN_DO, state);
	}
	else if (tokenizer_buffer_equals(buffer, "for")) {
		token = token_create(TOKEN_FOR, state);
	}
	else if (tokenizer_buffer_equals(buffer, "in")) {
		token = token_create(TOKEN_IN, state);
	}
	else if (tokenizer_buffer_equals(buffer, "struct")) {
		token = token_create(TOKEN_STRUCT, state);
	}
	else if (tokenizer_buffer_equals(buffer, "fun")) {
		token = token_create(TOKEN_FUNCTION, state);
	}
	else if (tokenizer_buffer_equals(buffer, "var")) {
		token = token_create(TOKEN_VAR, state);
	}
	else if (tokenizer_buffer_equals(buffer, "const")) {
		token = token_create(TOKEN_CONST, state);
	}
	else if (tokenizer_buffer_equals(buffer, "return")) {
		token = token_create(TOKEN_RETURN, state);
	}
	else {
		token            = token_create(TOKEN_IDENTIFIER, state);
		token.identifier = tokenizer_buffer_to_name(buffer);
	}

	token.column = buffer->column;
	token.line   = buffer->line;
	tokens_add(tokens, token);
}

tokens _tokenize(const char *filename, const char *source) {
	mode mode           = MODE_SELECT;
	bool number_has_dot = false;

	tokens tokens;
	tokens_init(&tokens);

	debug_context context = {0};
	context.filename      = filename;

	tokenizer_state state;
	tokenizer_state_init(&context, &state, source);

	tokenizer_buffer buffer;
	tokenizer_buffer_init(&buffer);

	for (;;) {
		if (state.next == 0) {
			switch (mode) {
			case MODE_IDENTIFIER:
				tokens_add_identifier(&state, &tokens, &buffer);
				break;
			case MODE_NUMBER: {
				token token  = token_create(number_has_dot ? TOKEN_FLOAT : TOKEN_INT, &state);
				token.number = tokenizer_buffer_parse_number(&buffer);
				tokens_add(&tokens, token);
				break;
			}
			case MODE_SELECT:
			case MODE_LINE_COMMENT:
				break;
			// case MODE_STRING:
			//	error("Unclosed string", state.column, state.line);
			case MODE_OPERATOR:
				error(context, "File ends with an operator");
			case MODE_COMMENT:
				error(context, "Unclosed comment");
			}

			tokens_add(&tokens, token_create(TOKEN_NONE, &state));
			////
			free(buffer.buf);
			////
			return tokens;
		}
		else {
			char ch = (char)state.next;
			switch (mode) {
			case MODE_SELECT: {
				if (ch == '/') {
					if (state.next_next >= 0) {
						char chch = state.next_next;
						switch (chch) {
						case '/':
							mode = MODE_LINE_COMMENT;
							break;
						case '*':
							mode = MODE_COMMENT;
							break;
						default:
							tokenizer_buffer_reset(&buffer, &state);
							tokenizer_buffer_add(&buffer, ch);
							mode = MODE_OPERATOR;
						}
					}
				}
				else if (is_num(ch, state.next_next)) {
					mode           = MODE_NUMBER;
					number_has_dot = false;
					tokenizer_buffer_reset(&buffer, &state);
					tokenizer_buffer_add(&buffer, ch);
				}
				else if (is_op(ch)) {
					mode = MODE_OPERATOR;
					tokenizer_buffer_reset(&buffer, &state);
					tokenizer_buffer_add(&buffer, ch);
				}
				else if (is_whitespace(ch)) {
				}
				else if (ch == '(') {
					tokens_add(&tokens, token_create(TOKEN_LEFT_PAREN, &state));
				}
				else if (ch == ')') {
					tokens_add(&tokens, token_create(TOKEN_RIGHT_PAREN, &state));
				}
				else if (ch == '{') {
					tokens_add(&tokens, token_create(TOKEN_LEFT_CURLY, &state));
				}
				else if (ch == '}') {
					tokens_add(&tokens, token_create(TOKEN_RIGHT_CURLY, &state));
				}
				else if (ch == '#') {
					tokens_add(&tokens, token_create(TOKEN_HASH, &state));
				}
				else if (ch == '[') {
					tokens_add(&tokens, token_create(TOKEN_LEFT_SQUARE, &state));
				}
				else if (ch == ']') {
					tokens_add(&tokens, token_create(TOKEN_RIGHT_SQUARE, &state));
				}
				else if (ch == ';') {
					tokens_add(&tokens, token_create(TOKEN_SEMICOLON, &state));
				}
				else if (ch == '.') {
					tokens_add(&tokens, token_create(TOKEN_DOT, &state));
				}
				else if (ch == ':') {
					tokens_add(&tokens, token_create(TOKEN_COLON, &state));
				}
				else if (ch == ',') {
					tokens_add(&tokens, token_create(TOKEN_COMMA, &state));
				}
				else if (ch == '"' || ch == '\'') {
					// mode = MODE_STRING;
					// tokenizer_buffer_reset(&buffer, &state);
					error(context, "Strings are not supported");
				}
				else {
					mode = MODE_IDENTIFIER;
					tokenizer_buffer_reset(&buffer, &state);
					tokenizer_buffer_add(&buffer, ch);
				}
				tokenizer_state_advance(&context, &state);
				break;
			}
			case MODE_LINE_COMMENT: {
				if (ch == '\n') {
					mode = MODE_SELECT;
				}
				tokenizer_state_advance(&context, &state);
				break;
			}
			case MODE_COMMENT: {
				if (ch == '*') {
					if (state.next_next >= 0) {
						char chch = (char)state.next_next;
						if (chch == '/') {
							mode = MODE_SELECT;
							tokenizer_state_advance(&context, &state);
						}
					}
				}
				tokenizer_state_advance(&context, &state);
				break;
			}
			case MODE_NUMBER: {
				if (is_num(ch, 0) || ch == '.') {
					if (ch == '.') {
						number_has_dot = true;
					}
					tokenizer_buffer_add(&buffer, ch);
					tokenizer_state_advance(&context, &state);
				}
				else {
					token token  = token_create(number_has_dot ? TOKEN_FLOAT : TOKEN_INT, &state);
					token.number = tokenizer_buffer_parse_number(&buffer);
					tokens_add(&tokens, token);
					mode = MODE_SELECT;
				}
				break;
			}
			case MODE_OPERATOR: {
				char long_op[3];
				long_op[0] = 0;
				if (buffer.current_size == 1) {
					long_op[0] = buffer.buf[0];
					long_op[1] = ch;
					long_op[2] = 0;
				}

				if (strcmp(long_op, "==") == 0 || strcmp(long_op, "!=") == 0 || strcmp(long_op, "<=") == 0 || strcmp(long_op, ">=") == 0 ||
				    strcmp(long_op, "||") == 0 || strcmp(long_op, "&&") == 0 || strcmp(long_op, "->") == 0 || strcmp(long_op, "-=") == 0 ||
				    strcmp(long_op, "+=") == 0 || strcmp(long_op, "/=") == 0 || strcmp(long_op, "*=") == 0 || strcmp(long_op, "<<") == 0 ||
				    strcmp(long_op, ">>") == 0) {
					tokenizer_buffer_add(&buffer, ch);
					tokenizer_state_advance(&context, &state);
				}

				if (tokenizer_buffer_equals(&buffer, "==")) {
					token token = token_create(TOKEN_OPERATOR, &state);
					token.op    = OPERATOR_EQUALS;
					tokens_add(&tokens, token);
				}
				else if (tokenizer_buffer_equals(&buffer, "!=")) {
					token token = token_create(TOKEN_OPERATOR, &state);
					token.op    = OPERATOR_NOT_EQUALS;
					tokens_add(&tokens, token);
				}
				else if (tokenizer_buffer_equals(&buffer, ">")) {
					token token = token_create(TOKEN_OPERATOR, &state);
					token.op    = OPERATOR_GREATER;
					tokens_add(&tokens, token);
				}
				else if (tokenizer_buffer_equals(&buffer, ">=")) {
					token token = token_create(TOKEN_OPERATOR, &state);
					token.op    = OPERATOR_GREATER_EQUAL;
					tokens_add(&tokens, token);
				}
				else if (tokenizer_buffer_equals(&buffer, "<")) {
					token token = token_create(TOKEN_OPERATOR, &state);
					token.op    = OPERATOR_LESS;
					tokens_add(&tokens, token);
				}
				else if (tokenizer_buffer_equals(&buffer, "<=")) {
					token token = token_create(TOKEN_OPERATOR, &state);
					token.op    = OPERATOR_LESS_EQUAL;
					tokens_add(&tokens, token);
				}
				else if (tokenizer_buffer_equals(&buffer, "-=")) {
					token token = token_create(TOKEN_OPERATOR, &state);
					token.op    = OPERATOR_MINUS_ASSIGN;
					tokens_add(&tokens, token);
				}
				else if (tokenizer_buffer_equals(&buffer, "+=")) {
					token token = token_create(TOKEN_OPERATOR, &state);
					token.op    = OPERATOR_PLUS_ASSIGN;
					tokens_add(&tokens, token);
				}
				else if (tokenizer_buffer_equals(&buffer, "/=")) {
					token token = token_create(TOKEN_OPERATOR, &state);
					token.op    = OPERATOR_DIVIDE_ASSIGN;
					tokens_add(&tokens, token);
				}
				else if (tokenizer_buffer_equals(&buffer, "*=")) {
					token token = token_create(TOKEN_OPERATOR, &state);
					token.op    = OPERATOR_MULTIPLY_ASSIGN;
					tokens_add(&tokens, token);
				}
				else if (tokenizer_buffer_equals(&buffer, "-")) {
					token token = token_create(TOKEN_OPERATOR, &state);
					token.op    = OPERATOR_MINUS;
					tokens_add(&tokens, token);
				}
				else if (tokenizer_buffer_equals(&buffer, "+")) {
					token token = token_create(TOKEN_OPERATOR, &state);
					token.op    = OPERATOR_PLUS;
					tokens_add(&tokens, token);
				}
				else if (tokenizer_buffer_equals(&buffer, "/")) {
					token token = token_create(TOKEN_OPERATOR, &state);
					token.op    = OPERATOR_DIVIDE;
					tokens_add(&tokens, token);
				}
				else if (tokenizer_buffer_equals(&buffer, "*")) {
					token token = token_create(TOKEN_OPERATOR, &state);
					token.op    = OPERATOR_MULTIPLY;
					tokens_add(&tokens, token);
				}
				else if (tokenizer_buffer_equals(&buffer, "!")) {
					token token = token_create(TOKEN_OPERATOR, &state);
					token.op    = OPERATOR_NOT;
					tokens_add(&tokens, token);
				}
				else if (tokenizer_buffer_equals(&buffer, "||")) {
					token token = token_create(TOKEN_OPERATOR, &state);
					token.op    = OPERATOR_OR;
					tokens_add(&tokens, token);
				}
				else if (tokenizer_buffer_equals(&buffer, "^")) {
					token token = token_create(TOKEN_OPERATOR, &state);
					token.op    = OPERATOR_BITWISE_XOR;
					tokens_add(&tokens, token);
				}
				else if (tokenizer_buffer_equals(&buffer, "&")) {
					token token = token_create(TOKEN_OPERATOR, &state);
					token.op    = OPERATOR_BITWISE_AND;
					tokens_add(&tokens, token);
				}
				else if (tokenizer_buffer_equals(&buffer, "|")) {
					token token = token_create(TOKEN_OPERATOR, &state);
					token.op    = OPERATOR_BITWISE_OR;
					tokens_add(&tokens, token);
				}
				else if (tokenizer_buffer_equals(&buffer, "<<")) {
					token token = token_create(TOKEN_OPERATOR, &state);
					token.op    = OPERATOR_LEFT_SHIFT;
					tokens_add(&tokens, token);
				}
				else if (tokenizer_buffer_equals(&buffer, ">>")) {
					token token = token_create(TOKEN_OPERATOR, &state);
					token.op    = OPERATOR_RIGHT_SHIFT;
					tokens_add(&tokens, token);
				}
				else if (tokenizer_buffer_equals(&buffer, "&&")) {
					token token = token_create(TOKEN_OPERATOR, &state);
					token.op    = OPERATOR_AND;
					tokens_add(&tokens, token);
				}
				else if (tokenizer_buffer_equals(&buffer, "%")) {
					token token = token_create(TOKEN_OPERATOR, &state);
					token.op    = OPERATOR_MOD;
					tokens_add(&tokens, token);
				}
				else if (tokenizer_buffer_equals(&buffer, "=")) {
					token token = token_create(TOKEN_OPERATOR, &state);
					token.op    = OPERATOR_ASSIGN;
					tokens_add(&tokens, token);
				}
				else {
					error(context, "Weird operator");
				}

				mode = MODE_SELECT;
				break;
			}
			/*case MODE_STRING: {
				if (ch == '"' || ch == '\'') {
				    token token = token_create(TOKEN_STRING, &state);
				    tokenizer_buffer_copy_to_string(&buffer, token.string);
				    token.column = buffer.column;
				    token.line = buffer.line;
				    tokens_add(&tokens, token);

				    tokenizer_state_advance(&state);
				    mode = MODE_SELECT;
				}
				else {
				    tokenizer_buffer_add(&buffer, ch);
				    tokenizer_state_advance(&state);
				}
				break;
			}*/
			case MODE_IDENTIFIER: {
				if (is_whitespace(ch) || is_op(ch) || ch == '(' || ch == ')' || ch == '{' || ch == '}' || ch == '[' || ch == ']' || ch == '"' || ch == '\'' ||
				    ch == ';' || ch == '.' || ch == ',' || ch == ':') {
					tokens_add_identifier(&state, &tokens, &buffer);
					mode = MODE_SELECT;
				}
				else {
					tokenizer_buffer_add(&buffer, ch);
					tokenizer_state_advance(&context, &state);
				}
				break;
			}
			}
		}
	}
}

static type_ref find_local_var_type(block *b, name_id name) {
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

static void resolve_types_in_element(statement *parent_block, expression *element) {
	resolve_types_in_expression(parent_block, element->element.of);
	resolve_types_in_expression(parent_block, element->element.element_index);

	type_id of_type = element->element.of->type.type;

	assert(of_type != NO_TYPE);

	type *of = get_type(of_type);

	if (of->array_size > 0) {
		element->type.type = of->base;
	}
	else {
		debug_context context = {0};
		error(context, "Indexed non-array %s", _get_name(of->name));
	}
}

static void resolve_types_in_member(statement *parent_block, expression *member) {
	resolve_types_in_expression(parent_block, member->member.of);

	type_id of_type     = member->member.of->type.type;
	name_id member_name = member->member.member_name;

	assert(of_type != NO_TYPE);

	type *of_struct = get_type(of_type);

	for (size_t i = 0; i < of_struct->members.size; ++i) {
		if (of_struct->members.m[i].name == member_name) {
			member->type = of_struct->members.m[i].type;
			return;
		}
	}

	debug_context context = {0};
	error(context, "Member %s not found", _get_name(member_name));
}

static bool types_compatible(type_id left, type_id right) {
	if (left == right) {
		return true;
	}
	if ((left == _int_id && right == _float_id) || (left == _float_id && right == _int_id)) {
		return true;
	}
	if ((left == _uint_id && right == _float_id) || (left == _float_id && right == _uint_id)) {
		return true;
	}
	if ((left == _uint_id && right == _int_id) || (left == _int_id && right == _uint_id)) {
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

	if (left == _int_id && right == _float_id) {
		return right_type;
	}
	if (left == _float_id && right == _int_id) {
		return left_type;
	}

	if (left == _uint_id && right == _float_id) {
		return right_type;
	}
	if (left == _float_id && right == _uint_id) {
		return left_type;
	}

	if (left == _uint_id && right == _int_id) {
		return right_type;
	}
	if (left == _int_id && right == _uint_id) {
		return left_type;
	}

	kong_log(LOG_LEVEL_WARNING, "Suspicious type upgrade");
	return left_type;
}

static void resolve_types_in_expression(statement *parent, expression *e) {
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
			e->type.type = _bool_id;
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
			if (types_compatible(left_type, right_type)) {
				e->type = upgrade_type(e->binary.left->type, e->binary.right->type);
			}
			else {
				debug_context context = {0};
				error(context, "Type mismatch %s vs %s", _get_name(get_type(left_type)->name), _get_name(get_type(right_type)->name));
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
				error(context, "Type mismatch %s vs %s", _get_name(get_type(left_type)->name), _get_name(get_type(right_type)->name));
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
				error(context, "Type mismatch %s vs %s", _get_name(get_type(left_type)->name), _get_name(get_type(right_type)->name));
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
			e->type.type = _bool_id;
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
		e->type.type = _bool_id;
		break;
	}
	case EXPRESSION_FLOAT: {
		e->type.type = _float_id;
		break;
	}
	case EXPRESSION_INT: {
		e->type.type = _int_id;
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
				error(context, "Variable %s not found", _get_name(e->variable));
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
		for (function_id i = 0; _get_function(i) != NULL; ++i) {
			function *f = _get_function(i);
			if (f->name == e->call.func_name) {
				e->type = f->return_type;
				break;
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
	}

	if (e->type.type == NO_TYPE) {
		debug_context context = {0};
		// const char *n = _get_name(e->call.func_name);
		error(context, "Could not resolve type");
	}
}

static void resolve_types_in_block(statement *parent, statement *block) {
	debug_context context = {0};
	check(block->kind == STATEMENT_BLOCK, context, "Malformed block");

	for (size_t i = 0; i < block->block.statements.size; ++i) {
		statement *s = block->block.statements.s[i];
		switch (s->kind) {
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
				error(context, "Could not find type %s for %s", _get_name(var_type_name), _get_name(var_name));
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

void _resolve_types(void) {
	for (type_id i = 0; get_type(i) != NULL; ++i) {
		type *s = get_type(i);
		for (size_t j = 0; j < s->members.size; ++j) {
			if (s->members.m[j].type.type == NO_TYPE) {
				name_id name              = s->members.m[j].type.unresolved.name;
				s->members.m[j].type.type = find_type_by_name(name);
				if (s->members.m[j].type.type == NO_TYPE) {
					debug_context context = {0};
					error(context, "Could not find type %s in %s", _get_name(name), _get_name(s->name));
				}
			}
		}
	}

	for (function_id i = 0; _get_function(i) != NULL; ++i) {
		function *f = _get_function(i);

		for (uint8_t parameter_index = 0; parameter_index < f->parameters_size; ++parameter_index) {
			if (f->parameter_types[parameter_index].type == NO_TYPE) {
				name_id parameter_type_name              = f->parameter_types[parameter_index].unresolved.name;
				f->parameter_types[parameter_index].type = find_type_by_name(parameter_type_name);
				if (f->parameter_types[parameter_index].type == NO_TYPE) {
					debug_context context = {0};
					error(context, "Could not find type %s for %s", _get_name(parameter_type_name), _get_name(f->name));
				}
			}
		}

		if (f->return_type.type == NO_TYPE) {
			f->return_type.type = find_type_by_ref(&f->return_type);

			if (f->return_type.type == NO_TYPE) {
				error_no_context("Could not find type %s for %s", _get_name(f->return_type.unresolved.name), _get_name(f->name));
			}
		}
	}

	for (function_id i = 0; _get_function(i) != NULL; ++i) {
		function *f = _get_function(i);

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

static void init_type_ref(type_ref *t, name_id name) {
	t->type                  = NO_TYPE;
	t->unresolved.name       = name;
	t->unresolved.array_size = 0;
}

void _types_init(void) {
	type         *new_types = realloc(types, types_size * sizeof(type));
	debug_context context   = {0};
	check(new_types != NULL, context, "Could not allocate types");
	types           = new_types;
	next_type_index = 0;

	_void_id                     = add_type(add_name("void"));
	get_type(_void_id)->built_in = true;

	_bool_id                      = add_type(add_name("bool"));
	get_type(_bool_id)->built_in  = true;
	_float_id                     = add_type(add_name("float"));
	get_type(_float_id)->built_in = true;
	_int_id                       = add_type(add_name("int"));
	get_type(_int_id)->built_in   = true;
	_uint_id                      = add_type(add_name("uint"));
	get_type(_uint_id)->built_in  = true;

	{
		function_type_id                     = add_type(add_name("fun"));
		get_type(function_type_id)->built_in = true;
	}
}

static void grow_types_if_needed(uint64_t size) {
	while (size >= types_size) {
		types_size *= 2;
		type         *new_types = realloc(types, types_size * sizeof(type));
		debug_context context   = {0};
		check(new_types != NULL, context, "Could not allocate types");
		types = new_types;
	}
}

static bool types_equal(type *a, type *b) {
	return a->name == b->name && a->attributes.attributes_count == 0 && b->attributes.attributes_count == 0 && a->members.size == 0 && b->members.size == 0 &&
	       a->built_in == b->built_in && a->array_size == b->array_size && a->base == b->base;
}

static type_id add_type(name_id name) {
	grow_types_if_needed(next_type_index + 1);

	type_id s = next_type_index;
	++next_type_index;

	types[s].name                        = name;
	types[s].attributes.attributes_count = 0;
	types[s].members.size                = 0;
	types[s].built_in                    = false;
	types[s].array_size                  = 0;
	types[s].base                        = NO_TYPE;

	return s;
}

static type_id add_full_type(type *t) {
	for (type_id type_index = 0; type_index < next_type_index; ++type_index) {
		if (types_equal(&types[type_index], t)) {
			return type_index;
		}
	}

	grow_types_if_needed(next_type_index + 1);

	type_id s = next_type_index;
	++next_type_index;

	types[s] = *t;

	return s;
}

static type_id find_type_by_name(name_id name) {
	debug_context context = {0};
	check(name != NO_NAME, context, "Attempted to find a no-name");
	for (type_id i = 0; i < next_type_index; ++i) {
		if (types[i].name == name) {
			return i;
		}
	}

	return NO_TYPE;
}

static type_id find_type_by_ref(type_ref *t) {
	if (t->type != NO_TYPE) {
		return t->type;
	}

	debug_context context = {0};
	check(t->unresolved.name != NO_NAME, context, "Attempted to find a no-name");

	type_id base_type_id = NO_TYPE;

	for (type_id i = 0; i < next_type_index; ++i) {
		if (types[i].name == t->unresolved.name) {
			if (types[i].array_size == t->unresolved.array_size) {
				return i;
			}
			base_type_id = i;
		}
	}

	if (base_type_id != NO_TYPE) {
		type_id new_type_id  = add_type(t->unresolved.name);
		type   *new_type     = get_type(new_type_id);
		new_type->array_size = t->unresolved.array_size;

		type *base_type    = get_type(base_type_id);
		new_type->base     = base_type_id;
		new_type->built_in = base_type->built_in;

		return new_type_id;
	}

	return NO_TYPE;
}

static type *get_type(type_id s) {
	if (s >= next_type_index) {
		return NULL;
	}
	return &types[s];
}
