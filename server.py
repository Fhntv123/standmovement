"""
yaw_cc build server
Runs on http://127.0.0.1:8770
Click "BUILD" -> executes build.bat -> streams live output line-by-line.

No external deps. Pure stdlib.
"""
import http.server
import socketserver
import subprocess
import threading
import queue
import json
import os
import sys
import webbrowser
import time
from urllib.parse import urlparse

REPO_DIR = r"C:\Users\НИЗКИЙ\source\repos\ImGui-DirectX-11-Kiero-Hook-master"
BAT_PATH = os.path.join(REPO_DIR, "build.bat")
PORT = 8770

# Global build state
_build_lock = threading.Lock()
_current_queue = None  # queue.Queue that receives dict events during a build
_build_thread = None


def stream_build(q: "queue.Queue"):
    """Run build.bat, push each output line into q as {type,line} dicts.
    GUARANTEE: always emits {'type':'done',...} exactly once at the end."""
    done_sent = {"v": False}
    def emit_done(code):
        if not done_sent["v"]:
            q.put({"type": "done", "code": code})
            done_sent["v"] = True

    try:
        q.put({"type": "status", "line": "starting build..."})
        if not os.path.exists(BAT_PATH):
            q.put({"type": "error", "line": f"build.bat not found at {BAT_PATH}"})
            emit_done(-1)
            return

        q.put({"type": "status", "line": f"cwd = {REPO_DIR}"})
        q.put({"type": "status", "line": f"running {BAT_PATH}"})
        q.put({"type": "status", "line": "-" * 60})

        # cmd /c to run .bat and inherit exit code; no new console window.
        try:
            proc = subprocess.Popen(
                ["cmd.exe", "/c", BAT_PATH],
                cwd=REPO_DIR,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                bufsize=1,
                universal_newlines=False,  # read raw bytes, decode ourselves
                creationflags=subprocess.CREATE_NO_WINDOW if hasattr(subprocess, "CREATE_NO_WINDOW") else 0,
            )
        except Exception as e:
            q.put({"type": "error", "line": f"failed to start: {e}"})
            emit_done(-1)
            return

        # Read stdout byte-by-byte with os.read to defeat any buffering in child.
        # MSBuild + cmd like to buffer huge chunks when stdout isn't a TTY.
        import os as _os
        _fd = proc.stdout.fileno()
        _partial = bytearray()

        def _iter_lines():
            nonlocal _partial
            while True:
                try:
                    chunk = _os.read(_fd, 4096)
                except (OSError, ValueError):
                    chunk = b""
                if not chunk:
                    # flush any remaining partial line
                    if _partial:
                        yield bytes(_partial)
                        _partial = bytearray()
                    return
                _partial.extend(chunk)
                while True:
                    nl = _partial.find(b"\n")
                    if nl < 0:
                        break
                    line_bytes = bytes(_partial[:nl])
                    del _partial[:nl+1]
                    yield line_bytes

        for raw in _iter_lines():
            if False:
                break
            # build.bat now runs with chcp 65001 (UTF-8). Try UTF-8 first, fall back to cp866.
            try:
                line = raw.decode("utf-8").rstrip("\r\n")
            except UnicodeDecodeError:
                try:
                    line = raw.decode("cp866", errors="replace").rstrip("\r\n")
                except Exception:
                    line = repr(raw)
            # Skip empty pause prompts
            low = line.lower().strip()
            if low.startswith("press any key") or low.startswith("нажмите"):
                # build.bat has pause at end — send Enter automatically
                try:
                    proc.stdin and proc.stdin.write(b"\r\n") and proc.stdin.flush()
                except Exception:
                    pass
                continue
            q.put({"type": "log", "line": line})

        proc.stdout.close()
        code = proc.wait()
        q.put({"type": "status", "line": "-" * 60})
        if code == 0:
            q.put({"type": "success", "line": f"build succeeded (exit {code})"})
        else:
            q.put({"type": "error", "line": f"build failed (exit {code})"})
        emit_done(code)
    except Exception as e:
        q.put({"type": "error", "line": f"internal error: {e}"})
        emit_done(-2)
    finally:
        # Safety net — should already be sent, but guarantee it.
        emit_done(-3)


def start_build(force=False):
    """Kick off a build in a background thread. Returns the queue to stream from.
    ALWAYS starts a fresh build — no "already running" refusals. If a previous
    build thread is somehow still alive, we let it die on its own; new queue
    replaces the old one so old thread's messages just get dropped."""
    global _current_queue, _build_thread
    with _build_lock:
        _current_queue = queue.Queue()
        _build_thread = threading.Thread(target=stream_build, args=(_current_queue,), daemon=True)
        _build_thread.start()
        return _current_queue


HTML = r"""<!DOCTYPE html>
<html lang="ru">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>yaw_cc // build</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
<link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700;800&family=JetBrains+Mono:wght@400;500;600&display=swap" rel="stylesheet">
<style>
  :root{
    --bg:#0a0710;
    --bg2:#120a1e;
    --panel:#150c26;
    --panel2:#1c1030;
    --border:#2a1747;
    --border-glow:#4a2597;
    --purple:#a855f7;
    --purple-bright:#c084fc;
    --purple-glow:#7c3aed;
    --purple-dim:#6b21a8;
    --white:#f5f0ff;
    --dim:#8b7ba8;
    --muted:#5c4a75;
    --green:#4ade80;
    --red:#f87171;
    --yellow:#facc15;
  }

  *{box-sizing:border-box;}
  html,body{margin:0;padding:0;height:100%;background:var(--bg);color:var(--white);
    font:14px/1.55 'JetBrains Mono',ui-monospace,Consolas,monospace;
    -webkit-font-smoothing:antialiased;
    text-rendering:optimizeLegibility;
    overflow:hidden;
  }

  /* Animated ambient glow */
  body::before{
    content:'';position:fixed;inset:-50%;pointer-events:none;z-index:0;
    background:
      radial-gradient(600px circle at 20% 20%, rgba(124,58,237,0.15), transparent 50%),
      radial-gradient(500px circle at 80% 70%, rgba(168,85,247,0.10), transparent 50%),
      radial-gradient(400px circle at 50% 100%, rgba(192,132,252,0.08), transparent 50%);
    animation:drift 20s ease-in-out infinite alternate;
  }
  @keyframes drift{
    0%{transform:translate(0,0) rotate(0deg);}
    100%{transform:translate(-30px,30px) rotate(3deg);}
  }

  header{
    position:relative;z-index:2;
    padding:18px 28px;
    background:linear-gradient(180deg, rgba(28,16,48,0.85) 0%, rgba(21,12,38,0.7) 100%);
    backdrop-filter:blur(20px);-webkit-backdrop-filter:blur(20px);
    border-bottom:1px solid var(--border);
    display:flex;align-items:center;gap:18px;
    box-shadow:0 4px 30px rgba(124,58,237,0.15);
  }
  header::after{
    content:'';position:absolute;left:0;right:0;bottom:-1px;height:1px;
    background:linear-gradient(90deg,transparent,var(--purple),transparent);
    opacity:0.6;
  }

  .brand{
    display:flex;align-items:center;gap:12px;
    font-family:'Inter',system-ui,sans-serif;
  }
  .brand .dot{
    width:10px;height:10px;border-radius:50%;
    background:var(--purple);
    box-shadow:0 0 12px var(--purple),0 0 24px rgba(168,85,247,0.5);
    animation:pulse 2s ease-in-out infinite;
  }
  @keyframes pulse{
    0%,100%{opacity:1;transform:scale(1);}
    50%{opacity:0.6;transform:scale(0.85);}
  }
  .brand h1{
    margin:0;font-size:16px;font-weight:700;letter-spacing:0.5px;
    background:linear-gradient(135deg,#fff 0%, var(--purple-bright) 100%);
    -webkit-background-clip:text;background-clip:text;color:transparent;
  }
  .brand h1 span{color:var(--purple);font-weight:500;margin:0 4px;}
  .brand .sub{color:var(--muted);font-size:11px;letter-spacing:1px;text-transform:uppercase;margin-top:2px;font-weight:500;}

  .actions{margin-left:auto;display:flex;align-items:center;gap:12px;}

  button{
    font-family:'Inter',system-ui,sans-serif;
    font-weight:600;letter-spacing:0.8px;font-size:12px;text-transform:uppercase;
    border:0;cursor:pointer;
    padding:12px 24px;border-radius:8px;
    transition:all 0.2s cubic-bezier(.4,0,.2,1);
    position:relative;overflow:hidden;
  }
  button.primary{
    background:linear-gradient(135deg,var(--purple-glow) 0%,var(--purple) 50%,var(--purple-bright) 100%);
    color:#fff;
    box-shadow:0 4px 20px rgba(124,58,237,0.4), inset 0 1px 0 rgba(255,255,255,0.2);
  }
  button.primary:hover:not(:disabled){
    transform:translateY(-1px);
    box-shadow:0 6px 28px rgba(168,85,247,0.6), inset 0 1px 0 rgba(255,255,255,0.3);
  }
  button.primary:active:not(:disabled){transform:translateY(0);}
  button.primary:disabled{
    background:linear-gradient(135deg,#2a1747 0%,#3a1f5c 100%);
    color:var(--muted);cursor:not-allowed;box-shadow:none;
  }
  button.primary::before{
    content:'';position:absolute;inset:0;
    background:linear-gradient(90deg,transparent,rgba(255,255,255,0.3),transparent);
    transform:translateX(-100%);transition:transform 0.6s;
  }
  button.primary:hover:not(:disabled)::before{transform:translateX(100%);}

  button.ghost{
    background:transparent;color:var(--dim);
    border:1px solid var(--border);
  }
  button.ghost:hover{background:var(--panel);color:var(--white);border-color:var(--purple);}

  .status{
    font-family:'Inter',system-ui,sans-serif;font-size:12px;font-weight:500;letter-spacing:0.5px;
    padding:6px 14px;border-radius:20px;
    background:rgba(139,123,168,0.1);color:var(--dim);
    border:1px solid transparent;
    transition:all 0.3s;
    text-transform:lowercase;
  }
  .status.running{
    background:rgba(168,85,247,0.15);color:var(--purple-bright);
    border-color:rgba(168,85,247,0.4);
    box-shadow:0 0 20px rgba(168,85,247,0.3);
    animation:pulseStatus 1.5s ease-in-out infinite;
  }
  @keyframes pulseStatus{
    0%,100%{box-shadow:0 0 20px rgba(168,85,247,0.3);}
    50%{box-shadow:0 0 28px rgba(168,85,247,0.5);}
  }
  .status.ok{background:rgba(74,222,128,0.15);color:var(--green);border-color:rgba(74,222,128,0.3);}
  .status.err{background:rgba(248,113,113,0.15);color:var(--red);border-color:rgba(248,113,113,0.3);}

  main{
    position:relative;z-index:1;
    height:calc(100vh - 68px);
    padding:20px 28px 24px;
  }

  #logs-wrap{
    height:100%;
    background:linear-gradient(180deg, rgba(21,12,38,0.7) 0%, rgba(10,7,16,0.7) 100%);
    border:1px solid var(--border);
    border-radius:12px;
    overflow:hidden;
    position:relative;
    box-shadow:0 10px 40px rgba(0,0,0,0.5), inset 0 1px 0 rgba(168,85,247,0.1);
  }
  #logs-wrap::before{
    content:'';position:absolute;top:0;left:0;right:0;height:2px;
    background:linear-gradient(90deg,transparent 0%,var(--purple) 50%,transparent 100%);
    opacity:0.5;
  }

  #logs{
    height:100%;padding:20px 26px;overflow-y:auto;
    scroll-behavior:smooth;
    font-family:'JetBrains Mono',monospace;font-size:13px;
  }

  .line{
    padding:2px 0;
    white-space:pre-wrap;word-break:break-word;
    animation:lineIn 0.35s cubic-bezier(.16,1,.3,1) both;
    position:relative;
    padding-left:14px;
  }
  @keyframes lineIn{
    from{opacity:0;transform:translateX(-8px);}
    to{opacity:1;transform:translateX(0);}
  }
  .line::before{
    content:'';position:absolute;left:0;top:11px;
    width:4px;height:4px;border-radius:50%;background:var(--muted);
    transition:all 0.3s;
  }
  .line.log{color:#d8d0e8;}
  .line.log::before{background:var(--purple-dim);}
  .line.status{color:var(--purple-bright);opacity:0.9;}
  .line.status::before{background:var(--purple);box-shadow:0 0 8px var(--purple);}
  .line.success{color:var(--green);font-weight:600;font-size:14px;}
  .line.success::before{background:var(--green);box-shadow:0 0 10px var(--green);}
  .line.error{color:var(--red);font-weight:600;}
  .line.error::before{background:var(--red);box-shadow:0 0 10px var(--red);}
  .line.log.err{color:#ff9b9b;}
  .line.log.warn{color:var(--yellow);}

  .empty{
    color:var(--muted);text-align:center;padding:80px 20px;
    font-family:'Inter',system-ui,sans-serif;font-size:13px;letter-spacing:1px;
    text-transform:uppercase;font-weight:500;
    animation:fadeIn 0.5s ease-out;
  }
  .empty .icon{
    display:block;font-size:48px;margin-bottom:16px;
    background:linear-gradient(135deg,var(--purple) 0%,var(--purple-bright) 100%);
    -webkit-background-clip:text;background-clip:text;color:transparent;
    opacity:0.5;
  }
  @keyframes fadeIn{from{opacity:0;}to{opacity:1;}}

  /* Scrollbar */
  ::-webkit-scrollbar{width:10px;height:10px;}
  ::-webkit-scrollbar-track{background:transparent;}
  ::-webkit-scrollbar-thumb{
    background:linear-gradient(180deg,var(--purple-dim),var(--purple-glow));
    border-radius:5px;border:2px solid var(--bg);
  }
  ::-webkit-scrollbar-thumb:hover{
    background:linear-gradient(180deg,var(--purple),var(--purple-bright));
  }

  /* Selection */
  ::selection{background:rgba(168,85,247,0.4);color:#fff;}
</style>
</head>
<body>
<header>
  <div class="brand">
    <div class="dot"></div>
    <div>
      <h1>yaw<span>_</span>cc<span>//</span>build</h1>
      <div class="sub">local build server</div>
    </div>
  </div>
  <div class="actions">
    <button id="clr" class="ghost">clear</button>
    <button id="btn" class="primary">Build</button>
    <span class="status" id="st">idle</span>
  </div>
</header>
<main>
  <div id="logs-wrap">
    <div id="logs">
      <div class="empty">
        <span class="icon">◈</span>
        press build to compile
      </div>
    </div>
  </div>
</main>

<script>
const logsEl = document.getElementById('logs');
const btn    = document.getElementById('btn');
const clr    = document.getElementById('clr');
const stEl   = document.getElementById('st');
let es = null;
let empty = true;
let autoScroll = true;

function setStatus(txt, cls){
  stEl.textContent = txt;
  stEl.className = 'status ' + (cls||'');
}
function addLine(text, kind){
  if(empty){ logsEl.innerHTML=''; empty=false; }
  const d = document.createElement('div');
  d.className = 'line ' + (kind||'log');
  if(kind==='log'){
    const low = (text||'').toLowerCase();
    if(low.includes('error') || low.includes('fatal') || low.includes(' err ')) d.classList.add('err');
    else if(low.includes('warning') || low.includes('warn')) d.classList.add('warn');
  }
  d.textContent = text || ' ';
  logsEl.appendChild(d);
  if(autoScroll){
    // smooth scroll to bottom
    requestAnimationFrame(() => {
      logsEl.scrollTo({top: logsEl.scrollHeight, behavior: 'smooth'});
    });
  }
}

// Detect user scroll — pause autoscroll if scrolled up
logsEl.addEventListener('scroll', () => {
  const nearBottom = logsEl.scrollHeight - logsEl.scrollTop - logsEl.clientHeight < 60;
  autoScroll = nearBottom;
});

btn.onclick = () => {
  if(es){ return; }
  logsEl.innerHTML=''; empty=true; autoScroll=true;
  btn.disabled = true;
  setStatus('running', 'running');
  es = new EventSource('/build');
  es.onmessage = ev => {
    let m; try{ m = JSON.parse(ev.data); } catch(e){ return; }
    if(m.type === 'done'){
      es.close(); es = null;
      btn.disabled = false;
      setStatus(m.code === 0 ? 'success' : 'failed ('+m.code+')', m.code===0?'ok':'err');
      return;
    }
    addLine(m.line, m.type);
  };
  es.onerror = () => {
    if(es){ es.close(); es = null; }
    btn.disabled = false;
    setStatus('connection lost', 'err');
  };
};

clr.onclick = () => {
  logsEl.innerHTML='<div class="empty"><span class="icon">◈</span>cleared</div>';
  empty=true; autoScroll=true;
  setStatus('idle','');
};
</script>
</body>
</html>
"""


class BuildHTTPHandler(http.server.BaseHTTPRequestHandler):
    def log_message(self, fmt, *args):
        # quieter: only errors to console
        if str(args[1] if len(args) > 1 else "") .startswith(("4","5")):
            sys.stderr.write("%s - %s\n" % (self.address_string(), fmt%args))

    def _send(self, code, body, ctype="text/plain; charset=utf-8", extra_headers=None):
        self.send_response(code)
        self.send_header("Content-Type", ctype)
        self.send_header("Cache-Control", "no-store")
        if extra_headers:
            for k,v in extra_headers.items():
                self.send_header(k,v)
        if isinstance(body, str):
            body = body.encode("utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_GET(self):
        path = urlparse(self.path).path
        if path in ("/", "/index.html"):
            self._send(200, HTML, "text/html; charset=utf-8",
                       extra_headers={"Cache-Control":"no-store, no-cache, must-revalidate", "Pragma":"no-cache"})
            return
        if path == "/build":
            self._sse_build()
            return
        self._send(404, "not found")

    def _sse_build(self):
        q = start_build()
        # start_build now always returns a queue — no "already running" refusals.

        self.send_response(200)
        self.send_header("Content-Type", "text/event-stream")
        self.send_header("Cache-Control", "no-store")
        self.send_header("Connection", "keep-alive")
        self.send_header("X-Accel-Buffering", "no")
        self.end_headers()

        try:
            while True:
                try:
                    msg = q.get(timeout=30)
                except queue.Empty:
                    # keepalive comment
                    try:
                        self.wfile.write(b": ping\n\n")
                        self.wfile.flush()
                    except Exception:
                        return
                    continue
                payload = ("data: " + json.dumps(msg, ensure_ascii=False) + "\n\n").encode("utf-8")
                try:
                    self.wfile.write(payload)
                    self.wfile.flush()
                except Exception:
                    return
                if msg.get("type") == "done":
                    return
        except (BrokenPipeError, ConnectionResetError):
            return


class ThreadedServer(socketserver.ThreadingMixIn, http.server.HTTPServer):
    daemon_threads = True
    allow_reuse_address = True


def main():
    url = f"http://127.0.0.1:{PORT}/"
    print("=" * 60)
    print(f"  yaw_cc build server")
    print(f"  {url}")
    print(f"  build.bat: {BAT_PATH}")
    print("=" * 60)
    # open browser after a short delay
    threading.Timer(0.7, lambda: webbrowser.open(url)).start()
    with ThreadedServer(("127.0.0.1", PORT), BuildHTTPHandler) as srv:
        try:
            srv.serve_forever()
        except KeyboardInterrupt:
            print("\nbye")


if __name__ == "__main__":
    main()
