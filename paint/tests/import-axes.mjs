// node paint/tests/import-axes.mjs [path/to/ArmorPaint.exe]
// Generated, asymmetric fixtures exercise source axes, normals, multiple parts and animation.
import { spawn } from 'node:child_process';
import { mkdir, writeFile } from 'node:fs/promises';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import assert from 'node:assert/strict';

const repo = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '../..');
const exe = path.resolve(process.argv[2] || path.join(repo, 'paint/build/out/ArmorPaint.exe'));
const dir = path.join(repo, 'paint/build/tests', `axes-${Date.now()}`);
await mkdir(dir, { recursive: true });
const quote = s => JSON.stringify(s.replaceAll('\\', '/'));
const points = [[1, 2, 3], [4, 2, 3], [1, 6, 3]];
const offset = [10, 20, 30];
const parts = [points, points.map(p => p.map((x, i) => x + offset[i]))];
const cross = (a, b) => [a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0]];
const dot = (a, b) => a.reduce((s, x, i) => s + x*b[i], 0);
const axis = i => [0, 1, 2].map(j => j === Math.floor(i/2) ? (i%2 ? -1 : 1) : 0);
function orient(p, up, forward) {
  if (up === -1) return [p[0], -p[2], p[1]];
  const u = axis(up), f = axis(forward);
  return [dot(p, cross(u, f)), -dot(p, f), dot(p, u)];
}
const obj = path.join(dir, 'parts.ObJ');
await writeFile(obj, parts.map((part, k) => `o Part${k}\n${part.map(p => `v ${p.join(' ')}`).join('\n')}
vt 0 0
vt 1 0
vt 0 1
vn 0 0 1
f ${k*3+1}/${k*3+1}/${k+1} ${k*3+2}/${k*3+2}/${k+1} ${k*3+3}/${k*3+3}/${k+1}
`).join('\n'));

function fbx(up) {
  return `; FBX 7.4.0 project file
FBXHeaderExtension: { FBXHeaderVersion: 1003
 FBXVersion: 7400
}
GlobalSettings: { Version: 1000
 Properties70: {
  P: "UpAxis", "int", "Integer", "", ${up}
  P: "UpAxisSign", "int", "Integer", "", 1
  P: "FrontAxis", "int", "Integer", "", ${up === 1 ? 2 : 1}
  P: "FrontAxisSign", "int", "Integer", "", ${up === 1 ? 1 : -1}
  P: "CoordAxis", "int", "Integer", "", 0
  P: "CoordAxisSign", "int", "Integer", "", 1
 }
}
Objects: {
${parts.map((_, k) => `Geometry: ${100+k}, "Geometry::Part${k}", "Mesh" {
 Vertices: *9 { a: ${points.flat().join(',')} }
 PolygonVertexIndex: *3 { a: 0,1,-3 }
 LayerElementNormal: 0 {
  Version: 101
  Name: ""
  MappingInformationType: "ByPolygonVertex"
  ReferenceInformationType: "Direct"
  Normals: *9 { a: 0,0,1,0,0,1,0,0,1 }
 }
 LayerElementUV: 0 {
  Version: 101
  Name: "UVMap"
  MappingInformationType: "ByPolygonVertex"
  ReferenceInformationType: "Direct"
  UV: *6 { a: 0,0,1,0,0,1 }
 }
 Layer: 0 {
  Version: 100
  LayerElement: { Type: "LayerElementNormal"
   TypedIndex: 0
  }
  LayerElement: { Type: "LayerElementUV"
   TypedIndex: 0
  }
 }
}
Model: ${200+k}, "Model::Part${k}", "Mesh" {
 Version: 232
 Properties70: {
  P: "Lcl Translation", "Lcl Translation", "", "A", ${k ? offset.join(',') : '0,0,0'}
 }
}
`).join('\n')}
}
Connections: {
 C: "OO", 100, 200
 C: "OO", 101, 201
 C: "OO", 200, 0
 C: "OO", 201, 0
}
`;
}
const fbxY = path.join(dir, 'y-up.FBX'), fbxZ = path.join(dir, 'z-up.FbX');
await writeFile(fbxY, fbx(1));
await writeFile(fbxZ, fbx(2));

// Translated and rotated nodes catch conversions applied before node/world transforms.
const bin = Buffer.alloc(3*3*4*2 + 3*2*4);
[...points.flat(), ...Array(3).fill([0,0,1]).flat(), 0,0,1,0,0,1].forEach((v,i) => bin.writeFloatLE(v, i*4));
const gltf = path.join(dir, 'parts.GlTf');
await writeFile(gltf, JSON.stringify({
  asset: { version: '2.0' }, scene: 0, scenes: [{ nodes: [0,1] }],
  nodes: [{ name: 'Part0', mesh: 0 }, { name: 'Part1', mesh: 0, translation: offset, rotation: [Math.SQRT1_2,0,0,Math.SQRT1_2], scale: [2,3,4] }],
  meshes: [{ primitives: [{ attributes: { POSITION: 0, NORMAL: 1, TEXCOORD_0: 2 } }] }],
  buffers: [{ byteLength: bin.length, uri: 'data:application/octet-stream;base64,'+bin.toString('base64') }],
  bufferViews: [{ buffer: 0, byteOffset: 0, byteLength: 36 }, { buffer: 0, byteOffset: 36, byteLength: 36 }, { buffer: 0, byteOffset: 72, byteLength: 24 }],
  accessors: [{ bufferView: 0, componentType: 5126, count: 3, type: 'VEC3', min: [1,2,3], max: [4,6,3] },
              { bufferView: 1, componentType: 5126, count: 3, type: 'VEC3' }, { bufferView: 2, componentType: 5126, count: 3, type: 'VEC2' }]
}));

// Native skin fixture: one weighted and two unweighted vertices, two distinct poses.
const skin = path.join(dir, 'animated.SkIn');
const skinName = Buffer.from('Animated');
const skinBytes = Buffer.alloc(32+8 + 3*8*4 + 3*4 + 3*4*2 + 3*4*4 + 2*12*4);
let cursor = 0;
const uint = x => { skinBytes.writeUInt32LE(x, cursor); cursor += 4; };
const float = x => { skinBytes.writeFloatLE(x, cursor); cursor += 4; };
[0x4e494b53,1,3,3,1,2].forEach(uint); float(100); uint(8);
skinName.copy(skinBytes, cursor); cursor += 8;
points.flat().forEach(float); Array(3).fill([0,0,1]).flat().forEach(float); [0,0,1,0,0,1].forEach(float);
[0,1,2].forEach(uint); cursor += 24;
[1,0,0,0, 0,0,0,0, 0,0,0,0].forEach(float);
[1,0,0,0, 0,1,0,0, 0,0,1,0, 1,0,0,10, 0,1,0,20, 0,0,1,30].forEach(float);
assert.equal(cursor, skinBytes.length);
await writeFile(skin, skinBytes);

const cases = [];
function add(name, file, up, forward, sourceParts = parts, normals = sourceParts.map(() => [0,0,1]), extra = {}) {
  cases.push({ name, file, up, forward, expected: sourceParts.map((ps,k) => ps.map(p => [...orient(p,up,forward), ...orient(normals[k],up,forward)])), ...extra });
}
for (let up=0; up<6; up++) for (let forward=0; forward<6; forward++) {
  if (Math.floor(up/2) !== Math.floor(forward/2)) add(`obj-${up}-${forward}`, obj, up, forward);
}
add('obj-auto', obj, -1, -1);
add('fbx-y-auto', fbxY, -1, -1);
add('fbx-z-auto', fbxZ, 4, 3, parts, undefined, { up: -1, forward: -1 });
add('fbx-y-manual', fbxY, 4, 3);
add('fbx-z-manual', fbxZ, 2, 4);
const gltfParts = [points, points.map(([x,y,z]) => [2*x+10, -4*z+20, 3*y+30])];
add('gltf-auto', gltf, -1, -1, gltfParts, [[0,0,1],[0,-1,0]]);
add('gltf-manual', gltf, 4, 3, gltfParts, [[0,0,1],[0,-1,0]]);
add('skin-manual', skin, 2, 4, [points], [[0,0,1]]);
add('skin-next-frame', skin, 2, 4, [[points[0].map((x,i)=>x+offset[i]), ...points.slice(1)]], [[0,0,1]], { pose: 1 });
add('append-base', obj, 4, 3);
add('append-oriented', fbxY, 2, 4, parts, undefined, { append: true });
const baseExpected = cases.at(-2).expected;
cases.at(-1).expected = [...baseExpected, ...cases.at(-1).expected];
cases.push({ ...cases.at(-1), name: 'invalid-axes-preserve', up: 4, forward: 5, expected: cases.at(-1).expected });
cases.push({ ...cases.at(-1), name: 'failed-import-preserve', file: path.join(dir, 'missing.FBX'), up: 4, forward: 3 });

let actions = '';
cases.forEach((c,i) => {
  const frame = i*10+2;
  actions += `if (frames == ${frame}) {
    ${c.pose === undefined ? `script_import_mesh(${quote(c.file)}, ${c.append ? 1 : 0}, ${c.up}, ${c.forward});` : `project_reskin_mesh(${c.pose});`}
  }
  if (frames == ${frame+6}) {
    printf("CASE ${c.name}");
    ${c.name === 'skin-next-frame' ? 'project_reskin_mesh(1);' : ''}
    object_t *root = script_get_object("Root");
    if (root == NULL) root = script_get_context()->paint_object->base;
    // Appended objects live beside Root. Visit the scene parent, excluding merged render geometry.
    if (root->parent != NULL) root = root->parent;
    visit(root);
    printf("END ${c.name}");
  }\n`;
});
const script = path.join(dir, 'axes.c');
await writeFile(script, `#include "global.h"
int frames = 0;
void visit(object_t *o) {
  if (string_equals(o->ext_type, "mesh_object_t") && !ends_with(o->name, "_merged")) {
    mesh_object_t *m = o->ext;
    if (m->data->index_array->length > 0 && (starts_with(o->name, "Part") || starts_with(o->name, "Animated"))) {
      vertex_array_t *pa = mesh_data_get_vertex_array(m->data, "pos");
      vertex_array_t *na = mesh_data_get_vertex_array(m->data, "nor");
      printf("MESH %s %d", o->name, m->data->index_array->length);
      for (int i=0; i<m->data->index_array->length; i++) {
        int v = m->data->index_array->buffer[i];
        float s = m->data->scale_pos / 32767.0;
        printf("V %f %f %f %f %f %f", pa->values->buffer[v*4]*s, pa->values->buffer[v*4+1]*s, pa->values->buffer[v*4+2]*s,
          na->values->buffer[v*2]/32767.0, na->values->buffer[v*2+1]/32767.0, pa->values->buffer[v*4+3]/32767.0);
      }
    }
  }
  for (int i=0; o->children != NULL && i<o->children->length; i++) visit(o->children->buffer[i]);
}
void update() {
  frames++;
  ${actions}
  if (frames > ${cases.length*10+2}) script_quit();
}
void main() { script_set_tools(0); script_notify_on_update(update); }
`);
const result = await new Promise((resolve, reject) => {
  const child = spawn(exe, ['--background','--script',script], { cwd: path.dirname(exe), windowsHide: true });
  let output = '';
  child.stdout.on('data', b => { output += b; });
  child.stderr.on('data', b => { output += b; });
  const timer = setTimeout(() => { child.kill(); reject(new Error('Timed out\n'+output)); }, 120000);
  child.on('error', e => { clearTimeout(timer); reject(e); });
  child.on('exit', code => { clearTimeout(timer); resolve({ code, output }); });
});
await writeFile(path.join(dir,'result.json'), JSON.stringify(result,null,2));
assert.equal(result.code, 0, result.output);
result.output = result.output.replaceAll('\r', '');
for (const c of cases) {
  const begin = result.output.indexOf(`CASE ${c.name}\n`);
  const end = result.output.indexOf(`END ${c.name}`,begin);
  assert(begin >= 0 && end > begin, `Missing ${c.name}: ${result.output}`);
  const output = result.output.slice(begin,end);
  const actual = [...output.matchAll(/^V (.+)$/gm)].map(m => m[1].trim().split(/\s+/).map(Number));
  const expected = c.expected.flat();
  assert.equal(actual.length, expected.length, `${c.name}: geometry count\n${output}`);
  for (const v of expected) {
    const match = actual.findIndex(a => a.every((x,i) => Math.abs(x-v[i]) < 0.01));
    assert(match >= 0, `${c.name}: missing vertex/normal ${v}\n${output}`);
    actual.splice(match,1);
  }
  console.log('PASS: '+c.name);
}
console.log('Logs: '+dir);
