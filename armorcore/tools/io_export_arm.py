"""Armory Mesh Exporter"""
#
#  https://github.com/armory3d/armorcore/blob/main/Tools/io_export_arm.py
#
#  Based on Open Game Engine Exchange
#  https://opengex.org/
#  Export plugin for Blender by Eric Lengyel
#  Copyright 2015, Terathon Software LLC
#
#  This software is licensed under the Creative Commons
#  Attribution-ShareAlike 3.0 Unported License:
#  http://creativecommons.org/licenses/by-sa/3.0/deed.en_US

import io
import os
import struct
import time
import bpy
from bpy_extras.io_utils import ExportHelper
from mathutils import Vector
import numpy as np

bl_info = {
    "name": "Armory Mesh Exporter",
    "category": "Import-Export",
    "location": "File -> Export",
    "description": "Armory mesh data",
    "author": "Armory3D.org",
    "version": (2024, 3, 0),
    "blender": (3, 6, 0),
    "doc_url": "",
    "tracker_url": "",
}

NodeTypeBone = 1
NodeTypeMesh = 2
structIdentifier = ["object", "bone_object", "mesh_object"]

class ArmoryExporter(bpy.types.Operator, ExportHelper):
    """Export to Armory format"""

    bl_idname = "export_scene.arm"
    bl_label = "Export Armory"
    filename_ext = ".arm"

    def execute(self, context):
        profile_time = time.time()
        current_frame = context.scene.frame_current
        current_subframe = context.scene.frame_subframe
        self.scene = context.scene
        self.output = {}
        self.bobjectArray = {}
        self.bobjectBoneArray = {}
        self.meshArray = {}
        self.boneParentArray = {}
        self.bone_tracks = []
        self.depsgraph = context.evaluated_depsgraph_get()
        scene_objects = self.scene.collection.all_objects

        for bobject in scene_objects:
            if not bobject.parent:
                self.process_bobject(bobject)

        self.process_skinned_meshes()

        self.output["name"] = self.scene.name
        self.output["objects"] = []
        for bo in scene_objects:
            if not bo.parent:
                self.export_object(bo, self.scene)

        self.output["mesh_datas"] = []
        for o in self.meshArray.items():
            self.export_mesh(o)

        self.output["camera_datas"] = None
        self.output["camera_ref"] = None
        self.output["material_datas"] = None
        self.output["shader_datas"] = None
        self.output["world_datas"] = None
        self.output["world_ref"] = None
        self.output["speaker_datas"] = None
        self.output["embedded_datas"] = None

        self.write_arm(self.filepath, self.output)
        self.scene.frame_set(current_frame, subframe=current_subframe)

        print(f"Scene exported in {str(time.time() - profile_time)}")

        return {"FINISHED"}

    def write_arm(self, filepath, output):
        with open(filepath, "wb") as f:
            f.write(packb(output))

    def write_matrix(self, matrix):
        return [
            matrix[0][0],
            matrix[0][1],
            matrix[0][2],
            matrix[0][3],
            matrix[1][0],
            matrix[1][1],
            matrix[1][2],
            matrix[1][3],
            matrix[2][0],
            matrix[2][1],
            matrix[2][2],
            matrix[2][3],
            matrix[3][0],
            matrix[3][1],
            matrix[3][2],
            matrix[3][3],
        ]

    def find_bone(self, name):
        return next(
            (
                bobject_ref
                for bobject_ref in self.bobjectBoneArray.items()
                if bobject_ref[0].name == name
            ),
            None,
        )

    def collect_bone_animation(self, armature, name):
        path = 'pose.bones["' + name + '"].'
        curve_array = []
        if armature.animation_data:
            if action := armature.animation_data.action:
                curve_array.extend(
                    fcurve
                    for fcurve in action.fcurves
                    if fcurve.data_path.startswith(path)
                )
        return curve_array

    def export_bone(self, armature, bone, scene, o, action):
        if bobjectRef := self.bobjectBoneArray.get(bone):
            o["name"] = bobjectRef["structName"]
            o["type"] = structIdentifier[bobjectRef["objectType"]]
            self.export_bone_transform(armature, bone, o, action)
        o["children"] = []
        for subbobject in bone.children:
            so = {}
            self.export_bone(armature, subbobject, scene, so, action)
            o["children"].append(so)

    def export_pose_markers(self, oanim, action):
        if action.pose_markers is None or len(action.pose_markers) == 0:
            return
        oanim["marker_frames"] = []
        oanim["marker_names"] = []
        for m in action.pose_markers:
            oanim["marker_frames"].append(int(m.frame))
            oanim["marker_names"].append(m.name)

    def process_bone(self, bone):
        self.bobjectBoneArray[bone] = {
            "objectType": NodeTypeBone,
            "structName": bone.name,
        }
        for subbobject in bone.children:
            self.process_bone(subbobject)

    def process_bobject(self, bobject):
        if bobject.type not in ["MESH", "ARMATURE"]:
            return

        btype = NodeTypeMesh if bobject.type == "MESH" else 0
        self.bobjectArray[bobject] = {"objectType": btype, "structName": bobject.name}

        if bobject.type == "ARMATURE":
            if skeleton := bobject.data:
                for bone in skeleton.bones:
                    if not bone.parent:
                        self.process_bone(bone)

        for subbobject in bobject.children:
            self.process_bobject(subbobject)

    def process_skinned_meshes(self):
        for bobjectRef in self.bobjectArray.items():
            if bobjectRef[1]["objectType"] == NodeTypeMesh:
                if armature := bobjectRef[0].find_armature():
                    for bone in armature.data.bones:
                        boneRef = self.find_bone(bone.name)
                        if boneRef:
                            boneRef[1]["objectType"] = NodeTypeBone

    def export_bone_transform(self, armature, bone, o, action):
        pose_bone = armature.pose.bones.get(bone.name)
        transform = bone.matrix_local.copy()
        if bone.parent is not None:
            transform = bone.parent.matrix_local.inverted_safe() @ transform

        o["transform"] = {}
        o["transform"]["values"] = self.write_matrix(transform)

        curve_array = self.collect_bone_animation(armature, bone.name)
        animation = len(curve_array) != 0

        if animation and pose_bone:
            begin_frame = int(action.frame_range[0])
            end_frame = int(action.frame_range[1])
            tracko = {}
            o["anim"] = {}
            o["anim"]["tracks"] = [tracko]
            tracko["target"] = "transform"
            tracko["frames"] = [
                i - begin_frame for i in range(begin_frame, end_frame + 1)
            ]
            tracko["values"] = []
            self.bone_tracks.append((tracko["values"], pose_bone))

    def write_bone_matrices(self, scene, action):
        if len(self.bone_tracks) > 0:
            begin_frame = int(action.frame_range[0])
            end_frame = int(action.frame_range[1])
            for i in range(begin_frame, end_frame + 1):
                scene.frame_set(i)
                for track in self.bone_tracks:
                    values, pose_bone = track[0], track[1]
                    if parent := pose_bone.parent:
                        values += self.write_matrix(
                            (parent.matrix.inverted_safe() @ pose_bone.matrix)
                        )
                    else:
                        values += self.write_matrix(pose_bone.matrix)

    def export_object(self, bobject, scene, parento=None):
        if bobjectRef := self.bobjectArray.get(bobject):
            o = {}
            o["name"] = bobjectRef["structName"]
            o["type"] = structIdentifier[bobjectRef["objectType"]]
            o["data_ref"] = None
            o["transform"] = self.write_matrix(bobject.matrix_local)
            o["dimensions"] = None
            o["visible"] = True
            o["spawn"] = True
            o["anim"] = None
            o["material_refs"] = None
            o["children"] = None

            if bobject.parent_type == "BONE":
                o["anim"]["parent_bone"] = bobject.parent_bone

            if bobjectRef["objectType"] == NodeTypeMesh:
                objref = bobject.data
                if objref not in self.meshArray:
                    self.meshArray[objref] = {
                        "structName": objref.name,
                        "objectTable": [bobject],
                    }
                else:
                    self.meshArray[objref]["objectTable"].append(bobject)
                oid = self.meshArray[objref]["structName"]
                o["data_ref"] = oid
                o["dimensions"] = self.calc_aabb(bobject)

            # If the object is parented to a bone and is not relative, undo the
            # bone's transform
            if bobject.parent_type == "BONE":
                armature = bobject.parent.data
                bone = armature.bones[bobject.parent_bone]
                o["anim"]["parent_bone_connected"] = bone.use_connect
                if bone.use_connect:
                    bone_translation = Vector((0, bone.length, 0)) + bone.head
                    o["anim"]["parent_bone_tail"] = [
                        bone_translation[0],
                        bone_translation[1],
                        bone_translation[2],
                    ]
                else:
                    bone_translation = bone.tail - bone.head
                    o["anim"]["parent_bone_tail"] = [
                        bone_translation[0],
                        bone_translation[1],
                        bone_translation[2],
                    ]
                    pose_bone = bobject.parent.pose.bones[bobject.parent_bone]
                    bone_translation_pose = pose_bone.tail - pose_bone.head
                    o["anim"]["parent_bone_tail_pose"] = [
                        bone_translation_pose[0],
                        bone_translation_pose[1],
                        bone_translation_pose[2],
                    ]

            if bobject.type == "ARMATURE" and bobject.data is not None:
                bdata = bobject.data
                action = None
                adata = bobject.animation_data

                # Active action
                if adata is not None:
                    action = adata.action
                if action is None:
                    bobject.animation_data_create()
                    actions = bpy.data.actions
                    action = actions.get("armory_pose")
                    if action is None:
                        action = actions.new(name="armory_pose")

                # Collect export actions
                export_actions = [action]
                if hasattr(adata, "nla_tracks") and adata.nla_tracks is not None:
                    for track in adata.nla_tracks:
                        if track.strips is None:
                            continue
                        for strip in track.strips:
                            if strip.action is None:
                                continue
                            if strip.action.name == action.name:
                                continue
                            export_actions.append(strip.action)

                basename = os.path.basename(self.filepath)[:-4]
                o["anim"]["bone_actions"] = []
                for action in export_actions:
                    o["anim"]["bone_actions"].append(basename + "_" + action.name)

                orig_action = bobject.animation_data.action
                for action in export_actions:
                    bobject.animation_data.action = action
                    bones = []
                    self.bone_tracks = []
                    for bone in bdata.bones:
                        if not bone.parent:
                            boneo = {}
                            self.export_bone(bobject, bone, scene, boneo, action)
                            bones.append(boneo)
                    self.write_bone_matrices(scene, action)
                    if len(bones) > 0 and "anim" in bones[0]:
                        self.export_pose_markers(bones[0]["anim"], action)
                    # Save action separately
                    action_obj = {}
                    action_obj["name"] = action.name
                    action_obj["objects"] = bones
                    self.write_arm(
                        self.filepath[:-4] + "_" + action.name + ".arm", action_obj
                    )

                bobject.animation_data.action = orig_action

            if parento is None:
                self.output["objects"].append(o)
            else:
                parento["children"].append(o)

            if not hasattr(o, "children") and len(bobject.children) > 0:
                o["children"] = []

        for subbobject in bobject.children:
            self.export_object(subbobject, scene, o)

    def export_skin(self, bobject, armature, exportMesh, o):
        # This function exports all skinning data, which includes the skeleton
        # and per-vertex bone influence data
        oskin = {}
        o["skin"] = oskin

        # Write the skin bind pose transform
        otrans = {}
        oskin["transform"] = otrans
        otrans["values"] = self.write_matrix(bobject.matrix_world)

        bone_array = armature.data.bones
        bone_count = len(bone_array)
        max_bones = 128
        bone_count = min(bone_count, max_bones)

        # Write the bone object reference array
        oskin["bone_ref_array"] = np.empty(bone_count, dtype=object)
        oskin["bone_len_array"] = np.empty(bone_count, dtype="<f4")

        for i in range(bone_count):
            if boneRef := self.find_bone(bone_array[i].name):
                oskin["bone_ref_array"][i] = boneRef[1]["structName"]
                oskin["bone_len_array"][i] = bone_array[i].length
            else:
                oskin["bone_ref_array"][i] = ""
                oskin["bone_len_array"][i] = 0.0

        # Write the bind pose transform array
        oskin["transforms_inv"] = []
        for i in range(bone_count):
            skeleton_inv = (
                armature.matrix_world @ bone_array[i].matrix_local
            ).inverted_safe()
            skeleton_inv = skeleton_inv @ bobject.matrix_world
            oskin["transforms_inv"].append(self.write_matrix(skeleton_inv))

        # Export the per-vertex bone influence data
        group_remap = []
        for group in bobject.vertex_groups:
            for i in range(bone_count):
                if bone_array[i].name == group.name:
                    group_remap.append(i)
                    break
            else:
                group_remap.append(-1)

        bone_count_array = np.empty(len(exportMesh.loops), dtype="<i2")
        bone_index_array = np.empty(len(exportMesh.loops) * 4, dtype="<i2")
        bone_weight_array = np.empty(len(exportMesh.loops) * 4, dtype="<f4")

        vertices = bobject.data.vertices
        count = 0
        for index, l in enumerate(exportMesh.loops):
            bone_count = 0
            total_weight = 0.0
            bone_values = []
            for g in vertices[l.vertex_index].groups:
                bone_index = group_remap[g.group]
                bone_weight = g.weight
                if bone_index >= 0:  # and bone_weight != 0.0:
                    bone_values.append((bone_weight, bone_index))
                    total_weight += bone_weight
                    bone_count += 1

            if bone_count > 4:
                bone_count = 4
                bone_values.sort(reverse=True)
                bone_values = bone_values[:4]

            bone_count_array[index] = bone_count
            for bv in bone_values:
                bone_weight_array[count] = bv[0]
                bone_index_array[count] = bv[1]
                count += 1

            if total_weight not in (0.0, 1.0):
                normalizer = 1.0 / total_weight
                for i in range(bone_count):
                    bone_weight_array[count - i - 1] *= normalizer

        bone_index_array = bone_index_array[:count]
        bone_weight_array = bone_weight_array[:count]
        bone_weight_array *= 32767
        bone_weight_array = np.array(bone_weight_array, dtype="<i2")

        oskin["bone_count_array"] = bone_count_array
        oskin["bone_index_array"] = bone_index_array
        oskin["bone_weight_array"] = bone_weight_array

    def calc_aabb(self, bobject):
        aabb_center = 0.125 * sum((Vector(b) for b in bobject.bound_box), Vector())
        return [
            abs(
                (bobject.bound_box[6][0] - bobject.bound_box[0][0]) / 2
                + abs(aabb_center[0])
            )
            * 2,
            abs(
                (bobject.bound_box[6][1] - bobject.bound_box[0][1]) / 2
                + abs(aabb_center[1])
            )
            * 2,
            abs(
                (bobject.bound_box[6][2] - bobject.bound_box[0][2]) / 2
                + abs(aabb_center[2])
            )
            * 2,
        ]

    def export_mesh_data(self, exportMesh, bobject, o, has_armature=False):
        exportMesh.calc_normals_split()
        exportMesh.calc_loop_triangles()

        loops = exportMesh.loops
        num_verts = len(loops)
        num_uv_layers = len(exportMesh.uv_layers)
        num_colors = len(exportMesh.vertex_colors)
        has_tex = num_uv_layers > 0
        has_tex1 = num_uv_layers > 1
        has_col = num_colors > 0
        has_tang = False

        # Scale for packed coords
        aabb = self.calc_aabb(bobject)
        maxdim = max(aabb[0], max(aabb[1], aabb[2]))
        if maxdim > 2:
            o["scale_pos"] = maxdim / 2
        else:
            o["scale_pos"] = 1.0
        if has_armature:  # Allow up to 2x bigger bounds for skinned mesh
            o["scale_pos"] *= 2.0

        pdata = np.empty(num_verts * 4, dtype="<f4")  # p.xyz, n.z
        ndata = np.empty(num_verts * 2, dtype="<f4")  # n.xy
        if has_tex:
            t0map = 0  # Get active uvmap
            t0data = np.empty(num_verts * 2, dtype="<f4")
            uv_layers = exportMesh.uv_layers
            if uv_layers is not None:
                for i in range(0, len(uv_layers)):
                    if uv_layers[i].active_render:
                        t0map = i
                        break
            if has_tex1:
                t1map = 1 if t0map == 0 else 0
                t1data = np.empty(num_verts * 2, dtype="<f4")
            # Scale for packed coords
            maxdim = 1.0
            lay0 = uv_layers[t0map]
            for v in lay0.data:
                if abs(v.uv[0]) > maxdim:
                    maxdim = abs(v.uv[0])
                if abs(v.uv[1]) > maxdim:
                    maxdim = abs(v.uv[1])
            if has_tex1:
                lay1 = uv_layers[t1map]
                for v in lay1.data:
                    if abs(v.uv[0]) > maxdim:
                        maxdim = abs(v.uv[0])
                    if abs(v.uv[1]) > maxdim:
                        maxdim = abs(v.uv[1])
            if maxdim > 1:
                o["scale_tex"] = maxdim
                invscale_tex = (1 / o["scale_tex"]) * 32767
            else:
                o["scale_tex"] = 1.0
                invscale_tex = 1 * 32767
            if has_tang:
                exportMesh.calc_tangents(uvmap=lay0.name)
                tangdata = np.empty(num_verts * 4, dtype="<f4")
        if has_col:
            cdata = np.empty(num_verts * 4, dtype="<f4")

        o["skin"] = None

        scale_pos = o["scale_pos"]
        invscale_pos = (1 / scale_pos) * 32767

        verts = exportMesh.vertices
        if has_tex:
            lay0 = exportMesh.uv_layers[t0map]
            if has_tex1:
                lay1 = exportMesh.uv_layers[t1map]
        if has_col:
            vcol0 = exportMesh.vertex_colors[0].data

        for i, loop in enumerate(loops):
            v = verts[loop.vertex_index]
            co = v.co
            normal = loop.normal
            tang = loop.tangent

            i4 = i * 4
            i2 = i * 2
            pdata[i4] = co[0]
            pdata[i4 + 1] = co[1]
            pdata[i4 + 2] = co[2]
            pdata[i4 + 3] = normal[2] * scale_pos  # Cancel scale
            ndata[i2] = normal[0]
            ndata[i2 + 1] = normal[1]
            if has_tex:
                uv = lay0.data[loop.index].uv
                t0data[i2] = uv[0]
                t0data[i2 + 1] = 1.0 - uv[1]  # Reverse Y
                if has_tex1:
                    uv = lay1.data[loop.index].uv
                    t1data[i2] = uv[0]
                    t1data[i2 + 1] = 1.0 - uv[1]
                if has_tang:
                    i4 = i * 4
                    tangdata[i4] = tang[0]
                    tangdata[i4 + 1] = tang[1]
                    tangdata[i4 + 2] = tang[2]
            if has_col:
                col = vcol0[loop.index].color
                i4 = i * 4
                cdata[i4] = col[0]
                cdata[i4 + 1] = col[1]
                cdata[i4 + 2] = col[2]
                cdata[i4 + 3] = col[3]

        # Pack
        pdata *= invscale_pos
        ndata *= 32767
        pdata = np.array(pdata, dtype="<i2")
        ndata = np.array(ndata, dtype="<i2")
        if has_tex:
            t0data *= invscale_tex
            t0data = np.array(t0data, dtype="<i2")
            if has_tex1:
                t1data *= invscale_tex
                t1data = np.array(t1data, dtype="<i2")
        if has_col:
            cdata *= 32767
            cdata = np.array(cdata, dtype="<i2")
        if has_tang:
            tangdata *= 32767
            tangdata = np.array(tangdata, dtype="<i2")

        # Output
        o["vertex_arrays"] = []
        o["vertex_arrays"].append(
            {"attrib": "pos", "data": "short4norm", "values": pdata}
        )
        o["vertex_arrays"].append(
            {"attrib": "nor", "data": "short2norm", "values": ndata}
        )
        if has_tex:
            o["vertex_arrays"].append(
                {"attrib": "tex", "data": "short2norm", "values": t0data}
            )
            if has_tex1:
                o["vertex_arrays"].append(
                    {"attrib": "tex1", "data": "short2norm", "values": t1data}
                )
        if has_col:
            o["vertex_arrays"].append(
                {"attrib": "col", "data": "short4norm", "values": cdata}
            )
        if has_tang:
            o["vertex_arrays"].append(
                {
                    "attrib": "tang",
                    "data": "short4norm",
                    "values": tangdata,
                }
            )

        mats = exportMesh.materials
        poly_map = []
        for i in range(max(len(mats), 1)):
            poly_map.append([])
        for poly in exportMesh.polygons:
            poly_map[poly.material_index].append(poly)

        o["index_arrays"] = []

        # map polygon indices to triangle loops
        tri_loops = {}
        for loop in exportMesh.loop_triangles:
            if loop.polygon_index not in tri_loops:
                tri_loops[loop.polygon_index] = []
            tri_loops[loop.polygon_index].append(loop)

        for index, polys in enumerate(poly_map):
            tris = 0
            for poly in polys:
                tris += poly.loop_total - 2
            if tris == 0:  # No face assigned
                continue
            prim = np.empty(tris * 3, dtype="<i4")

            i = 0
            for poly in polys:
                for loop in tri_loops[poly.index]:
                    prim[i] = loops[loop.loops[0]].index
                    prim[i + 1] = loops[loop.loops[1]].index
                    prim[i + 2] = loops[loop.loops[2]].index
                    i += 3

            ia = {}
            ia["material"] = 0
            if len(mats) > 1:
                for i in range(len(mats)):  # Multi-mat mesh
                    if mats[i] == mats[index]:  # Default material for empty slots
                        ia["material"] = i
                        break
            ia["values"] = prim
            o["index_arrays"].append(ia)

    def export_mesh(self, objectRef):
        # This function exports a single mesh object
        table = objectRef[1]["objectTable"]
        bobject = table[0]
        oid = objectRef[1]["structName"]
        o = {}
        o["name"] = oid

        armature = bobject.find_armature()
        apply_modifiers = not armature

        bobject_eval = (
            bobject.evaluated_get(self.depsgraph) if apply_modifiers else bobject
        )
        exportMesh = bobject_eval.to_mesh()

        self.export_mesh_data(exportMesh, bobject, o, has_armature=armature is not None)
        if armature:
            self.export_skin(bobject, armature, exportMesh, o)

        self.output["mesh_datas"].append(o)
        bobject_eval.to_mesh_clear()

def menu_func(self, context):
    self.layout.operator(ArmoryExporter.bl_idname, text="Armory (.arm)")

def register():
    bpy.utils.register_class(ArmoryExporter)
    bpy.types.TOPBAR_MT_file_export.append(menu_func)

def unregister():
    bpy.types.TOPBAR_MT_file_export.remove(menu_func)
    bpy.utils.unregister_class(ArmoryExporter)

if __name__ == "__main__":
    register()

# Msgpack parser with typed arrays
# Based on u-msgpack-python v2.4.1 - v at sergeev.io
# https://github.com/vsergeev/u-msgpack-python
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

def _pack_integer(obj, fp):
    fp.write(b"\xd2" + struct.pack("<i", obj))

def _pack_nil(fp):
    fp.write(b"\xc0")

def _pack_boolean(obj, fp):
    fp.write(b"\xc3" if obj else b"\xc2")

def _pack_float(obj, fp):
    fp.write(b"\xca" + struct.pack("<f", obj))

def _pack_string(obj, fp):
    obj = obj.encode("utf-8")
    fp.write(b"\xdb" + struct.pack("<I", len(obj)) + obj)

def _pack_binary(obj, fp):
    fp.write(b"\xc6" + struct.pack("<I", len(obj)) + obj)

def _pack_array(obj, fp):
    fp.write(b"\xdd" + struct.pack("<I", len(obj)))

    if len(obj) > 0 and isinstance(obj[0], float):
        fp.write(b"\xca")
        for e in obj:
            fp.write(struct.pack("<f", e))
    elif len(obj) > 0 and isinstance(obj[0], bool):
        for e in obj:
            pack(e, fp)
    elif len(obj) > 0 and isinstance(obj[0], int):
        fp.write(b"\xd2")
        for e in obj:
            fp.write(struct.pack("<i", e))
    # Float32
    elif len(obj) > 0 and isinstance(obj[0], np.float32):
        fp.write(b"\xca")
        fp.write(obj.tobytes())
    # Int32
    elif len(obj) > 0 and isinstance(obj[0], np.int32):
        fp.write(b"\xd2")
        fp.write(obj.tobytes())
    # Int16
    elif len(obj) > 0 and isinstance(obj[0], np.int16):
        fp.write(b"\xd1")
        fp.write(obj.tobytes())
    # Regular
    else:
        for e in obj:
            pack(e, fp)

def _pack_map(obj, fp):
    fp.write(b"\xdf" + struct.pack("<I", len(obj)))
    for k, v in obj.items():
        pack(k, fp)
        pack(v, fp)

def pack(obj, fp):
    if obj is None:
        _pack_nil(fp)
    elif isinstance(obj, bool):
        _pack_boolean(obj, fp)
    elif isinstance(obj, int):
        _pack_integer(obj, fp)
    elif isinstance(obj, float):
        _pack_float(obj, fp)
    elif isinstance(obj, str):
        _pack_string(obj, fp)
    elif isinstance(obj, bytes):
        _pack_binary(obj, fp)
    elif isinstance(obj, (list, np.ndarray, tuple)):
        _pack_array(obj, fp)
    elif isinstance(obj, dict):
        _pack_map(obj, fp)

def packb(obj):
    fp = io.BytesIO()
    pack(obj, fp)
    return fp.getvalue()
