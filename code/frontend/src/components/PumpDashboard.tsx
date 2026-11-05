import React from 'react';
import { TelemetryPacket } from '../types/telemetry';
import RealtimeForceChart from './RealtimeForceChart';
import PlungerTravelMeter from './PlungerTravelMeter';
import AlarmBanner from './AlarmBanner';
import PumpControlPanel from './PumpControlPanel';
import { Activity, Gauge, Footprints } from 'lucide-react';

interface Props {
  telemetry: TelemetryPacket;
  connected: boolean;
  forceHistory: number[];
  onSendCommand: (cmd: string) => void;
}

export default function PumpDashboard({ telemetry, connected, forceHistory, onSendCommand }: Props) {
  const isAlarm = telemetry.state.startsWith('ALARM');
  const OCCLUSION_LIMIT = 1800;

  return (
    <div className="space-y-6">
      {/* Alarm Banner */}
      {isAlarm && (
        <AlarmBanner state={telemetry.state} onAcknowledge={() => onSendCommand('PAUSE')} />
      )}

      {/* Metric Cards Row */}
      <div className="grid grid-cols-1 md:grid-cols-4 gap-4">
        {/* State */}
        <div className="p-4 bg-slate-900/90 rounded-xl border border-slate-800">
          <span className="text-xs text-slate-400 uppercase tracking-wider">Pump Status</span>
          <div className="mt-2 flex items-center justify-between">
            <span className={`text-xl font-bold ${isAlarm ? 'text-rose-400' : 'text-emerald-400'}`}>
              {telemetry.state}
            </span>
            <Activity className="w-5 h-5 text-slate-500" />
          </div>
        </div>

        {/* Force */}
        <div className="p-4 bg-slate-900/90 rounded-xl border border-slate-800">
          <span className="text-xs text-slate-400 uppercase tracking-wider">Occlusion Force</span>
          <div className="mt-2">
            <span className={`text-2xl font-mono font-bold ${telemetry.force_grams >= OCCLUSION_LIMIT ? 'text-rose-400' : 'text-cyan-400'}`}>
              {telemetry.force_grams.toFixed(0)}
              <span className="text-xs font-normal text-slate-400 ml-1">g</span>
            </span>
          </div>
          <div className="w-full bg-slate-800 rounded-full h-1.5 mt-3 overflow-hidden">
            <div
              className={`h-full ${telemetry.force_grams > OCCLUSION_LIMIT * 0.8 ? 'bg-rose-500' : 'bg-cyan-500'}`}
              style={{ width: `${Math.min(100, (telemetry.force_grams / OCCLUSION_LIMIT) * 100)}%` }}
            />
          </div>
        </div>

        {/* Distance */}
        <div className="p-4 bg-slate-900/90 rounded-xl border border-slate-800">
          <span className="text-xs text-slate-400 uppercase tracking-wider">Plunger Travel</span>
          <div className="mt-2 flex items-baseline justify-between">
            <span className="text-2xl font-mono font-bold text-amber-400">
              {telemetry.distance_mm >= 0 ? telemetry.distance_mm.toFixed(1) : 'OOR'}
              <span className="text-xs font-normal text-slate-400 ml-1">mm</span>
            </span>
            <Gauge className="w-5 h-5 text-slate-500" />
          </div>
        </div>

        {/* Steps */}
        <div className="p-4 bg-slate-900/90 rounded-xl border border-slate-800">
          <span className="text-xs text-slate-400 uppercase tracking-wider">Step Count</span>
          <div className="mt-2 flex items-baseline justify-between">
            <span className="text-2xl font-mono font-bold text-purple-400">
              {telemetry.steps.toLocaleString()}
            </span>
            <Footprints className="w-5 h-5 text-slate-500" />
          </div>
        </div>
      </div>

      {/* Charts Row */}
      <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
        <RealtimeForceChart forceHistory={forceHistory} limit={OCCLUSION_LIMIT} />
        <PlungerTravelMeter distanceMm={telemetry.distance_mm} />
      </div>

      {/* Control Panel */}
      <PumpControlPanel isAlarm={isAlarm} onSendCommand={onSendCommand} />
    </div>
  );
}

