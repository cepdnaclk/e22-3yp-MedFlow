import React from 'react';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, ReferenceLine, ResponsiveContainer } from 'recharts';

interface Props {
  forceHistory: number[];
  limit: number;
}

export default function RealtimeForceChart({ forceHistory, limit }: Props) {
  const data = forceHistory.map((val, idx) => ({ index: idx, force: val }));

  return (
    <div className="p-4 bg-slate-900/90 rounded-xl border border-slate-800">
      <h3 className="text-xs text-slate-400 uppercase tracking-wider mb-3">
        Occlusion Force Waveform (Live)
      </h3>
      <ResponsiveContainer width="100%" height={200}>
        <LineChart data={data}>
          <CartesianGrid strokeDasharray="3 3" stroke="#334155" />
          <XAxis dataKey="index" tick={false} />
          <YAxis domain={[0, limit * 1.2]} stroke="#64748b" fontSize={10} />
          <ReferenceLine
            y={limit}
            stroke="#ef4444"
            strokeDasharray="5 5"
            label={{ value: `${limit}g LIMIT`, fill: '#ef4444', fontSize: 10 }}
          />
          <Line
            type="monotone"
            dataKey="force"
            stroke="#22d3ee"
            strokeWidth={2}
            dot={false}
            isAnimationActive={false}
          />
        </LineChart>
      </ResponsiveContainer>
    </div>
  );
}

