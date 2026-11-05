from fastapi import APIRouter, Depends, HTTPException
from sqlalchemy.orm import Session
from ...database import get_db
from ...models import Device

router = APIRouter(prefix="/api/v1/devices", tags=["devices"])

@router.get("/")
def list_devices(db: Session = Depends(get_db)):
    """List all registered infusion pump devices."""
    return db.query(Device).all()

@router.get("/{device_id}")
def get_device(device_id: str, db: Session = Depends(get_db)):
    """Get a specific device by its ID."""
    device = db.query(Device).filter(Device.id == device_id).first()
    if not device:
        raise HTTPException(status_code=404, detail=f"Device {device_id} not found")
    return device

@router.post("/")
def register_device(device_id: str, name: str = "Smart Infusion Pump", db: Session = Depends(get_db)):
    """Manually register a new pump device."""
    existing = db.query(Device).filter(Device.id == device_id).first()
    if existing:
        raise HTTPException(status_code=409, detail=f"Device {device_id} already exists")
    device = Device(id=device_id, name=name, status="OFFLINE")
    db.add(device)
    db.commit()
    db.refresh(device)
    return device

@router.delete("/{device_id}")
def delete_device(device_id: str, db: Session = Depends(get_db)):
    """Remove a device and all its telemetry/alarm records (cascade)."""
    device = db.query(Device).filter(Device.id == device_id).first()
    if not device:
        raise HTTPException(status_code=404, detail=f"Device {device_id} not found")
    db.delete(device)
    db.commit()
    return {"status": "deleted", "device_id": device_id}

