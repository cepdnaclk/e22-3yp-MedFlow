import React from 'react';
import { Play, Pause, RotateCcw, Scale } from 'lucide-react';

interface Props {
  isAlarm: boolean;
  onSendCommand: (cmd: string) => void;
}

export default function PumpControlPanel({ isAlarm, onSendCommand }: Props) {
  return (
    <div className="p-6 bg-slate-900/90 rounded-xl border border-slate-800">
      <h2 className="text-sm font-semibold uppercase tracking-wider text-slate-300 mb-4">
        Remote Operations Panel
      </h2>
      <div className="flex flex-wrap gap-4">
        <button
          onClick={() => onSendCommand('INFUSE')}
          disabled={isAlarm}
          className="flex items-center gap-2 px-5 py-2.5 bg-emerald-600 hover:bg-emerald-500 disabled:opacity-40 disabled:cursor-not-allowed text-white rounded-lg font-medium text-sm transition"
        >
          <Play className="w-4 h-4" /> Start Infusion
        </button>

        <button
          onClick={() => onSendCommand('PAUSE')}
          className="flex items-center gap-2 px-5 py-2.5 bg-amber-600 hover:bg-amber-500 text-white rounded-lg font-medium text-sm transition"
        >
          <Pause className="w-4 h-4" /> Pause Motion
        </button>

        <button
          onClick={() => onSendCommand('RETRACT')}
          disabled={isAlarm}
          className="flex items-center gap-2 px-5 py-2.5 bg-slate-700 hover:bg-slate-600 disabled:opacity-40 disabled:cursor-not-allowed text-white rounded-lg font-medium text-sm transition"
        >
          <RotateCcw className="w-4 h-4" /> Retract Syringe
        </button>

        <button
          onClick={() => onSendCommand('TARE_SCALE')}
          className="flex items-center gap-2 px-5 py-2.5 bg-slate-800 hover:bg-slate-700 text-slate-200 border border-slate-700 rounded-lg font-medium text-sm transition"
        >
          <Scale className="w-4 h-4" /> Tare Load Cell
        </button>
      </div>
    </div>
  );
}

