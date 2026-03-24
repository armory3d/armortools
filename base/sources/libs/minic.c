
// Minimal C interpreter

#include "minic.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ████████╗ ██████╗ ██╗  ██╗███████╗███╗   ██╗
// ╚══██╔══╝██╔═══██╗██║ ██╔╝██╔════╝████╗  ██║
//    ██║   ██║   ██║█████╔╝ █████╗  ██╔██╗ ██║
//    ██║   ██║   ██║██╔═██╗ ██╔══╝  ██║╚██╗██║
//    ██║   ╚██████╔╝██║  ██╗███████╗██║ ╚████║
//    ╚═╝    ╚═════╝ ╚═╝  ╚═╝╚══════╝╚═╝  ╚═══╝

typedef enum {
	TOK_INT,
	TOK_FLOAT,
	TOK_CHAR,
	TOK_DOUBLE,
	TOK_BOOL,
	TOK_RETURN,
	TOK_IF,
	TOK_ELSE,
	TOK_WHILE,
	TOK_FOR,
	TOK_BREAK,
	TOK_CONTINUE,
	TOK_STRUCT,
	TOK_TYPEDEF,
	TOK_ENUM,
	TOK_VOID,
	TOK_IDENT,
	TOK_NUMBER,
	TOK_CHAR_LIT,
	TOK_STR_LIT,
	TOK_LPAREN,
	TOK_RPAREN,
	TOK_LBRACE,
	TOK_RBRACE,
	TOK_LBRACKET,
	TOK_RBRACKET,
	TOK_SEMICOLON,
	TOK_COMMA,
	TOK_ASSIGN,
	TOK_PLUS_ASSIGN,
	TOK_MINUS_ASSIGN,
	TOK_MUL_ASSIGN,
	TOK_DIV_ASSIGN,
	TOK_EQ,
	TOK_NEQ,
	TOK_LT,
	TOK_GT,
	TOK_LE,
	TOK_GE,
	TOK_AND,
	TOK_OR,
	TOK_NOT,
	TOK_AMP,
	TOK_PLUS,
	TOK_MINUS,
	TOK_INC,
	TOK_DEC,
	TOK_STAR,
	TOK_SLASH,
	TOK_DOT,
	TOK_ARROW,
	TOK_EOF
} minic_tok_type_t;

typedef struct {
	minic_tok_type_t type;
	char             text[64];
	minic_val_t      val; // TOK_NUMBER, TOK_CHAR_LIT, TOK_STR_LIT
} minic_token_t;

typedef struct {
	const char   *src;
	int           pos;
	minic_token_t cur;
} minic_lexer_t;

static minic_u8 *minic_active_mem      = NULL;
static int      *minic_active_mem_used = NULL;

static void minic_lex_next(minic_lexer_t *l) {
	// Skip whitespace and comments
	for (;;) {
		while (l->src[l->pos] != '\0' && isspace((unsigned char)l->src[l->pos])) {
			l->pos++;
		}
		if (l->src[l->pos] == '/' && l->src[l->pos + 1] == '/') {
			while (l->src[l->pos] != '\0' && l->src[l->pos] != '\n') {
				l->pos++;
			}
			continue;
		}
		// Skip preprocessor directives
		if (l->src[l->pos] == '#') {
			while (l->src[l->pos] != '\0' && l->src[l->pos] != '\n') {
				l->pos++;
			}
			continue;
		}
		if (l->src[l->pos] == '/' && l->src[l->pos + 1] == '*') {
			l->pos += 2;
			while (l->src[l->pos] != '\0' && !(l->src[l->pos] == '*' && l->src[l->pos + 1] == '/')) {
				l->pos++;
			}
			if (l->src[l->pos] != '\0') {
				l->pos += 2;
			}
			continue;
		}
		break;
	}

	if (l->src[l->pos] == '\0') {
		l->cur.type = TOK_EOF;
		return;
	}

	char c = l->src[l->pos];

	if (c == '0' && (l->src[l->pos + 1] == 'x' || l->src[l->pos + 1] == 'X')) {
		l->pos += 2; // Consume '0x'
		unsigned int n = 0;
		while (isxdigit((unsigned char)l->src[l->pos])) {
			char h     = l->src[l->pos++];
			int  digit = (h >= '0' && h <= '9') ? h - '0' : (h >= 'a' && h <= 'f') ? h - 'a' + 10 : h - 'A' + 10;
			n          = n * 16 + digit;
		}
		l->cur.val  = minic_val_int((int)n);
		l->cur.type = TOK_NUMBER;
		return;
	}

	if (isdigit((unsigned char)c)) {
		double n = 0;
		while (isdigit((unsigned char)l->src[l->pos])) {
			n = n * 10 + (l->src[l->pos++] - '0');
		}
		bool is_float = false;
		if (l->src[l->pos] == '.') {
			l->pos++;
			double frac = 0.1;
			while (isdigit((unsigned char)l->src[l->pos])) {
				n += (l->src[l->pos++] - '0') * frac;
				frac *= 0.1;
			}
			is_float = true;
		}
		if (l->src[l->pos] == 'f' || l->src[l->pos] == 'F') {
			l->pos++;
			is_float = true;
		}
		if (is_float) {
			l->cur.val = minic_val_float((float)n);
		}
		else {
			l->cur.val = minic_val_int((int)n);
		}
		l->cur.type = TOK_NUMBER;
		return;
	}

	if (c == '"') {
		l->pos++; // Consume opening '"'
		// Allocate space in the active context's arena for the string
		int start = (*minic_active_mem_used + 7) & ~7; // Aligned start
		int wi    = start;
		while (l->src[l->pos] != '"' && l->src[l->pos] != '\0') {
			char ch;
			if (l->src[l->pos] == '\\') {
				l->pos++;
				switch (l->src[l->pos++]) {
				case 'n':
					ch = '\n';
					break;
				case 't':
					ch = '\t';
					break;
				case '0':
					ch = '\0';
					break;
				case '\\':
					ch = '\\';
					break;
				case '"':
					ch = '"';
					break;
				case 'r':
					ch = '\r';
					break;
				case '\n': // Line continuation: backslash-newline, skip both
					continue;
				case '\r': // Handle \r\n line endings
					if (l->src[l->pos] == '\n') {
						l->pos++;
					}
					continue;
				default:
					ch = '\0';
					break;
				}
			}
			else {
				ch = l->src[l->pos++];
			}
			minic_active_mem[wi++] = (minic_u8)ch;
		}
		minic_active_mem[wi++] = '\0';
		*minic_active_mem_used = (wi + 7) & ~7;
		if (l->src[l->pos] == '"') {
			l->pos++; // Consume closing '"'
		}
		l->cur.type = TOK_STR_LIT;
		// Store a real pointer into the context arena so the dispatcher passes it correctly
		l->cur.val = minic_val_ptr((void *)&minic_active_mem[start]);
		return;
	}

	if (c == '\'') {
		l->pos++; // Consume opening '
		int v = 0;
		if (l->src[l->pos] == '\\') {
			l->pos++;
			switch (l->src[l->pos++]) {
			case 'n':
				v = '\n';
				break;
			case 't':
				v = '\t';
				break;
			case '0':
				v = '\0';
				break;
			case '\\':
				v = '\\';
				break;
			case '\'':
				v = '\'';
				break;
			default:
				v = 0;
				break;
			}
		}
		else {
			v = (unsigned char)l->src[l->pos++];
		}
		l->pos++; // Consume closing '
		l->cur.type = TOK_CHAR_LIT;
		l->cur.val  = minic_val_int(v);
		return;
	}

	if (isalpha((unsigned char)c) || c == '_') {
		int i = 0;
		while (isalnum((unsigned char)l->src[l->pos]) || l->src[l->pos] == '_') {
			l->cur.text[i++] = l->src[l->pos++];
		}
		l->cur.text[i] = '\0';

		if (strcmp(l->cur.text, "int") == 0) {
			l->cur.type = TOK_INT;
			return;
		}
		if (strcmp(l->cur.text, "float") == 0) {
			l->cur.type = TOK_FLOAT;
			return;
		}
		if (strcmp(l->cur.text, "char") == 0) {
			l->cur.type = TOK_CHAR;
			return;
		}
		if (strcmp(l->cur.text, "double") == 0) {
			l->cur.type = TOK_DOUBLE;
			return;
		}
		if (strcmp(l->cur.text, "bool") == 0) {
			l->cur.type = TOK_BOOL;
			return;
		}
		if (strcmp(l->cur.text, "true") == 0) {
			l->cur.type = TOK_NUMBER;
			l->cur.val  = minic_val_int(1);
			return;
		}
		if (strcmp(l->cur.text, "false") == 0) {
			l->cur.type = TOK_NUMBER;
			l->cur.val  = minic_val_int(0);
			return;
		}
		if (strcmp(l->cur.text, "return") == 0) {
			l->cur.type = TOK_RETURN;
			return;
		}
		if (strcmp(l->cur.text, "if") == 0) {
			l->cur.type = TOK_IF;
			return;
		}
		if (strcmp(l->cur.text, "else") == 0) {
			l->cur.type = TOK_ELSE;
			return;
		}
		if (strcmp(l->cur.text, "while") == 0) {
			l->cur.type = TOK_WHILE;
			return;
		}
		if (strcmp(l->cur.text, "for") == 0) {
			l->cur.type = TOK_FOR;
			return;
		}
		if (strcmp(l->cur.text, "break") == 0) {
			l->cur.type = TOK_BREAK;
			return;
		}
		if (strcmp(l->cur.text, "continue") == 0) {
			l->cur.type = TOK_CONTINUE;
			return;
		}
		if (strcmp(l->cur.text, "struct") == 0) {
			l->cur.type = TOK_STRUCT;
			return;
		}
		if (strcmp(l->cur.text, "typedef") == 0) {
			l->cur.type = TOK_TYPEDEF;
			return;
		}
		if (strcmp(l->cur.text, "enum") == 0) {
			l->cur.type = TOK_ENUM;
			return;
		}
		if (strcmp(l->cur.text, "void") == 0) {
			l->cur.type = TOK_VOID;
			return;
		}
		l->cur.type = TOK_IDENT;
		return;
	}

	l->pos++;
	switch (c) {
	case '(':
		l->cur.type = TOK_LPAREN;
		return;
	case ')':
		l->cur.type = TOK_RPAREN;
		return;
	case '{':
		l->cur.type = TOK_LBRACE;
		return;
	case '}':
		l->cur.type = TOK_RBRACE;
		return;
	case '[':
		l->cur.type = TOK_LBRACKET;
		return;
	case ']':
		l->cur.type = TOK_RBRACKET;
		return;
	case ';':
		l->cur.type = TOK_SEMICOLON;
		return;
	case ',':
		l->cur.type = TOK_COMMA;
		return;
	case '.':
		l->cur.type = TOK_DOT;
		return;
	case '*':
		if (l->src[l->pos] == '=') {
			l->pos++;
			l->cur.type = TOK_MUL_ASSIGN;
		}
		else {
			l->cur.type = TOK_STAR;
		}
		return;
	case '/':
		if (l->src[l->pos] == '=') {
			l->pos++;
			l->cur.type = TOK_DIV_ASSIGN;
		}
		else {
			l->cur.type = TOK_SLASH;
		}
		return;
	case '+':
		if (l->src[l->pos] == '+') {
			l->pos++;
			l->cur.type = TOK_INC;
		}
		else if (l->src[l->pos] == '=') {
			l->pos++;
			l->cur.type = TOK_PLUS_ASSIGN;
		}
		else {
			l->cur.type = TOK_PLUS;
		}
		return;
	case '-':
		if (l->src[l->pos] == '-') {
			l->pos++;
			l->cur.type = TOK_DEC;
		}
		else if (l->src[l->pos] == '=') {
			l->pos++;
			l->cur.type = TOK_MINUS_ASSIGN;
		}
		else if (l->src[l->pos] == '>') {
			l->pos++;
			l->cur.type = TOK_ARROW;
		}
		else {
			l->cur.type = TOK_MINUS;
		}
		return;
	case '=':
		if (l->src[l->pos] == '=') {
			l->pos++;
			l->cur.type = TOK_EQ;
		}
		else {
			l->cur.type = TOK_ASSIGN;
		}
		return;
	case '!':
		if (l->src[l->pos] == '=') {
			l->pos++;
			l->cur.type = TOK_NEQ;
		}
		else {
			l->cur.type = TOK_NOT;
		}
		return;
	case '&':
		if (l->src[l->pos] == '&') {
			l->pos++;
			l->cur.type = TOK_AND;
		}
		else {
			l->cur.type = TOK_AMP;
		}
		return;
	case '|':
		if (l->src[l->pos] == '|') {
			l->pos++;
			l->cur.type = TOK_OR;
		}
		return;
	case '<':
		if (l->src[l->pos] == '=') {
			l->pos++;
			l->cur.type = TOK_LE;
		}
		else {
			l->cur.type = TOK_LT;
		}
		return;
	case '>':
		if (l->src[l->pos] == '=') {
			l->pos++;
			l->cur.type = TOK_GE;
		}
		else {
			l->cur.type = TOK_GT;
		}
		return;
	}
}

// ███████╗██╗   ██╗███╗   ██╗ ██████╗███████╗
// ██╔════╝██║   ██║████╗  ██║██╔════╝██╔════╝
// █████╗  ██║   ██║██╔██╗ ██║██║     ███████╗
// ██╔══╝  ██║   ██║██║╚██╗██║██║     ╚════██║
// ██║     ╚██████╔╝██║ ╚████║╚██████╗███████║
// ╚═╝      ╚═════╝ ╚═╝  ╚═══╝ ╚═════╝╚══════╝

void *minic_alloc(int size) {
	// Align to 8 bytes
	int aligned            = (*minic_active_mem_used + 7) & ~7;
	*minic_active_mem_used = aligned + size;
	return &minic_active_mem[aligned];
}

typedef struct {
	char        name[64];
	minic_val_t val;
} minic_var_t;

typedef struct {
	char         name[64];
	int          offset; // index into global arr_data[]
	int          count;
	minic_type_t elem_type;
} minic_arr_t;

typedef struct {
	char name[64];
	char params[MINIC_MAX_PARAMS][64];
	char param_structs[MINIC_MAX_PARAMS][64]; // struct type name per param, or ""
	// Per-param declared type
	minic_type_t param_types[MINIC_MAX_PARAMS];
	int          param_count;
	int          body_pos; // lexer position of '{' that starts the body
	minic_type_t ret_type;
	minic_ctx_t *ctx; // owning context, set at parse time
} minic_func_t;

#define MINIC_INC_DELTA(l) ((l)->cur.type == TOK_INC ? 1.0 : -1.0)

typedef struct {
	char         name[64];
	char         fields[MINIC_MAX_STRUCT_FIELDS][64];
	char         field_struct_names[MINIC_MAX_STRUCT_FIELDS][64]; // struct type name for PTR fields, or ""
	int          field_count;
	int          size; // sizeof the native C struct, 0 if unknown
	bool         has_native_layout;
	int          field_offsets[MINIC_MAX_STRUCT_FIELDS];
	minic_type_t field_native_types[MINIC_MAX_STRUCT_FIELDS];
	minic_type_t field_deref_types[MINIC_MAX_STRUCT_FIELDS]; // pointed-to type for PTR fields
} minic_struct_def_t;

// Maps a variable name to its struct type name
typedef struct {
	char var_name[64];
	char struct_name[64];
} minic_vartype_t;

typedef struct minic_env_s {
	minic_lexer_t       lex;
	const char         *filename;
	minic_var_t        *vars;
	int                 var_count;
	int                 var_cap;
	minic_arr_t        *arrs;
	int                 arr_count;
	int                 arr_cap;
	minic_val_t        *arr_data;      // global array element storage
	int                *arr_data_used; // pointer to shared counter
	minic_func_t       *funcs;
	int                 func_count;
	int                 func_cap;
	minic_struct_def_t *structs; // shared across calls
	int                 struct_count;
	int                 struct_cap;
	minic_vartype_t    *vartypes; // local: var->struct type mapping
	int                 vartype_count;
	int                 vartype_cap;
	bool                returning;
	bool                breaking;
	bool                continuing;
	bool                error;
	minic_val_t         return_val;
	minic_type_t        decl_type;
	struct minic_env_s *global_env; // top-level env that owns the script globals
} minic_env_t;

struct minic_ctx_s {
	minic_u8   *mem;
	int         mem_used;
	minic_env_t e;
	float       result;
	char       *src_copy;
};

static minic_env_t *minic_active_env = NULL;

static minic_val_t minic_parse_expr(minic_env_t *e);
static minic_val_t minic_parse_cond(minic_env_t *e);
static void        minic_skip_block(minic_env_t *e);
static void        minic_parse_stmt(minic_env_t *e);
static void        minic_parse_block(minic_env_t *e);

static const char *minic_tok_name(minic_tok_type_t t) {
	switch (t) {
	case TOK_INT:
		return "'int'";
	case TOK_FLOAT:
		return "'float'";
	case TOK_CHAR:
		return "'char'";
	case TOK_DOUBLE:
		return "'double'";
	case TOK_BOOL:
		return "'bool'";
	case TOK_RETURN:
		return "'return'";
	case TOK_IF:
		return "'if'";
	case TOK_ELSE:
		return "'else'";
	case TOK_WHILE:
		return "'while'";
	case TOK_FOR:
		return "'for'";
	case TOK_BREAK:
		return "'break'";
	case TOK_CONTINUE:
		return "'continue'";
	case TOK_IDENT:
		return "identifier";
	case TOK_NUMBER:
		return "number";
	case TOK_CHAR_LIT:
		return "char literal";
	case TOK_STR_LIT:
		return "string literal";
	case TOK_LPAREN:
		return "'('";
	case TOK_RPAREN:
		return "')'";
	case TOK_LBRACE:
		return "'{'";
	case TOK_RBRACE:
		return "'}'";
	case TOK_LBRACKET:
		return "'['";
	case TOK_RBRACKET:
		return "']'";
	case TOK_SEMICOLON:
		return "';'";
	case TOK_COMMA:
		return "','";
	case TOK_ASSIGN:
		return "'='";
	case TOK_EQ:
		return "'=='";
	case TOK_NEQ:
		return "'!='";
	case TOK_LT:
		return "'<'";
	case TOK_GT:
		return "'>'";
	case TOK_LE:
		return "'<='";
	case TOK_GE:
		return "'>='";
	case TOK_AND:
		return "'&&'";
	case TOK_OR:
		return "'||'";
	case TOK_AMP:
		return "'&'";
	case TOK_PLUS:
		return "'+'";
	case TOK_MINUS:
		return "'-'";
	case TOK_INC:
		return "'++'";
	case TOK_DEC:
		return "'--'";
	case TOK_PLUS_ASSIGN:
		return "'+='";
	case TOK_MINUS_ASSIGN:
		return "'-='";
	case TOK_MUL_ASSIGN:
		return "'*='";
	case TOK_DIV_ASSIGN:
		return "'/='";
	case TOK_STAR:
		return "'*'";
	case TOK_SLASH:
		return "'/'";
	case TOK_DOT:
		return "'.'";
	case TOK_ARROW:
		return "'->'";
	case TOK_STRUCT:
		return "'struct'";
	case TOK_VOID:
		return "'void'";
	case TOK_EOF:
		return "end of file";
	default:
		return "unknown token";
	}
}

static int minic_current_line(minic_env_t *e) {
	int line = 1;
	for (int i = 0; i < e->lex.pos; i++) {
		if (e->lex.src[i] == '\n') line++;
	}
	return line;
}

static void minic_error(minic_env_t *e, const char *msg) {
	if (!e->error) {
		fprintf(stderr, "%s:%d: error: %s (got %s)\n", e->filename, minic_current_line(e), msg, minic_tok_name(e->lex.cur.type));
		e->error     = true;
		e->returning = true;
	}
}

static void minic_expect(minic_env_t *e, minic_tok_type_t expected) {
	if (e->lex.cur.type != expected) {
		char msg[128];
		snprintf(msg, sizeof(msg), "expected %s", minic_tok_name(expected));
		minic_error(e, msg);
		return;
	}
	minic_lex_next(&e->lex);
}

static minic_val_t minic_var_get(minic_env_t *e, const char *name) {
	for (int i = e->var_count - 1; i >= 0; --i) {
		if (strcmp(e->vars[i].name, name) == 0) {
			return e->vars[i].val;
		}
	}
	if (e->global_env != NULL) {
		for (int i = e->global_env->var_count - 1; i >= 0; --i) {
			if (strcmp(e->global_env->vars[i].name, name) == 0) {
				return e->global_env->vars[i].val;
			}
		}
	}
	return minic_val_int(0);
}

static void minic_var_set(minic_env_t *e, const char *name, minic_val_t val) {
	for (int i = e->var_count - 1; i >= 0; --i) {
		if (strcmp(e->vars[i].name, name) == 0) {
			// Preserve declared type, coerce if needed
			if (e->vars[i].val.type != val.type) {
				val = minic_val_cast(val, e->vars[i].val.type);
			}
			// Preserve deref_type for pointer variables (it was set at declaration)
			if (e->vars[i].val.type == MINIC_T_PTR && e->vars[i].val.deref_type != MINIC_T_PTR) {
				val.deref_type = e->vars[i].val.deref_type;
			}
			e->vars[i].val = val;
			return;
		}
	}
	// Fall back to globals: write directly into the top-level env
	if (e->global_env != NULL) {
		minic_env_t *g = e->global_env;
		for (int i = g->var_count - 1; i >= 0; --i) {
			if (strcmp(g->vars[i].name, name) == 0) {
				if (g->vars[i].val.type != val.type) {
					val = minic_val_cast(val, g->vars[i].val.type);
				}
				if (g->vars[i].val.type == MINIC_T_PTR && g->vars[i].val.deref_type != MINIC_T_PTR) {
					val.deref_type = g->vars[i].val.deref_type;
				}
				g->vars[i].val = val;
				return;
			}
		}
	}
	if (e->var_count < e->var_cap) {
		strncpy(e->vars[e->var_count].name, name, 63);
		e->vars[e->var_count].val = val;
		e->var_count++;
	}
}

static void minic_var_decl(minic_env_t *e, const char *name, minic_type_t type, minic_val_t init) {
	// Coerce init to declared type
	minic_val_t v = minic_val_cast(init, type);
	// If this is a typed pointer, preserve the deref_type from init
	if (type == MINIC_T_PTR) {
		v.deref_type = init.deref_type;
	}
	if (e->var_count < e->var_cap) {
		strncpy(e->vars[e->var_count].name, name, 63);
		e->vars[e->var_count].val = v;
		e->var_count++;
	}
}

static minic_val_t minic_var_addr(minic_env_t *e, const char *name) {
	for (int i = e->var_count - 1; i >= 0; --i) {
		if (strcmp(e->vars[i].name, name) == 0) {
			// Address of a minic_val_t — deref reads a full minic_val_t
			minic_val_t addr = minic_val_ptr(&e->vars[i].val);
			addr.deref_type  = MINIC_T_PTR; // sentinel: deref reads minic_val_t
			return addr;
		}
	}
	// Fall back to globals: return address into the top-level env's storage
	if (e->global_env != NULL) {
		minic_env_t *g = e->global_env;
		for (int i = g->var_count - 1; i >= 0; --i) {
			if (strcmp(g->vars[i].name, name) == 0) {
				minic_val_t addr = minic_val_ptr(&g->vars[i].val);
				addr.deref_type  = MINIC_T_PTR;
				return addr;
			}
		}
	}
	// Create it as a local
	if (e->var_count < e->var_cap) {
		strncpy(e->vars[e->var_count].name, name, 63);
		e->vars[e->var_count].val = minic_val_int(0);
		minic_val_t addr          = minic_val_ptr(&e->vars[e->var_count].val);
		addr.deref_type           = MINIC_T_PTR;
		e->var_count++;
		return addr;
	}
	return minic_val_ptr(NULL);
}

static minic_arr_t *minic_arr_get(minic_env_t *e, const char *name) {
	for (int i = 0; i < e->arr_count; ++i) {
		if (strcmp(e->arrs[i].name, name) == 0) {
			return &e->arrs[i];
		}
	}
	if (e->global_env != NULL) {
		minic_env_t *g = e->global_env;
		for (int i = 0; i < g->arr_count; ++i) {
			if (strcmp(g->arrs[i].name, name) == 0) {
				return &g->arrs[i];
			}
		}
	}
	return NULL;
}

static void minic_arr_decl(minic_env_t *e, const char *name, int count, minic_type_t elem_type) {
	if (e->arr_count >= e->arr_cap) {
		return;
	}
	minic_arr_t *a = &e->arrs[e->arr_count++];
	strncpy(a->name, name, 63);
	a->offset    = *e->arr_data_used;
	a->count     = count;
	a->elem_type = elem_type;
	*e->arr_data_used += count;
	// Zero-initialise
	for (int i = 0; i < count; i++) {
		e->arr_data[a->offset + i] = minic_val_coerce(0.0, elem_type);
	}
}

static minic_val_t minic_arr_elem_get(minic_env_t *e, const char *name, int idx) {
	minic_arr_t *a = minic_arr_get(e, name);
	if (a != NULL && idx >= 0 && idx < a->count) {
		return e->arr_data[a->offset + idx];
	}
	// Fall back to native pointer subscript
	minic_val_t pv = minic_var_get(e, name);
	if (pv.type == MINIC_T_PTR && pv.p != NULL) {
		switch (pv.deref_type) {
		case MINIC_T_FLOAT:
			return minic_val_float(((float *)pv.p)[idx]);
		case MINIC_T_INT:
			return minic_val_int(((int32_t *)pv.p)[idx]);
		default:
			return minic_val_ptr(((void **)pv.p)[idx]);
		}
	}
	return minic_val_int(0);
}

static void minic_arr_elem_set(minic_env_t *e, const char *name, int idx, minic_val_t val) {
	minic_arr_t *a = minic_arr_get(e, name);
	if (a != NULL && idx >= 0 && idx < a->count) {
		e->arr_data[a->offset + idx] = minic_val_cast(val, a->elem_type);
		return;
	}
	// Fall back to native pointer subscript
	minic_val_t pv = minic_var_get(e, name);
	if (pv.type == MINIC_T_PTR && pv.p != NULL) {
		switch (pv.deref_type) {
		case MINIC_T_FLOAT:
			((float *)pv.p)[idx] = (float)minic_val_to_d(val);
			break;
		case MINIC_T_INT:
			((int32_t *)pv.p)[idx] = (int32_t)minic_val_to_d(val);
			break;
		default:
			((void **)pv.p)[idx] = minic_val_to_ptr(val);
			break;
		}
	}
}

static minic_func_t *minic_func_get(minic_env_t *e, const char *name) {
	for (int i = 0; i < e->func_count; ++i) {
		if (strcmp(e->funcs[i].name, name) == 0) {
			return &e->funcs[i];
		}
	}
	return NULL;
}

static minic_struct_def_t *minic_struct_get(minic_env_t *e, const char *name);

static void minic_vartype_set(minic_env_t *e, const char *var_name, const char *struct_name) {
	if (e->vartype_count < e->vartype_cap) {
		strncpy(e->vartypes[e->vartype_count].var_name, var_name, 63);
		strncpy(e->vartypes[e->vartype_count].struct_name, struct_name, 63);
		e->vartype_count++;
	}
}

static minic_struct_def_t *minic_var_struct(minic_env_t *e, const char *var_name) {
	for (int i = e->vartype_count - 1; i >= 0; --i) {
		if (strcmp(e->vartypes[i].var_name, var_name) == 0) {
			return minic_struct_get(e, e->vartypes[i].struct_name);
		}
	}
	if (e->global_env != NULL) {
		for (int i = e->global_env->vartype_count - 1; i >= 0; --i) {
			if (strcmp(e->global_env->vartypes[i].var_name, var_name) == 0) {
				return minic_struct_get(e, e->global_env->vartypes[i].struct_name);
			}
		}
	}
	return NULL;
}

static minic_struct_def_t *minic_struct_get(minic_env_t *e, const char *name) {
	for (int i = 0; i < e->struct_count; ++i) {
		if (strcmp(e->structs[i].name, name) == 0) {
			return &e->structs[i];
		}
	}
	return NULL;
}

static int minic_struct_field_idx(minic_struct_def_t *def, const char *field) {
	for (int i = 0; i < def->field_count; ++i) {
		if (strcmp(def->fields[i], field) == 0) {
			return i;
		}
	}
	return -1;
}

static minic_val_t minic_struct_field_get_base(minic_env_t *e, void *base, minic_struct_def_t *def, const char *field) {
	int idx = minic_struct_field_idx(def, field);
	if (idx < 0) {
		fprintf(stderr, "%s:%d: error: struct '%s' has no field '%s'\n", e->filename, minic_current_line(e), def->name, field);
		e->error = e->returning = true;
		return minic_val_int(0);
	}
	bool in_arena = (base != NULL && (minic_u8 *)base >= minic_active_mem && (minic_u8 *)base < minic_active_mem + MINIC_MEM_SIZE);
	if (def->has_native_layout && !in_arena) {
		char *p = (char *)base + def->field_offsets[idx];
		switch (def->field_native_types[idx]) {
		case MINIC_T_PTR:
			return minic_val_typed_ptr(*(void **)p, def->field_deref_types[idx]);
		case MINIC_T_FLOAT:
			return minic_val_float(*(float *)p);
		default:
			return minic_val_int(*(int32_t *)p);
		}
	}
	minic_val_t v;
	memcpy(&v, (minic_val_t *)base + idx, sizeof(minic_val_t));
	return v;
}

static void minic_struct_field_set_base(minic_env_t *e, void *base, minic_struct_def_t *def, const char *field, minic_val_t val) {
	int idx = minic_struct_field_idx(def, field);
	if (idx < 0) {
		fprintf(stderr, "%s:%d: error: struct '%s' has no field '%s'\n", e->filename, minic_current_line(e), def->name, field);
		e->error = e->returning = true;
		return;
	}
	bool in_arena = (base != NULL && (minic_u8 *)base >= minic_active_mem && (minic_u8 *)base < minic_active_mem + MINIC_MEM_SIZE);
	if (def->has_native_layout && !in_arena) {
		char *p = (char *)base + def->field_offsets[idx];
		switch (def->field_native_types[idx]) {
		case MINIC_T_PTR:
			*(void **)p = minic_val_to_ptr(val);
			break;
		case MINIC_T_FLOAT:
			*(float *)p = (float)minic_val_to_d(val);
			break;
		default:
			*(int32_t *)p = (int32_t)minic_val_to_d(val);
			break;
		}
		return;
	}
	memcpy((minic_val_t *)base + idx, &val, sizeof(minic_val_t));
}

static minic_val_t minic_struct_field_get(minic_env_t *e, const char *var_name, minic_struct_def_t *def, const char *field) {
	minic_val_t base_val = minic_var_get(e, var_name);
	void       *base     = minic_val_to_ptr(base_val);
	return minic_struct_field_get_base(e, base, def, field);
}

static void minic_struct_field_set(minic_env_t *e, const char *var_name, minic_struct_def_t *def, const char *field, minic_val_t val) {
	minic_val_t base_val = minic_var_get(e, var_name);
	void       *base     = minic_val_to_ptr(base_val);
	minic_struct_field_set_base(e, base, def, field, val);
}

static minic_val_t minic_call(minic_env_t *e, minic_func_t *fn, minic_val_t *args, int argc) {
	minic_env_t child   = {0};
	child.lex.src       = e->lex.src;
	child.lex.pos       = fn->body_pos;
	child.filename      = e->filename;
	child.var_cap       = 64;
	child.vars          = minic_alloc(child.var_cap * sizeof(minic_var_t));
	child.global_env    = e->global_env != NULL ? e->global_env : e;
	child.arr_cap       = 32;
	child.arrs          = minic_alloc(child.arr_cap * sizeof(minic_arr_t));
	child.arr_data      = e->arr_data;
	child.arr_data_used = e->arr_data_used;
	child.func_count    = e->func_count;
	child.func_cap      = e->func_cap;
	child.funcs         = e->funcs;
	child.struct_count  = e->struct_count;
	child.struct_cap    = e->struct_cap;
	child.structs       = e->structs;
	child.vartype_cap   = 32;
	child.vartypes      = minic_alloc(child.vartype_cap * sizeof(minic_vartype_t));
	// Bind parameters
	for (int i = 0; i < argc && i < fn->param_count; ++i) {
		minic_val_t v = minic_val_cast(args[i], fn->param_types[i]);
		minic_var_decl(&child, fn->params[i], fn->param_types[i], v);
		if (fn->param_structs[i][0] != '\0') {
			minic_vartype_set(&child, fn->params[i], fn->param_structs[i]);
		}
	}
	minic_lex_next(&child.lex);
	minic_parse_block(&child);
	return child.return_val;
}

minic_val_t minic_call_fn(void *fn_ptr, minic_val_t *args, int argc) {
	if (fn_ptr == NULL) {
		return minic_val_int(0);
	}
	minic_func_t *fn  = (minic_func_t *)fn_ptr;
	minic_ctx_t  *ctx = fn->ctx;
	if (ctx == NULL) {
		return minic_val_int(0);
	}
	minic_u8    *prev_mem      = minic_active_mem;
	int         *prev_mem_used = minic_active_mem_used;
	minic_env_t *prev_env      = minic_active_env;
	minic_active_mem           = ctx->mem;
	minic_active_mem_used      = &ctx->mem_used;
	minic_active_env           = &ctx->e;
	int         saved_mem_used = ctx->mem_used;
	minic_val_t r              = minic_call(&ctx->e, fn, args, argc);
	ctx->mem_used              = saved_mem_used;
	minic_active_env           = prev_env;
	minic_active_mem           = prev_mem;
	minic_active_mem_used      = prev_mem_used;
	return r;
}

static minic_val_t minic_arith(minic_val_t a, minic_val_t b, char op) {
	// Determine result type (widening: int < float < ptr)
	minic_type_t rt;
	if (a.type == MINIC_T_PTR || b.type == MINIC_T_PTR)
		rt = MINIC_T_PTR;
	else if (a.type == MINIC_T_FLOAT || b.type == MINIC_T_FLOAT)
		rt = MINIC_T_FLOAT;
	else
		rt = MINIC_T_INT;

	double da = minic_val_to_d(a);
	double db = minic_val_to_d(b);
	double r;
	switch (op) {
	case '+':
		r = da + db;
		break;
	case '-':
		r = da - db;
		break;
	case '*':
		r = da * db;
		break;
	case '/':
		r = (db != 0.0) ? da / db : 0.0;
		break;
	default:
		r = 0.0;
	}
	return minic_val_coerce(r, rt);
}

// primary: '&' IDENT | '*' primary | '-' primary | '++' IDENT | '--' IDENT | NUMBER | CHAR_LIT | IDENT ['[' expr ']'] ['(' args ')'] | '(' expr ')'
static minic_val_t minic_parse_primary(minic_env_t *e) {
	if (e->lex.cur.type == TOK_AMP) {
		minic_lex_next(&e->lex); // Consume '&'
		const char  *aname = e->lex.cur.text;
		minic_arr_t *arr   = minic_arr_get(e, aname);
		minic_val_t  addr;
		if (arr != NULL) {
			addr = minic_val_ptr(&e->arr_data[arr->offset]);
		}
		else if (minic_var_struct(e, aname) != NULL) {
			// Struct var holds real base pointer
			addr = minic_var_get(e, aname);
		}
		else {
			addr = minic_var_addr(e, aname);
		}
		minic_lex_next(&e->lex); // Consume ident
		return addr;
	}
	if (e->lex.cur.type == TOK_STAR) {
		// Dereference: *expr → read through pointer using deref_type
		minic_lex_next(&e->lex); // consume '*'
		minic_val_t pv  = minic_parse_primary(e);
		void       *ptr = minic_val_to_ptr(pv);
		if (ptr == NULL) {
			return minic_val_int(0);
		}
		// Deref_type == MINIC_T_PTR means "pointer to minic_val_t" (interpreter-internal)
		// any other deref_type means a native C scalar at that address
		switch (pv.deref_type) {
		case MINIC_T_INT: {
			int v;
			memcpy(&v, ptr, sizeof(int));
			return minic_val_int(v);
		}
		case MINIC_T_FLOAT: {
			float v;
			memcpy(&v, ptr, sizeof(float));
			return minic_val_float(v);
		}
		default: {
			// MINIC_T_PTR or unknown: read a full minic_val_t (interpreter-internal ptr)
			minic_val_t v;
			memcpy(&v, ptr, sizeof(minic_val_t));
			return v;
		}
		}
	}
	if (e->lex.cur.type == TOK_MINUS) {
		minic_lex_next(&e->lex);
		minic_val_t v = minic_parse_primary(e);
		return minic_val_coerce(-minic_val_to_d(v), v.type);
	}
	if (e->lex.cur.type == TOK_NOT) {
		minic_lex_next(&e->lex);
		minic_val_t v = minic_parse_primary(e);
		return minic_val_int(!minic_val_is_true(v));
	}
	if (e->lex.cur.type == TOK_INC || e->lex.cur.type == TOK_DEC) {
		double delta = MINIC_INC_DELTA(&e->lex);
		minic_lex_next(&e->lex);
		char name[64];
		strncpy(name, e->lex.cur.text, 63);
		minic_lex_next(&e->lex);
		minic_val_t ov = minic_var_get(e, name);
		minic_val_t nv = minic_val_coerce(minic_val_to_d(ov) + delta, ov.type);
		minic_var_set(e, name, nv);
		return nv;
	}
	if (e->lex.cur.type == TOK_NUMBER || e->lex.cur.type == TOK_CHAR_LIT || e->lex.cur.type == TOK_STR_LIT) {
		minic_val_t v = e->lex.cur.val;
		minic_lex_next(&e->lex);
		return v;
	}
	if (e->lex.cur.type == TOK_IDENT) {
		char name[64];
		strncpy(name, e->lex.cur.text, 63);
		minic_lex_next(&e->lex);

		if (strcmp(name, "sizeof") == 0) {
			minic_expect(e, TOK_LPAREN); // checks and consumes '('
			char type_name[64];
			strncpy(type_name, e->lex.cur.text, 63);
			minic_lex_next(&e->lex); // Consume type name
			minic_expect(e, TOK_RPAREN); // checks and consumes ')'
			minic_struct_def_t *def = minic_struct_get(e, type_name);
			return minic_val_int(def != NULL ? def->size : 0);
		}

		if (e->lex.cur.type == TOK_LBRACKET) {
			minic_lex_next(&e->lex); // Consume '['
			int idx = (int)minic_val_to_d(minic_parse_expr(e));
			minic_lex_next(&e->lex); // Consume ']'
			return minic_arr_elem_get(e, name, idx);
		}

		if (e->lex.cur.type == TOK_LPAREN) {
			minic_lex_next(&e->lex); // Consume '('
			minic_val_t args[MINIC_MAX_PARAMS];
			int         argc = 0;
			while (e->lex.cur.type != TOK_RPAREN && e->lex.cur.type != TOK_EOF) {
				args[argc++] = minic_parse_expr(e);
				if (e->lex.cur.type == TOK_COMMA) {
					minic_lex_next(&e->lex);
				}
			}
			minic_lex_next(&e->lex); // Consume ')'
			minic_func_t *fn = minic_func_get(e, name);
			if (fn != NULL) {
				return minic_call(e, fn, args, argc);
			}
			minic_ext_func_t *ext = minic_ext_func_get(name);
			if (ext != NULL) {
				return minic_dispatch(ext, args, argc);
			}
			fprintf(stderr, "%s:%d: error: unknown function '%s'\n", e->filename, minic_current_line(e), name);
			e->error     = true;
			e->returning = true;
			return minic_val_int(0);
		}

		if (e->lex.cur.type == TOK_DOT) {
			minic_lex_next(&e->lex); // Consume '.'
			char field[64];
			strncpy(field, e->lex.cur.text, 63);
			minic_lex_next(&e->lex);
			minic_struct_def_t *def = minic_var_struct(e, name);
			if (def == NULL) {
				fprintf(stderr, "%s:%d: error: '%s' is not a struct\n", e->filename, minic_current_line(e), name);
				e->error = e->returning = true;
				return minic_val_int(0);
			}
			return minic_struct_field_get(e, name, def, field);
		}

		if (e->lex.cur.type == TOK_ARROW) {
			minic_lex_next(&e->lex); // Consume '->'
			char field[64];
			strncpy(field, e->lex.cur.text, 63);
			minic_lex_next(&e->lex);
			minic_struct_def_t *def = minic_var_struct(e, name);
			if (def == NULL) {
				fprintf(stderr, "%s:%d: error: '%s' is not a struct pointer\n", e->filename, minic_current_line(e), name);
				e->error = e->returning = true;
				return minic_val_int(0);
			}
			minic_val_t base_val  = minic_var_get(e, name);
			void       *base      = minic_val_to_ptr(base_val);
			minic_val_t field_val = minic_struct_field_get_base(e, base, def, field);
			// Handle chained -> or . access (e.g. node->inputs->buffer)
			while ((e->lex.cur.type == TOK_ARROW || e->lex.cur.type == TOK_DOT) && !e->error) {
				int fidx = minic_struct_field_idx(def, field);
				if (fidx < 0 || def->field_struct_names[fidx][0] == '\0') break;
				minic_struct_def_t *next_def = minic_struct_get(e, def->field_struct_names[fidx]);
				if (next_def == NULL) break;
				minic_lex_next(&e->lex); // Consume '->' or '.'
				strncpy(field, e->lex.cur.text, 63);
				minic_lex_next(&e->lex);
				base      = minic_val_to_ptr(field_val);
				def       = next_def;
				field_val = minic_struct_field_get_base(e, base, def, field);
			}
			if (e->lex.cur.type == TOK_LBRACKET) {
				minic_lex_next(&e->lex); // Consume '['
				int idx = (int)minic_val_to_d(minic_parse_expr(e));
				minic_expect(e, TOK_RBRACKET);
				if (field_val.type == MINIC_T_PTR && field_val.p != NULL) {
					switch (field_val.deref_type) {
					case MINIC_T_FLOAT:
						return minic_val_float(((float *)field_val.p)[idx]);
					case MINIC_T_INT:
						return minic_val_int(((int32_t *)field_val.p)[idx]);
					default:
						return minic_val_ptr(((void **)field_val.p)[idx]);
					}
				}
				return minic_val_int(0);
			}
			return field_val;
		}

		// If not a variable, check if it's a minic function (pass-as-pointer)
		minic_func_t *fn_val = minic_func_get(e, name);
		if (fn_val != NULL) {
			return minic_val_ptr(fn_val);
		}
		// Check for known enum constant
		{
			int ec = minic_enum_const_get(name);
			if (ec >= 0)
				return minic_val_int(ec);
		}
		return minic_var_get(e, name);
	}
	if (e->lex.cur.type == TOK_LPAREN) {
		minic_lex_next(&e->lex);
		minic_val_t v = minic_parse_expr(e);
		minic_lex_next(&e->lex); // Consume ')'
		return v;
	}
	return minic_val_int(0);
}

// term: primary (('*' | '/') primary)*
static minic_val_t minic_parse_term(minic_env_t *e) {
	minic_val_t v = minic_parse_primary(e);
	while (e->lex.cur.type == TOK_STAR || e->lex.cur.type == TOK_SLASH) {
		char op = (e->lex.cur.type == TOK_STAR) ? '*' : '/';
		minic_lex_next(&e->lex);
		minic_val_t r = minic_parse_primary(e);
		v             = minic_arith(v, r, op);
	}
	return v;
}

// expr: term (('+' | '-') term)*
static minic_val_t minic_parse_expr(minic_env_t *e) {
	minic_val_t v = minic_parse_term(e);
	while (e->lex.cur.type == TOK_PLUS || e->lex.cur.type == TOK_MINUS) {
		char op = (e->lex.cur.type == TOK_PLUS) ? '+' : '-';
		minic_lex_next(&e->lex);
		minic_val_t r = minic_parse_term(e);
		v             = minic_arith(v, r, op);
	}
	return v;
}

// cmp: expr (('=='|'!='|'<'|'>'|'<='|'>=') expr)?
static minic_val_t minic_parse_cmp(minic_env_t *e) {
	minic_val_t      v  = minic_parse_expr(e);
	minic_tok_type_t op = e->lex.cur.type;
	if (op == TOK_EQ || op == TOK_NEQ || op == TOK_LT || op == TOK_GT || op == TOK_LE || op == TOK_GE) {
		minic_lex_next(&e->lex);
		minic_val_t r  = minic_parse_expr(e);
		double      dv = minic_val_to_d(v);
		double      dr = minic_val_to_d(r);
		int         res;
		if (op == TOK_EQ)
			res = dv == dr;
		else if (op == TOK_NEQ)
			res = dv != dr;
		else if (op == TOK_LT)
			res = dv < dr;
		else if (op == TOK_GT)
			res = dv > dr;
		else if (op == TOK_LE)
			res = dv <= dr;
		else
			res = dv >= dr;
		return minic_val_int(res);
	}
	return v;
}

// cond: cmp (('&&' | '||') cmp)*
static minic_val_t minic_parse_cond(minic_env_t *e) {
	minic_val_t v = minic_parse_cmp(e);
	while (e->lex.cur.type == TOK_AND || e->lex.cur.type == TOK_OR) {
		minic_tok_type_t op = e->lex.cur.type;
		minic_lex_next(&e->lex);
		minic_val_t r  = minic_parse_cmp(e);
		int         vi = minic_val_is_true(v);
		int         ri = minic_val_is_true(r);
		v              = minic_val_int(op == TOK_AND ? (vi && ri) : (vi || ri));
	}
	return v;
}

static void minic_skip_block(minic_env_t *e) {
	if (e->lex.cur.type != TOK_LBRACE) {
		// Single-statement body: skip until ';'
		while (e->lex.cur.type != TOK_SEMICOLON && e->lex.cur.type != TOK_EOF)
			minic_lex_next(&e->lex);
		minic_lex_next(&e->lex); // Consume ';'
		return;
	}
	minic_lex_next(&e->lex); // Consume '{'
	int depth = 1;
	while (depth > 0 && e->lex.cur.type != TOK_EOF) {
		if (e->lex.cur.type == TOK_LBRACE)
			depth++;
		if (e->lex.cur.type == TOK_RBRACE)
			depth--;
		minic_lex_next(&e->lex);
	}
}

static minic_type_t minic_tok_to_type(minic_tok_type_t t) {
	switch (t) {
	case TOK_INT:
	case TOK_CHAR:
	case TOK_BOOL:
		return MINIC_T_INT;
	case TOK_FLOAT:
	case TOK_DOUBLE:
		return MINIC_T_FLOAT;
	default:
		return MINIC_T_PTR; // void * -> PTR
	}
}

static void minic_parse_stmt(minic_env_t *e) {
	// Skip bare typedef declarations inside function bodies
	if (e->lex.cur.type == TOK_TYPEDEF) {
		while (e->lex.cur.type != TOK_SEMICOLON && e->lex.cur.type != TOK_EOF) {
			minic_lex_next(&e->lex);
		}
		if (e->lex.cur.type == TOK_SEMICOLON) {
			minic_lex_next(&e->lex);
		}
		return;
	}

	if (e->lex.cur.type == TOK_STRUCT) {
		minic_lex_next(&e->lex); // Consume 'struct'
		char sname[64];
		strncpy(sname, e->lex.cur.text, 63);
		minic_lex_next(&e->lex); // Consume struct type name

		bool is_ptr = false;
		if (e->lex.cur.type == TOK_STAR) {
			is_ptr = true;
			minic_lex_next(&e->lex);
		}
		char vname[64];
		strncpy(vname, e->lex.cur.text, 63);
		minic_lex_next(&e->lex);

		minic_struct_def_t *def = minic_struct_get(e, sname);
		if (def == NULL) {
			fprintf(stderr, "%s:%d: error: unknown struct '%s'\n", e->filename, minic_current_line(e), sname);
			e->error = e->returning = true;
			return;
		}
		minic_vartype_set(e, vname, sname);
		if (is_ptr) {
			minic_expect(e, TOK_ASSIGN);
			minic_val_t v = minic_parse_expr(e);
			minic_var_decl(e, vname, MINIC_T_PTR, v);
			minic_expect(e, TOK_SEMICOLON);
		}
		else {
			void *base = minic_alloc(def->field_count * (int)sizeof(minic_val_t));
			memset(base, 0, def->field_count * sizeof(minic_val_t));
			if (e->lex.cur.type == TOK_ASSIGN) {
				minic_lex_next(&e->lex);
				minic_val_t v = minic_parse_expr(e);
				if (v.type == MINIC_T_PTR && v.p)
					memcpy(base, v.p, def->field_count * sizeof(minic_val_t));
			}
			minic_var_decl(e, vname, MINIC_T_PTR, minic_val_ptr(base));
			minic_expect(e, TOK_SEMICOLON);
		}
		return;
	}

	// Typedef'd int name (e.g. from typedef enum): alias_t var = expr;
	if (e->lex.cur.type == TOK_IDENT && minic_is_int_typedef(e->lex.cur.text)) {
		minic_lex_next(&e->lex); // Consume alias name
		char vname[64];
		strncpy(vname, e->lex.cur.text, 63);
		minic_lex_next(&e->lex); // Consume var name
		minic_val_t v = minic_val_int(0);
		if (e->lex.cur.type == TOK_ASSIGN) {
			minic_lex_next(&e->lex);
			v = minic_parse_cond(e);
		}
		minic_var_decl(e, vname, MINIC_T_INT, minic_val_coerce(minic_val_to_d(v), MINIC_T_INT));
		minic_expect(e, TOK_SEMICOLON);
		return;
	}

	// Typedef'd struct name used as variable type: alias_t var; or alias_t *var = ...;
	if (e->lex.cur.type == TOK_IDENT) {
		minic_struct_def_t *def = minic_struct_get(e, e->lex.cur.text);
		if (def != NULL) {
			char sname[64];
			strncpy(sname, e->lex.cur.text, 63);
			minic_lex_next(&e->lex); // Consume alias name
			bool is_ptr = (e->lex.cur.type == TOK_STAR);
			if (is_ptr) {
				minic_lex_next(&e->lex);
			}
			char vname[64];
			strncpy(vname, e->lex.cur.text, 63);
			minic_lex_next(&e->lex);
			minic_vartype_set(e, vname, sname);
			if (is_ptr) {
				minic_val_t v = minic_val_ptr(NULL);
				if (e->lex.cur.type == TOK_ASSIGN) {
					minic_lex_next(&e->lex);
					v = minic_parse_expr(e);
				}
				minic_var_decl(e, vname, MINIC_T_PTR, v);
			}
			else {
				void *base = minic_alloc(def->field_count * (int)sizeof(minic_val_t));
				memset(base, 0, def->field_count * sizeof(minic_val_t));
				if (e->lex.cur.type == TOK_ASSIGN) {
					minic_lex_next(&e->lex);
					minic_val_t v = minic_parse_expr(e);
					if (v.type == MINIC_T_PTR && v.p)
						memcpy(base, v.p, def->field_count * sizeof(minic_val_t));
				}
				minic_var_decl(e, vname, MINIC_T_PTR, minic_val_ptr(base));
			}
			minic_expect(e, TOK_SEMICOLON);
			return;
		}
	}

	if (e->lex.cur.type == TOK_INT || e->lex.cur.type == TOK_FLOAT || e->lex.cur.type == TOK_CHAR || e->lex.cur.type == TOK_DOUBLE ||
	    e->lex.cur.type == TOK_BOOL || e->lex.cur.type == TOK_VOID) {
		minic_type_t base_type = minic_tok_to_type(e->lex.cur.type); // Type before '*'
		minic_type_t dtype     = base_type;
		minic_lex_next(&e->lex); // Consume type keyword

		bool is_ptr = false;
		if (e->lex.cur.type == TOK_STAR) {
			is_ptr = true;
			dtype  = MINIC_T_PTR;
			minic_lex_next(&e->lex);
		}
		char name[64];
		strncpy(name, e->lex.cur.text, 63);
		minic_lex_next(&e->lex);

		if (e->lex.cur.type == TOK_LBRACKET) {
			minic_lex_next(&e->lex); // Consume '['
			int count = (int)minic_val_to_d(minic_parse_expr(e));
			minic_expect(e, TOK_RBRACKET);
			minic_arr_decl(e, name, count, is_ptr ? MINIC_T_PTR : dtype);
			minic_expect(e, TOK_SEMICOLON);
			return;
		}

		minic_expect(e, TOK_ASSIGN);
		minic_val_t v = minic_parse_cond(e);
		if (is_ptr) {
			v.type = MINIC_T_PTR;
			if (v.p == NULL) {
				// NULL literal passed as integer 0 — convert
				uintptr_t ua = (uintptr_t)(uint64_t)minic_val_to_d(v);
				v.p          = (ua == 0) ? NULL : (void *)ua;
			}
			// Only stamp the declared element type for native C pointers.
			// Pointers into the active arena (e.g. from &var) use MINIC_T_PTR sentinel
			// to signal that dereferencing reads a full minic_val_t.
			bool in_minic = (v.p != NULL && (minic_u8 *)v.p >= minic_active_mem && (minic_u8 *)v.p < minic_active_mem + MINIC_MEM_SIZE);
			if (!in_minic) {
				v.deref_type = base_type;
			}
		}
		minic_var_decl(e, name, dtype, v);
		minic_expect(e, TOK_SEMICOLON);
		return;
	}

	if (e->lex.cur.type == TOK_RETURN) {
		minic_lex_next(&e->lex);
		minic_val_t v = minic_parse_cond(e);
		e->return_val = v;
		e->returning  = true;
		minic_expect(e, TOK_SEMICOLON);
		return;
	}

	if (e->lex.cur.type == TOK_IDENT) {
		char name[64];
		strncpy(name, e->lex.cur.text, 63);
		minic_lex_next(&e->lex);

		// Unknown opaque type followed by '*' + ident: local pointer declaration.
		// e.g. ui_handle_t *h; or MyType *p = create_p();
		if (e->lex.cur.type == TOK_STAR) {
			minic_lex_next(&e->lex); // Consume '*'
			char vname[64];
			strncpy(vname, e->lex.cur.text, 63);
			minic_lex_next(&e->lex); // Consume var name
			minic_val_t v = minic_val_ptr(NULL);
			if (e->lex.cur.type == TOK_ASSIGN) {
				minic_lex_next(&e->lex);
				v = minic_parse_expr(e);
			}
			minic_var_decl(e, vname, MINIC_T_PTR, v);
			minic_expect(e, TOK_SEMICOLON);
			return;
		}

		if (e->lex.cur.type == TOK_DOT || e->lex.cur.type == TOK_ARROW) {
			bool is_arrow = (e->lex.cur.type == TOK_ARROW);
			minic_lex_next(&e->lex);
			char field[64];
			strncpy(field, e->lex.cur.text, 63);
			minic_lex_next(&e->lex);
			minic_struct_def_t *def = minic_var_struct(e, name);
			if (def == NULL) {
				fprintf(stderr, "%s:%d: error: '%s' is not a struct%s\n", e->filename, minic_current_line(e), name, is_arrow ? " pointer" : "");
				e->error = e->returning = true;
				return;
			}
			void *base;
			if (is_arrow) {
				minic_val_t base_val = minic_var_get(e, name);
				base                 = minic_val_to_ptr(base_val);
			}
			else {
				minic_val_t base_val = minic_var_get(e, name);
				base                 = minic_val_to_ptr(base_val);
			}
			if (e->lex.cur.type == TOK_LBRACKET) {
				minic_lex_next(&e->lex);
				int idx = (int)minic_val_to_d(minic_parse_expr(e));
				minic_expect(e, TOK_RBRACKET);
				minic_expect(e, TOK_ASSIGN);
				minic_val_t v         = minic_parse_expr(e);
				minic_val_t field_val = minic_struct_field_get_base(e, base, def, field);
				if (field_val.type == MINIC_T_PTR && field_val.p != NULL) {
					switch (field_val.deref_type) {
					case MINIC_T_FLOAT:
						((float *)field_val.p)[idx] = (float)minic_val_to_d(v);
						break;
					case MINIC_T_INT:
						((int32_t *)field_val.p)[idx] = (int32_t)minic_val_to_d(v);
						break;
					default:
						((void **)field_val.p)[idx] = minic_val_to_ptr(v);
						break;
					}
				}
			}
			else {
				minic_expect(e, TOK_ASSIGN);
				minic_val_t v = minic_parse_expr(e);
				minic_struct_field_set_base(e, base, def, field, v);
			}
			minic_expect(e, TOK_SEMICOLON);
			return;
		}

		if (e->lex.cur.type == TOK_LPAREN) {
			minic_lex_next(&e->lex);
			minic_val_t args[MINIC_MAX_PARAMS];
			int         argc = 0;
			while (e->lex.cur.type != TOK_RPAREN && e->lex.cur.type != TOK_EOF) {
				args[argc++] = minic_parse_expr(e);
				if (e->lex.cur.type == TOK_COMMA) {
					minic_lex_next(&e->lex);
				}
			}
			minic_expect(e, TOK_RPAREN);
			minic_func_t *fn = minic_func_get(e, name);
			if (fn != NULL) {
				minic_call(e, fn, args, argc);
			}
			else {
				minic_ext_func_t *ext = minic_ext_func_get(name);
				if (ext != NULL) {
					minic_dispatch(ext, args, argc);
				}
				else {
					fprintf(stderr, "%s:%d: error: unknown function '%s'\n", e->filename, minic_current_line(e), name);
					e->error     = true;
					e->returning = true;
				}
			}
			minic_expect(e, TOK_SEMICOLON);
			return;
		}

		if (e->lex.cur.type == TOK_INC || e->lex.cur.type == TOK_DEC) {
			double delta = MINIC_INC_DELTA(&e->lex);
			minic_lex_next(&e->lex);
			minic_val_t ov = minic_var_get(e, name);
			minic_var_set(e, name, minic_val_coerce(minic_val_to_d(ov) + delta, ov.type));
			minic_expect(e, TOK_SEMICOLON);
			return;
		}

		if (e->lex.cur.type == TOK_PLUS_ASSIGN || e->lex.cur.type == TOK_MINUS_ASSIGN || e->lex.cur.type == TOK_MUL_ASSIGN ||
		    e->lex.cur.type == TOK_DIV_ASSIGN) {
			minic_tok_type_t op = e->lex.cur.type;
			minic_lex_next(&e->lex);
			minic_val_t dv = minic_parse_expr(e);
			minic_val_t ov = minic_var_get(e, name);
			double      a = minic_val_to_d(ov), b = minic_val_to_d(dv);
			double      r = (op == TOK_PLUS_ASSIGN) ? a + b : (op == TOK_MINUS_ASSIGN) ? a - b : (op == TOK_MUL_ASSIGN) ? a * b : (b != 0.0 ? a / b : 0.0);
			minic_var_set(e, name, minic_val_coerce(r, ov.type));
			minic_expect(e, TOK_SEMICOLON);
			return;
		}

		if (e->lex.cur.type == TOK_LBRACKET) {
			minic_lex_next(&e->lex);
			int idx = (int)minic_val_to_d(minic_parse_expr(e));
			minic_expect(e, TOK_RBRACKET);
			minic_expect(e, TOK_ASSIGN);
			minic_val_t v = minic_parse_expr(e);
			minic_arr_elem_set(e, name, idx, v);
			minic_expect(e, TOK_SEMICOLON);
			return;
		}

		minic_expect(e, TOK_ASSIGN);
		minic_val_t v = minic_parse_expr(e);
		minic_var_set(e, name, v);
		minic_expect(e, TOK_SEMICOLON);
		return;
	}

	if (e->lex.cur.type == TOK_IF) {
		minic_lex_next(&e->lex);
		minic_expect(e, TOK_LPAREN);
		int taken = minic_val_is_true(minic_parse_cond(e));
		minic_expect(e, TOK_RPAREN);
		if (taken) {
			minic_parse_block(e);
		}
		else {
			minic_skip_block(e);
			taken = 0;
		}
		while (!taken && e->lex.cur.type == TOK_ELSE) {
			minic_lex_next(&e->lex);
			if (e->lex.cur.type == TOK_IF) {
				minic_lex_next(&e->lex);
				minic_expect(e, TOK_LPAREN);
				int c2 = minic_val_is_true(minic_parse_cond(e));
				minic_expect(e, TOK_RPAREN);
				if (c2) {
					minic_parse_block(e);
					taken = 1;
				}
				else {
					minic_skip_block(e);
				}
			}
			else {
				minic_parse_block(e);
				taken = 1;
			}
		}
		while (e->lex.cur.type == TOK_ELSE) {
			minic_lex_next(&e->lex);
			if (e->lex.cur.type == TOK_IF) {
				minic_lex_next(&e->lex);
				minic_expect(e, TOK_LPAREN);
				minic_parse_cond(e);
				minic_expect(e, TOK_RPAREN);
			}
			minic_skip_block(e);
		}
		return;
	}

	if (e->lex.cur.type == TOK_FOR) {
		minic_lex_next(&e->lex);
		minic_expect(e, TOK_LPAREN);

		if (e->lex.cur.type == TOK_INT || e->lex.cur.type == TOK_FLOAT || e->lex.cur.type == TOK_CHAR || e->lex.cur.type == TOK_DOUBLE ||
		    e->lex.cur.type == TOK_BOOL || e->lex.cur.type == TOK_VOID) {
			minic_lex_next(&e->lex);
		}
		{
			char iname[64];
			strncpy(iname, e->lex.cur.text, 63);
			minic_lex_next(&e->lex);
			minic_expect(e, TOK_ASSIGN);
			minic_val_t iv = minic_parse_expr(e);
			minic_var_set(e, iname, iv);
		}
		int cond_pos = e->lex.pos;
		minic_lex_next(&e->lex); // Consume ';'

		int incr_pos, body_pos;
		{
			minic_lexer_t tmp = {0};
			tmp.src           = e->lex.src;
			tmp.pos           = cond_pos;
			minic_lex_next(&tmp);
			while (tmp.cur.type != TOK_SEMICOLON && tmp.cur.type != TOK_EOF) {
				minic_lex_next(&tmp);
			}
			incr_pos = tmp.pos;
			minic_lex_next(&tmp);
			while (tmp.cur.type != TOK_RPAREN && tmp.cur.type != TOK_EOF) {
				minic_lex_next(&tmp);
			}
			minic_lex_next(&tmp);
			body_pos = tmp.pos - 1;
		}

		for (;;) {
			e->continuing = false;
			e->lex.pos    = cond_pos;
			minic_lex_next(&e->lex);
			int cond = minic_val_is_true(minic_parse_cond(e));
			if (!cond || e->returning || e->breaking) {
				e->lex.pos = body_pos;
				minic_lex_next(&e->lex);
				minic_skip_block(e);
				e->breaking = false;
				break;
			}
			e->lex.pos = body_pos;
			minic_lex_next(&e->lex);
			minic_parse_block(e);
			if (e->returning || e->breaking) {
				e->breaking = false;
				break;
			}
			e->lex.pos = incr_pos;
			minic_lex_next(&e->lex);
			if (e->lex.cur.type == TOK_INC || e->lex.cur.type == TOK_DEC) {
				double delta = MINIC_INC_DELTA(&e->lex);
				minic_lex_next(&e->lex);
				char iname[64];
				strncpy(iname, e->lex.cur.text, 63);
				minic_val_t ov = minic_var_get(e, iname);
				minic_var_set(e, iname, minic_val_coerce(minic_val_to_d(ov) + delta, ov.type));
			}
			else if (e->lex.cur.type == TOK_IDENT) {
				char iname[64];
				strncpy(iname, e->lex.cur.text, 63);
				minic_lex_next(&e->lex);
				if (e->lex.cur.type == TOK_INC || e->lex.cur.type == TOK_DEC) {
					double      delta = MINIC_INC_DELTA(&e->lex);
					minic_val_t ov    = minic_var_get(e, iname);
					minic_var_set(e, iname, minic_val_coerce(minic_val_to_d(ov) + delta, ov.type));
				}
				else if (e->lex.cur.type == TOK_PLUS_ASSIGN || e->lex.cur.type == TOK_MINUS_ASSIGN || e->lex.cur.type == TOK_MUL_ASSIGN ||
				         e->lex.cur.type == TOK_DIV_ASSIGN) {
					minic_tok_type_t op = e->lex.cur.type;
					minic_lex_next(&e->lex);
					minic_val_t dv = minic_parse_expr(e);
					minic_val_t ov = minic_var_get(e, iname);
					double      a = minic_val_to_d(ov), b = minic_val_to_d(dv);
					double r = (op == TOK_PLUS_ASSIGN) ? a + b : (op == TOK_MINUS_ASSIGN) ? a - b : (op == TOK_MUL_ASSIGN) ? a * b : (b != 0.0 ? a / b : 0.0);
					minic_var_set(e, iname, minic_val_coerce(r, ov.type));
				}
				else if (e->lex.cur.type == TOK_ASSIGN) {
					minic_lex_next(&e->lex);
					minic_val_t v = minic_parse_expr(e);
					minic_var_set(e, iname, v);
				}
			}
		}
		return;
	}

	if (e->lex.cur.type == TOK_STAR) {
		// Pointer write: *expr = val;  or  *expr += val;  or  *expr++;  etc.
		minic_lex_next(&e->lex);
		minic_val_t pv  = minic_parse_primary(e);
		void       *ptr = minic_val_to_ptr(pv);
		if (e->lex.cur.type == TOK_INC || e->lex.cur.type == TOK_DEC) {
			double delta = MINIC_INC_DELTA(&e->lex);
			minic_lex_next(&e->lex);
			if (ptr != NULL) {
				switch (pv.deref_type) {
				case MINIC_T_INT: {
					int ov = 0;
					memcpy(&ov, ptr, sizeof(int));
					ov += (int)delta;
					memcpy(ptr, &ov, sizeof(int));
					break;
				}
				case MINIC_T_FLOAT: {
					float ov = 0.0f;
					memcpy(&ov, ptr, sizeof(float));
					ov += (float)delta;
					memcpy(ptr, &ov, sizeof(float));
					break;
				}
				default: {
					minic_val_t ov;
					memcpy(&ov, ptr, sizeof(minic_val_t));
					minic_val_t nv = minic_val_coerce(minic_val_to_d(ov) + delta, ov.type);
					memcpy(ptr, &nv, sizeof(minic_val_t));
					break;
				}
				}
			}
			minic_expect(e, TOK_SEMICOLON);
			return;
		}
		minic_tok_type_t op = e->lex.cur.type; // TOK_ASSIGN, TOK_PLUS_ASSIGN, TOK_MINUS_ASSIGN, TOK_MUL_ASSIGN, TOK_DIV_ASSIGN
		minic_lex_next(&e->lex);
		minic_val_t v = minic_parse_expr(e);
		if (ptr != NULL) {
			switch (pv.deref_type) {
			case MINIC_T_INT: {
				int ov = 0;
				memcpy(&ov, ptr, sizeof(int));
				double b  = minic_val_to_d(v);
				double r  = (op == TOK_PLUS_ASSIGN)    ? ov + b
				            : (op == TOK_MINUS_ASSIGN) ? ov - b
				            : (op == TOK_MUL_ASSIGN)   ? ov * b
				            : (op == TOK_DIV_ASSIGN)   ? (b != 0.0 ? ov / b : 0.0)
				                                       : b;
				int    iv = (int)r;
				memcpy(ptr, &iv, sizeof(int));
				break;
			}
			case MINIC_T_FLOAT: {
				float ov = 0.0f;
				memcpy(&ov, ptr, sizeof(float));
				double b  = minic_val_to_d(v);
				double r  = (op == TOK_PLUS_ASSIGN)    ? ov + b
				            : (op == TOK_MINUS_ASSIGN) ? ov - b
				            : (op == TOK_MUL_ASSIGN)   ? ov * b
				            : (op == TOK_DIV_ASSIGN)   ? (b != 0.0 ? ov / b : 0.0)
				                                       : b;
				float  fv = (float)r;
				memcpy(ptr, &fv, sizeof(float));
				break;
			}
			default: {
				minic_val_t ov;
				memcpy(&ov, ptr, sizeof(minic_val_t));
				double      a = minic_val_to_d(ov), b = minic_val_to_d(v);
				double      r  = (op == TOK_PLUS_ASSIGN)    ? a + b
				                 : (op == TOK_MINUS_ASSIGN) ? a - b
				                 : (op == TOK_MUL_ASSIGN)   ? a * b
				                 : (op == TOK_DIV_ASSIGN)   ? (b != 0.0 ? a / b : 0.0)
				                                            : b;
				minic_val_t nv = (op == TOK_ASSIGN) ? v : minic_val_coerce(r, ov.type);
				memcpy(ptr, &nv, sizeof(minic_val_t));
				break;
			}
			}
		}
		minic_expect(e, TOK_SEMICOLON);
		return;
	}

	if (e->lex.cur.type == TOK_BREAK) {
		minic_lex_next(&e->lex);
		minic_expect(e, TOK_SEMICOLON);
		e->breaking = true;
		return;
	}
	if (e->lex.cur.type == TOK_CONTINUE) {
		minic_lex_next(&e->lex);
		minic_expect(e, TOK_SEMICOLON);
		e->continuing = true;
		return;
	}
	if (e->lex.cur.type == TOK_WHILE) {
		minic_lex_next(&e->lex);
		int cond_pos = e->lex.pos - 1;
		for (;;) {
			e->continuing = false;
			e->lex.pos    = cond_pos;
			minic_lex_next(&e->lex);
			minic_lex_next(&e->lex); // Consume '('
			int cond = minic_val_is_true(minic_parse_cond(e));
			minic_lex_next(&e->lex); // Consume ')'
			if (!cond || e->returning || e->breaking) {
				minic_skip_block(e);
				e->breaking = false;
				break;
			}
			minic_parse_block(e);
			if (e->breaking) {
				e->breaking = false;
				break;
			}
		}
		return;
	}

	{
		char msg[128];
		snprintf(msg, sizeof(msg), "unexpected token at start of statement");
		minic_error(e, msg);
	}
}

static void minic_parse_block(minic_env_t *e) {
	int saved_var_count     = e->var_count;
	int saved_vartype_count = e->vartype_count;
	if (e->lex.cur.type != TOK_LBRACE) {
		// Single-statement body without braces
		minic_parse_stmt(e);
		e->var_count     = saved_var_count;
		e->vartype_count = saved_vartype_count;
		return;
	}
	minic_expect(e, TOK_LBRACE);
	while (e->lex.cur.type != TOK_RBRACE && e->lex.cur.type != TOK_EOF && !e->returning && !e->breaking && !e->continuing && !e->error) {
		minic_parse_stmt(e);
	}
	if (e->lex.cur.type != TOK_RBRACE) {
		int depth = 1;
		while (depth > 0 && e->lex.cur.type != TOK_EOF) {
			if (e->lex.cur.type == TOK_LBRACE)
				depth++;
			if (e->lex.cur.type == TOK_RBRACE)
				depth--;
			minic_lex_next(&e->lex);
		}
	}
	else {
		minic_lex_next(&e->lex); // Consume '}'
	}
	e->var_count     = saved_var_count;
	e->vartype_count = saved_vartype_count;
}

// ██████╗ ██╗   ██╗███╗   ██╗
// ██╔══██╗██║   ██║████╗  ██║
// ██████╔╝██║   ██║██╔██╗ ██║
// ██╔══██╗██║   ██║██║╚██╗██║
// ██║  ██║╚██████╔╝██║ ╚████║
// ╚═╝  ╚═╝ ╚═════╝ ╚═╝  ╚═══╝

// Consume a type keyword (int, float, char, double, bool, void), return true if found
static bool minic_lex_type(minic_env_t *e) {
	if (e->lex.cur.type == TOK_INT || e->lex.cur.type == TOK_FLOAT || e->lex.cur.type == TOK_CHAR || e->lex.cur.type == TOK_DOUBLE ||
	    e->lex.cur.type == TOK_BOOL || e->lex.cur.type == TOK_VOID) {
		minic_lex_next(&e->lex);
		return true;
	}
	if (e->lex.cur.type == TOK_STRUCT) {
		minic_lex_next(&e->lex); // Consume 'struct'
		minic_lex_next(&e->lex); // Consume struct name
		return true;
	}
	// Typedef'd struct name used as a type specifier
	if (e->lex.cur.type == TOK_IDENT && minic_struct_get(e, e->lex.cur.text) != NULL) {
		minic_lex_next(&e->lex);
		return true;
	}
	// Typedef'd int name (e.g. from typedef enum) used as a type specifier
	if (e->lex.cur.type == TOK_IDENT && minic_is_int_typedef(e->lex.cur.text)) {
		minic_lex_next(&e->lex);
		return true;
	}
	return false;
}

// Zero pass: scan for struct definitions or typedef struct
static void minic_register_structs(minic_env_t *e) {
	minic_lexer_t l = {0};
	l.src           = e->lex.src;
	l.pos           = 0;
	minic_lex_next(&l);
	while (l.cur.type != TOK_EOF) {
		bool is_typedef = (l.cur.type == TOK_TYPEDEF);
		if (is_typedef) {
			minic_lex_next(&l); // Consume 'typedef'
		}
		if (l.cur.type == TOK_ENUM) {
			minic_lex_next(&l); // Consume 'enum'
			if (l.cur.type == TOK_IDENT) {
				minic_lex_next(&l); // Optional tag name
			}
			if (l.cur.type != TOK_LBRACE) {
				continue;
			}
			minic_lex_next(&l); // Consume '{'
			int val = 0;
			while (l.cur.type != TOK_RBRACE && l.cur.type != TOK_EOF) {
				if (l.cur.type == TOK_IDENT && minic_enum_const_count < MINIC_MAX_ENUM_CONSTS) {
					strncpy(minic_enum_consts[minic_enum_const_count].name, l.cur.text, 63);
					minic_enum_consts[minic_enum_const_count].value = val;
					minic_enum_const_count++;
					minic_lex_next(&l);
					if (l.cur.type == TOK_ASSIGN) {
						minic_lex_next(&l); // Consume '='
						val                                                 = (int)minic_val_to_d(l.cur.val);
						minic_enum_consts[minic_enum_const_count - 1].value = val;
						minic_lex_next(&l); // Consume number
					}
					val++;
				}
				else {
					minic_lex_next(&l);
				}
				if (l.cur.type == TOK_COMMA) {
					minic_lex_next(&l);
				}
			}
			if (l.cur.type == TOK_RBRACE) {
				minic_lex_next(&l);
			}
			if (is_typedef && l.cur.type == TOK_IDENT && minic_int_typedef_count < MINIC_MAX_INT_TYPEDEFS) {
				strncpy(minic_int_typedefs[minic_int_typedef_count++].name, l.cur.text, 63);
				minic_lex_next(&l);
			}
			while (l.cur.type != TOK_SEMICOLON && l.cur.type != TOK_EOF) {
				minic_lex_next(&l);
			}
			if (l.cur.type == TOK_SEMICOLON) {
				minic_lex_next(&l);
			}
			continue;
		}

		if (l.cur.type != TOK_STRUCT) {
			minic_lex_next(&l);
			continue;
		}
		minic_lex_next(&l); // Consume 'struct'

		// Optional struct tag name
		char struct_name[64] = "";
		if (l.cur.type == TOK_IDENT) {
			strncpy(struct_name, l.cur.text, 63);
			minic_lex_next(&l); // Consume struct name
		}

		if (l.cur.type != TOK_LBRACE) {
			continue; // Forward decl or typedef-without-body — skip
		}

		if (e->struct_count >= e->struct_cap) {
			break;
		}
		minic_struct_def_t *def = &e->structs[e->struct_count];
		strncpy(def->name, struct_name, 63);
		def->field_count = 0;
		minic_lex_next(&l); // Consume '{'

		while (l.cur.type != TOK_RBRACE && l.cur.type != TOK_EOF) {
			if (l.cur.type == TOK_STRUCT) {
				minic_lex_next(&l);
				minic_lex_next(&l);
			}
			else if (l.cur.type == TOK_INT || l.cur.type == TOK_FLOAT || l.cur.type == TOK_CHAR || l.cur.type == TOK_DOUBLE || l.cur.type == TOK_BOOL) {
				minic_lex_next(&l);
			}
			else if (l.cur.type == TOK_IDENT) {
				minic_lex_next(&l); // Typedef'd field type — consume and fall through to field name
			}
			else {
				minic_lex_next(&l);
				continue;
			}
			if (l.cur.type == TOK_STAR) {
				minic_lex_next(&l);
			}
			if (l.cur.type == TOK_IDENT && def->field_count < MINIC_MAX_STRUCT_FIELDS) {
				strncpy(def->fields[def->field_count++], l.cur.text, 63);
				minic_lex_next(&l);
			}
			while (l.cur.type != TOK_SEMICOLON && l.cur.type != TOK_RBRACE && l.cur.type != TOK_EOF) {
				minic_lex_next(&l);
			}
			if (l.cur.type == TOK_SEMICOLON) {
				minic_lex_next(&l);
			}
		}
		if (l.cur.type == TOK_RBRACE) {
			minic_lex_next(&l);
		}

		if (is_typedef && l.cur.type == TOK_IDENT) {
			// typedef struct [Name] { ... } alias;
			char alias[64];
			strncpy(alias, l.cur.text, 63);
			minic_lex_next(&l); // Consume alias name
			if (struct_name[0]) {
				// Register under struct tag name first
				e->struct_count++;
				// Also register a copy under the alias name
				if (e->struct_count < e->struct_cap) {
					minic_struct_def_t *adef = &e->structs[e->struct_count];
					*adef                    = *def;
					strncpy(adef->name, alias, 63);
					e->struct_count++;
				}
			}
			else {
				// Anonymous struct: name it after the alias
				strncpy(def->name, alias, 63);
				e->struct_count++;
			}
		}
		else {
			// Plain struct definition: must have a tag name to be usable
			if (struct_name[0]) {
				e->struct_count++;
			}
		}

		while (l.cur.type != TOK_SEMICOLON && l.cur.type != TOK_EOF) {
			minic_lex_next(&l);
		}
		if (l.cur.type == TOK_SEMICOLON) {
			minic_lex_next(&l);
		}
	}
}

// First pass: register all function definitions, stop at 'main'
static void minic_register_funcs(minic_env_t *e) {
	while (e->lex.cur.type != TOK_EOF) {
		// Remember the return-type token before consuming it
		minic_type_t ret_type = minic_tok_to_type(e->lex.cur.type);

		char decl_struct[64] = "";
		if (e->lex.cur.type == TOK_IDENT && minic_struct_get(e, e->lex.cur.text) != NULL) {
			strncpy(decl_struct, e->lex.cur.text, 63);
		}
		if (!minic_lex_type(e)) {
			if (e->lex.cur.type == TOK_IDENT) {
				// Unknown typedef'd type (e.g. ui_handle_t) — consume it and
				// treat as an opaque pointer type so the variable/function is
				// registered properly.
				strncpy(decl_struct, e->lex.cur.text, 63);
				minic_lex_next(&e->lex);
				ret_type = MINIC_T_PTR;
			}
			else {
				minic_lex_next(&e->lex);
				continue;
			}
		}
		if (e->lex.cur.type == TOK_STAR) {
			ret_type = MINIC_T_PTR;
			minic_lex_next(&e->lex);
		}
		if (e->lex.cur.type != TOK_IDENT) {
			continue;
		}
		char fname[64];
		strncpy(fname, e->lex.cur.text, 63);
		minic_lex_next(&e->lex);

		if (e->lex.cur.type != TOK_LPAREN) {
			// Global variable declaration: type [*] ident [= expr] ;
			if (decl_struct[0] != '\0') {
				minic_vartype_set(e, fname, decl_struct);
			}
			minic_val_t init = minic_val_coerce(0.0, ret_type);
			if (e->lex.cur.type == TOK_ASSIGN) {
				minic_lex_next(&e->lex); // Consume '='
				e->decl_type = ret_type;
				init         = minic_parse_expr(e);
			}
			minic_var_decl(e, fname, ret_type, init);
			while (e->lex.cur.type != TOK_SEMICOLON && e->lex.cur.type != TOK_EOF) {
				minic_lex_next(&e->lex);
			}
			if (e->lex.cur.type == TOK_SEMICOLON) {
				minic_lex_next(&e->lex);
			}
			continue;
		}
		minic_lex_next(&e->lex); // Consume '('

		minic_func_t fn = {0};
		strncpy(fn.name, fname, 63);
		fn.ret_type = ret_type;

		while (e->lex.cur.type != TOK_RPAREN && e->lex.cur.type != TOK_EOF) {
			char         pstruct[64] = "";
			minic_type_t ptype       = MINIC_T_INT;
			if (e->lex.cur.type == TOK_STRUCT) {
				minic_lex_next(&e->lex);
				strncpy(pstruct, e->lex.cur.text, 63);
				minic_lex_next(&e->lex);
				ptype = MINIC_T_PTR;
			}
			else {
				// Capture typedef'd struct name before consuming the type token
				if (e->lex.cur.type == TOK_IDENT && minic_struct_get(e, e->lex.cur.text) != NULL) {
					strncpy(pstruct, e->lex.cur.text, 63);
				}
				ptype = minic_tok_to_type(e->lex.cur.type);
				minic_lex_type(e); // consume type keyword
			}
			if (e->lex.cur.type == TOK_STAR) {
				ptype = MINIC_T_PTR;
				minic_lex_next(&e->lex);
			}
			if (e->lex.cur.type == TOK_IDENT && fn.param_count < MINIC_MAX_PARAMS) {
				int pi = fn.param_count++;
				strncpy(fn.params[pi], e->lex.cur.text, 63);
				strncpy(fn.param_structs[pi], pstruct, 63);
				fn.param_types[pi] = ptype;
				minic_lex_next(&e->lex);
			}
			if (e->lex.cur.type == TOK_COMMA) {
				minic_lex_next(&e->lex);
			}
		}
		minic_lex_next(&e->lex); // Consume ')'

		fn.body_pos = e->lex.pos - 1;

		if (strcmp(fname, "main") == 0) {
			break;
		}
		if (e->func_count < e->func_cap) {
			e->funcs[e->func_count++] = fn;
		}

		// Skip function body
		int depth = 1;
		minic_lex_next(&e->lex); // Consume '{'
		while (depth > 0 && e->lex.cur.type != TOK_EOF) {
			if (e->lex.cur.type == TOK_LBRACE)
				depth++;
			if (e->lex.cur.type == TOK_RBRACE)
				depth--;
			minic_lex_next(&e->lex);
		}
	}
}

minic_ctx_t *minic_eval_named(const char *src, const char *filename) {
	minic_register_builtins();

	minic_ctx_t *ctx = (minic_ctx_t *)malloc(sizeof(minic_ctx_t));
	memset(ctx, 0, sizeof(minic_ctx_t));
	ctx->mem      = (minic_u8 *)malloc(MINIC_MEM_SIZE);
	ctx->mem_used = 0;
	// Copy source so the context stays valid after the caller frees its buffer
	int src_len   = (int)strlen(src);
	ctx->src_copy = (char *)malloc(src_len + 1);
	memcpy(ctx->src_copy, src, src_len + 1);

	// Save and install arena pointers so minic_alloc and the lexer use this context
	minic_u8 *prev_mem      = minic_active_mem;
	int      *prev_mem_used = minic_active_mem_used;
	minic_active_mem        = ctx->mem;
	minic_active_mem_used   = &ctx->mem_used;

	int var_cap      = 64;
	int arr_cap      = 32;
	int arr_data_cap = 512;
	int func_cap     = 32;
	int struct_cap   = MINIC_MAX_STRUCTS;

	minic_env_t *e    = &ctx->e;
	e->lex.src        = ctx->src_copy;
	e->lex.pos        = 0;
	e->filename       = filename;
	e->var_cap        = var_cap;
	e->vars           = minic_alloc(var_cap * sizeof(minic_var_t));
	e->arr_cap        = arr_cap;
	e->arrs           = minic_alloc(arr_cap * sizeof(minic_arr_t));
	e->arr_data       = minic_alloc(arr_data_cap * sizeof(minic_val_t));
	e->arr_data_used  = minic_alloc(sizeof(int));
	*e->arr_data_used = 0;
	e->func_cap       = func_cap;
	e->funcs          = minic_alloc(func_cap * sizeof(minic_func_t));
	e->struct_cap     = struct_cap;
	e->structs        = minic_alloc(struct_cap * sizeof(minic_struct_def_t));
	e->vartype_cap    = 64;
	e->vartypes       = minic_alloc(e->vartype_cap * sizeof(minic_vartype_t));

	// Seed env with globally pre-registered struct definitions
	for (int i = 0; i < minic_global_struct_count && e->struct_count < e->struct_cap; ++i) {
		minic_struct_def_t *dst = &e->structs[e->struct_count++];
		strncpy(dst->name, minic_global_structs[i].name, 63);
		dst->field_count       = minic_global_structs[i].field_count;
		dst->size              = minic_global_structs[i].size;
		dst->has_native_layout = minic_global_structs[i].has_native_layout;
		for (int j = 0; j < dst->field_count; ++j) {
			strncpy(dst->fields[j], minic_global_structs[i].fields[j], 63);
			strncpy(dst->field_struct_names[j], minic_global_structs[i].field_struct_names[j], 63);
			dst->field_offsets[j]      = minic_global_structs[i].field_offsets[j];
			dst->field_native_types[j] = minic_global_structs[i].field_native_types[j];
			dst->field_deref_types[j]  = minic_global_structs[i].field_deref_types[j];
		}
	}

	minic_register_structs(e);
	minic_lex_next(&e->lex);
	minic_register_funcs(e);
	for (int i = 0; i < e->func_count; ++i) {
		e->funcs[i].ctx = ctx;
	}

	if (e->lex.cur.type == TOK_RPAREN) {
		minic_lex_next(&e->lex);
	}

	minic_env_t *prev_env = minic_active_env;
	minic_active_env      = e;
	minic_parse_block(e);
	minic_active_env      = prev_env;
	minic_active_mem      = prev_mem;
	minic_active_mem_used = prev_mem_used;

	ctx->result = e->error ? -1.0f : (float)minic_val_to_d(e->return_val);
	return ctx;
}

minic_ctx_t *minic_eval(const char *src) {
	return minic_eval_named(src, "<script>");
}

void minic_ctx_free(minic_ctx_t *ctx) {
	if (ctx) {
		free(ctx->mem);
		free(ctx->src_copy);
		free(ctx);
	}
}

float minic_ctx_result(minic_ctx_t *ctx) {
	return ctx ? ctx->result : -1.0f;
}

minic_val_t minic_ctx_call_fn(minic_ctx_t *ctx, void *fn_ptr, minic_val_t *args, int argc) {
	if (!ctx || !fn_ptr) {
		return minic_val_int(0);
	}
	minic_u8    *prev_mem      = minic_active_mem;
	int         *prev_mem_used = minic_active_mem_used;
	minic_env_t *prev_env      = minic_active_env;
	minic_active_mem           = ctx->mem;
	minic_active_mem_used      = &ctx->mem_used;
	minic_active_env           = &ctx->e;
	int         saved_mem_used = ctx->mem_used;
	minic_val_t r              = minic_call(&ctx->e, (minic_func_t *)fn_ptr, args, argc);
	ctx->mem_used              = saved_mem_used;
	minic_active_env           = prev_env;
	minic_active_mem           = prev_mem;
	minic_active_mem_used      = prev_mem_used;
	return r;
}
