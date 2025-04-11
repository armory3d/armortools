#include "disasm.h"

#include "compiler.h"
#include "errors.h"
#include "functions.h"
#include "log.h"
#include "parser.h"
#include "sets.h"
#include "shader_stage.h"
#include "types.h"

#include <assert.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*static char *type_string(type_id type) {
    if (type == float_id) {
        return "float";
    }
    if (type == float2_id) {
        return "float2";
    }
    if (type == float3_id) {
        return "float3";
    }
    if (type == float4_id) {
        return "float4";
    }
    if (type == float4x4_id) {
        return "float4x4";
    }
    if (type == ray_type_id) {
        return "RayDesc";
    }
    if (type == bvh_type_id) {
        return "RaytracingAccelerationStructure";
    }
    if (type == tex2d_type_id) {
        return "Texture2D<float4>";
    }
    return get_name(get_type(type)->name);
}*/

static void write_functions(void) {
	for (function_id i = 0; get_function(i) != NULL; ++i) {
		function *f = get_function(i);

		if (f->block == NULL) {
			continue;
		}

		kong_log(LOG_LEVEL_INFO, "Function: %s", get_name(f->name));

		uint8_t *data = f->code.o;
		size_t   size = f->code.size;

		size_t index = 0;
		while (index < size) {
			opcode *o = (opcode *)&data[index];
			switch (o->type) {
			case OPCODE_RETURN:
				kong_log(LOG_LEVEL_INFO, "RETURN $%zu", o->op_return.var.index);
				break;
			case OPCODE_LOAD_ACCESS_LIST: {
				char accesses[1024];
				int  offset = 0;

				for (int i = 0; i < o->op_load_access_list.access_list_size; ++i) {
					switch (o->op_load_access_list.access_list[i].kind) {
					case ACCESS_ELEMENT:
						offset += sprintf(&accesses[offset], "$%" PRIu64, o->op_load_access_list.access_list[i].access_element.index.index);
						break;
					case ACCESS_MEMBER:
						offset += sprintf(&accesses[offset], "%s", get_name(o->op_load_access_list.access_list[i].access_member.name));
						break;
					case ACCESS_SWIZZLE: {
						swizzle swizzle          = o->op_load_access_list.access_list[i].access_swizzle.swizzle;
						char    swizzle_chars[4] = "xyzw";

						for (uint32_t swizzle_index = 0; swizzle_index < swizzle.size; ++swizzle_index) {
							offset += sprintf(&accesses[offset], "%c", swizzle_chars[swizzle.indices[swizzle_index]]);
						}
						break;
					}
					}

					if (i < o->op_load_access_list.access_list_size - 1) {
						offset += sprintf(&accesses[offset], ", ");
					}
				}

				kong_log(LOG_LEVEL_INFO, "$%zu = LOAD_ACCESS_LIST $%zu[%s]", o->op_load_access_list.to.index, o->op_load_access_list.from.index, accesses);
				break;
			}
			case OPCODE_CALL: {
				char parameters[256];
				int  offset = 0;

				for (int i = 0; i < o->op_call.parameters_size; ++i) {
					offset += sprintf(&parameters[offset], "$%" PRIu64, o->op_call.parameters[i].index);

					if (i < o->op_call.parameters_size - 1) {
						offset += sprintf(&parameters[offset], ", ");
					}
				}

				kong_log(LOG_LEVEL_INFO, "$%zu = CALL %s(%s)", o->op_call.var.index, get_name(o->op_call.func), parameters);
				break;
			}
			case OPCODE_VAR:
				break;
			case OPCODE_NOT:
				break;
			case OPCODE_STORE_VARIABLE:
				kong_log(LOG_LEVEL_INFO, "$%zu = STORE_VARIABLE $%zu", o->op_store_var.to.index, o->op_store_var.from.index);
				break;
			case OPCODE_SUB_AND_STORE_VARIABLE:
				break;
			case OPCODE_ADD_AND_STORE_VARIABLE:
				break;
			case OPCODE_DIVIDE_AND_STORE_VARIABLE:
				break;
			case OPCODE_MULTIPLY_AND_STORE_VARIABLE:
				break;
			case OPCODE_STORE_ACCESS_LIST: {
				char accesses[1024];
				int  offset = 0;

				for (int i = 0; i < o->op_store_access_list.access_list_size; ++i) {
					switch (o->op_store_access_list.access_list[i].kind) {
					case ACCESS_ELEMENT:
						offset += sprintf(&accesses[offset], "$%" PRIu64, o->op_store_access_list.access_list[i].access_element.index.index);
						break;
					case ACCESS_MEMBER:
						offset += sprintf(&accesses[offset], "%s", get_name(o->op_store_access_list.access_list[i].access_member.name));
						break;
					case ACCESS_SWIZZLE: {
						swizzle swizzle          = o->op_store_access_list.access_list[i].access_swizzle.swizzle;
						char    swizzle_chars[4] = "xyzw";

						for (uint32_t swizzle_index = 0; swizzle_index < swizzle.size; ++swizzle_index) {
							offset += sprintf(&accesses[offset], "%c", swizzle_chars[swizzle.indices[swizzle_index]]);
						}
						break;
					}
					}

					if (i < o->op_store_access_list.access_list_size - 1) {
						offset += sprintf(&accesses[offset], ", ");
					}
				}

				kong_log(LOG_LEVEL_INFO, "$%zu[%s] = STORE_ACCESS_LIST $%zu", o->op_store_access_list.to.index, accesses, o->op_store_access_list.from.index);
				break;
			}
			case OPCODE_SUB_AND_STORE_ACCESS_LIST:
				break;
			case OPCODE_ADD_AND_STORE_ACCESS_LIST:
				break;
			case OPCODE_DIVIDE_AND_STORE_ACCESS_LIST:
				break;
			case OPCODE_MULTIPLY_AND_STORE_ACCESS_LIST:
				break;
			case OPCODE_LOAD_FLOAT_CONSTANT:
				kong_log(LOG_LEVEL_INFO, "$%zu = LOAD_FLOAT_CONSTANT %f", o->op_load_float_constant.to.index, o->op_load_float_constant.number);
				break;
			case OPCODE_LOAD_INT_CONSTANT:
				break;
			case OPCODE_LOAD_BOOL_CONSTANT:
				break;
			case OPCODE_ADD:
				break;
			case OPCODE_SUB:
				break;
			case OPCODE_MULTIPLY:
				kong_log(LOG_LEVEL_INFO, "$%zu = MULTIPLY $%zu, $%zu", o->op_binary.result.index, o->op_binary.left.index, o->op_binary.right.index);
				break;
			case OPCODE_DIVIDE:
				break;
			case OPCODE_MOD:
				break;
			case OPCODE_EQUALS:
				break;
			case OPCODE_NOT_EQUALS:
				break;
			case OPCODE_GREATER:
				break;
			case OPCODE_GREATER_EQUAL:
				break;
			case OPCODE_LESS:
				break;
			case OPCODE_LESS_EQUAL:
				break;
			case OPCODE_AND:
				break;
			case OPCODE_OR:
				break;
			case OPCODE_BITWISE_XOR:
				break;
			case OPCODE_BITWISE_AND:
				break;
			case OPCODE_BITWISE_OR:
				break;
			case OPCODE_LEFT_SHIFT:
				break;
			case OPCODE_RIGHT_SHIFT:
				break;
			case OPCODE_IF:
				break;
			case OPCODE_WHILE_START:
				break;
			case OPCODE_WHILE_CONDITION:
				break;
			case OPCODE_WHILE_END:
				break;
			case OPCODE_BLOCK_START:
				break;
			case OPCODE_BLOCK_END:
				break;
			default: {
				debug_context context = {0};
				error(context, "Unknown opcode");
				break;
			}
			}
			index += o->size;
		}

		kong_log(LOG_LEVEL_INFO, "");
	}
}

void disassemble(void) {
	write_functions();
}
