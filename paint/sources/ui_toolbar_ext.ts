
function ui_toolbar_ext_draw_tools(img: gpu_texture_t, icon_accent: i32, keys: string[]) {
	ui_toolbar_draw_tool(tool_type_t.BRUSH, img, icon_accent, keys);
	ui_toolbar_draw_tool(tool_type_t.ERASER, img, icon_accent, keys);
	ui_toolbar_draw_tool(tool_type_t.FILL, img, icon_accent, keys);
	ui_toolbar_draw_tool(tool_type_t.DECAL, img, icon_accent, keys);
	ui_toolbar_draw_tool(tool_type_t.TEXT, img, icon_accent, keys);
	ui_toolbar_draw_tool(tool_type_t.CLONE, img, icon_accent, keys);
	ui_toolbar_draw_tool(tool_type_t.BLUR, img, icon_accent, keys);
	ui_toolbar_draw_tool(tool_type_t.SMUDGE, img, icon_accent, keys);
	ui_toolbar_draw_tool(tool_type_t.PARTICLE, img, icon_accent, keys);
	ui_toolbar_draw_tool(tool_type_t.COLORID, img, icon_accent, keys);
	ui_toolbar_draw_tool(tool_type_t.PICKER, img, icon_accent, keys);
	ui_toolbar_draw_tool(tool_type_t.BAKE, img, icon_accent, keys);
	ui_toolbar_draw_tool(tool_type_t.MATERIAL, img, icon_accent, keys);
	ui_toolbar_draw_tool(tool_type_t.GIZMO, img, icon_accent, keys);
}
