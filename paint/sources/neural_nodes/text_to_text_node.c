
#include "../global.h"

static char *text_to_text_node_prompt = NULL;

string_array_t *text_to_text_node_qwen_args(char *dir, char *prompt) {
	char *history_file = string("%s%shistory.txt", dir, PATH_SEP);
	char *full_prompt  = prompt;
	if (iron_file_exists(history_file)) {
		full_prompt = string("%s%s", sys_buffer_to_string(iron_load_blob(history_file)), prompt);
	}

	string_array_t *argv = any_array_create_from_raw(
	    (void *[]){
	        string("%s/%s", dir, neural_node_llama_bin()),
	        "-m",
	        string("%s/Qwen3.6-27B-Q4_K_M.gguf", dir),
	        "-ngl",
	        "99",
	        "-c",
	        "20000",
	        "--single-turn",
	        "--prompt-cache",
	        string("%s/context.bin", dir),
	        "--prompt-cache-all",
	        "--prompt",
	        string("'%s'", full_prompt),
	        NULL,
	    },
	    14);
	return argv;
}

void text_to_text_node_check_result(void (*done)(char *)) {
	iron_delay_idle_sleep();
	if (iron_exec_async_done == 1) {
		char *file = string("%s%soutput.txt", neural_node_dir(), PATH_SEP);
		if (iron_file_exists(file)) {
			buffer_t *b = iron_load_blob(file);
			char     *s = sys_buffer_to_string(b);
			s           = substring(s, string_last_index_of(s, "</think>\n\n") + 10, string_last_index_of(s, "[end of text]"));

			char *history_file = string("%s%shistory.txt", neural_node_dir(), PATH_SEP);
			char *existing     = iron_file_exists(history_file) ? sys_buffer_to_string(iron_load_blob(history_file)) : "";
			char *entry        = string("%s%s\n%s\n\n", existing, text_to_text_node_prompt, s);
			iron_file_save_bytes(history_file, (buffer_t *)u8_array_create_from_string(entry), 0);

			done(s);
			base_redraw_console();
		}
		sys_remove_update(text_to_text_node_check_result);
	}
}

void text_to_text_node_clear(void) {
	char *dir = neural_node_dir();
	iron_delete_file(string("%s%shistory.txt", dir, PATH_SEP));
	iron_delete_file(string("%s%scontext.bin", dir, PATH_SEP));
}

void text_to_text_node_run(char *prompt, void (*done)(char *)) {
	char *dir                   = neural_node_dir();
	text_to_text_node_prompt    = prompt;
	iron_exec_async_output_file = string("%s%soutput.txt", dir, PATH_SEP);
	string_array_t *argv        = text_to_text_node_qwen_args(dir, prompt);
	iron_exec_async(argv->buffer[0], argv->buffer);
	sys_notify_on_update(text_to_text_node_check_result, done);
}
