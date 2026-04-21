const express = require('express');
const { execFile } = require('child_process');
const path = require('path');
const fs = require('fs');

const app = express();
app.use(express.json());
app.use(express.static(path.join(__dirname, 'public')));

const CLI_CANDIDATES = [
    path.resolve(__dirname, '..', 'build-msvc', 'cli', 'fo_cli.exe'),
    path.resolve(__dirname, '..', 'build', 'cli', 'fo_cli.exe')
];
const CLI = CLI_CANDIDATES.find(fs.existsSync) || CLI_CANDIDATES[0];

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
        res.json(JSON.parse(stdout || '[]'));
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
        res.json(JSON.parse(stdout || '[]'));
    } catch (e) { res.status(500).json({ error: e.message }); }
});

// ── Stats ──
app.post('/api/stats', async (req, res) => {
    try {
        const { paths = [] } = req.body;
        if (!paths.length) return res.status(400).json({ error: 'No paths provided' });
        const args = ['stats', '--format=json', ...paths];
        const { stdout } = await runCli(args);
        res.json(JSON.parse(stdout || '{}'));
    } catch (e) { res.status(500).json({ error: e.message }); }
});

// ── Hash ──
app.post('/api/hash', async (req, res) => {
    try {
        const { paths = [], threads = 1 } = req.body;
        if (!paths.length) return res.status(400).json({ error: 'No paths provided' });
        const args = ['hash', '--format=json', `--threads=${threads}`, ...paths];
        const { stdout } = await runCli(args);
        res.json(JSON.parse(stdout || '[]'));
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
        res.json(JSON.parse(stdout || '[]'));
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
        res.json(JSON.parse(stdout || '[]'));
    } catch (e) { res.status(500).json({ error: e.message }); }
});

// ── Ignore rules ──
app.get('/api/ignore', async (req, res) => {
    try {
        const { stdout } = await runCli(['ignore', '--format=json']);
        res.json(JSON.parse(stdout || '[]'));
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
        res.json(JSON.parse(stdout || '[]'));
    } catch (e) { res.status(500).json({ error: e.message }); }
});

// ── Count (quick) ──
app.post('/api/count', async (req, res) => {
    try {
        const { paths = [] } = req.body;
        if (!paths.length) return res.status(400).json({ error: 'No paths' });
        const { stdout } = await runCli(['scan', '--count', ...paths]);
        res.json({ count: parseInt(stdout, 10) || 0 });
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
        res.json(JSON.parse(stdout || '{"workflows":[]}'));
    } catch (e) { res.status(500).json({ error: e.message }); }
});

app.post('/api/flow/run', async (req, res) => {
    try {
        const { workflow_id, payload_path } = req.body;
        if (!workflow_id || !payload_path) return res.status(400).json({ error: 'workflow_id and payload_path required' });
        const { stdout } = await runCli(['flow', 'run', workflow_id, payload_path, '--format=json']);
        res.json(JSON.parse(stdout || '{"success":false}'));
    } catch (e) { res.status(500).json({ error: e.message }); }
});

// ── Scrub (Integrity Verification) ──
app.post('/api/scrub', async (req, res) => {
    try {
        const { paths = [] } = req.body;
        if (!paths.length) return res.status(400).json({ error: 'No paths provided' });
        const { stdout } = await runCli(['scrub', '--format=json', ...paths]);
        res.json(JSON.parse(stdout || '{"scanned":0,"corrupted":0}'));
    } catch (e) { res.status(500).json({ error: e.message }); }
});

// ── Search ──
app.post('/api/search', async (req, res) => {
    try {
        const { query, paths = [], content = false, regex = false } = req.body;
        if (!query) return res.status(400).json({ error: 'No query provided' });
        const args = ['search', query, '--format=json'];
        if (content) args.push('--content');
        if (regex) args.push('--regex');
        args.push(...paths);
        const { stdout } = await runCli(args);
        res.json(JSON.parse(stdout || '{"results":[]}'));
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
            res.json(JSON.parse(stdout || '{}'));
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
        const args = ['organize', '--rule', rule, '--dry-run', '--format=json', ...paths];
        const { stdout } = await runCli(args);
        res.json(JSON.parse(stdout || '{"moves":[]}'));
    } catch (e) { res.status(500).json({ error: e.message }); }
});


// ── Vault ──
app.post('/api/vault/init', async (req, res) => {
    try {
        const { vaultPath = '', password = '' } = req.body;
        if (!vaultPath || !password) return res.status(400).json({ error: 'vaultPath and password required' });
        const args = ['vault-init', '--vault=' + vaultPath, '--password=' + password, '--format=json'];
        const { stdout } = await runCli(args);
        res.json(JSON.parse(stdout || '{"success":false}'));
    } catch (e) { res.status(500).json({ error: e.message }); }
});

app.post('/api/vault/lock', async (req, res) => {
    try {
        const { vaultPath = '', password = '', file = '' } = req.body;
        if (!vaultPath || !password || !file) return res.status(400).json({ error: 'vaultPath, password, and file required' });
        const args = ['vault-lock', '--vault=' + vaultPath, '--password=' + password, '--format=json', file];
        const { stdout } = await runCli(args);
        res.json(JSON.parse(stdout || '{"success":false}'));
    } catch (e) { res.status(500).json({ error: e.message }); }
});

app.post('/api/vault/unlock', async (req, res) => {
    try {
        const { vaultPath = '', password = '', vaultId = '', dest = '' } = req.body;
        if (!vaultPath || !password || !vaultId || !dest) return res.status(400).json({ error: 'vaultPath, password, vaultId, and dest required' });
        const args = ['vault-unlock', '--vault=' + vaultPath, '--password=' + password, '--format=json', vaultId, dest];
        const { stdout } = await runCli(args);
        res.json(JSON.parse(stdout || '{"success":false}'));
    } catch (e) { res.status(500).json({ error: e.message }); }
});

app.post('/api/vault/list', async (req, res) => {
    try {
        const { vaultPath = '', password = '' } = req.body;
        if (!vaultPath || !password) return res.status(400).json({ error: 'vaultPath and password required' });
        const args = ['vault-list', '--vault=' + vaultPath, '--password=' + password, '--format=json'];
        const { stdout } = await runCli(args);
        res.json(JSON.parse(stdout || '[]'));
    } catch (e) { res.status(500).json({ error: e.message }); }
});

const PORT = process.env.PORT || 3131;
app.listen(PORT, () => {
    console.log(`\n  ╔══════════════════════════════════════╗`);
    console.log(`  ║  BobUI — bobfilez GUI               ║`);
    console.log(`  ║  http://localhost:${PORT}              ║`);
    console.log(`  ╚══════════════════════════════════════╝\n`);
    console.log(`  CLI: ${CLI}`);
});
