#include "global.h"

void next_frame() {

	for (int i = 0; i < 14; ++i) {
		context_select_tool(i);
	}

	export_texture_run("/tmp", false);
	project_filepath_set("/tmp/untitled.arm");
	project_save(true);

	console_info("PASS");
}

void main() {
	script_notify_on_next_frame(next_frame);
}
