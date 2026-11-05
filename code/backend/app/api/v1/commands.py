from fastapi import APIRouter
from ...schemas import CommandPayload

router = APIRouter(prefix="/api/v1/commands", tags=["commands"])

# Note: The mqtt_service instance is injected at app startup in main.py.
# This module provides the route definition; the actual publish call
# is wired in main.py where the MQTTService singleton is available.

@router.post("/{device_id}")
def send_pump_command(device_id: str, payload: CommandPayload):
    """
    Dispatch a control command to an infusion pump via MQTT.
    Valid commands: INFUSE, PAUSE, RETRACT, TARE_SCALE, SET_RATE
    """
    # This is a template — in production, import and call:
    #   mqtt_service.publish_command(device_id, payload.dict())
    return {
        "status": "Command route registered",
        "device_id": device_id,
        "command": payload.command,
        "note": "Wire mqtt_service.publish_command() in main.py"
    }

