const express = require('express');
const { execFile } = require('child_process');
const path = require('path');
const fs = require('fs');

const app = express();
app.use(express.json());
app.use(express.static(path.join(__dirname, 'public')));

const CLI_CANDIDATES = [
    path.resolve(__dirname, '..', 'build-msvc', 'cli', 'fo_cli.exe'),
    path.resolve(__dirname, '..', 'build', 'cli', 'fo_cli.exe'),
    path.resolve(__dirname, '..', 'build-mingw', 'cli', 'fo_cli.exe'),
    path.resolve(__dirname, '..', 'build-lean', 'cli', 'fo_cli.exe')
];
// Prefer the most recently built binary: a stale build-msvc tree must not
// shadow a fresh build-mingw one (bit us when build-msvc was still v6.3.0
// while source was v6.5.1 and clerk/oracle dispatch was missing).
function pickCli(candidates) {
    let best = null, bestMtime = -1;
    for (const c of candidates) {
        try {
            const m = fs.statSync(c).mtimeMs;
            if (m > bestMtime) { bestMtime = m; best = c; }
        } catch { /* candidate not built */ }
    }
    return best || candidates[0];
}
const CLI = pickCli(CLI_CANDIDATES);

function runCli(args, timeout = 120000) {
    if (!fs.existsSync(CLI)) {
        return Promise.reject(new Error(`CLI not found. Looked for: ${CLI_CANDIDATES.join(', ')}`));
    }
    return new Promise((resolve, reject) => {
        execFile(CLI, args, { maxBuffer: 50 * 1024 * 1024, timeout }, (err, stdout, stderr) => {
            if (err && err.killed) return reject(new Error('Command timed out'));
            if (err && !stdout) return reject(new Error(stderr || err.message));
            resolve({ stdout: stdout.trim(), stderr: stderr.trim() });
        });
    });
}

// fo_cli prints service chatter (e.g. "[SwarmHub] ...") to stdout ahead of and
// behind the JSON payload. Extract the payload: start at the first line that
// opens JSON, then cut at the matching close bracket. Log lines open with
// "[Name]"; payload lines open with { or [ (bare counts like "42" have no
// brackets, so fall back to the last non-empty line for those).
function extractJson(stdout) {
    const text = (stdout || "").trim();
    if (!text) return text;
    const lines = text.split(/\r?\n/);
    let start = -1;
    for (let i = 0; i < lines.length; i++) {
        const l = lines[i];
        if (/^\s*\{/.test(l) || /^\s*\[$/.test(l) || /^\s*\[[{"\d\s\]]/.test(l)) {
            if (/^\s*\[[A-Za-z][A-Za-z0-9 _-]{0,39}\](\s|$)/.test(l)) continue;
            start = i;
            break;
        }
    }
    if (start < 0) {
        for (let i = lines.length - 1; i >= 0; i--) {
            const l = lines[i].trim();
            if (!l) continue;
            if (/^\[[A-Za-z][A-Za-z0-9 _-]{0,39}\](\s|$)/.test(l)) continue;
            return l;
        }
        return "";
    }
    const candidate = lines.slice(start).join("\n").trim();
    let depth = 0, inStr = false, esc = false;
    for (let i = 0; i < candidate.length; i++) {
        const c = candidate[i];
        if (inStr) {
            if (esc) esc = false;
            else if (c === "\\") esc = true;
            else if (c === '"') inStr = false;
        } else if (c === '"') {
            inStr = true;
        } else if (c === "{" || c === "[") {
            depth++;
        } else if (c === "}" || c === "]") {
            depth--;
            if (depth === 0) return candidate.slice(0, i + 1);
        }
    }
    return candidate;
}
app.get('/api/health', async (req, res) => {
    res.json({
        ok: true,
        cli: CLI,
        cliExists: fs.existsSync(CLI),
        frontendLanes: ['classic-web', 'react-web', 'qt', 'bobui', 'juce', 'btk', 'bobgui']
    });
});

// ── Scan ──
app.post('/api/scan', async (req, res) => {
    try {
        const { paths = [], exts, minSize, maxSize, exclude, recursive = true } = req.body;
        if (!paths.length) return res.status(400).json({ error: 'No paths provided' });
        const args = ['scan', '--format=json', ...paths];
        if (exts) args.push(`--ext=${exts}`);
        if (minSize) args.push(`--min-size=${minSize}`);
        if (maxSize) args.push(`--max-size=${maxSize}`);
        if (exclude) exclude.forEach(p => args.push(`--exclude=${p}`));
        if (!recursive) args.push('--no-recursive');
        const { stdout } = await runCli(args);
        res.json(JSON.parse(extractJson(stdout) || '[]'));
    } catch (e) { res.status(500).json({ error: e.message }); }
});

// ── Duplicates ──
app.post('/api/duplicates', async (req, res) => {
    try {
        const { paths = [], mode = 'fast', threads = 1, exts, minSize, maxSize, exclude, recursive = true } = req.body;
        if (!paths.length) return res.status(400).json({ error: 'No paths provided' });
        const args = ['duplicates', '--format=json', `--mode=${mode}`, `--threads=${threads}`, ...paths];
        if (exts) args.push(`--ext=${exts}`);
        if (minSize) args.push(`--min-size=${minSize}`);
        if (maxSize) args.push(`--max-size=${maxSize}`);
        if (exclude) exclude.forEach(p => args.push(`--exclude=${p}`));
        if (!recursive) args.push('--no-recursive');
        const { stdout } = await runCli(args);
        res.json(JSON.parse(extractJson(stdout) || '[]'));
    } catch (e) { res.status(500).json({ error: e.message }); }
});

// ── Stats ──
app.post('/api/stats', async (req, res) => {
    try {
        const { paths = [] } = req.body;
        if (!paths.length) return res.status(400).json({ error: 'No paths provided' });
        const args = ['stats', '--format=json', ...paths];
        const { stdout } = await runCli(args);
        res.json(JSON.parse(extractJson(stdout) || '{}'));
    } catch (e) { res.status(500).json({ error: e.message }); }
});

// ── Hash ──
app.post('/api/hash', async (req, res) => {
    try {
        const { paths = [], threads = 1 } = req.body;
        if (!paths.length) return res.status(400).json({ error: 'No paths provided' });
        const args = ['hash', '--format=json', `--threads=${threads}`, ...paths];
        const { stdout } = await runCli(args);
        res.json(JSON.parse(extractJson(stdout) || '[]'));
    } catch (e) { res.status(500).json({ error: e.message }); }
});

// ── Metadata ──
app.post('/api/metadata', async (req, res) => {
    try {
        const { paths = [], exts } = req.body;
        if (!paths.length) return res.status(400).json({ error: 'No paths provided' });
        const args = ['metadata', '--format=json', ...paths];
        if (exts) args.push(`--ext=${exts}`);
        const { stdout } = await runCli(args);
        res.json(JSON.parse(extractJson(stdout) || '[]'));
    } catch (e) { res.status(500).json({ error: e.message }); }
});

// ── Delete files ──
app.post('/api/delete', async (req, res) => {
    try {
        const { files = [] } = req.body;
        if (!files.length) return res.status(400).json({ error: 'No files provided' });
        const results = [];
        for (const f of files) {
            try {
                fs.unlinkSync(f);
                results.push({ path: f, success: true });
            } catch (e) {
                results.push({ path: f, success: false, error: e.message });
            }
        }
        res.json({ deleted: results.filter(r => r.success).length, results });
    } catch (e) { res.status(500).json({ error: e.message }); }
});

// ── History ──
app.get('/api/history', async (req, res) => {
    try {
        const args = ['history', '--format=json'];
        const { stdout } = await runCli(args);
        res.json(JSON.parse(extractJson(stdout) || '[]'));
    } catch (e) { res.status(500).json({ error: e.message }); }
});

// ── Ignore rules ──
app.get('/api/ignore', async (req, res) => {
    try {
        const { stdout } = await runCli(['ignore', '--format=json']);
        res.json(JSON.parse(extractJson(stdout) || '[]'));
    } catch (e) { res.status(500).json({ error: e.message }); }
});

app.post('/api/ignore/add', async (req, res) => {
    try {
        const { pattern, reason = '' } = req.body;
        if (!pattern) return res.status(400).json({ error: 'No pattern' });
        const args = ['ignore', 'add', pattern];
        if (reason) args.push(reason);
        await runCli(args);
        res.json({ success: true, pattern });
    } catch (e) { res.status(500).json({ error: e.message }); }
});

app.post('/api/ignore/remove', async (req, res) => {
    try {
        const { pattern } = req.body;
        if (!pattern) return res.status(400).json({ error: 'No pattern' });
        await runCli(['ignore', 'remove', pattern]);
        res.json({ success: true, pattern });
    } catch (e) { res.status(500).json({ error: e.message }); }
});

// ── Lint ──
app.post('/api/lint', async (req, res) => {
    try {
        const { paths = [] } = req.body;
        if (!paths.length) return res.status(400).json({ error: 'No paths provided' });
        const args = ['lint', '--format=json', ...paths];
        const { stdout } = await runCli(args);
        res.json(JSON.parse(extractJson(stdout) || '[]'));
    } catch (e) { res.status(500).json({ error: e.message }); }
});

// ── Count (quick) ──
app.post('/api/count', async (req, res) => {
    try {
        const { paths = [] } = req.body;
        if (!paths.length) return res.status(400).json({ error: 'No paths' });
        const { stdout } = await runCli(['scan', '--count', ...paths]);
        res.json({ count: parseInt(extractJson(stdout), 10) || 0 });
    } catch (e) { res.status(500).json({ error: e.message }); }
});

// ── Prune (v3.0.0 feature) ──
app.post('/api/prune', async (req, res) => {
    try {
        const { paths = [] } = req.body;
        if (!paths.length) return res.status(400).json({ error: 'No paths' });
        const { stdout } = await runCli(['stats', '--prune', ...paths]);
        res.json({ success: true, details: stdout });
    } catch (e) { res.status(500).json({ error: e.message }); }
});

// ── Audit (v3.0.0 feature) ──
app.get('/api/audit', async (req, res) => {
    try {
        // Migration 5 audit log
        res.json({ history: [] }); // Stubbed for now
    } catch (e) { res.status(500).json({ error: e.message }); }
});

// ── Flow (OmniFlow Automation) ──
app.get('/api/flow/list', async (req, res) => {
    try {
        const { stdout } = await runCli(['flow', 'list', '--format=json']);
        res.json(JSON.parse(extractJson(stdout) || '{"workflows":[]}'));
    } catch (e) { res.status(500).json({ error: e.message }); }
});

app.post('/api/flow/run', async (req, res) => {
    try {
        const { workflow_id, payload_path } = req.body;
        if (!workflow_id || !payload_path) return res.status(400).json({ error: 'workflow_id and payload_path required' });
        const { stdout } = await runCli(['flow', 'run', workflow_id, payload_path, '--format=json']);
        res.json(JSON.parse(extractJson(stdout) || '{"success":false}'));
    } catch (e) { res.status(500).json({ error: e.message }); }
});

// ── Scrub (Integrity Verification) ──
app.post('/api/scrub', async (req, res) => {
    try {
        const { paths = [] } = req.body;
        if (!paths.length) return res.status(400).json({ error: 'No paths provided' });
        const { stdout } = await runCli(['scrub', '--format=json', ...paths]);
        res.json(JSON.parse(extractJson(stdout) || '{"scanned":0,"corrupted":0}'));
    } catch (e) { res.status(500).json({ error: e.message }); }
});

// ── Search ──
app.post('/api/search', async (req, res) => {
    try {
        const { query, paths = [], content = false, regex = false, wildcard = false, caseSensitive = false, wholeWord = false, invert = false, maxResults } = req.body;
        if (!query) return res.status(400).json({ error: 'No query provided' });
        const args = ['search', query, '--format=json'];
        if (content) args.push('--content');
        if (regex) args.push('--regex');
        if (wildcard) args.push('--wildcard');
        if (caseSensitive) args.push('--case-sensitive');
        if (wholeWord) args.push('--whole-word');
        if (invert) args.push('--invert');
        if (maxResults) args.push(`--max-results=${parseInt(maxResults, 10) || 0}`);
        args.push(...paths);
        const { stdout } = await runCli(args);
        res.json(JSON.parse(extractJson(stdout) || '{"results":[]}'));
    } catch (e) { res.status(500).json({ error: e.message }); }
});

// ── Export ──
app.post('/api/export', async (req, res) => {
    try {
        const { paths = [], format = 'json' } = req.body;
        if (!paths.length) return res.status(400).json({ error: 'No paths provided' });
        const args = ['export', `--format=${format}`, ...paths];
        const { stdout } = await runCli(args);
        if (format === 'json') {
            res.json(JSON.parse(extractJson(stdout) || '{}'));
        } else {
            res.type(format === 'csv' ? 'text/csv' : 'text/html').send(stdout);
        }
    } catch (e) { res.status(500).json({ error: e.message }); }
});

// ── Organize (Dry Run) ──
app.post('/api/organize', async (req, res) => {
    try {
        const { paths = [], rule = '' } = req.body;
        if (!paths.length || !rule) return res.status(400).json({ error: 'paths and rule required' });
        const args = ['organize', '--rule=' + rule, '--dry-run', '--format=json', ...paths];
        const { stdout } = await runCli(args);
        res.json(JSON.parse(extractJson(stdout) || '{"moves":[]}'));
    } catch (e) { res.status(500).json({ error: e.message }); }
});


// ── Vault ──
// Vault bridges call the real fo_cli vault-* commands (AES-256-GCM via
// VaultManager). Builds without OpenSSL fail honestly (CLI exit 1 → 500
// with the CLI message) instead of the old blanket 501.
async function vaultRun(res, cliArgs) {
    const { stdout } = await runCli(cliArgs, 60000);
    res.json({ success: true, raw: stdout });
}
app.post('/api/vault/init', async (req, res) => {
    try {
        const { vaultPath = '', password = '' } = req.body;
        if (!vaultPath || !password) return res.status(400).json({ error: 'vaultPath and password required' });
        await vaultRun(res, ['vault-init', vaultPath, password]);
    } catch (e) { res.status(500).json({ error: e.message }); }
});

app.post('/api/vault/lock', async (req, res) => {
    try {
        const { vaultPath = '', password = '', file = '' } = req.body;
        if (!vaultPath || !password || !file) return res.status(400).json({ error: 'vaultPath, password, and file required' });
        await vaultRun(res, ['vault-lock', vaultPath, password, file]);
    } catch (e) { res.status(500).json({ error: e.message }); }
});

app.post('/api/vault/unlock', async (req, res) => {
    try {
        const { vaultPath = '', password = '', vaultId = '', dest = '' } = req.body;
        if (!vaultPath || !password || !vaultId || !dest) return res.status(400).json({ error: 'vaultPath, password, vaultId, and dest required' });
        await vaultRun(res, ['vault-unlock', vaultPath, password, vaultId, dest]);
    } catch (e) { res.status(500).json({ error: e.message }); }
});

app.post('/api/vault/list', async (req, res) => {
    try {
        const { vaultPath = '', password = '' } = req.body;
        if (!vaultPath || !password) return res.status(400).json({ error: 'vaultPath and password required' });
        await vaultRun(res, ['vault-list', vaultPath, password]);
    } catch (e) { res.status(500).json({ error: e.message }); }
});


app.post('/api/delete-duplicates', async (req, res) => {
  try {
    const { path: dirPath, dryRun = true } = req.body;
    if (!dirPath) return res.status(400).json({ error: 'path required' });
    const args = ['delete-duplicates', '--format=json', dirPath];
    if (dryRun) args.push('--dry-run');
    const { stdout } = await runCli(args);
    res.json(JSON.parse(extractJson(stdout) || '{}'));
  } catch (e) { res.status(500).json({ error: e.message }); }
});

// ── Consolidated-UI Phase 1: missing CLI bridges (added 2026-11-03) ──
app.get('/api/modules', async (req, res) => {
    try {
        const { stdout } = await runCli(['--modules']);
        res.json({ raw: stdout });
    } catch (e) { res.status(500).json({ error: e.message }); }
});
app.post('/api/ocr', async (req, res) => {
    try {
        const { paths = [] } = req.body;
        if (!paths.length) return res.status(400).json({ error: 'No paths provided' });
        const { stdout } = await runCli(['ocr', '--format=json', ...paths]);
        try { res.json(JSON.parse(extractJson(stdout) || '[]')); } catch { res.json({ raw: stdout }); }
    } catch (e) { res.status(500).json({ error: e.message }); }
});
app.post('/api/similar', async (req, res) => {
    try {
        const { paths = [], threshold = 10 } = req.body;
        if (!paths.length) return res.status(400).json({ error: 'No paths provided' });
        const { stdout } = await runCli(['similar', '--format=json', `--threshold=${threshold}`, ...paths]);
        try { res.json(JSON.parse(extractJson(stdout) || '[]')); } catch { res.json({ raw: stdout }); }
    } catch (e) { res.status(500).json({ error: e.message }); }
});
app.post('/api/vhash', async (req, res) => {
    try {
        const { paths = [] } = req.body;
        if (!paths.length) return res.status(400).json({ error: 'No paths provided' });
        const { stdout } = await runCli(['vhash', '--format=json', ...paths]);
        try { res.json(JSON.parse(extractJson(stdout) || '[]')); } catch { res.json({ raw: stdout }); }
    } catch (e) { res.status(500).json({ error: e.message }); }
});
app.post('/api/afingerprint', async (req, res) => {
    try {
        const { paths = [] } = req.body;
        if (!paths.length) return res.status(400).json({ error: 'No paths provided' });
        const { stdout } = await runCli(['afingerprint', '--format=json', ...paths]);
        try { res.json(JSON.parse(extractJson(stdout) || '[]')); } catch { res.json({ raw: stdout }); }
    } catch (e) { res.status(500).json({ error: e.message }); }
});
app.post('/api/classify', async (req, res) => {
    try {
        const { paths = [] } = req.body;
        if (!paths.length) return res.status(400).json({ error: 'No paths provided' });
        const { stdout } = await runCli(['classify', '--format=json', ...paths]);
        try { res.json(JSON.parse(extractJson(stdout) || '[]')); } catch { res.json({ raw: stdout }); }
    } catch (e) { res.status(500).json({ error: e.message }); }
});
app.post('/api/rename', async (req, res) => {
    try {
        const { paths = [], pattern = '', dryRun = true } = req.body;
        if (!paths.length || !pattern) return res.status(400).json({ error: 'paths and pattern required' });
        const args = ['rename', '--pattern=' + pattern, '--format=json', ...paths];
        if (dryRun) args.push('--dry-run');
        const { stdout } = await runCli(args);
        try { res.json(JSON.parse(extractJson(stdout) || '{"moves":[]}')); } catch { res.json({ raw: stdout }); }
    } catch (e) { res.status(500).json({ error: e.message }); }
});
app.post('/api/rename-batch', async (req, res) => {
    try {
        const { paths = [], rules = '', dryRun = true } = req.body;
        if (!paths.length) return res.status(400).json({ error: 'No paths provided' });
        const args = ['rename-batch', '--format=json', ...paths];
        if (rules) args.push('--rules=' + rules);
        if (dryRun) args.push('--dry-run');
        const { stdout } = await runCli(args);
        try { res.json(JSON.parse(extractJson(stdout) || '{"moves":[]}')); } catch { res.json({ raw: stdout }); }
    } catch (e) { res.status(500).json({ error: e.message }); }
});
app.post('/api/convert', async (req, res) => {
    try {
        const { files = [], to = '' } = req.body;
        if (!files.length || !to) return res.status(400).json({ error: 'files and target format required' });
        const { stdout } = await runCli(['convert', '--to=' + to, '--format=json', ...files]);
        try { res.json(JSON.parse(extractJson(stdout) || '{"success":false}')); } catch { res.json({ raw: stdout }); }
    } catch (e) { res.status(500).json({ error: e.message }); }
});
app.post('/api/undo', async (req, res) => {
    try {
        const { stdout } = await runCli(['undo', '--format=json']);
        try { res.json(JSON.parse(extractJson(stdout) || '{"success":false}')); } catch { res.json({ raw: stdout }); }
    } catch (e) { res.status(500).json({ error: e.message }); }
});
app.post('/api/clerk', async (req, res) => {
    try {
        // fo_cli clerk only supports `start <inbox> <archive>` (blocking
        // listener, never launched from HTTP) and `status` (plain text).
        // The old bridge sent `clerk --format=json <paths>` which always
        // usage-errored; map the dashboard card to the honest status probe.
        const { stdout } = await runCli(['clerk', 'status'], 30000);
        res.json({ raw: stdout, note: 'Live filing: run `fo_cli clerk start <inbox> <archive>` in a terminal.' });
    } catch (e) { res.status(500).json({ error: e.message }); }
});
app.post('/api/oracle', async (req, res) => {
    try {
        const { question = '', paths = [] } = req.body;
        if (!question) return res.status(400).json({ error: 'question required' });
        // fo_cli oracle takes a bare query and prints plain text (no JSON
        // mode, no path args); forward only the question.
        const { stdout } = await runCli(['oracle', question], 300000);
        res.json({ answer: stdout, raw: stdout });
    } catch (e) { res.status(500).json({ error: e.message }); }
});
app.get('/api/watch/status', async (req, res) => {
    try {
        // fo_cli watch publishes a heartbeat; watch-status reads it
        // non-blockingly and reports watching:false when stale/absent.
        const { stdout } = await runCli(['watch-status', '--format=json'], 30000);
        try { res.json(JSON.parse(extractJson(stdout) || '{"watching":false}')); }
        catch { res.json({ watching: false, raw: stdout }); }
    } catch (e) { res.status(500).json({ error: e.message }); }
});
app.post('/api/quit', async (req, res) => {
    res.json({ success: true, message: 'Shutting down dashboard server' });
    setTimeout(() => process.exit(0), 500);
});

const PORT = process.env.PORT || 3131;
app.listen(PORT, () => {
    console.log(`\n  ╔══════════════════════════════════════╗`);
    console.log(`  ║  BobUI — bobfilez GUI               ║`);
    console.log(`  ║  http://localhost:${PORT}              ║`);
    console.log(`  ╚══════════════════════════════════════╝\n`);
    console.log(`  CLI: ${CLI}`);
});
