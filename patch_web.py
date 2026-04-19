import re

with open("bobui_web/server.js", "r") as f:
    text = f.read()

vault_api = """
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
"""

text = text.replace("const PORT = process.env.PORT || 3131;", vault_api + "\nconst PORT = process.env.PORT || 3131;")

with open("bobui_web/server.js", "w") as f:
    f.write(text)
