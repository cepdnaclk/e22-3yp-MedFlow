from fastapi import APIRouter, Depends, Query
from sqlalchemy.orm import Session
from ...database import get_db
from ...models import TelemetryRecord

router = APIRouter(prefix="/api/v1/telemetry", tags=["telemetry"])

@router.get("/{device_id}")
def get_telemetry(
    device_id: str,
    limit: int = Query(50, ge=1, le=1000),
    db: Session = Depends(get_db)
):
    """Retrieve the most recent telemetry records for a device."""
    records = (
        db.query(TelemetryRecord)
        .filter(TelemetryRecord.device_id == device_id)
        .order_by(TelemetryRecord.timestamp.desc())
        .limit(limit)
        .all()
    )
    return records

@router.get("/{device_id}/latest")
def get_latest_telemetry(device_id: str, db: Session = Depends(get_db)):
    """Get the single most recent telemetry reading."""
    record = (
        db.query(TelemetryRecord)
        .filter(TelemetryRecord.device_id == device_id)
        .order_by(TelemetryRecord.timestamp.desc())
        .first()
    )
    if not record:
        return {"message": "No telemetry data available"}
    return record

