import http.server
import socketserver
import webbrowser
import subprocess
import os
import threading
import json

PORT = 8080
BAT_PATH = r"C:\Users\НИЗКИЙ\source\repos\ImGui-DirectX-11-Kiero-Hook-master\build.bat"

HTML_PAGE = """
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Build Runner</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
            background: linear-gradient(135deg, #1e3c72, #2a5298);
        }
        .container {
            background: white;
            padding: 40px;
            border-radius: 15px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.3);
            text-align: center;
            max-width: 500px;
            width: 90%;
        }
        h1 {
            color: #333;
            margin-bottom: 20px;
        }
        .info {
            background: #f0f0f0;
            padding: 10px;
            border-radius: 5px;
            margin: 10px 0;
            font-size: 12px;
            word-break: break-all;
            color: #666;
        }
        button {
            background: #e94560;
            color: white;
            border: none;
            padding: 15px 40px;
            font-size: 20px;
            border-radius: 8px;
            cursor: pointer;
            transition: all 0.3s;
            font-weight: bold;
            width: 100%;
            margin: 10px 0;
        }
        button:hover {
            background: #c73e54;
            transform: scale(1.02);
        }
        button:disabled {
            opacity: 0.5;
            cursor: not-allowed;
            transform: none;
        }
        #status {
            margin-top: 15px;
            padding: 10px;
            border-radius: 5px;
            display: none;
            font-weight: bold;
        }
        #status.success {
            display: block;
            background: #d4edda;
            color: #155724;
        }
        #status.error {
            display: block;
            background: #f8d7da;
            color: #721c24;
        }
        #status.running {
            display: block;
            background: #fff3cd;
            color: #856404;
            animation: pulse 1s infinite;
        }
        @keyframes pulse {
            0% { opacity: 1; }
            50% { opacity: 0.6; }
            100% { opacity: 1; }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🔨 Build Runner</h1>
        <div class="info">
            📁 build.bat
        </div>
        <button id="runBtn" onclick="runBuild()">▶ ЗАПУСТИТЬ СБОРКУ</button>
        <div id="status"></div>
    </div>

    <script>
        async function runBuild() {
            const btn = document.getElementById('runBtn');
            const status = document.getElementById('status');
            
            btn.disabled = true;
            btn.textContent = '⏳ ЗАПУСК...';
            
            status.className = 'running';
            status.textContent = '⏳ Открывается CMD...';
            status.style.display = 'block';
            
            try {
                const response = await fetch('/run_build', {
                    method: 'POST'
                });
                
                const data = await response.json();
                
                if (data.success) {
                    status.className = 'success';
                    status.textContent = '✅ ' + data.message;
                } else {
                    status.className = 'error';
                    status.textContent = '❌ ' + data.message;
                }
            } catch (error) {
                status.className = 'error';
                status.textContent = '❌ Ошибка: ' + error.message;
            }
            
            btn.disabled = false;
            btn.textContent = '▶ ЗАПУСТИТЬ СБОРКУ';
        }
    </script>
</body>
</html>
"""

class CustomHandler(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/' or self.path == '/index.html':
            self.send_response(200)
            self.send_header('Content-type', 'text/html; charset=utf-8')
            self.end_headers()
            self.wfile.write(HTML_PAGE.encode('utf-8'))
        else:
            self.send_response(404)
            self.end_headers()
    
    def do_POST(self):
        if self.path == '/run_build':
            self.send_response(200)
            self.send_header('Content-type', 'application/json; charset=utf-8')
            self.end_headers()
            
            print("\n[LOG] Запрос на запуск build.bat")
            
            try:
                if not os.path.exists(BAT_PATH):
                    print(f"[ERROR] Файл не найден: {BAT_PATH}")
                    response = {
                        'success': False,
                        'message': f'Файл не найден!'
                    }
                    self.wfile.write(json.dumps(response, ensure_ascii=False).encode('utf-8'))
                    return
                
                print(f"[LOG] Файл найден, запускаем CMD...")
                
                # ГЛАВНОЕ - ОТКРЫВАЕМ ОКНО CMD И ЗАПУСКАЕМ build.bat
                subprocess.Popen(
                    f'start cmd /k "{BAT_PATH}"',  # /k - оставляет окно открытым
                    shell=True,
                    cwd=os.path.dirname(BAT_PATH)
                )
                
                print("[LOG] ✅ CMD открыт, сборка запущена!")
                response = {
                    'success': True,
                    'message': 'CMD открыт! Сборка запущена!'
                }
                
            except Exception as e:
                print(f"[ERROR] Ошибка: {e}")
                response = {
                    'success': False,
                    'message': f'Ошибка: {str(e)}'
                }
            
            self.wfile.write(json.dumps(response, ensure_ascii=False).encode('utf-8'))
        else:
            self.send_response(404)
            self.end_headers()

def open_browser():
    try:
        webbrowser.open(f'http://localhost:{PORT}')
        print(f"[LOG] Браузер открыт: http://localhost:{PORT}")
    except Exception as e:
        print(f"[LOG] Откройте вручную: http://localhost:{PORT}")

if __name__ == '__main__':
    print("=" * 60)
    print("🔨 Build Runner - Запуск через CMD")
    print("=" * 60)
    
    print(f"\n📁 Путь к build.bat:")
    print(f"   {BAT_PATH}")
    
    if os.path.exists(BAT_PATH):
        print("   ✅ Файл существует")
    else:
        print("   ❌ Файл НЕ НАЙДЕН!")
    
    print(f"\n🌐 Сервер: http://localhost:{PORT}")
    print("   Нажми Ctrl+C для остановки")
    print("=" * 60 + "\n")
    
    try:
        with socketserver.TCPServer(("", PORT), CustomHandler) as httpd:
            threading.Timer(1, open_browser).start()
            httpd.serve_forever()
    except KeyboardInterrupt:
        print("\n\n🛑 Сервер остановлен")
    except OSError as e:
        print(f"\n❌ Ошибка: {e}")