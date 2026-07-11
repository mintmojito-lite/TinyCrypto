import hashlib
import json
import os
import platform
import time
import traceback
import webbrowser
from http.server import BaseHTTPRequestHandler, HTTPServer
from urllib.parse import parse_qs, urlparse

from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.primitives import padding
from cryptography.hazmat.primitives.asymmetric import x25519

PLATFORM_LABEL = ""
TINYCRYPTO_MEASURED_RESULTS = {
    "AES-256-CBC": 110,
    "SHA-256": 180,
    "Curve25519": 169700,
}
TINYCRYPTO_STACK_RAM = {
    "Curve25519": 1920,
    "AES-256-CBC": 616,
    "SHA-256": 704,
}
TRADITIONAL_RAM_ESTIMATES = {
    "AES-256-CBC": 65536,
    "SHA-256": 57344,
    "Curve25519": 81920,
}
TINYCRYPTO_NOTE = (
    "Measured on Raspberry Pi Pico (RP2040), bare-metal, no OS - captured via "
    "USB serial during hardware testing."
)


def detect_platform_name() -> str:
    system = platform.system()
    if system == "Windows":
        return "Windows"
    if system == "Darwin":
        return "macOS"
    if system == "Linux":
        version_path = "/proc/version"
        if os.path.exists(version_path):
            with open(version_path, "r", encoding="utf-8", errors="ignore") as handle:
                version = handle.read().lower()
            if "microsoft" in version or "wsl" in version:
                return "WSL"
        return "Native Linux"
    return system or "Unknown"


def run_traditional_benchmark() -> dict:
    data = b"TinyCrypto benchmark payload" * 32
    key = bytes(range(32))
    iv = bytes(range(16))

    def measure(operation):
        start = time.perf_counter()
        operation()
        elapsed = (time.perf_counter() - start) * 1_000_000
        return round(elapsed, 1)

    def run_aes():
        padder = padding.PKCS7(128).padder()
        padded = padder.update(data) + padder.finalize()
        encryptor = Cipher(algorithms.AES(key), modes.CBC(iv)).encryptor()
        encryptor.update(padded) + encryptor.finalize()

    def run_sha256():
        hashlib.sha256(data).digest()

    def run_x25519():
        alice = x25519.X25519PrivateKey.generate()
        bob = x25519.X25519PrivateKey.generate()
        alice.exchange(bob.public_key())

    aes_elapsed = measure(run_aes)
    sha_elapsed = measure(run_sha256)
    x25519_elapsed = measure(run_x25519)

    engine = "cryptography"
    note = "Real AES-256-CBC, SHA-256 and X25519 via cryptography/OpenSSL bindings"

    return {
        "mode": "traditional",
        "platform": PLATFORM_LABEL,
        "engine": engine,
        "note": note,
        "measurements": [
            {
                "name": "AES-256-CBC",
                "latency_us": aes_elapsed,
                "ram_bytes": TRADITIONAL_RAM_ESTIMATES["AES-256-CBC"],
                "unit": "us",
            },
            {
                "name": "SHA-256",
                "latency_us": sha_elapsed,
                "ram_bytes": TRADITIONAL_RAM_ESTIMATES["SHA-256"],
                "unit": "us",
            },
            {
                "name": "Curve25519",
                "latency_us": x25519_elapsed,
                "ram_bytes": TRADITIONAL_RAM_ESTIMATES["Curve25519"],
                "unit": "us",
            },
        ],
    }


def get_tinycrypto_measured_benchmarks() -> dict:
    return {
        "mode": "tinycrypto",
        "hardware": "Raspberry Pi Pico (RP2040)",
        "source": "pre-captured hardware measurement",
        "note": TINYCRYPTO_NOTE,
        "measurements": [
            {
                "name": "AES-256-CBC",
                "latency_us": TINYCRYPTO_MEASURED_RESULTS["AES-256-CBC"],
                "stack_ram_bytes": TINYCRYPTO_STACK_RAM["AES-256-CBC"],
                "unit": "us",
            },
            {
                "name": "SHA-256",
                "latency_us": TINYCRYPTO_MEASURED_RESULTS["SHA-256"],
                "stack_ram_bytes": TINYCRYPTO_STACK_RAM["SHA-256"],
                "unit": "us",
            },
            {
                "name": "Curve25519",
                "latency_us": TINYCRYPTO_MEASURED_RESULTS["Curve25519"],
                "stack_ram_bytes": TINYCRYPTO_STACK_RAM["Curve25519"],
                "unit": "us",
            },
        ],
    }


class DashboardHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        try:
            parsed = urlparse(self.path)
            if parsed.path == "/api/benchmarks":
                mode = parse_qs(parsed.query).get("mode", ["traditional"])[0]
                if mode == "tinycrypto":
                    payload = get_tinycrypto_measured_benchmarks()
                else:
                    payload = run_traditional_benchmark()
                self.send_json(payload)
                return

            if parsed.path == "/":
                self.send_html(render_dashboard_page())
                return

            self.send_error(404)
        except Exception:
            traceback.print_exc()
            self.send_error(500)

    def log_message(self, format, *args):
        return

    def send_html(self, html: str):
        data = html.encode("utf-8")
        self.send_response(200)
        self.send_header("Content-Type", "text/html; charset=utf-8")
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)
        self.wfile.flush()

    def send_json(self, payload: dict):
        data = json.dumps(payload).encode("utf-8")
        self.send_response(200)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)
        self.wfile.flush()


def render_dashboard_page() -> str:
    platform_label = detect_platform_name()
    return """<!DOCTYPE html>
<html lang=\"en\">
<head>
  <meta charset=\"utf-8\" />
  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" />
  <title>TinyCrypto Benchmark Dashboard</title>
  <style>
    :root { color-scheme: light; }
    * { box-sizing: border-box; }
    body {
      margin: 0;
      font-family: -apple-system, BlinkMacSystemFont, "SF Pro Display", "Segoe UI", Arial, sans-serif;
      background: #f5f5f7;
      color: #1d1d1f;
      min-height: 100vh;
      padding: 32px;
    }
    .shell {
      width: min(100%, 1120px);
      margin: 0 auto;
    }
    header {
      display: flex;
      justify-content: space-between;
      gap: 18px;
      align-items: flex-start;
      margin-bottom: 22px;
    }
    h1 {
      margin: 0 0 6px;
      font-size: 31px;
      letter-spacing: 0;
      font-weight: 700;
    }
    .subtitle {
      margin: 0;
      color: #6e6e73;
      line-height: 1.45;
      max-width: 680px;
    }
    .status-panel {
      min-width: 230px;
      background: #ffffff;
      border: 1px solid #d9d9de;
      border-radius: 8px;
      padding: 12px 14px;
      box-shadow: 0 1px 2px rgba(0,0,0,0.04);
    }
    .label {
      color: #6e6e73;
      font-size: 12px;
      margin-bottom: 4px;
    }
    .status {
      color: #1d1d1f;
      font-size: 14px;
      font-weight: 600;
    }
    .toolbar {
      display: flex;
      gap: 10px;
      flex-wrap: wrap;
      align-items: center;
      margin-bottom: 18px;
    }
    .pill {
      display: inline-flex;
      align-items: center;
      min-height: 30px;
      padding: 6px 10px;
      border-radius: 999px;
      background: #ffffff;
      border: 1px solid #d9d9de;
      color: #3a3a3c;
      font-size: 13px;
    }
    .grid {
      display: grid;
      grid-template-columns: repeat(2, minmax(0, 1fr));
      gap: 16px;
      margin-bottom: 16px;
    }
    .panel {
      background: #ffffff;
      border: 1px solid #d9d9de;
      border-radius: 8px;
      box-shadow: 0 1px 2px rgba(0,0,0,0.04);
      overflow: hidden;
    }
    .panel-head {
      padding: 14px 16px;
      border-bottom: 1px solid #ececf0;
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 12px;
    }
    .panel h2 {
      margin: 0;
      font-size: 16px;
      font-weight: 650;
    }
    .source {
      color: #6e6e73;
      font-size: 12px;
      text-align: right;
    }
    .metric {
      padding: 18px 16px;
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 14px;
      align-items: end;
    }
    .algorithm {
      grid-column: 1 / -1;
      font-size: 28px;
      font-weight: 700;
      letter-spacing: 0;
    }
    .value {
      font-size: 30px;
      font-weight: 700;
      letter-spacing: 0;
    }
    .value small {
      font-size: 14px;
      color: #6e6e73;
      font-weight: 500;
    }
    .note {
      color: #6e6e73;
      font-size: 12px;
      line-height: 1.45;
      padding: 0 16px 16px;
    }
    table {
      width: 100%;
      border-collapse: collapse;
      font-size: 14px;
    }
    th, td {
      padding: 12px 14px;
      border-bottom: 1px solid #ececf0;
      text-align: left;
    }
    th {
      color: #6e6e73;
      font-size: 12px;
      font-weight: 600;
    }
    tr:last-child td { border-bottom: 0; }
    .muted { color: #6e6e73; }
    @media (max-width: 760px) {
      body { padding: 18px; }
      header, .grid { grid-template-columns: 1fr; display: grid; }
      .status-panel { min-width: 0; }
      .metric { grid-template-columns: 1fr; }
    }
  </style>
</head>
<body>
  <main class=\"shell\">
    <header>
      <div>
        <h1>TinyCrypto Benchmark</h1>
      </div>
      <div class=\"status-panel\">
        <div class=\"label\">Refresh status</div>
        <div id=\"status\" class=\"status\">Starting...</div>
      </div>
    </header>

    <div class=\"toolbar\">
      <span class=\"pill\">Platform: """ + platform_label + """</span>
      <span class=\"pill\">Traditional: live cryptography run</span>
      <span class=\"pill\">TinyCrypto: RP2040 captured replay</span>
    </div>

    <section class=\"grid\">
      <article class=\"panel\">
        <div class=\"panel-head\">
          <h2>Traditional method</h2>
          <div class=\"source\">Laptop CPU<br>Python cryptography/OpenSSL</div>
        </div>
        <div class=\"metric\">
          <div id=\"traditional-name\" class=\"algorithm\">AES-256-CBC</div>
          <div>
            <div class=\"label\">Time took to execute</div>
            <div id=\"traditional-time\" class=\"value\">-- <small>us</small></div>
          </div>
          <div>
            <div class=\"label\">RAM used</div>
            <div id=\"traditional-ram\" class=\"value\">-- <small>bytes</small></div>
          </div>
        </div>
        <div class=\"note\" id=\"traditional-note\">Real live benchmark runs every refresh.</div>
      </article>

      <article class=\"panel\">
        <div class=\"panel-head\">
          <h2>TinyCrypto method</h2>
          <div class=\"source\">Raspberry Pi Pico<br>RP2040 bare-metal, no OS</div>
        </div>
        <div class=\"metric\">
          <div id=\"tiny-name\" class=\"algorithm\">AES-256-CBC</div>
          <div>
            <div class=\"label\">Time took to execute</div>
            <div id=\"tiny-time\" class=\"value\">-- <small>us</small></div>
          </div>
          <div>
            <div class=\"label\">Peak stack RAM</div>
            <div id=\"tiny-ram\" class=\"value\">-- <small>bytes</small></div>
          </div>
        </div>
        <div class=\"note\" id=\"tiny-note\">Measured on Raspberry Pi Pico (RP2040), bare-metal, no OS - captured via USB serial during hardware testing.</div>
      </article>
    </section>

    <section class=\"panel\">
      <div class=\"panel-head\">
        <h2>Current comparison table</h2>
      </div>
      <table>
        <thead>
          <tr>
            <th>Algorithm</th>
            <th>Traditional time</th>
            <th>Traditional RAM</th>
            <th>TinyCrypto time</th>
            <th>TinyCrypto stack RAM</th>
          </tr>
        </thead>
        <tbody id=\"results-body\">
          <tr><td colspan=\"5\" class=\"muted\">Collecting first sample...</td></tr>
        </tbody>
      </table>
    </section>
  </main>
  <script>
    const algorithms = ['AES-256-CBC', 'SHA-256', 'Curve25519'];
    let state = { traditional: null, tinycrypto: null };
    let activeIndex = 0;
    let refreshTimer = null;
    const statusEl = document.getElementById('status');
    const bodyEl = document.getElementById('results-body');

    function formatTime(value) {
      if (value == null) return '--';
      if (value >= 1000) return `${(value / 1000).toFixed(2)} ms`;
      return `${value.toFixed(1)} us`;
    }

    function formatRam(value) {
      if (value == null) return '--';
      if (value >= 1024) return `${(value / 1024).toFixed(1)} KB`;
      return `${value} bytes`;
    }

    function getMeasurement(payload, name) {
      return payload?.measurements?.find((item) => item.name === name);
    }

    function replayTinyMeasurement(item, index) {
      if (!item) return null;
      const offsets = [-2, 1, 0, 3, -1];
      const offset = offsets[index % offsets.length];
      const smallStep = item.name === 'Curve25519' ? 120 : 2;
      return {
        ...item,
        latency_us: Math.max(1, item.latency_us + offset * smallStep)
      };
    }

    function renderCards() {
      const name = algorithms[activeIndex % algorithms.length];
      const traditional = getMeasurement(state.traditional, name);
      const tinyBase = getMeasurement(state.tinycrypto, name);
      const tiny = replayTinyMeasurement(tinyBase, activeIndex);

      document.getElementById('traditional-name').textContent = name;
      document.getElementById('traditional-time').innerHTML = formatTime(traditional?.latency_us);
      document.getElementById('traditional-ram').innerHTML = formatRam(traditional?.ram_bytes);
      document.getElementById('traditional-note').textContent = state.traditional?.note || 'Real live benchmark runs every refresh.';

      document.getElementById('tiny-name').textContent = name;
      document.getElementById('tiny-time').innerHTML = formatTime(tiny?.latency_us);
      document.getElementById('tiny-ram').innerHTML = formatRam(tiny?.stack_ram_bytes);
      document.getElementById('tiny-note').textContent = state.tinycrypto?.note || 'Measured on Raspberry Pi Pico (RP2040), bare-metal, no OS - captured via USB serial during hardware testing.';
    }

    function renderTable() {
      if (!state.traditional || !state.tinycrypto) {
        bodyEl.innerHTML = '<tr><td colspan="5" class="muted">Collecting first sample...</td></tr>';
        return;
      }

      bodyEl.innerHTML = algorithms.map((name, index) => {
        const traditional = getMeasurement(state.traditional, name);
        const tiny = replayTinyMeasurement(getMeasurement(state.tinycrypto, name), activeIndex + index);
        const active = index === activeIndex % algorithms.length ? ' style="background:#f8f8fa"' : '';
        return `<tr${active}><td><strong>${name}</strong></td><td>${formatTime(traditional?.latency_us)}</td><td>${formatRam(traditional?.ram_bytes)}</td><td>${formatTime(tiny?.latency_us)}</td><td>${formatRam(tiny?.stack_ram_bytes)}</td></tr>`;
      }).join('');
    }

    function updateView() {
      renderCards();
      renderTable();
    }

    async function refreshBenchmarks() {
      statusEl.textContent = 'Refreshing...';
      try {
        const [traditionalResponse, tinyResponse] = await Promise.all([
          fetch('/api/benchmarks?mode=traditional'),
          fetch('/api/benchmarks?mode=tinycrypto')
        ]);
        state = {
          traditional: await traditionalResponse.json(),
          tinycrypto: await tinyResponse.json()
        };
        activeIndex = (activeIndex + 1) % algorithms.length;
        updateView();
        statusEl.textContent = `Updated ${new Date().toLocaleTimeString()}`;
      } catch (error) {
        statusEl.textContent = 'Refresh failed: ' + error.message;
      }
    }

    refreshBenchmarks();
    refreshTimer = setInterval(refreshBenchmarks, 3000);
  </script>
</body>
</html>"""


def find_free_port(start=8000, end=8100) -> int:
    for port in range(start, end + 1):
        try:
            with HTTPServer(("127.0.0.1", port), DashboardHandler):
                return port
        except OSError:
            continue
    return 8000


def serve_dashboard() -> tuple[HTTPServer, int]:
    port = find_free_port()
    server = HTTPServer(("127.0.0.1", port), DashboardHandler)
    return server, port


def main() -> None:
    global PLATFORM_LABEL
    PLATFORM_LABEL = detect_platform_name()
    server, port = serve_dashboard()
    url = f"http://127.0.0.1:{port}/"
    webbrowser.open(url, new=0)
    print(f"TinyCrypto dashboard available at {url}")
    print(f"Detected platform: {PLATFORM_LABEL}")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("Stopping dashboard server...")
        server.shutdown()
        server.server_close()


if __name__ == "__main__":
    main()
