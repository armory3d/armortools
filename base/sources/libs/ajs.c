#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_TOKENS 1000
#define MAX_VARS 100
#define MAX_STACK 100
#define MAX_SCOPES 10

typedef enum {
    VAL_NUM,
    VAL_STR,
    VAL_OBJ
} value_type_t;

typedef enum {
    TOK_NUM,
    TOK_STR,
    TOK_IDENT,
    TOK_LET,
    TOK_IF,
    TOK_ELSE,
    TOK_WHILE,
    TOK_FOR,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_SEMI,
    TOK_EQ,
    TOK_PLUS,
    TOK_MINUS,
    TOK_MUL,
    TOK_DIV,
    TOK_LT,
    TOK_GT,
    TOK_LE,
    TOK_GE,
    TOK_EQEQ,
    TOK_NE,
    TOK_FUNC,
    TOK_RETURN,
    TOK_COMMA,
    TOK_DOT,
    TOK_COLON,
    TOK_EOF
} token_type_t;

typedef struct interpreter interpreter_t;

typedef struct value {
    value_type_t type;
    union {
        double num;
        char *str;
        struct {
            struct value *fields;
            char **field_names;
            int field_count;
        } obj;
        struct {
            int func_start;
            int param_count;
            char **param_names;
            int is_native;
            struct value (*native_func)(interpreter_t *, struct value *, int);
        } func;
    } data;
} value_t;

typedef struct token {
    token_type_t type;
    char *value;
    value_t val;
} token_t;

typedef struct variable {
    char *name;
    value_t value;
    int is_func;
    int func_start;
    int param_count;
    char **param_names;
} variable_t;

typedef struct scope {
    variable_t vars[MAX_VARS];
    int var_count;
} scope_t;

typedef struct interpreter {
    token_t tokens[MAX_TOKENS];
    int token_count;
    int pos;
    scope_t scopes[MAX_SCOPES];
    int scope_depth;
    value_t stack[MAX_STACK];
    int stack_ptr;
} interpreter_t;

void init_interpreter(interpreter_t *interp);
void push_scope(interpreter_t *interp);
void pop_scope(interpreter_t *interp);
value_t *get_var(interpreter_t *interp, const char *name);
variable_t *get_var_def(interpreter_t *interp, const char *name);
void skip_whitespace(const char **p);
void tokenize(interpreter_t *interp, const char *code);
value_t eval_expression(interpreter_t *interp);
value_t eval_object(interpreter_t *interp);
void execute_statement(interpreter_t *interp);
void execute(interpreter_t *interp, const char *code);

void print_value(value_t val) {
    switch (val.type) {
        case VAL_NUM:
            printf("%f", val.data.num);
            break;
        case VAL_STR:
            printf("%s", val.data.str);
            break;
        case VAL_OBJ:
            if (val.data.func.func_start != 0) {
                printf("[Function]");
            }
            else {
                printf("{");
                for (int i = 0; i < val.data.obj.field_count; i++) {
                    printf("%s: ", val.data.obj.field_names[i]);
                    print_value(val.data.obj.fields[i]);
                    if (i < val.data.obj.field_count - 1) {
                        printf(", ");
                    }
                }
                printf("}");
            }
            break;
    }
}

value_t native_console_log(interpreter_t *interp, value_t *args, int arg_count) {
    if (arg_count > 0) {
        print_value(args[0]);
        printf("\n");
    }
    value_t result = {VAL_NUM};
    result.data.num = 0;
    return result;
}

void init_interpreter(interpreter_t *interp) {
    interp->token_count = 0;
    interp->pos = 0;
    interp->scope_depth = 0;
    interp->scopes[0].var_count = 0;
    interp->stack_ptr = 0;

    variable_t *console = &interp->scopes[0].vars[interp->scopes[0].var_count++];
    console->name = strdup("console");
    console->value.type = VAL_OBJ;
    console->value.data.obj.field_count = 1;
    console->value.data.obj.fields = malloc(sizeof(value_t));
    console->value.data.obj.field_names = malloc(sizeof(char *));
    console->value.data.obj.field_names[0] = strdup("log");
    console->is_func = 0;

    value_t log_func = {0};
    log_func.type = VAL_OBJ;
    log_func.data.func.func_start = -1;
    log_func.data.func.is_native = 1;
    log_func.data.func.native_func = native_console_log;
    log_func.data.func.param_count = 1;
    log_func.data.func.param_names = malloc(sizeof(char *));
    log_func.data.func.param_names[0] = strdup("arg");

    console->value.data.obj.fields[0] = log_func;
}

void push_scope(interpreter_t *interp) {
    interp->scope_depth++;
    interp->scopes[interp->scope_depth].var_count = 0;
}

void pop_scope(interpreter_t *interp) {
    interp->scope_depth--;
}

value_t *get_var(interpreter_t *interp, const char *name) {
    for (int s = interp->scope_depth; s >= 0; s--) {
        for (int i = 0; i < interp->scopes[s].var_count; i++) {
            if (strcmp(interp->scopes[s].vars[i].name, name) == 0) {
                return &interp->scopes[s].vars[i].value;
            }
        }
    }
    variable_t *v = &interp->scopes[interp->scope_depth].vars[interp->scopes[interp->scope_depth].var_count++];
    v->name = strdup(name);
    v->value.type = VAL_NUM;
    v->value.data.num = 0;
    v->is_func = 0;
    return &v->value;
}

variable_t *get_var_def(interpreter_t *interp, const char *name) {
    for (int s = interp->scope_depth; s >= 0; s--) {
        for (int i = 0; i < interp->scopes[s].var_count; i++) {
            if (strcmp(interp->scopes[s].vars[i].name, name) == 0) {
                return &interp->scopes[s].vars[i];
            }
        }
    }
    return NULL;
}

void skip_whitespace(const char **p) {
    while (isspace(**p)) {
        (*p)++;
    }
}

void tokenize(interpreter_t *interp, const char *code) {
    const char *p = code;
    while (*p) {
        skip_whitespace(&p);
        if (!*p) {
            break;
        }

        token_t *t = &interp->tokens[interp->token_count++];

        if (isdigit(*p)) {
            t->type = TOK_NUM;
            t->val.type = VAL_NUM;
            t->val.data.num = strtod(p, (char **)&p);
        }
        else if (*p == '"') {
            t->type = TOK_STR;
            t->val.type = VAL_STR;
            p++;
            const char *start = p;
            while (*p && *p != '"') {
                p++;
            }
            t->val.data.str = strndup(start, p - start);
            p++;
        }
        else if (isalpha(*p)) {
            const char *start = p;
            while (isalnum(*p)) p++;
            t->value = strndup(start, p - start);
            if (strcmp(t->value, "let") == 0) t->type = TOK_LET;
            else if (strcmp(t->value, "if") == 0) t->type = TOK_IF;
            else if (strcmp(t->value, "else") == 0) t->type = TOK_ELSE;
            else if (strcmp(t->value, "while") == 0) t->type = TOK_WHILE;
            else if (strcmp(t->value, "for") == 0) t->type = TOK_FOR;
            else if (strcmp(t->value, "function") == 0) t->type = TOK_FUNC;
            else if (strcmp(t->value, "return") == 0) t->type = TOK_RETURN;
            else t->type = TOK_IDENT;
        }
        else {
            switch (*p) {
                case '(': t->type = TOK_LPAREN; break;
                case ')': t->type = TOK_RPAREN; break;
                case '{': t->type = TOK_LBRACE; break;
                case '}': t->type = TOK_RBRACE; break;
                case ';': t->type = TOK_SEMI; break;
                case '=':
                    if (*(p+1) == '=') { t->type = TOK_EQEQ; p++; }
                    else t->type = TOK_EQ;
                    break;
                case '+': t->type = TOK_PLUS; break;
                case '-': t->type = TOK_MINUS; break;
                case '*': t->type = TOK_MUL; break;
                case '/': t->type = TOK_DIV; break;
                case '<':
                    if (*(p+1) == '=') { t->type = TOK_LE; p++; }
                    else t->type = TOK_LT;
                    break;
                case '>':
                    if (*(p+1) == '=') { t->type = TOK_GE; p++; }
                    else t->type = TOK_GT;
                    break;
                case '!':
                    if (*(p+1) == '=') { t->type = TOK_NE; p++; }
                    break;
                case ',': t->type = TOK_COMMA; break;
                case '.': t->type = TOK_DOT; break;
                case ':': t->type = TOK_COLON; break;
                default: printf("Unknown char: %c\n", *p); exit(1);
            }
            p++;
        }
    }
    interp->tokens[interp->token_count++].type = TOK_EOF;
}

value_t eval_object(interpreter_t *interp) {
    value_t obj;
    obj.type = VAL_OBJ;
    obj.data.obj.field_count = 0;
    obj.data.obj.fields = NULL;
    obj.data.obj.field_names = NULL;

    interp->pos++;
    while (interp->tokens[interp->pos].type != TOK_RBRACE) {
        char *field_name = interp->tokens[interp->pos++].value;
        interp->pos++;
        value_t field_value = eval_expression(interp);

        obj.data.obj.fields = realloc(obj.data.obj.fields,
            sizeof(value_t) * (obj.data.obj.field_count + 1));
        obj.data.obj.field_names = realloc(obj.data.obj.field_names,
            sizeof(char *) * (obj.data.obj.field_count + 1));
        obj.data.obj.field_names[obj.data.obj.field_count] = strdup(field_name);
        obj.data.obj.fields[obj.data.obj.field_count] = field_value;
        obj.data.obj.field_count++;

        if (interp->tokens[interp->pos].type == TOK_COMMA) {
            interp->pos++;
        }
    }
    interp->pos++;
    return obj;
}

value_t eval_expression(interpreter_t *interp) {
    value_t left;
    memset(&left, 0, sizeof(value_t));

    if (interp->tokens[interp->pos].type == TOK_NUM ||
        interp->tokens[interp->pos].type == TOK_STR) {
        left = interp->tokens[interp->pos++].val;
    }
    else if (interp->tokens[interp->pos].type == TOK_IDENT) {
        char *name = interp->tokens[interp->pos++].value;

        if (interp->tokens[interp->pos].type == TOK_LPAREN) {
            variable_t *func_var = get_var_def(interp, name);
            if (func_var && func_var->is_func) {
                interp->pos++;
                value_t params[10];
                int param_count = 0;
                while (interp->tokens[interp->pos].type != TOK_RPAREN) {
                    params[param_count++] = eval_expression(interp);
                    if (interp->tokens[interp->pos].type == TOK_COMMA) {
                        interp->pos++;
                    }
                }
                interp->pos++;

                push_scope(interp);
                for (int i = 0; i < func_var->param_count; i++) {
                    *get_var(interp, func_var->param_names[i]) = params[i];
                }
                int old_pos = interp->pos;
                interp->pos = func_var->func_start;
                execute_statement(interp);
                pop_scope(interp);
                interp->pos = old_pos;
                return interp->stack[--interp->stack_ptr];
            }
        }
        else if (interp->tokens[interp->pos].type == TOK_DOT) {
            interp->pos++;
            char *field = interp->tokens[interp->pos++].value;
            value_t *obj = get_var(interp, name);

            if (obj->type == VAL_OBJ) {
                for (int i = 0; i < obj->data.obj.field_count; i++) {
                    if (strcmp(obj->data.obj.field_names[i], field) == 0) {
                        value_t field_val = obj->data.obj.fields[i];

                        if (interp->tokens[interp->pos].type == TOK_LPAREN) {
                            interp->pos++;
                            value_t params[10];
                            int param_count = 0;
                            while (interp->tokens[interp->pos].type != TOK_RPAREN) {
                                params[param_count++] = eval_expression(interp);
                                if (interp->tokens[interp->pos].type == TOK_COMMA) {
                                    interp->pos++;
                                }
                            }
                            interp->pos++;

                            if (field_val.type == VAL_OBJ && field_val.data.func.func_start != 0) {
                                if (field_val.data.func.is_native) {
                                    return field_val.data.func.native_func(interp, params, param_count);
                                }
                                else {
                                    push_scope(interp);
                                    for (int j = 0; j < field_val.data.func.param_count; j++) {
                                        *get_var(interp, field_val.data.func.param_names[j]) = params[j];
                                    }
                                    int old_pos = interp->pos;
                                    interp->pos = field_val.data.func.func_start;
                                    execute_statement(interp);
                                    pop_scope(interp);
                                    interp->pos = old_pos;
                                    return interp->stack[--interp->stack_ptr];
                                }
                            }
                        }
                        return field_val;
                    }
                }
            }
        }
        left = *get_var(interp, name);
    }
    else if (interp->tokens[interp->pos].type == TOK_LBRACE) {
        return eval_object(interp);
    }
    else if (interp->tokens[interp->pos].type == TOK_LPAREN) {
        interp->pos++;
        left = eval_expression(interp);
        interp->pos++;
    }

    while (interp->pos < interp->token_count) {
        token_type_t op = interp->tokens[interp->pos].type;
        if (op != TOK_PLUS && op != TOK_MINUS && op != TOK_MUL && op != TOK_DIV &&
            op != TOK_LT && op != TOK_GT && op != TOK_LE && op != TOK_GE &&
            op != TOK_EQEQ && op != TOK_NE) break;
        interp->pos++;

        value_t right = eval_expression(interp);

        if (left.type == VAL_NUM && right.type == VAL_NUM) {
            switch (op) {
                case TOK_PLUS: left.data.num += right.data.num; break;
                case TOK_MINUS: left.data.num -= right.data.num; break;
                case TOK_MUL: left.data.num *= right.data.num; break;
                case TOK_DIV: left.data.num /= right.data.num; break;
                case TOK_LT: left.type = VAL_NUM; left.data.num = left.data.num < right.data.num; break;
                case TOK_GT: left.type = VAL_NUM; left.data.num = left.data.num > right.data.num; break;
                case TOK_LE: left.type = VAL_NUM; left.data.num = left.data.num <= right.data.num; break;
                case TOK_GE: left.type = VAL_NUM; left.data.num = left.data.num >= right.data.num; break;
                case TOK_EQEQ: left.type = VAL_NUM; left.data.num = left.data.num == right.data.num; break;
                case TOK_NE: left.type = VAL_NUM; left.data.num = left.data.num != right.data.num; break;
            }
        }
    }
    return left;
}

void execute_statement(interpreter_t *interp) {
    while (interp->pos < interp->token_count &&
           interp->tokens[interp->pos].type != TOK_RBRACE) {
        token_t t = interp->tokens[interp->pos];

        if (t.type == TOK_IF) {
            interp->pos++; // Skip IF
            interp->pos++; // Skip LPAREN
            value_t cond = eval_expression(interp);
            interp->pos++; // Skip RPAREN
            interp->pos++; // Skip LBRACE
            push_scope(interp);
            int if_end_pos = interp->pos;

            if (cond.type == VAL_NUM && cond.data.num) {
                execute_statement(interp);
                if_end_pos = interp->pos;
            }
			else {
                while (interp->tokens[interp->pos].type != TOK_RBRACE) {
                    interp->pos++;
                }
                if_end_pos = interp->pos;
            }
            pop_scope(interp);
            interp->pos++; // Skip RBRACE

            if (interp->tokens[interp->pos].type == TOK_ELSE) {
                interp->pos++; // Skip ELSE
                interp->pos++; // Skip LBRACE
                push_scope(interp);

                if (cond.type == VAL_NUM && !cond.data.num) {
                    execute_statement(interp);
                }
				else {
                    while (interp->tokens[interp->pos].type != TOK_RBRACE) {
                        interp->pos++;
                    }
                }
                pop_scope(interp);
                interp->pos++; // Skip RBRACE
            }
			else {
                interp->pos = if_end_pos + 1;
            }
        }
        else if (t.type == TOK_LET) {
            interp->pos++;
            char *name = interp->tokens[interp->pos++].value;
            interp->pos++;
            *get_var(interp, name) = eval_expression(interp);
            interp->pos++;
        }
        else if (t.type == TOK_FUNC) {
            interp->pos++;
            variable_t *v = &interp->scopes[interp->scope_depth].vars[interp->scopes[interp->scope_depth].var_count++];
            v->name = interp->tokens[interp->pos++].value;
            v->is_func = 1;
            interp->pos++;
            v->param_count = 0;
            v->param_names = NULL;
            while (interp->tokens[interp->pos].type != TOK_RPAREN) {
                v->param_names = realloc(v->param_names, sizeof(char *) * (v->param_count + 1));
                v->param_names[v->param_count++] = interp->tokens[interp->pos++].value;
                if (interp->tokens[interp->pos].type == TOK_COMMA) {
                    interp->pos++;
                }
            }
            interp->pos++;
            interp->pos++;
            v->func_start = interp->pos;
            while (interp->tokens[interp->pos].type != TOK_RBRACE) {
                interp->pos++;
            }
            interp->pos++;
        }
        else if (t.type == TOK_RETURN) {
            interp->pos++;
            interp->stack[interp->stack_ptr++] = eval_expression(interp);
            interp->pos++;
            while (interp->tokens[interp->pos].type != TOK_RBRACE) {
                interp->pos++;
            }
        }
        else if (t.type == TOK_WHILE) {
            int start_pos = interp->pos;
            interp->pos++;
            interp->pos++;
            int cond_pos = interp->pos;
            value_t cond = eval_expression(interp);
            interp->pos++;
            interp->pos++;
            push_scope(interp);
            while (cond.type == VAL_NUM && cond.data.num) {
                execute_statement(interp);
                interp->pos = cond_pos;
                cond = eval_expression(interp);
            }
            pop_scope(interp);
            interp->pos++;
        }
        else if (t.type == TOK_FOR) {
            interp->pos++;
            interp->pos++;
            push_scope(interp);
            execute_statement(interp);
            int cond_pos = interp->pos;
            value_t cond = eval_expression(interp);
            interp->pos++;
            int update_pos = interp->pos;
            while (interp->tokens[interp->pos].type != TOK_RPAREN) interp->pos++;
            interp->pos++;
            interp->pos++;
            while (cond.type == VAL_NUM && cond.data.num) {
                execute_statement(interp);
                int save_pos = interp->pos;
                interp->pos = update_pos;
                eval_expression(interp);
                interp->pos = cond_pos;
                cond = eval_expression(interp);
                interp->pos = save_pos;
            }
            pop_scope(interp);
            interp->pos++;
        }
        else if (t.type == TOK_IDENT && interp->tokens[interp->pos+1].type == TOK_EQ) {
            char *name = t.value;
            interp->pos += 2;
            *get_var(interp, name) = eval_expression(interp);
            interp->pos++;
        }
        else {
            eval_expression(interp);
            interp->pos++;
        }
    }
}

void execute(interpreter_t *interp, const char *code) {
    tokenize(interp, code);
    interp->pos = 0;
    execute_statement(interp);
}

int main() {
    interpreter_t interp;
    init_interpreter(&interp);

    const char *code =
        "console.log(\"Hello, world!\");\n";

    execute(&interp, code);

    return 0;
}
