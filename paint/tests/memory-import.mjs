// Windows process/GPU memory across repeated imports and project resets.
// node paint/tests/memory-import.mjs model.FBX|shape:plane_2048 [ArmorPaint.exe]
// MEMORY_MEASURE_ONLY=1 records a baseline without enforcing growth limits.
import {spawn, execFile} from 'node:child_process';
import {mkdir, writeFile, access} from 'node:fs/promises';
import {promisify} from 'node:util';
import {fileURLToPath} from 'node:url';
import path from 'node:path';
import assert from 'node:assert/strict';

assert(process.argv[2], 'Supply a local mesh fixture or shape:name');
const repo = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '../..');
const fixture = path.resolve(process.argv[2]);
const shape = process.argv[2].startsWith('shape:') ? process.argv[2].slice(6) : null;
const exe = path.resolve(process.argv[3] || path.join(repo, 'paint/build/out/ArmorPaint.exe'));
const dir = path.join(repo, 'paint/build/tests', `memory-import-${Date.now()}`);
await mkdir(dir, {recursive: true});
const quote = value => JSON.stringify(value.replaceAll('\\', '/'));
const script = path.join(dir, 'measure.c');
await writeFile(script, `#include "global.h"
int phase=0, frames=0;
object_t *shape = NULL;
void update() {
 frames++;
 if(frames==30) iron_file_save_bytes(string(${quote(dir+'/ready-%d')},phase),sys_string_to_buffer("ready"),0);
 if(frames>30 && iron_file_exists(string(${quote(dir+'/advance-%d')},phase))) {
  phase++;frames=0;
  if(phase==9){printf("PASS: repeated imports and resets");script_quit();return;}
  if(phase%2==1) {
   ${shape ? `shape=script_shape_add(${quote(shape)}); if(shape==NULL){printf("FAIL: shape creation");script_quit();return;}` : `script_import_mesh(${quote(fixture)},0,-1,-1);`}
  }
  else {
   ${shape ? 'if(phase%4==2) script_object_remove(shape); else script_project_new(); shape=NULL;' : 'script_project_new();'}
  }
 }
}
void main(){script_set_tools(0);script_notify_on_update(update);}
`);
const child = spawn(exe, ['--background', '--script', script], {cwd: path.dirname(exe), windowsHide: true});
let output='', exited=false, exitCode;
child.stdout.on('data', b=>output+=b);
child.stderr.on('data', b=>output+=b);
child.on('exit', code=>{exited=true;exitCode=code;});
child.on('error', error=>{output+=error.message;exited=true;});
const delay = ms=>new Promise(resolve=>setTimeout(resolve,ms));
const exec = promisify(execFile);
const samples=[];
try {
 for(let phase=0;phase<9;phase++) {
  const deadline=Date.now()+60000;
  for(;;) {
   assert(!exited, `App exited (${exitCode}): ${output}`);
   assert(Date.now()<deadline, `Timeout: ${output}`);
   try {await access(path.join(dir,`ready-${phase}`));break;} catch {}
   await delay(100);
  }
  await delay(1500);
  const command=`$app=Get-Process -Id ${child.pid}; $gpu=@(Get-CimInstance Win32_PerfFormattedData_GPUPerformanceCounters_GPUProcessMemory | Where-Object { $_.Name -like 'pid_${child.pid}_*' }); [pscustomobject]@{private=$app.PrivateMemorySize64;working=$app.WorkingSet64;committed=($gpu | Measure-Object TotalCommitted -Sum).Sum;dedicated=($gpu | Measure-Object DedicatedUsage -Sum).Sum;shared=($gpu | Measure-Object SharedUsage -Sum).Sum} | ConvertTo-Json -Compress`;
  const {stdout}=await exec('powershell.exe',['-NoProfile','-Command',command],{windowsHide:true,timeout:15000});
  const sample={phase,...JSON.parse(stdout)};
  assert(Number.isFinite(sample.private) && sample.committed>0, 'Memory counters unavailable');
  samples.push(sample);
  console.log(JSON.stringify({...sample, state:phase===0?'startup':phase%2?'import':'reset'}));
  await writeFile(path.join(dir,`advance-${phase}`),'go');
 }
 const deadline=Date.now()+10000;
 while(!exited && Date.now()<deadline) await delay(100);
 assert(exited && exitCode===0 && output.includes('PASS:') && !/error:|FAIL/i.test(output),output);
 if(!process.env.MEMORY_MEASURE_ONLY) {
  assert(samples[8].private < samples[2].private+32*1024*1024, 'CPU memory grows across repeated import/reset cycles');
  assert(samples[8].committed < samples[2].committed+16*1024*1024, 'GPU memory grows across repeated import/reset cycles');
 }
 console.log('PASS: memory samples recorded: '+dir);
} finally {
 if(!exited) child.kill();
 await writeFile(path.join(dir,'results.json'),JSON.stringify({exe,shape,samples,output},null,2));
}
