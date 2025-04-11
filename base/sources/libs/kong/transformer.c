#include "transformer.h"

#include "compiler.h"
#include "functions.h"
#include "types.h"

#include <assert.h>
#include <string.h>

opcodes new_code;

static void copy_opcode(opcode *o) {
	uint8_t *new_data = &new_code.o[new_code.size];

	assert(new_code.size + o->size < OPCODES_SIZE);

	uint8_t *location = &new_code.o[new_code.size];

	memcpy(&new_code.o[new_code.size], o, o->size);

	new_code.size += o->size;
}

void transform(uint32_t flags) {
	for (function_id i = 0; get_function(i) != NULL; ++i) {
		function *f = get_function(i);

		if (f->block == NULL) {
			continue;
		}

		uint8_t *data = f->code.o;
		size_t   size = f->code.size;

		new_code.size = 0;

		size_t index = 0;
		while (index < size) {
			opcode *o = (opcode *)&data[index];

			switch (o->type) {
			case OPCODE_STORE_ACCESS_LIST: {
				access a = o->op_store_access_list.access_list[o->op_store_access_list.access_list_size - 1];

				if (a.kind == ACCESS_SWIZZLE && a.access_swizzle.swizzle.size > 1) {
					for (uint32_t swizzle_index = 0; swizzle_index < a.access_swizzle.swizzle.size; ++swizzle_index) {
						assert(is_vector(o->op_store_access_list.from.type.type));

						type_id from_type = vector_base_type(o->op_store_access_list.from.type.type);

						type_ref t;
						init_type_ref(&t, NO_NAME);
						t.type = from_type;

						variable from = allocate_variable(t, o->op_store_access_list.from.kind);

						opcode from_opcode;
						from_opcode.type                                                               = OPCODE_LOAD_ACCESS_LIST;
						from_opcode.size                                                               = OP_SIZE(from_opcode, op_load_access_list);
						from_opcode.op_load_access_list.from                                           = o->op_store_access_list.from;
						from_opcode.op_load_access_list.to                                             = from;
						from_opcode.op_load_access_list.access_list_size                               = 1;
						from_opcode.op_load_access_list.access_list->kind                              = ACCESS_SWIZZLE;
						from_opcode.op_load_access_list.access_list->access_swizzle.swizzle.size       = 1;
						from_opcode.op_load_access_list.access_list->access_swizzle.swizzle.indices[0] = swizzle_index;
						from_opcode.op_load_access_list.access_list->type                              = from_type;

						copy_opcode(&from_opcode);

						opcode new_opcode = *o;

						new_opcode.op_store_access_list.from = from;

						access *new_access = &new_opcode.op_store_access_list.access_list[new_opcode.op_store_access_list.access_list_size - 1];
						new_access->access_swizzle.swizzle.size       = 1;
						new_access->access_swizzle.swizzle.indices[0] = a.access_swizzle.swizzle.indices[swizzle_index];
						new_access->type                              = from_type;

						copy_opcode(&new_opcode);
					}
				}
				else {
					copy_opcode(o);
				}
				break;
			}
			case OPCODE_LOAD_ACCESS_LIST: {
				access a = o->op_load_access_list.access_list[o->op_load_access_list.access_list_size - 1];

				if (a.kind == ACCESS_SWIZZLE && a.access_swizzle.swizzle.size > 1) {
					for (uint32_t swizzle_index = 0; swizzle_index < a.access_swizzle.swizzle.size; ++swizzle_index) {
						opcode new_opcode = *o;

						access *new_access = &new_opcode.op_load_access_list.access_list[new_opcode.op_load_access_list.access_list_size - 1];
						new_access->access_swizzle.swizzle.size       = 1;
						new_access->access_swizzle.swizzle.indices[0] = a.access_swizzle.swizzle.indices[swizzle_index];

						copy_opcode(&new_opcode);
					}
				}
				else {
					copy_opcode(o);
				}
				break;
			}
			default: {
				copy_opcode(o);
			}
			}

			index += o->size;
		}

		f->code = new_code;
	}
}
