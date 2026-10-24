"""Export evaluated high-poly geometry for the normal baker, without saving the blend."""
import sys
import struct
import bpy

output, object_name, scope, operation = sys.argv[sys.argv.index("--") + 1:]
if operation == "inspect":
    objects = [o.name for o in bpy.context.scene.objects if o.type == "MESH"]
    with open(output, "wb") as stream:
        stream.write(struct.pack("<I", len(objects)))
        for name in objects:
            data = name.encode("utf-8")
            stream.write(struct.pack("<I", len(data)))
            stream.write(data)
    # The native importer waits for an explicit selection when there are multiple meshes.
    sys.exit(0)
if object_name:
    obj = bpy.context.scene.objects.get(object_name)
    if obj is None or obj.type != "MESH":
        raise RuntimeError("High-poly mesh object not found: " + object_name)
    sources = [obj]
elif scope == "selected":
    sources = [o for o in bpy.context.selected_objects if o.type == "MESH"]
else:
    sources = [o for o in bpy.context.scene.objects if o.type == "MESH"]
if not sources:
    raise RuntimeError("No source meshes. Specify an object name or select meshes in Blender and save the file.")

# Work only on this temporary Blender process. Never change the source file.
def include_collection(layer):
    layer.exclude = False
    layer.hide_viewport = False
    for child in layer.children:
        include_collection(child)
include_collection(bpy.context.view_layer.layer_collection)
for obj in bpy.context.view_layer.objects:
    obj.select_set(False)
for obj in sources:
    obj.hide_set(False)
    obj.hide_viewport = False
    obj.select_set(True)
bpy.context.view_layer.objects.active = sources[0]
bpy.context.view_layer.update()
bpy.ops.wm.obj_export(
    filepath=output,
    check_existing=False,
    export_selected_objects=True,
    apply_modifiers=True,
    export_eval_mode="DAG_EVAL_RENDER",
    export_triangulated_mesh=True,
    export_uv=False,
    export_normals=True,
    export_materials=False,
    forward_axis="NEGATIVE_Z",
    up_axis="Y",
)
