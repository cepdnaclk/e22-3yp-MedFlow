import React from 'react';
import { AlertTriangle } from 'lucide-react';
import { PumpState } from '../types/telemetry';

interface Props {
  state: PumpState;
  onAcknowledge: () => void;
}

export default function AlarmBanner({ state, onAcknowledge }: Props) {
  const messages: Record<string, string> = {
    ALARM_OCCLUSION: 'OCCLUSION DETECTED — Back-pressure exceeded 1800g safety threshold. Motor halted by Core 1 ISR.',
    ALARM_LIMIT_END: 'END-OF-TRAVEL LIMIT HIT — Syringe fully dispensed. Motor halted.',
    ALARM_LIMIT_HOME: 'HOME LIMIT HIT — Carriage fully retracted. Motor halted.'
  };

  return (
    <div className="p-4 rounded-xl bg-rose-950/80 border-2 border-rose-600 animate-pulse flex items-center justify-between">
      <div className="flex items-center gap-3">
        <AlertTriangle className="w-7 h-7 text-rose-400 shrink-0" />
        <div>
          <p className="font-bold text-rose-200 text-lg">{state.replace(/_/g, ' ')}</p>
          <p className="text-sm text-rose-300 mt-0.5">{messages[state] || 'Safety alarm triggered.'}</p>
        </div>
      </div>
      <button
        onClick={onAcknowledge}
        className="px-5 py-2.5 bg-rose-600 hover:bg-rose-700 active:bg-rose-800 font-semibold rounded-lg text-sm text-white transition whitespace-nowrap"
      >
        Acknowledge &amp; Clear
      </button>
    </div>
  );
}

