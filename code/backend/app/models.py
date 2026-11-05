import datetime
from sqlalchemy import Column, Integer, String, Float, Boolean, DateTime, ForeignKey, Index
from sqlalchemy.orm import relationship
from .database import Base

class Device(Base):
    __tablename__ = "devices"

    id = Column(String, primary_key=True, index=True) # e.g., "PUMP-S3-001"
    name = Column(String, nullable=False, default="Smart Infusion Pump")
    firmware_version = Column(String, default="1.0.0")
    status = Column(String, default="OFFLINE") # OFFLINE, IDLE, INFUSING, ALARM
    registered_at = Column(DateTime, default=datetime.datetime.utcnow)
    last_seen = Column(DateTime, default=datetime.datetime.utcnow)

    telemetry = relationship("TelemetryRecord", back_populates="device", cascade="all, delete-orphan")
    alarms = relationship("AlarmRecord", back_populates="device", cascade="all, delete-orphan")

class TelemetryRecord(Base):
    __tablename__ = "telemetry_records"

    id = Column(Integer, primary_key=True, autoincrement=True)
    device_id = Column(String, ForeignKey("devices.id"), nullable=False, index=True)
    timestamp = Column(DateTime, default=datetime.datetime.utcnow, index=True)
    state = Column(String, nullable=False) # INFUSING, RETRACTING, IDLE, ALARM
    steps = Column(Integer, default=0)
    force_grams = Column(Float, nullable=False)
    distance_mm = Column(Float, nullable=False)
    flow_rate_ml_h = Column(Float, default=0.0)
    home_switch = Column(Boolean, default=False)
    end_switch = Column(Boolean, default=False)
    wifi_rssi = Column(Integer, nullable=True)

    device = relationship("Device", back_populates="telemetry")

    __table_args__ = (
        Index("idx_device_timestamp", "device_id", "timestamp"),
    )

class AlarmRecord(Base):
    __tablename__ = "alarm_records"

    id = Column(Integer, primary_key=True, autoincrement=True)
    device_id = Column(String, ForeignKey("devices.id"), nullable=False, index=True)
    timestamp = Column(DateTime, default=datetime.datetime.utcnow, index=True)
    alarm_type = Column(String, nullable=False) # OCCLUSION_DETECTED, LIMIT_REACHED_END, LIMIT_REACHED_HOME
    severity = Column(String, default="CRITICAL")
    force_grams = Column(Float, nullable=True)
    message = Column(String, nullable=False)
    resolved = Column(Boolean, default=False)

    device = relationship("Device", back_populates="alarms")

