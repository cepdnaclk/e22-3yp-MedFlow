import json
import logging
import asyncio
import os
import paho.mqtt.client as mqtt
from datetime import datetime
from .database import SessionLocal
from .models import Device, TelemetryRecord, AlarmRecord

logger = logging.getLogger("mqtt_service")
logging.basicConfig(level=logging.INFO)

MQTT_BROKER_HOST = os.getenv("MQTT_BROKER_HOST", "localhost")
MQTT_BROKER_PORT = int(os.getenv("MQTT_BROKER_PORT", 1883))

class MQTTService:
    def __init__(self, ws_broadcast_callback=None):
        self.client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
        self.ws_broadcast_callback = ws_broadcast_callback
        self.loop = None

        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message

    def on_connect(self, client, userdata, flags, rc, properties=None):
        logger.info(f"Connected to MQTT Broker with code {rc}")
        # Subscribe to all telemetry and alarm topics
        client.subscribe("pumps/+/telemetry")
        client.subscribe("pumps/+/alarms")
        logger.info("Subscribed to 'pumps/+/telemetry' and 'pumps/+/alarms'")

    def on_message(self, client, userdata, msg):
        try:
            payload = json.loads(msg.payload.decode())
            topic = msg.topic
            logger.info(f"Received MQTT msg on {topic}")

            device_id = payload.get("device_id")
            if not device_id:
                return

            db = SessionLocal()
            try:
                # Ensure device exists
                device = db.query(Device).filter(Device.id == device_id).first()
                if not device:
                    device = Device(id=device_id, name=f"Pump {device_id}", status="ONLINE")
                    db.add(device)
                    db.commit()
                else:
                    device.last_seen = datetime.utcnow()
                    device.status = payload.get("state", "ONLINE")

                if topic.endswith("/telemetry"):
                    limits = payload.get("limit_switches", {})
                    record = TelemetryRecord(
                        device_id=device_id,
                        state=payload.get("state", "UNKNOWN"),
                        steps=payload.get("steps", 0),
                        force_grams=payload.get("force_grams", 0.0),
                        distance_mm=payload.get("distance_mm", 0.0),
                        flow_rate_ml_h=payload.get("flow_rate_ml_h", 0.0),
                        home_switch=limits.get("home", False),
                        end_switch=limits.get("end", False),
                        wifi_rssi=payload.get("wifi_rssi")
                    )
                    db.add(record)
                    db.commit()

                elif topic.endswith("/alarms"):
                    device.status = "ALARM"
                    alarm = AlarmRecord(
                        device_id=device_id,
                        alarm_type=payload.get("alarm_type", "UNKNOWN_ALARM"),
                        severity=payload.get("severity", "CRITICAL"),
                        force_grams=payload.get("force_grams"),
                        message=payload.get("message", "Alarm triggered")
                    )
                    db.add(alarm)
                    db.commit()

            finally:
                db.close()

            # Broadcast to real-time WebSockets
            if self.ws_broadcast_callback and self.loop:
                asyncio.run_coroutine_threadsafe(
                    self.ws_broadcast_callback(device_id, payload),
                    self.loop
                )

        except Exception as e:
            logger.error(f"Error processing MQTT message: {e}")

    def publish_command(self, device_id: str, command_data: dict):
        topic = f"pumps/{device_id}/cmd"
        self.client.publish(topic, json.dumps(command_data), qos=1)
        logger.info(f"Published command to {topic}: {command_data}")

    def start(self, event_loop):
        self.loop = event_loop
        try:
            self.client.connect(MQTT_BROKER_HOST, MQTT_BROKER_PORT, 60)
            self.client.loop_start()
            logger.info("MQTT loop started successfully.")
        except Exception as e:
            logger.warning(f"Could not connect to MQTT broker ({e}). Will retry...")

    def stop(self):
        self.client.loop_stop()
        self.client.disconnect()

