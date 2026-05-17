const GAP_SECONDS = 2;
const MAX_POINTS = 600;
const MAX_RAW_CHARS = 200000;
const UNSET_S32_HEX = "7FFFFFFF";

const elements = {
  baudInput: document.getElementById("baudInput"),
  connectBtn: document.getElementById("connectBtn"),
  disconnectBtn: document.getElementById("disconnectBtn"),
  parseBtn: document.getElementById("parseBtn"),
  sampleBtn: document.getElementById("sampleBtn"),
  clearBtn: document.getElementById("clearBtn"),
  clearChartBtn: document.getElementById("clearChartBtn"),
  meanToggle: document.getElementById("meanToggle"),
  instantToggle: document.getElementById("instantToggle"),
  rawInput: document.getElementById("rawInput"),
  validCount: document.getElementById("validCount"),
  invalidCount: document.getElementById("invalidCount"),
  ignoredCount: document.getElementById("ignoredCount"),
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
  serialActive: false,
  parseTimer: null,
  stats: makeStats()
};

function makeStats() {
  return {
    lines: 0,
    valid: 0,
    invalid: 0,
    ignored: 0,
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

function nmeaFrame(payload) {
  return `$${payload}*${hexByte(checksum(payload))}\r\n`;
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

function parsePgdosFrame(frame) {
  const star = frame.indexOf("*");
  if (!frame.startsWith("$PGDOS,") || star < 0 || star + 3 !== frame.length) {
    return { ok: false, reason: "format" };
  }

  const payload = frame.slice(1, star);
  const expected = frame.slice(star + 1, star + 3).toUpperCase();
  if (!/^[0-9A-F]{2}$/.test(expected)) {
    return { ok: false, reason: "checksum" };
  }

  const actual = hexByte(checksum(payload));
  if (actual !== expected) {
    return { ok: false, reason: "checksum" };
  }

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

function extractFrame(line) {
  const start = line.indexOf("$PGDOS,");
  if (start < 0) {
    return null;
  }

  const candidate = line.slice(start).trim();
  const match = candidate.match(/^\$PGDOS,[^*\r\n]*(?:\*[0-9A-Fa-f]{2})?/);
  return match ? match[0] : null;
}

function handleLine(line) {
  const trimmed = line.trim();
  if (trimmed.length === 0) {
    return;
  }

  state.stats.lines += 1;

  const frame = extractFrame(trimmed);
  if (frame === null) {
    state.stats.ignored += 1;
    return;
  }

  const parsed = parsePgdosFrame(frame);
  if (!parsed.ok) {
    state.stats.invalid += 1;
    if (parsed.reason === "checksum") {
      state.stats.checksumBad += 1;
    }
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

  render();
}

function parseCurrentInput() {
  resetData();
  feedText(elements.rawInput.value, true);
  setInputStatus(state.serialActive ? "Serial connected" : "Parsed input");
}

function scheduleParse() {
  window.clearTimeout(state.parseTimer);
  state.parseTimer = window.setTimeout(parseCurrentInput, 120);
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
  elements.validCount.textContent = String(state.stats.valid);
  elements.invalidCount.textContent = String(state.stats.invalid);
  elements.ignoredCount.textContent = String(state.stats.ignored);

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
  renderMetrics();
  renderTable();
  drawChart();
}

function appendRawText(text) {
  const input = elements.rawInput;
  input.value += text;
  if (input.value.length > MAX_RAW_CHARS) {
    input.value = input.value.slice(input.value.length - MAX_RAW_CHARS);
  }
  input.scrollTop = input.scrollHeight;
}

async function connectSerial() {
  if (!("serial" in navigator)) {
    setInputStatus("Web Serial unavailable");
    return;
  }

  try {
    const baudRate = Number(elements.baudInput.value) || 115200;
    state.serialPort = await navigator.serial.requestPort();
    await state.serialPort.open({ baudRate });
    state.serialReader = state.serialPort.readable.getReader();
    state.serialActive = true;
    elements.connectBtn.disabled = true;
    elements.disconnectBtn.disabled = false;
    setInputStatus("Serial connected");
    readSerialLoop();
  } catch (error) {
    setInputStatus(error.name === "NotFoundError" ? "Serial not selected" : "Serial error");
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
      appendRawText(text);
      feedText(text, false);
    }
  } catch (error) {
    if (state.serialActive) {
      setInputStatus("Serial read error");
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
  if (elements.inputStatus.textContent === "Serial connected") {
    setInputStatus("Serial disconnected");
  }
}

function loadSample() {
  const sample = [
    nmeaFrame("GPGGA,180000.00,5000.000,N,03000.000,E,1,08,1.0,120.0,M,0.0,M,,"),
    nmeaFrame("PGDOS,W+,00000082,07,0000001D,7FFFFFFF,04E3,010002"),
    nmeaFrame("GPRMC,180001.00,A,5000.000,N,03000.000,E,0.0,0.0,170526,,,A"),
    nmeaFrame("PGDOS,++,00000083,08,00000019,0000001C,04E4,010002"),
    nmeaFrame("PGDOS,++,00000088,09,FFFFFFEC,FFFFFFF6,04E6,0100FF"),
    nmeaFrame("PGDOS,+N,00000089,04,FFFFFFF0,FFFFFFF1,04E2,020100")
  ].join("");

  elements.rawInput.value = sample;
  parseCurrentInput();
}

elements.connectBtn.addEventListener("click", connectSerial);
elements.disconnectBtn.addEventListener("click", closeSerial);
elements.parseBtn.addEventListener("click", parseCurrentInput);
elements.sampleBtn.addEventListener("click", loadSample);
elements.clearBtn.addEventListener("click", () => {
  elements.rawInput.value = "";
  resetData();
  setInputStatus(state.serialActive ? "Serial connected" : "Idle");
  render();
});
elements.clearChartBtn.addEventListener("click", () => {
  state.points = [];
  drawChart();
});
elements.meanToggle.addEventListener("change", drawChart);
elements.instantToggle.addEventListener("change", drawChart);
elements.rawInput.addEventListener("input", scheduleParse);
window.addEventListener("resize", drawChart);

render();
