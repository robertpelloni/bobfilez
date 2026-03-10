// ═══════════════════════════════════════════════════════════════
// BobUI — dupeGuru-style GUI for bobfilez
// ═══════════════════════════════════════════════════════════════

const $ = (s, p = document) => p.querySelector(s);
const $$ = (s, p = document) => [...p.querySelectorAll(s)];

// ── State ──
const state = {
    view: 'duplicates',
    scanning: false,
    dupeGroups: [],
    scanFiles: [],
    stats: null,
    history: [],
    ignoreRules: [],
    markedFiles: new Set(),
    expandedGroups: new Set(),
    scanPath: '',
    mode: 'fast',
    threads: navigator.hardwareConcurrency || 4,
    recursive: true,
    exts: '',
    minSize: '',
    maxSize: '',
    excludes: '',
};

// ── API Client ──
async function api(path, body) {
    const opts = body ? { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(body) } : {};
    const res = await fetch(`/api/${path}`, opts);
    if (!res.ok) {
        const err = await res.json().catch(() => ({ error: res.statusText }));
        throw new Error(err.error || 'Request failed');
    }
    return res.json();
}

// ── Navigation ──
$$('.nav-item').forEach(el => {
    el.addEventListener('click', () => {
        $$('.nav-item').forEach(n => n.classList.remove('active'));
        el.classList.add('active');
        state.view = el.dataset.view;
        render();
    });
});

// ── Formatting ──
function fmtSize(bytes) {
    if (!bytes || bytes === 0) return '0 B';
    const u = ['B', 'KB', 'MB', 'GB', 'TB'];
    const i = Math.floor(Math.log(bytes) / Math.log(1024));
    return (bytes / Math.pow(1024, i)).toFixed(i > 0 ? 1 : 0) + ' ' + u[i];
}

function fmtPath(p) {
    const parts = p.replace(/\\/g, '/').split('/');
    if (parts.length <= 3) return p;
    return '...' + '/' + parts.slice(-3).join('/');
}

// ── Get filter params ──
function getFilters() {
    const f = {};
    if (state.exts) f.exts = state.exts;
    if (state.minSize) f.minSize = state.minSize;
    if (state.maxSize) f.maxSize = state.maxSize;
    if (state.excludes) f.exclude = state.excludes.split(',').map(s => s.trim()).filter(Boolean);
    f.recursive = state.recursive;
    return f;
}

// ═══════════════════════════════════════════════════════════════
// VIEWS
// ═══════════════════════════════════════════════════════════════

function render() {
    const main = $('#mainContent');
    const title = $('#viewTitle');
    const views = {
        duplicates: { title: 'Duplicate Finder', render: renderDuplicates },
        scanner: { title: 'File Scanner', render: renderScanner },
        stats: { title: 'Statistics', render: renderStats },
        ignore: { title: 'Ignore Rules', render: renderIgnore },
        history: { title: 'History', render: renderHistory },
    };
    const v = views[state.view] || views.duplicates;
    title.textContent = v.title;
    main.innerHTML = '';
    v.render(main);
}

// ── Duplicates View ──
function renderDuplicates(el) {
    el.innerHTML = `
    <div class="card fade-in" style="margin-bottom: 16px">
      <div class="card-header">
        <span class="card-title">🔍 Scan Configuration</span>
        <div style="display:flex; gap:8px; align-items:center;">
          ${state.scanning ? '<span class="pill pill-accent scanning">Scanning...</span>' : ''}
        </div>
      </div>
      <div class="scan-controls">
        <div class="scan-row">
          <input type="text" id="dupePath" placeholder="Enter directory path to scan (e.g., C:\\Users\\Photos)" value="${state.scanPath}">
          <button class="btn btn-primary" id="scanDupeBtn" ${state.scanning ? 'disabled' : ''}>
            ${state.scanning ? '⏳ Scanning...' : '🔍 Find Duplicates'}
          </button>
        </div>
        <div style="display:flex; gap:16px; align-items:center; flex-wrap:wrap;">
          <div>
            <label style="font-size:11px;color:var(--text-muted);margin-bottom:4px;display:block;">VERIFICATION MODE</label>
            <div class="mode-selector">
              <button class="mode-btn ${state.mode === 'fast' ? 'active' : ''}" data-mode="fast">⚡ Fast</button>
              <button class="mode-btn ${state.mode === 'safe' ? 'active' : ''}" data-mode="safe">🔒 Safe</button>
              <button class="mode-btn ${state.mode === 'paranoid' ? 'active' : ''}" data-mode="paranoid">🛡️ Paranoid</button>
            </div>
          </div>
          <div class="filter-group">
            <label>Threads</label>
            <input type="number" id="dupeThreads" value="${state.threads}" min="1" max="32" style="width:60px">
          </div>
          <div class="filter-group">
            <label>Extensions</label>
            <input type="text" id="dupeExts" value="${state.exts}" placeholder=".jpg,.png" style="width:120px">
          </div>
          <div class="filter-group">
            <label>Min Size</label>
            <input type="text" id="dupeMinSize" value="${state.minSize}" placeholder="e.g. 1M" style="width:80px">
          </div>
          <div class="filter-group">
            <label>Max Size</label>
            <input type="text" id="dupeMaxSize" value="${state.maxSize}" placeholder="e.g. 1G" style="width:80px">
          </div>
          <div class="toggle-row" style="margin-top:14px;">
            <div class="toggle ${state.recursive ? 'on' : ''}" id="recursiveToggle"></div>
            <span>Recursive</span>
          </div>
        </div>
      </div>
    </div>

    ${state.dupeGroups.length > 0 ? renderDupeResults() : renderDupeEmpty()}
  `;

    // Event handlers
    const scanBtn = $('#scanDupeBtn');
    if (scanBtn) scanBtn.onclick = startDupeScan;

    $$('.mode-btn').forEach(btn => {
        btn.onclick = () => {
            state.mode = btn.dataset.mode;
            render();
        };
    });

    const toggle = $('#recursiveToggle');
    if (toggle) toggle.onclick = () => { state.recursive = !state.recursive; render(); };

    // Bind group expand/collapse
    $$('.group-header').forEach(h => {
        h.onclick = () => {
            const idx = parseInt(h.dataset.idx);
            state.expandedGroups.has(idx) ? state.expandedGroups.delete(idx) : state.expandedGroups.add(idx);
            render();
        };
    });

    // Bind file checkboxes
    $$('.file-check').forEach(cb => {
        cb.onchange = () => {
            cb.checked ? state.markedFiles.add(cb.dataset.path) : state.markedFiles.delete(cb.dataset.path);
            render();
        };
    });
}

function renderDupeEmpty() {
    return `
    <div class="empty-state fade-in">
      <div class="icon">🔍</div>
      <h3>No duplicates found yet</h3>
      <p>Enter a directory path above and click "Find Duplicates" to scan for duplicate files.</p>
    </div>
  `;
}

function renderDupeResults() {
    const totalDupes = state.dupeGroups.reduce((s, g) => s + g.files.length - 1, 0);
    const wastedBytes = state.dupeGroups.reduce((s, g) => s + g.size * (g.files.length - 1), 0);
    const markedCount = state.markedFiles.size;

    return `
    <div class="card fade-in" style="margin-bottom: 16px">
      <div class="card-header">
        <span class="card-title">
          Results <span class="pill pill-accent">${state.dupeGroups.length} groups</span>
          <span class="pill pill-danger">${totalDupes} duplicates</span>
          <span class="pill pill-accent">${fmtSize(wastedBytes)} reclaimable</span>
        </span>
        <div style="display:flex; gap:8px;">
          <select id="autoMarkSelect" style="width:180px;">
            <option value="">Auto-mark strategy...</option>
            <option value="newest">Keep Oldest (mark newest)</option>
            <option value="oldest">Keep Newest (mark oldest)</option>
            <option value="shortest">Keep Shortest Path</option>
            <option value="longest">Keep Longest Path</option>
          </select>
          ${markedCount > 0 ? `<button class="btn btn-danger" id="deleteMarkedBtn">🗑️ Delete ${markedCount} files</button>` : ''}
          <button class="btn btn-sm" id="expandAllBtn">Expand All</button>
          <button class="btn btn-sm" id="clearMarksBtn">Clear Marks</button>
        </div>
      </div>

      <table class="dupe-table">
        <thead>
          <tr>
            <th style="width:30px"></th>
            <th style="width:30px"></th>
            <th>File Path</th>
            <th style="width:100px">Size</th>
          </tr>
        </thead>
        <tbody>
          ${state.dupeGroups.map((g, gi) => renderDupeGroup(g, gi)).join('')}
        </tbody>
      </table>
    </div>
  `;
}

function renderDupeGroup(group, idx) {
    const expanded = state.expandedGroups.has(idx);
    const groupFiles = group.files.map((f, fi) => {
        const isMarked = state.markedFiles.has(f);
        const isKept = !isMarked && fi === 0;
        return `
      <tr class="file-row ${isMarked ? 'marked' : ''} ${isKept ? 'kept' : ''}" style="display:${expanded ? '' : 'none'}">
        <td></td>
        <td><input type="checkbox" class="checkbox file-check" data-path="${f}" ${isMarked ? 'checked' : ''}></td>
        <td class="file-path" title="${f}">${f}</td>
        <td class="file-size">${fmtSize(group.size)}</td>
      </tr>
    `;
    }).join('');

    return `
    <tr class="group-header ${expanded ? 'expanded' : ''}" data-idx="${idx}">
      <td><span class="chevron">▶</span></td>
      <td></td>
      <td>
        <strong>Group ${idx + 1}</strong> — ${group.files.length} files
        <span class="pill pill-accent" style="margin-left:8px">${group.hash?.substring(0, 12) || group.fast64?.substring(0, 12) || ''}</span>
      </td>
      <td class="file-size">${fmtSize(group.size)}</td>
    </tr>
    ${groupFiles}
  `;
}

async function startDupeScan() {
    const pathInput = $('#dupePath');
    const path = pathInput?.value?.trim();
    if (!path) { alert('Please enter a directory path'); return; }

    state.scanPath = path;
    state.exts = $('#dupeExts')?.value || '';
    state.minSize = $('#dupeMinSize')?.value || '';
    state.maxSize = $('#dupeMaxSize')?.value || '';
    state.threads = parseInt($('#dupeThreads')?.value) || 4;
    state.scanning = true;
    state.dupeGroups = [];
    state.markedFiles.clear();
    state.expandedGroups.clear();
    render();

    try {
        const data = await api('duplicates', {
            paths: [path],
            mode: state.mode,
            threads: state.threads,
            ...getFilters()
        });
        state.dupeGroups = data;
        // Auto-expand first few groups
        data.slice(0, 5).forEach((_, i) => state.expandedGroups.add(i));
    } catch (e) {
        alert('Scan error: ' + e.message);
    }
    state.scanning = false;
    render();

    // Rebind action buttons after render
    setTimeout(bindDupeActions, 0);
}

function bindDupeActions() {
    const autoSelect = $('#autoMarkSelect');
    if (autoSelect) autoSelect.onchange = () => applyAutoMark(autoSelect.value);

    const deleteBtn = $('#deleteMarkedBtn');
    if (deleteBtn) deleteBtn.onclick = () => showDeleteModal();

    const expandBtn = $('#expandAllBtn');
    if (expandBtn) expandBtn.onclick = () => {
        state.dupeGroups.forEach((_, i) => state.expandedGroups.add(i));
        render();
        setTimeout(bindDupeActions, 0);
    };

    const clearBtn = $('#clearMarksBtn');
    if (clearBtn) clearBtn.onclick = () => {
        state.markedFiles.clear();
        render();
        setTimeout(bindDupeActions, 0);
    };
}

function applyAutoMark(strategy) {
    if (!strategy) return;
    state.markedFiles.clear();
    for (const g of state.dupeGroups) {
        const sorted = [...g.files];
        // Keep the first after sorting, mark the rest
        // For "newest" strategy: keep oldest (first by default), mark rest
        // The CLI already outputs files in scan order
        if (strategy === 'newest') {
            // Mark all except first (oldest by scan order)
            sorted.slice(1).forEach(f => state.markedFiles.add(f));
        } else if (strategy === 'oldest') {
            sorted.slice(0, -1).forEach(f => state.markedFiles.add(f));
        } else if (strategy === 'shortest') {
            sorted.sort((a, b) => a.length - b.length);
            sorted.slice(1).forEach(f => state.markedFiles.add(f));
        } else if (strategy === 'longest') {
            sorted.sort((a, b) => b.length - a.length);
            sorted.slice(1).forEach(f => state.markedFiles.add(f));
        }
    }
    render();
    setTimeout(bindDupeActions, 0);
}

function showDeleteModal() {
    const modal = $('#deleteModal');
    const text = $('#deleteModalText');
    text.textContent = `You are about to permanently delete ${state.markedFiles.size} files. This cannot be undone.`;
    modal.classList.add('show');

    $('#confirmDeleteBtn').onclick = async () => {
        closeModal();
        try {
            const result = await api('delete', { files: [...state.markedFiles] });
            alert(`Deleted ${result.deleted} files.`);
            // Remove deleted files from groups
            for (const g of state.dupeGroups) {
                g.files = g.files.filter(f => !state.markedFiles.has(f));
            }
            state.dupeGroups = state.dupeGroups.filter(g => g.files.length >= 2);
            state.markedFiles.clear();
            render();
            setTimeout(bindDupeActions, 0);
        } catch (e) {
            alert('Delete error: ' + e.message);
        }
    };
}

function closeModal() {
    $('#deleteModal').classList.remove('show');
}

// ── Scanner View ──
function renderScanner(el) {
    el.innerHTML = `
    <div class="card fade-in" style="margin-bottom:16px">
      <div class="card-header">
        <span class="card-title">📂 Scan Directory</span>
      </div>
      <div class="scan-controls">
        <div class="scan-row">
          <input type="text" id="scannerPath" placeholder="Enter directory path..." value="${state.scanPath}">
          <button class="btn btn-primary" id="scanFilesBtn" ${state.scanning ? 'disabled' : ''}>
            ${state.scanning ? '⏳ Scanning...' : '📂 Scan Files'}
          </button>
        </div>
        <div class="filter-grid">
          <div class="filter-group">
            <label>Extensions</label>
            <input type="text" id="scanExts" value="${state.exts}" placeholder=".jpg,.png,.pdf">
          </div>
          <div class="filter-group">
            <label>Exclude Patterns</label>
            <input type="text" id="scanExcludes" value="${state.excludes}" placeholder="*.tmp, node_modules">
          </div>
          <div class="filter-group">
            <label>Min Size</label>
            <input type="text" id="scanMinSize" value="${state.minSize}" placeholder="e.g. 100K">
          </div>
          <div class="filter-group">
            <label>Max Size</label>
            <input type="text" id="scanMaxSize" value="${state.maxSize}" placeholder="e.g. 500M">
          </div>
        </div>
      </div>
    </div>

    ${state.scanFiles.length > 0 ? `
      <div class="card fade-in">
        <div class="card-header">
          <span class="card-title">Results <span class="pill pill-accent">${state.scanFiles.length} files</span></span>
        </div>
        <div style="max-height:500px;overflow-y:auto;">
          <table class="dupe-table">
            <thead><tr><th>Path</th><th style="width:100px">Size</th></tr></thead>
            <tbody>
              ${state.scanFiles.slice(0, 500).map(f => `
                <tr class="file-row">
                  <td class="file-path" title="${f.path}">${f.path}</td>
                  <td class="file-size">${fmtSize(f.size)}</td>
                </tr>
              `).join('')}
              ${state.scanFiles.length > 500 ? `<tr><td colspan="2" style="text-align:center;color:var(--text-muted);padding:12px;">...and ${state.scanFiles.length - 500} more files</td></tr>` : ''}
            </tbody>
          </table>
        </div>
      </div>
    ` : `
      <div class="empty-state fade-in">
        <div class="icon">📂</div>
        <h3>No files scanned</h3>
        <p>Enter a path and click scan to view files.</p>
      </div>
    `}
  `;

    const scanBtn = $('#scanFilesBtn');
    if (scanBtn) scanBtn.onclick = async () => {
        const path = $('#scannerPath')?.value?.trim();
        if (!path) return;
        state.scanPath = path;
        state.exts = $('#scanExts')?.value || '';
        state.excludes = $('#scanExcludes')?.value || '';
        state.minSize = $('#scanMinSize')?.value || '';
        state.maxSize = $('#scanMaxSize')?.value || '';
        state.scanning = true;
        render();
        try {
            state.scanFiles = await api('scan', { paths: [path], ...getFilters() });
        } catch (e) { alert('Error: ' + e.message); }
        state.scanning = false;
        render();
    };
}

// ── Stats View ──
function renderStats(el) {
    el.innerHTML = `
    <div class="card fade-in" style="margin-bottom:16px">
      <div class="card-header">
        <span class="card-title">📊 File Statistics</span>
      </div>
      <div class="scan-row">
        <input type="text" id="statsPath" placeholder="Enter directory path..." value="${state.scanPath}">
        <button class="btn btn-primary" id="statsBtn">📊 Analyze</button>
      </div>
    </div>

    ${state.stats ? renderStatsResults() : `
      <div class="empty-state fade-in">
        <div class="icon">📊</div>
        <h3>No statistics yet</h3>
        <p>Enter a path and click analyze.</p>
      </div>
    `}
  `;

    const btn = $('#statsBtn');
    if (btn) btn.onclick = async () => {
        const path = $('#statsPath')?.value?.trim();
        if (!path) return;
        state.scanPath = path;
        try {
            state.stats = await api('stats', { paths: [path] });
        } catch (e) { alert('Error: ' + e.message); }
        render();
    };
}

function renderStatsResults() {
    const s = state.stats;
    const exts = s.extensions || [];

    return `
    <div class="stats-grid fade-in">
      <div class="stat-card">
        <div class="label">Total Files</div>
        <div class="value">${(s.total_files || 0).toLocaleString()}</div>
      </div>
      <div class="stat-card">
        <div class="label">Total Size</div>
        <div class="value">${fmtSize(s.total_size || 0)}</div>
      </div>
      <div class="stat-card">
        <div class="label">Extensions</div>
        <div class="value">${exts.length}</div>
      </div>
      <div class="stat-card">
        <div class="label">Oldest File</div>
        <div class="value" style="font-size:14px">${s.oldest || '—'}</div>
      </div>
      <div class="stat-card">
        <div class="label">Newest File</div>
        <div class="value" style="font-size:14px">${s.newest || '—'}</div>
      </div>
    </div>

    ${exts.length > 0 ? `
      <div class="card fade-in">
        <div class="card-header">
          <span class="card-title">Extension Breakdown</span>
        </div>
        ${exts.slice(0, 20).map(e => {
        const pct = s.total_files ? (e.count / s.total_files * 100) : 0;
        return `
            <div class="ext-bar">
              <span class="ext-name">${e.ext}</span>
              <div class="ext-fill"><div class="ext-fill-inner" style="width:${pct}%"></div></div>
              <span class="ext-count">${e.count}</span>
            </div>
          `;
    }).join('')}
      </div>
    ` : ''}
  `;
}

// ── Ignore View ──
function renderIgnore(el) {
    el.innerHTML = `
    <div class="card fade-in" style="margin-bottom:16px">
      <div class="card-header">
        <span class="card-title">🚫 Add Ignore Rule</span>
      </div>
      <div class="scan-row">
        <input type="text" id="ignorePattern" placeholder="Pattern (e.g., *.tmp, thumbs.db)">
        <input type="text" id="ignoreReason" placeholder="Reason (optional)" style="width:200px">
        <button class="btn btn-primary" id="addIgnoreBtn">Add Rule</button>
      </div>
    </div>

    <div class="card fade-in">
      <div class="card-header">
        <span class="card-title">Active Rules <span class="pill pill-accent">${state.ignoreRules.length}</span></span>
        <button class="btn btn-sm" id="refreshIgnoreBtn">↻ Refresh</button>
      </div>
      ${state.ignoreRules.length === 0 ? `
        <div class="empty-state" style="padding:30px">
          <p>No ignore rules configured.</p>
        </div>
      ` : state.ignoreRules.map(r => `
        <div class="rule-item">
          <div>
            <span>${r.pattern}</span>
            ${r.reason ? `<span class="rule-reason"> — ${r.reason}</span>` : ''}
          </div>
          <button class="btn btn-sm btn-danger remove-ignore" data-pattern="${r.pattern}">✕</button>
        </div>
      `).join('')}
    </div>
  `;

    const addBtn = $('#addIgnoreBtn');
    if (addBtn) addBtn.onclick = async () => {
        const pattern = $('#ignorePattern')?.value?.trim();
        if (!pattern) return;
        const reason = $('#ignoreReason')?.value?.trim() || '';
        await api('ignore/add', { pattern, reason });
        await loadIgnoreRules();
        render();
    };

    const refreshBtn = $('#refreshIgnoreBtn');
    if (refreshBtn) refreshBtn.onclick = async () => { await loadIgnoreRules(); render(); };

    $$('.remove-ignore').forEach(btn => {
        btn.onclick = async () => {
            await api('ignore/remove', { pattern: btn.dataset.pattern });
            await loadIgnoreRules();
            render();
        };
    });
}

async function loadIgnoreRules() {
    try { state.ignoreRules = await api('ignore'); } catch (e) { state.ignoreRules = []; }
}

// ── History View ──
function renderHistory(el) {
    el.innerHTML = `
    <div class="card fade-in">
      <div class="card-header">
        <span class="card-title">📜 Operation History</span>
        <button class="btn btn-sm" id="refreshHistoryBtn">↻ Refresh</button>
      </div>
      ${state.history.length === 0 ? `
        <div class="empty-state" style="padding:30px">
          <p>No operations recorded.</p>
        </div>
      ` : `
        <table class="dupe-table">
          <thead><tr><th>Time</th><th>Type</th><th>Source</th><th>Dest</th><th>Status</th></tr></thead>
          <tbody>
            ${state.history.map(op => `
              <tr class="file-row">
                <td style="white-space:nowrap;font-size:12px;color:var(--text-secondary)">${op.timestamp || ''}</td>
                <td><span class="pill pill-accent">${op.type || ''}</span></td>
                <td class="file-path" title="${op.source || ''}">${fmtPath(op.source || '')}</td>
                <td class="file-path" title="${op.dest || ''}">${op.dest ? fmtPath(op.dest) : '—'}</td>
                <td>${op.undone ? '<span class="pill pill-danger">undone</span>' : '<span class="pill pill-success">done</span>'}</td>
              </tr>
            `).join('')}
          </tbody>
        </table>
      `}
    </div>
  `;

    const btn = $('#refreshHistoryBtn');
    if (btn) btn.onclick = async () => {
        try { state.history = await api('history'); } catch (e) { }
        render();
    };
}

// ── Init ──
async function init() {
    render();
    // Pre-load ignore rules and history
    try { state.ignoreRules = await api('ignore'); } catch (e) { }
    try { state.history = await api('history'); } catch (e) { }
}

init();
