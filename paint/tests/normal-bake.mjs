// node paint/tests/normal-bake.mjs [ArmorPaint.exe] [optional blender executable] [optional FBX] [optional target mesh name]
import { spawn } from 'node:child_process';
import { mkdir, readFile, writeFile } from 'node:fs/promises';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import assert from 'node:assert/strict';

const repo = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '../..');
const exe = path.resolve(process.argv[2] || path.join(repo, 'paint/build/out/ArmorPaint.exe'));
const dir = path.join(repo, 'paint/build/tests', `normal-bake-${Date.now()}`);
await mkdir(dir, { recursive: true });
const q = s => JSON.stringify(s);
const low = path.join(dir, 'low.obj'), high = path.join(dir, 'high.obj');
const saved = path.join(dir, 'baked.arm');
const blend = path.join(dir, "high & detail's.blend");
const lowFbx = path.join(dir, 'low-uv1.fbx');
const multiFbx = path.join(dir, 'multiple.fbx'), multiObj = path.join(dir, 'multiple.obj');
const blender = process.argv[3] ? path.resolve(process.argv[3]) : null;
let originalBlend;
await writeFile(low, `o Low
v -1 -1 0
v 1 -1 0
v 1 1 0
v -1 1 0
vt 0 0
vt 1 0
vt 1 1
vt 0 1
f 1/1 2/2 3/3
f 1/1 3/3 4/4
`);

if (blender) {
  const fixture = path.join(dir, 'make-blend.py');
  await writeFile(fixture, `import bpy
bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False)
mesh=bpy.data.meshes.new('High')
mesh.from_pydata([(-1,0.3,-1),(1,-0.1,-1),(1,-0.3,1),(-1,0.1,1)],[],[(0,1,2,3)])
obj=bpy.data.objects.new("High 'detail'",mesh)
bpy.context.collection.objects.link(obj)
obj.select_set(True)
bpy.context.view_layer.objects.active=obj
mod=obj.modifiers.new('Render subdivision','SUBSURF')
mod.subdivision_type='SIMPLE'
mod.levels=0
mod.render_levels=3
bpy.ops.mesh.primitive_cube_add(location=(10,0,0))
bpy.context.object.name='Distractor'
obj.select_set(False)
bpy.ops.wm.save_as_mainfile(filepath=${q(blend)})
bpy.ops.object.select_all(action='SELECT')
bpy.ops.export_scene.fbx(filepath=${q(multiFbx)},use_selection=True,object_types={'MESH'},axis_forward='-Z',axis_up='Y')
bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False)
mesh=bpy.data.meshes.new('LowUV')
mesh.from_pydata([(-1,0,-1),(1,0,-1),(1,0,1),(-1,0,1)],[],[(0,1,2,3)])
obj=bpy.data.objects.new('LowUV',mesh)
bpy.context.collection.objects.link(obj)
obj.select_set(True)
bpy.context.view_layer.objects.active=obj
for name,coords in [('UVMap',[(0,0),(1,0),(1,1),(0,1)]),('DetailUV',[(1,0),(0,0),(0,1),(1,1)])]:
    uv=mesh.uv_layers.new(name=name)
    for i,value in enumerate(coords): uv.data[i].uv=value
bpy.ops.export_scene.fbx(filepath=${q(lowFbx)},use_selection=True,object_types={'MESH'},axis_forward='-Z',axis_up='Y')
`);
  await new Promise((resolve, reject) => {
    const child = spawn(blender, ['--background', '--factory-startup', '--python-exit-code', '1', '--python', fixture], { windowsHide: true });
    let output = ''; child.stdout.on('data', b => output += b); child.stderr.on('data', b => output += b);
    const timer = setTimeout(() => { child.kill(); reject(Error('Blender fixture timeout')); }, 60000);
    child.on('error', reject);
    child.on('exit', code => { clearTimeout(timer); code === 0 ? resolve() : reject(Error(output)); });
  });
  originalBlend = await readFile(blend);
}
// Sloping geometry with no UVs or vertex normals. Importer must generate normals.
await writeFile(high, `o High
v -1 -1 -0.3
v 1 -1 0.1
v 1 1 0.3
v -1 1 -0.1
f 1 2 3
f 1 3 4
`);
await writeFile(multiObj, (await readFile(high, 'utf8')) + '\no Distractor\nv 10 0 0\nv 11 0 0\nv 10 1 0\nf 5 6 7\n');

async function run(name, source) {
  const script = path.join(dir, name + '.c');
  await writeFile(script, '#include "global.h"\n' + source);
  const output = await new Promise((resolve, reject) => {
    const child = spawn(exe, ['--background', '--script', script], { cwd: path.dirname(exe), windowsHide: true });
    let out = '';
    child.stdout.on('data', b => out += b);
    child.stderr.on('data', b => out += b);
    const timer = setTimeout(() => { child.kill(); reject(Error(name + ' timeout\n' + out)); }, 120000);
    child.on('error', e => { clearTimeout(timer); reject(e); });
    child.on('exit', code => { clearTimeout(timer); code === 0 ? resolve(out) : reject(Error(`Exit ${code}\n${out}`)); });
  });
  await writeFile(path.join(dir, name + '.log'), output);
  assert(!/FAIL:|error:/i.test(output), output);
  assert(output.includes('PASS'), output);
  console.log('PASS: ' + name);
}

for (const name of ['opengl', 'directx', 'ao', 'curvature', 'height', ...(blender ? ['blend', 'uv1'] : [])]) {
  const directx = name === 'directx';
  const type = { ao: 1, curvature: 2, height: 3 }[name] || 0;
  await run(name, `
int frame=0,stage=0,objects=0,layers=0,applied_frame=0;
mesh_object_t *target;
void fail(char *message){script_screenshot(${q(path.join(dir, name + '-failure.png'))});printf("FAIL: %s: %s",message,bake_normal_error());script_quit();}
void update(){frame++;
 if(frame==2)script_import_asset(${q(name === 'uv1' ? lowFbx : low)},false);
 if(frame==12){
  any_array_t *meshes=script_get_mesh_objects();objects=meshes->length;
  for(int i=0;i<objects;i++){mesh_object_t *o=meshes->buffer[i];if(o->data->index_array->length>0)target=o;}
  if(target==NULL||!string_equals(target->base->name,${JSON.stringify(name === 'uv1' ? 'LowUV' : 'Low')})){fail("low-poly fixture was not imported");return;}
  layers=script_layer_count();
  box_bake_normal_show();
  if(!bake_normal_select_low(target,${name === 'uv1' ? 1 : 0})){fail("select low");return;}
  ${name === 'uv1' ? '' : 'if(bake_normal_select_low(target,1)){fail("missing UV1 accepted");return;}'}
  ${name === 'blend' ? `bake_normal_configure_blender(${q(blender)},"High 'detail'",true);` : ''}
  if(!bake_normal_load(${q(name === 'blend' ? blend : high)})){fail("load high");return;}
  stage=1;
 }
 if(stage==1&&bake_normal_status()!=1){
  if(bake_normal_status()==5){bake_normal_select_source(-1);return;}
  if(bake_normal_status()<0){fail("import worker");return;}
  ${name === 'blend' ? 'if(bake_normal_triangle_count(true)!=128){fail("render subdivision was not evaluated");return;}' : ''}
  if(script_get_mesh_objects()->length!=objects||script_layer_count()!=layers){fail("high import changed project");return;}
  bake_normal_set_type(${type},16,0.5,2);
  if(!bake_normal_start(128,1,1,8,${directx},true,4)){fail("start bake");return;}
  stage=2;
 }
 if(stage==2&&bake_normal_status()!=2){
  if(bake_normal_status()!=3){fail("bake worker");return;}
  script_screenshot(${q(path.join(dir, name + '-preview.png'))});stage=5;applied_frame=frame;
 }
 if(stage==5&&frame>=applied_frame+4){
  if(!bake_normal_apply(true)){fail("apply");return;}
  if(bake_normal_apply(true)){fail("duplicate apply");return;}
  if(script_layer_count()!=layers+1){fail("missing layer");return;}
  slot_layer_t *l=script_get_context()->layer;
  if(l->fill_material==NULL||l->fill_material->paint_nor!=${type === 0}||l->fill_material->paint_base!=${type === 2}||l->fill_material->paint_occ!=${type === 1}||l->fill_material->paint_height!=${type === 3}||!l->fill_material->paint_opac||l->uv_map!=${name === 'uv1' ? 1 : 0}||l->object_mask<=0){fail("map fill flags");return;}
  if(!script_packed_asset_save(".png",${q(path.join(dir, name + '.png'))})){fail("packed PNG missing");return;}
  bake_normal_close();stage=3;applied_frame=frame;
 }
 if(stage==3&&frame>=applied_frame+8){
  slot_layer_t *l=script_get_context()->layer;
  buffer_t *pixels=script_get_texture_pixels(l->${type === 0 ? 'texpaint_nor' : type === 2 ? 'texpaint' : 'texpaint_pack'});
  iron_file_save_bytes(${q(path.join(dir, name + '.rgba'))},pixels,pixels->length);
  pixels=script_get_texture_pixels(l->texpaint);
  iron_file_save_bytes(${q(path.join(dir, name + '-coverage.rgba'))},pixels,pixels->length);
  if(${name === 'opengl'}){project_filepath_set(${q(saved)});project_save(0);}
  stage=4;applied_frame=frame;
 }
 if(stage==4&&frame>=applied_frame+8){printf("PASS: baked no-UV high to normal fill");script_quit();}
}
void main(){script_set_tools(0);script_notify_on_update(update);}
`);
}
await run('batch-maps', `
int frame=0,stage=0,layers=0,finished=0,last=0;mesh_object_t *target;
void fail(char *message){printf("FAIL: %s: %s",message,bake_normal_error());script_quit();}
void update(){frame++;
 if(frame==2)script_import_asset(${q(low)},false);
 if(frame==12){
  any_array_t *meshes=script_get_mesh_objects();for(int i=0;i<meshes->length;i++){mesh_object_t *o=meshes->buffer[i];if(o->data->index_array->length>0)target=o;}
  layers=script_layer_count();box_bake_normal_show();
  if(!bake_normal_select_low(target,0)||!bake_normal_load(${q(high)})){fail("batch setup");return;}stage=1;
 }
 if(stage==1&&bake_normal_status()!=1){
  bake_normal_set_type(1,16,0.5,2);bake_normal_set_type(2,16,0.5,2);
  if(bake_normal_set_maps(0)||bake_normal_set_maps(16)||!bake_normal_set_maps(15)||!bake_normal_start(128,1,1,8,false,true,4)){fail("batch selection");return;}stage=2;
 }
 if(stage==2){
  int status=bake_normal_status(),p=bake_normal_progress();if(p<last){fail("batch progress backwards");return;}last=p;
  if(status!=2){
   if(status!=3||bake_normal_result_mask()!=15||p!=100){fail("batch results");return;}
   for(int i=0;i<4;i++)if(!bake_normal_select_result(i)){fail("batch preview");return;}
   if(!bake_normal_analyze(1,1,true)){fail("batch projection check");return;}stage=3;
  }
 }
 if(stage==3&&bake_normal_status()!=4){
  if(bake_normal_result_mask()!=15||!bake_normal_apply_all(true)||bake_normal_apply_all(true)||script_layer_count()!=layers+4){fail("batch apply");return;}
  stage=4;finished=frame;
 }
 if(stage==4&&frame>=finished+8){
  for(int type=0;type<4;type++){
   slot_layer_t *l=script_layer_at(layers+type);
   if(l->fill_material==NULL||l->fill_material->paint_nor!=(type==0)||l->fill_material->paint_occ!=(type==1)||l->fill_material->paint_base!=(type==2)||l->fill_material->paint_height!=(type==3)||l->uv_map!=0||l->object_mask<=0){fail("batch layer channels");return;}
   buffer_t *pixels=script_get_texture_pixels(type==0?l->texpaint_nor:type==2?l->texpaint:l->texpaint_pack);
   ${[0,1,2,3].map(type => `if(type==${type})iron_file_save_bytes(${q(path.join(dir, `batch-${type}.rgba`))},pixels,pixels->length);`).join("")}
  }
  if(!bake_normal_set_type(1,128,0.5,2)||!bake_normal_set_maps(15)||!bake_normal_start(1024,1,1,8,false,true,1)){fail("cancel batch start");return;}
  stage=5;last=0;
 }
 if(stage==5){
  int status=bake_normal_status(),p=bake_normal_progress();if(p<last){fail("cancel batch progress");return;}last=p;
  if(p>42&&status==2){bake_normal_cancel();stage=6;}
  else if(status!=2){fail("batch finished before cancellation");return;}
 }
 if(stage==6&&bake_normal_status()!=2){
  if(bake_normal_status()!=-1||bake_normal_result_mask()!=0||bake_normal_apply_all(true)||script_layer_count()!=layers+4){fail("cancelled batch retained outputs");return;}
  if(!bake_normal_set_maps(10)||!bake_normal_start(64,1,1,4,false,true,1)){fail("batch retry");return;}stage=7;
 }
 if(stage==7&&bake_normal_status()!=2){
  if(bake_normal_result_mask()!=10||bake_normal_select_result(0)||!bake_normal_select_result(3)){fail("subset results");return;}
  if(!bake_normal_apply(true)||!bake_normal_apply_all(true)||script_layer_count()!=layers+6){fail("apply remaining subset");return;}
  bake_normal_close();if(bake_normal_result_mask()!=0){fail("batch cleanup");return;}
  printf("PASS: four maps, independent previews, layer channels, cancellation and subset retry");script_quit();
 }
}
void main(){script_set_tools(0);script_notify_on_update(update);}
`);
for (const [type, name] of ['opengl', 'ao', 'curvature', 'height'].entries()) {
  assert.deepEqual(await readFile(path.join(dir, `batch-${type}.rgba`)), await readFile(path.join(dir, name + '.rgba')), `${name}: batch differs from single-map bake`);
}
const gl = await readFile(path.join(dir, 'opengl.rgba'));
const dx = await readFile(path.join(dir, 'directx.rgba'));
assert.equal(gl.length, dx.length);
let changed = 0, differs = 0;
for (let i = 0; i < gl.length; i += 4) {
  if (Math.abs(gl[i] - 128) > 8 || Math.abs(gl[i + 1] - 128) > 8) changed++;
  if (Math.abs(gl[i] - dx[i]) > 2 || Math.abs(gl[i + 1] - dx[i + 1]) > 2 || Math.abs(gl[i + 2] - dx[i + 2]) > 2) differs++;
}
assert(changed > gl.length / 16, 'Slope was not projected to the normal texture');
assert(differs < gl.length / 400, `OpenGL and DirectX materials render differently: ${differs}`);
const coverage = await readFile(path.join(dir, 'opengl-coverage.rgba'));
let opaque = 0;
for (let i = 3; i < coverage.length; i += 4) if (coverage[i] > 200) opaque++;
assert(opaque > coverage.length / 8, 'Normal fill has no coverage and would be invisible');
for (const [name, channel, expected] of [['ao', 0, 255], ['curvature', 0, 128], ['height', 3, 128]]) {
  const pixels = await readFile(path.join(dir, name + '.rgba'));
  const size = Math.sqrt(pixels.length / 4), middle = (Math.floor(size / 2) * size + Math.floor(size / 2)) * 4;
  assert(Math.abs(pixels[middle + channel] - expected) <= 3, `${name}: wrong scalar material channel or gamma`);
}
if (blender) {
  const pixels = await readFile(path.join(dir, 'blend.rgba'));
  let different = 0;
  for (let i = 0; i < pixels.length; i++) if (Math.abs(pixels[i] - gl[i]) > 2) different++;
  assert(different < pixels.length / 100, 'Blender geometry differs from the equivalent OBJ');
  assert.deepEqual(await readFile(blend), originalBlend, 'Blender source was modified');
  const uv1 = await readFile(path.join(dir, 'uv1.rgba'));
  const size = Math.sqrt(gl.length / 4);
  const middle = (Math.floor(size / 2) * size + Math.floor(size / 2)) * 4;
  assert(Math.abs(uv1[middle] + gl[middle] - 255) <= 2, 'Mirrored UV1 did not flip the tangent normal');
  assert(Math.abs(uv1[middle + 1] - gl[middle + 1]) <= 2, 'UV1 changed the bitangent normal');
  assert(Math.abs(uv1[middle + 2] - gl[middle + 2]) <= 2, 'UV1 changed the surface normal');
}

for (const [format, source, wanted, triangles] of [
  ['obj', multiObj, 'High', 2],
  ...(blender ? [['blend', blend, "High 'detail'", 128], ['fbx', multiFbx, "High 'detail'", 2]] : []),
]) {
  await run('object-picker-' + format, `
int frame=0,stage=0,picked=-1,wait_frame=0;mesh_object_t *target;
void fail(char *message){printf("FAIL: %s: %s",message,bake_normal_error());script_quit();}
void update(){frame++;
 if(frame==2)script_import_asset(${q(low)},false);
 if(frame==12){
  any_array_t *meshes=script_get_mesh_objects();for(int i=0;i<meshes->length;i++){mesh_object_t *o=meshes->buffer[i];if(o->data->index_array->length>0)target=o;}
  box_bake_normal_show();bake_normal_select_low(target,0);
  ${blender ? `bake_normal_configure_blender(${q(blender)},"",false);` : ''}
  if(!bake_normal_load(${q(source)})){fail("open multiple objects");return;}stage=1;
 }
 if(stage==1&&bake_normal_status()!=1){
  if(bake_normal_status()!=5||bake_normal_source_count()!=2||bake_normal_triangle_count(true)!=0||bake_normal_start(64,1,1,4,false,true,1)){fail("import did not wait for object choice");return;}
  for(int i=0;i<bake_normal_source_count();i++)if(string_equals(bake_normal_source_name(i),${JSON.stringify(wanted)}))picked=i;
  if(picked<0){fail("object name missing");return;}
  script_screenshot(${q(path.join(dir, format + '-objects.png'))});wait_frame=frame;stage=2;
 }
 if(stage==2&&frame>=wait_frame+4){if(!bake_normal_select_source(picked)){fail("select source object");return;}stage=3;}
 if(stage==3&&bake_normal_status()!=1&&bake_normal_status()!=4){
  if(bake_normal_triangle_count(true)!=${triangles}){fail("selected object triangle count");return;}
  if(!bake_normal_start(64,1,1,4,false,true,1)){fail("bake selected object");return;}stage=4;
 }
 if(stage==4&&bake_normal_status()!=2){
  if(bake_normal_status()!=3||!bake_normal_analyze(1,1,true)){fail("check projection");return;}stage=5;
 }
 if(stage==5&&bake_normal_status()!=4){
  if(bake_normal_status()!=3||!bake_normal_apply(false)){fail("projection check lost bake result");return;}
  if(!bake_normal_select_source(picked==0?1:0)){fail("switch source object");return;}stage=6;
 }
 if(stage==6&&bake_normal_status()!=1&&bake_normal_status()!=4){
  if(bake_normal_status()<0||bake_normal_triangle_count(true)!=${format === 'obj' ? 1 : 12}){fail("wrong geometry after source switch");return;}
  bake_normal_close();printf("PASS: object picker and projection check");script_quit();
 }
}
void main(){script_set_tools(0);script_notify_on_update(update);}
`);
}

await run('cancel-and-retry', `
int frame=0,stage=0;mesh_object_t *target;
void fail(char *message){printf("FAIL: %s: %s",message,bake_normal_error());script_quit();}
void update(){frame++;
 if(frame==2)script_import_asset(${q(low)},false);
 if(frame==12){
  any_array_t *meshes=script_get_mesh_objects();for(int i=0;i<meshes->length;i++){mesh_object_t *o=meshes->buffer[i];if(o->data->index_array->length>0)target=o;}
  bake_normal_cancel();
  if(!bake_normal_select_low(target,0)||!bake_normal_load(${q(blender ? blend : high)})){fail("load to cancel");return;}
  bake_normal_cancel();stage=1;
 }
 if(stage==1&&bake_normal_status()!=1){
  if(bake_normal_status()!=-1||bake_normal_apply(false)){fail("import cancellation");return;}
  if(!bake_normal_load(${q(high)})){fail("retry import");return;}stage=2;
 }
 if(stage==2&&bake_normal_status()!=1){
  if(bake_normal_status()<0||!bake_normal_start(4096,1,1,16,false,true,4)){fail("large bake");return;}
  bake_normal_cancel();stage=3;
 }
 if(stage==3&&bake_normal_status()!=2){
  if(bake_normal_status()!=-1||bake_normal_apply(false)){fail("bake cancellation");return;}
  if(!bake_normal_start(64,1,1,4,false,true,1)){fail("retry bake");return;}stage=4;
 }
 if(stage==4&&bake_normal_status()!=2){
  if(bake_normal_status()!=3){fail("retry result");return;}
  if(!bake_normal_load(${q(path.join(dir, 'missing.obj'))})){fail("missing file start");return;}stage=5;
 }
 if(stage==5&&bake_normal_status()!=1){
  if(bake_normal_status()!=-1||bake_normal_start(64,1,1,4,false,true,1)){fail("failed import retained stale source");return;}
  bake_normal_close();bake_normal_cancel();
  if(bake_normal_status()!=0||bake_normal_triangle_count(true)!=0){fail("close cleanup");return;}
  printf("PASS: cancellation, retry, failure and cleanup");script_quit();
 }
}
void main(){script_set_tools(0);script_notify_on_update(update);}
`);

await run('project-source', `
int frame=0,stage=0;mesh_object_t *low,*high;
void update(){frame++;
 if(frame==2)script_import_asset(${q(low)},false);
 if(frame==12)script_append_mesh(${q(high)});
 if(frame==24){
  any_array_t *meshes=script_get_mesh_objects();
  for(int i=0;i<meshes->length;i++){mesh_object_t *o=meshes->buffer[i];if(string_equals(o->base->name,"Low"))low=o;if(string_equals(o->base->name,"High"))high=o;}
  if(!bake_normal_select_low(low,0)||!bake_normal_select_high(high)||!bake_normal_start(64,1,1,4,false,true,1)){printf("FAIL: project source setup");script_quit();return;}
  stage=1;
 }
 if(stage==1&&bake_normal_status()!=2){
  if(bake_normal_status()!=3||!bake_normal_apply(false))printf("FAIL: project source bake: %s",bake_normal_error());
  else printf("PASS: project mesh source");
  bake_normal_close();script_quit();
 }
}
void main(){script_set_tools(0);script_notify_on_update(update);}
`);

if (process.argv[4]) {
  const fbx = path.resolve(process.argv[4]);
  await run('fbx-target', `
int frame=0,stage=0;mesh_object_t *target;
void update(){frame++;
 if(frame==2)script_import_asset(${q(fbx)},false);
 if(frame==20){
  any_array_t *meshes=script_get_mesh_objects();
  for(int i=0;i<meshes->length;i++){mesh_object_t *o=meshes->buffer[i];if(o->data->index_array->length>0 && ${process.argv[5] ? `string_equals(o->base->name,${JSON.stringify(process.argv[5])})` : 'target==NULL'})target=o;}
  box_bake_normal_show();
  if(!bake_normal_select_low(target,0)||!bake_normal_load(${q(fbx)})){printf("FAIL: FBX source: %s",bake_normal_error());script_quit();return;}stage=1;
 }
 if(stage==1&&bake_normal_status()!=1){
  if(bake_normal_status()==5){bake_normal_select_source(-1);return;}
  if(bake_normal_status()<0||!bake_normal_start(128,0.05,0.05,8,false,true,4)){printf("FAIL: FBX start: %s",bake_normal_error());script_quit();return;}stage=2;
 }
 if(stage==2&&bake_normal_status()!=2){
  if(bake_normal_status()!=3||!bake_normal_apply(true)){printf("FAIL: FBX apply: %s",bake_normal_error());script_quit();return;}
  script_screenshot(${q(path.join(dir, 'fbx-preview.png'))});stage=3;frame=100;
 }
 if(stage==3&&frame==110){printf("PASS: FBX target");script_quit();}
}
void main(){script_set_tools(0);script_notify_on_update(update);}
`);
}

await run('reopen', `
int frame=0;
void update(){frame++;
 if(frame==2)script_project_open(${q(saved)});
 if(frame==20){
  slot_layer_t *l=script_layer_at(script_layer_count()-1);
  if(l->fill_material==NULL||!l->fill_material->paint_nor||l->fill_material->paint_base)printf("FAIL: fill not restored");
  if(!script_packed_asset_save(".png",${q(path.join(dir, 'reopened.png'))}))printf("FAIL: packed texture lost");
  buffer_t *pixels=script_get_texture_pixels(l->texpaint_nor);
  iron_file_save_bytes(${q(path.join(dir, 'reopened.rgba'))},pixels,pixels->length);
  printf("PASS: saved bake restored");script_quit();
 }
}
void main(){script_set_tools(0);script_notify_on_update(update);}
`);
assert.deepEqual(await readFile(path.join(dir, 'reopened.rgba')), gl, 'Reopening changed the normal layer');
await run('bake-progress', `
int frame=0,stage=0,last=0,updates=0;bool captured=false;mesh_object_t *target;
void update(){frame++;
 if(frame==2)script_import_asset(${q(low)},false);
 if(frame==12){
  any_array_t *meshes=script_get_mesh_objects();for(int i=0;i<meshes->length;i++){mesh_object_t *o=meshes->buffer[i];if(o->data->index_array->length>0)target=o;}
  box_bake_normal_show();
  if(!bake_normal_select_low(target,0)||!bake_normal_load(${q(high)})){printf("FAIL: progress setup");script_quit();return;}stage=1;
 }
 if(stage==1&&bake_normal_status()!=1){
  if(!bake_normal_set_type(1,128,1,2)||!bake_normal_start(1024,1,1,8,false,true,1)){printf("FAIL: progress bake");script_quit();return;}stage=2;
 }
 if(stage==2){
  int status=bake_normal_status(),p=bake_normal_progress();
  if(p<last){printf("FAIL: progress moved backwards");script_quit();return;}if(p>last)updates++;last=p;
  if(p>30&&p<85&&!captured){script_screenshot(${q(path.join(dir, 'bake-progress.png'))});captured=true;}
  if(status!=2){
   if(status!=3||p!=100||updates<3||!captured)printf("FAIL: bake progress incomplete");
   else printf("PASS: bake progress and popup");script_quit();
  }
 }
}
void main(){script_set_tools(0);script_notify_on_update(update);}
`);

if (process.env.BAKE_STRESS === '1') {
  const large = path.join(dir, 'five-million.obj');
  await writeFile(large, 'o High\nv -1 -1 0\nv 1 -1 0\nv 0 1 0\nvn 0 0 1\n' + 'f 1//1 2//1 3//1\n'.repeat(5000000));
  await run('large-import', `
int frame=0,stage=0,last=0,updates=0,objects=0;bool captured=false;mesh_object_t *target;
void update(){frame++;
 if(frame==2)script_import_asset(${q(low)},false);
 if(frame==12){
  any_array_t *meshes=script_get_mesh_objects();objects=meshes->length;
  for(int i=0;i<meshes->length;i++){mesh_object_t *o=meshes->buffer[i];if(o->data->index_array->length>0)target=o;}
  box_bake_normal_show();
  if(!bake_normal_select_low(target,0)||!bake_normal_load(${q(large)})){printf("FAIL: stress setup");script_quit();return;}stage=1;
 }
 if(stage==1){
  int status=bake_normal_status(),p=bake_normal_progress();
  if(p<last){printf("FAIL: import progress moved backwards");script_quit();return;}if(p>last)updates++;last=p;
  if(p>5&&p<95&&!captured){script_screenshot(${q(path.join(dir, 'import-progress.png'))});captured=true;}
  if(status!=1){
   if(status<0||bake_normal_triangle_count(true)!=5000000||script_get_mesh_objects()->length!=objects||updates<3||!captured)printf("FAIL: stress import: %s",bake_normal_error());
   else printf("PASS: five million triangles, progress and scene isolation");
   bake_normal_close();script_quit();
  }
 }
}
void main(){script_set_tools(0);script_notify_on_update(update);}
`);
}
console.log('Results: ' + dir);
