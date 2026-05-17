const MAX_LOG_CHARS = 120000;

const elements = {
  baudInput: document.getElementById("baudInput"),
  connectBtn: document.getElementById("connectBtn"),
  disconnectBtn: document.getElementById("disconnectBtn"),
  loadHotStartBtn: document.getElementById("loadHotStartBtn"),
  clearLogBtn: document.getElementById("clearLogBtn"),
  sendBtn: document.getElementById("sendBtn"),
  commandMode: document.getElementById("commandMode"),
  commandInput: document.getElementById("commandInput"),
  commandLabel: document.getElementById("commandLabel"),
  previewText: document.getElementById("previewText"),
  portStatus: document.getElementById("portStatus"),
  txStatus: document.getElementById("txStatus"),
  lineCount: document.getElementById("lineCount"),
  modeStatus: document.getElementById("modeStatus"),
  serialLog: document.getElementById("serialLog")
};

const state = {
  port: null,
  reader: null,
  writer: null,
  active: false,
  rxBuffer: "",
  lineCount: 0
};

function nowText() {
  return new Date().toLocaleTimeString();
}

function appendLog(direction, text) {
  elements.serialLog.textContent += `[${nowText()}] ${direction} ${text}`;
  if (!text.endsWith("\n")) {
    elements.serialLog.textContent += "\n";
  }
  if (elements.serialLog.textContent.length > MAX_LOG_CHARS) {
    elements.serialLog.textContent = elements.serialLog.textContent.slice(-MAX_LOG_CHARS);
  }
  elements.serialLog.scrollTop = elements.serialLog.scrollHeight;
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
  elements.commandLabel.textContent = mode === "raw" ? "Raw command line" : "NMEA payload";
  elements.modeStatus.textContent = mode === "raw" ? "Raw" : "Payload";
  const command = buildCommand();
  elements.previewText.textContent = command.length > 0 ? visibleEscapes(command) : "$...*CS\\r\\n";
  elements.sendBtn.disabled = state.writer === null || command.length === 0;
}

function setPortStatus(text) {
  elements.portStatus.textContent = text;
}

function setTxStatus(text, tone) {
  elements.txStatus.textContent = text;
  elements.txStatus.className = `result ${tone}`;
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

async function connectSerial() {
  if (!("serial" in navigator)) {
    setPortStatus("Web Serial unavailable");
    return;
  }

  try {
    const baudRate = Number(elements.baudInput.value) || 115200;
    state.port = await navigator.serial.requestPort();
    await state.port.open({ baudRate });
    state.reader = state.port.readable.getReader();
    state.writer = state.port.writable.getWriter();
    state.active = true;
    elements.connectBtn.disabled = true;
    elements.disconnectBtn.disabled = false;
    setPortStatus(`Connected @ ${baudRate}`);
    setTxStatus("Ready", "neutral");
    updatePreview();
    readLoop();
  } catch (error) {
    setPortStatus(error.name === "NotFoundError" ? "Port not selected" : "Open failed");
  }
}

async function readLoop() {
  const decoder = new TextDecoder();

  try {
    while (state.active && state.reader !== null) {
      const result = await state.reader.read();
      if (result.done) {
        break;
      }
      feedReceivedText(decoder.decode(result.value, { stream: true }));
    }
  } catch (error) {
    if (state.active) {
      setPortStatus("Read error");
    }
  } finally {
    await disconnectSerial();
  }
}

async function disconnectSerial() {
  state.active = false;

  if (state.reader !== null) {
    try {
      await state.reader.cancel();
    } catch (error) {
      // The reader may already be closed.
    }
    try {
      state.reader.releaseLock();
    } catch (error) {
      // Some browsers release the lock during cancel.
    }
    state.reader = null;
  }

  if (state.writer !== null) {
    try {
      state.writer.releaseLock();
    } catch (error) {
      // The writer may already be released.
    }
    state.writer = null;
  }

  if (state.port !== null) {
    try {
      await state.port.close();
    } catch (error) {
      // The port may already be closed.
    }
    state.port = null;
  }

  elements.connectBtn.disabled = false;
  elements.disconnectBtn.disabled = true;
  setPortStatus("Disconnected");
  updatePreview();
}

async function sendCommand() {
  const command = buildCommand();
  if (state.writer === null || command.length === 0) {
    return;
  }

  try {
    const encoder = new TextEncoder();
    await state.writer.write(encoder.encode(command));
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

elements.connectBtn.addEventListener("click", connectSerial);
elements.disconnectBtn.addEventListener("click", disconnectSerial);
elements.sendBtn.addEventListener("click", sendCommand);
elements.loadHotStartBtn.addEventListener("click", loadHotStartCommand);
elements.clearLogBtn.addEventListener("click", () => {
  elements.serialLog.textContent = "";
  state.lineCount = 0;
  elements.lineCount.textContent = "0";
});
elements.commandMode.addEventListener("change", updatePreview);
elements.commandInput.addEventListener("input", updatePreview);

updatePreview();
