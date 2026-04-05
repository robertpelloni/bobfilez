import React, { useEffect, useMemo, useState } from 'https://esm.sh/react@18';
import { createRoot } from 'https://esm.sh/react-dom@18/client';

function Card({ title, children }) {
  return React.createElement('div', {
    style: {
      background: 'rgba(15, 23, 42, 0.9)',
      border: '1px solid rgba(148, 163, 184, 0.2)',
      borderRadius: 16,
      padding: 20,
      boxShadow: '0 20px 60px rgba(0,0,0,0.35)'
    }
  }, [
    React.createElement('h3', { key: 'title', style: { marginTop: 0 } }, title),
    children
  ]);
}

function App() {
  const [activeTab, setActiveTab] = useState('dashboard');
  const [apiStatus, setApiStatus] = useState('checking');
  const [version] = useState('6.x multi-frontend preview');
  
  const [scanPath, setScanPath] = useState('');
  const [scanResults, setScanResults] = useState(null);
  const [isScanning, setIsScanning] = useState(false);

  useEffect(() => {
    fetch('/api/health')
      .then((res) => setApiStatus(res.ok ? 'online' : 'degraded'))
      .catch(() => setApiStatus('offline'));
  }, []);

  const handleScan = async (e) => {
    e.preventDefault();
    if (!scanPath) return;
    setIsScanning(true);
    setScanResults(null);
    try {
      const res = await fetch('/api/scan', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ paths: [scanPath], recursive: true })
      });
      const data = await res.json();
      setScanResults(data);
    } catch (e) {
      setScanResults({ error: e.message });
    }
    setIsScanning(false);
  };

  const statusColor = useMemo(() => {
    if (apiStatus === 'online') return '#34d399';
    if (apiStatus === 'degraded') return '#fbbf24';
    if (apiStatus === 'checking') return '#60a5fa';
    return '#f87171';
  }, [apiStatus]);

  const renderDashboard = () => React.createElement('div', {
    style: { display: 'grid', gap: 24, gridTemplateColumns: 'repeat(auto-fit, minmax(260px, 1fr))' }
  }, [
    React.createElement(Card, { key: 'qt', title: 'Qt / BobUI' }, React.createElement('p', null, 'Modern native shell path using Qt6 with BobUI Omni wiring.')),
    React.createElement(Card, { key: 'juce', title: 'JUCE' }, React.createElement('p', null, 'Audio/media-oriented native lane with a dedicated JUCE demo target that calls fo_core directly.')),
    React.createElement(Card, { key: 'btk', title: 'BTK / CopperSpice' }, React.createElement('p', null, 'Separate native research lane with a lightweight widget demo target.')),
    React.createElement(Card, { key: 'bobgui', title: 'BobGUI' }, React.createElement('p', null, 'GTK-style Meson lane with a demo application scaffold.'))
  ]);

  const renderScanner = () => React.createElement(Card, { title: 'File Scanner' }, [
    React.createElement('form', { key: 'form', onSubmit: handleScan, style: { display: 'flex', gap: 12, marginBottom: 20 } }, [
      React.createElement('input', {
        key: 'input',
        type: 'text',
        value: scanPath,
        onChange: e => setScanPath(e.target.value),
        placeholder: 'Enter absolute directory path (e.g. C:/Users/Public)',
        style: { flex: 1, padding: '10px 14px', borderRadius: 8, border: '1px solid #334155', background: '#0f172a', color: '#fff' }
      }),
      React.createElement('button', {
        key: 'btn',
        type: 'submit',
        disabled: isScanning,
        style: { padding: '10px 24px', borderRadius: 8, background: '#3b82f6', color: '#fff', border: 'none', cursor: isScanning ? 'wait' : 'pointer' }
      }, isScanning ? 'Scanning...' : 'Scan')
    ]),
    scanResults && (
      scanResults.error ? React.createElement('div', { key: 'err', style: { color: '#f87171' } }, scanResults.error) :
      React.createElement('div', { key: 'res' }, [
        React.createElement('p', { key: 'count' }, `Found ${scanResults.length || 0} files.`),
        Array.isArray(scanResults) && scanResults.length > 0 && React.createElement('div', { key: 'table', style: { maxHeight: 400, overflowY: 'auto', background: '#0f172a', borderRadius: 8 } },
          React.createElement('table', { style: { width: '100%', textAlign: 'left', borderCollapse: 'collapse' } }, [
            React.createElement('thead', { key: 'th' }, React.createElement('tr', null, [
              React.createElement('th', { style: { padding: 12, borderBottom: '1px solid #1e293b' } }, 'Filename'),
              React.createElement('th', { style: { padding: 12, borderBottom: '1px solid #1e293b' } }, 'Size (bytes)')
            ])),
            React.createElement('tbody', { key: 'tb' }, scanResults.slice(0, 100).map((f, i) => React.createElement('tr', { key: i }, [
              React.createElement('td', { style: { padding: '8px 12px', borderBottom: '1px solid #1e293b' } }, f.filename),
              React.createElement('td', { style: { padding: '8px 12px', borderBottom: '1px solid #1e293b' } }, f.size)
            ])))
          ])
        ),
        Array.isArray(scanResults) && scanResults.length > 100 && React.createElement('p', { key: 'trunc', style: { color: '#94a3b8', fontSize: 13, marginTop: 8 } }, 'Showing first 100 files...')
      ])
    )
  ]);

  const [dupePath, setDupePath] = useState('');
  const [dupeResults, setDupeResults] = useState(null);
  const [isFindingDupes, setIsFindingDupes] = useState(false);

  const handleDupes = async (e) => {
    e.preventDefault();
    if (!dupePath) return;
    setIsFindingDupes(true);
    setDupeResults(null);
    try {
      const res = await fetch('/api/duplicates', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ paths: [dupePath], mode: 'fast', recursive: true })
      });
      const data = await res.json();
      setDupeResults(data);
    } catch (e) {
      setDupeResults({ error: e.message });
    }
    setIsFindingDupes(false);
  };

  const renderDuplicates = () => React.createElement(Card, { title: 'Duplicate Finder' }, [
    React.createElement('form', { key: 'form', onSubmit: handleDupes, style: { display: 'flex', gap: 12, marginBottom: 20 } }, [
      React.createElement('input', {
        key: 'input',
        type: 'text',
        value: dupePath,
        onChange: e => setDupePath(e.target.value),
        placeholder: 'Enter absolute directory path (e.g. C:/Users/Public)',
        style: { flex: 1, padding: '10px 14px', borderRadius: 8, border: '1px solid #334155', background: '#0f172a', color: '#fff' }
      }),
      React.createElement('button', {
        key: 'btn',
        type: 'submit',
        disabled: isFindingDupes,
        style: { padding: '10px 24px', borderRadius: 8, background: '#8b5cf6', color: '#fff', border: 'none', cursor: isFindingDupes ? 'wait' : 'pointer' }
      }, isFindingDupes ? 'Searching...' : 'Find Dupes')
    ]),
    dupeResults && (
      dupeResults.error ? React.createElement('div', { key: 'err', style: { color: '#f87171' } }, dupeResults.error) :
      React.createElement('div', { key: 'res' }, [
        React.createElement('p', { key: 'count' }, `Found ${dupeResults.length || 0} duplicate groups.`),
        Array.isArray(dupeResults) && dupeResults.length > 0 && React.createElement('div', { key: 'groups', style: { display: 'flex', flexDirection: 'column', gap: 16 } },
          dupeResults.slice(0, 50).map((group, i) => React.createElement('div', { key: i, style: { background: '#0f172a', border: '1px solid #1e293b', borderRadius: 8, padding: 16 } }, [
            React.createElement('div', { key: 'header', style: { display: 'flex', justifyContent: 'space-between', color: '#94a3b8', fontSize: 13, marginBottom: 8 } }, [
              React.createElement('span', { key: 'hash' }, `Hash: ${group.hash || group.fast64 || 'unknown'}`),
              React.createElement('span', { key: 'size' }, `Size: ${group.size || 0} bytes`)
            ]),
            React.createElement('ul', { key: 'list', style: { margin: 0, paddingLeft: 20 } },
              (group.files || []).map((f, j) => React.createElement('li', { key: j, style: { color: '#e2e8f0', wordBreak: 'break-all' } }, f.path || f.filename || f))
            )
          ]))
        ),
        Array.isArray(dupeResults) && dupeResults.length > 50 && React.createElement('p', { key: 'trunc', style: { color: '#94a3b8', fontSize: 13, marginTop: 8 } }, 'Showing first 50 groups...')
      ])
    )
  ]);

  return React.createElement('div', {
    style: { minHeight: '100vh', padding: 32, background: 'radial-gradient(circle at top, #16213a 0%, #0b1220 60%)', color: '#e2e8f0' }
  }, React.createElement('div', {
    style: { maxWidth: 1200, margin: '0 auto', display: 'grid', gap: 24 }
  }, [
    React.createElement('div', { key: 'header', style: { display: 'flex', justifyContent: 'space-between', alignItems: 'center' } }, [
      React.createElement('h1', { key: 'title', style: { margin: 0, fontSize: 28, color: '#f8fafc' } }, 'bobfilez React Web UI'),
      React.createElement('div', { key: 'status', style: { color: statusColor, fontWeight: 600, background: 'rgba(0,0,0,0.2)', padding: '6px 12px', borderRadius: 20 } }, 'API: ' + apiStatus)
    ]),
    
    // Navigation Tabs
    React.createElement('div', { key: 'nav', style: { display: 'flex', gap: 16, borderBottom: '1px solid rgba(148, 163, 184, 0.2)', paddingBottom: 16 } }, [
      React.createElement('button', { key: 'tab-dash', onClick: () => setActiveTab('dashboard'), style: { background: 'none', border: 'none', color: activeTab === 'dashboard' ? '#38bdf8' : '#94a3b8', cursor: 'pointer', fontSize: 16, fontWeight: activeTab === 'dashboard' ? 700 : 400 } }, 'Dashboard'),
      React.createElement('button', { key: 'tab-scan', onClick: () => setActiveTab('scanner'), style: { background: 'none', border: 'none', color: activeTab === 'scanner' ? '#38bdf8' : '#94a3b8', cursor: 'pointer', fontSize: 16, fontWeight: activeTab === 'scanner' ? 700 : 400 } }, 'Scanner'),
      React.createElement('button', { key: 'tab-dupes', onClick: () => setActiveTab('duplicates'), style: { background: 'none', border: 'none', color: activeTab === 'duplicates' ? '#38bdf8' : '#94a3b8', cursor: 'pointer', fontSize: 16, fontWeight: activeTab === 'duplicates' ? 700 : 400 } }, 'Duplicates')
    ]),

    activeTab === 'dashboard' ? renderDashboard() :
    activeTab === 'scanner' ? renderScanner() :
    renderDuplicates()
  ]));
}

createRoot(document.getElementById('root')).render(React.createElement(App));
