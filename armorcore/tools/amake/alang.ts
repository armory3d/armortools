
// ../../make --graphics opengl --compile --alangjs

///include <stdio.h>

let alang_output: string = null;
let alang_source: string = null;
let alang_source_length: i32;

// ██████╗      █████╗     ██████╗     ███████╗    ███████╗
// ██╔══██╗    ██╔══██╗    ██╔══██╗    ██╔════╝    ██╔════╝
// ██████╔╝    ███████║    ██████╔╝    ███████╗    █████╗
// ██╔═══╝     ██╔══██║    ██╔══██╗    ╚════██║    ██╔══╝
// ██║         ██║  ██║    ██║  ██║    ███████║    ███████╗
// ╚═╝         ╚═╝  ╚═╝    ╚═╝  ╚═╝    ╚══════╝    ╚══════╝

let specials: string[] = [":", ";", ",", "(", ")", "[", "]", "{", "}", "<", ">", "!"];
let is_comment: bool = false;
let is_string: bool = false;
let pos: i32 = 0;
let tokens: string[] = [];
let header: string = "";

function is_alpha_numeric(code: i32): bool {
	return (code > 47 && code < 58) || // 0-9
		   (code == 45) 			|| // -
		   (code == 43) 			|| // +
		   (code == 95) 			|| // _
		   (code == 46) 			|| // .
		   (code > 64 && code < 91) || // A-Z
		   (code > 96 && code < 123);  // a-z
}

function is_alpha(code: i32): bool {
	return (code > 96 && code < 123) || // a-z
		   (code > 64 && code < 91)  || // A-Z
		   (code == 95);				// _
}

function is_numeric(code: i32): bool {
	return (code > 47 && code < 58); // 0-9
}

function is_white_space(code: i32): bool {
	return code == 32 || // " "
		   code == 9  || // "\t"
		   code == 13 || // "\r"
		   code == 10;   // "\n"
}

function read_token(): string {
	// Skip white space
	while (is_white_space(char_code_at(alang_source, pos))) {
		pos++;
	}

	let token: string = "";
	let first: bool = true;
	let is_anum: bool = false;

	while (pos < alang_source_length) {
		let c: string = char_at(alang_source, pos);

		// Comment start
		if (token == "//") {
			is_comment = true;
		}

		if (is_comment) {
			token += c;
			pos++;

			// Comment end
			if (c == "\n") {
				is_comment = false;
				if (starts_with(token, "///include") || starts_with(token, "///define")) {
					header += "#" + substring(token, 3, token.length);
				}
				// Remove comments
				return read_token();
			}

			// Prevent parsing of comments
			continue;
		}

		// String start / end
		if (c == "\"") {
			// Escaped \"
			let last: string = token.length > 0 ? char_at(token, token.length - 1) : "";
			let last_last: string = token.length > 1 ? char_at(token, token.length - 2) : "";
			let is_escaped: bool = last == "\\" && last_last != "\\";

			if (!is_escaped) {
				is_string = !is_string;
			}

			// Token end - string end
			if (!is_string) {
				token += c;
				pos++;
				break;
			}
		}

		if (is_string) {
			if (c == "\r") {
				pos++;
				continue;
			}

			token += c;
			pos++;

			// Prevent parsing of strings
			continue;
		}

		// Token end
		if (is_white_space(char_code_at(alang_source, pos))) {
			break;
		}

		// Parsing an alphanumeric token
		if (first) {
			first = false;
			is_anum = is_alpha_numeric(char_code_at(c, 0));
		}

		// Token end
		let is_special: bool = array_index_of(specials, c) > -1;
		if (is_anum && is_special) {
			break;
		}

		// Add char to token and advance pos
		token += c;
		pos++;

		// Token end, got special
		if (is_special) {
			break;
		}
	}

	return token;
}

function parse() {
	pos = 0;

	while (true) {
		let token: string = read_token();

		if (token == "=" && tokens[tokens.length - 1] == "!") { // Merge "!" and "=" into "!=" token
			tokens[tokens.length - 1] = "!=";
			continue;
		}
		if (token == "") { // No more tokens
			break;
		}
		array_push(tokens, token);
	}
}

// ██╗         ██╗    ██████╗
// ██║         ██║    ██╔══██╗
// ██║         ██║    ██████╔╝
// ██║         ██║    ██╔══██╗
// ███████╗    ██║    ██████╔╝
// ╚══════╝    ╚═╝    ╚═════╝

let fhandle: any;
let strings: string[] = [];
let tabs: i32 = 0;
let new_line: bool = false;
let basic_types: string[] = ["i8", "u8", "i16", "u16", "i32", "u32", "f32", "i64", "u64", "f64", "bool"];
let pass_by_value_types: string[] = ["vec4_t", "vec3_t", "vec2_t", "quat_t", "mat4_t", "mat3_t"];
let enums: string[] = [];
let value_types: map_t<string, string> = map_create();
type string_map_t = map_t<string, string>;
let struct_types: map_t<string, string_map_t> = map_create();
let fn_declarations: map_t<string, string>;
let fn_default_params: map_t<string, string>;
let fn_call_stack: string[] = [];
let param_pos_stack: i32[] = [];
let is_for_loop: bool = false;
let global_inits: string[] = [];
let global_ptrs: string[] = [];
type string_array_t = string[];
let acontents: string_array_t[] = [];
let fnested: i32[] = [];
let add_space_keywords: string[] = ["return", "else"];
let array_contents_depth: i32 = 0;

function handle_tabs(token: string) {
	// Entering block, add tab
	if (token == "{") {
		tabs++;
	}
	else if (token == "}") {
		tabs--;
	}

	if (is_for_loop) {
		return;
	}

	// New line, add tabs
	if (new_line) {
		for (let i: i32 = 0; i < tabs; ++i) {
			out("\t");
		}
	}
}

function handle_new_line(token: string) {
	if (is_for_loop) {
		return;
	}

	// Insert new line
	new_line = token == ";" || token == "{" || token == "}";
	if (new_line) {
		out("\n");
	}
}

function handle_spaces(token: string) {
	// Add space to separate keywords
	if (array_index_of(add_space_keywords, token) > -1) {
		out(" ");
	}
}

function get_token(off: i32 = 0): string {
	if (pos + off >= 0 && pos + off < tokens.length) {
		return tokens[pos + off];
	}
	return "";
}

function get_token_after_piece(): string {
	let _pos: i32 = pos;
	skip_piece();
	let t: string = get_token(1);
	pos = _pos;
	return t;
}

function skip_until(s: string) {
	while (true) {
		let token: string = get_token();
		if (token == s) {
			break;
		}
		pos++;
	}
}

function skip_block() { // { ... }
	pos++; // {
	let nested: i32 = 1;
	while (true) {
		let token: string = get_token();
		if (token == "}") {
			nested--;
			if (nested == 0) {
				break;
			}
		}
		else if (token == "{") {
			nested++;
		}
		pos++;
	}
}

function function_return_type(): string {
	let _pos: i32 = pos;
	skip_until("{");
	let t: string = get_token(-1);
	if (t == "]") { // ): i32[]
		pos -= 2;
	}
	else if (t == ">") { // ): map_t<a, b>
		pos -= 5;
	}
	pos -= 2; // ): i32 {
	let result: string = get_token() == ":" ? read_type() : "void";
	pos = _pos;
	return result;
}

function join_type_name(type: string, name: string): string {
	map_set(value_types, name, type);
	if (string_index_of(type, "(*NAME)(") > 0) { // Function pointer
		return string_replace_all(type, "NAME", name);
	}
	return type + " " + name;
}

function read_type(): string { // Cursor at ":"
	pos++;
	let type: string = get_token();

	if (type == "(" && get_token(1) == "(") { // (()=>void)[]
		pos++;
	}

	if (get_token(1) == ")" && get_token(2) == "[") { // (()=>void)[]
		pos++;
	}

	if (type == "(") { // ()=>void
		let params: string = "(";

		if (get_token(1) != ")") { // Has params
			while (true) {
				skip_until(":");
				params += read_type();

				pos++;
				if (get_token() == ")") { // End of params
					break;
				}

				params += ",";
			}
		}
		else {
			params += "void";
		}

		params += ")";
		skip_until("=>");
		let ret: string = read_type();
		type = ret + "(*NAME)" + params;
	}

	if (type == "color_t") {
		type = "i32";
	}
	else if (type == "string") {
		type = "string_t";
	}

	if (get_token(1) == "[") { // Array
		if (is_struct(type)) {
			type = type + "_array_t";
		}
		else if (type == "bool") {
			type = "u8_array_t";
		}
		else {
			type = type + "_array_t";
		}
		pos += 2; // Skip "[]"
	}

	if (is_struct(type) && array_index_of(pass_by_value_types, type) == -1) {
		type += " *";
	}

	if (type == "map_t *" && get_token(1) == "<") { // Map
		let mv: string = get_token(4);
		if (mv == "f32") {
			type = "f32_" + type;
		}
		else if (array_index_of(basic_types, mv) > -1) {
			let mk: string = get_token(2);
			if (mk == "i32") {
				type = "i32_i" + type;
			}
			else {
				type = "i32_" + type;
			}
		}
		else {
			let mk: string = get_token(2);
			if (mk == "i32") {
				type = "any_i" + type;
			}
			else {
				type = "any_" + type;
			}
		}
		skip_until(">"); // Skip <a, b>
	}

	return type;
}

function enum_access(s: string): string {
	// Turn enum_t.VALUE into enum_t_VALUE
	if (string_index_of(s, "_t.") > -1) {
		for (let i: i32 = 0; i < enums.length; ++i) {
			let e: string = enums[i];
			if (string_index_of(s, e) > -1) {
				s = string_replace_all(s, ".", "_");
				break;
			}
		}
	}
	return s;
}

function struct_access(s: string): string {
	// Turn a.b into a->b
	let dot: i32 = string_index_of(s, ".");
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

	// Accessing pass_by_value_types struct like vec4.x
	let has_minus: bool = starts_with(s, "-");
	let base: string = substring(s, has_minus ? 1 : 0, dot);
	let type: string = map_get(value_types, base);
	if (type != null) {
		let dot_last: i32 = string_last_index_of(s, ".");
		if (dot != dot_last) { // a.vec4.x
			let parts: string[] = string_split(s, ".");
			for (let i: i32 = 1; i < parts.length - 1; ++i) {
				let struct_value_types: map_t<string, string> = map_get(struct_types, type);
				if (struct_value_types != null) {
					type = map_get(struct_value_types, parts[i]);

					// struct my_struct -> my_struct_t
					// my_struct * -> my_struct
					if (starts_with(type, "struct ") && ends_with(type, " *")) {
						type = substring(type, 0, type.length - 2);
						type = substring(type, 7, type.length) + "_t *";
					}
				}
			}
			if (!ends_with(type, " *")) { // vec4_t
				let first: string = string_replace_all(substring(s, 0, dot_last), ".", "->");
				let last: string = substring(s, dot_last, s.length);
				return first + last; // a->vec4.x
			}
		}
		else if (!ends_with(type, " *")) { // vec4.x
			return s;
		}
	}

	return string_replace_all(s, ".", "->");
}

function struct_alloc(token: string, type: string = null): string {
	// "= { a: b, ... }" -> GC_ALLOC_INIT
	if ((get_token(-1) == "=" && token == "{" && get_token(-3) != "type") || type != null) {
		if (type == null) {
			// let a: b = {, a = {, a.b = {
			let i: i32 = get_token(-3) == ":" ? -4 : -2;
			let t: string = struct_access(get_token(i));
			type = value_type(t);
			if (ends_with(type, " *")) { // my_struct * -> my_struct
				type = substring(type, 0, type.length - 2);
			}
			if (starts_with(type, "struct ")) { // struct my_struct -> my_struct_t
				type = substring(type, 7, type.length) + "_t";
			}
		}

		token = "GC_ALLOC_INIT(" + type + ", {";
		pos++; // {
		let ternary: i32 = 0;
		while (true) {
			let t: string = get_token();
			if (t == "}") {
				break;
			}

			let tc: string = get_token_c();
			tc = string_ops(tc);
			if (get_token(1) == ":" && !ternary) {
				tc = "." + tc; // a: b -> .a = b
			}
			if (t == ":") {
				if (ternary > 0) {
					ternary--;
				}
				else {
					tc = "=";
				}
			}
			else if (t == "?") {
				ternary++;
			}
			else if (t == "[" && get_token(-1) == ":") {
				let member: string = get_token(-2);
				let types: map_t<string, string> = map_get(struct_types, type + " *");
				let content_type: string = map_get(types, member);
				content_type = substring(content_type, 7, content_type.length - 8); // struct my_struct_array_t * -> my_struct
				tc = array_create("[", "", content_type);
			}
			token += tc;
			pos++;
		}

		// "= {}" -> "= {0}", otherwise msvc refuses to init members to zero
		if (get_token(-1) == "{") {
			token += "0";
		}

		token += get_token(); // "}"
		token += ")";
		tabs--;
	}
	return token;
}

function array_type(name: string): string {
	let type: string = value_type(name);
	if (type == null) {
		return "any";
	}
	if (starts_with(type, "struct ")) { // struct u8_array * -> u8_array_t *
		type = substring(type, 7, type.length - 2) + "_t *";
	}
	type = strip(type, 10); // Strip _array_t *
	if (array_index_of(basic_types, type) > -1) { // u8
		return type;
	}
	return "any";
}

function array_contents(type: string): string[] {

	// [1, 2, ..] or [{a:b}, {a:b}, ..] or [a(), b(), ..]
	let contents: string[] = acontents[array_contents_depth];
	array_contents_depth++;
	contents.length = 0;

	let content: string = "";
	while (true) {
		pos++;
		let token: string = get_token();
		if (token == "]") {
			if (content != "") {
				array_push(contents, content);
			}
			break;
		}
		if (token == ",") {
			array_push(contents, content);
			content = "";
			continue;
		}
		if (token == "{") {
			token = struct_alloc(token, type);
			tabs++; // Undo tabs-- in struct_alloc
		}
		if (get_token(1) == "(") {
			// Function call
			while (true) {
				pos++;
				token += get_token();
				token = fill_default_params(token);
				if (ends_with(token, ")")) {
					break;
				}
			}
		}
		// a / b - > a / (float)b
		if (token == "/") {
			token = "/(float)";
		}
		token = struct_access(token);
		token = string_ops(token);
		content += token;
	}

	array_contents_depth--;

	return contents;
}

function member_name(name: string): string {
	let i: i32 = string_last_index_of(name, ".");
	if (i > -1) {
		name = substring(name, i + 1, name.length);
	}
	return name;
}

function array_create(token: string, name: string, content_type: string = null): string {
	if ((get_token(-1) == "=" && token == "[") || content_type != null) {
		if (name == "]") {
			name = get_token(-6); // ar: i32[] = [
		}
		let member: string = member_name(name);
		let type: string = array_type(member);

		// = [], = [1, 2, ..] -> any/i32/.._array_create_from_raw
		if (content_type == null) {
			name = struct_access(name);
			content_type = value_type(name);
			if (content_type != null) {
				content_type = substring(content_type, 0, content_type.length - 10);
			}
		}
		let contents: string[] = array_contents(content_type);
		token = type + "_array_create_from_raw((" + type + "[]){";
		for (let i: i32 = 0; i < contents.length; ++i) {
			let e: string = contents[i];
			token += e + ",";
		}
		let len: i32 = contents.length;
		token += "}," + len + ")";
	}
	return token;
}

function array_access(token: string): string {
	// array[0] -> array->buffer[0]
	if (token == "[" && get_token(-1) != "=") {
		return "->buffer[";
	}
	return token;
}

function is_struct(type: string): bool {
	// Ends with _t and is not an enum
	return ends_with(type, "_t") && array_index_of(enums, type) == -1;
}

function strip(name: string, len: i32): string {
	return substring(name, 0, name.length - len);
}

function strip_optional(name: string): string {
	if (ends_with(name, "?")) {
		return strip(name, 1); // :val? -> :val
	}
	return name;
}

function param_pos(): i32 {
	return param_pos_stack[param_pos_stack.length - 1];
}

function param_pos_add() {
	param_pos_stack[param_pos_stack.length - 1]++;
}

function fn_call(): string {
	return fn_call_stack[fn_call_stack.length - 1];
}

function fill_fn_params(token: string): string {

	if (token == "(") {
		// Function is being called to init this variable
		array_push(fn_call_stack, get_token(-1));
		array_push(param_pos_stack, 0);
	}

	else if (token == ",") {
		// Param has beed passed manually
		param_pos_add();
	}

	else if (token == ")") {
		// Fill in default parameters if needed
		if (get_token(-1) != "(") {
			// Param has beed passed manually
			param_pos_add();
		}

		// If default param exists, fill it
		let res: string = "";
		while (true) {
			let i: i32 = param_pos();
			let fn: string = fn_call();
			let param: string = map_get(fn_default_params, fn + i);
			if (param == null) {
				break;
			}
			if (i > 0) {
				res += ",";
			}
			res += param;
			param_pos_add();
		}

		array_pop(fn_call_stack);
		array_pop(param_pos_stack);

		token = res + token;
	}

	return token;
}

function get_token_c(): string {
	let t: string = get_token();
	t = enum_access(t);
	t = struct_access(t);
	t = array_access(t);
	return t;
}

function fill_default_params(piece: string): string {
	// fn(a, 1, ..)
	if (get_token(1) == ")") {
		let fn_name: string = "";
		let params: i32 = 0;
		let i: i32 = 0;
		let nested: i32 = 1;
		while (true) {
			let ti: string = get_token(i);
			if (ti == ")") {
				nested++;
			}
			else if (ti == "(") {
				if (get_token(i - 1) == ":") { // : ()=>void
					return piece;
				}
				nested--;
				if (nested == 0) {
					fn_name = get_token(i - 1);
					if (get_token(i + 1) != ")") {
						params++;
					}
					break;
				}
			}
			else if (ti == ",") {
				params++;
			}
			i--;
		}
		while (true) {
			let param: string = map_get(fn_default_params, fn_name + params);
			if (param == null) {
				break;
			}
			if (params > 0) {
				piece += ",";
			}
			piece += param;
			params++;
		}
	}
	return piece;
}

function skip_piece(nested: i32 = 0) {
	let t1: string = get_token(1);
	if (t1 == "(" || t1 == "[") {
		nested++;
		pos++;
		skip_piece(nested);
		return;
	}

	if (t1 == ")" || t1 == "]") {
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
		skip_piece();
		return;
	}
}

function read_piece(nested: i32 = 0): string {
	// call(a, b, c), a[b + c]
	let piece: string = get_token_c();
	piece = string_len(piece);
	piece = map_ops(piece);

	let t1: string = get_token(1);
	if (t1 == "(" || t1 == "[") {
		nested++;
		pos++;
		return piece + read_piece(nested);
	}

	if (t1 == ")" || t1 == "]") {
		nested--;
		if (nested < 0) {
			return piece;
		}
		piece = fill_default_params(piece);
		pos++;
		return piece + read_piece(nested);
	}

	if (nested > 0) {
		pos++;
		return piece + read_piece(nested);
	}

	if (starts_with(t1, ".")) {
		pos++;
		return piece + read_piece();
	}

	return piece;
}

function string_len(token: string): string {
	// "str->length" -> "string_length(str)"
	if (!ends_with(token, "->length")) {
		return token;
	}
	let base: string = strip(token, 8); // ->length
	let type: string = value_type(base);
	if (type == "string_t *" || type == "struct string *") {
		token = "string_length(" + base + ")";
	}
	return token;
}

function value_type(value: string): string {
	let arrow: i32 = string_index_of(value, "->");
	if (arrow > -1) {
		let base: string = substring(value, 0, arrow);
		let type: string = map_get(value_types, base);
		let member: string = substring(value, arrow + 2, value.length);
		let struct_value_types: map_t<string, string> = map_get(struct_types, type);
		if (struct_value_types != null) {
			return map_get(struct_value_types, member);
		}
	}
	return map_get(value_types, value);
}

function set_fn_param_types() {
	pos++; // (
	while (true) {
		pos++;
		let t: string = get_token(); // Param name, ) or ,
		if (t == ")") { // Params end
			break;
		}
		if (t == ",") { // Next param
			continue;
		}

		pos++; // :
		let type: string = read_type();
		map_set(value_types, t, type);

		if (get_token(1) == "=") { // Skip default value
			pos += 2;
		}
	}
}

function number_to_string(token: string): string {
	let type: string = value_type(token);
	if (type == "i32") {
		return "i32_to_string(" + token + ")";
	}
	if (type == "f32") {
		return "f32_to_string(" + token + ")";
	}
	return token;
}

function token_is_string(token: string): bool {
	let is_string: bool = starts_with(token, "\"") || value_type(token) == "string_t *";

	if (!is_string) {
		let _pos: i32 = pos;
		skip_piece();
		let t1: string = get_token(1);
		if (t1 == "+" || t1 == "+=" || t1 == "==" || t1 == "!=") {
			let t2: string = get_token(2);
			if (starts_with(t2, "\"") || value_type(t2) == "string_t *") {
				is_string = true;
			}
		}
		pos = _pos;
	}

	if (is_string && get_token(2) == "null") {
		is_string = false;
	}

	return is_string;
}

function string_add(token: string): string {
	token = "string_join(" + token + ",";
	pos++;

	let token_b: string = read_piece();
	token_b = number_to_string(token_b);
	token += token_b;
	token += ")";
	return token;
}

function string_ops(token: string): string {
	// "str" + str + 1 + ...

	if (!token_is_string(token)) {
		return token;
	}

	let first: bool = true;
	while (get_token_after_piece() == "+") {
		if (first) {
			first = false;
			token = read_piece();
			token = number_to_string(token);
		}
		pos++;
		token = string_add(token);
	}

	// "str" += str + str2
	let p1: string = get_token_after_piece();
	if (p1 == "+=") {
		token = read_piece();
		pos++;
		token = token + "=string_join(" + token + ",";
		pos++;
		if (get_token_after_piece() == "+") {
			let token_b: string = read_piece();
			token_b = number_to_string(token_b);
			while (get_token_after_piece() == "+") {
				pos++;
				token_b = string_add(token_b);
			}
			token += token_b;
		}
		else {
			token += read_piece();
		}
		token += ")";
	}

	// str = str
	else if (p1 == "=") {
		let _pos: i32 = pos;
		let t1: string = read_piece();
		pos++;
		pos++; // =
		let t2: string = read_piece();
		// TODO: let s: string = str;
		if (get_token(1) == ";" && !starts_with(t2, "\"")) { // str = str;
			token = t1 + "=string_copy(" + t2 + ")"; // Copy string
		}
		else {
			pos = _pos;
		}
	}

	// "str" == str
	else if (p1 == "==") {
		token = read_piece();
		pos++;

		token = "string_equals(" + token + ",";
		pos++;
		token += read_piece();
		token += ")";
	}

	// "str" != str
	else if (p1 == "!=") {
		token = read_piece();
		pos++;

		token = "!string_equals(" + token + ",";
		pos++;
		token += read_piece();
		token += ")";
	}

	return token;
}

function array_ops(token: string): string {
	// array_push -> i32_array_push/any_array_push
	if (token == "array_push") {
		let value: string = get_token(2);

		if (get_token(3) == "[") { // raws[i].value
			let t6: string = get_token(6);
			value = substring(t6, 1, t6.length); // .value
		}

		if (string_last_index_of(value, ".") > -1) {
			value = substring(value, string_last_index_of(value, ".") + 1, value.length);
		}
		let type: string = array_type(value);
		token = type + "_array_push";
	}

	// array_remove -> i32_array_remove / char_ptr_array_remove
	else if (token == "array_remove") {
		let value: string = get_token(2);
		value = struct_access(value);
		let type: string = value_type(value);
		if (type != null && starts_with(type, "struct ")) { // struct u8_array * -> u8_array_t *
			type = substring(type, 7, type.length - 2) + "_t *";
		}

		if (type == "i32_array_t *") {
			token = "i32_array_remove";
		}
		else if (type == "string_t_array_t *") {
			token = "char_ptr_array_remove";
		}
	}

	// array_index_of -> i32_array_index_of / char_ptr_array_index_of
	else if (token == "array_index_of") {
		let value: string = get_token(2);
		value = struct_access(value);
		let type: string = value_type(value);
		if (type != null && starts_with(type, "struct ")) { // struct u8_array * -> u8_array_t *
			type = substring(type, 7, type.length - 2) + "_t *";
		}

		if (type == "i32_array_t *") {
			token = "i32_array_index_of";
		}
		else if (type == "string_t_array_t *") {
			token = "char_ptr_array_index_of";
		}
		else {
			let fn: string = map_get(fn_declarations, value);
			if (fn != null && starts_with(fn, "string_t_array_t *")) {
				token = "char_ptr_array_index_of";
			}
		}
	}

	return token;
}

function map_ops(token: string): string {
	if (token == "map_create") {
		let t: string = value_type(get_token(-2));
		if (t == "i32_map_t *") {
			token = "i32_map_create";
		}
		else if (t == "i32_imap_t *") {
			token = "i32_imap_create";
		}
		else if (t == "any_imap_t *") {
			token = "any_imap_create";
		}
		else {
			token = "any_map_create";
		}
	}

	else if (token == "map_set") {
		let t: string = value_type(get_token(2));
		if (t == "i32_map_t *") {
			token = "i32_map_set";
		}
		else if (t == "i32_imap_t *") {
			token = "i32_imap_set";
		}
		else if (t == "any_imap_t *") {
			token = "any_imap_set";
		}
		else {
			token = "any_map_set";
		}
	}

	else if (token == "map_get") {
		let t: string = value_type(get_token(2));
		if (t == "i32_map_t *") {
			token = "i32_map_get";
		}
		else if (t == "i32_imap_t *") {
			token = "i32_imap_get";
		}
		else if (t == "any_imap_t *") {
			token = "any_imap_get";
		}
		else {
			token = "any_map_get";
		}
	}

	else if (token == "map_delete") {
		let t: string = value_type(get_token(2));
		if (t == "any_imap_t *") {
			token = "imap_delete";
		}
	}

	return token;
}

function _t_to_struct(type: string): string {
	// type_t * -> struct type *
	if (ends_with(type, "_t *") && type != "string_t *") {
		let type_short: string = strip(type, 4); // _t *
		if (ends_with(type_short, "_array")) { // tex_format_t_array -> i32_array
			for (let i: i32 = 0; i < enums.length; ++i) {
				let e: string = enums[i];
				if (starts_with(type_short, e)) {
					type_short = "i32_array";
					break;
				}
			}
		}
		type = "struct " + type_short + " *";
	}
	return type;
}

// ██╗    ██╗    ██████╗     ██╗    ████████╗    ███████╗         ██████╗
// ██║    ██║    ██╔══██╗    ██║    ╚══██╔══╝    ██╔════╝        ██╔════╝
// ██║ █╗ ██║    ██████╔╝    ██║       ██║       █████╗          ██║
// ██║███╗██║    ██╔══██╗    ██║       ██║       ██╔══╝          ██║
// ╚███╔███╔╝    ██║  ██║    ██║       ██║       ███████╗        ╚██████╗
//  ╚══╝╚══╝     ╚═╝  ╚═╝    ╚═╝       ╚═╝       ╚══════╝         ╚═════╝

function stream_write(token: string) {
	fwrite(token, 1, token.length, fhandle);
}

function string_write(token: string) {
	strings[strings.length - 1] += token;
}

function null_write(token: string) {
	return;
}

let out: (token: string)=>void = stream_write;

function write_enums() {
	for (pos = 0; pos < tokens.length; ++pos) {
		let token: string = get_token();

		// Turn "enum name {}" into "typedef enum {} name;"
		if (token == "enum") {

			out = get_token(-1) == "declare" ? &null_write : &stream_write;

			pos++;
			let enum_name: string = get_token();
			array_push(enums, enum_name);

			out("typedef enum{\n");
			pos++; // {

			while (true) {
				// Enum contents
				pos++;
				token = get_token(); // Item name

				if (token == "}") { // Enum end
					out("}" + enum_name + ";\n");
					break;
				}

				out("\t" + enum_name + "_" + token);

				pos++; // = or ,
				token = get_token();

				if (token == "=") { // Enum value
					pos++; // n
					token = get_token();
					out("=" + token);
					pos++; // ,
				}

				out(",\n");
			}

			out("\n");
			continue;
		}
	}

	out = &stream_write;
}

function write_types() {
	for (pos = 0; pos < tokens.length; ++pos) {
		let token: string = get_token();

		// Turn "type x = {};" into "typedef struct {} x;"
		// Turn "type x = y;" into "typedef x y;"
		if (token == "type") {

			out = get_token(-1) == "declare" ? &null_write : &stream_write;

			pos++;
			let struct_name: string = get_token();
			let stuct_name_short: string = strip(struct_name, 2); // _t

			pos++;
			token = get_token(); // =
			if (token != "=") {
				continue;
			}

			pos++;
			token = get_token();

			// "type x = y;"
			if (token != "{") {
				pos--;
				let type: string = read_type();
				type = _t_to_struct(type);

				out("typedef " + type + " " + struct_name + ";\n\n");
				skip_until(";");
				continue;
			}

			// "type x = {};"
			out("typedef struct " + stuct_name_short + "{\n");

			let struct_value_types: map_t<string, string> = map_create();
			any_array_resize(struct_value_types.keys, 512 * 2);
			any_array_resize(struct_value_types.values, 512 * 2);

			map_set(struct_types, struct_name + " *", struct_value_types);

			while (true) {
				// Struct contents
				pos++;
				let name: string = get_token();

				if (name == "}") { // Struct end
					out("}" + struct_name + ";\n");
					break;
				}

				// val?: i32 -> val: i32
				name = strip_optional(name);

				pos++;
				// :
				let type: string = read_type();

				// type_t * -> struct type *
				type = _t_to_struct(type);

				// my_struct_t*(*NAME)(my_struct_t *) -> struct my_struct*(*NAME)(struct my_struct*)
				if (string_index_of(type, "(*NAME)") > -1) {
					let args_str: string = substring(type, string_index_of(type, ")(") + 2, string_last_index_of(type, ")"));
					let args: string[] = string_split(args_str, ",");

					type = substring(type, 0, string_index_of(type, "(*NAME)"));
					type = _t_to_struct(type);
					type += "(*NAME)(";

					for (let i: i32 = 0; i < args.length; ++i) {
						let arg: string = args[i];
						arg = _t_to_struct(arg);
						if (i > 0) {
							type += ",";
						}
						type += arg;
					}
					type += ")";
				}

				skip_until(";");

				out("\t" + join_type_name(type, name) + ";\n");
				map_set(struct_value_types, name, type);
			}

			out("\n");
			continue;
		}
	}

	out = &stream_write;
}

function write_array_types() {
	// Array structs (any_array_t -> scene_t_array_t)
	let array_structs: map_t<string, string> = map_create();
	any_array_resize(array_structs.keys, 256 * 2);
	any_array_resize(array_structs.values, 256 * 2);

	for (pos = 0; pos < tokens.length; ++pos) {
		let token: string = get_token();
		if (get_token(1) == "[") {
			let type: string = token;
			if (type == "string") {
				type = "string_t";
			}
			if (is_struct(type)) {
				if (map_get(array_structs, type) == null) {
					let as: string = "typedef struct " + type + "_array{" +
						type + "**buffer;int length;int capacity;}" +
						type + "_array_t;";
					map_set(array_structs, type, as);
				}
			}
			pos += 2; // Skip "[]"
		}
	}

	let keys: string[] = map_keys(array_structs);
	for (let i: i32 = 0; i < keys.length; ++i) {
		let as: string = map_get(array_structs, keys[i]);
		out(as);
		out("\n");
	}

	out("\n");
}

function write_fn_declarations() {
	fn_declarations = map_create();
	any_array_resize(fn_declarations.keys, 4096 * 2);
	any_array_resize(fn_declarations.values, 4096 * 2);

	fn_default_params = map_create();
	any_array_resize(fn_default_params.keys, 1024 * 2);
	any_array_resize(fn_default_params.values, 1024 * 2);

	let last_fn_name: string = "";
	for (pos = 0; pos < tokens.length; ++pos) {
		let token: string = get_token();

		if (token == "function") {

			out = get_token(-1) == "declare" ? &null_write : &stream_write;

			// Return type + name
			pos++;
			let fn_name: string = get_token();
			let ret: string = function_return_type();

			if (fn_name == "(") { // Anonymous function
				pos--;
				fn_name = last_fn_name + "_" + pos;
			}
			else {
				if (fn_name == "main") {
					fn_name = "_main";
				}
				last_fn_name = fn_name;
			}

			// Params
			let _param_pos: i32 = 0;
			let params: string = "(";
			pos++; // (
			while (true) {
				pos++;
				token = get_token(); // Param name, ) or ,

				if (token == ")") { // Params end
					break;
				}

				if (token == ",") { // Next param
					params += ",";
					_param_pos++;
					continue;
				}

				pos++; // :
				let type: string = read_type();
				params += join_type_name(type, token);

				// Store param default
				if (get_token(1) == "=") {
					let param: string = get_token(2);
					param = enum_access(param);
					map_set(fn_default_params, fn_name + _param_pos, param);
					pos += 2;
				}
			}
			params += ")";

			let fn_decl: string = ret + " " + fn_name + params;
			out(fn_decl + ";\n");

			map_set(fn_declarations, fn_name, fn_decl);
		}
	}
	out("\n");

	out = &stream_write;
}

function write_globals() {
	for (pos = 0; pos < tokens.length; ++pos) {
		let token: string = get_token();

		if (token == "let") {

			out = get_token(-1) == "declare" ? &null_write : &stream_write;

			pos++;
			let name: string = get_token();

			pos++; // :
			let type: string = read_type();

			if (array_index_of(basic_types, type) == -1 &&
				array_index_of(pass_by_value_types, type) == -1 &&
				array_index_of(enums, type) == -1) {
				array_push(global_ptrs, name);
			}

			out(join_type_name(type, name) + ";");

			// Init this var in _kickstart()
			let is_initialized: bool = get_token(1) == "=";
			if (is_initialized) {
				let init: string = name;
				tabs = 1;
				while (true) {
					pos++;
					token = get_token();

					token = enum_access(token);
					token = array_access(token);
					token = struct_alloc(token);

					if (token == ";") {
						break;
					}

					token = fill_fn_params(token);

					// [] -> _array_create
					token = array_create(token, name);

					if (token == "map_create") {
						let t: string = value_type(name);
						if (t == "i32_map_t *") {
							token = "i32_map_create";
						}
						else if (t == "i32_imap_t *") {
							token = "i32_imap_create";
						}
						else if (t == "any_imap_t *") {
							token = "any_imap_create";
						}
						else {
							token = "any_map_create";
						}
					}

					init += token;
				}
				array_push(global_inits, init);
			}

			out("\n");
		}

		// Skip function blocks
		if (token == "function" && get_token(-1) != "declare") {
			skip_until("{");
			skip_block();
		}
	}

	out = &stream_write;
}

function write_kickstart() {
	// Start function
	out("\nvoid _kickstart() {\n");
	// Init globals
	for (let i: i32 = 0; i < global_inits.length; ++i) {
		let val: string = global_inits[i];
		out("\t");
		let name: string = string_split(val, "=")[0];
		let global_alloc: bool = array_index_of(global_ptrs, name) > -1 && !ends_with(val, "=null") && !ends_with(val, "\"");
		if (global_alloc) {
			out("gc_unroot(" + name + ");");
		}
		out(val + ";");
		if (global_alloc) {
			out("gc_root(" + name + ");");
		}
		out("\n");
	}
	out("\t_main();\n");
	out("\t#ifndef NO_KINC_START\n");
	out("\tkinc_start();\n");
	out("\t#endif\n");
	out("}\n\n");
}

function write_function() {
	// Function declaration
	let fn_name: string = get_token();

	if (fn_name == "main") {
		fn_name = "_main";
	}

	let fn_decl: string = map_get(fn_declarations, fn_name);
	out(fn_decl + "{\n");

	// Function body
	// Re-set function param types into value_types map
	set_fn_param_types();
	skip_until("{");

	tabs = 1;
	new_line = true;
	let mark_as_root: string = null;
	fnested.length = 0;

	while (true) {
		pos++;
		let token: string = get_token();

		if (token == "function") { // Begin nested function
			let anon_fn: string = fn_name + "_" + pos;
			out("&" + anon_fn);
			if (get_token(-1) == "=") {
				out(";\n");
			}

			skip_until("{");
			out = &string_write;
			array_push(strings, "");
			let fn_decl: string = map_get(fn_declarations, anon_fn);
			out(fn_decl + "{\n");

			let find: string = substring(fn_decl, 0, string_index_of(fn_decl, "("));
			let fn_pos: i32 = parse_int(substring(find, string_last_index_of(find, "_") + 1, find.length));
			let _pos: i32 = pos;
			pos = fn_pos;
			set_fn_param_types();
			pos = _pos;

			array_push(fnested, 1);
			continue;
		}

		if (fnested.length > 0) {
			if (token == "{") {
				fnested[fnested.length - 1]++;
			}
			else if (token == "}") { // End fnested function
				fnested[fnested.length - 1]--;
				if (fnested[fnested.length - 1] == 0) {
					array_pop(fnested);
					out("}\n\n");
					if (strings.length > 1) {
						let s: string = array_pop(strings);
						strings[strings.length - 1] = s + strings[strings.length - 1];
					}

					if (fnested.length == 0) {
						out = &stream_write;
					}
					continue;
				}
			}
		}

		// Function end
		if (token == "}" && tabs == 1) {
			out("}\n\n");
			break;
		}

		handle_tabs(token);

		if (token == "for") { // Skip new lines in for (;;) loop
			is_for_loop = true;
		}
		else if (token == "{") {
			is_for_loop = false;
		}
		// Write type and name
		else if (token == "let") {
			pos++;
			let name: string = get_token();
			pos++; // :
			let type: string = read_type();
			out(join_type_name(type, name));
			// = or ;
			pos++;
			token = get_token();
		}

		// Turn val.a into val_a or val->a
		token = enum_access(token);

		token = struct_access(token);

		// array[0] -> array->buffer[0]
		token = array_access(token);

		// Use static alloc for global pointers
		let is_assign: bool = get_token(1) == "=" || get_token(1) == "+="; // += for string_join
		if (is_assign && token != ":") {
			if (array_index_of(global_ptrs, token) > -1) {
				out("gc_unroot(" + token + ");");
				if (get_token(2) != "null") {
					mark_as_root = token;
				}
			}
		}
		if (token == ";" && mark_as_root != null) {
			out(";gc_root(" + mark_as_root + ")");
			mark_as_root = null;
		}

		// "= { ... }" -> GC_ALLOC_INIT
		token = struct_alloc(token);

		// [] -> _array_create
		token = array_create(token, get_token(-2));
		token = array_ops(token);

		// Maps
		token = map_ops(token);

		// Strings
		token = string_ops(token);

		// "str->length" -> "string_length(str)"
		token = string_len(token);

		// a / b - > a / (float)b
		if (token == "/") {
			token = "/(float)";
		}

		// a(1) -> a(1, 2) // a(x: i32, y: i32 = 2)
		token = fill_fn_params(token);

		// Write token
		out(token);

		handle_spaces(token);
		handle_new_line(token);
	}
}

function write_functions() {
	for (pos = 0; pos < tokens.length; ++pos) {
		let token: string = get_token();
		if (token == "function") {
			pos++;
			if (get_token(-2) == "declare") {
				continue;
			}
			write_function();
		}

		// Write anonymous function body
		while (strings.length > 0) {
			let s: string = array_pop(strings);
			out(s);
		}
	}
}

function write_iron_c() {
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

// ██╗  ██╗    ██╗     ██████╗    ██╗  ██╗    ███████╗    ████████╗     █████╗     ██████╗     ████████╗
// ██║ ██╔╝    ██║    ██╔════╝    ██║ ██╔╝    ██╔════╝    ╚══██╔══╝    ██╔══██╗    ██╔══██╗    ╚══██╔══╝
// █████╔╝     ██║    ██║         █████╔╝     ███████╗       ██║       ███████║    ██████╔╝       ██║
// ██╔═██╗     ██║    ██║         ██╔═██╗     ╚════██║       ██║       ██╔══██║    ██╔══██╗       ██║
// ██║  ██╗    ██║    ╚██████╗    ██║  ██╗    ███████║       ██║       ██║  ██║    ██║  ██║       ██║
// ╚═╝  ╚═╝    ╚═╝     ╚═════╝    ╚═╝  ╚═╝    ╚══════╝       ╚═╝       ╚═╝  ╚═╝    ╚═╝  ╚═╝       ╚═╝

function main() {}

function alang(_alang_source: string, _alang_output: string) {
	kickstart();

	alang_source = (_alang_source); // () - no string copy
	alang_source_length = alang_source.length;
	alang_output = (_alang_output);

	// HEAP_SIZE
	any_array_resize(tokens, 512000);
	any_array_resize(strings, 32);
	any_array_resize(enums, 512);
	any_array_resize(fn_call_stack, 32);
	i32_array_resize(param_pos_stack, 32);
	any_array_resize(global_inits, 1024);
	any_array_resize(global_ptrs, 1024);
	any_array_resize(acontents, 2);
	acontents[0] = [];
	acontents[1] = [];
	any_array_resize(acontents[0], 4096);
	any_array_resize(acontents[1], 4096);
	i32_array_resize(fnested, 32);
	any_array_resize(value_types.keys, 4096 * 2);
	any_array_resize(value_types.values, 4096 * 2);
	any_array_resize(struct_types.keys, 512 * 2);
	any_array_resize(struct_types.values, 512 * 2);
	//

	parse();
	fhandle = fopen(alang_output, "wb");
	write_iron_c();
	fclose(fhandle);
}
