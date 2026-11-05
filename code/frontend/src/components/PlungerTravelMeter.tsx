import React from 'react';

interface Props {
  distanceMm: number;
}

export default function PlungerTravelMeter({ distanceMm }: Props) {
  // Assume a typical 60mm syringe travel range
  const MAX_TRAVEL = 60;
  const percentage = distanceMm >= 0 ? Math.min(100, (distanceMm / MAX_TRAVEL) * 100) : 0;
  const isOOR = distanceMm < 0;

  return (
    <div className="p-4 bg-slate-900/90 rounded-xl border border-slate-800">
      <h3 className="text-xs text-slate-400 uppercase tracking-wider mb-3">
        Syringe Fluid Level (Plunger Position)
      </h3>

      <div className="flex items-center gap-4">
        {/* Syringe barrel visualization */}
        <div className="flex-1 relative h-10 bg-slate-800 rounded-full overflow-hidden border border-slate-700">
          <div
            className="absolute left-0 top-0 h-full bg-gradient-to-r from-cyan-600 to-cyan-400 rounded-full transition-all duration-300"
            style={{ width: `${100 - percentage}%` }}
          />
          <div className="absolute inset-0 flex items-center justify-center text-xs font-mono font-bold text-white drop-shadow">
            {isOOR ? 'Out of Range' : `${distanceMm.toFixed(1)} mm / ${MAX_TRAVEL} mm`}
          </div>
        </div>

        {/* Percentage readout */}
        <div className="text-right min-w-[60px]">
          <span className="text-2xl font-mono font-bold text-amber-400">
            {isOOR ? '—' : `${(100 - percentage).toFixed(0)}%`}
          </span>
          <p className="text-[10px] text-slate-500">remaining</p>
        </div>
      </div>
    </div>
  );
}

