async function loadFiles() {
    try {
        const res = await fetch('/api/scan', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ paths: ['.'] })
        });
        const files = await res.json();
        const body = document.getElementById('file-body');
        body.innerHTML = '';
        
        files.forEach(f => {
            const tr = document.createElement('tr');
            tr.innerHTML = `
                <td>${f.path.split('/').pop()}</td>
                <td>${(f.size / 1024).toFixed(1)} KB</td>
                <td>${new Date(f.mtime * 1000).toLocaleDateString()}</td>
            `;
            body.appendChild(tr);
        });
    } catch (e) {
        console.error("Failed to load files", e);
    }
}

function updateTime() {
    const now = new Date();
    document.getElementById('tray-clock').innerText = now.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' });
}

// Initial load
loadFiles();
setInterval(updateTime, 1000);
updateTime();
