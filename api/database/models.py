from pydantic import BaseModel, Field
from typing import Any, Dict, Optional, List
from datetime import datetime

class ModuleInfo(BaseModel):
    vendor_name: str
    vendor_pn: str
    vendor_rev: str
    vendor_sn: str
    date_code: str
    type: str

class ReadingBase(BaseModel):
    timestamp: str
    created_at: datetime = Field(default_factory=datetime.utcnow)
    rx_power_dbm: float
    temperature_c: Optional[float] = None
    voltage_v: Optional[float] = None
    bias_ma: Optional[float] = None
    signal_quality: Optional[str] = None

class CurrentReading(ReadingBase):
    module: Dict[str, Any]

class HistoryPoint(BaseModel):
    timestamp: str
    rx_power_dbm: Optional[float] = None

class MigrationLog(BaseModel):
    version: int
    description: str
    applied_at: datetime = Field(default_factory=datetime.utcnow)
