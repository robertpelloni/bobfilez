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

test('MSI packaging takes its version from VERSION.md', () => {
  const bat = fs.readFileSync(path.join(root, '..', 'scripts', 'package_msi.bat'), 'utf8');
  const wxs = fs.readFileSync(path.join(root, '..', 'wix', 'main.wxs'), 'utf8');
  // Version truth is VERSION.md; the .wxs used to hardcode 2.2.1 and
  // the script named the output 2.2.0 from a legacy build/ tree.
  assert.match(bat, /VERSION\.md/);
  assert.match(bat, /ProductVersion=%VERSION%/);
  assert.match(bat, /filez-%VERSION%-win64\.msi/);
  assert.match(wxs, /Version="\$\(var\.ProductVersion\)"/);
  assert.doesNotMatch(wxs, /Version="2\./);
});

test('fuzzer script disables the default-on Omni shell', () => {
  const bat = fs.readFileSync(path.join(root, '..', 'scripts', 'build_fuzzer.bat'), 'utf8');
  // FO_BUILD_OMNI defaults ON and its BobUI/Qt probe fails any
  // configure on this host; the fuzz lane must opt out explicitly.
  assert.match(bat, /FO_BUILD_OMNI=OFF/);
  assert.match(bat, /FO_BUILD_FUZZERS=ON/);
});

test('no raw delete endpoint remains on the server', () => {
  const srv = read('server.js');
  // /api/delete unlinked arbitrary request paths with no journal or
  // audit, and no frontend called it. It must stay retired; the
  // scoped, journaled delete-duplicates route is the only deleter.
  assert.doesNotMatch(srv, /app\.post\('\/api\/delete'/);
  assert.match(srv, /app\.post\('\/api\/delete-duplicates'/);
});

test('dashboard launch scripts set the OCR tessdata env', () => {
  // A server started from start-server.bat (or a tray spawned by
  // tray.bat) must inherit TESSDATA_PREFIX or OCR silently fails.
  const bat = fs.readFileSync(path.join(root, 'start-server.bat'), 'utf8');
  assert.match(bat, /TESSDATA_PREFIX/);
  assert.match(bat, /build-lean-deps/);
});

test('tray cannot double-spawn or race the port on restart', () => {
  const tray = read('tray.py');
  // An externally started server must not get a duplicate spawn
  // (EADDRINUSE), stop must wait for the port to free before any
  // restart, and tray-spawned servers get the OCR tessdata env.
  assert.match(tray, /already running \(external\)/);
  assert.match(tray, /Wait for the port to actually free up/);
  assert.match(tray, /TESSDATA_PREFIX/);
});

test('history primary button never undoes', () => {
  const html = read('public/unified.html');
  // The GET-branch must not fire the destructive undo endpoint.
  assert.doesNotMatch(html, /if \(sec\.extra === 'undo'\) \{ api\('\/api\/undo'/);
  // Undo still exists, but only on its dedicated danger button.
  assert.match(html, /Undo last operation/);
});

test('ignore card can add and remove rules', () => {
  const html = read('public/unified.html');
  const srv = read('server.js');
  assert.match(html, /\/api\/ignore\/add/);
  // Remove existed server-side long before the card exposed it; the card
  // copy already promised "or remove it" with no button.
  assert.match(html, /\/api\/ignore\/remove/);
  assert.match(html, /Remove rule/);
  assert.match(srv, /app\.post\('\/api\/ignore\/remove'/);
});

test('audit card states the truncation limit honestly', () => {
  const html = read('public/unified.html');
  // Hash chains detect edits to remaining entries, not deletion of
  // the newest ones; the card must not overpromise tamper evidence.
  assert.match(html, /Deleting the newest entries outright is not detectable/);
});

test('vault card warns that lock removes the original', () => {
  const html = read('public/unified.html');
  // VaultManager::lock_file deletes the plaintext source after a
  // successful encrypt; the UI must say so before the click.
  assert.match(html, /REMOVES the original plaintext/);
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

test('media cards declare their provider and get badged when missing', () => {
  const html = read('public/unified.html');
  assert.match(html, /needs:'OCR'/);
  assert.match(html, /needs:'Classifiers'/);
  assert.match(html, /needs:'Perceptual Hash'/);
  assert.match(html, /needs:'Video Hash'/);
  assert.match(html, /needs:'Audio Fingerprint'/);
  assert.match(html, /data-needs/);
  assert.match(html, /\/api\/providers/);
  assert.match(html, /no ' \+ name \+ ' provider/);
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

test('organize card teaches the extension-preserving template', () => {
  const html = read('public/unified.html');
  // {name} drops the extension; the card must teach {filename} so
  // users following its example don't strip extensions off files.
  assert.match(html, /\{ext\}\/\{filename\}/);
  assert.match(html, /\{name\} \(drops extension!\)/);
});

test('dashboard JSON payloads carry schema_version 2', async () => {
  // Golden contract (round-5/6): object-shaped bridge payloads
  // are schema-versioned so CLI shape drift fails here instead
  // of silently changing under the dashboard.
  const stats = await fetch('http://localhost:3199/api/stats', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ paths: [root] }),
  });
  assert.equal(stats.status, 200);
  const sj = await stats.json();
  assert.equal(sj.schema_version, 2);
  assert.equal(typeof sj.total_files, 'number');

  const org = await fetch('http://localhost:3199/api/organize', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ paths: [root], rule: '{ext}/{filename}' }),
  });
  assert.equal(org.status, 200);
  const oj = await org.json();
  assert.equal(oj.schema_version, 2);
  assert.ok(Array.isArray(oj.moves));
});

test('CLI failures surface as typed errors, not empty successes', async () => {
  // runCli used to resolve whenever stdout was non-empty, so a
  // failed run that printed a partial "[]" payload looked like an
  // empty success (e.g. vhash on a folder). Nonzero exits now
  // reject with a typed envelope: CLI exit 1 -> HTTP 422 FAILED,
  // error stays a human string for the legacy React lane.
  const r = await fetch('http://localhost:3199/api/vhash', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ paths: [root] }),
  });
  assert.equal(r.status, 422);
  const j = await r.json();
  assert.equal(j.schema_version, 2);
  assert.equal(j.error_code, 'FAILED');
  assert.equal(j.retryable, false);
  assert.match(j.error, /failed/i);

  // Bad input is a 400 BAD_REQUEST envelope, not a bare string.
  const bad = await fetch('http://localhost:3199/api/scan', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({}),
  });
  assert.equal(bad.status, 400);
  const bj = await bad.json();
  assert.equal(bj.error_code, 'BAD_REQUEST');
});

test('health endpoint answers over HTTP', async () => {
  const r = await fetch('http://localhost:3199/api/health');
  assert.equal(r.status, 200);
  const j = await r.json();
  assert.equal(j.ok, true);
});

test('vault bridges call the real CLI (no blanket 501)', () => {
  const srv = read('server.js');
  assert.match(srv, /'vault-init'/);
  assert.match(srv, /'vault-lock'/);
  assert.match(srv, /'vault-unlock'/);
  assert.match(srv, /'vault-list'/);
  assert.doesNotMatch(srv, /Vault is not wired yet/);
});

test('manifest build supplements vcpkg with the verified lean provider prefix', () => {
  const build = fs.readFileSync(path.join(root, '..', 'scripts', 'build_headless.bat'), 'utf8');
  assert.match(build, /WindowsPowerShell/);
  assert.match(build, /Z_VCPKG_PWSH_PATH/);
  assert.match(build, /build-lean-deps\/x64-windows|build-lean-deps\\x64-windows/);
  assert.match(build, /OpenCV_DIR/);
  assert.match(build, /build-lean-deps\\x64-windows\\bin\\\*\.dll/);
  assert.match(build, /build-lean-deps\\opencv\\x64\\vc16\\bin\\\*\.dll/);
  const juce = fs.readFileSync(path.join(root, '..', 'scripts', 'build_juce_gui.bat'), 'utf8');
  assert.match(juce, /Z_VCPKG_PWSH_PATH/);
});

test('providers and autonomous service status bridges are wired', () => {
  const srv = read('server.js');
  const html = read('public/unified.html');
  assert.match(srv, /\/api\/providers/);
  assert.match(srv, /--list-scanners/);
  assert.match(srv, /--list-vhash/);
  assert.match(srv, /--list-ahash/);
  assert.match(srv, /\/api\/autodev\/status/);
  assert.match(srv, /\['autodev', 'status'\]/);
  assert.match(srv, /\/api\/sync\/status/);
  assert.match(srv, /\['sync', 'status'\]/);
  assert.match(srv, /\/api\/sync\/history/);
  assert.match(srv, /\['sync', 'history'/);
  assert.match(html, /Provider directory/);
  assert.match(html, /Autonomous services/);
  assert.match(html, /Forensic audit ledger/);
});

test('dev_all invokes the maintained wiring test file explicitly', () => {
  const devAll = fs.readFileSync(path.join(root, '..', 'scripts', 'dev_all.bat'), 'utf8');
  assert.match(devAll, /node --test test\/wiring\.test\.js/);
});

test('audit bridge never returns fabricated empty history', () => {
  const srv = read('server.js');
  assert.doesNotMatch(srv, /history: \[\] \}\); \/\/ Stubbed/);
  assert.doesNotMatch(srv, /Stubbed for now/);
});

test('media providers use explicit registration and link swresample', () => {
  const reg = fs.readFileSync(path.join(root, '..', 'core', 'src', 'providers', 'register_all_providers.cpp'), 'utf8');
  assert.match(reg, /register_video_hasher_ffmpeg\(\)/);
  assert.match(reg, /register_audio_fingerprinter_chromaprint\(\)/);
  const cmake = fs.readFileSync(path.join(root, '..', 'core', 'CMakeLists.txt'), 'utf8');
  assert.match(cmake, /FFmpeg::swresample/);
});

test('rename-batch bridge speaks the CLI find/replace grammar', () => {
  const srv = read('server.js');
  assert.match(srv, /--find=\$\{find\}/);
  assert.match(srv, /--replace=\$\{replace\}/);
  assert.doesNotMatch(srv, /--rules=' \+ rules/);
  const html = read('public/unified.html');
  assert.match(html, /Find text/);
});

test('hash accepts a single file path (scanner file roots)', async () => {
  const r = await fetch('http://localhost:3199/api/hash', {
    method: 'POST', headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ paths: ['C:/Users/hyper/workspace/bobfilez/bobui_web/public/app.js'] }),
  });
  assert.equal(r.status, 200);
  const j = await r.json();
  // Schema v2: array payloads are wrapped, never bare roots.
  assert.equal(j.schema_version, 2);
  assert.ok(Array.isArray(j.items) && j.items.length >= 1, 'expected at least one hash result for a file path');
});

test('partial success keeps its per-file payload on failure', async () => {
  // Round 8: a run that finishes partway and exits 1 printed its
  // per-file JSON first. The bridge envelope must carry it as
  // `partial` so the dashboard can show what landed (and offer
  // undo) instead of only an error string.
  const os = require('node:os');
  const tmp = fs.mkdtempSync(path.join(os.tmpdir(), 'bobfilez-partial-'));
  try {
    // The rename bridge (dryRun=false) has the same shape as a
    // real organize: one target pre-exists (failure), the other
    // renames fine, and the CLI prints JSON then exits 1.
    const fix = path.join(tmp, 'fix');
    fs.mkdirSync(fix, { recursive: true });
    // {name} drops the extension, so a.txt and a.md both map to
    // "a": exactly one wins the rename, the other fails with
    // "destination exists" — partial success in any scan order.
    fs.writeFileSync(path.join(fix, 'a.txt'), 'aaa');
    fs.writeFileSync(path.join(fix, 'a.md'), 'amd');
    fs.writeFileSync(path.join(fix, 'c.txt'), 'ccc');
    const r = await fetch('http://localhost:3199/api/rename', {
      method: 'POST', headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ paths: [fix], pattern: '{name}', dryRun: false }),
    });
    assert.equal(r.status, 422);
    const j = await r.json();
    assert.equal(j.error_code, 'FAILED');
    assert.ok(j.partial, 'expected partial payload on the envelope');
    assert.ok(j.partial.failed >= 1, 'expected at least one failed rename');
    assert.ok(j.partial.renamed >= 1, 'expected at least one successful rename');
  } finally {
    fs.rmSync(tmp, { recursive: true, force: true });
  }
});

test('every array bridge route returns the wrapped v2 shape', async () => {
  // Round 9: goldens previously pinned only /api/scan and
  // /api/hash, so other array routes could drift back to bare
  // arrays unnoticed. All of them must be {schema_version: 2,
  // items: [...]}.
  const posts = ['/api/duplicates', '/api/metadata', '/api/lint'];
  for (const route of posts) {
    const r = await fetch('http://localhost:3199' + route, {
      method: 'POST', headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ paths: [root] }),
    });
    assert.equal(r.status, 200, route);
    const j = await r.json();
    assert.ok(!Array.isArray(j), route + ' returned a bare array');
    assert.equal(j.schema_version, 2, route);
    assert.ok(Array.isArray(j.items), route);
  }
  for (const route of ['/api/history', '/api/ignore']) {
    const r = await fetch('http://localhost:3199' + route);
    assert.equal(r.status, 200, route);
    const j = await r.json();
    assert.ok(!Array.isArray(j), route + ' returned a bare array');
    assert.equal(j.schema_version, 2, route);
    assert.ok(Array.isArray(j.items), route);
  }
});

test('no recursive_directory_iterator remains in shipped sources', () => {
  // Round 12 invariant: the recursive iterator family crashed
  // (AV), threw (exit 3), or silently truncated on deep trees
  // in scanner after scanner. All walkers use stack DFS now;
  // a grep is the enforcement, so reintroducing one fails CI.
  const roots = ['../core/src', '../cli', '../core/c_api', '../core/include'];
  const offenders = [];
  const walk = (d) => {
    for (const e of fs.readdirSync(d, { withFileTypes: true })) {
      const p = path.join(d, e.name);
      if (e.isDirectory()) walk(p);
      else if (/\.(cpp|hpp|h)$/.test(e.name)) {
        const text = fs.readFileSync(p, 'utf8');
        // Strip comments crudely: only flag code lines.
        for (const line of text.split('\n')) {
          const t = line.trim();
          if (t.startsWith('//') || t.startsWith('*')) continue;
          if (t.includes('recursive_directory_iterator')) offenders.push(p + ': ' + t.slice(0, 80));
        }
      }
    }
  };
  for (const r of roots) walk(path.join(root, r));
  assert.deepEqual(offenders, []);
});

test('bridge refuses cross-origin and non-loopback callers', async () => {
  // The bridge performs destructive filesystem operations
  // for any page that can reach localhost. A cross-origin
  // POST must be refused before it can delete anything;
  // same-host dashboard calls must keep working.
  const evil = await fetch('http://localhost:3199/api/scan', {
    method: 'POST', headers: { 'Content-Type': 'application/json', Origin: 'https://evil.example' },
    body: JSON.stringify({ paths: [root] }),
  });
  assert.equal(evil.status, 403);
  const ej = await evil.json();
  assert.equal(ej.error_code, 'FORBIDDEN');
  const okRes = await fetch('http://localhost:3199/api/scan', {
    method: 'POST', headers: { 'Content-Type': 'application/json', Origin: 'http://localhost:3199' },
    body: JSON.stringify({ paths: [root] }),
  });
  assert.equal(okRes.status, 200);
  // And the listener itself must be loopback-bound, with
  // bounded subprocess fan-out — origin checks alone are
  // not a bind.
  const srv = read('server.js');
  assert.match(srv, /app\.listen\(PORT, '127\.0\.0\.1'/);
  assert.match(srv, /CLI_MAX_IN_FLIGHT/);
});

test('bridge refuses cross-origin browser posts (Gemini rebinding class)', async () => {
  // A malicious page on another origin gets 403 before any
  // mutation, a rebinding Host is refused, and a simple
  // (text/plain) request body is never parsed into paths.
  const evil = await fetch('http://localhost:3199/api/scan', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json', Origin: 'https://evil.example' },
    body: JSON.stringify({ paths: [root] }),
  });
  assert.equal(evil.status, 403);
  const opaque = await fetch('http://localhost:3199/api/scan', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json', Origin: 'null' },
    body: JSON.stringify({ paths: [root] }),
  });
  assert.equal(opaque.status, 403);
  const simple = await fetch('http://localhost:3199/api/scan', {
    method: 'POST',
    headers: { 'Content-Type': 'text/plain' },
    body: JSON.stringify({ paths: [root] }),
  });
  assert.notEqual(simple.status, 200);
});

test('bridge passes quote-bearing paths as one literal argv', async () => {
  // Gemini quoting class: execFile array-argv means a path
  // containing `"`, `&`, `%VAR%` reaches fo_cli as a single
  // literal path and fails cleanly as not-found — it cannot
  // break out into options or a shell.
  const weird = 'C:\\definitely-missing\\" --dry-run "&%PATH%';
  const r = await fetch('http://localhost:3199/api/scan', {
    method: 'POST', headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ paths: [weird] }),
  });
  const j = await r.json();
  assert.equal(j.schema_version, 2);
  assert.notEqual(r.status, 200);
  assert.ok(j.error_code, 'expected a typed error, not success or a hang');
});

test('bridge rejects dash-prefixed path entries (argument injection)', async () => {
  // A "path" of --db=... would be consumed by fo_cli's
  // global option parser and reprogram the command. The
  // bridge must refuse it as a bad request, and normal
  // paths must keep working.
  const bad = await fetch('http://localhost:3199/api/scan', {
    method: 'POST', headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ paths: ['--db=C:/evil.db', root] }),
  });
  assert.equal(bad.status, 400);
  const bj = await bad.json();
  assert.equal(bj.error_code, 'BAD_REQUEST');
  // No destructive GET endpoints exist: quit is POST-only.
  const srv = read('server.js');
  assert.match(srv, /app\.post\('\/api\/quit'/);
  assert.doesNotMatch(srv, /app\.get\('\/api\/quit'/);
});

test('scan bridge returns wrapped items, not a bare array', async () => {
  const r = await fetch('http://localhost:3199/api/scan', {
    method: 'POST', headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ paths: [root] }),
  });
  assert.equal(r.status, 200);
  const j = await r.json();
  assert.ok(!Array.isArray(j), 'bare array root is a schema-v1 contract break');
  assert.equal(j.schema_version, 2);
  assert.ok(Array.isArray(j.items));
});

test('audit bridge queries the persistent ledger via fo_cli', async () => {
  const srv = read('server.js');
  assert.match(srv, /\['audit', '--format=json'/);
  assert.doesNotMatch(srv, /not exposed by fo_cli yet/);
  const html = read('public/unified.html');
  assert.match(html, /Load audit ledger/);
  const r = await fetch('http://localhost:3199/api/audit');
  assert.equal(r.status, 200);
  const j = await r.json();
  assert.ok(Array.isArray(j.entries));
  assert.equal(typeof j.verified, 'boolean');
});

test('watch status is queryable (heartbeat bridge, not 501)', async () => {
  const srv = read('server.js');
  assert.match(srv, /'watch-status', '--format=json'/);
  assert.doesNotMatch(srv, /Watch status is not queryable/);
  const w = await fetch('http://localhost:3199/api/watch/status');
  assert.equal(w.status, 200);
  const j = await w.json();
  assert.equal(typeof j.watching, 'boolean');
});
