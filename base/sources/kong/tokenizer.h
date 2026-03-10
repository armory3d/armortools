#pragma once

#include "names.h"

#include <stdbool.h>
#include <stddef.h>

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
		TOKEN_RETURN,
		TOKEN_DISCARD
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

token tokens_get(tokens *arr, size_t index);

tokens tokenize(const char *filename, const char *source);
