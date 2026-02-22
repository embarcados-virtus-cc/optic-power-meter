import os
import json
import socket
from typing import Any, Dict, List, Optional
from fastapi import FastAPI, HTTPException
from fastapi.responses import JSONResponse
from pydantic import BaseModel
from pymongo import MongoClient, ASCENDING, DESCENDING
from pymongo.errors import PyMongoError


class CurrentReading(BaseModel):
    timestamp: str
    rx_power_dbm: float
    temperature_c: Optional[float] = None
    voltage_v: Optional[float] = None
    bias_ma: Optional[float] = None
    signal_quality: Optional[str] = None
    module: Dict[str, Any]


class HistoryPoint(BaseModel):
    timestamp: str
    rx_power_dbm: Optional[float] = None


def get_mongo_client() -> Optional[MongoClient]:
    uri = os.getenv("MONGO_URI")
    if not uri:
        return None
    try:
        return MongoClient(uri, serverSelectionTimeoutMS=2000)
    except Exception:
        return None


def sfp_socket_path() -> str:
    return os.getenv("SFP_DAEMON_SOCKET", "/run/sfp-daemon/sfp.sock")


def send_command(command: str) -> Dict[str, Any]:
    path = sfp_socket_path()
    try:
        with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as s:
            s.settimeout(float(os.getenv("SFP_SOCKET_TIMEOUT", "3")))
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
    except (FileNotFoundError, ConnectionRefusedError, TimeoutError, socket.error) as e:
        raise HTTPException(status_code=503, detail=f"Socket error: {e}")
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
    return payload


def map_current(payload: Dict[str, Any]) -> CurrentReading:
    ts = payload.get("timestamps", {})
    a0 = payload.get("a0", {}) or {}
    a2 = payload.get("a2", {}) or {}
    voltage = a2.get("voltage_v")
    rx_power = payload.get("rx_power_dbm")
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
        "ext_identifier": a0.get("ext_identifier"),
        "connector": a0.get("connector"),
        "encoding": a0.get("encoding"),
        "vendor_name": a0.get("vendor_name"),
        "vendor_pn": a0.get("vendor_pn"),
        "vendor_rev": a0.get("vendor_rev"),
        "cc_base_valid": a0.get("cc_base_valid"),
    }
    return CurrentReading(
        timestamp=str(ts.get("last_a2_read") or ts.get("last_a0_read") or ""),
        rx_power_dbm=float(rx_power),
        temperature_c=None,
        voltage_v=voltage if isinstance(voltage, (int, float)) else None,
        bias_ma=None,
        signal_quality=sq,
        module=module,
    )


app = FastAPI()
mongo_client = get_mongo_client()
mongo_db = mongo_client["optic_power_meter"] if mongo_client else None
mongo_coll = mongo_db["readings"] if mongo_db else None


@app.get("/health")
def health() -> JSONResponse:
    return JSONResponse({"status": "ok"})


@app.get("/api/v1/current")
def api_current() -> CurrentReading:
    payload = send_command("GET CURRENT")
    current = map_current(payload)
    if mongo_coll:
        doc = current.model_dump()
        try:
            mongo_coll.insert_one(doc)
        except PyMongoError:
            pass
    return current


@app.get("/api/static")
def api_static() -> Dict[str, Any]:
    payload = send_command("GET STATIC")
    a0 = payload.get("a0", {}) or {}
    return a0


@app.get("/api/v1/a0h")
def api_a0h() -> Dict[str, Any]:
    payload = send_command("GET STATIC")
    a0 = payload.get("a0", {}) or {}
    return a0


@app.get("/api/v1/a2h")
def api_a2h() -> Dict[str, Any]:
    payload = send_command("GET DYNAMIC")
    a2 = payload.get("a2", {}) or {}
    return a2


@app.get("/api/v1/raw")
def api_raw() -> Dict[str, Any]:
    payload = send_command("GET CURRENT")
    return payload


@app.get("/api/v1/history")
def api_history(limit: int = 30) -> List[HistoryPoint]:
    if not mongo_coll:
        return []
    try:
    uvicorn.run(app, host="0.0.0.0", port=8000)
        cur = mongo_coll.find({}, {"timestamp": 1, "rx_power_dbm": 1}).sort([("_id", DESCENDING)]).limit(limit)
    import uvicorn

        items = list(cur)
        items.reverse()
if __name__ == "__main__":
        return [HistoryPoint(timestamp=str(i.get("timestamp", "")), rx_power_dbm=i.get("rx_power_dbm")) for i in items]
    except PyMongoError:
        return []
