from pydantic import BaseModel, Field
from typing import Optional, Dict
from datetime import datetime

class LimitSwitchStatus(BaseModel):
    home: bool = False
    end: bool = False

class TelemetryPayload(BaseModel):
    device_id: str
    timestamp: Optional[float] = None
    state: str
    steps: int
    flow_rate_ml_h: float = 0.0
    distance_mm: float
    force_grams: float
    limit_switches: LimitSwitchStatus
    wifi_rssi: Optional[int] = None

class AlarmPayload(BaseModel):
    device_id: str
    timestamp: Optional[float] = None
    alarm_type: str
    severity: str = "CRITICAL"
    force_grams: Optional[float] = None
    motor_stopped: bool = True
    message: str

class CommandPayload(BaseModel):
    command: str = Field(..., description="INFUSE, PAUSE, RETRACT, TARE_SCALE, or SET_RATE")
    param: Optional[float] = None
    issued_by: Optional[str] = "Admin"

