// node paint/tests/layer-lock.mjs [ArmorPaint.exe]
import { spawn } from 'node:child_process';
import { mkdir, readFile, writeFile } from 'node:fs/promises';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import assert from 'node:assert/strict';

const repo = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '../..');
const exe = path.resolve(process.argv[2] || path.join(repo, 'paint/build/out/ArmorPaint.exe'));
const dir = path.join(repo, 'paint/build/tests', `layer-lock-${Date.now()}`);
await mkdir(dir, { recursive: true });
const q = JSON.stringify;
const saved = path.join(dir, 'locked.arm');
async function run(name, source) {
  const script = path.join(dir, name + '.c');
  await writeFile(script, '#include "global.h"\n' + source);
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
  assert(!/FAIL:|error:/i.test(output), output);
  assert(output.includes('PASS:'), output);
}
await run('operations', `
int frame=0; slot_layer_t *layer; slot_layer_t *group; slot_layer_t *mask;
void fail(char *text){printf("FAIL: %s",text);script_quit();}
void pixels(char *name){buffer_t *p=script_get_texture_pixels(layer->texpaint);iron_file_save_bytes(name,p,p->length);}
void update(){frame++;
 if(frame==2){
  layer=script_layer_new_color(0xff123456,1,0.3,0);
  script_layer_set_name(layer,"Protected");
  script_material_set_color(script_material_get_node("RGB"),0,0,1,0,0,1);script_material_update();
 }
 if(frame==10){
  pixels(${q(path.join(dir, 'before.rgba'))});
  tab_layers_layer_toggle_locked(layer);
  if(!layer->locked || !slot_layer_is_locked(layer)){fail("lock");return;}
  script_layer_clear(layer);script_fill_layer();script_paint(0.5,0.5);script_paint_end();
  int count=script_layer_count();script_layer_delete(layer);
  if(count!=script_layer_count() || script_layer_move(layer,0) || script_layer_new_mask(layer,1)!=NULL || script_layer_to_fill(layer)) {fail("locked operation allowed");return;}
  script_layer_set_name(layer,"Changed");
  if(!string_equals(layer->name,"Protected")){fail("locked rename");return;}
  script_undo();if(layer->locked){fail("undo lock");return;}
  script_redo();if(!layer->locked){fail("redo lock");return;}
 }
 if(frame==16){
  pixels(${q(path.join(dir, 'locked.rgba'))});
  slot_layer_t *upper=script_layer_new_color(0xffabcdef,1,0.5,0);
  if(script_layer_merge_down(upper)!=NULL){fail("merge into locked destination");return;}
  script_layer_delete(upper);
  tab_layers_layer_toggle_locked(layer);script_layer_set(layer);script_fill_layer();
 }
 if(frame==22){
  pixels(${q(path.join(dir, 'unlocked.rgba'))});
  mask=script_layer_new_mask(layer,1);script_layer_set_name(mask,"Protected mask");
  group=script_layer_new_group(layer);script_layer_set_name(group,"Protected group");
  tab_layers_layer_toggle_locked(group);
  if(!slot_layer_is_locked(layer)||!slot_layer_is_locked(mask)||layer->locked){fail("inherited lock");return;}
  tab_layers_layer_toggle_locked(layer);if(layer->locked){fail("child bypassed group lock");return;}
  int count=script_layer_count();script_layer_delete(group);
  if(count!=script_layer_count()||script_layer_merge_group(group)!=NULL||script_layer_apply_mask(mask)!=NULL){fail("protected group mutation");return;}
  script_layer_set(layer);slot_layer_t *outside=script_layer_new();
  if(outside->parent!=NULL||slot_layer_is_locked(outside)){fail("new layer inside locked group");return;}
  script_layer_delete(outside);
  tab_layers_layer_toggle_locked(group);tab_layers_layer_toggle_locked(mask);
  count=script_layer_count();script_layer_delete(group);
  if(count!=script_layer_count()||script_layer_merge_group(group)!=NULL){fail("locked descendant lost");return;}
  tab_layers_layer_toggle_locked(group);
  project_filepath_set(${q(saved)});project_save(false);
 }
 if(frame==30){printf("PASS: layer and group locks, undo/redo, protected operations and save");script_quit();}
}
void main(){script_set_tools(0);script_notify_on_update(update);}
`);
assert.deepEqual(await readFile(path.join(dir, 'before.rgba')), await readFile(path.join(dir, 'locked.rgba')), 'Locked pixels changed');
assert.notDeepEqual(await readFile(path.join(dir, 'before.rgba')), await readFile(path.join(dir, 'unlocked.rgba')), 'Unlocked control did not change');
await run('reload', `
int frame=0;
void update(){frame++;
 if(frame==2)script_project_open(${q(saved)});
 if(frame==12){
  slot_layer_t *group=script_layer_get("Protected group");slot_layer_t *layer=script_layer_get("Protected");slot_layer_t *mask=script_layer_get("Protected mask");
  if(group==NULL||layer==NULL||mask==NULL||!group->locked||!mask->locked||layer->locked||!slot_layer_is_locked(layer))printf("FAIL: saved lock state");
  else {tab_layers_layer_toggle_locked(group);if(slot_layer_is_locked(layer)||!slot_layer_is_locked(mask))printf("FAIL: independent child lock");else printf("PASS: project lock round trip");}
  script_quit();
 }
}
void main(){script_set_tools(0);script_notify_on_update(update);}
`);
console.log('PASS: layer locks preserve pixels, protect groups and survive undo/redo and project reload');
console.log(dir);
