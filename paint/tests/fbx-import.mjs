// node paint/tests/fbx-import.mjs path/to/model.FBX [path/to/ArmorPaint.exe]
// The local fixture is copied only into ignored build output, never into source control.
import { spawn } from 'node:child_process';
import { copyFile, mkdir, writeFile } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import path from 'node:path';
import assert from 'node:assert/strict';

assert(process.argv[2], 'Supply an FBX fixture path');
const source = path.resolve(process.argv[2]);
const repo = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '../..');
const exe = path.resolve(process.argv[3] || path.join(repo, 'paint/build/out/ArmorPaint.exe'));
const dir = path.join(repo, 'paint/build/tests', `fbx-${Date.now()}`);
await mkdir(dir, { recursive: true });
const quote = s => JSON.stringify(s.replaceAll('\\', '/'));
let reference;
for (const [index, extension] of [null, 'fbx', 'FBX', 'FbX'].entries()) {
  const name = extension || 'original';
  const caseDir = path.join(dir, `${index}-${name}`);
  await mkdir(caseDir);
  const file = extension ? path.join(caseDir, 'fixture.' + extension) : source;
  if (extension) await copyFile(source, file);
  const script = path.join(caseDir, 'import.c');
  await writeFile(script, `#include "global.h"
int frames = 0;
int mesh_count = 0;
int index_count = 0;
void visit(object_t *object) {
  if (string_equals(object->ext_type, "mesh_object_t")) {
    mesh_object_t *mesh = object->ext;
    if (mesh->data->index_array->length > 0) {
      mesh_count++;
      index_count += mesh->data->index_array->length;
      printf("MESH: %s, %d indices", object->name, mesh->data->index_array->length);
    }
  }
  for (int i = 0; object->children != NULL && i < object->children->length; i++) visit(object->children->buffer[i]);
}
void update() {
  frames++;
  if (frames == 2) script_import_asset(${quote(file)}, 0);
  if (frames == 20) {
    object_t *root = script_get_object("Root");
    if (root == NULL) root = script_get_context()->paint_object->base;
    if (string_equals(root->name, "Tessellated")) printf("FAIL: original project mesh remains");
    visit(root);
    printf("RESULT: %d %d", mesh_count, index_count);
    script_quit();
  }
}
void main() { script_set_tools(0); script_notify_on_update(update); }
`);
  const result = await new Promise((resolve, reject) => {
    const child = spawn(exe, ['--background', '--script', script], { cwd: path.dirname(exe), windowsHide: true });
    let output = '';
    child.stdout.on('data', b => { output += b; });
    child.stderr.on('data', b => { output += b; });
    const timer = setTimeout(() => { child.kill(); reject(new Error('Import timed out: ' + output)); }, 60000);
    child.on('error', e => { clearTimeout(timer); reject(e); });
    child.on('exit', code => { clearTimeout(timer); resolve({ code, output }); });
  });
  await writeFile(path.join(caseDir, 'result.json'), JSON.stringify(result, null, 2));
  assert.equal(result.code, 0, `${name}: process exited ${result.code}\n${result.output}`);
  assert(!result.output.includes('FAIL:'), result.output);
  const counts = result.output.match(/RESULT: (\d+) (\d+)/);
  assert(counts && Number(counts[1]) > 0 && Number(counts[2]) > 0, result.output);
  const actual = counts.slice(1).map(Number);
  if (reference) assert.deepEqual(actual, reference, 'Extension changed imported geometry');
  else reference = actual;
  console.log(`PASS: ${name}: ${actual[0]} meshes, ${actual[1] / 3} triangles`);
}
console.log('Logs: ' + dir);
