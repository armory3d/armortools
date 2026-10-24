// node paint/tests/layer-object.mjs [ArmorPaint.exe] [optional FBX] [material name]
// Compare a real mouse stroke after changing the layer object with the same stroke
// after a full shader refresh. script_paint() would refresh the shader and hide this bug.
import { spawn } from 'node:child_process';
import { mkdir, readFile, writeFile } from 'node:fs/promises';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import assert from 'node:assert/strict';

const repo = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '../..');
const exe = path.resolve(process.argv[2] || path.join(repo, 'paint/build/out/ArmorPaint.exe'));
const dir = path.join(repo, 'paint/build/tests', `layer-object-${Date.now()}`);
await mkdir(dir, { recursive: true });
const q = s => JSON.stringify(s.replaceAll('\\', '/'));
const fixture = process.argv[3] ? path.resolve(process.argv[3]) : path.join(dir, 'tiles.obj');
if (!process.argv[3]) {
  await writeFile(fixture, [0, 1, 2].map((i) => `o Tile.${1001+i}
v ${i*3-1} -1 0
v ${i*3+1} -1 0
v ${i*3+1} 1 0
v ${i*3-1} 1 0
vt 0 0
vt 1 0
vt 1 1
vt 0 1
f ${i*4+1}/${i*4+1} ${i*4+2}/${i*4+2} ${i*4+3}/${i*4+3}
f ${i*4+1}/${i*4+1} ${i*4+3}/${i*4+3} ${i*4+4}/${i*4+4}
`).join('\n'));
}
const material = process.argv[4] || 'Tile.1002';
// Coordinates must lie on the target in the current dock layout.
const x = Number(process.env.PAINT_TEST_X || 375), y = Number(process.env.PAINT_TEST_Y || 490);
assert(Number.isInteger(x) && Number.isInteger(y));
async function run(mode, refresh) {
const name = `${mode}-${refresh ? 'refreshed' : 'switched'}`;
const script = path.join(dir, name + '.c');
const pixels = path.join(dir, name + '.rgba');
await writeFile(script, `#include "global.h"
int frame=0;
slot_layer_t *layer;
mesh_object_t *target;
void update(){frame++;
 if(frame==2)script_import_mesh(${q(fixture)},0,-1,-1);
 if(frame==12){
  any_array_t *objects=script_get_mesh_objects();
  for(int i=0;i<objects->length;i++){
   mesh_object_t *o=objects->buffer[i];
   if(o->data->index_array->length==0||!o->base->visible)continue;
   if(string_equals(o->base->name,${q(material)})||string_equals(o->data->paint_uvs->material,${q(material)}))target=o;
  }
  if(target==NULL){printf("FAIL: missing target");script_quit();return;}
  layer=script_layer_new();script_get_context()->brush_radius=0.2;
  script_get_config()->brush_live=0;
  script_material_set_color(script_material_get_node("RGB"),0,0,1,0,0,1);script_material_update();
  ${process.argv[3] ? '' : `vertex_array_t *pos=mesh_data_get_vertex_array(target->data,"pos");vec4_t c;c.x=0;c.y=0;c.z=0;c.w=1;
  for(int j=0;j<pos->values->length;j+=4){c.x+=pos->values->buffer[j];c.y+=pos->values->buffer[j+1];c.z+=pos->values->buffer[j+2];}
  float s=target->data->scale_pos/32767.0/(pos->values->length/4);c.x*=s;c.y*=s;c.z*=s;
  object_t *parent=target->base;while(parent!=NULL){transform_t *pt=parent->transform;c.x*=pt->scale.x;c.y*=pt->scale.y;c.z*=pt->scale.z;c=vec4_apply_quat(c,pt->rot);c.x+=pt->loc.x;c.y+=pt->loc.y;c.z+=pt->loc.z;parent=parent->parent;}
  transform_t *t=script_get_camera()->base->transform;t->loc.x=c.x;t->loc.y=c.y-3;t->loc.z=c.z;t->rot=quat_from_euler(1.57079633,0,0);transform_build_matrix(t);camera_object_build_mat(script_get_camera());`}
  script_get_context()->ddirty=3;
 }
 if(frame==16)script_layer_set_object(layer,target->base);
 ${mode === 'shared' ? 'if(frame==18)script_layer_set_object(layer,NULL);' : ''}
 ${mode === 'undo' || mode === 'redo' ? 'if(frame==18)script_undo();' : ''}
 ${mode === 'redo' ? 'if(frame==20)script_redo();' : ''}
 if(frame==22)script_mouse_click(${x},${y},3);
 ${refresh ? 'if(frame==24)script_layer_update(layer);' : ''}
 if(frame==26)script_mouse_click(${x},${y},0);
 if(frame==34){
  buffer_t *p=script_get_texture_pixels(layer->texpaint);
  if(!iron_file_save_bytes(${q(pixels)},p,p->length))printf("FAIL: saving stroke pixels");
  script_screenshot(${q(path.join(dir, name + '.png'))});
 }
 if(frame==40){printf("PASS: stroke captured");script_quit();}
}
void main(){script_set_tools(0);script_notify_on_update(update);}
`);
const output = await new Promise((resolve, reject) => {
  const child = spawn(exe, ['--background', '--script', script], { cwd: path.dirname(exe), windowsHide: true });
  let out = '';
  child.stdout.on('data', b => out += b);
  child.stderr.on('data', b => out += b);
  const timer = setTimeout(() => { child.kill(); reject(Error('Timeout\n' + out)); }, 60000);
  child.on('error', e => { clearTimeout(timer); reject(e); });
  child.on('exit', code => { clearTimeout(timer); code === 0 ? resolve(out) : reject(Error(`Exit ${code}\n${out}`)); });
});
await writeFile(path.join(dir, name + '.log'), output);
assert(!/FAIL|error:/i.test(output), output);
assert(output.includes('PASS'), output);
return readFile(pixels);
}
for (const mode of ['selected', 'shared', 'undo', 'redo']) {
  const actual = await run(mode, false), expected = await run(mode, true);
  assert.equal(actual.length, expected.length);
  let count = 0, wrong = 0;
  for (let i = 3; i < actual.length; i += 4) {
    if (expected[i] > 0) count++;
    if ((actual[i] > 0) !== (expected[i] > 0)) wrong++;
  }
  assert(count > 100, `${mode}: stroke missed mesh; set PAINT_TEST_X/Y for the dock layout. Results: ${dir}`);
  assert.equal(wrong, 0, `${mode}: stale UV mapping. Results: ${dir}`);
  console.log(`PASS: ${mode}, ${count} painted pixels match a full shader refresh`);
}
console.log('Results: ' + dir);
