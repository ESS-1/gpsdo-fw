const GAP_SECONDS = 2;
const MAX_POINTS = 600;
const RENDER_INTERVAL_MS = 250;
const UNSET_S32_HEX = "7FFFFFFF";

const elements = {
  baudInput: document.getElementById("baudInput"),
  connectBtn: document.getElementById("connectBtn"),
  disconnectBtn: document.getElementById("disconnectBtn"),
  clearBtn: document.getElementById("clearBtn"),
  clearChartBtn: document.getElementById("clearChartBtn"),
  meanToggle: document.getElementById("meanToggle"),
  instantToggle: document.getElementById("instantToggle"),
  nmeaValidCount: document.getElementById("nmeaValidCount"),
  nmeaInvalidCount: document.getElementById("nmeaInvalidCount"),
  validCount: document.getElementById("validCount"),
  invalidCount: document.getElementById("invalidCount"),
  inputStatus: document.getElementById("inputStatus"),
  deviceStatus: document.getElementById("deviceStatus"),
  gpsStatus: document.getElementById("gpsStatus"),
  uptimeValue: document.getElementById("uptimeValue"),
  satValue: document.getElementById("satValue"),
  meanPpbValue: document.getElementById("meanPpbValue"),
  instPpbValue: document.getElementById("instPpbValue"),
  pwmValue: document.getElementById("pwmValue"),
  commErrorsValue: document.getElementById("commErrorsValue"),
  gapCount: document.getElementById("gapCount"),
  framesBody: document.getElementById("framesBody"),
  lastFrameAt: document.getElementById("lastFrameAt"),
  chart: document.getElementById("ppbChart")
};

const state = {
  partial: "",
  latest: null,
  points: [],
  recent: [],
  serialPort: null,
  serialReader: null,
  serialWriter: null,
  serialActive: false,
  renderTimer: null,
  stats: makeStats()
};

function makeStats() {
  return {
    lines: 0,
    nmeaValid: 0,
    nmeaInvalid: 0,
    nonNmea: 0,
    valid: 0,
    invalid: 0,
    checksumBad: 0
  };
}

function resetData() {
  state.partial = "";
  state.latest = null;
  state.points = [];
  state.recent = [];
  state.stats = makeStats();
}

function checksum(payload) {
  let value = 0;
  for (let i = 0; i < payload.length; i += 1) {
    value ^= payload.charCodeAt(i);
  }
  return value & 0xff;
}

function hexByte(value) {
  return value.toString(16).toUpperCase().padStart(2, "0");
}

function parseHex(text, length) {
  if (text.length !== length || !/^[0-9A-Fa-f]+$/.test(text)) {
    return null;
  }
  return parseInt(text, 16);
}

function parseSignedPpbHundredths(text) {
  if (text.toUpperCase() === UNSET_S32_HEX) {
    return null;
  }

  const raw = parseHex(text, 8);
  if (raw === null) {
    return undefined;
  }

  const signed = raw >= 0x80000000 ? raw - 0x100000000 : raw;
  return signed / 100;
}

function parseNmeaFrame(line) {
  const start = line.indexOf("$");
  if (start < 0) {
    return { ok: false, reason: "not-nmea" };
  }

  const candidate = line.slice(start).trim();
  const match = candidate.match(/^\$([^*\r\n]+)\*([0-9A-Fa-f]{2})/);
  if (match === null) {
    return { ok: false, reason: "format" };
  }

  const payload = match[1];
  const expected = match[2].toUpperCase();
  const actual = hexByte(checksum(payload));
  if (actual !== expected) {
    return {
      ok: false,
      reason: "checksum",
      frame: match[0],
      payload
    };
  }

  return {
    ok: true,
    frame: match[0],
    payload
  };
}

function parsePgdosFrame(payload, frame) {
  const fields = payload.split(",");
  if (fields.length !== 8 || fields[0] !== "PGDOS") {
    return { ok: false, reason: "fields" };
  }

  const status = fields[1];
  const uptime = parseHex(fields[2], 8);
  const satellites = parseHex(fields[3], 2);
  const meanPpb = parseSignedPpbHundredths(fields[4]);
  const instantPpb = parseSignedPpbHundredths(fields[5]);
  const pwm = parseHex(fields[6], 4);
  const errorsRaw = fields[7];

  if (
    !/^[+WF][+NF]$/.test(status) ||
    uptime === null ||
    satellites === null ||
    meanPpb === undefined ||
    instantPpb === undefined ||
    pwm === null ||
    errorsRaw.length !== 6 ||
    !/^[0-9A-Fa-f]+$/.test(errorsRaw)
  ) {
    return { ok: false, reason: "fields" };
  }

  return {
    ok: true,
    data: {
      raw: frame,
      deviceStatus: status[0],
      gpsStatus: status[1],
      uptime,
      satellites,
      meanPpb,
      instantPpb,
      pwm,
      invalidGpsFrames: parseHex(errorsRaw.slice(0, 2), 2),
      gpsFifoOverflows: parseHex(errorsRaw.slice(2, 4), 2),
      pcCommFifoOverflows: parseHex(errorsRaw.slice(4, 6), 2),
      receivedAt: new Date()
    }
  };
}

function handleLine(line) {
  const trimmed = line.trim();
  if (trimmed.length === 0) {
    return;
  }

  state.stats.lines += 1;

  const nmea = parseNmeaFrame(trimmed);
  if (!nmea.ok) {
    if (nmea.reason === "not-nmea") {
      state.stats.nonNmea += 1;
    } else {
      state.stats.nmeaInvalid += 1;
      if (nmea.reason === "checksum") {
        state.stats.checksumBad += 1;
      }
    }
    return;
  }

  state.stats.nmeaValid += 1;

  if (!nmea.payload.startsWith("PGDOS,")) {
    return;
  }

  const parsed = parsePgdosFrame(nmea.payload, nmea.frame);
  if (!parsed.ok) {
    state.stats.invalid += 1;
    return;
  }

  state.stats.valid += 1;
  state.latest = parsed.data;
  state.points.push(parsed.data);
  state.recent.unshift(parsed.data);

  if (state.points.length > MAX_POINTS) {
    state.points.shift();
  }
  if (state.recent.length > 30) {
    state.recent.pop();
  }
}

function feedText(text, finalChunk) {
  state.partial += text;
  const lines = state.partial.split(/\r?\n/);
  state.partial = lines.pop() || "";

  for (const line of lines) {
    handleLine(line);
  }

  if (finalChunk && state.partial.trim().length > 0) {
    handleLine(state.partial);
    state.partial = "";
  }

  scheduleRender();
}

function describeDeviceStatus(value) {
  if (value === "+") {
    return { label: "Normal", tone: "good" };
  }
  if (value === "W") {
    return { label: "Warmup", tone: "warn" };
  }
  return { label: "Fault", tone: "bad" };
}

function describeGpsStatus(value) {
  if (value === "+") {
    return { label: "Lock", tone: "good" };
  }
  if (value === "N") {
    return { label: "No lock", tone: "warn" };
  }
  return { label: "Fault", tone: "bad" };
}

function formatCounter(value) {
  return value === 0xff ? ">=255" : String(value);
}

function formatPpb(value) {
  return value === null ? "unset" : `${value.toFixed(2)} ppb`;
}

function formatUptime(seconds) {
  const days = Math.floor(seconds / 86400);
  const rem = seconds % 86400;
  const hours = Math.floor(rem / 3600);
  const minutes = Math.floor((rem % 3600) / 60);
  const secs = rem % 60;
  const clock = [hours, minutes, secs].map((part) => String(part).padStart(2, "0")).join(":");
  return days > 0 ? `${seconds} s (${days} d ${clock})` : `${seconds} s (${clock})`;
}

function setStatusPill(element, status) {
  element.textContent = status.label;
  element.dataset.tone = status.tone;
}

function setInputStatus(text) {
  elements.inputStatus.textContent = text;
}

function renderMetrics() {
  const latest = state.latest;
  elements.nmeaValidCount.textContent = String(state.stats.nmeaValid);
  elements.nmeaInvalidCount.textContent = String(state.stats.nmeaInvalid);
  elements.validCount.textContent = String(state.stats.valid);
  elements.invalidCount.textContent = String(state.stats.invalid);

  if (latest === null) {
    elements.deviceStatus.textContent = "--";
    elements.gpsStatus.textContent = "--";
    elements.deviceStatus.removeAttribute("data-tone");
    elements.gpsStatus.removeAttribute("data-tone");
    elements.uptimeValue.textContent = "--";
    elements.satValue.textContent = "--";
    elements.meanPpbValue.textContent = "--";
    elements.instPpbValue.textContent = "--";
    elements.pwmValue.textContent = "--";
    elements.commErrorsValue.textContent = "--";
    elements.lastFrameAt.textContent = "No frame yet";
    return;
  }

  setStatusPill(elements.deviceStatus, describeDeviceStatus(latest.deviceStatus));
  setStatusPill(elements.gpsStatus, describeGpsStatus(latest.gpsStatus));
  elements.uptimeValue.textContent = formatUptime(latest.uptime);
  elements.satValue.textContent = String(latest.satellites);
  elements.meanPpbValue.textContent = formatPpb(latest.meanPpb);
  elements.instPpbValue.textContent = formatPpb(latest.instantPpb);
  elements.pwmValue.textContent = String(latest.pwm);
  elements.commErrorsValue.textContent = [
    formatCounter(latest.invalidGpsFrames),
    formatCounter(latest.gpsFifoOverflows),
    formatCounter(latest.pcCommFifoOverflows)
  ].join(" / ");
  elements.lastFrameAt.textContent = latest.receivedAt.toLocaleTimeString();
}

function appendCell(row, text) {
  const cell = document.createElement("td");
  cell.textContent = text;
  row.appendChild(cell);
}

function renderTable() {
  elements.framesBody.textContent = "";

  if (state.recent.length === 0) {
    const row = document.createElement("tr");
    const cell = document.createElement("td");
    cell.colSpan = 8;
    cell.className = "empty-cell";
    cell.textContent = "No valid PGDOS frames";
    row.appendChild(cell);
    elements.framesBody.appendChild(row);
    return;
  }

  for (const frame of state.recent) {
    const row = document.createElement("tr");
    appendCell(row, String(frame.uptime));
    appendCell(row, describeDeviceStatus(frame.deviceStatus).label);
    appendCell(row, describeGpsStatus(frame.gpsStatus).label);
    appendCell(row, String(frame.satellites));
    appendCell(row, formatPpb(frame.meanPpb));
    appendCell(row, formatPpb(frame.instantPpb));
    appendCell(row, String(frame.pwm));
    appendCell(row, [
      formatCounter(frame.invalidGpsFrames),
      formatCounter(frame.gpsFifoOverflows),
      formatCounter(frame.pcCommFifoOverflows)
    ].join(" / "));
    elements.framesBody.appendChild(row);
  }
}

function resizeCanvas(canvas) {
  const rect = canvas.getBoundingClientRect();
  const dpr = window.devicePixelRatio || 1;
  const width = Math.max(320, Math.floor(rect.width * dpr));
  const height = Math.max(220, Math.floor(rect.height * dpr));

  if (canvas.width !== width || canvas.height !== height) {
    canvas.width = width;
    canvas.height = height;
  }

  return { width, height, dpr };
}

function drawEmptyChart(ctx, width, height, text) {
  ctx.fillStyle = "#f9fbfc";
  ctx.fillRect(0, 0, width, height);
  ctx.fillStyle = "#657384";
  ctx.textAlign = "center";
  ctx.textBaseline = "middle";
  ctx.font = "14px system-ui, sans-serif";
  ctx.fillText(text, width / 2, height / 2);
}

function countGaps(points) {
  let gaps = 0;
  for (let i = 1; i < points.length; i += 1) {
    if (points[i].uptime - points[i - 1].uptime > GAP_SECONDS) {
      gaps += 1;
    }
  }
  return gaps;
}

function paddedRange(values) {
  if (values.length === 0) {
    return null;
  }

  let min = Math.min(...values);
  let max = Math.max(...values);
  if (min === max) {
    min -= 1;
    max += 1;
  }

  const padding = Math.max((max - min) * 0.12, 0.05);
  return {
    min: min - padding,
    max: max + padding
  };
}

function drawChart() {
  const canvas = elements.chart;
  const ctx = canvas.getContext("2d");
  const { width, height, dpr } = resizeCanvas(canvas);
  const showMean = elements.meanToggle.checked;
  const showInstant = elements.instantToggle.checked;
  elements.gapCount.textContent = `Pauses: ${countGaps(state.points)}`;
  const pad = {
    left: 64 * dpr,
    right: 64 * dpr,
    top: 18 * dpr,
    bottom: 36 * dpr
  };

  const meanValues = [];
  const instantValues = [];
  for (const point of state.points) {
    if (showMean && point.meanPpb !== null) {
      meanValues.push(point.meanPpb);
    }
    if (showInstant && point.instantPpb !== null) {
      instantValues.push(point.instantPpb);
    }
  }

  if (!showMean && !showInstant) {
    drawEmptyChart(ctx, width, height, "No chart selected");
    return;
  }
  if (state.points.length === 0) {
    drawEmptyChart(ctx, width, height, "Waiting for PGDOS frames");
    return;
  }
  if (meanValues.length === 0 && instantValues.length === 0) {
    drawEmptyChart(ctx, width, height, "PPB values are unset");
    return;
  }

  const xMin = state.points[0].uptime;
  const xMax = Math.max(xMin + 1, state.points[state.points.length - 1].uptime);
  const meanRange = showMean ? paddedRange(meanValues) : null;
  const instantRange = showInstant ? paddedRange(instantValues) : null;

  const plotW = width - pad.left - pad.right;
  const plotH = height - pad.top - pad.bottom;
  const xFor = (uptime) => pad.left + ((uptime - xMin) / (xMax - xMin)) * plotW;
  const yForRange = (range) => (value) => pad.top + (1 - (value - range.min) / (range.max - range.min)) * plotH;

  ctx.fillStyle = "#f9fbfc";
  ctx.fillRect(0, 0, width, height);
  ctx.strokeStyle = "#d8e0e8";
  ctx.lineWidth = 1 * dpr;
  ctx.fillStyle = "#657384";
  ctx.font = `${12 * dpr}px system-ui, sans-serif`;
  ctx.textBaseline = "middle";

  for (let i = 0; i <= 4; i += 1) {
    const y = pad.top + (plotH * i) / 4;
    ctx.beginPath();
    ctx.moveTo(pad.left, y);
    ctx.lineTo(width - pad.right, y);
    ctx.stroke();

    if (meanRange !== null) {
      const meanValue = meanRange.max - ((meanRange.max - meanRange.min) * i) / 4;
      ctx.fillStyle = "#0f766e";
      ctx.textAlign = "right";
      ctx.fillText(meanValue.toFixed(2), pad.left - 8 * dpr, y);
    }

    if (instantRange !== null) {
      const instantValue = instantRange.max - ((instantRange.max - instantRange.min) * i) / 4;
      ctx.fillStyle = "rgba(37, 99, 235, 0.72)";
      ctx.textAlign = "left";
      ctx.fillText(instantValue.toFixed(2), width - pad.right + 8 * dpr, y);
    }
  }

  ctx.strokeStyle = "#d8e0e8";
  ctx.fillStyle = "#657384";
  ctx.textBaseline = "top";
  for (let i = 0; i <= 4; i += 1) {
    const x = pad.left + (plotW * i) / 4;
    const value = Math.round(xMin + ((xMax - xMin) * i) / 4);
    ctx.beginPath();
    ctx.moveTo(x, pad.top);
    ctx.lineTo(x, height - pad.bottom);
    ctx.stroke();
    ctx.textAlign = "center";
    ctx.fillText(`${value}s`, x, height - pad.bottom + 9 * dpr);
  }

  if (instantRange !== null) {
    drawSeries(ctx, "instantPpb", "rgba(37, 99, 235, 0.48)", xFor, yForRange(instantRange), dpr);
  }
  if (meanRange !== null) {
    drawSeries(ctx, "meanPpb", "#0f766e", xFor, yForRange(meanRange), dpr);
  }
}

function drawSeries(ctx, key, color, xFor, yFor, dpr) {
  ctx.strokeStyle = color;
  ctx.lineWidth = 2 * dpr;
  ctx.lineJoin = "round";
  ctx.lineCap = "round";
  ctx.beginPath();

  let previous = null;
  let hasPath = false;
  for (const point of state.points) {
    const value = point[key];
    if (value === null) {
      previous = null;
      continue;
    }

    const x = xFor(point.uptime);
    const y = yFor(value);
    if (previous === null || point.uptime - previous.uptime > GAP_SECONDS) {
      ctx.moveTo(x, y);
    } else {
      ctx.lineTo(x, y);
    }
    previous = point;
    hasPath = true;
  }

  if (hasPath) {
    ctx.stroke();
  }
}

function render() {
  if (state.renderTimer !== null) {
    window.clearTimeout(state.renderTimer);
    state.renderTimer = null;
  }
  renderMetrics();
  renderTable();
  drawChart();
}

function scheduleRender() {
  if (state.renderTimer !== null) {
    return;
  }

  state.renderTimer = window.setTimeout(() => {
    state.renderTimer = null;
    render();
  }, RENDER_INTERVAL_MS);
}

async function connectSerial() {
  if (!("serial" in navigator)) {
    setInputStatus("Web Serial unavailable");
    window.terminalSetConnectionStatus?.(false, "Web Serial unavailable");
    return;
  }

  try {
    const baudRate = Number(elements.baudInput.value) || 115200;
    state.serialPort = await navigator.serial.requestPort();
    await state.serialPort.open({ baudRate });
    state.serialReader = state.serialPort.readable.getReader();
    state.serialWriter = state.serialPort.writable.getWriter();
    state.serialActive = true;
    elements.connectBtn.disabled = true;
    elements.disconnectBtn.disabled = false;
    setInputStatus(`Connected @ ${baudRate}`);
    window.terminalSetConnectionStatus?.(true, `Connected @ ${baudRate}`);
    readSerialLoop();
  } catch (error) {
    setInputStatus(error.name === "NotFoundError" ? "Serial not selected" : "Serial error");
    window.terminalSetConnectionStatus?.(false, "Not connected");
  }
}

async function readSerialLoop() {
  const decoder = new TextDecoder();

  try {
    while (state.serialActive && state.serialReader !== null) {
      const result = await state.serialReader.read();
      if (result.done) {
        break;
      }

      const text = decoder.decode(result.value, { stream: true });
      window.terminalReceiveText?.(text);
      feedText(text, false);
    }
  } catch (error) {
    if (state.serialActive) {
      setInputStatus("Serial read error");
      window.terminalSetConnectionStatus?.(false, "Read error");
    }
  } finally {
    await closeSerial();
  }
}

async function closeSerial() {
  state.serialActive = false;

  if (state.serialReader !== null) {
    try {
      await state.serialReader.cancel();
    } catch (error) {
      // The reader may already be closed by the stream.
    }
    try {
      state.serialReader.releaseLock();
    } catch (error) {
      // Some browsers release the lock during cancel.
    }
    state.serialReader = null;
  }

  if (state.serialWriter !== null) {
    try {
      state.serialWriter.releaseLock();
    } catch (error) {
      // The writer may already be released.
    }
    state.serialWriter = null;
  }

  if (state.serialPort !== null) {
    try {
      await state.serialPort.close();
    } catch (error) {
      // The port may already be closed after an unplug or read error.
    }
    state.serialPort = null;
  }

  elements.connectBtn.disabled = false;
  elements.disconnectBtn.disabled = true;
  if (elements.inputStatus.textContent.startsWith("Connected")) {
    setInputStatus("Serial disconnected");
  }
  window.terminalSetConnectionStatus?.(false, "Disconnected");
}

window.gpsdoSerialIsConnected = function gpsdoSerialIsConnected() {
  return state.serialActive && state.serialWriter !== null;
};

window.gpsdoSerialWrite = async function gpsdoSerialWrite(text) {
  if (!window.gpsdoSerialIsConnected()) {
    throw new Error("Serial port is not connected");
  }

  const encoder = new TextEncoder();
  await state.serialWriter.write(encoder.encode(text));
};

elements.connectBtn.addEventListener("click", connectSerial);
elements.disconnectBtn.addEventListener("click", closeSerial);
elements.clearBtn.addEventListener("click", () => {
  resetData();
  setInputStatus(state.serialActive ? "Connected" : "Idle");
  render();
});
elements.clearChartBtn.addEventListener("click", () => {
  state.points = [];
  drawChart();
});
elements.meanToggle.addEventListener("change", drawChart);
elements.instantToggle.addEventListener("change", drawChart);
window.addEventListener("resize", drawChart);
document.addEventListener("visibilitychange", () => {
  if (!document.hidden) {
    render();
  }
});
document.querySelectorAll(".tab-button").forEach((button) => {
  button.addEventListener("click", () => {
    document.querySelectorAll(".tab-button").forEach((item) => {
      item.classList.toggle("active", item === button);
    });
    document.querySelectorAll(".tab-panel").forEach((panel) => {
      panel.classList.toggle("active", panel.id === button.dataset.tabTarget);
    });
    if (button.dataset.tabTarget === "monitorTab") {
      drawChart();
    } else if (button.dataset.tabTarget === "terminalTab") {
      window.terminalFlushLog?.();
    }
  });
});

render();
