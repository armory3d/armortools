// Windows GPU-memory regression: fill the undo ring, reset the project, then paint again.
// Run after building: node paint/tests/memory.mjs [path/to/ArmorPaint.exe]
import { spawn, execFile } from 'node:child_process';
import { mkdir, writeFile, access } from 'node:fs/promises';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { promisify } from 'node:util';
import assert from 'node:assert/strict';
const exec = promisify(execFile);
const repo = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '../..');
const exe = path.resolve(process.argv[2] || path.join(repo, 'paint/build/out/ArmorPaint.exe'));
const dir = path.join(repo, 'paint/build/tests', `memory-${Date.now()}`);
await mkdir(dir, { recursive: true });
const quote = s => JSON.stringify(s.replaceAll('\\', '/'));
const script = path.join(dir, 'memory.c');
await writeFile(script, `#include "global.h"
int phase = 0;
int frames = 0;
void mark(int n) {
  iron_file_save_bytes(string(${quote(dir + '/ready-%d')}, n), sys_string_to_buffer("ready"), 0);
}
void update() {
  frames++;
  if (phase == 0 && frames == 10) { mark(0); phase = 1; }
  if (phase > 0 && iron_file_exists(string(${quote(dir + '/advance-%d')}, phase - 1))) {
    if (phase == 1 || phase == 3) {
      for (int i = 0; i < script_get_config()->undo_steps; i++) script_layer_clear(script_layer_at(0));
    }
    if (phase == 2 || phase == 4) script_project_new();
    if (phase == 5) { printf("PASS: paint/reset cycles completed"); script_quit(); return; }
    mark(phase);
    phase++;
  }
}
void main() {
  script_set_tools(1); // Sculpt mode also checks disposal of the old layer's RGBA128 texture.
  script_notify_on_update(update);
}
`);

const child = spawn(exe, ['--background', '--script', script], { cwd: path.dirname(exe), windowsHide: true });
let output = '', exited = false, exitCode;
child.stdout.on('data', b => { output += b; });
child.stderr.on('data', b => { output += b; });
child.on('exit', code => { exited = true; exitCode = code; });
const delay = ms => new Promise(r => setTimeout(r, ms));
const samples = [];
try {
  for (let phase = 0; phase < 5; phase++) {
    const deadline = Date.now() + 60000;
    for (;;) {
      assert(!exited, `App exited (${exitCode}): ${output}`);
      assert(Date.now() < deadline, 'App timed out: ' + output);
      try { await access(path.join(dir, 'ready-' + phase)); break; } catch {}
      await delay(100);
    }
    await delay(1500); // Allow GPU destruction and Windows performance counters to settle.
    const command = `Get-CimInstance Win32_PerfFormattedData_GPUPerformanceCounters_GPUProcessMemory | Where-Object { $_.Name -like 'pid_${child.pid}_*' } | Select-Object DedicatedUsage,SharedUsage,TotalCommitted | ConvertTo-Json -Compress`;
    const { stdout } = await exec('powershell.exe', ['-NoProfile', '-Command', command], { windowsHide: true, timeout: 15000 });
    assert(stdout.trim(), 'No GPU memory counters available for this process');
    const raw = JSON.parse(stdout);
    const entries = Array.isArray(raw) ? raw : [raw];
    const sample = { phase, dedicated: 0, shared: 0, committed: 0 };
    for (const e of entries) {
      sample.dedicated += Number(e.DedicatedUsage);
      sample.shared += Number(e.SharedUsage);
      sample.committed += Number(e.TotalCommitted);
    }
    samples.push(sample);
    console.log(sample);
    await writeFile(path.join(dir, 'advance-' + phase), 'go');
  }
  const deadline = Date.now() + 10000;
  while (!exited && Date.now() < deadline) await delay(100);
  assert(exited && exitCode === 0 && output.includes('PASS:'), output);
  await writeFile(path.join(dir, 'results.json'), JSON.stringify({ exe, samples, output }, null, 2));
  if (!process.env.ALLOW_RETAINED_UNDO) {
    assert(samples[1].committed > samples[2].committed + 32 * 1024 * 1024, 'Reset did not release undo memory');
    assert(samples[3].committed > samples[4].committed + 32 * 1024 * 1024, 'Second reset did not release undo memory');
    assert(samples[4].committed < samples[2].committed + 16 * 1024 * 1024, 'Sculpt memory grew between project resets');
  }
  console.log('PASS: GPU memory measured across two paint/reset cycles; ' + dir);
} finally {
  if (!exited) child.kill();
  await writeFile(path.join(dir, 'app.log'), output);
}
