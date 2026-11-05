import asyncio
import logging
from typing import List, Dict
from fastapi import WebSocket

logger = logging.getLogger("websocket_manager")

class WebSocketManager:
    """Manages WebSocket connections grouped by device_id for real-time broadcasting."""

    def __init__(self):
        self.active_connections: Dict[str, List[WebSocket]] = {}

    async def connect(self, device_id: str, websocket: WebSocket):
        await websocket.accept()
        if device_id not in self.active_connections:
            self.active_connections[device_id] = []
        self.active_connections[device_id].append(websocket)
        logger.info(f"WS client connected for device {device_id}. "
                     f"Total: {len(self.active_connections[device_id])}")

    def disconnect(self, device_id: str, websocket: WebSocket):
        if device_id in self.active_connections:
            self.active_connections[device_id].remove(websocket)
            if not self.active_connections[device_id]:
                del self.active_connections[device_id]
            logger.info(f"WS client disconnected from device {device_id}.")

    async def broadcast(self, device_id: str, message: dict):
        """Send a JSON message to all WebSocket clients subscribed to a device."""
        if device_id not in self.active_connections:
            return
        disconnected = []
        for ws in self.active_connections[device_id]:
            try:
                await ws.send_json(message)
            except Exception:
                disconnected.append(ws)
        for ws in disconnected:
            self.disconnect(device_id, ws)

    def get_connection_count(self, device_id: str = None) -> int:
        if device_id:
            return len(self.active_connections.get(device_id, []))
        return sum(len(v) for v in self.active_connections.values())

