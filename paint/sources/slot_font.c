slot_font_t *slot_font_create(char *name, draw_font_t *font, char *file) {
	slot_font_t *raw   = GC_ALLOC_INIT(slot_font_t, {0});
	raw->preview_ready = false;
	raw->id            = 0;

	for (i32 i = 0; i < project_fonts->length; ++i) {
		slot_font_t *slot = project_fonts->buffer[i];
		if (slot->id >= raw->id) {
			raw->id = slot->id + 1;
		}
	}

	raw->name = string_copy(name);
	raw->font = font;
	raw->file = string_copy(file);

	return raw;
}
