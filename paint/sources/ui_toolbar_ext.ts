
function ui_toolbar_ext_draw_tools(ui: ui_t, img: gpu_texture_t, icon_accent: i32, keys: string[]) {

    ui_toolbar_draw_tool(workspace_tool_t.BRUSH, ui, img, icon_accent, keys);
    ui_toolbar_draw_tool(workspace_tool_t.ERASER, ui, img, icon_accent, keys);
    ui_toolbar_draw_tool(workspace_tool_t.FILL, ui, img, icon_accent, keys);
    ui_toolbar_draw_tool(workspace_tool_t.DECAL, ui, img, icon_accent, keys);
    ui_toolbar_draw_tool(workspace_tool_t.TEXT, ui, img, icon_accent, keys);
    ui_toolbar_draw_tool(workspace_tool_t.CLONE, ui, img, icon_accent, keys);
    ui_toolbar_draw_tool(workspace_tool_t.BLUR, ui, img, icon_accent, keys);
    ui_toolbar_draw_tool(workspace_tool_t.SMUDGE, ui, img, icon_accent, keys);
    ui_toolbar_draw_tool(workspace_tool_t.PARTICLE, ui, img, icon_accent, keys);
    ui_toolbar_draw_tool(workspace_tool_t.COLORID, ui, img, icon_accent, keys);
    ui_toolbar_draw_tool(workspace_tool_t.PICKER, ui, img, icon_accent, keys);
    ui_toolbar_draw_tool(workspace_tool_t.BAKE, ui, img, icon_accent, keys);
    ui_toolbar_draw_tool(workspace_tool_t.MATERIAL, ui, img, icon_accent, keys);
}
