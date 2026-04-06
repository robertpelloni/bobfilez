import React, { useEffect, useMemo, useState } from 'https://esm.sh/react@18';
import { createRoot } from 'https://esm.sh/react-dom@18/client';

function cardStyle() {
  return {
    background: 'rgba(15, 23, 42, 0.9)',
    border: '1px solid rgba(148, 163, 184, 0.2)',
    borderRadius: 16,
    padding: 20,
    boxShadow: '0 20px 60px rgba(0,0,0,0.35)'
  };
}

function buttonStyle(active) {
  return {
    background: 'none',
    border: 'none',
    color: active ? '#38bdf8' : '#94a3b8',
    cursor: 'pointer',
    fontSize: 16,
    fontWeight: active ? 700 : 400
  };
}

function primaryButtonStyle(color, busy) {
  return {
    padding: '10px 24px',
    borderRadius: 8,
    background: color,
    color: '#fff',
    border: 'none',
    cursor: busy ? 'wait' : 'pointer'
  };
}

function textInputStyle() {
  return {
    flex: 1,
    padding: '10px 14px',
    borderRadius: 8,
    border: '1px solid #334155',
    background: '#0f172a',
    color: '#fff'
  };
}

function tableShell(children) {
  return React.createElement('div', {
    style: {
      maxHeight: 420,
      overflowY: 'auto',
      background: '#0f172a',
      borderRadius: 8
    }
  }, children);
}

function Card(props) {
  return React.createElement('div', { style: cardStyle() }, [
    React.createElement('h3', { key: 'title', style: { marginTop: 0 } }, props.title),
    props.children
  ]);
}

function StatusBadge(props) {
  return React.createElement('div', {
    style: {
      color: props.color,
      fontWeight: 600,
      background: 'rgba(0,0,0,0.2)',
      padding: '6px 12px',
      borderRadius: 20
    }
  }, props.children);
}

function normalizeScanResults(data) {
  if (!Array.isArray(data)) {
    return [];
  }

  return data.map(function (row) {
    return {
      path: row.path || row.filename || row.uri || '',
      size: row.size || 0
    };
  });
}

function normalizeDuplicateResults(data) {
  if (!Array.isArray(data)) {
    return [];
  }

  return data.map(function (group) {
    var files = Array.isArray(group.files) ? group.files.map(function (file) {
      if (typeof file === 'string') {
        return file;
      }
      return file.path || file.filename || file.uri || '';
    }) : [];

    return {
      hash: group.hash || group.fast64 || '',
      size: group.size || 0,
      files: files
    };
  });
}

function normalizeHashResults(data) {
  if (!Array.isArray(data)) {
    return [];
  }

  return data.map(function (row) {
    return {
      path: row.path || row.filename || row.uri || '',
      hash: row.hash || row.fast64 || ''
    };
  });
}

function normalizeStats(data) {
  if (!data || typeof data !== 'object') {
    return null;
  }

  return {
    totalFiles: data.total_files || 0,
    totalDirectories: data.total_directories || 0,
    totalSize: data.total_size || 0,
    totalSizeHuman: data.total_size_human || '0 B',
    extensions: Array.isArray(data.extensions) ? data.extensions : [],
    sizeDistribution: data.size_distribution || {}
  };
}

function normalizeMetadata(data) {
  if (!Array.isArray(data)) {
    return [];
  }

  return data.map(function (row) {
    return {
      path: row.path || '',
      taken: row.taken || 'n/a',
      gps: row.gps_lat !== undefined && row.gps_lon !== undefined
        ? row.gps_lat + ', ' + row.gps_lon
        : 'n/a'
    };
  });
}

function normalizeLintResults(data) {
  if (!Array.isArray(data)) {
    return [];
  }

  return data.map(function (row) {
    return {
      path: row.path || row.uri || '',
      type: row.type || 'Unknown',
      details: row.details || ''
    };
  });
}

function normalizeHistory(data) {
  if (!Array.isArray(data)) {
    return [];
  }

  return data.map(function (row) {
    return {
      id: row.id || 0,
      type: row.type || 'unknown',
      source: row.source || '',
      dest: row.dest || '',
      timestamp: row.timestamp || '',
      undone: Boolean(row.undone)
    };
  });
}

function normalizeIgnoreRules(data) {
  if (!Array.isArray(data)) {
    return [];
  }

  return data.map(function (row) {
    return {
      id: row.id || 0,
      pattern: row.pattern || '',
      reason: row.reason || ''
    };
  });
}

function fetchJson(url, body) {
  return fetch(url, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(body)
  }).then(function (response) {
    return response.json();
  });
}

function fetchGetJson(url) {
  return fetch(url).then(function (response) {
    return response.json();
  });
}

function App() {
  var _React$useState = useState('dashboard');
  var activeTab = _React$useState[0];
  var setActiveTab = _React$useState[1];

  var _React$useState2 = useState('checking');
  var apiStatus = _React$useState2[0];
  var setApiStatus = _React$useState2[1];

  var _React$useState3 = useState('');
  var scanPath = _React$useState3[0];
  var setScanPath = _React$useState3[1];
  var _React$useState4 = useState([]);
  var scanResults = _React$useState4[0];
  var setScanResults = _React$useState4[1];
  var _React$useState5 = useState(null);
  var scanError = _React$useState5[0];
  var setScanError = _React$useState5[1];
  var _React$useState6 = useState(false);
  var isScanning = _React$useState6[0];
  var setIsScanning = _React$useState6[1];

  var _React$useState7 = useState('');
  var dupePath = _React$useState7[0];
  var setDupePath = _React$useState7[1];
  var _React$useState8 = useState([]);
  var dupeResults = _React$useState8[0];
  var setDupeResults = _React$useState8[1];
  var _React$useState9 = useState(null);
  var dupeError = _React$useState9[0];
  var setDupeError = _React$useState9[1];
  var _React$useState10 = useState(false);
  var isFindingDupes = _React$useState10[0];
  var setIsFindingDupes = _React$useState10[1];

  var _React$useState11 = useState('');
  var statsPath = _React$useState11[0];
  var setStatsPath = _React$useState11[1];
  var _React$useState12 = useState(null);
  var statsResults = _React$useState12[0];
  var setStatsResults = _React$useState12[1];
  var _React$useState13 = useState(null);
  var statsError = _React$useState13[0];
  var setStatsError = _React$useState13[1];
  var _React$useState14 = useState(false);
  var isFindingStats = _React$useState14[0];
  var setIsFindingStats = _React$useState14[1];

  var _React$useState15 = useState('');
  var hashPath = _React$useState15[0];
  var setHashPath = _React$useState15[1];
  var _React$useState16 = useState([]);
  var hashResults = _React$useState16[0];
  var setHashResults = _React$useState16[1];
  var _React$useState17 = useState(null);
  var hashError = _React$useState17[0];
  var setHashError = _React$useState17[1];
  var _React$useState18 = useState(false);
  var isHashing = _React$useState18[0];
  var setIsHashing = _React$useState18[1];

  var _React$useState19 = useState('');
  var metadataPath = _React$useState19[0];
  var setMetadataPath = _React$useState19[1];
  var _React$useState20 = useState([]);
  var metadataResults = _React$useState20[0];
  var setMetadataResults = _React$useState20[1];
  var _React$useState21 = useState(null);
  var metadataError = _React$useState21[0];
  var setMetadataError = _React$useState21[1];
  var _React$useState22 = useState(false);
  var isReadingMetadata = _React$useState22[0];
  var setIsReadingMetadata = _React$useState22[1];

  var _React$useState23 = useState('');
  var lintPath = _React$useState23[0];
  var setLintPath = _React$useState23[1];
  var _React$useState24 = useState([]);
  var lintResults = _React$useState24[0];
  var setLintResults = _React$useState24[1];
  var _React$useState25 = useState(null);
  var lintError = _React$useState25[0];
  var setLintError = _React$useState25[1];
  var _React$useState26 = useState(false);
  var isLinting = _React$useState26[0];
  var setIsLinting = _React$useState26[1];

  var _React$useState27 = useState([]);
  var historyResults = _React$useState27[0];
  var setHistoryResults = _React$useState27[1];
  var _React$useState28 = useState(null);
  var historyError = _React$useState28[0];
  var setHistoryError = _React$useState28[1];
  var _React$useState29 = useState(false);
  var isLoadingHistory = _React$useState29[0];
  var setIsLoadingHistory = _React$useState29[1];

  var _React$useState30 = useState([]);
  var ignoreRules = _React$useState30[0];
  var setIgnoreRules = _React$useState30[1];
  var _React$useState31 = useState(null);
  var ignoreError = _React$useState31[0];
  var setIgnoreError = _React$useState31[1];
  var _React$useState32 = useState(false);
  var isLoadingIgnore = _React$useState32[0];
  var setIsLoadingIgnore = _React$useState32[1];
  var _React$useState33 = useState('');
  var ignorePattern = _React$useState33[0];
  var setIgnorePattern = _React$useState33[1];
  var _React$useState34 = useState('');
  var ignoreReason = _React$useState34[0];
  var setIgnoreReason = _React$useState34[1];
  var _React$useState35 = useState(false);
  var isSavingIgnore = _React$useState35[0];
  var setIsSavingIgnore = _React$useState35[1];

  useEffect(function () {
    fetch('/api/health')
      .then(function (response) {
        setApiStatus(response.ok ? 'online' : 'degraded');
      })
      .catch(function () {
        setApiStatus('offline');
      });
  }, []);

  var statusColor = useMemo(function () {
    if (apiStatus === 'online') {
      return '#34d399';
    }
    if (apiStatus === 'degraded') {
      return '#fbbf24';
    }
    if (apiStatus === 'checking') {
      return '#60a5fa';
    }
    return '#f87171';
  }, [apiStatus]);

  function handleScan(event) {
    event.preventDefault();
    if (!scanPath) {
      return;
    }

    setIsScanning(true);
    setScanError(null);
    setScanResults([]);

    fetchJson('/api/scan', { paths: [scanPath], recursive: true })
      .then(function (data) {
        if (data.error) {
          setScanError(data.error);
          return;
        }
        setScanResults(normalizeScanResults(data));
      })
      .catch(function (error) {
        setScanError(error.message);
      })
      .finally(function () {
        setIsScanning(false);
      });
  }

  function handleDuplicates(event) {
    event.preventDefault();
    if (!dupePath) {
      return;
    }

    setIsFindingDupes(true);
    setDupeError(null);
    setDupeResults([]);

    fetchJson('/api/duplicates', { paths: [dupePath], mode: 'fast', recursive: true })
      .then(function (data) {
        if (data.error) {
          setDupeError(data.error);
          return;
        }
        setDupeResults(normalizeDuplicateResults(data));
      })
      .catch(function (error) {
        setDupeError(error.message);
      })
      .finally(function () {
        setIsFindingDupes(false);
      });
  }

  function handleStats(event) {
    event.preventDefault();
    if (!statsPath) {
      return;
    }

    setIsFindingStats(true);
    setStatsError(null);
    setStatsResults(null);

    fetchJson('/api/stats', { paths: [statsPath] })
      .then(function (data) {
        if (data.error) {
          setStatsError(data.error);
          return;
        }
        setStatsResults(normalizeStats(data));
      })
      .catch(function (error) {
        setStatsError(error.message);
      })
      .finally(function () {
        setIsFindingStats(false);
      });
  }

  function handleHash(event) {
    event.preventDefault();
    if (!hashPath) {
      return;
    }

    setIsHashing(true);
    setHashError(null);
    setHashResults([]);

    fetchJson('/api/hash', { paths: [hashPath], threads: 4 })
      .then(function (data) {
        if (data.error) {
          setHashError(data.error);
          return;
        }
        setHashResults(normalizeHashResults(data));
      })
      .catch(function (error) {
        setHashError(error.message);
      })
      .finally(function () {
        setIsHashing(false);
      });
  }

  function handleMetadata(event) {
    event.preventDefault();
    if (!metadataPath) {
      return;
    }

    setIsReadingMetadata(true);
    setMetadataError(null);
    setMetadataResults([]);

    fetchJson('/api/metadata', { paths: [metadataPath] })
      .then(function (data) {
        if (data.error) {
          setMetadataError(data.error);
          return;
        }
        setMetadataResults(normalizeMetadata(data));
      })
      .catch(function (error) {
        setMetadataError(error.message);
      })
      .finally(function () {
        setIsReadingMetadata(false);
      });
  }

  function handleLint(event) {
    event.preventDefault();
    if (!lintPath) {
      return;
    }

    setIsLinting(true);
    setLintError(null);
    setLintResults([]);

    fetchJson('/api/lint', { paths: [lintPath] })
      .then(function (data) {
        if (data.error) {
          setLintError(data.error);
          return;
        }
        setLintResults(normalizeLintResults(data));
      })
      .catch(function (error) {
        setLintError(error.message);
      })
      .finally(function () {
        setIsLinting(false);
      });
  }

  function loadHistory() {
    setIsLoadingHistory(true);
    setHistoryError(null);

    fetchGetJson('/api/history')
      .then(function (data) {
        if (data.error) {
          setHistoryError(data.error);
          return;
        }
        setHistoryResults(normalizeHistory(data));
      })
      .catch(function (error) {
        setHistoryError(error.message);
      })
      .finally(function () {
        setIsLoadingHistory(false);
      });
  }

  function loadIgnoreRules() {
    setIsLoadingIgnore(true);
    setIgnoreError(null);

    fetchGetJson('/api/ignore')
      .then(function (data) {
        if (data.error) {
          setIgnoreError(data.error);
          return;
        }
        setIgnoreRules(normalizeIgnoreRules(data));
      })
      .catch(function (error) {
        setIgnoreError(error.message);
      })
      .finally(function () {
        setIsLoadingIgnore(false);
      });
  }

  function handleIgnoreAdd(event) {
    event.preventDefault();
    if (!ignorePattern) {
      return;
    }

    setIsSavingIgnore(true);
    setIgnoreError(null);

    fetchJson('/api/ignore/add', { pattern: ignorePattern, reason: ignoreReason })
      .then(function (data) {
        if (data.error) {
          setIgnoreError(data.error);
          return;
        }
        setIgnorePattern('');
        setIgnoreReason('');
        loadIgnoreRules();
      })
      .catch(function (error) {
        setIgnoreError(error.message);
      })
      .finally(function () {
        setIsSavingIgnore(false);
      });
  }

  function handleIgnoreRemove(pattern) {
    setIsSavingIgnore(true);
    setIgnoreError(null);

    fetchJson('/api/ignore/remove', { pattern: pattern })
      .then(function (data) {
        if (data.error) {
          setIgnoreError(data.error);
          return;
        }
        loadIgnoreRules();
      })
      .catch(function (error) {
        setIgnoreError(error.message);
      })
      .finally(function () {
        setIsSavingIgnore(false);
      });
  }

  useEffect(function () {
    if (activeTab === 'history' && historyResults.length === 0 && !isLoadingHistory && !historyError) {
      loadHistory();
    }
  }, [activeTab, historyResults.length, isLoadingHistory, historyError]);

  useEffect(function () {
    if (activeTab === 'ignore' && ignoreRules.length === 0 && !isLoadingIgnore && !ignoreError) {
      loadIgnoreRules();
    }
  }, [activeTab, ignoreRules.length, isLoadingIgnore, ignoreError]);

  function renderDashboard() {
    return React.createElement('div', {
      style: { display: 'grid', gap: 24, gridTemplateColumns: 'repeat(auto-fit, minmax(260px, 1fr))' }
    }, [
      React.createElement(Card, { key: 'qt', title: 'Qt / BobUI' }, React.createElement('p', null, 'Modern native shell path using Qt6 with BobUI Omni wiring and dedicated demo frontends.')),
      React.createElement(Card, { key: 'juce', title: 'JUCE' }, React.createElement('p', null, 'Native lane with scanner, duplicate, statistics, metadata, and lint workflows driven directly from fo_core.')),
      React.createElement(Card, { key: 'btk', title: 'BTK / CopperSpice' }, React.createElement('p', null, 'Research-native lane with a simplified widget demo and safer queued cross-thread result handoff across scan, duplicates, stats, hash, metadata, and lint.')),
      React.createElement(Card, { key: 'web', title: 'React / Express' }, React.createElement('p', null, 'No-build SPA wired to fo_cli-backed JSON endpoints for scanning, duplicates, stats, hashing, metadata, lint, history, and ignore-rule management.'))
    ]);
  }

  function renderScan() {
    return React.createElement(Card, { title: 'File Scanner' }, [
      React.createElement('form', { key: 'form', onSubmit: handleScan, style: { display: 'flex', gap: 12, marginBottom: 20 } }, [
        React.createElement('input', {
          key: 'input',
          type: 'text',
          value: scanPath,
          onChange: function (event) { setScanPath(event.target.value); },
          placeholder: 'Enter absolute directory path (e.g. C:/Users/Public)',
          style: textInputStyle()
        }),
        React.createElement('button', {
          key: 'button',
          type: 'submit',
          disabled: isScanning,
          style: primaryButtonStyle('#3b82f6', isScanning)
        }, isScanning ? 'Scanning...' : 'Scan')
      ]),
      scanError && React.createElement('div', { key: 'error', style: { color: '#f87171' } }, scanError),
      React.createElement('p', { key: 'count' }, 'Found ' + scanResults.length + ' items.'),
      scanResults.length > 0 && tableShell(
        React.createElement('table', { style: { width: '100%', textAlign: 'left', borderCollapse: 'collapse' } }, [
          React.createElement('thead', { key: 'head' }, React.createElement('tr', null, [
            React.createElement('th', { key: 'path', style: { padding: 12, borderBottom: '1px solid #1e293b' } }, 'Path'),
            React.createElement('th', { key: 'size', style: { padding: 12, borderBottom: '1px solid #1e293b' } }, 'Size (bytes)')
          ])),
          React.createElement('tbody', { key: 'body' }, scanResults.slice(0, 100).map(function (row, index) {
            return React.createElement('tr', { key: index }, [
              React.createElement('td', { key: 'path', style: { padding: '8px 12px', borderBottom: '1px solid #1e293b', wordBreak: 'break-all' } }, row.path),
              React.createElement('td', { key: 'size', style: { padding: '8px 12px', borderBottom: '1px solid #1e293b' } }, row.size)
            ]);
          }))
        ])
      ),
      scanResults.length > 100 && React.createElement('p', { key: 'truncated', style: { color: '#94a3b8', fontSize: 13, marginTop: 8 } }, 'Showing first 100 items...')
    ]);
  }

  function renderDuplicates() {
    return React.createElement(Card, { title: 'Duplicate Finder' }, [
      React.createElement('form', { key: 'form', onSubmit: handleDuplicates, style: { display: 'flex', gap: 12, marginBottom: 20 } }, [
        React.createElement('input', {
          key: 'input',
          type: 'text',
          value: dupePath,
          onChange: function (event) { setDupePath(event.target.value); },
          placeholder: 'Enter absolute directory path (e.g. C:/Users/Public)',
          style: textInputStyle()
        }),
        React.createElement('button', {
          key: 'button',
          type: 'submit',
          disabled: isFindingDupes,
          style: primaryButtonStyle('#8b5cf6', isFindingDupes)
        }, isFindingDupes ? 'Searching...' : 'Find Dupes')
      ]),
      dupeError && React.createElement('div', { key: 'error', style: { color: '#f87171' } }, dupeError),
      React.createElement('p', { key: 'count' }, 'Found ' + dupeResults.length + ' duplicate groups.'),
      dupeResults.length > 0 && React.createElement('div', { key: 'groups', style: { display: 'flex', flexDirection: 'column', gap: 16 } },
        dupeResults.slice(0, 50).map(function (group, index) {
          return React.createElement('div', {
            key: index,
            style: { background: '#0f172a', border: '1px solid #1e293b', borderRadius: 8, padding: 16 }
          }, [
            React.createElement('div', {
              key: 'header',
              style: { display: 'flex', justifyContent: 'space-between', color: '#94a3b8', fontSize: 13, marginBottom: 8 }
            }, [
              React.createElement('span', { key: 'hash' }, 'Hash: ' + group.hash),
              React.createElement('span', { key: 'size' }, 'Size: ' + group.size + ' bytes')
            ]),
            React.createElement('ul', { key: 'files', style: { margin: 0, paddingLeft: 20 } },
              group.files.map(function (file, fileIndex) {
                return React.createElement('li', { key: fileIndex, style: { color: '#e2e8f0', wordBreak: 'break-all' } }, file);
              })
            )
          ]);
        })
      ),
      dupeResults.length > 50 && React.createElement('p', { key: 'truncated', style: { color: '#94a3b8', fontSize: 13, marginTop: 8 } }, 'Showing first 50 groups...')
    ]);
  }

  function renderStats() {
    var stats = statsResults;
    return React.createElement(Card, { title: 'Directory Statistics' }, [
      React.createElement('form', { key: 'form', onSubmit: handleStats, style: { display: 'flex', gap: 12, marginBottom: 20 } }, [
        React.createElement('input', {
          key: 'input',
          type: 'text',
          value: statsPath,
          onChange: function (event) { setStatsPath(event.target.value); },
          placeholder: 'Enter absolute directory path (e.g. C:/Users/Public)',
          style: textInputStyle()
        }),
        React.createElement('button', {
          key: 'button',
          type: 'submit',
          disabled: isFindingStats,
          style: primaryButtonStyle('#10b981', isFindingStats)
        }, isFindingStats ? 'Calculating...' : 'Get Stats')
      ]),
      statsError && React.createElement('div', { key: 'error', style: { color: '#f87171' } }, statsError),
      stats && React.createElement('div', { key: 'stats-grid', style: { display: 'grid', gap: 16, gridTemplateColumns: 'repeat(auto-fit, minmax(220px, 1fr))' } }, [
        React.createElement('div', { key: 'files', style: { background: '#0f172a', padding: 16, borderRadius: 8 } }, [
          React.createElement('h4', { key: 'title', style: { margin: '0 0 8px 0', color: '#94a3b8' } }, 'Total Files'),
          React.createElement('div', { key: 'value', style: { fontSize: 24, fontWeight: 'bold' } }, stats.totalFiles)
        ]),
        React.createElement('div', { key: 'dirs', style: { background: '#0f172a', padding: 16, borderRadius: 8 } }, [
          React.createElement('h4', { key: 'title', style: { margin: '0 0 8px 0', color: '#94a3b8' } }, 'Directories'),
          React.createElement('div', { key: 'value', style: { fontSize: 24, fontWeight: 'bold' } }, stats.totalDirectories)
        ]),
        React.createElement('div', { key: 'size', style: { background: '#0f172a', padding: 16, borderRadius: 8 } }, [
          React.createElement('h4', { key: 'title', style: { margin: '0 0 8px 0', color: '#94a3b8' } }, 'Total Size'),
          React.createElement('div', { key: 'value', style: { fontSize: 24, fontWeight: 'bold' } }, stats.totalSizeHuman)
        ]),
        React.createElement('div', { key: 'extensions', style: { background: '#0f172a', padding: 16, borderRadius: 8, gridColumn: '1 / -1' } }, [
          React.createElement('h4', { key: 'title', style: { margin: '0 0 12px 0', color: '#94a3b8' } }, 'Top Extensions'),
          React.createElement('div', { key: 'chips', style: { display: 'flex', flexWrap: 'wrap', gap: 8 } },
            stats.extensions.slice(0, 20).map(function (entry) {
              return React.createElement('div', {
                key: entry.ext,
                style: { background: '#1e293b', padding: '4px 10px', borderRadius: 12, fontSize: 13 }
              }, (entry.ext || '(none)') + ': ' + entry.count);
            })
          )
        ]),
        React.createElement('div', { key: 'distribution', style: { background: '#0f172a', padding: 16, borderRadius: 8, gridColumn: '1 / -1' } }, [
          React.createElement('h4', { key: 'title', style: { margin: '0 0 12px 0', color: '#94a3b8' } }, 'Size Distribution'),
          React.createElement('div', { key: 'rows', style: { display: 'grid', gap: 8, gridTemplateColumns: 'repeat(auto-fit, minmax(160px, 1fr))' } },
            Object.entries(stats.sizeDistribution).map(function (entry) {
              return React.createElement('div', { key: entry[0], style: { background: '#111827', padding: 12, borderRadius: 8 } }, [
                React.createElement('div', { key: 'bucket', style: { color: '#94a3b8', fontSize: 12 } }, entry[0]),
                React.createElement('div', { key: 'count', style: { fontSize: 20, fontWeight: 'bold' } }, entry[1])
              ]);
            })
          )
        ])
      ])
    ]);
  }

  function renderHash() {
    return React.createElement(Card, { title: 'Fast File Hasher' }, [
      React.createElement('form', { key: 'form', onSubmit: handleHash, style: { display: 'flex', gap: 12, marginBottom: 20 } }, [
        React.createElement('input', {
          key: 'input',
          type: 'text',
          value: hashPath,
          onChange: function (event) { setHashPath(event.target.value); },
          placeholder: 'Enter absolute directory path (e.g. C:/Users/Public)',
          style: textInputStyle()
        }),
        React.createElement('button', {
          key: 'button',
          type: 'submit',
          disabled: isHashing,
          style: primaryButtonStyle('#f59e0b', isHashing)
        }, isHashing ? 'Hashing...' : 'Calculate Hash')
      ]),
      hashError && React.createElement('div', { key: 'error', style: { color: '#f87171' } }, hashError),
      React.createElement('p', { key: 'count' }, 'Hashed ' + hashResults.length + ' files.'),
      hashResults.length > 0 && tableShell(
        React.createElement('table', { style: { width: '100%', textAlign: 'left', borderCollapse: 'collapse' } }, [
          React.createElement('thead', { key: 'head' }, React.createElement('tr', null, [
            React.createElement('th', { key: 'path', style: { padding: 12, borderBottom: '1px solid #1e293b' } }, 'Path'),
            React.createElement('th', { key: 'hash', style: { padding: 12, borderBottom: '1px solid #1e293b' } }, 'Hash')
          ])),
          React.createElement('tbody', { key: 'body' }, hashResults.slice(0, 100).map(function (row, index) {
            return React.createElement('tr', { key: index }, [
              React.createElement('td', { key: 'path', style: { padding: '8px 12px', borderBottom: '1px solid #1e293b', wordBreak: 'break-all' } }, row.path),
              React.createElement('td', { key: 'hash', style: { padding: '8px 12px', borderBottom: '1px solid #1e293b', fontFamily: 'monospace', color: '#fbbf24' } }, row.hash)
            ]);
          }))
        ])
      ),
      hashResults.length > 100 && React.createElement('p', { key: 'truncated', style: { color: '#94a3b8', fontSize: 13, marginTop: 8 } }, 'Showing first 100 results...')
    ]);
  }

  function renderMetadata() {
    return React.createElement(Card, { title: 'Image Metadata' }, [
      React.createElement('form', { key: 'form', onSubmit: handleMetadata, style: { display: 'flex', gap: 12, marginBottom: 20 } }, [
        React.createElement('input', {
          key: 'input',
          type: 'text',
          value: metadataPath,
          onChange: function (event) { setMetadataPath(event.target.value); },
          placeholder: 'Enter directory path containing images',
          style: textInputStyle()
        }),
        React.createElement('button', {
          key: 'button',
          type: 'submit',
          disabled: isReadingMetadata,
          style: primaryButtonStyle('#ec4899', isReadingMetadata)
        }, isReadingMetadata ? 'Reading...' : 'Load Metadata')
      ]),
      metadataError && React.createElement('div', { key: 'error', style: { color: '#f87171' } }, metadataError),
      React.createElement('p', { key: 'count' }, 'Loaded ' + metadataResults.length + ' metadata records.'),
      metadataResults.length > 0 && tableShell(
        React.createElement('table', { style: { width: '100%', textAlign: 'left', borderCollapse: 'collapse' } }, [
          React.createElement('thead', { key: 'head' }, React.createElement('tr', null, [
            React.createElement('th', { key: 'path', style: { padding: 12, borderBottom: '1px solid #1e293b' } }, 'Path'),
            React.createElement('th', { key: 'taken', style: { padding: 12, borderBottom: '1px solid #1e293b' } }, 'Taken'),
            React.createElement('th', { key: 'gps', style: { padding: 12, borderBottom: '1px solid #1e293b' } }, 'GPS')
          ])),
          React.createElement('tbody', { key: 'body' }, metadataResults.slice(0, 100).map(function (row, index) {
            return React.createElement('tr', { key: index }, [
              React.createElement('td', { key: 'path', style: { padding: '8px 12px', borderBottom: '1px solid #1e293b', wordBreak: 'break-all' } }, row.path),
              React.createElement('td', { key: 'taken', style: { padding: '8px 12px', borderBottom: '1px solid #1e293b' } }, row.taken),
              React.createElement('td', { key: 'gps', style: { padding: '8px 12px', borderBottom: '1px solid #1e293b' } }, row.gps)
            ]);
          }))
        ])
      )
    ]);
  }

  function renderLint() {
    return React.createElement(Card, { title: 'Filesystem Lint' }, [
      React.createElement('form', { key: 'form', onSubmit: handleLint, style: { display: 'flex', gap: 12, marginBottom: 20 } }, [
        React.createElement('input', {
          key: 'input',
          type: 'text',
          value: lintPath,
          onChange: function (event) { setLintPath(event.target.value); },
          placeholder: 'Enter absolute directory path to lint',
          style: textInputStyle()
        }),
        React.createElement('button', {
          key: 'button',
          type: 'submit',
          disabled: isLinting,
          style: primaryButtonStyle('#ef4444', isLinting)
        }, isLinting ? 'Linting...' : 'Run Lint')
      ]),
      lintError && React.createElement('div', { key: 'error', style: { color: '#f87171' } }, lintError),
      React.createElement('p', { key: 'count' }, 'Found ' + lintResults.length + ' lint issues.'),
      lintResults.length > 0 && tableShell(
        React.createElement('table', { style: { width: '100%', textAlign: 'left', borderCollapse: 'collapse' } }, [
          React.createElement('thead', { key: 'head' }, React.createElement('tr', null, [
            React.createElement('th', { key: 'path', style: { padding: 12, borderBottom: '1px solid #1e293b' } }, 'Path'),
            React.createElement('th', { key: 'type', style: { padding: 12, borderBottom: '1px solid #1e293b' } }, 'Type'),
            React.createElement('th', { key: 'details', style: { padding: 12, borderBottom: '1px solid #1e293b' } }, 'Details')
          ])),
          React.createElement('tbody', { key: 'body' }, lintResults.slice(0, 100).map(function (row, index) {
            return React.createElement('tr', { key: index }, [
              React.createElement('td', { key: 'path', style: { padding: '8px 12px', borderBottom: '1px solid #1e293b', wordBreak: 'break-all' } }, row.path),
              React.createElement('td', { key: 'type', style: { padding: '8px 12px', borderBottom: '1px solid #1e293b', color: '#fca5a5', fontWeight: 'bold' } }, row.type),
              React.createElement('td', { key: 'details', style: { padding: '8px 12px', borderBottom: '1px solid #1e293b' } }, row.details)
            ]);
          }))
        ])
      ),
      lintResults.length > 100 && React.createElement('p', { key: 'truncated', style: { color: '#94a3b8', fontSize: 13, marginTop: 8 } }, 'Showing first 100 lint issues...')
    ]);
  }

  function renderHistory() {
    return React.createElement(Card, { title: 'Operation History' }, [
      React.createElement('div', { key: 'actions', style: { display: 'flex', gap: 12, marginBottom: 20 } }, [
        React.createElement('button', {
          key: 'button',
          type: 'button',
          disabled: isLoadingHistory,
          onClick: loadHistory,
          style: primaryButtonStyle('#6366f1', isLoadingHistory)
        }, isLoadingHistory ? 'Loading...' : 'Refresh History')
      ]),
      historyError && React.createElement('div', { key: 'error', style: { color: '#f87171' } }, historyError),
      React.createElement('p', { key: 'count' }, 'Loaded ' + historyResults.length + ' operations.'),
      historyResults.length > 0 && tableShell(
        React.createElement('table', { style: { width: '100%', textAlign: 'left', borderCollapse: 'collapse' } }, [
          React.createElement('thead', { key: 'head' }, React.createElement('tr', null, [
            React.createElement('th', { key: 'time', style: { padding: 12, borderBottom: '1px solid #1e293b' } }, 'Timestamp'),
            React.createElement('th', { key: 'type', style: { padding: 12, borderBottom: '1px solid #1e293b' } }, 'Type'),
            React.createElement('th', { key: 'source', style: { padding: 12, borderBottom: '1px solid #1e293b' } }, 'Source'),
            React.createElement('th', { key: 'dest', style: { padding: 12, borderBottom: '1px solid #1e293b' } }, 'Destination')
          ])),
          React.createElement('tbody', { key: 'body' }, historyResults.slice(0, 100).map(function (row, index) {
            return React.createElement('tr', { key: row.id || index }, [
              React.createElement('td', { key: 'time', style: { padding: '8px 12px', borderBottom: '1px solid #1e293b' } }, row.timestamp + (row.undone ? ' (undone)' : '')),
              React.createElement('td', { key: 'type', style: { padding: '8px 12px', borderBottom: '1px solid #1e293b', textTransform: 'uppercase', color: '#a5b4fc' } }, row.type),
              React.createElement('td', { key: 'source', style: { padding: '8px 12px', borderBottom: '1px solid #1e293b', wordBreak: 'break-all' } }, row.source),
              React.createElement('td', { key: 'dest', style: { padding: '8px 12px', borderBottom: '1px solid #1e293b', wordBreak: 'break-all' } }, row.dest || '—')
            ]);
          }))
        ])
      )
    ]);
  }

  function renderIgnoreRules() {
    return React.createElement(Card, { title: 'Ignore Rules' }, [
      React.createElement('form', { key: 'form', onSubmit: handleIgnoreAdd, style: { display: 'grid', gap: 12, gridTemplateColumns: '2fr 2fr auto', marginBottom: 20 } }, [
        React.createElement('input', {
          key: 'pattern',
          type: 'text',
          value: ignorePattern,
          onChange: function (event) { setIgnorePattern(event.target.value); },
          placeholder: 'Pattern (e.g. thumbs.db)',
          style: textInputStyle()
        }),
        React.createElement('input', {
          key: 'reason',
          type: 'text',
          value: ignoreReason,
          onChange: function (event) { setIgnoreReason(event.target.value); },
          placeholder: 'Reason (optional)',
          style: textInputStyle()
        }),
        React.createElement('button', {
          key: 'add',
          type: 'submit',
          disabled: isSavingIgnore,
          style: primaryButtonStyle('#14b8a6', isSavingIgnore)
        }, isSavingIgnore ? 'Saving...' : 'Add Rule')
      ]),
      React.createElement('div', { key: 'toolbar', style: { display: 'flex', gap: 12, marginBottom: 20 } }, [
        React.createElement('button', {
          key: 'refresh',
          type: 'button',
          disabled: isLoadingIgnore,
          onClick: loadIgnoreRules,
          style: primaryButtonStyle('#0ea5e9', isLoadingIgnore)
        }, isLoadingIgnore ? 'Refreshing...' : 'Refresh Rules')
      ]),
      ignoreError && React.createElement('div', { key: 'error', style: { color: '#f87171' } }, ignoreError),
      React.createElement('p', { key: 'count' }, 'Loaded ' + ignoreRules.length + ' ignore rules.'),
      ignoreRules.length > 0 && tableShell(
        React.createElement('table', { style: { width: '100%', textAlign: 'left', borderCollapse: 'collapse' } }, [
          React.createElement('thead', { key: 'head' }, React.createElement('tr', null, [
            React.createElement('th', { key: 'pattern', style: { padding: 12, borderBottom: '1px solid #1e293b' } }, 'Pattern'),
            React.createElement('th', { key: 'reason', style: { padding: 12, borderBottom: '1px solid #1e293b' } }, 'Reason'),
            React.createElement('th', { key: 'action', style: { padding: 12, borderBottom: '1px solid #1e293b' } }, 'Action')
          ])),
          React.createElement('tbody', { key: 'body' }, ignoreRules.map(function (row, index) {
            return React.createElement('tr', { key: row.id || index }, [
              React.createElement('td', { key: 'pattern', style: { padding: '8px 12px', borderBottom: '1px solid #1e293b', fontFamily: 'monospace' } }, row.pattern),
              React.createElement('td', { key: 'reason', style: { padding: '8px 12px', borderBottom: '1px solid #1e293b' } }, row.reason || '—'),
              React.createElement('td', { key: 'action', style: { padding: '8px 12px', borderBottom: '1px solid #1e293b' } },
                React.createElement('button', {
                  type: 'button',
                  disabled: isSavingIgnore,
                  onClick: function () { handleIgnoreRemove(row.pattern); },
                  style: primaryButtonStyle('#f97316', isSavingIgnore)
                }, isSavingIgnore ? 'Working...' : 'Remove')
              )
            ]);
          }))
        ])
      )
    ]);
  }

  var content = renderDashboard();
  if (activeTab === 'scanner') {
    content = renderScan();
  } else if (activeTab === 'duplicates') {
    content = renderDuplicates();
  } else if (activeTab === 'stats') {
    content = renderStats();
  } else if (activeTab === 'hash') {
    content = renderHash();
  } else if (activeTab === 'metadata') {
    content = renderMetadata();
  } else if (activeTab === 'lint') {
    content = renderLint();
  } else if (activeTab === 'history') {
    content = renderHistory();
  } else if (activeTab === 'ignore') {
    content = renderIgnoreRules();
  }

  return React.createElement('div', {
    style: {
      minHeight: '100vh',
      padding: 32,
      background: 'radial-gradient(circle at top, #16213a 0%, #0b1220 60%)',
      color: '#e2e8f0'
    }
  }, React.createElement('div', {
    style: { maxWidth: 1240, margin: '0 auto', display: 'grid', gap: 24 }
  }, [
    React.createElement('div', { key: 'header', style: { display: 'flex', justifyContent: 'space-between', alignItems: 'center' } }, [
      React.createElement('h1', { key: 'title', style: { margin: 0, fontSize: 28, color: '#f8fafc' } }, 'bobfilez React Web UI'),
      React.createElement(StatusBadge, { key: 'status', color: statusColor }, 'API: ' + apiStatus)
    ]),
    React.createElement('div', { key: 'nav', style: { display: 'flex', gap: 16, borderBottom: '1px solid rgba(148, 163, 184, 0.2)', paddingBottom: 16, flexWrap: 'wrap' } }, [
      React.createElement('button', { key: 'dashboard', onClick: function () { setActiveTab('dashboard'); }, style: buttonStyle(activeTab === 'dashboard') }, 'Dashboard'),
      React.createElement('button', { key: 'scanner', onClick: function () { setActiveTab('scanner'); }, style: buttonStyle(activeTab === 'scanner') }, 'Scanner'),
      React.createElement('button', { key: 'duplicates', onClick: function () { setActiveTab('duplicates'); }, style: buttonStyle(activeTab === 'duplicates') }, 'Duplicates'),
      React.createElement('button', { key: 'stats', onClick: function () { setActiveTab('stats'); }, style: buttonStyle(activeTab === 'stats') }, 'Statistics'),
      React.createElement('button', { key: 'hash', onClick: function () { setActiveTab('hash'); }, style: buttonStyle(activeTab === 'hash') }, 'Hasher'),
      React.createElement('button', { key: 'metadata', onClick: function () { setActiveTab('metadata'); }, style: buttonStyle(activeTab === 'metadata') }, 'Metadata'),
      React.createElement('button', { key: 'lint', onClick: function () { setActiveTab('lint'); }, style: buttonStyle(activeTab === 'lint') }, 'Lint'),
      React.createElement('button', { key: 'history', onClick: function () { setActiveTab('history'); }, style: buttonStyle(activeTab === 'history') }, 'History'),
      React.createElement('button', { key: 'ignore', onClick: function () { setActiveTab('ignore'); }, style: buttonStyle(activeTab === 'ignore') }, 'Ignore Rules')
    ]),
    content
  ]));
}

createRoot(document.getElementById('root')).render(React.createElement(App));
