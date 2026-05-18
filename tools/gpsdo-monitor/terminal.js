(function () {
const MAX_LOG_CHARS = 120000;
const LOG_FLUSH_MS = 250;

const elements = {
  loadHotStartBtn: document.getElementById("terminalLoadHotStartBtn"),
  clearLogBtn: document.getElementById("terminalClearLogBtn"),
  sendBtn: document.getElementById("terminalSendBtn"),
  commandMode: document.getElementById("terminalCommandMode"),
  commandInput: document.getElementById("terminalCommandInput"),
  commandLabel: document.getElementById("terminalCommandLabel"),
  previewText: document.getElementById("terminalPreviewText"),
  portStatus: document.getElementById("terminalPortStatus"),
  txStatus: document.getElementById("terminalTxStatus"),
  lineCount: document.getElementById("terminalLineCount"),
  modeStatus: document.getElementById("terminalModeStatus"),
  serialLog: document.getElementById("terminalSerialLog")
};

const state = {
  rxBuffer: "",
  lineCount: 0,
  logFlushTimer: null,
  pendingLogText: ""
};

function nowText() {
  return new Date().toLocaleTimeString();
}

function isConnected() {
  return window.gpsdoSerialIsConnected?.() === true;
}

function appendLog(direction, text) {
  state.pendingLogText += `[${nowText()}] ${direction} ${text}`;
  if (!text.endsWith("\n")) {
    state.pendingLogText += "\n";
  }

  if (state.pendingLogText.length > MAX_LOG_CHARS) {
    state.pendingLogText = state.pendingLogText.slice(-MAX_LOG_CHARS);
  }

  scheduleLogFlush();
}

function flushLog() {
  if (state.logFlushTimer !== null) {
    window.clearTimeout(state.logFlushTimer);
    state.logFlushTimer = null;
  }

  if (state.pendingLogText.length === 0) {
    return;
  }

  elements.serialLog.textContent += state.pendingLogText;
  state.pendingLogText = "";

  if (elements.serialLog.textContent.length > MAX_LOG_CHARS) {
    elements.serialLog.textContent = elements.serialLog.textContent.slice(-MAX_LOG_CHARS);
  }

  elements.serialLog.scrollTop = elements.serialLog.scrollHeight;
}

function scheduleLogFlush() {
  if (state.logFlushTimer !== null) {
    return;
  }

  state.logFlushTimer = window.setTimeout(flushLog, LOG_FLUSH_MS);
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

function normalizeEscapes(text) {
  return text
    .replace(/\\r/g, "\r")
    .replace(/\\n/g, "\n")
    .replace(/\\t/g, "\t");
}

function visibleEscapes(text) {
  return text
    .replace(/\r/g, "\\r")
    .replace(/\n/g, "\\n")
    .replace(/\t/g, "\\t");
}

function buildCommand() {
  const mode = elements.commandMode.value;
  const source = elements.commandInput.value.trim();

  if (source.length === 0) {
    return "";
  }

  if (mode === "raw") {
    const command = normalizeEscapes(source);
    return command.endsWith("\n") ? command : `${command}\r\n`;
  }

  const payload = source.replace(/^\$/, "").replace(/\*[0-9A-Fa-f]{2}$/, "");
  return `$${payload}*${hexByte(checksum(payload))}\r\n`;
}

function updatePreview() {
  const mode = elements.commandMode.value;
  const command = buildCommand();

  elements.commandLabel.textContent = mode === "raw" ? "Raw command line" : "NMEA payload";
  elements.modeStatus.textContent = mode === "raw" ? "Raw" : "Payload";
  elements.previewText.textContent = command.length > 0 ? visibleEscapes(command) : "$...*CS\\r\\n";
  elements.sendBtn.disabled = !isConnected() || command.length === 0;
}

function setTxStatus(text, tone) {
  elements.txStatus.textContent = text;
  elements.txStatus.className = `result ${tone}`;
}

function setConnectionStatus(connected, label) {
  elements.portStatus.textContent = label;
  if (connected) {
    setTxStatus("Ready", "neutral");
  }
  updatePreview();
}

function handleLine(line) {
  state.lineCount += 1;
  elements.lineCount.textContent = String(state.lineCount);
  appendLog("RX", line);
}

function feedReceivedText(text) {
  state.rxBuffer += text;
  const lines = state.rxBuffer.split(/\r?\n/);
  state.rxBuffer = lines.pop() || "";

  for (const line of lines) {
    handleLine(line);
  }
}

async function sendCommand() {
  const command = buildCommand();
  if (!isConnected() || command.length === 0) {
    return;
  }

  try {
    await window.gpsdoSerialWrite(command);
    appendLog("TX", visibleEscapes(command));
    setTxStatus(nowText(), "pass");
  } catch (error) {
    setTxStatus("Send failed", "fail");
    appendLog("ERR", "Send failed");
  }
}

function loadHotStartCommand() {
  elements.commandMode.value = "payload";
  elements.commandInput.value = "PCAS10,0";
  updatePreview();
}

elements.sendBtn.addEventListener("click", sendCommand);
elements.loadHotStartBtn.addEventListener("click", loadHotStartCommand);
elements.clearLogBtn.addEventListener("click", () => {
  elements.serialLog.textContent = "";
  state.pendingLogText = "";
  state.rxBuffer = "";
  state.lineCount = 0;
  elements.lineCount.textContent = "0";
});
elements.commandMode.addEventListener("change", updatePreview);
elements.commandInput.addEventListener("input", updatePreview);

window.terminalReceiveText = feedReceivedText;
window.terminalSetConnectionStatus = setConnectionStatus;
window.terminalFlushLog = flushLog;

setConnectionStatus(isConnected(), isConnected() ? "Connected" : "Disconnected");
updatePreview();
})();
