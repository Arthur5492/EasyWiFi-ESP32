// State management
let isScanning = false;
let isConnecting = false;
let scanAttempts = 0;
const maxScanAttempts = 5;

async function connect(ssid, pass, isProtected) {
  if (isConnecting) return;
  
  try {
    isConnecting = true;
    elements.connectBtn.disabled = true;
    elements.connectBtn.textContent = 'Connecting...';

    const response = await fetch('/save', {
      method: 'POST',
      headers: {'Content-Type': 'application/x-www-form-urlencoded'},
      body: new URLSearchParams({
        ssid,
        password: pass,
        isProtected: isProtected
      })
    });
    
    const message = await response.text();
    
    if (response.ok) {
      showMsg(message);
      setTimeout(checkConnectionStatus, 3000);
    } else {
      showMsg(message, 'error');
      isConnecting = false;
      elements.connectBtn.disabled = false;
      elements.connectBtn.textContent = 'Connect';
    }
  } catch (error) {
    showMsg('Connection failed', 'error');
    isConnecting = false;
    elements.connectBtn.disabled = false;
    elements.connectBtn.textContent = 'Connect';
  }
}

async function scanNetworks() {
  if (isScanning) return;

  try {
    isScanning = true;
    elements.scanBtn.className = 'scan-button loading';
    
    const response = await fetch('/scan');
    if (!response.ok) throw new Error('Scan failed');

    const networks = await response.json();
    
    if (networks.length === 0) {
      scanAttempts++;
      if (scanAttempts >= maxScanAttempts) {
        elements.list.innerHTML = '<div class="error">No networks found. Please try again later.</div>';
        return;
      }
      elements.list.innerHTML = '<div class="scanning">Scanning in progress...</div>';
      setTimeout(scanNetworks, 3000);
      return;
    }

    scanAttempts = 0;
    elements.list.innerHTML = '';
    networks.forEach(network => {
      elements.list.appendChild(createNetworkElement(network));
    });

  } catch (error) {
    showMsg('Scan failed', 'error');
    elements.list.innerHTML = '<div class="error">Failed to scan networks</div>';
  } finally {
    isScanning = false;
    elements.scanBtn.className = 'scan-button';
  }
}

async function checkConnectionStatus() {
  try {
    const response = await fetch('/wifi-status');
    const message = await response.text();
    
    if (response.status === 200) {
      showMsg(message, 'success');
      isConnecting = false;
      elements.connectBtn.disabled = false;
      elements.connectBtn.textContent = 'Connect';
      elements.modal.className = 'password-modal';
      return true;
    }
    
    if (response.status === 202) {
      showMsg(message);
      setTimeout(checkConnectionStatus, 3000);
      return false;
    }
    
    showMsg(message, 'error');
    isConnecting = false;
    elements.connectBtn.disabled = false;
    elements.connectBtn.textContent = 'Connect';
    return false;
    
  } catch (error) {
    showMsg('Error checking connection status', 'error');
    isConnecting = false;
    elements.connectBtn.disabled = false;
    elements.connectBtn.textContent = 'Connect';
    return false;
  }
}

function createNetworkElement(network) {
  const div = document.createElement('div');
  div.className = 'wifi-item';
  div.innerHTML = `
    <div class="wifi-icon">
      <img src="easyWifi/icons/wifi-${getSignalIcon(network.rssi)}.svg" alt="Signal" width="24" height="24">
    </div>
    <div class="wifi-info">
      <div class="wifi-name">
        ${escapeHtml(network.ssid)}
        ${network.isProtected ? '<img src="easyWifi/icons/lock-wifi.svg" alt="Locked" width="16" height="16">' : ''}
      </div>
      <div class="wifi-signal">${network.rssi}dBm</div>
    </div>
  `;
  
  div.addEventListener('click', () => selectNetwork(network.ssid, network.isProtected));
  return div;
}

function selectNetwork(ssid, isProtected) {
  if (isConnecting) return;
  
  if (!isProtected) {
    connect(ssid, '', isProtected);
    return;
  }
  
  elements.netName.textContent = ssid;
  elements.modal.className = 'password-modal active';
  elements.passInput.value = '';
  elements.passInput.focus();
  
  const newConnectBtn = elements.connectBtn.cloneNode(true);
  elements.connectBtn.parentNode.replaceChild(newConnectBtn, elements.connectBtn);
  elements.connectBtn = newConnectBtn;
  
  elements.connectBtn.addEventListener('click', () => {
    const pass = elements.passInput.value;
    if (pass) connect(ssid, pass, isProtected);
  });
}

// Utility functions
function escapeHtml(unsafe) {
  return unsafe
    .replace(/&/g, "&amp;")
    .replace(/</g, "&lt;")
    .replace(/>/g, "&gt;")
    .replace(/"/g, "&quot;")
    .replace(/'/g, "&#039;");
}

function showMsg(msg, type = '') {
  const status = document.getElementById('status-message');
  status.textContent = msg;
  status.className = `status-message active ${type}`;
  setTimeout(() => status.className = 'status-message', 3000);
}

function getSignalIcon(rssi) {
  if (rssi >= -50) return 4;
  if (rssi >= -60) return 3;
  if (rssi >= -70) return 2;
  if (rssi >= -80) return 1;
  return 0;
}

// DOM Elements cache
const elements = {
  list: document.getElementById('wifi-list'),
  scanBtn: document.getElementById('scan-button'),
  modal: document.getElementById('password-modal'),
  passInput: document.getElementById('password-input'),
  netName: document.getElementById('modal-network-name'),
  connectBtn: document.getElementById('connect-button'),
  cancelBtn: document.getElementById('cancel-button')
};

// Event Listeners
elements.scanBtn.addEventListener('click', scanNetworks);
elements.cancelBtn.addEventListener('click', () => {
  if (!isConnecting) {
    elements.modal.className = 'password-modal';
  }
});

elements.passInput.addEventListener('keydown', (e) => {
  if (e.key === 'Enter' && !isConnecting && elements.passInput.value) {
    elements.connectBtn.click();
  }
});

// Initial scan
scanNetworks();