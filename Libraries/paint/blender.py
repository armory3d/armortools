import bpy
import arm.material.mat_utils
import arm.write_data
import arm.assets
import arm.make_renderpath
import arm.make_state
import arm.utils

def on_make_world():
    wrd = bpy.data.worlds['Arm']
    wrd.world_defs += '_EnvTex'
    wrd.world_defs += '_EnvStr'

def on_make_renderpath():
    wrd = bpy.data.worlds['Arm']
    arm.assets.add_shader_pass('copy_mrt3_pass')
    arm.assets.add(arm.utils.get_sdk_path() + '/armory/Assets/noise256.png')
    arm.assets.add_embedded_data('noise256.png')
    wrd.world_defs += '_CGrainStatic'
    wrd.world_defs += '_Emission'
    arm.assets.add_khafile_param('--macro include("arm.brushnode")')
    # arm.assets.add_khafile_param('--macro include("armory.logicnode")')
    arm.assets.add_khafile_def('arm_appwh')
    arm.assets.add_khafile_def('arm_skip_envmap')
    wrd.world_defs += '_Brdf'
    wrd.world_defs += '_Irr'
    wrd.world_defs += '_Rad'
    wrd.arm_envtex_num_mips = 10
    wrd.arm_envtex_name = 'World.hdr'
    wrd.arm_envtex_irr_name = 'World'
    wrd.arm_envtex_strength = 4.0

def register():
    arm.make_world.callback = on_make_world
    arm.make_renderpath.callback = on_make_renderpath
