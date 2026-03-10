const express = require('express');
const { execFile } = require('child_process');
const path = require('path');
const fs = require('fs');

const app = express();
app.use(express.json());
app.use(express.static(path.join(__dirname, 'public')));

const CLI = path.resolve(__dirname, '..', 'build', 'cli', 'fo_cli.exe');

function runCli(args, timeout = 120000) {
    return new Promise((resolve, reject) => {
        execFile(CLI, args, { maxBuffer: 50 * 1024 * 1024, timeout }, (err, stdout, stderr) => {
            if (err && err.killed) return reject(new Error('Command timed out'));
            if (err && !stdout) return reject(new Error(stderr || err.message));
            resolve({ stdout: stdout.trim(), stderr: stderr.trim() });
        });
    });
}

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

const PORT = process.env.PORT || 3131;
app.listen(PORT, () => {
    console.log(`\n  ╔══════════════════════════════════════╗`);
    console.log(`  ║  BobUI — bobfilez GUI               ║`);
    console.log(`  ║  http://localhost:${PORT}              ║`);
    console.log(`  ╚══════════════════════════════════════╝\n`);
    console.log(`  CLI: ${CLI}`);
});
