#include <kinc/graphics5/indexbuffer.h>
#include <kinc/graphics5/commandlist.h>

extern kinc_g5_command_list_t commandList;

void kinc_g4_index_buffer_unlock_all(kinc_g5_index_buffer_t *buffer) {
	kinc_g5_index_buffer_unlock_all(buffer);
	kinc_g5_command_list_upload_index_buffer(&commandList, buffer);
}

void kinc_g4_index_buffer_unlock(kinc_g5_index_buffer_t *buffer, int count) {
	kinc_g5_index_buffer_unlock(buffer, count);
	kinc_g5_command_list_upload_index_buffer(&commandList, buffer);
}
