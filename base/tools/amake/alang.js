
let flags = globalThis.flags;
if (flags == null) {
	flags = {};
	flags.alang_source = null;
	flags.alang_output = "./test.c";
}

// ██████╗      █████╗     ██████╗     ███████╗    ███████╗
// ██╔══██╗    ██╔══██╗    ██╔══██╗    ██╔════╝    ██╔════╝
// ██████╔╝    ███████║    ██████╔╝    ███████╗    █████╗
// ██╔═══╝     ██╔══██║    ██╔══██╗    ╚════██║    ██╔══╝
// ██║         ██║  ██║    ██║  ██║    ███████║    ███████╗
// ╚═╝         ╚═╝  ╚═╝    ╚═╝  ╚═╝    ╚══════╝    ╚══════╝

let specials = [":", ";", ",", "(", ")", "[", "]", "{", "}", "<", ">", "!"];
let is_comment = false;
let is_string = false;
let pos = 0;
let tokens = [];
let header = "";

function is_alpha_numeric(code) {
	return (code > 47 && code < 58) || // 0-9
		   (code === 45) 			|| // -
		   (code === 43) 			|| // +
		   (code === 95) 			|| // _
		   (code === 46) 			|| // .
		   (code > 64 && code < 91) || // A-Z
		   (code > 96 && code < 123);  // a-z
}

function is_alpha(code) {
	return (code > 96 && code < 123) || // a-z
		   (code > 64 && code < 91)  || // A-Z
		   (code === 95);				// _
}

function is_numeric(code) {
	return (code > 47 && code < 58); // 0-9
}

function is_white_space(code) {
	return code === 32 || // " "
		   code === 9  || // "\t"
		   code === 13 || // "\r"
		   code === 10;   // "\n"
}

function read_token() {
	// Skip white space
	while (is_white_space(flags.alang_source.charCodeAt(pos))) {
		pos++;
	}

	let token = "";
	let first = true;
	let is_anum = false;

	while (pos < flags.alang_source.length) {
		let c = flags.alang_source.charAt(pos);

		// Comment start
		if (token === "//") {
			is_comment = true;
		}

		if (is_comment) {
			token += c;
			pos++;

			// Comment end
			if (c === "\n") {
				is_comment = false;
				if (token.startsWith("///include") || token.startsWith("///define")) {
					header += "#" + token.substring(3, token.length);
				}
				// Remove comments
				return null;
			}

			// Prevent parsing of comments
			continue;
		}

		// String start / end
		if (c === "\"") {
			// Escaped \"
			let last = token.length > 0 ? token.charAt(token.length - 1) : "";
			let last_last = token.length > 1 ? token.charAt(token.length - 2) : "";
			let is_escaped = last === "\\" && last_last != "\\";

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
		if (is_white_space(flags.alang_source.charCodeAt(pos))) {
			break;
		}

		// Parsing an alphanumeric token
		if (first) {
			first = false;
			is_anum = is_alpha_numeric(c.charCodeAt(0));
		}

		// Token end
		let is_special = specials.indexOf(c) > -1;
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
	tokens = [];
	pos = 0;

	while (true) {
		let token = read_token();
		if (token === "") { // No more tokens
			break;
		}
		if (token === "=" && tokens[tokens.length - 1] === "!") { // Merge "!" and "=" into "!=" token
			tokens[tokens.length - 1] = "!=";
			continue;
		}
		if (token == null) { // Throw away this token
			continue;
		}
		tokens.push(token);
	}
}

// ██╗         ██╗    ██████╗
// ██║         ██║    ██╔══██╗
// ██║         ██║    ██████╔╝
// ██║         ██║    ██╔══██╗
// ███████╗    ██║    ██████╔╝
// ╚══════╝    ╚═╝    ╚═════╝

let fhandle;
let strings = [];
let tabs = 0;
let new_line = false;
let basic_types = ["i8", "u8", "i16", "u16", "i32", "u32", "f32", "i64", "u64", "f64", "bool"];
let pass_by_value_types = ["vec4_t", "vec3_t", "vec2_t", "quat_t", "mat4_t", "mat3_t"];
let enums = [];
let value_types = new Map();
let struct_types = new Map();
let fn_declarations = new Map();
let fn_default_params = new Map();
let fn_call_stack = [];
let param_pos_stack = [];
let is_for_loop = false;
let global_inits = [];
let global_ptrs = [];

function handle_tabs(token) {
	// Entering block, add tab
	if (token === "{") {
		tabs++;
	}
	else if (token === "}") {
		tabs--;
	}

	if (is_for_loop) {
		return;
	}

	// New line, add tabs
	if (new_line) {
		for (let i = 0; i < tabs; ++i) {
			out("\t");
		}
	}
}

function handle_new_line(token) {
	if (is_for_loop) {
		return;
	}

	// Insert new line
	new_line = token === ";" || token === "{" || token === "}";
	if (new_line) {
		out("\n");
	}
}

let add_space_keywords = ["return", "else"];

function handle_spaces(token) {
	// Add space to separate keywords
	if (add_space_keywords.indexOf(token) > -1) {
		out(" ");
	}
}

function get_token(off = 0) {
	// if (pos + off < tokens.length) {
		return tokens[pos + off];
	// }
	// return "";
}

function get_token_after_piece() {
	let _pos = pos;
	skip_piece();
	let t = get_token(1);
	pos = _pos;
	return t;
}

function skip_until(s) {
	while (true) {
		let token = get_token();
		if (token === s) {
			break;
		}
		pos++;
	}
}

function skip_block() { // { ... }
	pos++; // {
	let nested = 1;
	while (true) {
		let token = get_token();
		if (token === "}") {
			nested--;
			if (nested === 0) {
				break;
			}
		}
		else if (token === "{") {
			nested++;
		}
		pos++;
	}
}

function function_return_type() {
	let _pos = pos;
	skip_until("{");
	let t = get_token(-1);
	if (t === "]") { // ): i32[]
		pos -= 2;
	}
	else if (t === ">") { // ): map_t<a, b>
		pos -= 5;
	}
	pos -= 2; // ): i32 {
	let result = get_token() === ":" ? read_type() : "void";
	pos = _pos;
	return result;
}

function join_type_name(type, name) {
	value_types.set(name, type);
	if (type.indexOf("(*NAME)(") > 0) { // Function pointer
		return type.replace("NAME", name);
	}
	return type + " " + name;
}

function read_type() { // Cursor at ":"
	pos++;
	let type = get_token();

	if (type === "(" && get_token(1) === "(") { // (()=>void)[]
		pos++;
	}

	if (get_token(1) === ")" && get_token(2) === "[") { // (()=>void)[]
		pos++;
	}

	if (type === "(") { // ()=>void
		let params = "(";

		if (get_token(1) != ")") { // Has params
			while (true) {
				skip_until(":");
				params += read_type();

				pos++;
				if (get_token() === ")") { // End of params
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
		let ret = read_type();
		type = ret + "(*NAME)" + params;
	}

	if (type === "color_t") {
		type = "i32";
	}
	else if (type === "string") {
		type = "string_t";
	}

	if (get_token(1) === "[") { // Array
		if (is_struct(type)) {
			type = type + "_array_t";
		}
		else if (type === "bool") {
			type = "u8_array_t";
		}
		else {
			type = type + "_array_t";
		}
		pos += 2; // Skip "[]"
	}

	if (is_struct(type) && pass_by_value_types.indexOf(type) === -1) {
		type += " *";
	}

	if (type === "map_t *" && get_token(1) === "<") { // Map
		let mv = get_token(4);
		if (mv === "f32") {
			type = "f32_" + type;
		}
		else if (basic_types.indexOf(mv) > -1) {
			let mk = get_token(2);
			if (mk === "i32") {
				type = "i32_i" + type;
			}
			else {
				type = "i32_" + type;
			}
		}
		else {
			let mk = get_token(2);
			if (mk === "i32") {
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

function enum_access(s) {
	// Turn enum_t.VALUE into enum_t_VALUE
	if (s.indexOf("_t.") > -1) {
		for (let e of enums) {
			if (s.indexOf(e) > -1) {
				s = s.replaceAll(".", "_");
				break;
			}
		}
	}
	return s;
}

function struct_access(s) {
	// Turn a.b into a->b
	let dot = s.indexOf(".");
	if (dot === -1) {
		return s;
	}
	if (is_numeric(s.charCodeAt(dot + 1))) {
		return s;
	}
	if (s.indexOf("\"") > -1) {
		return s;
	}
	if (s.startsWith("GC_ALLOC_INIT")) {
		return s;
	}

	// Accessing pass_by_value_types struct like vec4.x
	let has_minus = s.startsWith("-");
	let base = s.substring(has_minus ? 1 : 0, dot);
	let type = value_types.get(base);
	if (type != null) {
		let dot_last = s.lastIndexOf(".");
		if (dot != dot_last) { // a.vec4.x
			let parts = s.split(".");
			for (let i = 1; i < parts.length - 1; ++i) {
				let struct_value_types = struct_types.get(type);
				if (struct_value_types != null) {
					type = struct_value_types.get(parts[i]);

					// struct my_struct -> my_struct_t
					// my_struct * -> my_struct
					if (type.startsWith("struct ") && type.endsWith(" *")) {
						type = type.substring(0, type.length - 2);
						type = type.substring(7, type.length) + "_t *";
					}
				}
			}

			if (!type.endsWith(" *")) { // vec4_t
				let first = s.substring(0, dot_last).replaceAll(".", "->");
				let last = s.substring(dot_last, s.length);
				return first + last; // a->vec4.x
			}
		}
		else if (!type.endsWith(" *")) { // vec4.x
			return s;
		}
	}

	return s.replaceAll(".", "->");
}

function struct_alloc(token, type = null) {
	// "= { a: b, ... }" -> GC_ALLOC_INIT
	if ((get_token(-1) === "=" && token === "{" && get_token(-3) != "type") || type != null) {
		if (type == null) {
			// let a: b = {, a = {, a.b = {
			let i = get_token(-3) === ":" ? -4 : -2;
			let t = struct_access(get_token(i));
			type = value_type(t);
			if (type.endsWith(" *")) { // my_struct * -> my_struct
				type = type.substring(0, type.length - 2);
			}
			if (type.startsWith("struct ")) { // struct my_struct -> my_struct_t
				type = type.substring(7, type.length) + "_t";
			}
		}

		token = "GC_ALLOC_INIT(" + type + ", {";
		pos++; // {
		let ternary = 0;
		while (true) {
			let t = get_token();
			if (t == "}") {
				break;
			}

			let tc = get_token_c();
			tc = string_ops(tc);
			if (get_token(1) === ":" && !ternary) {
				tc = "." + tc; // a: b -> .a = b
			}
			if (t === ":") {
				if (ternary > 0) {
					ternary--;
				}
				else {
					tc = "=";
				}
			}
			else if (t === "?") {
				ternary++;
			}
			else if (t === "[" && get_token(-1) === ":") {
				let member = get_token(-2);
				let types = struct_types.get(type + " *");
				let content_type = types.get(member);
				content_type = content_type.substring(7, content_type.length - 8); // struct my_struct_array_t * -> my_struct
				tc = array_create("[", "", content_type);
			}
			token += tc;
			pos++;
		}

		// "= {}" -> "= {0}", otherwise msvc refuses to init members to zero
		if (get_token(-1) === "{") {
			token += "0";
		}

		token += get_token(); // "}"
		token += ")";
		tabs--;
	}
	return token;
}

function array_type(name) {
	let type = value_type(name);
	if (type == null) {
		return "any";
	}
	if (type.startsWith("struct ")) { // struct u8_array * -> u8_array_t *
		type = type.substring(7, type.length - 2) + "_t *";
	}
	type = strip(type, 10); // Strip _array_t *
	if (basic_types.indexOf(type) > -1) { // u8
		return type;
	}
	return "any";
}

function array_contents(type) {
	// [1, 2, ..] or [{a:b}, {a:b}, ..] or [a(), b(), ..]
	let contents = [];
	let content = "";
	while (true) {
		pos++;
		let token = get_token();
		if (token === "]") {
			if (content != "") {
				contents.push(content);
			}
			break;
		}
		if (token === ",") {
			contents.push(content);
			content = "";
			continue;
		}
		if (token === "{") {
			token = struct_alloc(token, type);
			tabs++; // Undo tabs-- in struct_alloc
		}
		if (get_token(1) === "(") {
			// Function call
			while (true) {
				pos++;
				token += get_token();
				token = fill_default_params(token);
				if (token.endsWith(")")) {
					break;
				}
			}
		}
		// a / b - > a / (float)b
		if (token === "/") {
			token = "/(float)";
		}
		token = struct_access(token);
		token = string_ops(token);
		content += token;
	}
	return contents;
}

function member_name(name) {
	let i = name.lastIndexOf(".");
	if (i > -1) {
		name = name.substring(i + 1, name.length);
	}
	return name;
}

function array_create(token, name, content_type = null) {
	if ((get_token(-1) === "=" && token === "[") || content_type != null) {
		if (name === "]") {
			name = get_token(-6); // ar: i32[] = [
		}
		let member = member_name(name);
		let type = array_type(member);

		// = [], = [1, 2, ..] -> any/i32/.._array_create_from_raw
		if (content_type == null) {
			name = struct_access(name);
			content_type = value_type(name);
			if (content_type != null) {
				content_type = content_type.substring(0, content_type.length - 10);
			}
		}
		let contents = array_contents(content_type);
		token = type + "_array_create_from_raw((" + type + "[]){";
		for (let e of contents) {
			token += e + ",";
		}
		token += "}," + contents.length + ")";
	}
	return token;
}

function array_access(token) {
	// array[0] -> array->buffer[0]
	if (token === "[" && get_token(-1) != "=") {
		return "->buffer[";
	}
	return token;
}

function is_struct(type) {
	// Ends with _t and is not an enum
	return type.endsWith("_t") && enums.indexOf(type) === -1;
}

function strip(name, len) {
	return name.substring(0, name.length - len);
}

function strip_optional(name) {
	if (name.endsWith("?")) {
		return strip(name, 1); // :val? -> :val
	}
	return name;
}

function param_pos() {
	return param_pos_stack[param_pos_stack.length - 1];
}

function param_pos_add() {
	param_pos_stack[param_pos_stack.length - 1]++;
}

function fn_call() {
	return fn_call_stack[fn_call_stack.length - 1];
}

function fill_fn_params(token) {

	if (token === "(") {
		// Function is being called to init this variable
		fn_call_stack.push(get_token(-1));
		param_pos_stack.push(0);
	}

	else if (token === ",") {
		// Param has beed passed manually
		param_pos_add();
	}

	else if (token === ")") {
		// Fill in default parameters if needed
		if (get_token(-1) != "(") {
			// Param has beed passed manually
			param_pos_add();
		}

		// If default param exists, fill it
		let res = "";
		while (fn_default_params.has(fn_call() + param_pos())) {
			if (param_pos() > 0) {
				res += ",";
			}
			res += fn_default_params.get(fn_call() + param_pos());
			param_pos_add();
		}

		fn_call_stack.pop();
		param_pos_stack.pop();
		token = res + token;
	}

	return token;
}

function get_token_c() {
	let t = get_token();
	t = enum_access(t);
	t = struct_access(t);
	t = array_access(t);
	return t;
}

function fill_default_params(piece) {
	// fn(a, 1, ..)
	if (get_token(1) === ")") {
		let fn_name = "";
		let params = 0;
		let i = 0;
		let nested = 1;
		while (true) {
			let ti = get_token(i);
			if (ti === ")") {
				nested++;
			}
			else if (ti === "(") {
				if (get_token(i - 1) === ":") { // : ()=>void
					return piece;
				}
				nested--;
				if (nested === 0) {
					fn_name = get_token(i - 1);
					if (get_token(i + 1) != ")") {
						params++;
					}
					break;
				}
			}
			else if (ti === ",") {
				params++;
			}
			i--;
		}
		while (fn_default_params.has(fn_name + params)) {
			if (params > 0) {
				piece += ",";
			}
			piece += fn_default_params.get(fn_name + params);
			params++;
		}
	}
	return piece;
}

function skip_piece(nested = 0) {
	let t1 = get_token(1);
	if (t1 === "(" || t1 === "[") {
		nested++;
		pos++;
		skip_piece(nested);
		return;
	}

	if (t1 === ")" || t1 === "]") {
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

	if (t1.startsWith(".")) {
		pos++;
		skip_piece();
		return;
	}
}

function read_piece(nested = 0) {
	// call(a, b, c), a[b + c]
	let piece = get_token_c();
	piece = string_length(piece);
	piece = map_ops(piece);

	let t1 = get_token(1);
	if (t1 === "(" || t1 === "[") {
		nested++;
		pos++;
		return piece + read_piece(nested);
	}

	if (t1 === ")" || t1 === "]") {
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

	if (t1.startsWith(".")) {
		pos++;
		return piece + read_piece();
	}

	return piece;
}

function string_length(token) {
	// "str->length" -> "string_length(str)"
	if (!token.endsWith("->length")) {
		return token;
	}
	let base = strip(token, 8); // ->length
	let type = value_type(base);
	if (type === "string_t *" || type === "struct string *") {
		token = "string_length(" + base + ")";
	}
	return token;
}

function value_type(value) {
	let arrow = value.indexOf("->");
	if (arrow > -1) {
		let base = value.substring(0, arrow);
		let type = value_types.get(base);
		let member = value.substring(arrow + 2, value.length);
		let struct_value_types = struct_types.get(type);
		if (struct_value_types != null) {
			return struct_value_types.get(member);
		}
	}
	return value_types.get(value);
}

function set_fn_param_types() {
	pos++; // (
	while (true) {
		pos++;
		let t = get_token(); // Param name, ) or ,
		if (t === ")") { // Params end
			break;
		}
		if (t === ",") { // Next param
			continue;
		}

		pos++; // :
		let type = read_type();
		value_types.set(t, type);

		if (get_token(1) === "=") { // Skip default value
			pos += 2;
		}
	}
}

function number_to_string(token) {
	let type = value_type(token);
	if (type === "i32") {
		return "i32_to_string(" + token + ")";
	}
	if (type === "f32") {
		return "f32_to_string(" + token + ")";
	}
	return token;
}

function token_is_string(token) {
	let is_string = token.startsWith("\"") || value_type(token) === "string_t *";

	if (!is_string) {
		let _pos = pos;
		skip_piece();
		let t1 = get_token(1);
		if (t1 === "+" || t1 === "+=" || t1 === "==" || t1 === "!=") {
			let t2 = get_token(2);
			if (t2.startsWith("\"") || value_type(t2) === "string_t *") {
				is_string = true;
			}
		}
		pos = _pos;
	}

	if (is_string && get_token(2) === "null") {
		is_string = false;
	}

	return is_string;
}

function string_add(token) {
	token = "string_join(" + token + ",";
	pos++;

	let token_b = read_piece();
	token_b = number_to_string(token_b);
	token += token_b;
	token += ")";
	return token;
}

function string_ops(token) {
	// "str" + str + 1 + ...

	if (!token_is_string(token)) {
		return token;
	}

	let first = true;
	while (get_token_after_piece() === "+") {
		if (first) {
			first = false;
			token = read_piece();
			token = number_to_string(token);
		}
		pos++;
		token = string_add(token);
	}

	// "str" += str + str2
	let p1 = get_token_after_piece();
	if (p1 === "+=") {
		token = read_piece();
		pos++;
		token = token + "=string_join(" + token + ",";
		pos++;
		if (get_token_after_piece() === "+") {
			let token_b = read_piece();
			token_b = number_to_string(token_b);
			while (get_token_after_piece() === "+") {
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
	else if (p1 === "=") {
		let _pos = pos;
		let t1 = read_piece();
		pos++;
		pos++; // =
		let t2 = read_piece();
		if (get_token(1) === ";" && !t2.startsWith("\"")) { // str = str;
			token = t1 + "=string_copy(" + t2 + ")"; // Copy string
		}
		else {
			pos = _pos;
		}
	}

	// "str" === str
	else if (p1 === "==") {
		token = read_piece();
		pos++;

		token = "string_equals(" + token + ",";
		pos++;
		token += read_piece();
		token += ")";
	}

	// "str" != str
	else if (p1 === "!=") {
		token = read_piece();
		pos++;

		token = "!string_equals(" + token + ",";
		pos++;
		token += read_piece();
		token += ")";
	}

	return token;
}

function array_ops(token) {
	// array_push -> i32_array_push/any_array_push
	if (token === "array_push") {
		let value = get_token(2);

		if (get_token(3) === "[") { // raws[i].value
			value = get_token(6).substring(1); // .value
		}

		if (value.lastIndexOf(".") > -1) {
			value = value.substring(value.lastIndexOf(".") + 1, value.length);
		}
		let type = array_type(value);
		token = type + "_array_push";
	}

	// array_remove -> i32_array_remove / char_ptr_array_remove
	else if (token === "array_remove") {
		let value = get_token(2);
		value = struct_access(value);
		let type = value_type(value);
		if (type != null && type.startsWith("struct ")) { // struct u8_array * -> u8_array_t *
			type = type.substring(7, type.length - 2) + "_t *";
		}

		if (type === "i32_array_t *") {
			token = "i32_array_remove";
		}
		else if (type === "string_t_array_t *") {
			token = "char_ptr_array_remove";
		}
	}

	// array_index_of -> i32_array_index_of / char_ptr_array_index_of
	else if (token === "array_index_of") {
		let value = get_token(2);
		value = struct_access(value);
		let type = value_type(value);
		if (type != null && type.startsWith("struct ")) { // struct u8_array * -> u8_array_t *
			type = type.substring(7, type.length - 2) + "_t *";
		}

		if (type === "i32_array_t *") {
			token = "i32_array_index_of";
		}
		else if (type === "string_t_array_t *") {
			token = "char_ptr_array_index_of";
		}
		else {
			let fn = fn_declarations.get(value);
			if (fn != null && fn.startsWith("string_t_array_t *")) {
				token = "char_ptr_array_index_of";
			}
		}
	}

	return token;
}

function map_ops(token) {
	if (token === "map_create") {
		let t = value_type(get_token(-2));
		if (t === "i32_map_t *") {
			token = "i32_map_create";
		}
		else if (t === "i32_imap_t *") {
			token = "i32_imap_create";
		}
		else if (t === "any_imap_t *") {
			token = "any_imap_create";
		}
		else {
			token = "any_map_create";
		}
	}

	else if (token === "map_set") {
		let t = value_type(get_token(2));
		if (t === "i32_map_t *") {
			token = "i32_map_set";
		}
		else if (t === "i32_imap_t *") {
			token = "i32_imap_set";
		}
		else if (t === "any_imap_t *") {
			token = "any_imap_set";
		}
		else {
			token = "any_map_set";
		}
	}

	else if (token === "map_get") {
		let t = value_type(get_token(2));
		if (t === "i32_map_t *") {
			token = "i32_map_get";
		}
		else if (t === "i32_imap_t *") {
			token = "i32_imap_get";
		}
		else if (t === "any_imap_t *") {
			token = "any_imap_get";
		}
		else {
			token = "any_map_get";
		}
	}

	else if (token === "map_delete") {
		let t = value_type(get_token(2));
		if (t === "any_imap_t *") {
			token = "imap_delete";
		}
	}

	return token;
}

function _t_to_struct(type) {
	// type_t * -> struct type *
	if (type.endsWith("_t *") && type != "string_t *") {
		let type_short = strip(type, 4); // _t *
		if (type_short.endsWith("_array")) { // tex_format_t_array -> i32_array
			for (let e of enums) {
				if (type_short.startsWith(e)) {
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

function stream_write(token) {
	fhandle.puts(token);
}

function string_write(token) {
	strings[strings.length - 1] += token;
}

function null_write(token) {
	return;
}

let out = stream_write;

function write_enums() {
	enums = [];
	for (pos = 0; pos < tokens.length; ++pos) {
		let token = get_token();

		// Turn "enum name {}" into "typedef enum {} name;"
		if (token === "enum") {

			out = get_token(-1) === "declare" ? null_write : stream_write;

			pos++;
			let enum_name = get_token();
			enums.push(enum_name);

			out("typedef enum{\n");
			pos++; // {

			while (true) {
				// Enum contents
				pos++;
				token = get_token(); // Item name

				if (token === "}") { // Enum end
					out("}" + enum_name + ";\n");
					break;
				}

				out("\t" + enum_name + "_" + token);

				pos++; // = or ,
				token = get_token();

				if (token === "=") { // Enum value
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

	out = stream_write;
}

function write_types() {
	for (pos = 0; pos < tokens.length; ++pos) {
		let token = get_token();

		// Turn "type x = {};" into "typedef struct {} x;"
		// Turn "type x = y;" into "typedef x y;"
		if (token === "type") {

			out = get_token(-1) === "declare" ? null_write : stream_write;

			pos++;
			let struct_name = get_token();
			let stuct_name_short = strip(struct_name, 2); // _t

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
				let type = read_type();
				type = _t_to_struct(type);

				out("typedef " + type + " " + struct_name + ";\n\n");
				skip_until(";");
				continue;
			}

			// "type x = {};"
			out("typedef struct " + stuct_name_short + "{\n");

			let struct_value_types = new Map();
			struct_types.set(struct_name + " *", struct_value_types);

			while (true) {
				// Struct contents
				pos++;
				let name = get_token();

				if (name === "}") { // Struct end
					out("}" + struct_name + ";\n");
					break;
				}

				// val?: i32 -> val: i32
				name = strip_optional(name);

				pos++;
				// :
				let type = read_type();

				// type_t * -> struct type *
				type = _t_to_struct(type);

				// my_struct_t*(*NAME)(my_struct_t *) -> struct my_struct*(*NAME)(struct my_struct*)
				if (type.indexOf("(*NAME)") > -1) {
					let args = type.substring(type.indexOf(")(") + 2, type.lastIndexOf(")"));
					args = args.split(",");

					type = type.substring(0, type.indexOf("(*NAME)"));
					type = _t_to_struct(type);
					type += "(*NAME)(";

					for (let i = 0; i < args.length; ++i) {
						let arg = args[i];
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
				struct_value_types.set(name, type);
			}

			out("\n");
			continue;
		}
	}

	out = stream_write;
}

function write_array_types() {
	// Array structs (any_array_t -> scene_t_array_t)
	let array_structs = new Map();
	for (pos = 0; pos < tokens.length; ++pos) {
		let token = get_token();
		if (get_token(1) === "[") {
			let type = token;
			if (type === "string") {
				type = "string_t";
			}
			if (is_struct(type)) {
				if (!array_structs.has(type)) {
					let as = "typedef struct " + type + "_array{" +
						type + "**buffer;int length;int capacity;}" +
						type + "_array_t;";
					array_structs.set(type, as);
				}
			}
			pos += 2; // Skip "[]"
		}
	}

	for (let as of array_structs.values()) {
		out(as);
		out("\n");
	}
	out("\n");
}

function write_fn_declarations() {
	fn_declarations = new Map();
	fn_default_params = new Map();
	let last_fn_name = "";
	for (pos = 0; pos < tokens.length; ++pos) {
		let token = get_token();

		if (token === "function") {

			out = get_token(-1) === "declare" ? null_write : stream_write;

			// Return type + name
			pos++;
			let fn_name = get_token();
			let ret = function_return_type();

			if (fn_name === "(") { // Anonymous function
				fn_name = last_fn_name + "_" + (pos - 1);
				pos--;
			}
			else {
				if (fn_name === "main") {
					fn_name = "_main";
				}
				last_fn_name = fn_name;
			}

			// Params
			let _param_pos = 0;
			let params = "(";
			pos++; // (
			while (true) {
				pos++;
				token = get_token(); // Param name, ) or ,

				if (token === ")") { // Params end
					break;
				}

				if (token === ",") { // Next param
					params += ",";
					_param_pos++;
					continue;
				}

				pos++; // :
				let type = read_type();
				params += join_type_name(type, token);

				// Store param default
				if (get_token(1) === "=") {
					let param = get_token(2);
					param = enum_access(param);
					fn_default_params.set(fn_name + _param_pos, param);
					pos += 2;
				}
			}
			params += ")";

			let fn_decl = ret + " " + fn_name + params;
			out(fn_decl + ";\n");

			fn_declarations.set(fn_name, fn_decl);
		}
	}
	out("\n");

	out = stream_write;
}

function write_globals() {
	global_inits = [];
	global_ptrs = [];
	for (pos = 0; pos < tokens.length; ++pos) {
		let token = get_token();

		if (token === "let") {

			out = get_token(-1) === "declare" ? null_write : stream_write;

			pos++;
			let name = get_token();

			pos++; // :
			let type = read_type();

			if (basic_types.indexOf(type) === -1 &&
				pass_by_value_types.indexOf(type) === -1 &&
				enums.indexOf(type) === -1) {
				global_ptrs.push(name);
			}

			out(join_type_name(type, name) + ";");

			// Init this var in _kickstart()
			let is_initialized = get_token(1) === "=";
			if (is_initialized) {
				let init = name;
				tabs = 1;
				while (true) {
					pos++;
					token = get_token();

					token = enum_access(token);
					token = array_access(token);
					token = struct_alloc(token);

					if (token === ";") {
						break;
					}

					token = fill_fn_params(token);

					// [] -> _array_create
					token = array_create(token, name);

					if (token === "map_create") {
						let t = value_type(name);
						if (t === "i32_map_t *") {
							token = "i32_map_create";
						}
						else if (t === "i32_imap_t *") {
							token = "i32_imap_create";
						}
						else if (t === "any_imap_t *") {
							token = "any_imap_create";
						}
						else {
							token = "any_map_create";
						}
					}

					init += token;
				}
				global_inits.push(init);
			}

			out("\n");
		}

		// Skip function blocks
		if (token === "function" && get_token(-1) != "declare") {
			skip_until("{");
			skip_block();
		}
	}

	out = stream_write;
}

function write_kickstart() {
	// Start function
	out("\nvoid _kickstart() {\n");
	// Init globals
	for (let val of global_inits) {
		out("\t");
		let name = val.split("=")[0];
		let global_alloc = global_ptrs.indexOf(name) > -1 && !val.endsWith("=null") && !val.endsWith("\"");
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
	let fn_name = get_token();

	if (fn_name === "main") {
		fn_name = "_main";
	}

	let fn_decl = fn_declarations.get(fn_name);
	out(fn_decl + "{\n");

	// Function body
	// Re-set function param types into value_types map
	set_fn_param_types();
	skip_until("{");

	tabs = 1;
	new_line = true;
	let mark_as_root = null;
	let nested = [];

	while (true) {
		pos++;
		let token = get_token();

		if (token === "function") { // Begin nested function
			let anon_fn = fn_name + "_" + pos;
			out("&" + anon_fn);
			if (get_token(-1) === "=") {
				out(";\n");
			}

			skip_until("{");
			out = string_write;
			strings.push("");
			let fn_decl = fn_declarations.get(anon_fn);
			out(fn_decl + "{\n");

			let find = fn_decl.substring(0, fn_decl.indexOf("("));
			let fn_pos = parseInt(find.substring(find.lastIndexOf("_") + 1, find.length));
			let _pos = pos;
			pos = fn_pos;
			set_fn_param_types();
			pos = _pos;

			nested.push(1);
			continue;
		}

		if (nested.length > 0) {
			if (token === "{") {
				nested[nested.length - 1]++;
			}
			else if (token === "}") { // End nested function
				nested[nested.length - 1]--;
				if (nested[nested.length - 1] === 0) {
					nested.pop();
					out("}\n\n");
					if (strings.length > 1) {
						let s = strings.pop();
						strings[strings.length - 1] = s + strings[strings.length - 1];
					}

					if (nested.length === 0) {
						out = stream_write;
					}
					continue;
				}
			}
		}

		// Function end
		if (token === "}" && tabs === 1) {
			out("}\n\n");
			break;
		}

		handle_tabs(token);

		if (token === "for") { // Skip new lines in for (;;) loop
			is_for_loop = true;
		}
		else if (token === "{") {
			is_for_loop = false;
		}
		// Write type and name
		else if (token === "let") {
			pos++;
			let name = get_token();
			pos++; // :
			let type = read_type();
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
		let is_assign = get_token(1) === "=" || get_token(1) === "+="; // += for string_join
		if (is_assign && token != ":") {
			if (global_ptrs.indexOf(token) > -1) {
				out("gc_unroot(" + token + ");");
				if (get_token(2) != "null") {
					mark_as_root = token;
				}
			}
		}
		if (token === ";" && mark_as_root != null) {
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
		token = string_length(token);

		// a / b - > a / (float)b
		if (token === "/") {
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
		let token = get_token();
		if (token === "function") {
			pos++;
			if (get_token(-2) === "declare") {
				continue;
			}
			write_function();
		}

		// Write anonymous function body
		while (strings.length > 0) {
			let s = strings.pop();
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

function alang() {
	parse();
	fhandle = std.open(flags.alang_output, "w");
	write_iron_c();
	fhandle.close();
}

let t = Date.now();
alang();
console.log("alang took " + (Date.now() - t) + "ms.");
