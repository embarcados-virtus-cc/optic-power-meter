import ctypes
import os
import platform
import random
from datetime import datetime, timezone
from datetime import timedelta
from typing import Optional

from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
from pydantic_settings import BaseSettings, SettingsConfigDict
from sqlalchemy import Boolean, DateTime, Float, Integer, String, create_engine, select
from sqlalchemy.orm import DeclarativeBase, Mapped, Session, mapped_column


class Settings(BaseSettings):
    model_config = SettingsConfigDict(env_prefix="OPM_", extra="ignore")

    # I2C
    i2c_device: str = "/dev/i2c-1"

    # DB (Supabase = Postgres normal). Ex.: postgresql+psycopg://user:pass@host:5432/dbname
    database_url: Optional[str] = None

    # Amostragem/Histórico
    sample_period_seconds: float = 2.0
    history_default_limit: int = 30

    # Permite rodar no PC sem SFP (gera valores plausíveis)
    enable_mock_when_i2c_fails: bool = True


settings = Settings()


def _resolve_lib_path() -> str:
    # libsfp.so atualmente está dentro do diretório `sfp-interface/`
    here = os.path.dirname(__file__)
    candidates = [
        os.path.join(here, "sfp-interface", "libsfp.so"),
        os.path.join(here, "libsfp.so"),
    ]
    for p in candidates:
        ap = os.path.abspath(p)
        if os.path.exists(ap):
            return ap
    raise RuntimeError(
        "Não encontrei a biblioteca compartilhada `libsfp.so`. "
        "Compile em `sfp-interface/` (make) e garanta que `sfp-interface/libsfp.so` exista."
    )


LIB_PATH = _resolve_lib_path()
libsfp = None
try:
    # Em dev (x86_64), a libsfp.so pode estar compilada para ARM (aarch64) e falhar no load.
    # Nesses casos, seguimos com fallback mock.
    if os.environ.get("OPM_DISABLE_SFP_LIB", "").strip() not in ("1", "true", "TRUE", "yes", "YES"):
        libsfp = ctypes.CDLL(LIB_PATH)
except OSError:
    libsfp = None


SFP_A0_BASE_SIZE = 64
SFP_I2C_ADDR_A0 = 0x50
SFP_I2C_ADDR_A2 = 0x51
SFP_A2_DIAG_OFFSET = 96
SFP_A2_DIAG_SIZE = 16


class SfpA0hBase(ctypes.Structure):
    # Mantido minimalista; a lib C ainda não cobre A2/diagnósticos no Python.
    _fields_ = [
        ("identifier", ctypes.c_int),
        ("ext_identifier", ctypes.c_uint8),
        ("connector", ctypes.c_uint8),
        ("encoding", ctypes.c_int),
        ("nominal_rate", ctypes.c_uint8),
        ("rate_identifier", ctypes.c_uint8),
        ("smf_length_m", ctypes.c_uint16),
        ("smf_status", ctypes.c_int),
        ("om2_length_m", ctypes.c_uint16),
        ("om2_status", ctypes.c_int),
        ("om1_length_m", ctypes.c_uint16),
        ("om1_status", ctypes.c_int),
        ("om4_or_copper_length_m", ctypes.c_uint16),
        ("om4_or_copper_status", ctypes.c_int),
        ("vendor_name", ctypes.c_char * 17),
        ("ext_compliance", ctypes.c_int),
        ("vendor_oui", ctypes.c_uint8 * 3),
        ("vendor_pn", ctypes.c_char * 17),
        ("vendor_rev", ctypes.c_char * 5),
        ("dummy_media_info", ctypes.c_uint8 * 2),
        ("fc_speed2", ctypes.c_uint8),
        ("cc_base", ctypes.c_uint8),
        ("cc_base_is_valid", ctypes.c_bool),
    ]


# A2h Diagnostics structure (defined before use)
class SfpA2hDiagnostics(ctypes.Structure):
    _fields_ = [
        ("temperature_raw", ctypes.c_int16),
        ("temperature_c", ctypes.c_float),
        ("voltage_raw", ctypes.c_uint16),
        ("voltage_v", ctypes.c_float),
        ("bias_current_raw", ctypes.c_uint16),
        ("bias_current_ma", ctypes.c_float),
        ("tx_power_raw", ctypes.c_uint16),
        ("tx_power_mw", ctypes.c_float),
        ("tx_power_dbm", ctypes.c_float),
        ("rx_power_raw", ctypes.c_uint16),
        ("rx_power_mw", ctypes.c_float),
        ("rx_power_dbm", ctypes.c_float),
        # Alarm flags (simplified - just the main fields)
        ("_alarm_padding", ctypes.c_uint8 * 20),  # Padding for alarm flags structure
        ("temperature_valid", ctypes.c_bool),
        ("voltage_valid", ctypes.c_bool),
        ("bias_current_valid", ctypes.c_bool),
        ("tx_power_valid", ctypes.c_bool),
        ("rx_power_valid", ctypes.c_bool),
    ]


if libsfp is not None:
    libsfp.sfp_i2c_init.argtypes = [ctypes.c_char_p]
    libsfp.sfp_i2c_init.restype = ctypes.c_int

    libsfp.sfp_i2c_close.argtypes = [ctypes.c_int]
    libsfp.sfp_i2c_close.restype = None

    libsfp.sfp_read_block.argtypes = [
        ctypes.c_int,
        ctypes.c_uint8,
        ctypes.c_uint8,
        ctypes.POINTER(ctypes.c_uint8),
        ctypes.c_size_t,
    ]
    libsfp.sfp_read_block.restype = ctypes.c_bool

    libsfp.sfp_parse_a0_base_identifier.argtypes = [
        ctypes.POINTER(ctypes.c_uint8),
        ctypes.POINTER(SfpA0hBase),
    ]
    libsfp.sfp_parse_a0_base_ext_identifier.argtypes = [
        ctypes.POINTER(ctypes.c_uint8),
        ctypes.POINTER(SfpA0hBase),
    ]
    libsfp.sfp_parse_a0_base_connector.argtypes = [
        ctypes.POINTER(ctypes.c_uint8),
        ctypes.POINTER(SfpA0hBase),
    ]
    libsfp.sfp_parse_a0_base_encoding.argtypes = [
        ctypes.POINTER(ctypes.c_uint8),
        ctypes.POINTER(SfpA0hBase),
    ]
    libsfp.sfp_parse_a0_base_smf.argtypes = [
        ctypes.POINTER(ctypes.c_uint8),
        ctypes.POINTER(SfpA0hBase),
    ]
    libsfp.sfp_parse_a0_base_om1.argtypes = [
        ctypes.POINTER(ctypes.c_uint8),
        ctypes.POINTER(SfpA0hBase),
    ]
    libsfp.sfp_parse_a0_base_om2.argtypes = [
        ctypes.POINTER(ctypes.c_uint8),
        ctypes.POINTER(SfpA0hBase),
    ]
    libsfp.sfp_parse_a0_base_om4_or_copper.argtypes = [
        ctypes.POINTER(ctypes.c_uint8),
        ctypes.POINTER(SfpA0hBase),
    ]
    libsfp.sfp_parse_a0_base_ext_compliance.argtypes = [
        ctypes.POINTER(ctypes.c_uint8),
        ctypes.POINTER(SfpA0hBase),
    ]
    libsfp.sfp_parse_a0_base_cc_base.argtypes = [
        ctypes.POINTER(ctypes.c_uint8),
        ctypes.POINTER(SfpA0hBase),
    ]

    # A2h Diagnostics functions
    libsfp.sfp_parse_a2h_diagnostics.argtypes = [
        ctypes.POINTER(ctypes.c_uint8),
        ctypes.POINTER(SfpA2hDiagnostics),
    ]
    libsfp.sfp_parse_a2h_diagnostics.restype = None

    libsfp.sfp_a2h_get_rx_power_dbm.argtypes = [ctypes.POINTER(SfpA2hDiagnostics)]
    libsfp.sfp_a2h_get_rx_power_dbm.restype = ctypes.c_float

    libsfp.sfp_a2h_get_temperature_c.argtypes = [ctypes.POINTER(SfpA2hDiagnostics)]
    libsfp.sfp_a2h_get_temperature_c.restype = ctypes.c_float

    libsfp.sfp_a2h_get_voltage_v.argtypes = [ctypes.POINTER(SfpA2hDiagnostics)]
    libsfp.sfp_a2h_get_voltage_v.restype = ctypes.c_float

    libsfp.sfp_a2h_get_bias_current_ma.argtypes = [ctypes.POINTER(SfpA2hDiagnostics)]
    libsfp.sfp_a2h_get_bias_current_ma.restype = ctypes.c_float


class Base(DeclarativeBase):
    pass


class Measurement(Base):
    __tablename__ = "sfp_measurements"

    id: Mapped[int] = mapped_column(Integer, primary_key=True, autoincrement=True)
    created_at: Mapped[datetime] = mapped_column(
        DateTime(timezone=True), default=lambda: datetime.now(timezone.utc), index=True
    )

    # Identificação do módulo (A0 base)
    identifier: Mapped[Optional[int]] = mapped_column(Integer, nullable=True)
    ext_identifier: Mapped[Optional[int]] = mapped_column(Integer, nullable=True)
    connector: Mapped[Optional[int]] = mapped_column(Integer, nullable=True)
    encoding: Mapped[Optional[int]] = mapped_column(Integer, nullable=True)
    vendor_name: Mapped[Optional[str]] = mapped_column(String(64), nullable=True)
    vendor_pn: Mapped[Optional[str]] = mapped_column(String(64), nullable=True)
    vendor_rev: Mapped[Optional[str]] = mapped_column(String(16), nullable=True)
    cc_base_valid: Mapped[Optional[bool]] = mapped_column(Boolean, nullable=True)

    # Métricas do power meter (A2/diagnósticos ainda não implementados na lib C)
    rx_power_dbm: Mapped[Optional[float]] = mapped_column(Float, nullable=True)
    temperature_c: Mapped[Optional[float]] = mapped_column(Float, nullable=True)
    voltage_v: Mapped[Optional[float]] = mapped_column(Float, nullable=True)
    bias_ma: Mapped[Optional[float]] = mapped_column(Float, nullable=True)

    signal_quality: Mapped[Optional[str]] = mapped_column(String(16), nullable=True)


engine = None
if settings.database_url:
    engine = create_engine(settings.database_url, pool_pre_ping=True)
    Base.metadata.create_all(engine)


class ModuleInfo(BaseModel):
    identifier: Optional[int] = None
    ext_identifier: Optional[int] = None
    connector: Optional[int] = None
    encoding: Optional[int] = None
    vendor_name: Optional[str] = None
    vendor_pn: Optional[str] = None
    vendor_rev: Optional[str] = None
    cc_base_valid: Optional[bool] = None


class CurrentReading(BaseModel):
    timestamp: datetime
    rx_power_dbm: float
    temperature_c: Optional[float] = None
    voltage_v: Optional[float] = None
    bias_ma: Optional[float] = None
    signal_quality: str
    module: ModuleInfo


class HistoryPoint(BaseModel):
    timestamp: datetime
    rx_power_dbm: Optional[float] = None


def _read_a0_base(device: str) -> ModuleInfo:
    if libsfp is None:
        raise RuntimeError(
            "libsfp.so não pôde ser carregada neste ambiente "
            f"(arch atual: {platform.machine()})."
        )
    fd = libsfp.sfp_i2c_init(device.encode())
    if fd < 0:
        raise RuntimeError(f"Falha ao abrir I2C: {device}")
    try:
        buffer = (ctypes.c_uint8 * SFP_A0_BASE_SIZE)()
        ok = libsfp.sfp_read_block(fd, SFP_I2C_ADDR_A0, 0, buffer, SFP_A0_BASE_SIZE)
        if not ok:
            raise RuntimeError("Falha ao ler EEPROM A0h")

        a0 = SfpA0hBase()
        libsfp.sfp_parse_a0_base_identifier(buffer, ctypes.byref(a0))
        libsfp.sfp_parse_a0_base_ext_identifier(buffer, ctypes.byref(a0))
        libsfp.sfp_parse_a0_base_connector(buffer, ctypes.byref(a0))
        libsfp.sfp_parse_a0_base_encoding(buffer, ctypes.byref(a0))
        libsfp.sfp_parse_a0_base_smf(buffer, ctypes.byref(a0))
        libsfp.sfp_parse_a0_base_om1(buffer, ctypes.byref(a0))
        libsfp.sfp_parse_a0_base_om2(buffer, ctypes.byref(a0))
        libsfp.sfp_parse_a0_base_om4_or_copper(buffer, ctypes.byref(a0))
        libsfp.sfp_parse_a0_base_ext_compliance(buffer, ctypes.byref(a0))
        libsfp.sfp_parse_a0_base_cc_base(buffer, ctypes.byref(a0))

        vendor_name = bytes(buffer[20:36]).decode("ascii", errors="ignore").strip()
        vendor_pn = bytes(buffer[40:56]).decode("ascii", errors="ignore").strip()
        vendor_rev = bytes(buffer[56:60]).decode("ascii", errors="ignore").strip()

        return ModuleInfo(
            identifier=int(a0.identifier),
            ext_identifier=int(a0.ext_identifier),
            connector=int(a0.connector),
            encoding=int(a0.encoding),
            vendor_name=vendor_name or None,
            vendor_pn=vendor_pn or None,
            vendor_rev=vendor_rev or None,
            cc_base_valid=bool(a0.cc_base_is_valid),
        )
    finally:
        libsfp.sfp_i2c_close(fd)


_mock_state = {"rx": -8.0}


def _mock_reading(module: Optional[ModuleInfo]) -> CurrentReading:
    # Simula RX power com ruído suave (range típico), e parâmetros básicos.
    current = _mock_state["rx"]
    current = max(-50.0, min(3.0, current + random.uniform(-1.0, 1.0)))
    current = round(current, 2)
    _mock_state["rx"] = current
    return CurrentReading(
        timestamp=datetime.now(timezone.utc),
        rx_power_dbm=current,
        temperature_c=37.0,
        voltage_v=3.33,
        bias_ma=32.0,
        signal_quality="OK",
        module=module or ModuleInfo(),
    )


def _read_a2h_diagnostics(device: str) -> Optional[dict]:
    """Lê diagnósticos da página A2h. Retorna None se falhar."""
    if libsfp is None:
        return None
    fd = libsfp.sfp_i2c_init(device.encode())
    if fd < 0:
        return None
    try:
        buffer = (ctypes.c_uint8 * SFP_A2_DIAG_SIZE)()
        ok = libsfp.sfp_read_block(
            fd, SFP_I2C_ADDR_A2, SFP_A2_DIAG_OFFSET, buffer, SFP_A2_DIAG_SIZE
        )
        if not ok:
            return None

        # Usar a estrutura definida acima
        diag = SfpA2hDiagnostics()
        libsfp.sfp_parse_a2h_diagnostics(buffer, ctypes.byref(diag))

        if not diag.rx_power_valid:
            return None

        return {
            "rx_power_dbm": float(diag.rx_power_dbm),
            "temperature_c": float(diag.temperature_c) if diag.temperature_valid else None,
            "voltage_v": float(diag.voltage_v) if diag.voltage_valid else None,
            "bias_ma": float(diag.bias_current_ma) if diag.bias_current_valid else None,
        }
    except Exception:
        return None
    finally:
        libsfp.sfp_i2c_close(fd)


def get_current_reading() -> CurrentReading:
    module: Optional[ModuleInfo] = None
    diag: Optional[dict] = None

    try:
        module = _read_a0_base(settings.i2c_device)
        diag = _read_a2h_diagnostics(settings.i2c_device)
    except Exception:
        if not settings.enable_mock_when_i2c_fails:
            raise

    # Se temos diagnósticos reais, usar; senão, mock
    if diag:
        return CurrentReading(
            timestamp=datetime.now(timezone.utc),
            rx_power_dbm=diag["rx_power_dbm"],
            temperature_c=diag.get("temperature_c"),
            voltage_v=diag.get("voltage_v"),
            bias_ma=diag.get("bias_ma"),
            signal_quality="OK",
            module=module or ModuleInfo(),
        )
    else:
        return _mock_reading(module)


app = FastAPI(title="Optic Power Meter API")
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)


@app.get("/health")
def health():
    return {
        "ok": True,
        "time": datetime.now(timezone.utc).isoformat(),
        "lib_path": LIB_PATH,
        "db_enabled": bool(engine),
    }


@app.get("/api/v1/current", response_model=CurrentReading)
def api_current():
    reading = get_current_reading()
    if engine:
        with Session(engine) as session:
            m = Measurement(
                created_at=reading.timestamp,
                identifier=reading.module.identifier,
                ext_identifier=reading.module.ext_identifier,
                connector=reading.module.connector,
                encoding=reading.module.encoding,
                vendor_name=reading.module.vendor_name,
                vendor_pn=reading.module.vendor_pn,
                vendor_rev=reading.module.vendor_rev,
                cc_base_valid=reading.module.cc_base_valid,
                rx_power_dbm=reading.rx_power_dbm,
                temperature_c=reading.temperature_c,
                voltage_v=reading.voltage_v,
                bias_ma=reading.bias_ma,
                signal_quality=reading.signal_quality,
            )
            session.add(m)
            session.commit()
    return reading


@app.get("/api/v1/history", response_model=list[HistoryPoint])
def api_history(limit: int = settings.history_default_limit):
    limit = max(1, min(500, int(limit)))
    if not engine:
        # Sem DB: devolve um histórico sintético curto (útil pra dashboard)
        now = datetime.now(timezone.utc).replace(microsecond=0)
        rx = _mock_state["rx"]
        points: list[HistoryPoint] = []
        for i in range(limit):
            rx = max(-50.0, min(3.0, rx + random.uniform(-0.5, 0.5)))
            points.append(
                HistoryPoint(
                    timestamp=now - timedelta(seconds=(limit - 1 - i) * settings.sample_period_seconds),
                    rx_power_dbm=round(rx, 2),
                )
            )
        return points

    with Session(engine) as session:
        rows = session.execute(
            select(Measurement.created_at, Measurement.rx_power_dbm)
            .order_by(Measurement.created_at.desc())
            .limit(limit)
        ).all()
    # Retornar em ordem cronológica
    rows = list(reversed(rows))
    return [
        HistoryPoint(timestamp=created_at, rx_power_dbm=rx_power_dbm)
        for created_at, rx_power_dbm in rows
    ]


if __name__ == "__main__":
    import uvicorn

    uvicorn.run(app, host="0.0.0.0", port=8000)
