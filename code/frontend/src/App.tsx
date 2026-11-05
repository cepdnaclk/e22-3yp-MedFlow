import React, { useState, useEffect } from 'react';
import { TelemetryPacket, PumpState } from './types/telemetry';
import { 
  Activity, 
  AlertTriangle, 
  Play, 
  Pause, 
  RotateCcw, 
  Wifi, 
  Scale, 
  Gauge, 
  CheckCircle2 
} from 'lucide-react';

export default function App() {
  const [device, setDevice] = useState<TelemetryPacket>({
    device_id: 'PUMP-S3-001',
    state: 'INFUSING',
    steps: 12450,
    flow_rate_ml_h: 25.0,
    distance_mm: 36.5,
    force_grams: 480.0,
    limit_switches: { home: false, end: false },
    wifi_rssi: -62
  });

  const [connected, setConnected] = useState(false);
  const OCCLUSION_LIMIT = 1800; // Grams threshold

  useEffect(() => {
    // Connect to WebSocket gateway
    const ws = new WebSocket(`ws://${window.location.hostname}:8000/ws/pumps/PUMP-S3-001`);
    
    ws.onopen = () => setConnected(true);
    ws.onclose = () => setConnected(false);
    ws.onmessage = (event) => {
      try {
        const data = JSON.parse(event.data);
        setDevice((prev) => ({ ...prev, ...data }));
      } catch (err) {
        console.error('Error parsing WS message:', err);
      }
    };

    return () => ws.close();
  }, []);

  const sendCommand = async (cmd: string) => {
    try {
      await fetch(`http://${window.location.hostname}:8000/api/devices/PUMP-S3-001/command`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ command: cmd, issued_by: 'Dashboard UI' })
      });
    } catch (err) {
      console.error('Failed to send command:', err);
    }
  };

  const isAlarm = device.state.startsWith('ALARM');

  return (
    <div className="min-h-screen bg-slate-950 text-slate-100 p-6 font-sans">
      {/* Top Header */}
      <header className="max-w-6xl mx-auto flex items-center justify-between pb-6 border-b border-slate-800">
        <div className="flex items-center gap-3">
          <Activity className="w-8 h-8 text-cyan-400" />
          <div>
            <h1 className="text-2xl font-bold tracking-tight">Smart Infusion Telemetry Station</h1>
            <p className="text-xs text-slate-400">ESP32-S3 Medical IoT Gateway</p>
          </div>
        </div>
        <div className="flex items-center gap-4">
          <div className="flex items-center gap-2 px-3 py-1.5 rounded-full bg-slate-900 border border-slate-700 text-xs">
            <span className={`w-2.5 h-2.5 rounded-full ${connected ? 'bg-emerald-400 animate-pulse' : 'bg-rose-500'}`} />
            {connected ? 'Cloud WS Connected' : 'Connecting to Gateway...'}
          </div>
          <div className="flex items-center gap-1.5 text-xs text-slate-400">
            <Wifi className="w-4 h-4 text-cyan-400" />
            {device.wifi_rssi ? `${device.wifi_rssi} dBm` : 'N/A'}
          </div>
        </div>
      </header>

      {/* Main Container */}
      <main className="max-w-6xl mx-auto py-6 space-y-6">
        {/* Critical Alarm Banner */}
        {isAlarm && (
          <div className="p-4 rounded-xl bg-rose-950/80 border border-rose-600 flex items-center justify-between animate-bounce">
            <div className="flex items-center gap-3">
              <AlertTriangle className="w-6 h-6 text-rose-400" />
              <div>
                <p className="font-bold text-rose-200">SAFETY SHUTDOWN ACTIVE: {device.state}</p>
                <p className="text-xs text-rose-300">Motor step driver halted immediately by ESP32 Core 1 ISR</p>
              </div>
            </div>
            <button 
              onClick={() => sendCommand('PAUSE')}
              className="px-4 py-2 bg-rose-600 hover:bg-rose-700 font-medium rounded-lg text-xs"
            >
              Acknowledge & Clear
            </button>
          </div>
        )}

        {/* Status Metrics Cards */}
        <div className="grid grid-cols-1 md:grid-cols-4 gap-4">
          {/* State Card */}
          <div className="p-4 bg-slate-900/90 rounded-xl border border-slate-800">
            <span className="text-xs text-slate-400 uppercase tracking-wider">Pump Status</span>
            <div className="mt-2 flex items-center justify-between">
              <span className={`text-xl font-bold ${isAlarm ? 'text-rose-400' : 'text-emerald-400'}`}>
                {device.state}
              </span>
              <Activity className="w-5 h-5 text-slate-500" />
            </div>
          </div>

          {/* Occlusion Pressure Card */}
          <div className="p-4 bg-slate-900/90 rounded-xl border border-slate-800">
            <span className="text-xs text-slate-400 uppercase tracking-wider">Occlusion Force</span>
            <div className="mt-2 flex items-baseline justify-between">
              <span className={`text-2xl font-mono font-bold ${device.force_grams >= OCCLUSION_LIMIT ? 'text-rose-400' : 'text-cyan-400'}`}>
                {device.force_grams.toFixed(0)} <span className="text-xs font-normal text-slate-400">g</span>
              </span>
              <span className="text-[10px] text-slate-500">Max: {OCCLUSION_LIMIT}g</span>
            </div>
            {/* Pressure Bar */}
            <div className="w-full bg-slate-800 rounded-full h-1.5 mt-3 overflow-hidden">
              <div 
                className={`h-full ${device.force_grams > OCCLUSION_LIMIT * 0.8 ? 'bg-rose-500' : 'bg-cyan-500'}`}
                style={{ width: `${Math.min(100, (device.force_grams / OCCLUSION_LIMIT) * 100)}%` }}
              />
            </div>
          </div>

          {/* Plunger Distance Card */}
          <div className="p-4 bg-slate-900/90 rounded-xl border border-slate-800">
            <span className="text-xs text-slate-400 uppercase tracking-wider">Plunger Travel (ToF)</span>
            <div className="mt-2 flex items-baseline justify-between">
              <span className="text-2xl font-mono font-bold text-amber-400">
                {device.distance_mm >= 0 ? device.distance_mm.toFixed(1) : 'OOR'}{' '}
                <span className="text-xs font-normal text-slate-400">mm</span>
              </span>
              <Gauge className="w-5 h-5 text-slate-500" />
            </div>
          </div>

          {/* Total Steps */}
          <div className="p-4 bg-slate-900/90 rounded-xl border border-slate-800">
            <span className="text-xs text-slate-400 uppercase tracking-wider">ISR Step Count</span>
            <div className="mt-2 flex items-baseline justify-between">
              <span className="text-2xl font-mono font-bold text-purple-400">
                {device.steps.toLocaleString()}
              </span>
              <span className="text-[10px] text-slate-500">1250 Hz</span>
            </div>
          </div>
        </div>

        {/* Remote Command Bar */}
        <div className="p-6 bg-slate-900/90 rounded-xl border border-slate-800">
          <h2 className="text-sm font-semibold uppercase tracking-wider text-slate-300 mb-4">
            Remote Operations Panel
          </h2>
          <div className="flex flex-wrap gap-4">
            <button
              onClick={() => sendCommand('INFUSE')}
              disabled={isAlarm}
              className="flex items-center gap-2 px-5 py-2.5 bg-emerald-600 hover:bg-emerald-500 disabled:opacity-50 text-white rounded-lg font-medium text-sm transition"
            >
              <Play className="w-4 h-4" /> Start Infusion
            </button>

            <button
              onClick={() => sendCommand('PAUSE')}
              className="flex items-center gap-2 px-5 py-2.5 bg-amber-600 hover:bg-amber-500 text-white rounded-lg font-medium text-sm transition"
            >
              <Pause className="w-4 h-4" /> Pause Motion
            </button>

            <button
              onClick={() => sendCommand('RETRACT')}
              disabled={isAlarm}
              className="flex items-center gap-2 px-5 py-2.5 bg-slate-700 hover:bg-slate-600 disabled:opacity-50 text-white rounded-lg font-medium text-sm transition"
            >
              <RotateCcw className="w-4 h-4" /> Retract Syringe Plunger
            </button>

            <button
              onClick={() => sendCommand('TARE_SCALE')}
              className="flex items-center gap-2 px-5 py-2.5 bg-slate-800 hover:bg-slate-700 text-slate-200 border border-slate-700 rounded-lg font-medium text-sm transition"
            >
              <Scale className="w-4 h-4" /> Tare HX711 Load Cell
            </button>
          </div>
        </div>
      </main>
    </div>
  );
}

