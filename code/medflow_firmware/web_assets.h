#pragma once

// ============================================================
//  MedFlow — Embedded Web Dashboard
//  Served from ESP32 PROGMEM via ESPAsyncWebServer
//  Single-file HTML + CSS + JS, no external dependencies
//  except Chart.js loaded from CDN (cached by browser).
//
//  WebSocket endpoint : ws://192.168.4.1/ws
//  Telemetry rate     : 10 Hz (every 100 ms from ESP32)
// ============================================================

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>MedFlow — Infusion Monitor</title>
  <meta name="description" content="Real-time monitoring dashboard for the MedFlow smart syringe infusion pump." />

  <!-- Chart.js — loaded once and cached by the browser -->
  <script src="https://cdn.jsdelivr.net/npm/chart.js@4.4.3/dist/chart.umd.min.js"></script>

  <style>
    /* ── Design tokens ──────────────────────────────────────── */
    :root {
      --bg:          #080e1a;
      --surface:     #0e1929;
      --card:        #131f30;
      --border:      #1e3048;
      --text:        #e2eaf5;
      --muted:       #6b83a0;
      --accent:      #3b82f6;
      --accent-glow: rgba(59,130,246,0.25);
      --green:       #22c55e;
      --amber:       #f59e0b;
      --red:         #ef4444;
      --red-glow:    rgba(239,68,68,0.35);
      --font:        'Segoe UI', system-ui, sans-serif;
      --radius:      12px;
      --radius-sm:   8px;
    }

    /* ── Reset & base ───────────────────────────────────────── */
    *, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

    body {
      font-family: var(--font);
      background: var(--bg);
      color: var(--text);
      min-height: 100vh;
      padding: 0;
      overflow-x: hidden;
    }

    /* ── Top navigation bar ─────────────────────────────────── */
    .topbar {
      display: flex;
      align-items: center;
      justify-content: space-between;
      padding: 0 28px;
      height: 60px;
      background: var(--surface);
      border-bottom: 1px solid var(--border);
      position: sticky;
      top: 0;
      z-index: 100;
    }

    .topbar-brand {
      display: flex;
      align-items: center;
      gap: 10px;
      font-weight: 700;
      font-size: 1.1rem;
      letter-spacing: 0.02em;
      color: var(--text);
    }

    .topbar-brand .pulse-dot {
      width: 10px; height: 10px;
      border-radius: 50%;
      background: var(--green);
      box-shadow: 0 0 8px var(--green);
      animation: pulse 2s ease-in-out infinite;
    }

    .topbar-brand .pulse-dot.offline {
      background: var(--red);
      box-shadow: 0 0 8px var(--red);
      animation: none;
    }

    @keyframes pulse {
      0%, 100% { opacity: 1; transform: scale(1); }
      50%       { opacity: 0.5; transform: scale(0.8); }
    }

    .topbar-right {
      display: flex;
      align-items: center;
      gap: 18px;
      font-size: 0.82rem;
      color: var(--muted);
    }

    .ws-badge {
      padding: 3px 10px;
      border-radius: 20px;
      font-size: 0.75rem;
      font-weight: 600;
      letter-spacing: 0.05em;
      background: rgba(34,197,94,0.15);
      color: var(--green);
      border: 1px solid rgba(34,197,94,0.3);
      transition: all 0.3s;
    }

    .ws-badge.connecting {
      background: rgba(245,158,11,0.15);
      color: var(--amber);
      border-color: rgba(245,158,11,0.3);
    }

    .ws-badge.disconnected {
      background: rgba(239,68,68,0.15);
      color: var(--red);
      border-color: rgba(239,68,68,0.3);
    }

    /* ── Alarm banner ───────────────────────────────────────── */
    .alarm-banner {
      display: none;
      align-items: center;
      justify-content: center;
      gap: 12px;
      padding: 14px 24px;
      font-weight: 700;
      font-size: 1rem;
      letter-spacing: 0.04em;
      text-transform: uppercase;
      animation: bannerFlash 1s ease-in-out infinite alternate;
    }

    .alarm-banner.active { display: flex; }

    .alarm-banner.critical {
      background: linear-gradient(90deg, rgba(239,68,68,0.25), rgba(239,68,68,0.12), rgba(239,68,68,0.25));
      border-bottom: 2px solid var(--red);
      color: #fca5a5;
    }

    .alarm-banner.warning {
      background: linear-gradient(90deg, rgba(245,158,11,0.2), rgba(245,158,11,0.08), rgba(245,158,11,0.2));
      border-bottom: 2px solid var(--amber);
      color: #fcd34d;
    }

    @keyframes bannerFlash {
      from { opacity: 1; }
      to   { opacity: 0.7; }
    }

    .alarm-icon { font-size: 1.4rem; }

    /* ── Page layout ────────────────────────────────────────── */
    .page { padding: 24px 28px; max-width: 1300px; margin: 0 auto; }

    .grid {
      display: grid;
      gap: 20px;
    }

    .grid-top   { grid-template-columns: repeat(4, 1fr); }
    .grid-mid   { grid-template-columns: 2fr 1fr; }
    .grid-bot   { grid-template-columns: 1fr; }

    /* ── Card ───────────────────────────────────────────────── */
    .card {
      background: var(--card);
      border: 1px solid var(--border);
      border-radius: var(--radius);
      padding: 20px;
      transition: border-color 0.3s;
    }

    .card:hover { border-color: #2a4060; }

    .card-title {
      font-size: 0.7rem;
      font-weight: 600;
      letter-spacing: 0.1em;
      text-transform: uppercase;
      color: var(--muted);
      margin-bottom: 10px;
    }

    /* ── KPI metric tiles ───────────────────────────────────── */
    .kpi-value {
      font-size: 2.2rem;
      font-weight: 800;
      line-height: 1;
      color: var(--text);
      font-variant-numeric: tabular-nums;
      letter-spacing: -0.02em;
    }

    .kpi-unit {
      font-size: 0.8rem;
      color: var(--muted);
      margin-top: 4px;
    }

    .kpi-sub {
      font-size: 0.78rem;
      color: var(--muted);
      margin-top: 6px;
    }

    /* State badge colouring */
    #kpiState { font-size: 1.5rem; }
    #kpiState.INFUSING  { color: var(--green); }
    #kpiState.PAUSED    { color: var(--amber); }
    #kpiState.IDLE      { color: var(--muted); }
    #kpiState.alarm     { color: var(--red); }

    /* Battery bar */
    .batt-bar-wrap {
      background: var(--border);
      border-radius: 4px;
      height: 6px;
      margin-top: 8px;
      overflow: hidden;
    }

    .batt-bar-fill {
      height: 100%;
      border-radius: 4px;
      background: var(--green);
      transition: width 1s, background 0.5s;
    }

    /* Progress ring */
    .ring-wrap {
      display: flex;
      align-items: center;
      justify-content: center;
      margin: 8px 0 12px;
    }

    svg.ring { transform: rotate(-90deg); }
    .ring-track  { fill: none; stroke: var(--border); stroke-width: 8; }
    .ring-fill   { fill: none; stroke: var(--accent); stroke-width: 8;
                   stroke-linecap: round;
                   stroke-dasharray: 283;
                   stroke-dashoffset: 283;
                   transition: stroke-dashoffset 0.8s ease; }
    .ring-label  {
      transform: rotate(90deg);
      font-size: 0.9rem;
      font-weight: 700;
      fill: var(--text);
      text-anchor: middle;
      dominant-baseline: middle;
    }

    /* ── Chart area ─────────────────────────────────────────── */
    .chart-wrap { position: relative; height: 240px; }

    /* ── Controls panel ─────────────────────────────────────── */
    .ctrl-section { margin-bottom: 18px; }

    .ctrl-label {
      font-size: 0.72rem;
      font-weight: 600;
      letter-spacing: 0.08em;
      text-transform: uppercase;
      color: var(--muted);
      margin-bottom: 8px;
    }

    .rate-input-wrap {
      display: flex;
      align-items: center;
      gap: 8px;
    }

    input[type="number"] {
      flex: 1;
      background: var(--surface);
      border: 1px solid var(--border);
      color: var(--text);
      border-radius: var(--radius-sm);
      padding: 10px 14px;
      font-size: 1.1rem;
      font-family: var(--font);
      outline: none;
      transition: border-color 0.2s;
    }

    input[type="number"]:focus { border-color: var(--accent); }

    .unit-label {
      font-size: 0.82rem;
      color: var(--muted);
      white-space: nowrap;
    }

    .btn-row {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 10px;
      margin-top: 10px;
    }

    .btn-row-3 { grid-template-columns: 1fr 1fr 1fr; }

    button {
      font-family: var(--font);
      font-size: 0.88rem;
      font-weight: 700;
      letter-spacing: 0.04em;
      border: none;
      border-radius: var(--radius-sm);
      padding: 11px 14px;
      cursor: pointer;
      transition: opacity 0.2s, transform 0.1s, box-shadow 0.2s;
      text-transform: uppercase;
    }

    button:active { transform: scale(0.97); }
    button:disabled { opacity: 0.38; cursor: not-allowed; transform: none; }

    .btn-start {
      background: linear-gradient(135deg, #16a34a, #22c55e);
      color: #fff;
      box-shadow: 0 4px 14px rgba(34,197,94,0.25);
    }

    .btn-pause {
      background: linear-gradient(135deg, #b45309, #f59e0b);
      color: #fff;
    }

    .btn-stop {
      background: linear-gradient(135deg, #b91c1c, #ef4444);
      color: #fff;
      box-shadow: 0 4px 14px rgba(239,68,68,0.25);
      grid-column: 1 / -1;
    }

    .btn-set {
      background: linear-gradient(135deg, #1d4ed8, #3b82f6);
      color: #fff;
    }

    /* ── Alarm log ──────────────────────────────────────────── */
    .alarm-log {
      max-height: 180px;
      overflow-y: auto;
      display: flex;
      flex-direction: column;
      gap: 6px;
    }

    .log-entry {
      display: flex;
      align-items: baseline;
      gap: 10px;
      padding: 7px 10px;
      border-radius: 6px;
      font-size: 0.8rem;
      background: rgba(255,255,255,0.03);
      border-left: 3px solid transparent;
      animation: slideIn 0.3s ease;
    }

    @keyframes slideIn {
      from { opacity: 0; transform: translateX(-10px); }
      to   { opacity: 1; transform: translateX(0); }
    }

    .log-entry.critical { border-color: var(--red);   color: #fca5a5; }
    .log-entry.warning  { border-color: var(--amber);  color: #fcd34d; }
    .log-entry.info     { border-color: var(--accent);  color: #93c5fd; }

    .log-time { font-size: 0.7rem; color: var(--muted); white-space: nowrap; }

    .log-empty {
      text-align: center;
      color: var(--muted);
      font-size: 0.82rem;
      padding: 20px;
    }

    /* ── Confirm dialog overlay ─────────────────────────────── */
    .overlay {
      display: none;
      position: fixed;
      inset: 0;
      background: rgba(0,0,0,0.7);
      backdrop-filter: blur(4px);
      z-index: 200;
      align-items: center;
      justify-content: center;
    }

    .overlay.open { display: flex; }

    .dialog {
      background: var(--card);
      border: 1px solid var(--border);
      border-radius: var(--radius);
      padding: 28px 32px;
      max-width: 380px;
      width: 90%;
      box-shadow: 0 20px 60px rgba(0,0,0,0.6);
    }

    .dialog h3 { margin-bottom: 10px; font-size: 1rem; }
    .dialog p  { font-size: 0.88rem; color: var(--muted); margin-bottom: 20px; line-height: 1.5; }

    .dialog-btns { display: flex; gap: 10px; }
    .dialog-btns button { flex: 1; }

    .btn-confirm {
      background: linear-gradient(135deg, #16a34a, #22c55e);
      color: #fff;
    }

    .btn-cancel {
      background: var(--surface);
      color: var(--text);
      border: 1px solid var(--border);
    }

    /* ── Responsive ─────────────────────────────────────────── */
    @media (max-width: 900px) {
      .grid-top { grid-template-columns: repeat(2, 1fr); }
      .grid-mid { grid-template-columns: 1fr; }
      .topbar { padding: 0 16px; }
      .page   { padding: 16px; }
    }

    @media (max-width: 500px) {
      .grid-top { grid-template-columns: 1fr 1fr; }
      .kpi-value { font-size: 1.7rem; }
    }
  </style>
</head>

<body>

<!-- ── Top bar ───────────────────────────────────────────────────── -->
<header class="topbar">
  <div class="topbar-brand">
    <div class="pulse-dot" id="wsDot"></div>
    <span>MedFlow</span>
    <span style="font-weight:400;color:var(--muted);font-size:0.85rem;">Infusion Monitor</span>
  </div>
  <div class="topbar-right">
    <span id="clockDisplay">--:--:--</span>
    <span class="ws-badge connecting" id="wsBadge">CONNECTING…</span>
  </div>
</header>

<!-- ── Alarm banner ──────────────────────────────────────────────── -->
<div class="alarm-banner" id="alarmBanner">
  <span class="alarm-icon" id="alarmIcon">⚠</span>
  <span id="alarmText">ALARM</span>
</div>

<!-- ── Dashboard ─────────────────────────────────────────────────── -->
<main class="page">

  <!-- Row 1: KPI tiles -->
  <div class="grid grid-top" style="margin-bottom:20px;">

    <!-- State -->
    <div class="card">
      <div class="card-title">Pump State</div>
      <div class="kpi-value IDLE" id="kpiState">IDLE</div>
      <div class="kpi-sub" id="kpiSyringe">No syringe profile</div>
    </div>

    <!-- Flow rate -->
    <div class="card">
      <div class="card-title">Flow Rate</div>
      <div class="kpi-value" id="kpiFlow">0.0</div>
      <div class="kpi-unit">mL / hr</div>
    </div>

    <!-- Delivered + progress ring -->
    <div class="card">
      <div class="card-title">Volume Delivered</div>
      <div style="display:flex;align-items:center;gap:14px;">
        <div>
          <div class="kpi-value" id="kpiVol">0.00</div>
          <div class="kpi-unit">mL</div>
          <div class="kpi-sub">of <span id="kpiTarget">—</span> mL target</div>
        </div>
        <div class="ring-wrap">
          <svg class="ring" width="64" height="64" viewBox="0 0 100 100">
            <circle class="ring-track" cx="50" cy="50" r="45"/>
            <circle class="ring-fill"  cx="50" cy="50" r="45" id="ringFill"/>
            <text class="ring-label" x="50" y="50" id="ringLabel">0%</text>
          </svg>
        </div>
      </div>
    </div>

    <!-- Battery -->
    <div class="card">
      <div class="card-title">Battery</div>
      <div class="kpi-value" id="kpiBatt">—</div>
      <div class="kpi-unit">Volts</div>
      <div class="batt-bar-wrap">
        <div class="batt-bar-fill" id="battBar" style="width:0%"></div>
      </div>
      <div class="kpi-sub" id="kpiBattLabel">—</div>
    </div>

  </div>

  <!-- Row 2: Graph + Controls -->
  <div class="grid grid-mid" style="margin-bottom:20px;">

    <!-- Occlusion force graph -->
    <div class="card">
      <div class="card-title">Occlusion Force — Live Waveform</div>
      <div class="chart-wrap">
        <canvas id="forceChart"></canvas>
      </div>
      <div style="display:flex;justify-content:space-between;margin-top:10px;font-size:0.78rem;color:var(--muted);">
        <span>Latest: <b id="forceNow" style="color:var(--text);">— N</b></span>
        <span>Limit: <b id="forceLimit" style="color:var(--red);">— N</b></span>
      </div>
    </div>

    <!-- Controls -->
    <div class="card">
      <div class="card-title" style="margin-bottom:16px;">Pump Controls</div>

      <!-- Rate input -->
      <div class="ctrl-section">
        <div class="ctrl-label">Target Rate</div>
        <div class="rate-input-wrap">
          <input type="number" id="inRate" min="0.1" max="150" step="0.5" value="25.0" />
          <span class="unit-label">mL / hr</span>
        </div>
        <div class="btn-row" style="grid-template-columns:1fr;">
          <button class="btn-set" id="btnSetRate" onclick="cmdSetRate()">Set Rate</button>
        </div>
      </div>

      <!-- Action buttons -->
      <div class="ctrl-section">
        <div class="ctrl-label">Actions</div>
        <div class="btn-row">
          <button class="btn-start" id="btnStart" onclick="cmdStart()">▶ Start</button>
          <button class="btn-pause" id="btnPause" onclick="cmdSend('PAUSE')">⏸ Pause</button>
        </div>
        <div class="btn-row">
          <button class="btn-stop"  id="btnStop"  onclick="cmdStop()">■ Emergency Stop</button>
        </div>
      </div>

      <!-- Force threshold display -->
      <div class="ctrl-section">
        <div class="ctrl-label">Occlusion Threshold</div>
        <div style="font-size:1.3rem;font-weight:700;color:var(--red);" id="dispThreshold">—</div>
        <div class="kpi-unit">Newtons (set on pump)</div>
      </div>
    </div>

  </div>

  <!-- Row 3: Alarm log -->
  <div class="grid grid-bot">
    <div class="card">
      <div class="card-title" style="margin-bottom:12px;">Event Log</div>
      <div class="alarm-log" id="alarmLog">
        <div class="log-empty">No events recorded this session.</div>
      </div>
    </div>
  </div>

</main>

<!-- ── Confirm dialog ─────────────────────────────────────────────── -->
<div class="overlay" id="overlay">
  <div class="dialog">
    <h3 id="dialogTitle">Confirm Action</h3>
    <p id="dialogBody">Are you sure?</p>
    <div class="dialog-btns">
      <button class="btn-cancel" onclick="closeDialog()">Cancel</button>
      <button class="btn-confirm" id="dialogConfirm">Confirm</button>
    </div>
  </div>
</div>

<!-- ── Application script ─────────────────────────────────────────── -->
<script>
// ── Clock ────────────────────────────────────────────────────────────
(function clockTick() {
  document.getElementById('clockDisplay').textContent =
    new Date().toLocaleTimeString('en-GB');
  setTimeout(clockTick, 1000);
})();

// ── WebSocket ─────────────────────────────────────────────────────────
const WS_URL = `ws://${location.hostname}/ws`;
let ws, reconnectTimer, txId = 1;

function connect() {
  ws = new WebSocket(WS_URL);

  ws.onopen = () => {
    setWsStatus('connected');
    clearTimeout(reconnectTimer);
    logEvent('info', 'WebSocket connected to pump.');
  };

  ws.onclose = () => {
    setWsStatus('disconnected');
    logEvent('warning', 'WebSocket disconnected — retrying in 3 s…');
    reconnectTimer = setTimeout(connect, 3000);
  };

  ws.onerror = () => setWsStatus('disconnected');

  ws.onmessage = (e) => {
    try { handleTelemetry(JSON.parse(e.data)); }
    catch (_) {}
  };
}

function setWsStatus(s) {
  const badge = document.getElementById('wsBadge');
  const dot   = document.getElementById('wsDot');
  const map = {
    connected:    ['LIVE',         'ws-badge',             false],
    connecting:   ['CONNECTING…',  'ws-badge connecting',  true],
    disconnected: ['OFFLINE',      'ws-badge disconnected', true],
  };
  const [text, cls, offline] = map[s] || map.connecting;
  badge.textContent = text;
  badge.className = cls;
  dot.classList.toggle('offline', offline);
}

// ── Telemetry handler ─────────────────────────────────────────────────
const ALARM_DEFS = {
  0: null,
  1: { level:'warning',  icon:'⚡', text:'NEAR END — Volume ≥ 90% delivered' },
  2: { level:'critical', icon:'🚫', text:'CRITICAL — OCCLUSION DETECTED — Motor stopped' },
  3: { level:'critical', icon:'🛑', text:'CRITICAL — END OF TRAVEL — Carriage limit hit' },
  4: { level:'warning',  icon:'🔋', text:'WARNING — BATTERY LOW (< 10.2 V)' },
};

let lastAlarmCode = 0;

function handleTelemetry(d) {
  // State
  const stateEl = document.getElementById('kpiState');
  stateEl.textContent = d.state || 'IDLE';
  const isAlarm = d.alarm_code > 0;
  stateEl.className = 'kpi-value ' + (isAlarm ? 'alarm' : (d.state || 'IDLE'));

  // Syringe profile
  document.getElementById('kpiSyringe').textContent = d.syringe_profile || 'No profile';

  // Flow rate
  document.getElementById('kpiFlow').textContent =
    (d.flow_rate_ml_hr ?? 0).toFixed(1);

  // Volume
  const vol = d.vol_delivered_ml ?? 0;
  const tgt = d.vol_target_ml   ?? 0;
  document.getElementById('kpiVol').textContent = vol.toFixed(2);
  document.getElementById('kpiTarget').textContent = tgt > 0 ? tgt.toFixed(0) : '—';

  // Progress ring  (circumference = 2π×45 ≈ 283)
  const pct = tgt > 0 ? Math.min(vol / tgt, 1) : 0;
  document.getElementById('ringFill').style.strokeDashoffset = 283 * (1 - pct);
  document.getElementById('ringLabel').textContent = Math.round(pct * 100) + '%';

  // Battery
  const bv = (d.battery_mv ?? 0) / 1000;
  document.getElementById('kpiBatt').textContent = bv.toFixed(2);
  const battPct = Math.max(0, Math.min(100, ((bv - 10.2) / (12.6 - 10.2)) * 100));
  const battBar = document.getElementById('battBar');
  battBar.style.width = battPct.toFixed(0) + '%';
  battBar.style.background = battPct > 40 ? 'var(--green)' : battPct > 20 ? 'var(--amber)' : 'var(--red)';
  document.getElementById('kpiBattLabel').textContent = battPct > 40 ? 'Good' : battPct > 20 ? 'Low — charge soon' : 'Critical';

  // Force graph
  const force = d.force_newtons ?? 0;
  const limit = d.occlusion_limit_n ?? 0;
  updateForceChart(force, limit);
  document.getElementById('forceNow').textContent   = force.toFixed(2) + ' N';
  document.getElementById('forceLimit').textContent = limit > 0 ? limit.toFixed(1) + ' N' : '—';
  document.getElementById('dispThreshold').textContent = limit > 0 ? limit.toFixed(1) : '—';

  // Alarm banner
  if (d.alarm_code !== lastAlarmCode) {
    lastAlarmCode = d.alarm_code;
    updateAlarmBanner(d.alarm_code);
    if (d.alarm_code > 0) {
      const a = ALARM_DEFS[d.alarm_code];
      logEvent(a ? a.level : 'critical', a ? a.text : 'Unknown alarm code ' + d.alarm_code);
    }
  }
}

function updateAlarmBanner(code) {
  const banner = document.getElementById('alarmBanner');
  const a = ALARM_DEFS[code];
  if (!a) {
    banner.className = 'alarm-banner';
    return;
  }
  document.getElementById('alarmIcon').textContent = a.icon;
  document.getElementById('alarmText').textContent = a.text;
  banner.className = 'alarm-banner active ' + a.level;
}

// ── Chart.js force waveform ───────────────────────────────────────────
const CHART_WINDOW = 60; // seconds of data visible
const forceLabels = [];
const forceData   = [];
let   limitData   = [];

const ctx = document.getElementById('forceChart').getContext('2d');
const forceChart = new Chart(ctx, {
  type: 'line',
  data: {
    labels: forceLabels,
    datasets: [
      {
        label: 'Line Force (N)',
        data: forceData,
        borderColor: '#3b82f6',
        backgroundColor: 'rgba(59,130,246,0.08)',
        fill: true,
        tension: 0.3,
        pointRadius: 0,
        borderWidth: 2,
      },
      {
        label: 'Occlusion Limit',
        data: limitData,
        borderColor: 'rgba(239,68,68,0.6)',
        borderDash: [6, 3],
        pointRadius: 0,
        borderWidth: 1.5,
        fill: false,
      },
    ],
  },
  options: {
    responsive: true,
    maintainAspectRatio: false,
    animation: false,
    interaction: { mode: 'index', intersect: false },
    plugins: {
      legend: { labels: { color: '#6b83a0', boxWidth: 12, font: { size: 11 } } },
      tooltip: {
        backgroundColor: '#0e1929',
        borderColor: '#1e3048',
        borderWidth: 1,
        titleColor: '#e2eaf5',
        bodyColor: '#6b83a0',
      },
    },
    scales: {
      x: {
        ticks: { color: '#6b83a0', maxTicksLimit: 6, maxRotation: 0 },
        grid:  { color: 'rgba(255,255,255,0.04)' },
      },
      y: {
        beginAtZero: true,
        suggestedMax: 30,
        title: { display: true, text: 'Force (N)', color: '#6b83a0', font: { size: 11 } },
        ticks: { color: '#6b83a0' },
        grid:  { color: 'rgba(255,255,255,0.04)' },
      },
    },
  },
});

function updateForceChart(force, limit) {
  const t = new Date().toLocaleTimeString('en-GB');
  forceLabels.push(t);
  forceData.push(+force.toFixed(3));
  limitData.push(limit > 0 ? limit : null);
  if (forceLabels.length > CHART_WINDOW) {
    forceLabels.shift(); forceData.shift(); limitData.shift();
  }
  forceChart.update('none'); // 'none' skips animation for performance
}

// ── Event log ─────────────────────────────────────────────────────────
function logEvent(level, msg) {
  const log = document.getElementById('alarmLog');
  const empty = log.querySelector('.log-empty');
  if (empty) empty.remove();

  const t = new Date().toLocaleTimeString('en-GB');
  const el = document.createElement('div');
  el.className = 'log-entry ' + level;
  el.innerHTML = `<span class="log-time">${t}</span><span>${msg}</span>`;
  log.prepend(el);

  // Keep only the last 50 entries
  while (log.children.length > 50) log.lastChild.remove();
}

// ── Dialog (confirmation gate) ────────────────────────────────────────
let _pendingAction = null;

function openDialog(title, body, onConfirm) {
  document.getElementById('dialogTitle').textContent = title;
  document.getElementById('dialogBody').textContent  = body;
  document.getElementById('dialogConfirm').onclick   = () => { closeDialog(); onConfirm(); };
  document.getElementById('overlay').classList.add('open');
}

function closeDialog() {
  document.getElementById('overlay').classList.remove('open');
}

// ── Command senders ───────────────────────────────────────────────────
function cmdSend(command, value = 0.0) {
  if (!ws || ws.readyState !== WebSocket.OPEN) {
    logEvent('warning', 'Cannot send command — WebSocket not connected.');
    return;
  }
  const frame = { command, value, transaction_id: txId++ };
  ws.send(JSON.stringify(frame));
}

function cmdStart() {
  const rate = parseFloat(document.getElementById('inRate').value);
  if (isNaN(rate) || rate <= 0 || rate > 150) {
    logEvent('warning', 'Invalid rate entered. Must be 0.1 – 150 mL/hr.');
    return;
  }
  openDialog(
    'Confirm Infusion Start',
    `Start infusion at ${rate.toFixed(1)} mL/hr? Verify the syringe is correctly loaded before proceeding.`,
    () => {
      cmdSend('START', rate);
      logEvent('info', `Start command sent — target ${rate.toFixed(1)} mL/hr.`);
    }
  );
}

function cmdSetRate() {
  const rate = parseFloat(document.getElementById('inRate').value);
  if (isNaN(rate) || rate <= 0 || rate > 150) {
    logEvent('warning', 'Invalid rate. Must be 0.1 – 150 mL/hr.');
    return;
  }
  openDialog(
    'Confirm Rate Change',
    `Change delivery rate to ${rate.toFixed(1)} mL/hr while running?`,
    () => {
      cmdSend('SET_RATE', rate);
      logEvent('info', `Rate change command sent → ${rate.toFixed(1)} mL/hr.`);
    }
  );
}

function cmdStop() {
  openDialog(
    '⛔ Emergency Stop',
    'This will immediately halt the motor and reset the pump to IDLE. Confirm only in an emergency.',
    () => {
      cmdSend('STOP');
      logEvent('critical', 'Emergency stop triggered by operator.');
    }
  );
}

// ── Boot ──────────────────────────────────────────────────────────────
setWsStatus('connecting');
connect();
</script>
</body>
</html>
)rawliteral";
