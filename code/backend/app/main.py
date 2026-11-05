import asyncio
from typing import List, Dict
from fastapi import FastAPI, WebSocket, WebSocketDisconnect, Depends, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from sqlalchemy.orm import Session

from .database import engine, Base, get_db
from .models import Device, TelemetryRecord, AlarmRecord
from .schemas import CommandPayload
from .mqtt_service import MQTTService

# Auto-create Postgres tables
Base.metadata.create_all(bind=engine)

app = FastAPI(
    title="Smart Infusion Pump IoT Platform",
    description="Real-time monitoring, telemetry logging, and command API for ESP32-S3 Infusion Pumps",
    version="1.0.0"
)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# -------------------------------------------------------------
# WebSocket Manager for Real-time Dashboard Updates
# -------------------------------------------------------------
class ConnectionManager:
    def __init__(self):
        self.active_connections: Dict[str, List[WebSocket]] = {}

    async def connect(self, device_id: str, websocket: WebSocket):
        await websocket.accept()
        if device_id not in self.active_connections:
            self.active_connections[device_id] = []
        self.active_connections[device_id].append(websocket)

    def disconnect(self, device_id: str, websocket: WebSocket):
        if device_id in self.active_connections:
            self.active_connections[device_id].remove(websocket)
            if not self.active_connections[device_id]:
                del self.active_connections[device_id]

    async def broadcast(self, device_id: str, message: dict):
        if device_id in self.active_connections:
            for connection in self.active_connections[device_id]:
                try:
                    await connection.send_json(message)
                except Exception:
                    pass

ws_manager = ConnectionManager()

async def ws_forwarder(device_id: str, data: dict):
    await ws_manager.broadcast(device_id, data)

mqtt_service = MQTTService(ws_broadcast_callback=ws_forwarder)

@app.on_event("startup")
async def startup_event():
    loop = asyncio.get_running_loop()
    mqtt_service.start(loop)

@app.on_event("shutdown")
def shutdown_event():
    mqtt_service.stop()

# -------------------------------------------------------------
# REST Endpoints
# -------------------------------------------------------------
@app.get("/")
def health_check():
    return {"status": "online", "system": "Smart Medical Infusion Pump Gateway"}

@app.get("/api/devices")
def get_devices(db: Session = Depends(get_db)):
    devices = db.query(Device).all()
    return devices

@app.get("/api/devices/{device_id}/telemetry")
def get_recent_telemetry(device_id: str, limit: int = 50, db: Session = Depends(get_db)):
    records = (
        db.query(TelemetryRecord)
        .filter(TelemetryRecord.device_id == device_id)
        .order_by(TelemetryRecord.timestamp.desc())
        .limit(limit)
        .all()
    )
    return records

@app.get("/api/devices/{device_id}/alarms")
def get_device_alarms(device_id: str, limit: int = 20, db: Session = Depends(get_db)):
    alarms = (
        db.query(AlarmRecord)
        .filter(AlarmRecord.device_id == device_id)
        .order_by(AlarmRecord.timestamp.desc())
        .limit(limit)
        .all()
    )
    return alarms

@app.post("/api/devices/{device_id}/command")
def send_command(device_id: str, payload: CommandPayload):
    cmd_data = {
        "command": payload.command,
        "param": payload.param,
        "issued_by": payload.issued_by
    }
    mqtt_service.publish_command(device_id, cmd_data)
    return {"status": "Command dispatched", "target": device_id, "command": cmd_data}

# -------------------------------------------------------------
# WebSocket Endpoint for Live Telemetry Stream
# -------------------------------------------------------------
@app.websocket("/ws/pumps/{device_id}")
async def websocket_endpoint(websocket: WebSocket, device_id: str):
    await ws_manager.connect(device_id, websocket)
    try:
        while True:
            # Keep-alive ping loop
            await websocket.receive_text()
    except WebSocketDisconnect:
        ws_manager.disconnect(device_id, websocket)

