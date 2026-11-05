export interface LimitSwitches {
  home: boolean;
  end: boolean;
}

export type PumpState =
  | 'IDLE'
  | 'INFUSING'
  | 'RETRACTING'
  | 'ALARM_OCCLUSION'
  | 'ALARM_LIMIT_END'
  | 'ALARM_LIMIT_HOME';

export interface TelemetryPacket {
  device_id: string;
  state: PumpState;
  steps: number;
  flow_rate_ml_h?: number;
  distance_mm: number;
  force_grams: number;
  limit_switches: LimitSwitches;
  wifi_rssi?: number;
  timestamp?: number;
}

export interface AlarmEvent {
  device_id: string;
  alarm_type: string;
  severity: 'INFO' | 'WARNING' | 'CRITICAL';
  force_grams?: number;
  motor_stopped: boolean;
  message: string;
  timestamp?: number;
}

