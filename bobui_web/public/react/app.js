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
  const [apiStatus, setApiStatus] = useState('checking');
  const [version] = useState('6.x multi-frontend preview');

  useEffect(() => {
    fetch('/api/health')
      .then((res) => setApiStatus(res.ok ? 'online' : 'degraded'))
      .catch(() => setApiStatus('offline'));
  }, []);

  const statusColor = useMemo(() => {
    if (apiStatus === 'online') return '#34d399';
    if (apiStatus === 'degraded') return '#fbbf24';
    if (apiStatus === 'checking') return '#60a5fa';
    return '#f87171';
  }, [apiStatus]);

  return React.createElement('div', {
    style: {
      minHeight: '100vh',
      padding: 32,
      background: 'radial-gradient(circle at top, #16213a 0%, #0b1220 60%)'
    }
  }, React.createElement('div', {
    style: {
      maxWidth: 1200,
      margin: '0 auto',
      display: 'grid',
      gap: 24
    }
  }, [
    React.createElement(Card, { key: 'hero', title: 'bobfilez React Web UI' }, React.createElement('div', { style: { display: 'grid', gap: 12 } }, [
      React.createElement('p', { key: 'intro' }, 'A lightweight React shell running directly from the existing bobui_web server. This lane sits alongside the Qt, BobUI, JUCE, BTK, and BobGUI frontends.'),
      React.createElement('p', { key: 'version' }, 'Frontend matrix: ' + version),
      React.createElement('p', { key: 'status', style: { color: statusColor, fontWeight: 700 } }, 'API status: ' + apiStatus)
    ])),
    React.createElement('div', {
      key: 'grid',
      style: { display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(260px, 1fr))', gap: 24 }
    }, [
      React.createElement(Card, { key: 'qt', title: 'Qt / BobUI' }, React.createElement('p', null, 'Modern native shell path using Qt6 with BobUI Omni wiring.')),
      React.createElement(Card, { key: 'juce', title: 'JUCE' }, React.createElement('p', null, 'Audio/media-oriented native lane with a dedicated JUCE demo target.')),
      React.createElement(Card, { key: 'btk', title: 'BTK / CopperSpice' }, React.createElement('p', null, 'Separate native research lane with a lightweight widget demo target.')),
      React.createElement(Card, { key: 'bobgui', title: 'BobGUI' }, React.createElement('p', null, 'GTK-style Meson lane with a demo application scaffold.'))
    ])
  ]));
}

createRoot(document.getElementById('root')).render(React.createElement(App));
