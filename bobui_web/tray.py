"""bobfilez dashboard system-tray icon (started 2026-11-03).

Lives in the Windows notification area while the Express dashboard runs.
Menu: Open Unified Dashboard | Open legacy React UI | Server status |
Restart server | Open logs folder | Quit server + tray.
Falls back to a tkinter window if pystray/Pillow are unavailable.
"""
import os, sys, json, subprocess, threading, time, urllib.request, webbrowser

ROOT = os.path.dirname(os.path.abspath(__file__))
PORT = int(os.environ.get("PORT", "3131"))
BASE = f"http://localhost:{PORT}"
LOGDIR = os.path.join(os.path.dirname(ROOT), "logs")

def api(path, body=None, timeout=200):
    data = json.dumps(body or {}).encode() if body is not None else None
    req = urllib.request.Request(BASE + path, data=data,
        headers={"Content-Type": "application/json"},
        method="POST" if body is not None else "GET")
    with urllib.request.urlopen(req, timeout=timeout) as r:
        return r.read().decode("utf-8", "replace")

def health():
    try:
        return json.loads(api("/health"))
    except Exception as e:
        return {"ok": False, "error": str(e)}

server_proc = None

def start_server():
    global server_proc
    if server_proc and server_proc.poll() is None:
        return "already running"
    cmd = [sys.executable if False else "node", "server.js"]
    server_proc = subprocess.Popen(cmd, cwd=ROOT,
        stdout=open(os.path.join(LOGDIR, "tray-server.log"), "a"),
        stderr=subprocess.STDOUT)
    return f"started pid={server_proc.pid}"

def stop_server():
    global server_proc
    try:
        api("/quit", {}, timeout=5)
    except Exception:
        pass
    if server_proc and server_proc.poll() is None:
        time.sleep(1)
        try:
            server_proc.terminate()
        except Exception:
            pass
    return "stopped"

def open_logs():
    os.startfile(LOGDIR) if hasattr(os, "startfile") else webbrowser.open(LOGDIR)

def run_tk():
    import tkinter as tk
    from tkinter import messagebox
    w = tk.Tk(); w.title("bobfilez tray (fallback)"); w.geometry("320x260")
    st = tk.StringVar(value="status: unknown")
    def refresh():
        h = health()
        st.set("status: " + ("ONLINE " + str(h.get("cli", "")) if h.get("ok") else "OFFLINE " + str(h.get("error", ""))[:80]))
    tk.Label(w, text="bobfilez dashboard").pack(pady=6)
    tk.Label(w, textvariable=st, wraplength=300).pack(pady=6)
    for label, fn in [
        ("Open unified dashboard", lambda: webbrowser.open(BASE + "/unified.html")),
        ("Open legacy React UI", lambda: webbrowser.open(BASE + "/react/")),
        ("Refresh status", refresh),
        ("Restart server", lambda: (stop_server(), start_server(), refresh())),
        ("Quit server + tray", lambda: (stop_server(), w.destroy())),
    ]:
        tk.Button(w, text=label, width=32, command=fn).pack(pady=2)
    refresh(); w.mainloop()

def run_pystray():
    import pystray
    from PIL import Image, ImageDraw
    img = Image.new("RGB", (64, 64), (17, 28, 51))
    dr = ImageDraw.Draw(img)
    dr.rectangle([14, 20, 50, 52], fill=(56, 189, 248))
    dr.rectangle([22, 8, 42, 20], fill=(52, 211, 153))
    def item_open(icon, _): webbrowser.open(BASE + "/unified.html")
    def item_react(icon, _): webbrowser.open(BASE + "/react/")
    def item_status(icon, _):
        h = health()
        icon.notify("ONLINE: " + str(h.get("cli", "?")) if h.get("ok") else "OFFLINE: " + str(h.get("error", "?"))[:120])
    def item_restart(icon, _): stop_server(); start_server(); icon.notify("server restarted")
    def item_logs(icon, _): open_logs()
    def item_quit(icon, _): stop_server(); icon.stop()
    menu = pystray.Menu(
        pystray.MenuItem("Open unified dashboard", item_open, default=True),
        pystray.MenuItem("Open legacy React UI", item_react),
        pystray.MenuItem("Server status", item_status),
        pystray.MenuItem("Restart server", item_restart),
        pystray.MenuItem("Open logs folder", item_logs),
        pystray.MenuItem("Quit server + tray", item_quit),
    )
    icon = pystray.Icon("bobfilez", img, "bobfilez dashboard", menu)
    threading.Thread(target=lambda: (time.sleep(2), start_server()), daemon=True).start()
    icon.run()

if __name__ == "__main__":
    os.makedirs(LOGDIR, exist_ok=True)
    try:
        import pystray  # noqa
        run_pystray()
    except Exception as e:
        print(f"[tray] pystray unavailable ({e}); tkinter fallback", flush=True)
        start_server()
        run_tk()