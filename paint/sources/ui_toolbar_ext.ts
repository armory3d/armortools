
function ui_toolbar_ext_draw_tools(ui: ui_t, img: gpu_texture_t, icon_accent: i32, keys: string[]) {

    ui_toolbar_draw_tool(tool_type_t.BRUSH, ui, img, icon_accent, keys);
    ui_toolbar_draw_tool(tool_type_t.ERASER, ui, img, icon_accent, keys);
    ui_toolbar_draw_tool(tool_type_t.FILL, ui, img, icon_accent, keys);
    ui_toolbar_draw_tool(tool_type_t.DECAL, ui, img, icon_accent, keys);
    ui_toolbar_draw_tool(tool_type_t.TEXT, ui, img, icon_accent, keys);
    ui_toolbar_draw_tool(tool_type_t.CLONE, ui, img, icon_accent, keys);
    ui_toolbar_draw_tool(tool_type_t.BLUR, ui, img, icon_accent, keys);
    ui_toolbar_draw_tool(tool_type_t.SMUDGE, ui, img, icon_accent, keys);
    ui_toolbar_draw_tool(tool_type_t.PARTICLE, ui, img, icon_accent, keys);
    ui_toolbar_draw_tool(tool_type_t.COLORID, ui, img, icon_accent, keys);
    ui_toolbar_draw_tool(tool_type_t.PICKER, ui, img, icon_accent, keys);
    ui_toolbar_draw_tool(tool_type_t.BAKE, ui, img, icon_accent, keys);
    ui_toolbar_draw_tool(tool_type_t.MATERIAL, ui, img, icon_accent, keys);
}
