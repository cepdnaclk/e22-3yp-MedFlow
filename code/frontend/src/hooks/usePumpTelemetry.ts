import { useState, useEffect, useRef } from 'react';
import { TelemetryPacket } from '../types/telemetry';
import { useWebSocket } from './useWebSocket';

const MAX_HISTORY = 100; // Keep last 100 force readings for the chart

const DEFAULT_TELEMETRY: TelemetryPacket = {
  device_id: 'PUMP-S3-001',
  state: 'IDLE',
  steps: 0,
  flow_rate_ml_h: 0,
  distance_mm: 0,
  force_grams: 0,
  limit_switches: { home: false, end: false },
  wifi_rssi: 0
};

/**
 * Hook that connects to the pump's WebSocket stream and
 * maintains both the latest telemetry snapshot and a rolling
 * history of force readings for charting.
 */
export function usePumpTelemetry(deviceId: string) {
  const wsUrl = `ws://${window.location.hostname}:8000/ws/pumps/${deviceId}`;
  const { connected, lastMessage } = useWebSocket(wsUrl);

  const [telemetry, setTelemetry] = useState<TelemetryPacket>(DEFAULT_TELEMETRY);
  const [forceHistory, setForceHistory] = useState<number[]>([]);
  const historyRef = useRef<number[]>([]);

  useEffect(() => {
    if (!lastMessage) return;

    setTelemetry((prev) => ({ ...prev, ...lastMessage }));

    const force = lastMessage.force_grams ?? 0;
    historyRef.current = [...historyRef.current.slice(-(MAX_HISTORY - 1)), force];
    setForceHistory([...historyRef.current]);
  }, [lastMessage]);

  return { telemetry, forceHistory, connected };
}

