import os
import json
import socket
import asyncio
import io
import csv
import time
import threading
from datetime import datetime, timezone
from typing import Any, Dict, List, Optional, Tuple
from contextlib import asynccontextmanager
from fastapi import FastAPI, HTTPException, BackgroundTasks, Depends
from fastapi.security import APIKeyHeader
from fastapi.responses import JSONResponse, StreamingResponse
from pydantic import BaseModel
from pymongo import ASCENDING, DESCENDING
from pymongo.errors import PyMongoError

# Importações do novo pacote de banco de dados
from database import get_mongo_client, get_database, run_migrations, CurrentReading, HistoryPoint

try:
    import docker as docker_sdk
    _docker_client = docker_sdk.from_env()
except Exception:
    _docker_client = None

_container_api_key = os.getenv("CONTAINER_API_KEY", "")
_api_key_header = APIKeyHeader(name="X-Api-Key", auto_error=False)

def _require_container_key(key: Optional[str] = Depends(_api_key_header)):
    if _container_api_key and key != _container_api_key:
        raise HTTPException(status_code=401, detail="Invalid or missing API key")


class DynamicReading(BaseModel):
    timestamp: str
    rx_power_dbm: Optional[float] = None
    tx_power_dbm: Optional[float] = None
    temperature_c: Optional[float] = None
    voltage_v: Optional[float] = None
    bias_ma: Optional[float] = None
    data_ready: Optional[bool] = None


def sfp_socket_path() -> str:
    return os.getenv("SFP_DAEMON_SOCKET", "/run/sfp-daemon/sfp.sock")


_socket_lock = threading.Lock()
_socket_cache: Dict[str, Tuple[float, Any]] = {}
_SOCKET_CACHE_TTL = 5.0


def first_numeric(d: Dict[str, Any], keys: List[str]) -> Optional[float]:
    for k in keys:
        v = d.get(k)
        if isinstance(v, (int, float)):
            try:
                return float(v)
            except Exception:
                continue
    return None


def send_command(command: str) -> Dict[str, Any]:
    with _socket_lock:
        now = time.time()
        if command in _socket_cache:
            ts, cached = _socket_cache[command]
            if now - ts < _SOCKET_CACHE_TTL:
                return cached

        path = sfp_socket_path()
        timeout = float(os.getenv("SFP_SOCKET_TIMEOUT", "3"))
        last_exc: Exception = RuntimeError("no attempts")
        data = ""
        for attempt in range(3):
            try:
                with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as s:
                    s.settimeout(timeout)
                    s.connect(path)
                    s.sendall((command + "\n").encode("utf-8"))
                    chunks: List[bytes] = []
                    while True:
                        b = s.recv(4096)
                        if not b:
                            break
                        chunks.append(b)
                        if b.endswith(b"\n"):
                            break
                    data = b"".join(chunks).decode("utf-8", errors="replace")
                break
            except (FileNotFoundError, ConnectionRefusedError, TimeoutError, socket.error, BlockingIOError) as e:
                last_exc = e
                if attempt < 2:
                    time.sleep(0.3 * (attempt + 1))
        else:
            raise HTTPException(status_code=503, detail=f"Socket error: {last_exc}")

        lines = data.split("\n", 1)
        if not lines:
            raise HTTPException(status_code=502, detail="Empty daemon response")
        status_line = lines[0].strip()
        body = lines[1] if len(lines) > 1 else ""
        if not body.strip():
            raise HTTPException(status_code=502, detail="Missing JSON body")
        try:
            payload = json.loads(body)
        except json.JSONDecodeError:
            raise HTTPException(status_code=502, detail="Invalid JSON from daemon")
        if not status_line.startswith("STATUS"):
            raise HTTPException(status_code=502, detail="Invalid status line")

        _socket_cache[command] = (time.time(), payload)
        return payload


def map_current(payload: Dict[str, Any], dynamic_payload: Optional[Dict[str, Any]] = None) -> CurrentReading:
    ts = payload.get("timestamps", {})
    a0 = payload.get("a0", {}) or {}
    a2_current = payload.get("a2", {}) or {}
    if not isinstance(a2_current, dict):
        a2_current = {}
    a2_dyn: Dict[str, Any] = {}
    if dynamic_payload and isinstance(dynamic_payload, dict):
        candidate = dynamic_payload.get("a2", {}) or {}
        if isinstance(candidate, dict):
            a2_dyn = candidate
    voltage = first_numeric(a2_current, ["voltage_v", "vcc_realtime"])
    rx_power = first_numeric(a2_current, ["rx_power_dbm"])
    tx_power = first_numeric(a2_dyn or a2_current, ["tx_power_dbm"])
    temp = first_numeric(a2_dyn or a2_current, ["temperature_c", "temp_c", "temp_realtime"])
    bias = first_numeric(a2_dyn or a2_current, ["tx_bias_ma", "tx_bias_realtime"])
    if rx_power is None:
        rx_power = -8.0
    sq: Optional[str] = None
    if isinstance(rx_power, (int, float)):
        if rx_power >= -3.0:
            sq = "Excelente"
        elif rx_power >= -9.0:
            sq = "OK"
        else:
            sq = "Ruim"
    module = {
        "identifier": a0.get("identifier"),
        "identifier_type": a0.get("identifier_type"),
        "ext_identifier": a0.get("ext_identifier"),
        "connector": a0.get("connector"),
        "connector_type": a0.get("connector_type"),
        "encoding": a0.get("encoding"),
        "vendor_name": a0.get("vendor_name"),
        "vendor_pn": a0.get("vendor_pn"),
        "vendor_sn": a0.get("vendor_sn"),
        "vendor_rev": a0.get("vendor_rev"),
        "wavelength_nm": a0.get("wavelength_nm"),
        "ext_compliance_desc": a0.get("ext_compliance_desc"),
        "cc_base_valid": a0.get("cc_base_valid"),
    }
    
    # Se não houver timestamp do daemon, usamos o local
    last_read = ts.get("last_a2_read") or ts.get("last_a0_read")
    created_at = datetime.now(timezone.utc)
    
    if not last_read:
        last_read = created_at.isoformat()
    else:
        # Daemon envia epoch, convertemos para ISO string para o banco
        if isinstance(last_read, (int, float)):
            last_read = datetime.fromtimestamp(last_read, tz=timezone.utc).isoformat()

    return CurrentReading(
        timestamp=str(last_read),
        created_at=created_at,
        rx_power_dbm=float(rx_power),
        tx_power_dbm=tx_power,
        temperature_c=temp,
        voltage_v=voltage,
        bias_ma=bias,
        signal_quality=sq,
        module=module,
    )


def map_dynamic(payload: Dict[str, Any]) -> DynamicReading:
    a2 = payload.get("a2", {}) or {}
    if not isinstance(a2, dict):
        a2 = {}
    rx_power = first_numeric(a2, ["rx_power_dbm"])
    tx_power = first_numeric(a2, ["tx_power_dbm"])
    temp = first_numeric(a2, ["temperature_c", "temp_c", "temp_realtime"])
    voltage = first_numeric(a2, ["voltage_v", "vcc_realtime"])
    bias = first_numeric(a2, ["tx_bias_ma", "tx_bias_realtime"])
    ts = payload.get("last_a2_read")
    
    if not ts:
        ts = datetime.now(timezone.utc).isoformat()
    else:
        if isinstance(ts, (int, float)):
            ts = datetime.fromtimestamp(ts, tz=timezone.utc).isoformat()

    data_ready_val = a2.get("data_ready")
    data_ready: Optional[bool]
    if isinstance(data_ready_val, bool):
        data_ready = data_ready_val
    else:
        data_ready = None
    return DynamicReading(
        timestamp=str(ts),
        rx_power_dbm=rx_power,
        tx_power_dbm=tx_power,
        temperature_c=temp,
        voltage_v=voltage,
        bias_ma=bias,
        data_ready=data_ready,
    )


# Inicialização do Banco de Dados
mongo_client = get_mongo_client()
mongo_db = get_database()
mongo_coll = mongo_db["readings"] if mongo_db is not None else None

# Executa as migrações (índices, etc)
run_migrations()


async def background_sampler():
    """
    Coleta dados a cada 5 segundos para persistir histórico,
    mesmo que ninguém esteja olhando o frontend.
    """
    while True:
        try:
            if mongo_coll is not None:
                payload = await asyncio.to_thread(send_command, "GET CURRENT")
                if payload.get("status") in ("not_found", "error"):
                    await asyncio.sleep(5)
                    continue
                try:
                    dynamic_payload = await asyncio.to_thread(send_command, "GET DYNAMIC")
                except Exception:
                    dynamic_payload = None

                current = map_current(payload, dynamic_payload)
                doc = current.model_dump()
                mongo_coll.insert_one(doc)
        except Exception:
            pass
        await asyncio.sleep(5)


@asynccontextmanager
async def lifespan(app: FastAPI):
    # Inicializa o sampler em background
    sampler_task = asyncio.create_task(background_sampler())
    yield
    # Cancela o sampler ao fechar o app
    sampler_task.cancel()
    if mongo_client:
        mongo_client.close()
    print("MongoDB connection closed.")


app = FastAPI(lifespan=lifespan)


@app.get("/health")
def health() -> JSONResponse:
    import os as _os
    daemon_ok = _os.path.exists(sfp_socket_path())
    return JSONResponse({
        "status": "ok",
        "db_connected": mongo_coll is not None,
        "daemon_ok": daemon_ok,
    })


@app.get("/api/v1/ping")
def daemon_ping() -> JSONResponse:
    try:
        result = send_command("PING")
        return JSONResponse(result)
    except Exception as e:
        raise HTTPException(status_code=503, detail=str(e))


@app.get("/api/v1/current")
def api_current() -> CurrentReading:
    payload = send_command("GET CURRENT")
    dynamic_payload: Optional[Dict[str, Any]] = None
    try:
        dynamic_payload = send_command("GET DYNAMIC")
    except HTTPException:
        dynamic_payload = None
    
    current = map_current(payload, dynamic_payload)
    return current


@app.get("/api/static")
def api_static() -> Dict[str, Any]:
    def fetch_a0(cmd: str) -> Optional[Dict[str, Any]]:
        try:
            payload = send_command(cmd)
            a0 = payload.get("a0", {}) or {}
            if not isinstance(a0, dict) or not a0 or a0.get("valid") is False:
                return None
            return a0
        except HTTPException:
            return None
        except Exception:
            return None

    for cmd in ("GET STATIC", "GET CURRENT"):
        a0 = fetch_a0(cmd)
        if a0:
            return a0
    return {}


@app.get("/api/v1/a0h")
def api_a0h() -> Dict[str, Any]:
    return api_static()


@app.get("/api/v1/a2h")
def api_a2h() -> Dict[str, Any]:
    payload = send_command("GET DYNAMIC")
    a2 = payload.get("a2", {}) or {}
    return a2


@app.get("/api/dynamic")
def api_dynamic() -> DynamicReading:
    payload = send_command("GET DYNAMIC")
    return map_dynamic(payload)


@app.get("/api/v1/raw")
def api_raw() -> Dict[str, Any]:
    payload = send_command("GET CURRENT")
    return payload


def _cmd_from_alias(alias: str) -> Optional[str]:
    a = alias.strip().lower()
    if a in ("current", "curr"):
        return "GET CURRENT"
    if a in ("static", "a0", "a0h"):
        return "GET STATIC"
    if a in ("dynamic", "a2", "a2h"):
        return "GET DYNAMIC"
    if a in ("state",):
        return "GET STATE"
    return None


@app.get("/api/v1/raw/{cmd}")
def api_raw_cmd(cmd: str) -> Dict[str, Any]:
    mapped = _cmd_from_alias(cmd)
    if not mapped:
        raise HTTPException(status_code=400, detail="Invalid cmd. Use: current|static|dynamic|state")
    return send_command(mapped)


@app.get("/api/v1/debug/all")
def api_debug_all() -> Dict[str, Any]:
    result: Dict[str, Any] = {}
    for key, alias in (
        ("current", "current"),
        ("static", "static"),
        ("dynamic", "dynamic"),
        ("state", "state"),
    ):
        try:
            mapped = _cmd_from_alias(alias)
            if mapped:
                result[key] = send_command(mapped)
            else:
                result[key] = {"error": "invalid"}
        except HTTPException as he:
            result[key] = {"error": he.detail}
        except Exception as e:
            result[key] = {"error": str(e)}
    return result


@app.get("/api/v1/history")
def api_history(limit: int = 30) -> List[HistoryPoint]:
    if mongo_coll is None:
        return []
    try:
        cur = mongo_coll.find({}, {"timestamp": 1, "rx_power_dbm": 1}).sort([("timestamp", DESCENDING)]).limit(limit)
        items = list(cur)
        items.reverse()
        return [HistoryPoint(timestamp=str(i.get("timestamp", "")), rx_power_dbm=i.get("rx_power_dbm")) for i in items]
    except PyMongoError:
        return []


@app.get("/api/v1/export/csv")
def export_csv():
    if mongo_coll is None:
        raise HTTPException(status_code=503, detail="Database not connected")
    
    try:
        # Busca todos os dados (estáticos + dinâmicos)
        cur = mongo_coll.find({}).sort([("timestamp", ASCENDING)])
        
        output = io.StringIO()
        writer = csv.writer(output)
        
        # Header
        writer.writerow([
            "Data/Hora (UTC)", 
            "RX Power (dBm)", 
            "Temp (C)", 
            "VCC (V)", 
            "Bias (mA)", 
            "Signal Quality",
            "Vendor",
            "Part Number",
            "Serial Number",
            "Connector",
            "Wavelength (nm)",
            "Type"
        ])
        
        for doc in cur:
            mod = doc.get("module", {})
            writer.writerow([
                doc.get("timestamp", ""),
                doc.get("rx_power_dbm", ""),
                doc.get("temperature_c", ""),
                doc.get("voltage_v", ""),
                doc.get("bias_ma", ""),
                doc.get("signal_quality", ""),
                mod.get("vendor_name", ""),
                mod.get("vendor_pn", ""),
                mod.get("vendor_sn", ""),
                mod.get("connector_type", ""),
                mod.get("wavelength_nm", ""),
                mod.get("ext_compliance_desc", "")
            ])
        
        output.seek(0)
        filename = f"sfp_readings_{datetime.now().strftime('%Y%m%d_%H%M%S')}.csv"
        return StreamingResponse(
            output,
            media_type="text/csv",
            headers={"Content-Disposition": f"attachment; filename={filename}"}
        )
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Export error: {e}")


class ContainerInfo(BaseModel):
    id: str
    name: str
    status: str
    image: str


class ContainerStats(BaseModel):
    cpu_percent: float
    memory_mb: float
    memory_percent: float


def _get_docker() -> Any:
    if _docker_client is None:
        raise HTTPException(status_code=503, detail="Docker socket unavailable")
    return _docker_client


@app.get("/api/v1/containers")
def list_containers() -> List[ContainerInfo]:
    dc = _get_docker()
    result = []
    for c in dc.containers.list(all=True):
        tags = c.image.tags
        image = tags[0] if tags else c.image.short_id
        result.append(ContainerInfo(id=c.short_id, name=c.name, status=c.status, image=image))
    return result


@app.post("/api/v1/containers/{name}/start", dependencies=[Depends(_require_container_key)])
def start_container(name: str):
    dc = _get_docker()
    try:
        dc.containers.get(name).start()
        return {"ok": True}
    except docker_sdk.errors.NotFound:
        raise HTTPException(status_code=404, detail="Container not found")


@app.post("/api/v1/containers/{name}/stop", dependencies=[Depends(_require_container_key)])
def stop_container(name: str):
    dc = _get_docker()
    try:
        dc.containers.get(name).stop()
        return {"ok": True}
    except docker_sdk.errors.NotFound:
        raise HTTPException(status_code=404, detail="Container not found")


@app.post("/api/v1/containers/{name}/restart", dependencies=[Depends(_require_container_key)])
def restart_container(name: str):
    dc = _get_docker()
    try:
        dc.containers.get(name).restart()
        return {"ok": True}
    except docker_sdk.errors.NotFound:
        raise HTTPException(status_code=404, detail="Container not found")


@app.get("/api/v1/containers/{name}/logs")
def get_container_logs(name: str, lines: int = 100):
    dc = _get_docker()
    try:
        c = dc.containers.get(name)
        logs = c.logs(tail=lines, timestamps=True).decode("utf-8", errors="replace")
        return {"logs": logs}
    except docker_sdk.errors.NotFound:
        raise HTTPException(status_code=404, detail="Container not found")


@app.get("/api/v1/containers/{name}/stats")
def get_container_stats(name: str) -> ContainerStats:
    dc = _get_docker()
    try:
        c = dc.containers.get(name)
        if c.status != "running":
            return ContainerStats(cpu_percent=0.0, memory_mb=0.0, memory_percent=0.0)
        s = c.stats(stream=False)
        cpu_delta = s["cpu_stats"]["cpu_usage"]["total_usage"] - s["precpu_stats"]["cpu_usage"]["total_usage"]
        sys_delta = s["cpu_stats"]["system_cpu_usage"] - s["precpu_stats"]["system_cpu_usage"]
        ncpus = s["cpu_stats"].get("online_cpus") or len(s["cpu_stats"]["cpu_usage"].get("percpu_usage", [1]))
        cpu_pct = (cpu_delta / sys_delta) * ncpus * 100.0 if sys_delta > 0 else 0.0
        mem_use = s["memory_stats"].get("usage", 0)
        mem_lim = s["memory_stats"].get("limit", 1)
        return ContainerStats(
            cpu_percent=round(cpu_pct, 1),
            memory_mb=round(mem_use / 1024 / 1024, 1),
            memory_percent=round(mem_use / mem_lim * 100, 1),
        )
    except docker_sdk.errors.NotFound:
        raise HTTPException(status_code=404, detail="Container not found")
    except (KeyError, ZeroDivisionError):
        return ContainerStats(cpu_percent=0.0, memory_mb=0.0, memory_percent=0.0)


if __name__ == "__main__":
    import uvicorn

    uvicorn.run(app, host="0.0.0.0", port=8000)
