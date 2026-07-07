import json
import os
import socket
import threading
import time
import urllib.request

_API_URL = os.getenv("SFP_API_URL", "http://localhost:8080/api/v1/raw/current")
_API_TIMEOUT = float(os.getenv("SFP_API_TIMEOUT", "1.5"))
_API_RETRIES = 3
_API_RETRY_DELAY = 0.5


class SFPReader:
    SOCKET_PATH = "/run/sfp-daemon/sfp.sock"
    _cache = None
    _lock = threading.Lock()
    _last_read: float = 0

    @staticmethod
    def get_data():
        now = time.time()
        with SFPReader._lock:
            if SFPReader._cache and (now - SFPReader._last_read < 0.5):
                return SFPReader._cache
        data = SFPReader._fetch_api() or SFPReader._fetch_socket()
        if data and data.get("status") in ("not_found", "error"):
            data = None
        if data:
            with SFPReader._lock:
                SFPReader._cache = data
                SFPReader._last_read = now
        return data

    @staticmethod
    def _fetch_api():
        for attempt in range(_API_RETRIES):
            try:
                with urllib.request.urlopen(_API_URL, timeout=_API_TIMEOUT) as resp:
                    return json.loads(resp.read())
            except Exception:
                if attempt < _API_RETRIES - 1:
                    time.sleep(_API_RETRY_DELAY)
        return None

    @staticmethod
    def ping() -> dict | None:
        """Lightweight daemon health check via PING command."""
        try:
            with urllib.request.urlopen(
                _API_URL.replace("/raw/current", "").replace("/api/v1", "") + "/health",
                timeout=1.0,
            ) as resp:
                return json.loads(resp.read())
        except Exception:
            pass
        try:
            if not os.path.exists(SFPReader.SOCKET_PATH):
                return None
            with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as s:
                s.settimeout(1.0)
                s.connect(SFPReader.SOCKET_PATH)
                s.sendall(b"PING\n")
                buf = b""
                while True:
                    chunk = s.recv(4096)
                    if not chunk:
                        break
                    buf += chunk
                    if buf.strip().endswith(b"}"):
                        break
                raw = buf.decode("utf-8", errors="replace")
                if "{" in raw:
                    return json.loads(raw[raw.find("{"):])
        except Exception:
            pass
        return None

    @staticmethod
    def _fetch_socket():
        try:
            if not os.path.exists(SFPReader.SOCKET_PATH):
                return None
            with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as s:
                s.settimeout(1.0)
                s.connect(SFPReader.SOCKET_PATH)
                s.sendall(b"GET CURRENT\n")
                buf = ""
                while True:
                    chunk = s.recv(4096).decode("utf-8", errors="replace")
                    if not chunk:
                        break
                    buf += chunk
                    if buf.strip().endswith("}"):
                        break
                if "{" in buf:
                    return json.loads(buf[buf.find("{"):])
        except Exception as e:
            print(f"SFP socket erro: {e}")
        return None
