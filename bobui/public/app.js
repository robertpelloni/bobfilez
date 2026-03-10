// ═══════════════════════════════════════════════════════════════
// BobUI v2.4.1 — Ultimate File Management GUI
// ═══════════════════════════════════════════════════════════════

const $ = (s, p = document) => p.querySelector(s);
const $$ = (s, p = document) => [...p.querySelectorAll(s)];

// ── State ──
const state = {
  view: 'dashboard',
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
  // Smart Clean wizard
  wizardStep: 0,      // 0=idle, 1=scanning, 2=deduping, 3=done
  wizardStats: null,
  wizardDupes: [],
  wizardPath: '',
  // Dashboard quick stats
  dashStats: null,
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
function bindNav() {
  $$('.nav-item').forEach(el => {
    el.onclick = () => {
      $$('.nav-item').forEach(n => n.classList.remove('active'));
      el.classList.add('active');
      state.view = el.dataset.view;
      render();
    };
  });
}
bindNav();

// ── Formatting ──
function fmtSize(bytes) {
  if (!bytes || bytes === 0) return '0 B';
  const u = ['B', 'KB', 'MB', 'GB', 'TB'];
  const i = Math.floor(Math.log(bytes) / Math.log(1024));
  return (bytes / Math.pow(1024, i)).toFixed(i > 0 ? 1 : 0) + ' ' + u[i];
}

function fmtPath(p) {
  const parts = (p || '').replace(/\\/g, '/').split('/');
  if (parts.length <= 3) return p;
  return '…/' + parts.slice(-3).join('/');
}

function fileIcon(path) {
  const ext = (path || '').split('.').pop().toLowerCase();
  const icons = {
    jpg: '🖼️', jpeg: '🖼️', png: '🖼️', gif: '🖼️', bmp: '🖼️', webp: '🖼️', svg: '🖼️', tiff: '🖼️', ico: '🖼️',
    mp4: '🎬', avi: '🎬', mkv: '🎬', mov: '🎬', wmv: '🎬', flv: '🎬', webm: '🎬',
    mp3: '🎵', wav: '🎵', flac: '🎵', aac: '🎵', ogg: '🎵', wma: '🎵',
    pdf: '📄', doc: '📝', docx: '📝', txt: '📃', rtf: '📃', md: '📃',
    xls: '📊', xlsx: '📊', csv: '📊',
    ppt: '📊', pptx: '📊',
    zip: '📦', rar: '📦', '7z': '📦', tar: '📦', gz: '📦',
    exe: '⚙️', msi: '⚙️', dll: '⚙️', bat: '⚙️',
    js: '💻', ts: '💻', py: '💻', cpp: '💻', c: '💻', h: '💻', java: '💻', rs: '💻', go: '💻',
    html: '🌐', css: '🌐', json: '📋', xml: '📋', yaml: '📋', yml: '📋',
  };
  return icons[ext] || '📄';
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
// DRAG AND DROP
// ═══════════════════════════════════════════════════════════════

let dragCounter = 0;
document.addEventListener('dragenter', e => {
  e.preventDefault();
  dragCounter++;
  $('#dropOverlay').classList.add('show');
});
document.addEventListener('dragleave', e => {
  e.preventDefault();
  dragCounter--;
  if (dragCounter <= 0) {
    dragCounter = 0;
    $('#dropOverlay').classList.remove('show');
  }
});
document.addEventListener('dragover', e => e.preventDefault());
document.addEventListener('drop', async e => {
  e.preventDefault();
  dragCounter = 0;
  $('#dropOverlay').classList.remove('show');
  // Get folder path from drop
  const items = e.dataTransfer?.items;
  if (items && items.length > 0) {
    // In a web browser, we can't get actual folder paths from drag-and-drop
    // So we'll prompt the user with the Smart Clean view
    $$('.nav-item').forEach(n => n.classList.remove('active'));
    $$('.nav-item[data-view="smartclean"]').forEach(n => n.classList.add('active'));
    state.view = 'smartclean';
    render();
  }
});

// ═══════════════════════════════════════════════════════════════
// VIEWS
// ═══════════════════════════════════════════════════════════════

function render() {
  const main = $('#mainContent');
  const title = $('#viewTitle');
  const views = {
    dashboard: { title: 'Dashboard', render: renderDashboard },
    smartclean: { title: 'Smart Clean', render: renderSmartClean },
    duplicates: { title: 'Duplicate Finder', render: renderDuplicates },
    scanner: { title: 'File Scanner', render: renderScanner },
    stats: { title: 'Statistics', render: renderStats },
    ignore: { title: 'Ignore Rules', render: renderIgnore },
    history: { title: 'History', render: renderHistory },
  };
  const v = views[state.view] || views.dashboard;
  title.textContent = v.title;
  main.innerHTML = '';
  v.render(main);
}

// ═══════════════════════════════════════════════════════════════
// DASHBOARD
// ═══════════════════════════════════════════════════════════════

function renderDashboard(el) {
  el.innerHTML = `
    <div class="hero fade-in">
      <h2>Point it at chaos. Get back order.</h2>
      <p>BobUI scans millions of files, finds every duplicate, and organizes your mess — insanely fast with multi-threaded hashing.</p>
      <div class="hero-actions">
        <button class="hero-btn hero-btn-primary" id="heroSmartClean">🧹 Smart Clean</button>
        <button class="hero-btn hero-btn-secondary" id="heroDupes">🔍 Find Duplicates</button>
        <button class="hero-btn hero-btn-secondary" id="heroStats">📊 Analyze</button>
      </div>
    </div>

    <div class="quick-stats fade-in" style="animation-delay:0.1s">
      <div class="quick-stat" id="qsFiles">
        <div class="qs-icon">📂</div>
        <div class="qs-value">—</div>
        <div class="qs-label">Files Tracked</div>
      </div>
      <div class="quick-stat" id="qsSize">
        <div class="qs-icon">💾</div>
        <div class="qs-value">—</div>
        <div class="qs-label">Total Size</div>
      </div>
      <div class="quick-stat" id="qsDupes">
        <div class="qs-icon">🔍</div>
        <div class="qs-value">—</div>
        <div class="qs-label">Duplicate Groups</div>
      </div>
      <div class="quick-stat" id="qsRules">
        <div class="qs-icon">🚫</div>
        <div class="qs-value">${state.ignoreRules.length}</div>
        <div class="qs-label">Ignore Rules</div>
      </div>
    </div>

    <div style="display:grid;grid-template-columns:1fr 1fr;gap:16px;">
      <div class="card fade-in" style="animation-delay:0.2s">
        <div class="card-header">
          <span class="card-title">⚡ Quick Scan</span>
        </div>
        <div class="scan-row">
          <input type="text" id="dashPath" placeholder="Drag a folder here or type a path..." value="${state.scanPath}">
          <button class="btn btn-primary" id="dashScanBtn">Scan</button>
        </div>
        <p style="font-size:12px;color:var(--text-muted);margin-top:8px;">
          Tip: Drop any folder onto BobUI to start a Smart Clean
        </p>
      </div>
      <div class="card fade-in" style="animation-delay:0.3s">
        <div class="card-header">
          <span class="card-title">💡 Capabilities</span>
        </div>
        <div style="font-size:12px;color:var(--text-secondary);line-height:2;">
          ✅ Multi-threaded parallel hashing<br>
          ✅ 3 verification modes (Fast / Safe / Paranoid)<br>
          ✅ Size, extension, and glob filters<br>
          ✅ Auto-mark duplicates (keep oldest/newest/shortest/longest)<br>
          ✅ AWS S3, Google Drive, Azure Blob support<br>
          ✅ Ignore rules management<br>
          ✅ JSON/CSV/HTML export
        </div>
      </div>
    </div>
  `;

  // Hero buttons
  $('#heroSmartClean').onclick = () => navigateTo('smartclean');
  $('#heroDupes').onclick = () => navigateTo('duplicates');
  $('#heroStats').onclick = () => navigateTo('stats');

  // Quick scan
  $('#dashScanBtn').onclick = () => {
    state.scanPath = $('#dashPath')?.value?.trim() || '';
    if (state.scanPath) navigateTo('smartclean');
  };

  // Load quick stats if a path is available
  if (state.scanPath) loadDashStats();
}

async function loadDashStats() {
  try {
    const count = await api('count', { paths: [state.scanPath] });
    const qsFiles = $('#qsFiles .qs-value');
    if (qsFiles) qsFiles.textContent = count.count?.toLocaleString() || '—';
  } catch (e) { }
}

function navigateTo(view) {
  $$('.nav-item').forEach(n => n.classList.remove('active'));
  $$(`[data-view="${view}"]`).forEach(n => n.classList.add('active'));
  state.view = view;
  render();
}

// ═══════════════════════════════════════════════════════════════
// SMART CLEAN WIZARD
// ═══════════════════════════════════════════════════════════════

function renderSmartClean(el) {
  const steps = ['Select Folder', 'Scanning', 'Find Duplicates', 'Review & Clean'];

  el.innerHTML = `
    <div class="wizard-steps fade-in">
      ${steps.map((s, i) => `
        <div class="wizard-step ${i < state.wizardStep ? 'done' : ''} ${i === state.wizardStep ? 'active' : ''}">
          <span class="step-num">${i < state.wizardStep ? '✓' : i + 1}</span> ${s}
        </div>
      `).join('')}
    </div>
    <div id="wizardContent"></div>
  `;

  const content = $('#wizardContent');
  if (state.wizardStep === 0) renderWizardSelect(content);
  else if (state.wizardStep === 1) renderWizardScanning(content);
  else if (state.wizardStep === 2) renderWizardDeduping(content);
  else if (state.wizardStep === 3) renderWizardReview(content);
}

function renderWizardSelect(el) {
  el.innerHTML = `
    <div class="card fade-in" style="max-width:600px;margin:0 auto;text-align:center;padding:48px">
      <div style="font-size:64px;margin-bottom:16px;">🧹</div>
      <h3 style="font-size:20px;margin-bottom:8px;">Smart Clean</h3>
      <p style="color:var(--text-secondary);margin-bottom:24px;font-size:13px;">
        Point it at any folder. BobUI will scan everything, find all duplicates,<br>and let you clean up with one click.
      </p>
      <div class="scan-row" style="max-width:450px;margin:0 auto;">
        <input type="text" id="wizardPath" placeholder="C:\\Users\\Photos or any folder..." value="${state.wizardPath || state.scanPath}">
        <button class="btn btn-primary btn-lg" id="wizardStartBtn">🚀 Start</button>
      </div>
      <div style="margin-top:16px;display:flex;gap:8px;justify-content:center;flex-wrap:wrap;">
        <div class="filter-group" style="text-align:left;">
          <label>Mode</label>
          <div class="mode-selector">
            <button class="mode-btn ${state.mode === 'fast' ? 'active' : ''}" data-mode="fast">⚡ Fast</button>
            <button class="mode-btn ${state.mode === 'safe' ? 'active' : ''}" data-mode="safe">🔒 Safe</button>
            <button class="mode-btn ${state.mode === 'paranoid' ? 'active' : ''}" data-mode="paranoid">🛡️ Paranoid</button>
          </div>
        </div>
        <div class="filter-group" style="text-align:left;">
          <label>Threads</label>
          <input type="number" id="wizardThreads" value="${state.threads}" min="1" max="64" style="width:70px;">
        </div>
      </div>
    </div>
  `;

  $$('.mode-btn').forEach(b => {
    b.onclick = () => { state.mode = b.dataset.mode; render(); };
  });

  $('#wizardStartBtn').onclick = async () => {
    const path = $('#wizardPath')?.value?.trim();
    if (!path) { alert('Please enter a folder path'); return; }
    state.wizardPath = path;
    state.scanPath = path;
    state.threads = parseInt($('#wizardThreads')?.value) || 4;
    state.wizardStep = 1;
    render();
    // Step 1: Scan + Stats
    try {
      state.wizardStats = await api('stats', { paths: [path] });
    } catch (e) { state.wizardStats = { total_files: 0, total_size: 0 }; }
    state.wizardStep = 2;
    render();
    // Step 2: Find duplicates
    try {
      state.wizardDupes = await api('duplicates', { paths: [path], mode: state.mode, threads: state.threads });
    } catch (e) { state.wizardDupes = []; }
    state.wizardStep = 3;
    state.markedFiles.clear();
    state.expandedGroups.clear();
    // Auto-mark: keep oldest by default
    for (const g of state.wizardDupes) {
      if (g.files && g.files.length > 1) {
        g.files.slice(1).forEach(f => state.markedFiles.add(f));
      }
    }
    // Auto-expand first 10
    state.wizardDupes.slice(0, 10).forEach((_, i) => state.expandedGroups.add(i));
    render();
  };
}

function renderWizardScanning(el) {
  el.innerHTML = `
    <div class="card fade-in" style="text-align:center;padding:48px">
      <div style="font-size:48px;margin-bottom:16px;" class="scanning">📂</div>
      <h3 style="font-size:18px;margin-bottom:8px;">Scanning files...</h3>
      <p style="color:var(--text-secondary);font-size:13px;margin-bottom:16px;">${state.wizardPath}</p>
      <div class="big-progress"><div class="fill" style="width:35%"></div></div>
      <p style="color:var(--text-muted);font-size:12px;">Analyzing file system and gathering statistics</p>
    </div>
  `;
}

function renderWizardDeduping(el) {
  const s = state.wizardStats;
  el.innerHTML = `
    <div class="quick-stats fade-in" style="margin-bottom:16px;">
      <div class="quick-stat">
        <div class="qs-icon">📂</div>
        <div class="qs-value">${(s?.total_files || 0).toLocaleString()}</div>
        <div class="qs-label">Files Found</div>
      </div>
      <div class="quick-stat">
        <div class="qs-icon">💾</div>
        <div class="qs-value">${fmtSize(s?.total_size || 0)}</div>
        <div class="qs-label">Total Size</div>
      </div>
      <div class="quick-stat">
        <div class="qs-icon">📁</div>
        <div class="qs-value">${(s?.extensions || []).length}</div>
        <div class="qs-label">File Types</div>
      </div>
      <div class="quick-stat">
        <div class="qs-icon">⏱️</div>
        <div class="qs-value">${state.mode}</div>
        <div class="qs-label">Verification</div>
      </div>
    </div>
    <div class="card fade-in" style="text-align:center;padding:48px">
      <div style="font-size:48px;margin-bottom:16px;" class="scanning">🔍</div>
      <h3 style="font-size:18px;margin-bottom:8px;">Finding duplicates...</h3>
      <p style="color:var(--text-secondary);font-size:13px;margin-bottom:16px;">
        Hashing ${(s?.total_files || 0).toLocaleString()} files with ${state.threads} threads (${state.mode} mode)
      </p>
      <div class="big-progress"><div class="fill" style="width:70%"></div></div>
    </div>
  `;
}

function renderWizardReview(el) {
  const dupes = state.wizardDupes;
  const totalDupeFiles = dupes.reduce((s, g) => s + g.files.length - 1, 0);
  const wastedBytes = dupes.reduce((s, g) => s + g.size * (g.files.length - 1), 0);
  const s = state.wizardStats;
  const markedCount = state.markedFiles.size;

  el.innerHTML = `
    <div class="quick-stats fade-in" style="margin-bottom:16px;">
      <div class="quick-stat">
        <div class="qs-icon">📂</div>
        <div class="qs-value">${(s?.total_files || 0).toLocaleString()}</div>
        <div class="qs-label">Total Files</div>
      </div>
      <div class="quick-stat">
        <div class="qs-icon">🔍</div>
        <div class="qs-value">${dupes.length}</div>
        <div class="qs-label">Dupe Groups</div>
      </div>
      <div class="quick-stat">
        <div class="qs-icon">⚠️</div>
        <div class="qs-value">${totalDupeFiles}</div>
        <div class="qs-label">Duplicate Files</div>
      </div>
      <div class="quick-stat">
        <div class="qs-icon">💰</div>
        <div class="qs-value">${fmtSize(wastedBytes)}</div>
        <div class="qs-label">Reclaimable</div>
      </div>
    </div>

    ${dupes.length === 0 ? `
      <div class="card fade-in" style="text-align:center;padding:48px">
        <div style="font-size:48px;margin-bottom:16px;">✅</div>
        <h3 style="font-size:18px;margin-bottom:8px;color:var(--success)">No duplicates found!</h3>
        <p style="color:var(--text-secondary);font-size:13px;">Your folder is clean. No duplicate files detected.</p>
        <button class="btn btn-primary" style="margin-top:16px" onclick="state.wizardStep=0;render();">Scan Another Folder</button>
      </div>
    ` : `
      <div class="card fade-in">
        <div class="card-header">
          <span class="card-title">
            Review Duplicates
            <span class="pill pill-accent">${dupes.length} groups</span>
            <span class="pill pill-danger">${markedCount} marked for deletion</span>
          </span>
          <div style="display:flex;gap:8px;flex-wrap:wrap;">
            <select id="wizAutoMark" style="width:180px;">
              <option value="">Auto-mark strategy...</option>
              <option value="newest" selected>Keep Oldest</option>
              <option value="oldest">Keep Newest</option>
              <option value="shortest">Keep Shortest Path</option>
              <option value="longest">Keep Longest Path</option>
            </select>
            ${markedCount > 0 ? `<button class="btn btn-danger" id="wizDeleteBtn">🗑️ Delete ${markedCount} files (${fmtSize(wastedBytes)})</button>` : ''}
            <button class="btn btn-sm" id="wizExpandAll">Expand All</button>
            <button class="btn btn-sm" id="wizClearMarks">Clear</button>
          </div>
        </div>
        <div style="max-height:500px;overflow-y:auto;">
          <table class="dupe-table">
            <thead><tr><th style="width:30px"></th><th style="width:30px"></th><th>File</th><th style="width:100px">Size</th></tr></thead>
            <tbody>
              ${dupes.map((g, gi) => renderGroupRows(g, gi)).join('')}
            </tbody>
          </table>
        </div>
      </div>
      <div style="text-align:center;margin-top:16px;">
        <button class="btn" onclick="state.wizardStep=0;render();">← Start Over</button>
      </div>
    `}
  `;

  bindGroupActions();
  bindWizardActions();
}

// ═══════════════════════════════════════════════════════════════
// DUPLICATE FINDER (standalone)
// ═══════════════════════════════════════════════════════════════

function renderDuplicates(el) {
  el.innerHTML = `
    <div class="card fade-in" style="margin-bottom: 16px">
      <div class="card-header">
        <span class="card-title">🔍 Scan Configuration</span>
        ${state.scanning ? '<span class="pill pill-accent scanning">Scanning...</span>' : ''}
      </div>
      <div class="scan-controls">
        <div class="scan-row">
          <input type="text" id="dupePath" placeholder="Enter directory path (e.g., C:\\Users\\Photos)" value="${state.scanPath}">
          <button class="btn btn-primary" id="scanDupeBtn" ${state.scanning ? 'disabled' : ''}>
            ${state.scanning ? '⏳ Scanning...' : '🔍 Find Duplicates'}
          </button>
        </div>
        <div style="display:flex;gap:16px;align-items:center;flex-wrap:wrap;">
          <div>
            <label style="font-size:11px;color:var(--text-muted);margin-bottom:4px;display:block;">VERIFICATION MODE</label>
            <div class="mode-selector">
              <button class="mode-btn ${state.mode === 'fast' ? 'active' : ''}" data-mode="fast">⚡ Fast</button>
              <button class="mode-btn ${state.mode === 'safe' ? 'active' : ''}" data-mode="safe">🔒 Safe</button>
              <button class="mode-btn ${state.mode === 'paranoid' ? 'active' : ''}" data-mode="paranoid">🛡️ Paranoid</button>
            </div>
          </div>
          <div class="filter-group"><label>Threads</label><input type="number" id="dupeThreads" value="${state.threads}" min="1" max="64" style="width:60px"></div>
          <div class="filter-group"><label>Extensions</label><input type="text" id="dupeExts" value="${state.exts}" placeholder=".jpg,.png" style="width:120px"></div>
          <div class="filter-group"><label>Min Size</label><input type="text" id="dupeMinSize" value="${state.minSize}" placeholder="1M" style="width:80px"></div>
          <div class="filter-group"><label>Max Size</label><input type="text" id="dupeMaxSize" value="${state.maxSize}" placeholder="1G" style="width:80px"></div>
          <div class="toggle-row" style="margin-top:14px;">
            <div class="toggle ${state.recursive ? 'on' : ''}" id="recursiveToggle"></div>
            <span>Recursive</span>
          </div>
        </div>
      </div>
    </div>
    ${state.dupeGroups.length > 0 ? renderDupeResults() : `
      <div class="empty-state fade-in">
        <div class="icon">🔍</div>
        <h3>No duplicates found yet</h3>
        <p>Enter a path and click "Find Duplicates" to scan.</p>
      </div>
    `}
  `;

  $('#scanDupeBtn').onclick = startDupeScan;
  $$('.mode-btn').forEach(b => { b.onclick = () => { state.mode = b.dataset.mode; render(); }; });
  const toggle = $('#recursiveToggle');
  if (toggle) toggle.onclick = () => { state.recursive = !state.recursive; render(); };
  bindGroupActions();
  bindDupeActions();
}

function renderDupeResults() {
  const totalDupes = state.dupeGroups.reduce((s, g) => s + g.files.length - 1, 0);
  const wastedBytes = state.dupeGroups.reduce((s, g) => s + g.size * (g.files.length - 1), 0);
  const markedCount = state.markedFiles.size;

  return `
    <div class="card fade-in">
      <div class="card-header">
        <span class="card-title">
          Results <span class="pill pill-accent">${state.dupeGroups.length} groups</span>
          <span class="pill pill-danger">${totalDupes} duplicates</span>
          <span class="pill pill-accent">${fmtSize(wastedBytes)} reclaimable</span>
        </span>
        <div style="display:flex;gap:8px;">
          <select id="autoMarkSelect" style="width:180px;">
            <option value="">Auto-mark strategy...</option>
            <option value="newest">Keep Oldest</option>
            <option value="oldest">Keep Newest</option>
            <option value="shortest">Keep Shortest Path</option>
            <option value="longest">Keep Longest Path</option>
          </select>
          ${markedCount > 0 ? `<button class="btn btn-danger" id="deleteMarkedBtn">🗑️ Delete ${markedCount}</button>` : ''}
          <button class="btn btn-sm" id="expandAllBtn">Expand All</button>
          <button class="btn btn-sm" id="clearMarksBtn">Clear</button>
        </div>
      </div>
      <div style="max-height:500px;overflow-y:auto;">
        <table class="dupe-table">
          <thead><tr><th style="width:30px"></th><th style="width:30px"></th><th>File</th><th style="width:100px">Size</th></tr></thead>
          <tbody>${state.dupeGroups.map((g, gi) => renderGroupRows(g, gi)).join('')}</tbody>
        </table>
      </div>
    </div>
  `;
}

// ── Shared group rendering ──
function renderGroupRows(group, idx) {
  const expanded = state.expandedGroups.has(idx);
  const rows = group.files.map((f, fi) => {
    const isMarked = state.markedFiles.has(f);
    const isKept = !isMarked && fi === 0;
    return `
      <tr class="file-row ${isMarked ? 'marked' : ''} ${isKept ? 'kept' : ''}" style="display:${expanded ? '' : 'none'}">
        <td></td>
        <td><input type="checkbox" class="checkbox file-check" data-path="${f}" ${isMarked ? 'checked' : ''}></td>
        <td class="file-path" title="${f}"><span class="ftype">${fileIcon(f)}</span>${f}</td>
        <td class="file-size">${fmtSize(group.size)}</td>
      </tr>
    `;
  }).join('');

  const hash = group.hash?.substring(0, 12) || group.fast64?.substring(0, 12) || '';
  return `
    <tr class="group-header ${expanded ? 'expanded' : ''}" data-idx="${idx}">
      <td><span class="chevron">▶</span></td>
      <td></td>
      <td><strong>Group ${idx + 1}</strong> — ${group.files.length} files ${hash ? `<span class="pill pill-accent" style="margin-left:6px">${hash}</span>` : ''}</td>
      <td class="file-size">${fmtSize(group.size)}</td>
    </tr>
    ${rows}
  `;
}

function bindGroupActions() {
  $$('.group-header').forEach(h => {
    h.onclick = () => {
      const idx = parseInt(h.dataset.idx);
      state.expandedGroups.has(idx) ? state.expandedGroups.delete(idx) : state.expandedGroups.add(idx);
      render();
    };
  });
  $$('.file-check').forEach(cb => {
    cb.onchange = () => {
      cb.checked ? state.markedFiles.add(cb.dataset.path) : state.markedFiles.delete(cb.dataset.path);
      render();
    };
  });
}

function bindDupeActions() {
  const autoSelect = $('#autoMarkSelect');
  if (autoSelect) autoSelect.onchange = () => applyAutoMark(autoSelect.value, state.dupeGroups);
  const deleteBtn = $('#deleteMarkedBtn');
  if (deleteBtn) deleteBtn.onclick = showDeleteModal;
  const expandBtn = $('#expandAllBtn');
  if (expandBtn) expandBtn.onclick = () => { state.dupeGroups.forEach((_, i) => state.expandedGroups.add(i)); render(); };
  const clearBtn = $('#clearMarksBtn');
  if (clearBtn) clearBtn.onclick = () => { state.markedFiles.clear(); render(); };
}

function bindWizardActions() {
  const autoSelect = $('#wizAutoMark');
  if (autoSelect) autoSelect.onchange = () => applyAutoMark(autoSelect.value, state.wizardDupes);
  const deleteBtn = $('#wizDeleteBtn');
  if (deleteBtn) deleteBtn.onclick = showDeleteModal;
  const expandBtn = $('#wizExpandAll');
  if (expandBtn) expandBtn.onclick = () => { state.wizardDupes.forEach((_, i) => state.expandedGroups.add(i)); render(); };
  const clearBtn = $('#wizClearMarks');
  if (clearBtn) clearBtn.onclick = () => { state.markedFiles.clear(); render(); };
}

async function startDupeScan() {
  const path = $('#dupePath')?.value?.trim();
  if (!path) return;
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
    state.dupeGroups = await api('duplicates', { paths: [path], mode: state.mode, threads: state.threads, ...getFilters() });
    state.dupeGroups.slice(0, 5).forEach((_, i) => state.expandedGroups.add(i));
  } catch (e) { alert('Scan error: ' + e.message); }
  state.scanning = false;
  render();
}

function applyAutoMark(strategy, groups) {
  if (!strategy) return;
  state.markedFiles.clear();
  for (const g of groups) {
    const sorted = [...g.files];
    if (strategy === 'newest') sorted.slice(1).forEach(f => state.markedFiles.add(f));
    else if (strategy === 'oldest') sorted.slice(0, -1).forEach(f => state.markedFiles.add(f));
    else if (strategy === 'shortest') { sorted.sort((a, b) => a.length - b.length); sorted.slice(1).forEach(f => state.markedFiles.add(f)); }
    else if (strategy === 'longest') { sorted.sort((a, b) => b.length - a.length); sorted.slice(1).forEach(f => state.markedFiles.add(f)); }
  }
  render();
}

function showDeleteModal() {
  const modal = $('#deleteModal');
  $('#deleteModalText').textContent = `Permanently delete ${state.markedFiles.size} files? This cannot be undone.`;
  modal.classList.add('show');
  $('#confirmDeleteBtn').onclick = async () => {
    closeModal();
    try {
      const result = await api('delete', { files: [...state.markedFiles] });
      alert(`Deleted ${result.deleted} files.`);
      // Remove from both dupe groups and wizard dupes
      for (const list of [state.dupeGroups, state.wizardDupes]) {
        for (const g of list) g.files = g.files.filter(f => !state.markedFiles.has(f));
      }
      state.dupeGroups = state.dupeGroups.filter(g => g.files.length >= 2);
      state.wizardDupes = state.wizardDupes.filter(g => g.files.length >= 2);
      state.markedFiles.clear();
      render();
    } catch (e) { alert('Error: ' + e.message); }
  };
}

function closeModal() { $('#deleteModal').classList.remove('show'); }

// ═══════════════════════════════════════════════════════════════
// SCANNER
// ═══════════════════════════════════════════════════════════════

function renderScanner(el) {
  el.innerHTML = `
    <div class="card fade-in" style="margin-bottom:16px">
      <div class="card-header"><span class="card-title">📂 Scan Directory</span></div>
      <div class="scan-controls">
        <div class="scan-row">
          <input type="text" id="scannerPath" placeholder="Enter directory path..." value="${state.scanPath}">
          <button class="btn btn-primary" id="scanFilesBtn" ${state.scanning ? 'disabled' : ''}>
            ${state.scanning ? '⏳ Scanning...' : '📂 Scan'}
          </button>
        </div>
        <div class="filter-grid">
          <div class="filter-group"><label>Extensions</label><input type="text" id="scanExts" value="${state.exts}" placeholder=".jpg,.png,.pdf"></div>
          <div class="filter-group"><label>Exclude</label><input type="text" id="scanExcludes" value="${state.excludes}" placeholder="*.tmp, node_modules"></div>
          <div class="filter-group"><label>Min Size</label><input type="text" id="scanMinSize" value="${state.minSize}" placeholder="100K"></div>
          <div class="filter-group"><label>Max Size</label><input type="text" id="scanMaxSize" value="${state.maxSize}" placeholder="500M"></div>
        </div>
      </div>
    </div>
    ${state.scanFiles.length > 0 ? `
      <div class="card fade-in">
        <div class="card-header"><span class="card-title">Results <span class="pill pill-accent">${state.scanFiles.length} files</span></span></div>
        <div style="max-height:500px;overflow-y:auto;">
          <table class="dupe-table">
            <thead><tr><th>File</th><th style="width:100px">Size</th></tr></thead>
            <tbody>
              ${state.scanFiles.slice(0, 500).map(f => `
                <tr class="file-row">
                  <td class="file-path" title="${f.path}"><span class="ftype">${fileIcon(f.path)}</span>${f.path}</td>
                  <td class="file-size">${fmtSize(f.size)}</td>
                </tr>
              `).join('')}
              ${state.scanFiles.length > 500 ? `<tr><td colspan="2" style="text-align:center;color:var(--text-muted);padding:12px;">...and ${state.scanFiles.length - 500} more</td></tr>` : ''}
            </tbody>
          </table>
        </div>
      </div>
    ` : '<div class="empty-state fade-in"><div class="icon">📂</div><h3>No files scanned</h3><p>Enter a path and click scan.</p></div>'}
  `;
  $('#scanFilesBtn').onclick = async () => {
    const path = $('#scannerPath')?.value?.trim();
    if (!path) return;
    state.scanPath = path;
    state.exts = $('#scanExts')?.value || '';
    state.excludes = $('#scanExcludes')?.value || '';
    state.minSize = $('#scanMinSize')?.value || '';
    state.maxSize = $('#scanMaxSize')?.value || '';
    state.scanning = true; render();
    try { state.scanFiles = await api('scan', { paths: [path], ...getFilters() }); }
    catch (e) { alert('Error: ' + e.message); }
    state.scanning = false; render();
  };
}

// ═══════════════════════════════════════════════════════════════
// STATS
// ═══════════════════════════════════════════════════════════════

function renderStats(el) {
  el.innerHTML = `
    <div class="card fade-in" style="margin-bottom:16px">
      <div class="card-header"><span class="card-title">📊 File Statistics</span></div>
      <div class="scan-row">
        <input type="text" id="statsPath" placeholder="Enter directory path..." value="${state.scanPath}">
        <button class="btn btn-primary" id="statsBtn">📊 Analyze</button>
      </div>
    </div>
    ${state.stats ? renderStatsBody() : '<div class="empty-state fade-in"><div class="icon">📊</div><h3>No statistics yet</h3><p>Enter a path and click analyze.</p></div>'}
  `;
  $('#statsBtn').onclick = async () => {
    const path = $('#statsPath')?.value?.trim();
    if (!path) return;
    state.scanPath = path;
    try { state.stats = await api('stats', { paths: [path] }); } catch (e) { alert('Error: ' + e.message); }
    render();
  };
}

function renderStatsBody() {
  const s = state.stats;
  const exts = s.extensions || [];
  return `
    <div class="stats-grid fade-in">
      <div class="stat-card"><div class="label">Total Files</div><div class="value">${(s.total_files || 0).toLocaleString()}</div></div>
      <div class="stat-card"><div class="label">Total Size</div><div class="value">${fmtSize(s.total_size || 0)}</div></div>
      <div class="stat-card"><div class="label">Extensions</div><div class="value">${exts.length}</div></div>
      <div class="stat-card"><div class="label">Oldest</div><div class="value" style="font-size:14px">${s.oldest || '—'}</div></div>
      <div class="stat-card"><div class="label">Newest</div><div class="value" style="font-size:14px">${s.newest || '—'}</div></div>
    </div>
    ${exts.length > 0 ? `
      <div class="card fade-in"><div class="card-header"><span class="card-title">Extension Breakdown</span></div>
        ${exts.slice(0, 25).map(e => {
    const pct = s.total_files ? (e.count / s.total_files * 100) : 0;
    return `<div class="ext-bar"><span class="ext-name">${e.ext}</span><div class="ext-fill"><div class="ext-fill-inner" style="width:${pct}%"></div></div><span class="ext-count">${e.count}</span></div>`;
  }).join('')}
      </div>
    ` : ''}
  `;
}

// ═══════════════════════════════════════════════════════════════
// IGNORE & HISTORY
// ═══════════════════════════════════════════════════════════════

function renderIgnore(el) {
  el.innerHTML = `
    <div class="card fade-in" style="margin-bottom:16px">
      <div class="card-header"><span class="card-title">🚫 Add Ignore Rule</span></div>
      <div class="scan-row">
        <input type="text" id="ignorePattern" placeholder="Pattern (e.g., *.tmp, thumbs.db)">
        <input type="text" id="ignoreReason" placeholder="Reason (optional)" style="width:200px">
        <button class="btn btn-primary" id="addIgnoreBtn">Add</button>
      </div>
    </div>
    <div class="card fade-in">
      <div class="card-header">
        <span class="card-title">Active Rules <span class="pill pill-accent">${state.ignoreRules.length}</span></span>
        <button class="btn btn-sm" id="refreshIgnoreBtn">↻ Refresh</button>
      </div>
      ${state.ignoreRules.length === 0 ? '<div class="empty-state" style="padding:30px"><p>No ignore rules configured.</p></div>'
      : state.ignoreRules.map(r => `
          <div class="rule-item">
            <div><span>${r.pattern}</span>${r.reason ? `<span class="rule-reason"> — ${r.reason}</span>` : ''}</div>
            <button class="btn btn-sm btn-danger remove-ignore" data-pattern="${r.pattern}">✕</button>
          </div>
        `).join('')}
    </div>
  `;
  $('#addIgnoreBtn').onclick = async () => {
    const pattern = $('#ignorePattern')?.value?.trim();
    if (!pattern) return;
    await api('ignore/add', { pattern, reason: $('#ignoreReason')?.value?.trim() || '' });
    await loadIgnoreRules(); render();
  };
  $('#refreshIgnoreBtn').onclick = async () => { await loadIgnoreRules(); render(); };
  $$('.remove-ignore').forEach(b => {
    b.onclick = async () => { await api('ignore/remove', { pattern: b.dataset.pattern }); await loadIgnoreRules(); render(); };
  });
}

function renderHistory(el) {
  el.innerHTML = `
    <div class="card fade-in">
      <div class="card-header">
        <span class="card-title">📜 Operation History</span>
        <button class="btn btn-sm" id="refreshHistoryBtn">↻ Refresh</button>
      </div>
      ${state.history.length === 0 ? '<div class="empty-state" style="padding:30px"><p>No operations recorded.</p></div>'
      : `<table class="dupe-table"><thead><tr><th>Time</th><th>Type</th><th>Source</th><th>Dest</th><th>Status</th></tr></thead><tbody>
          ${state.history.map(op => `
            <tr class="file-row">
              <td style="white-space:nowrap;font-size:12px;color:var(--text-secondary)">${op.timestamp || ''}</td>
              <td><span class="pill pill-accent">${op.type || ''}</span></td>
              <td class="file-path" title="${op.source || ''}">${fmtPath(op.source || '')}</td>
              <td class="file-path" title="${op.dest || ''}">${op.dest ? fmtPath(op.dest) : '—'}</td>
              <td>${op.undone ? '<span class="pill pill-danger">undone</span>' : '<span class="pill pill-success">done</span>'}</td>
            </tr>
          `).join('')}
        </tbody></table>`}
    </div>
  `;
  $('#refreshHistoryBtn').onclick = async () => {
    try { state.history = await api('history'); } catch (e) { }
    render();
  };
}

async function loadIgnoreRules() {
  try { state.ignoreRules = await api('ignore'); } catch (e) { state.ignoreRules = []; }
}

// ── Init ──
async function init() {
  render();
  try { state.ignoreRules = await api('ignore'); } catch (e) { }
  try { state.history = await api('history'); } catch (e) { }
}
init();
