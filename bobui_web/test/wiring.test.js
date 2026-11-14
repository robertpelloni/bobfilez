// Durable wiring checks for the unified dashboard (node:test, no new deps).
// Run: node --test test/  (from bobui_web/)
const { test, before, after } = require('node:test');
const assert = require('node:assert');
const fs = require('node:fs');
const path = require('node:path');
const { spawn } = require('node:child_process');

const root = path.join(__dirname, '..');
const read = (p) => fs.readFileSync(path.join(root, p), 'utf8');

test('tray polls the real health/quit routes', () => {
  const tray = read('tray.py');
  assert.match(tray, /\/api\/health/);
  assert.match(tray, /\/api\/quit/);
  assert.doesNotMatch(tray, /api\("\/health"\)/);
  assert.doesNotMatch(tray, /api\("\/quit"/);
});

test('history primary button never undoes', () => {
  const html = read('public/unified.html');
  // The GET-branch must not fire the destructive undo endpoint.
  assert.doesNotMatch(html, /if \(sec\.extra === 'undo'\) \{ api\('\/api\/undo'/);
  // Undo still exists, but only on its dedicated danger button.
  assert.match(html, /Undo last operation/);
});

test('clerk/oracle bridges match the real CLI grammar', () => {
  const srv = read('server.js');
  assert.match(srv, /\['clerk', 'status'\]/);
  assert.doesNotMatch(srv, /\['clerk', '--format=json'/);
  assert.match(srv, /\['oracle', question\]/);
  assert.doesNotMatch(srv, /\['oracle', question, '--format=json'/);
});

test('server prefers the newest CLI build, not the first existing one', () => {
  const srv = read('server.js');
  assert.match(srv, /function pickCli/);
  assert.match(srv, /mtimeMs/);
});

test('dashboard surfaces HTTP errors instead of logging OK', () => {
  const html = read('public/unified.html');
  assert.match(html, /if \(!r\.ok\)/);
});

let child;
before(async () => {
  child = spawn(process.execPath, ['server.js'], {
    cwd: root,
    env: { ...process.env, PORT: '3199' },
    stdio: 'ignore',
  });
  for (let i = 0; i < 40; i++) {
    try { const r = await fetch('http://localhost:3199/api/health'); if (r.ok) return; } catch {}
    await new Promise((r) => setTimeout(r, 250));
  }
  throw new Error('server did not come up on :3199');
});
after(() => { if (child) child.kill(); });

test('health endpoint answers over HTTP', async () => {
  const r = await fetch('http://localhost:3199/api/health');
  assert.equal(r.status, 200);
  const j = await r.json();
  assert.equal(j.ok, true);
});

test('honest 501 endpoints stay honest (vault, watch)', async () => {
  const v = await fetch('http://localhost:3199/api/vault/list', {
    method: 'POST', headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ vaultPath: 'x', password: 'y' }),
  });
  assert.equal(v.status, 501);
  const w = await fetch('http://localhost:3199/api/watch/status');
  assert.equal(w.status, 501);
});
