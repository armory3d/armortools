// Run after building: node paint/tests/critical.mjs [path/to/ArmorPaint.exe]
import { spawn } from 'node:child_process';
import { mkdir, writeFile, stat } from 'node:fs/promises';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import assert from 'node:assert/strict';

const repo = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '../..');
const exe = path.resolve(process.argv[2] || path.join(repo, 'paint/build/out/ArmorPaint.exe'));
const dir = path.join(repo, 'paint/build/tests', `critical-${Date.now()}`);
await mkdir(dir, { recursive: true });
const quote = s => JSON.stringify(s.replaceAll('\\', '/'));

async function run(name, source, expected) {
  const script = path.join(dir, name + '.c');
  await writeFile(script, '#include "global.h"\n' + source);
  const output = await new Promise((resolve, reject) => {
    const child = spawn(exe, ['--background', '--script', script], { cwd: path.dirname(exe), windowsHide: true });
    let out = '';
    child.stdout.on('data', b => { out += b; });
    child.stderr.on('data', b => { out += b; });
    const timeout = setTimeout(() => { child.kill(); reject(new Error(name + ' timed out\n' + out)); }, 60000);
    child.on('error', e => { clearTimeout(timeout); reject(e); });
    child.on('exit', code => {
      clearTimeout(timeout);
      if (code !== 0) reject(new Error(`${name}: exit ${code}\n${out}`));
      else resolve(out);
    });
  });
  await writeFile(path.join(dir, name + '.log'), output);
  assert(!output.includes('FAIL'), output);
  assert(output.includes(expected), output);
  console.log('PASS: ' + name);
}

await run('failed-save-quit', `
int frames = 0;
void update() {
  frames++;
  if (frames == 1) {
    project_filepath_set(${quote(path.join(dir, 'missing/project.arm'))});
    project_save(1);
  }
  if (frames == 8) {
    if (string_index_of(sys_title(), "*") < 0) printf("FAIL: failed save cleared dirty title");
    else printf("PASS: still running with unsaved changes");
    script_quit();
  }
}
void main() { script_notify_on_update(update); }
`, 'PASS: still running with unsaved changes');

const saved = path.join(dir, 'saved.arm');
await run('successful-save-quit', `
int frames = 0;
void update() {
  frames++;
  if (frames == 1) { project_filepath_set(${quote(saved)}); project_save(1); }
  if (frames == 20) { printf("FAIL: successful save did not quit"); script_quit(); }
}
void main() { script_notify_on_update(update); }
`, 'Project saved');
assert((await stat(saved)).size > 100);

await run('mesh-only-import', `
int frames = 0;
void update() {
  frames++;
  if (frames == 1) script_project_open(${quote(path.join(repo, 'paint/assets/meshes/cube_shared_uvs.arm'))});
  if (frames == 8) {
    context_t *ctx = script_get_context();
    if (ctx->paint_object->data->index_array->length != 36) printf("FAIL: imported cube indices");
    else printf("PASS: mesh-only project imported");
    script_quit();
  }
}
void main() { script_notify_on_update(update); }
`, 'PASS: mesh-only project imported');

await run('saved-project-reopen', `
int frames = 0;
void update() {
  frames++;
  if (frames == 1) script_project_open(${quote(saved)});
  if (frames == 8) {
    if (script_layer_count() < 1) printf("FAIL: no saved layers");
    else printf("PASS: saved project reopened");
    script_quit();
  }
}
void main() { script_notify_on_update(update); }
`, 'PASS: saved project reopened');
await run('undo-after-project-reset', `
int frame = 0;
int cycles = 0;
int pixels[12];
slot_layer_t *layer;
slot_layer_t *mask;
void remember(gpu_texture_t *tex, int offset, int compare) {
  buffer_t *p = script_get_texture_pixels(tex);
  for (int i = 0; i < 4; i++) {
    if (compare && pixels[offset + i] != p->buffer[i]) printf("FAIL: undo changed texture channel");
    if (!compare) pixels[offset + i] = p->buffer[i];
  }
}
void check_alpha(int expected) {
  buffer_t *p = script_get_texture_pixels(layer->texpaint);
  if (p->buffer[3] != expected) printf("FAIL: layer alpha");
}
void check_mask(int expected) {
  buffer_t *p = script_get_texture_pixels(mask->texpaint);
  if (p->buffer[0] != expected) printf("FAIL: mask value at frame %d: got %d, expected %d", frame, p->buffer[0], expected);
}
void update() {
  frame++;
  if (frame == 1) {
    for (int i = 0; i < script_get_config()->undo_steps; i++) script_layer_clear(script_layer_at(0));
    script_project_new();
  }
  if (frame == 4) layer = script_layer_new_color(0xff123456, 0.7, 0.3, 0.5);
  if (frame == 8) {
    remember(layer->texpaint, 0, 0);
    remember(layer->texpaint_nor, 4, 0);
    remember(layer->texpaint_pack, 8, 0);
    script_layer_clear(layer);
  }
  if (frame == 12) { check_alpha(0); script_layer_set(script_layer_at(0)); script_undo(); }
  if (frame == 16) {
    remember(layer->texpaint, 0, 1);
    remember(layer->texpaint_nor, 4, 1);
    remember(layer->texpaint_pack, 8, 1);
    script_redo();
  }
  if (frame == 20) { check_alpha(0); mask = script_layer_new_mask(layer, 0); }
  if (frame == 24) { check_mask(0); script_layer_clear(mask); }
  if (frame == 28) { check_mask(255); script_layer_set(layer); script_undo(); }
  if (frame == 32) { check_mask(0); script_redo(); }
  if (frame == 36) {
    check_mask(255);
    script_project_new();
    cycles++;
    frame = 0;
    if (cycles == 2) { printf("PASS: layer and mask undo/redo after two project resets"); script_quit(); }
  }
}
void main() { script_notify_on_update(update); }
`, 'PASS: layer and mask undo/redo after two project resets');
console.log('Logs: ' + dir);
