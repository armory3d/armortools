#include <iron.h>
#include <stdio.h>

typedef struct any_map        *string_map_t;
typedef struct string_t_array *string_array_t;
typedef struct string_t_array {
	string_t **buffer;
	int        length;
	int        capacity;
} string_t_array_t;
typedef struct string_array_t_array {
	string_array_t **buffer;
	int              length;
	int              capacity;
} string_array_t_array_t;

bool              is_alpha_numeric(i32 code);
bool              is_alpha(i32 code);
bool              is_numeric(i32 code);
bool              is_white_space(i32 code);
string_t         *read_token();
void              alang_parse();
void              handle_tabs(string_t *token);
void              handle_new_line(string_t *token);
void              handle_spaces(string_t *token);
string_t         *get_token(i32 off);
string_t         *get_token_after_piece();
void              skip_until(string_t *s);
void              skip_block();
string_t         *function_return_type();
string_t         *join_type_name(string_t *type, string_t *name);
string_t         *read_type();
string_t         *enum_access(string_t *s);
string_t         *struct_access(string_t *s);
string_t         *struct_alloc(string_t *token, string_t *type);
string_t         *array_type(string_t *name);
string_t_array_t *array_contents(string_t *type);
string_t         *member_name(string_t *name);
string_t         *array_create(string_t *token, string_t *name, string_t *content_type);
string_t         *array_access(string_t *token);
bool              is_struct(string_t *type);
string_t         *strip(string_t *name, i32 len);
string_t         *strip_optional(string_t *name);
i32               param_pos();
void              param_pos_add();
string_t         *fn_call();
string_t         *fill_fn_params(string_t *token);
string_t         *get_token_c();
string_t         *fill_default_params(string_t *piece);
void              skip_piece(i32 nested);
string_t         *read_piece(i32 nested);
string_t         *string_len(string_t *token);
string_t         *value_type(string_t *value);
void              set_fn_param_types();
string_t         *number_to_string(string_t *token);
bool              token_is_string(string_t *token);
string_t         *string_add(string_t *token);
string_t         *string_ops(string_t *token);
string_t         *array_ops(string_t *token);
string_t         *map_ops(string_t *token);
string_t         *_t_to_struct(string_t *type);
void              stream_write(string_t *token);
void              string_write(string_t *token);
void              null_write(string_t *token);
void              write_enums();
void              write_types();
void              write_array_types();
void              write_fn_declarations();
void              write_globals();
void              write_kickstart();
void              write_function();
void              write_functions();
void              write_iron_c();
void              alang(string_t *_alang_source, string_t *_alang_output);

string_t               *alang_output;
string_t               *alang_source;
i32                     alang_source_length;
string_t_array_t       *specials;
bool                    is_comment;
bool                    is_string;
i32                     pos;
string_t_array_t       *tokens;
string_t               *header;
any                     fhandle;
string_t_array_t       *strings;
i32                     tabs;
bool                    new_line;
string_t_array_t       *basic_types;
string_t_array_t       *pass_by_value_types;
string_t_array_t       *enums;
any_map_t              *value_types;
any_map_t              *struct_types;
any_map_t              *fn_declarations;
any_map_t              *fn_default_params;
string_t_array_t       *fn_call_stack;
i32_array_t            *param_pos_stack;
bool                    is_for_loop;
string_t_array_t       *global_inits;
string_t_array_t       *global_ptrs;
string_array_t_array_t *acontents;
i32_array_t            *fnested;
string_t_array_t       *add_space_keywords;
i32                     array_contents_depth;
void (*out)(string_t *);

void _kickstart() {
	alang_output = null;
	alang_source = null;
	gc_unroot(specials);
	specials = any_array_create_from_raw(
	    (any[]){
	        ":",
	        ";",
	        ",",
	        "(",
	        ")",
	        "[",
	        "]",
	        "{",
	        "}",
	        "<",
	        ">",
	        "!",
	    },
	    12);
	gc_root(specials);
	is_comment = false;
	is_string  = false;
	pos        = 0;
	gc_unroot(tokens);
	tokens = any_array_create_from_raw((any[]){}, 0);
	gc_root(tokens);
	header = "";
	gc_unroot(strings);
	strings = any_array_create_from_raw((any[]){}, 0);
	gc_root(strings);
	tabs     = 0;
	new_line = false;
	gc_unroot(basic_types);
	basic_types = any_array_create_from_raw(
	    (any[]){
	        "i8",
	        "u8",
	        "i16",
	        "u16",
	        "i32",
	        "u32",
	        "f32",
	        "i64",
	        "u64",
	        "f64",
	        "bool",
	    },
	    11);
	gc_root(basic_types);
	gc_unroot(pass_by_value_types);
	pass_by_value_types = any_array_create_from_raw(
	    (any[]){
	        "vec4_t",
	        "vec3_t",
	        "vec2_t",
	        "quat_t",
	        "mat4_t",
	        "mat3_t",
	    },
	    6);
	gc_root(pass_by_value_types);
	gc_unroot(enums);
	enums = any_array_create_from_raw((any[]){}, 0);
	gc_root(enums);
	gc_unroot(value_types);
	value_types = any_map_create();
	gc_root(value_types);
	gc_unroot(struct_types);
	struct_types = any_map_create();
	gc_root(struct_types);
	gc_unroot(fn_call_stack);
	fn_call_stack = any_array_create_from_raw((any[]){}, 0);
	gc_root(fn_call_stack);
	gc_unroot(param_pos_stack);
	param_pos_stack = i32_array_create_from_raw((i32[]){}, 0);
	gc_root(param_pos_stack);
	is_for_loop = false;
	gc_unroot(global_inits);
	global_inits = any_array_create_from_raw((any[]){}, 0);
	gc_root(global_inits);
	gc_unroot(global_ptrs);
	global_ptrs = any_array_create_from_raw((any[]){}, 0);
	gc_root(global_ptrs);
	gc_unroot(acontents);
	acontents = any_array_create_from_raw((any[]){}, 0);
	gc_root(acontents);
	gc_unroot(fnested);
	fnested = i32_array_create_from_raw((i32[]){}, 0);
	gc_root(fnested);
	gc_unroot(add_space_keywords);
	add_space_keywords = any_array_create_from_raw(
	    (any[]){
	        "return",
	        "else",
	    },
	    2);
	gc_root(add_space_keywords);
	array_contents_depth = 0;
	gc_unroot(out);
	out = stream_write;
	gc_root(out);
}

bool is_alpha_numeric(i32 code) {
	return (code > 47 && code < 58) || (code == 45) || (code == 43) || (code == 95) || (code == 46) || (code > 64 && code < 91) || (code > 96 && code < 123);
}

bool is_alpha(i32 code) {
	return (code > 96 && code < 123) || (code > 64 && code < 91) || (code == 95);
}

bool is_numeric(i32 code) {
	return (code > 47 && code < 58);
}

bool is_white_space(i32 code) {
	return code == 32 || code == 9 || code == 13 || code == 10;
}

string_t *read_token() {
	while (is_white_space(char_code_at(alang_source, pos))) {
		pos++;
	}
	string_t *token   = "";
	bool      first   = true;
	bool      is_anum = false;
	while (pos < alang_source_length) {
		string_t *c = char_at(alang_source, pos);
		if (string_equals(token, "//")) {
			is_comment = true;
		}
		if (is_comment) {
			token = string_join(token, c);
			pos++;
			if (string_equals(c, "\n")) {
				is_comment = false;
				if (starts_with(token, "/// include") || starts_with(token, "/// define")) {
					gc_unroot(header);
					header = string_join(header, string_join("#", substring(token, 4, string_length(token))));
					gc_root(header);
				}
				return read_token();
			}
			continue;
		}
		if (string_equals(c, "\"")) {
			string_t *last       = string_length(token) > 0 ? char_at(token, string_length(token) - 1) : "";
			string_t *last_last  = string_length(token) > 1 ? char_at(token, string_length(token) - 2) : "";
			bool      is_escaped = string_equals(last, "\\") && !string_equals(last_last, "\\");
			if (!is_escaped) {
				is_string = !is_string;
			}
			if (!is_string) {
				token = string_join(token, c);
				pos++;
				break;
			}
		}
		if (is_string) {
			if (string_equals(c, "\r")) {
				pos++;
				continue;
			}
			token = string_join(token, c);
			pos++;
			continue;
		}
		if (is_white_space(char_code_at(alang_source, pos))) {
			break;
		}
		if (first) {
			first   = false;
			is_anum = is_alpha_numeric(char_code_at(c, 0));
		}
		bool is_special = char_ptr_array_index_of(specials, c) > -1;
		if (is_anum && is_special) {
			break;
		}
		token = string_join(token, c);
		pos++;
		if (is_special) {
			break;
		}
	}
	return token;
}

void alang_parse() {
	pos = 0;
	while (true) {
		string_t *token = read_token();
		if (string_equals(token, "=") && string_equals(tokens->buffer[tokens->length - 1], "!")) {
			tokens->buffer[tokens->length - 1] = "!=";
			continue;
		}
		if (string_equals(token, "")) {
			break;
		}
		any_array_push(tokens, token);
	}
}

void handle_tabs(string_t *token) {
	if (string_equals(token, "{")) {
		tabs++;
	}
	else if (string_equals(token, "}")) {
		tabs--;
	}
	if (is_for_loop) {
		return;
	}
	if (new_line) {
		for (i32 i = 0; i < tabs; ++i) {
			out("\t");
		}
	}
}

void handle_new_line(string_t *token) {
	if (is_for_loop) {
		return;
	}
	new_line = string_equals(token, ";") || string_equals(token, "{") || string_equals(token, "}");
	if (new_line) {
		out("\n");
	}
}

void handle_spaces(string_t *token) {
	if (char_ptr_array_index_of(add_space_keywords, token) > -1) {
		out(" ");
	}
}

string_t *get_token(i32 off) {
	if (pos + off >= 0 && pos + off < tokens->length) {
		return tokens->buffer[pos + off];
	}
	return "";
}

string_t *get_token_after_piece() {
	i32 _pos = pos;
	skip_piece(0);
	string_t *t = get_token(1);
	pos         = _pos;
	return t;
}

void skip_until(string_t *s) {
	while (true) {
		string_t *token = get_token(0);
		if (string_equals(token, s)) {
			break;
		}
		pos++;
	}
}

void skip_block() {
	pos++;
	i32 nested = 1;
	while (true) {
		string_t *token = get_token(0);
		if (string_equals(token, "}")) {
			nested--;
			if (nested == 0) {
				break;
			}
		}
		else if (string_equals(token, "{")) {
			nested++;
		}
		pos++;
	}
}

string_t *function_return_type() {
	i32 _pos = pos;
	skip_until("{");
	string_t *t = get_token(-1);
	if (string_equals(t, "]")) {
		pos -= 2;
	}
	else if (string_equals(t, ">")) {
		pos -= 5;
	}
	pos -= 2;
	string_t *result = string_equals(get_token(0), ":") ? read_type() : "void";
	pos              = _pos;
	return result;
}

string_t *join_type_name(string_t *type, string_t *name) {
	any_map_set(value_types, name, type);
	if (string_index_of(type, "(*NAME)(") > 0) {
		return string_replace_all(type, "NAME", name);
	}
	return string_join(string_join(type, " "), name);
}

string_t *read_type() {
	pos++;
	string_t *type = get_token(0);
	if (string_equals(type, "(") && string_equals(get_token(1), "(")) {
		pos++;
	}
	if (string_equals(get_token(1), ")") && string_equals(get_token(2), "[")) {
		pos++;
	}
	if (string_equals(type, "(")) {
		string_t *params = "(";
		if (!string_equals(get_token(1), ")")) {
			while (true) {
				skip_until(":");
				params = string_join(params, read_type());
				pos++;
				if (string_equals(get_token(0), ")")) {
					break;
				}
				params = string_join(params, ",");
			}
		}
		else {
			params = string_join(params, "void");
		}
		params = string_join(params, ")");
		skip_until("=>");
		string_t *ret = read_type();
		type          = string_join(string_join(ret, "(*NAME)"), params);
	}
	if (string_equals(type, "color_t")) {
		type = "i32";
	}
	else if (string_equals(type, "string")) {
		type = "string_t";
	}
	if (string_equals(get_token(1), "[")) {
		if (is_struct(type)) {
			type = string_join(type, "_array_t");
		}
		else if (string_equals(type, "bool")) {
			type = "u8_array_t";
		}
		else {
			type = string_join(type, "_array_t");
		}
		pos += 2;
	}
	if (is_struct(type) && char_ptr_array_index_of(pass_by_value_types, type) == -1) {
		type = string_join(type, " *");
	}
	if (string_equals(type, "map_t *") && string_equals(get_token(1), "<")) {
		string_t *mv = get_token(4);
		if (string_equals(mv, "f32")) {
			type = string_join("f32_", type);
		}
		else if (char_ptr_array_index_of(basic_types, mv) > -1) {
			string_t *mk = get_token(2);
			if (string_equals(mk, "i32")) {
				type = string_join("i32_i", type);
			}
			else {
				type = string_join("i32_", type);
			}
		}
		else {
			string_t *mk = get_token(2);
			if (string_equals(mk, "i32")) {
				type = string_join("any_i", type);
			}
			else {
				type = string_join("any_", type);
			}
		}
		skip_until(">");
	}
	return type;
}

string_t *enum_access(string_t *s) {
	if (string_index_of(s, "_t.") > -1) {
		for (i32 i = 0; i < enums->length; ++i) {
			string_t *e = enums->buffer[i];
			if (string_index_of(s, e) > -1) {
				s = string_copy(string_replace_all(s, "_t.", "_"));
				s = string_copy(to_upper_case(s));
				break;
			}
		}
	}
	return s;
}

string_t *struct_access(string_t *s) {
	i32 dot = string_index_of(s, ".");
	if (dot == -1) {
		return s;
	}
	if (is_numeric(char_code_at(s, dot + 1))) {
		return s;
	}
	if (string_index_of(s, "\"") > -1) {
		return s;
	}
	if (starts_with(s, "GC_ALLOC_INIT")) {
		return s;
	}
	bool      has_minus = starts_with(s, "-");
	string_t *base      = substring(s, has_minus ? 1 : 0, dot);
	string_t *type      = any_map_get(value_types, base);
	if (type != null) {
		i32 dot_last = string_last_index_of(s, ".");
		if (dot != dot_last) {
			string_t_array_t *parts = string_split(s, ".");
			for (i32 i = 1; i < parts->length - 1; ++i) {
				any_map_t *struct_value_types = any_map_get(struct_types, type);
				if (struct_value_types != null) {
					type = string_copy(any_map_get(struct_value_types, parts->buffer[i]));
					if (starts_with(type, "struct ") && ends_with(type, " *")) {
						type = string_copy(substring(type, 0, string_length(type) - 2));
						type = string_join(substring(type, 7, string_length(type)), "_t *");
					}
				}
			}
			if (!ends_with(type, " *")) {
				string_t *first = string_replace_all(substring(s, 0, dot_last), ".", "->");
				string_t *last  = substring(s, dot_last, string_length(s));
				return string_join(first, last);
			}
		}
		else if (!ends_with(type, " *")) {
			return s;
		}
	}
	return string_replace_all(s, ".", "->");
}

string_t *struct_alloc(string_t *token, string_t *type) {
	if ((string_equals(get_token(-1), "=") && string_equals(token, "{") && !string_equals(get_token(-3), "type")) || type != null) {
		if (type == null) {
			i32       i = string_equals(get_token(-3), ":") ? -4 : -2;
			string_t *t = struct_access(get_token(i));
			type        = string_copy(value_type(t));
			if (ends_with(type, " *")) {
				type = string_copy(substring(type, 0, string_length(type) - 2));
			}
			if (starts_with(type, "struct ")) {
				type = string_join(substring(type, 7, string_length(type)), "_t");
			}
		}
		token = string_join(string_join("GC_ALLOC_INIT(", type), ", {");
		pos++;
		i32 ternary = 0;
		while (true) {
			string_t *t = get_token(0);
			if (string_equals(t, "}")) {
				break;
			}
			string_t *tc = get_token_c();
			tc           = string_copy(string_ops(tc));
			if (string_equals(get_token(1), ":") && !ternary) {
				tc = string_join(".", tc);
			}
			if (string_equals(t, ":")) {
				if (ternary > 0) {
					ternary--;
				}
				else {
					tc = "=";
				}
			}
			else if (string_equals(t, "?")) {
				ternary++;
			}
			else if (string_equals(t, "[") && string_equals(get_token(-1), ":")) {
				string_t  *member       = get_token(-2);
				any_map_t *types        = any_map_get(struct_types, string_join(type, " *"));
				string_t  *content_type = any_map_get(types, member);
				content_type            = string_copy(substring(content_type, 7, string_length(content_type) - 8));
				tc                      = string_copy(array_create("[", "", content_type));
			}
			token = string_join(token, tc);
			pos++;
		}
		if (string_equals(get_token(-1), "{")) {
			token = string_join(token, "0");
		}
		token = string_join(token, get_token(0));
		token = string_join(token, ")");
		tabs--;
	}
	return token;
}

string_t *array_type(string_t *name) {
	string_t *type = value_type(name);
	if (type == null) {
		return "any";
	}
	if (starts_with(type, "struct ")) {
		type = string_join(substring(type, 7, string_length(type) - 2), "_t *");
	}
	type = string_copy(strip(type, 10));
	if (char_ptr_array_index_of(basic_types, type) > -1) {
		return type;
	}
	return "any";
}

string_t_array_t *array_contents(string_t *type) {
	string_t_array_t *contents = acontents->buffer[array_contents_depth];
	array_contents_depth++;
	contents->length  = 0;
	string_t *content = "";
	while (true) {
		pos++;
		string_t *token = get_token(0);
		if (string_equals(token, "]")) {
			if (!string_equals(content, "")) {
				any_array_push(contents, content);
			}
			break;
		}
		if (string_equals(token, ",")) {
			any_array_push(contents, content);
			content = "";
			continue;
		}
		if (string_equals(token, "{")) {
			token = string_copy(struct_alloc(token, type));
			tabs++;
		}
		if (string_equals(get_token(1), "(")) {
			while (true) {
				pos++;
				token = string_join(token, get_token(0));
				token = string_copy(fill_default_params(token));
				if (ends_with(token, ")")) {
					break;
				}
			}
		}
		if (string_equals(token, "/")) {
			token = "/(float)";
		}
		token   = string_copy(struct_access(token));
		token   = string_copy(string_ops(token));
		content = string_join(content, token);
	}
	array_contents_depth--;
	return contents;
}

string_t *member_name(string_t *name) {
	i32 i = string_last_index_of(name, ".");
	if (i > -1) {
		name = string_copy(substring(name, i + 1, string_length(name)));
	}
	return name;
}

string_t *array_create(string_t *token, string_t *name, string_t *content_type) {
	if ((string_equals(get_token(-1), "=") && string_equals(token, "[")) || content_type != null) {
		if (string_equals(name, "]")) {
			name = string_copy(get_token(-6));
		}
		string_t *member = member_name(name);
		string_t *type   = array_type(member);
		if (content_type == null) {
			name         = string_copy(struct_access(name));
			content_type = string_copy(value_type(name));
			if (content_type != null) {
				content_type = string_copy(substring(content_type, 0, string_length(content_type) - 10));
			}
		}
		string_t_array_t *contents = array_contents(content_type);
		token                      = string_join(string_join(string_join(type, "_array_create_from_raw(("), type), "[]){");
		for (i32 i = 0; i < contents->length; ++i) {
			string_t *e = contents->buffer[i];
			token       = string_join(token, string_join(e, ","));
		}
		i32 len = contents->length;
		token   = string_join(token, string_join(string_join("},", i32_to_string(len)), ")"));
	}
	return token;
}

string_t *array_access(string_t *token) {
	if (string_equals(token, "[") && !string_equals(get_token(-1), "=")) {
		return "->buffer[";
	}
	return token;
}

bool is_struct(string_t *type) {
	return ends_with(type, "_t") && char_ptr_array_index_of(enums, type) == -1;
}

string_t *strip(string_t *name, i32 len) {
	return substring(name, 0, string_length(name) - len);
}

string_t *strip_optional(string_t *name) {
	if (ends_with(name, "?")) {
		return strip(name, 1);
	}
	return name;
}

i32 param_pos() {
	return param_pos_stack->buffer[param_pos_stack->length - 1];
}

void param_pos_add() {
	param_pos_stack->buffer[param_pos_stack->length - 1]++;
}

string_t *fn_call() {
	return fn_call_stack->buffer[fn_call_stack->length - 1];
}

string_t *fill_fn_params(string_t *token) {
	if (string_equals(token, "(")) {
		any_array_push(fn_call_stack, get_token(-1));
		i32_array_push(param_pos_stack, 0);
	}
	else if (string_equals(token, ",")) {
		param_pos_add();
	}
	else if (string_equals(token, ")")) {
		if (!string_equals(get_token(-1), "(")) {
			param_pos_add();
		}
		string_t *res = "";
		while (true) {
			i32       i     = param_pos();
			string_t *fn    = fn_call();
			string_t *param = any_map_get(fn_default_params, string_join(fn, i32_to_string(i)));
			if (param == null) {
				break;
			}
			if (i > 0) {
				res = string_join(res, ",");
			}
			res = string_join(res, param);
			param_pos_add();
		}
		array_pop(fn_call_stack);
		array_pop(param_pos_stack);
		token = string_join(res, token);
	}
	return token;
}

string_t *get_token_c() {
	string_t *t = get_token(0);
	t           = string_copy(enum_access(t));
	t           = string_copy(struct_access(t));
	t           = string_copy(array_access(t));
	return t;
}

string_t *fill_default_params(string_t *piece) {
	if (string_equals(get_token(1), ")")) {
		string_t *fn_name = "";
		i32       params  = 0;
		i32       i       = 0;
		i32       nested  = 1;
		while (true) {
			string_t *ti = get_token(i);
			if (string_equals(ti, ")")) {
				nested++;
			}
			else if (string_equals(ti, "(")) {
				if (string_equals(get_token(i - 1), ":")) {
					return piece;
				}
				nested--;
				if (nested == 0) {
					fn_name = string_copy(get_token(i - 1));
					if (!string_equals(get_token(i + 1), ")")) {
						params++;
					}
					break;
				}
			}
			else if (string_equals(ti, ",")) {
				params++;
			}
			i--;
		}
		while (true) {
			string_t *param = any_map_get(fn_default_params, string_join(fn_name, i32_to_string(params)));
			if (param == null) {
				break;
			}
			if (params > 0) {
				piece = string_join(piece, ",");
			}
			piece = string_join(piece, param);
			params++;
		}
	}
	return piece;
}

void skip_piece(i32 nested) {
	string_t *t1 = get_token(1);
	if (string_equals(t1, "(") || string_equals(t1, "[")) {
		nested++;
		pos++;
		skip_piece(nested);
		return;
	}
	if (string_equals(t1, ")") || string_equals(t1, "]")) {
		nested--;
		if (nested < 0) {
			return;
		}
		pos++;
		skip_piece(nested);
		return;
	}
	if (nested > 0) {
		pos++;
		skip_piece(nested);
		return;
	}
	if (starts_with(t1, ".")) {
		pos++;
		skip_piece(0);
		return;
	}
}

string_t *read_piece(i32 nested) {
	string_t *piece = get_token_c();
	piece           = string_copy(string_len(piece));
	piece           = string_copy(map_ops(piece));
	string_t *t1    = get_token(1);
	if (string_equals(t1, "(") || string_equals(t1, "[")) {
		nested++;
		pos++;
		return string_join(piece, read_piece(nested));
	}
	if (string_equals(t1, ")") || string_equals(t1, "]")) {
		nested--;
		if (nested < 0) {
			return piece;
		}
		piece = string_copy(fill_default_params(piece));
		pos++;
		return string_join(piece, read_piece(nested));
	}
	if (nested > 0) {
		pos++;
		return string_join(piece, read_piece(nested));
	}
	if (starts_with(t1, ".")) {
		pos++;
		return string_join(piece, read_piece(0));
	}
	return piece;
}

string_t *string_len(string_t *token) {
	if (!ends_with(token, "->length")) {
		return token;
	}
	string_t *base = strip(token, 8);
	string_t *type = value_type(base);
	if (string_equals(type, "string_t *") || string_equals(type, "struct string *")) {
		token = string_join(string_join("string_length(", base), ")");
	}
	return token;
}

string_t *value_type(string_t *value) {
	i32 arrow = string_index_of(value, "->");
	if (arrow > -1) {
		string_t  *base               = substring(value, 0, arrow);
		string_t  *type               = any_map_get(value_types, base);
		string_t  *member             = substring(value, arrow + 2, string_length(value));
		any_map_t *struct_value_types = any_map_get(struct_types, type);
		if (struct_value_types != null) {
			return any_map_get(struct_value_types, member);
		}
	}
	return any_map_get(value_types, value);
}

void set_fn_param_types() {
	pos++;
	while (true) {
		pos++;
		string_t *t = get_token(0);
		if (string_equals(t, ")")) {
			break;
		}
		if (string_equals(t, ",")) {
			continue;
		}
		pos++;
		string_t *type = read_type();
		any_map_set(value_types, t, type);
		if (string_equals(get_token(1), "=")) {
			pos += 2;
		}
	}
}

string_t *number_to_string(string_t *token) {
	string_t *type = value_type(token);
	if (string_equals(type, "i32")) {
		return string_join(string_join("i32_to_string(", token), ")");
	}
	if (string_equals(type, "f32")) {
		return string_join(string_join("f32_to_string(", token), ")");
	}
	return token;
}

bool token_is_string(string_t *token) {
	bool is_string = starts_with(token, "\"") || string_equals(value_type(token), "string_t *");
	if (!is_string) {
		i32 _pos = pos;
		skip_piece(0);
		string_t *t1 = get_token(1);
		if (string_equals(t1, "+") || string_equals(t1, "+=") || string_equals(t1, "==") || string_equals(t1, "!=")) {
			string_t *t2 = get_token(2);
			if (starts_with(t2, "\"") || string_equals(value_type(t2), "string_t *")) {
				is_string = true;
			}
		}
		pos = _pos;
	}
	if (is_string && string_equals(get_token(2), "null")) {
		is_string = false;
	}
	return is_string;
}

string_t *string_add(string_t *token) {
	token = string_join(string_join("string_join(", token), ",");
	pos++;
	string_t *token_b = read_piece(0);
	token_b           = string_copy(number_to_string(token_b));
	token             = string_join(token, token_b);
	token             = string_join(token, ")");
	return token;
}

string_t *string_ops(string_t *token) {
	if (!token_is_string(token)) {
		return token;
	}
	bool first = true;
	while (string_equals(get_token_after_piece(), "+")) {
		if (first) {
			first = false;
			token = string_copy(read_piece(0));
			token = string_copy(number_to_string(token));
		}
		pos++;
		token = string_copy(string_add(token));
	}
	string_t *p1 = get_token_after_piece();
	if (string_equals(p1, "+=")) {
		token = string_copy(read_piece(0));
		pos++;
		token = string_join(string_join(string_join(token, "=string_join("), token), ",");
		pos++;
		if (string_equals(get_token_after_piece(), "+")) {
			string_t *token_b = read_piece(0);
			token_b           = string_copy(number_to_string(token_b));
			while (string_equals(get_token_after_piece(), "+")) {
				pos++;
				token_b = string_copy(string_add(token_b));
			}
			token = string_join(token, token_b);
		}
		else {
			token = string_join(token, read_piece(0));
		}
		token = string_join(token, ")");
	}
	else if (string_equals(p1, "=")) {
		i32       _pos = pos;
		string_t *t1   = read_piece(0);
		pos++;
		pos++;
		string_t *t2 = read_piece(0);
		if (string_equals(get_token(1), ";") && !starts_with(t2, "\"")) {
			token = string_join(string_join(string_join(t1, "=string_copy("), t2), ")");
		}
		else {
			pos = _pos;
		}
	}
	else if (string_equals(p1, "==")) {
		token = string_copy(read_piece(0));
		pos++;
		token = string_join(string_join("string_equals(", token), ",");
		pos++;
		token = string_join(token, read_piece(0));
		token = string_join(token, ")");
	}
	else if (string_equals(p1, "!=")) {
		token = string_copy(read_piece(0));
		pos++;
		token = string_join(string_join("!string_equals(", token), ",");
		pos++;
		token = string_join(token, read_piece(0));
		token = string_join(token, ")");
	}
	return token;
}

string_t *array_ops(string_t *token) {
	if (string_equals(token, "array_push")) {
		string_t *value = get_token(2);
		if (string_equals(get_token(3), "[")) {
			string_t *t6 = get_token(6);
			value        = string_copy(substring(t6, 1, string_length(t6)));
		}
		if (string_last_index_of(value, ".") > -1) {
			value = string_copy(substring(value, string_last_index_of(value, ".") + 1, string_length(value)));
		}
		string_t *type = array_type(value);
		token          = string_join(type, "_array_push");
	}
	else if (string_equals(token, "array_remove")) {
		string_t *value = get_token(2);
		value           = string_copy(struct_access(value));
		string_t *type  = value_type(value);
		if (type != null && starts_with(type, "struct ")) {
			type = string_join(substring(type, 7, string_length(type) - 2), "_t *");
		}
		if (string_equals(type, "i32_array_t *")) {
			token = "i32_array_remove";
		}
		else if (string_equals(type, "string_t_array_t *")) {
			token = "char_ptr_array_remove";
		}
	}
	else if (string_equals(token, "array_index_of")) {
		string_t *value = get_token(2);
		value           = string_copy(struct_access(value));
		string_t *type  = value_type(value);
		if (type != null && starts_with(type, "struct ")) {
			type = string_join(substring(type, 7, string_length(type) - 2), "_t *");
		}
		if (string_equals(type, "i32_array_t *")) {
			token = "i32_array_index_of";
		}
		else if (string_equals(type, "string_t_array_t *")) {
			token = "char_ptr_array_index_of";
		}
		else {
			string_t *fn = any_map_get(fn_declarations, value);
			if (fn != null && starts_with(fn, "string_t_array_t *")) {
				token = "char_ptr_array_index_of";
			}
		}
	}
	return token;
}

string_t *map_ops(string_t *token) {
	if (string_equals(token, "map_create")) {
		string_t *t = value_type(get_token(-2));
		if (string_equals(t, "i32_map_t *")) {
			token = "i32_map_create";
		}
		else if (string_equals(t, "i32_imap_t *")) {
			token = "i32_imap_create";
		}
		else if (string_equals(t, "any_imap_t *")) {
			token = "any_imap_create";
		}
		else {
			token = "any_map_create";
		}
	}
	else if (string_equals(token, "map_set")) {
		string_t *t = value_type(get_token(2));
		if (string_equals(t, "i32_map_t *")) {
			token = "i32_map_set";
		}
		else if (string_equals(t, "i32_imap_t *")) {
			token = "i32_imap_set";
		}
		else if (string_equals(t, "any_imap_t *")) {
			token = "any_imap_set";
		}
		else {
			token = "any_map_set";
		}
	}
	else if (string_equals(token, "map_get")) {
		string_t *t = value_type(get_token(2));
		if (string_equals(t, "i32_map_t *")) {
			token = "i32_map_get";
		}
		else if (string_equals(t, "i32_imap_t *")) {
			token = "i32_imap_get";
		}
		else if (string_equals(t, "any_imap_t *")) {
			token = "any_imap_get";
		}
		else {
			token = "any_map_get";
		}
	}
	else if (string_equals(token, "map_delete")) {
		string_t *t = value_type(get_token(2));
		if (string_equals(t, "any_imap_t *")) {
			token = "imap_delete";
		}
	}
	return token;
}

string_t *_t_to_struct(string_t *type) {
	if (ends_with(type, "_t *") && !string_equals(type, "string_t *")) {
		string_t *type_short = strip(type, 4);
		if (ends_with(type_short, "_array")) {
			for (i32 i = 0; i < enums->length; ++i) {
				string_t *e = enums->buffer[i];
				if (starts_with(type_short, e)) {
					type_short = "i32_array";
					break;
				}
			}
		}
		type = string_join(string_join("struct ", type_short), " *");
	}
	return type;
}

void stream_write(string_t *token) {
	fwrite(token, 1, string_length(token), fhandle);
}

void string_write(string_t *token) {
	strings->buffer[strings->length - 1] = string_join(strings->buffer[strings->length - 1], token);
}

void null_write(string_t *token) {
	return;
}

void write_enums() {
	for (pos = 0; pos < tokens->length; ++pos) {
		string_t *token = get_token(0);
		if (string_equals(token, "enum")) {
			gc_unroot(out);
			out = string_equals(get_token(-1), "declare") ? &null_write : &stream_write;
			gc_root(out);
			pos++;
			string_t *enum_name = get_token(0);
			any_array_push(enums, enum_name);
			out("typedef enum{\n");
			pos++;
			while (true) {
				pos++;
				token = string_copy(get_token(0));
				if (string_equals(token, "}")) {
					out(string_join(string_join("}", enum_name), ";\n"));
					break;
				}
				string_t *enum_base = strip(enum_name, 2);
				enum_base           = string_copy(to_upper_case(enum_base));
				out(string_join(string_join(string_join("\t", enum_base), "_"), token));
				pos++;
				token = string_copy(get_token(0));
				if (string_equals(token, "=")) {
					pos++;
					token = string_copy(get_token(0));
					out(string_join("=", token));
					pos++;
				}
				out(",\n");
			}
			out("\n");
			continue;
		}
	}
	gc_unroot(out);
	out = &stream_write;
	gc_root(out);
}

void write_types() {
	for (pos = 0; pos < tokens->length; ++pos) {
		string_t *token = get_token(0);
		if (string_equals(token, "type")) {
			gc_unroot(out);
			out = string_equals(get_token(-1), "declare") ? &null_write : &stream_write;
			gc_root(out);
			pos++;
			string_t *struct_name      = get_token(0);
			string_t *stuct_name_short = strip(struct_name, 2);
			pos++;
			token = string_copy(get_token(0));
			if (!string_equals(token, "=")) {
				continue;
			}
			pos++;
			token = string_copy(get_token(0));
			if (!string_equals(token, "{")) {
				pos--;
				string_t *type = read_type();
				type           = string_copy(_t_to_struct(type));
				out(string_join(string_join(string_join(string_join("typedef ", type), " "), struct_name), ";\n\n"));
				skip_until(";");
				continue;
			}
			out(string_join(string_join("typedef struct ", stuct_name_short), "{\n"));
			any_map_t *struct_value_types = any_map_create();
			any_array_resize(struct_value_types->keys, 512 * 2);
			any_array_resize(struct_value_types->values, 512 * 2);
			any_map_set(struct_types, string_join(struct_name, " *"), struct_value_types);
			while (true) {
				pos++;
				string_t *name = get_token(0);
				if (string_equals(name, "}")) {
					out(string_join(string_join("}", struct_name), ";\n"));
					break;
				}
				name = string_copy(strip_optional(name));
				pos++;
				string_t *type = read_type();
				type           = string_copy(_t_to_struct(type));
				if (string_index_of(type, "(*NAME)") > -1) {
					string_t         *args_str = substring(type, string_index_of(type, ")(") + 2, string_last_index_of(type, ")"));
					string_t_array_t *args     = string_split(args_str, ",");
					type                       = string_copy(substring(type, 0, string_index_of(type, "(*NAME)")));
					type                       = string_copy(_t_to_struct(type));
					type                       = string_join(type, "(*NAME)(");
					for (i32 i = 0; i < args->length; ++i) {
						string_t *arg = args->buffer[i];
						arg           = string_copy(_t_to_struct(arg));
						if (i > 0) {
							type = string_join(type, ",");
						}
						type = string_join(type, arg);
					}
					type = string_join(type, ")");
				}
				skip_until(";");
				out(string_join(string_join("\t", join_type_name(type, name)), ";\n"));
				any_map_set(struct_value_types, name, type);
			}
			out("\n");
			continue;
		}
	}
	gc_unroot(out);
	out = &stream_write;
	gc_root(out);
}

void write_array_types() {
	any_map_t *array_structs = any_map_create();
	any_array_resize(array_structs->keys, 256 * 2);
	any_array_resize(array_structs->values, 256 * 2);
	for (pos = 0; pos < tokens->length; ++pos) {
		string_t *token = get_token(0);
		if (string_equals(get_token(1), "[")) {
			string_t *type = token;
			if (string_equals(type, "string")) {
				type = "string_t";
			}
			if (is_struct(type)) {
				if (any_map_get(array_structs, type) == null) {
					string_t *as = string_join(string_join(string_join(string_join(string_join(string_join("typedef struct ", type), "_array{"), type),
					                                                   "**buffer;int length;int capacity;}"),
					                                       type),
					                           "_array_t;");
					any_map_set(array_structs, type, as);
				}
			}
			pos += 2;
		}
	}
	string_t_array_t *keys = map_keys(array_structs);
	for (i32 i = 0; i < keys->length; ++i) {
		string_t *as = any_map_get(array_structs, keys->buffer[i]);
		out(as);
		out("\n");
	}
	out("\n");
}

void write_fn_declarations() {
	gc_unroot(fn_declarations);
	fn_declarations = any_map_create();
	gc_root(fn_declarations);
	any_array_resize(fn_declarations->keys, 4096 * 2);
	any_array_resize(fn_declarations->values, 4096 * 2);
	gc_unroot(fn_default_params);
	fn_default_params = any_map_create();
	gc_root(fn_default_params);
	any_array_resize(fn_default_params->keys, 1024 * 2);
	any_array_resize(fn_default_params->values, 1024 * 2);
	string_t *last_fn_name = "";
	for (pos = 0; pos < tokens->length; ++pos) {
		string_t *token = get_token(0);
		if (string_equals(token, "function")) {
			gc_unroot(out);
			out = string_equals(get_token(-1), "declare") ? &null_write : &stream_write;
			gc_root(out);
			pos++;
			string_t *fn_name = get_token(0);
			string_t *ret     = function_return_type();
			if (string_equals(fn_name, "(")) {
				pos--;
				fn_name = string_join(string_join(last_fn_name, "_"), i32_to_string(pos));
			}
			else {
				if (string_equals(fn_name, "main")) {
					fn_name = "_main";
				}
				last_fn_name = string_copy(fn_name);
			}
			i32       _param_pos = 0;
			string_t *params     = "(";
			pos++;
			while (true) {
				pos++;
				token = string_copy(get_token(0));
				if (string_equals(token, ")")) {
					break;
				}
				if (string_equals(token, ",")) {
					params = string_join(params, ",");
					_param_pos++;
					continue;
				}
				pos++;
				string_t *type = read_type();
				params         = string_join(params, join_type_name(type, token));
				if (string_equals(get_token(1), "=")) {
					string_t *param = get_token(2);
					param           = string_copy(enum_access(param));
					any_map_set(fn_default_params, string_join(fn_name, i32_to_string(_param_pos)), param);
					pos += 2;
				}
			}
			params            = string_join(params, ")");
			string_t *fn_decl = string_join(string_join(string_join(ret, " "), fn_name), params);
			out(string_join(fn_decl, ";\n"));
			any_map_set(fn_declarations, fn_name, fn_decl);
		}
	}
	out("\n");
	gc_unroot(out);
	out = &stream_write;
	gc_root(out);
}

void write_globals() {
	for (pos = 0; pos < tokens->length; ++pos) {
		string_t *token = get_token(0);
		if (string_equals(token, "let")) {
			gc_unroot(out);
			out = string_equals(get_token(-1), "declare") ? &null_write : &stream_write;
			gc_root(out);
			pos++;
			string_t *name = get_token(0);
			pos++;
			string_t *type = read_type();
			if (char_ptr_array_index_of(basic_types, type) == -1 && char_ptr_array_index_of(pass_by_value_types, type) == -1 &&
			    char_ptr_array_index_of(enums, type) == -1) {
				any_array_push(global_ptrs, name);
			}
			out(string_join(join_type_name(type, name), ";"));
			bool is_initialized = string_equals(get_token(1), "=");
			if (is_initialized) {
				string_t *init = name;
				tabs           = 1;
				while (true) {
					pos++;
					token = string_copy(get_token(0));
					token = string_copy(enum_access(token));
					token = string_copy(array_access(token));
					token = string_copy(struct_alloc(token, null));
					if (string_equals(token, ";")) {
						break;
					}
					token = string_copy(fill_fn_params(token));
					token = string_copy(array_create(token, name, null));
					if (string_equals(token, "map_create")) {
						string_t *t = value_type(name);
						if (string_equals(t, "i32_map_t *")) {
							token = "i32_map_create";
						}
						else if (string_equals(t, "i32_imap_t *")) {
							token = "i32_imap_create";
						}
						else if (string_equals(t, "any_imap_t *")) {
							token = "any_imap_create";
						}
						else {
							token = "any_map_create";
						}
					}
					init = string_join(init, token);
				}
				any_array_push(global_inits, init);
			}
			out("\n");
		}
		if (string_equals(token, "function") && !string_equals(get_token(-1), "declare")) {
			skip_until("{");
			skip_block();
		}
	}
	gc_unroot(out);
	out = &stream_write;
	gc_root(out);
}

void write_kickstart() {
	out("\nvoid _kickstart() {\n");
	for (i32 i = 0; i < global_inits->length; ++i) {
		string_t *val = global_inits->buffer[i];
		out("\t");
		string_t *name         = string_split(val, "=")->buffer[0];
		bool      global_alloc = char_ptr_array_index_of(global_ptrs, name) > -1 && !ends_with(val, "=null") && !ends_with(val, "\"");
		if (global_alloc) {
			out(string_join(string_join("gc_unroot(", name), ");"));
		}
		out(string_join(val, ";"));
		if (global_alloc) {
			out(string_join(string_join("gc_root(", name), ");"));
		}
		out("\n");
	}
	out("\t_main();\n");
	out("\t#ifndef NO_IRON_START\n");
	out("\tiron_start();\n");
	out("\t#endif\n");
	out("}\n\n");
}

void write_function() {
	string_t *fn_name = get_token(0);
	if (string_equals(fn_name, "main")) {
		fn_name = "_main";
	}
	string_t *fn_decl = any_map_get(fn_declarations, fn_name);
	out(string_join(fn_decl, "{\n"));
	set_fn_param_types();
	skip_until("{");
	tabs                   = 1;
	new_line               = true;
	string_t *mark_as_root = null;
	fnested->length        = 0;
	while (true) {
		pos++;
		string_t *token = get_token(0);
		if (string_equals(token, "function")) {
			string_t *anon_fn = string_join(string_join(fn_name, "_"), i32_to_string(pos));
			out(string_join("&", anon_fn));
			if (string_equals(get_token(-1), "=")) {
				out(";\n");
			}
			skip_until("{");
			gc_unroot(out);
			out = &string_write;
			gc_root(out);
			any_array_push(strings, "");
			string_t *fn_decl = any_map_get(fn_declarations, anon_fn);
			out(string_join(fn_decl, "{\n"));
			string_t *find   = substring(fn_decl, 0, string_index_of(fn_decl, "("));
			i32       fn_pos = parse_int(substring(find, string_last_index_of(find, "_") + 1, string_length(find)));
			i32       _pos   = pos;
			pos              = fn_pos;
			set_fn_param_types();
			pos = _pos;
			i32_array_push(fnested, 1);
			continue;
		}
		if (fnested->length > 0) {
			if (string_equals(token, "{")) {
				fnested->buffer[fnested->length - 1]++;
			}
			else if (string_equals(token, "}")) {
				fnested->buffer[fnested->length - 1]--;
				if (fnested->buffer[fnested->length - 1] == 0) {
					array_pop(fnested);
					out("}\n\n");
					if (strings->length > 1) {
						string_t *s                          = array_pop(strings);
						strings->buffer[strings->length - 1] = string_join(s, strings->buffer[strings->length - 1]);
					}
					if (fnested->length == 0) {
						gc_unroot(out);
						out = &stream_write;
						gc_root(out);
					}
					continue;
				}
			}
		}
		if (string_equals(token, "}") && tabs == 1) {
			out("}\n\n");
			break;
		}
		handle_tabs(token);
		if (string_equals(token, "for")) {
			is_for_loop = true;
		}
		else if (string_equals(token, "{")) {
			is_for_loop = false;
		}
		else if (string_equals(token, "let")) {
			pos++;
			string_t *name = get_token(0);
			pos++;
			string_t *type = read_type();
			out(join_type_name(type, name));
			pos++;
			token = string_copy(get_token(0));
		}
		token          = string_copy(enum_access(token));
		token          = string_copy(struct_access(token));
		token          = string_copy(array_access(token));
		bool is_assign = string_equals(get_token(1), "=") || string_equals(get_token(1), "+=");
		if (is_assign && !string_equals(token, ":")) {
			if (char_ptr_array_index_of(global_ptrs, token) > -1) {
				out(string_join(string_join("gc_unroot(", token), ");"));
				if (!string_equals(get_token(2), "null")) {
					mark_as_root = string_copy(token);
				}
			}
		}
		if (string_equals(token, ";") && mark_as_root != null) {
			out(string_join(string_join(";gc_root(", mark_as_root), ")"));
			mark_as_root = null;
		}
		token = string_copy(struct_alloc(token, null));
		token = string_copy(array_create(token, get_token(-2), null));
		token = string_copy(array_ops(token));
		token = string_copy(map_ops(token));
		token = string_copy(string_ops(token));
		token = string_copy(string_len(token));
		if (string_equals(token, "/")) {
			token = "/(float)";
		}
		token = string_copy(fill_fn_params(token));
		out(token);
		handle_spaces(token);
		handle_new_line(token);
	}
}

void write_functions() {
	for (pos = 0; pos < tokens->length; ++pos) {
		string_t *token = get_token(0);
		if (string_equals(token, "function")) {
			pos++;
			if (string_equals(get_token(-2), "declare")) {
				continue;
			}
			write_function();
		}
		while (strings->length > 0) {
			string_t *s = array_pop(strings);
			out(s);
		}
	}
}

void write_iron_c() {
	out("#include <iron.h>\n\n");
	out(header);
	write_enums();
	write_types();
	write_array_types();
	write_fn_declarations();
	write_globals();
	write_kickstart();
	write_functions();
}

void alang(string_t *_alang_source, string_t *_alang_output) {
	kickstart();
	gc_unroot(alang_source);
	alang_source = (_alang_source);
	gc_root(alang_source);
	alang_source_length = string_length(alang_source);
	gc_unroot(alang_output);
	alang_output = (_alang_output);
	gc_root(alang_output);
	any_array_resize(tokens, 512000);
	any_array_resize(strings, 32);
	any_array_resize(enums, 512);
	any_array_resize(fn_call_stack, 32);
	i32_array_resize(param_pos_stack, 32);
	any_array_resize(global_inits, 1024);
	any_array_resize(global_ptrs, 1024);
	any_array_resize(acontents, 2);
	acontents->buffer[0] = any_array_create_from_raw((any[]){}, 0);
	acontents->buffer[1] = any_array_create_from_raw((any[]){}, 0);
	any_array_resize(acontents->buffer[0], 4096);
	any_array_resize(acontents->buffer[1], 4096);
	i32_array_resize(fnested, 32);
	any_array_resize(value_types->keys, 4096 * 2);
	any_array_resize(value_types->values, 4096 * 2);
	any_array_resize(struct_types->keys, 512 * 2);
	any_array_resize(struct_types->values, 512 * 2);
	alang_parse();
	gc_unroot(fhandle);
	fhandle = fopen(alang_output, "wb");
	gc_root(fhandle);
	write_iron_c();
	fclose(fhandle);
}
