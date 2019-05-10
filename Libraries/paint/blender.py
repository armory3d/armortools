import bpy
import arm.material.mat_utils
import arm.write_data
import arm.assets
import arm.make_renderpath
import arm.make_state
import arm.utils

def on_make_renderpath():
    arm.assets.add_shader_pass('copy_mrt3_pass')
    arm.assets.add(arm.utils.get_sdk_path() + '/armory/Assets/noise256.png')
    arm.assets.add_embedded_data('noise256.png')
    bpy.data.worlds['Arm'].world_defs += '_CGrainStatic'

    arm.assets.add_khafile_param('--macro include("arm.brushnode")')
    arm.assets.add_khafile_def('arm_appwh')
    # arm.assets.add_khafile_param('--macro include("armory.logicnode")')

def register():
    arm.make_renderpath.callback = on_make_renderpath
