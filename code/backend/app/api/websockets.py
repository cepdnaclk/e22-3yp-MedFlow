from fastapi import APIRouter, WebSocket, WebSocketDisconnect
from ..services.websocket_manager import WebSocketManager

router = APIRouter()
ws_manager = WebSocketManager()

@router.websocket("/ws/telemetry/{pump_id}")
async def pump_telemetry_ws(websocket: WebSocket, pump_id: str):
    """
    WebSocket endpoint for real-time telemetry streaming.
    Clients connect to /ws/telemetry/{pump_id} and receive JSON
    telemetry packets as they arrive from the MQTT bridge.
    """
    await ws_manager.connect(pump_id, websocket)
    try:
        while True:
            # Keep the connection alive — client can also send pings
            await websocket.receive_text()
    except WebSocketDisconnect:
        ws_manager.disconnect(pump_id, websocket)

