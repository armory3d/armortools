// node paint/tests/import-framing.mjs [optional FBX fixture] [optional executable]
// Check world-space framing as well as raw import data. Fixtures stay in build output.
// IMPORT_TEST_SCULPT=1 also exercises the deferred sculpt initialization after import.
import { spawn } from 'node:child_process';
import { mkdir, writeFile } from 'node:fs/promises';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import assert from 'node:assert/strict';

const repo = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '../..');
const exe = path.resolve(process.argv[3] || path.join(repo, 'paint/build/out/ArmorPaint.exe'));
const dir = path.join(repo, 'paint/build/tests', `framing-${Date.now()}`);
await mkdir(dir, { recursive: true });
const quote = s => JSON.stringify(s.replaceAll('\\', '/'));
async function fixture(name, centers, point = false) {
  const file = path.join(dir, name+'.OBJ');
  await writeFile(file, centers.map(([x,y,z],k) => `o Part${k}
v ${x} ${y} ${z}
v ${x+(point?0:3)} ${y} ${z}
v ${x} ${y+(point?0:4)} ${z}
vt 0 0
vt 1 0
vt 0 1
vn 0 0 1
f ${k*3+1}/${k*3+1}/${k+1} ${k*3+2}/${k*3+2}/${k+1} ${k*3+3}/${k*3+3}/${k+1}
`).join('\n'));
  return file;
}
const offset = await fixture('offset', [[100,200,300],[110,210,320]]);
const cases = [
  { name:'centered', file:await fixture('centered',[[-1.5,-2,0]]), up:-1, forward:-1 },
  { name:'negative-offset', file:await fixture('negative',[[-100,-200,-300]]), up:-1, forward:-1 },
  { name:'offset-auto', file:offset, up:-1, forward:-1 },
  { name:'point', file:await fixture('point',[[100,200,300]],true), up:-1, forward:-1, point:true },
];
for(let up=0;up<6;up++) for(let forward=0;forward<6;forward++) {
  if(Math.floor(up/2)!==Math.floor(forward/2)) cases.push({name:`offset-${up}-${forward}`,file:offset,up,forward});
}
if(process.argv[2]) {
  cases.push({name:'user-fbx-auto',file:path.resolve(process.argv[2]),up:-1,forward:-1,screenshot:true});
  for(const forward of [0,1,2,3]) cases.push({name:`user-fbx-z-up-${forward}`,file:path.resolve(process.argv[2]),up:4,forward,screenshot:true});
}
const actions = cases.map((c,i) => `
 if(frames==${i*10+2}) script_import_mesh(${quote(c.file)},0,${c.up},${c.forward});
 if(frames==${i*10+8}) {
  report("${c.name}");
  ${c.screenshot ? `script_screenshot(${quote(path.join(dir,c.name+'.png'))});` : ''}
 }
`).join('\n');
const script = path.join(dir,'framing.c');
await writeFile(script, `#include "global.h"
int frames=0;
float x=0,y=0,z=0;
void apply(transform_t *t) {
 x*=t->scale.x; y*=t->scale.y; z*=t->scale.z;
 float tx=2*(t->rot.y*z-t->rot.z*y), ty=2*(t->rot.z*x-t->rot.x*z), tz=2*(t->rot.x*y-t->rot.y*x);
 float nx=x+t->rot.w*tx+t->rot.y*tz-t->rot.z*ty;
 float ny=y+t->rot.w*ty+t->rot.z*tx-t->rot.x*tz;
 float nz=z+t->rot.w*tz+t->rot.x*ty-t->rot.y*tx;
 x=nx+t->loc.x; y=ny+t->loc.y; z=nz+t->loc.z;
}
void report(char *name) {
 float lx=1e20,ly=1e20,lz=1e20,hx=-1e20,hy=-1e20,hz=-1e20;
 int count=0;
 any_array_t *objects=script_get_mesh_objects();
 for(int i=0;i<objects->length;i++) {
  mesh_object_t *m=objects->buffer[i];
  vertex_array_t *pa=mesh_data_get_vertex_array(m->data,"pos");
  for(int j=0;j<m->data->index_array->length;j++) {
   int v=m->data->index_array->buffer[j]*4;
   float s=m->data->scale_pos/32767.0;
   x=pa->values->buffer[v]*s; y=pa->values->buffer[v+1]*s; z=pa->values->buffer[v+2]*s;
   object_t *o=m->base;
   while(o!=NULL) { apply(o->transform); o=o->parent; }
   if(x<lx)lx=x; if(y<ly)ly=y; if(z<lz)lz=z;
   if(x>hx)hx=x; if(y>hy)hy=y; if(z>hz)hz=z;
   count++;
  }
 }
 printf("FRAME %s %d %f %f %f %f %f %f",name,count,lx,ly,lz,hx,hy,hz);
}
void update() { frames++; ${actions} if(frames>${cases.length*10+4})script_quit(); }
void main(){script_set_tools(${process.env.IMPORT_TEST_SCULPT ? 1 : 0});script_notify_on_update(update);}
`);
const result = await new Promise((resolve,reject) => {
  const child=spawn(exe,['--background','--script',script],{cwd:path.dirname(exe),windowsHide:true});
  let output='';
  child.stdout.on('data',b=>{output+=b;}); child.stderr.on('data',b=>{output+=b;});
  const timer=setTimeout(()=>{child.kill();reject(new Error('Framing test timed out\n'+output));},120000);
  child.on('error',e=>{clearTimeout(timer);reject(e);});
  child.on('exit',code=>{clearTimeout(timer);resolve({code,output});});
});
await writeFile(path.join(dir,'result.json'),JSON.stringify(result,null,2));
assert.equal(result.code,0,result.output);
for(const c of cases) {
  const line=result.output.split(/\r?\n/).find(l=>l.startsWith(`FRAME ${c.name} `));
  assert(line,`Missing ${c.name}: ${result.output}`);
  const [count,...b]=line.trim().split(/\s+/).slice(2).map(Number);
  assert(count>0 && b.length===6 && b.every(Number.isFinite),line);
  for(let i=0;i<3;i++) assert(Math.abs((b[i]+b[i+3])/2)<0.002,`Not centered: ${line}`);
  const diagonal=Math.hypot(b[3]-b[0],b[4]-b[1],b[5]-b[2]);
  assert(Math.abs(diagonal-(c.point?0:2))<0.005,`Incorrect size: ${line}`);
  console.log('PASS: '+c.name);
}
console.log('Logs and screenshots: '+dir);
