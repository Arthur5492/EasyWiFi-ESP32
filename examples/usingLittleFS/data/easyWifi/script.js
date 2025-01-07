/*====================================================================
  GENERAL STATES (STATE MANAGEMENT)
  ====================================================================*/
  let isScanning = false;      // Indicates if a network scan is in progress
  let isConnecting = false;    // Indicates if a connection attempt is in progress
  let scanAttempts = 0;        // Tracks the number of scan attempts
  const maxScanAttempts = 5;   // Maximum number of scan retries
  
  const elements = {
    list: document.getElementById('wifi-list'),
    scanBtn: document.getElementById('scan-button'),
    modal: document.getElementById('password-modal'),
    passInput: document.getElementById('password-input'),
    netName: document.getElementById('modal-network-name'),
    connectBtn: document.getElementById('connect-button'),
    cancelBtn: document.getElementById('cancel-button'),
    statusMsg: document.getElementById('status-message')
  };

  const icons = {
    "lock-wifi" :`<svg xmlns="http://www.w3.org/2000/svg" height="24px" width="24px" viewBox="0 -960 960 960"  fill="#000000"><path d="M240-80q-33 0-56.5-23.5T160-160v-400q0-33 23.5-56.5T240-640h40v-80q0-83 58.5-141.5T480-920q83 0 141.5 58.5T680-720v80h40q33 0 56.5 23.5T800-560v400q0 33-23.5 56.5T720-80H240Zm0-80h480v-400H240v400Zm240-120q33 0 56.5-23.5T560-360q0-33-23.5-56.5T480-440q-33 0-56.5 23.5T400-360q0 33 23.5 56.5T480-280ZM360-640h240v-80q0-50-35-85t-85-35q-50 0-85 35t-35 85v80ZM240-160v-400 400Z"/></svg>`,
    "wifi-find" :`<svg xmlns="http://www.w3.org/2000/svg" height="24px" width="24px" viewBox="0 -960 960 960" fill="#000000"><path d="M480-120 0-601q93-93 215.5-146T480-800q142 0 264.5 53T960-601l-56 57q-81-81-190-128.5T480-720q-103 0-195 32.5T117-597l419 420-56 57Zm384-40L761-262q-18 11-38 16.5t-43 5.5q-68 0-114-46t-46-114q0-68 46-114t114-46q68 0 114 46t46 114q0 23-5.5 43T818-319l102 103-56 56ZM680-320q34 0 57-23t23-57q0-34-23-57t-57-23q-34 0-57 23t-23 57q0 34 23 57t57 23ZM480-177Z"/></svg>`,
    "wifi-0"    :`<svg xmlns="http://www.w3.org/2000/svg" height="24px" width="24px" viewBox="0 -960 960 960" fill="#808080"><path d="M480-120 0-600q95-97 219.5-148.5T480-800q136 0 260.5 51.5T960-600L480-120Zm0-114 364-364q-79-60-172-91t-192-31q-99 0-192 31t-172 91l364 364Z"/></svg>`,
    "wifi-1"    :`<svg xmlns="http://www.w3.org/2000/svg" height="24px" width="24px" viewBox="0 -960 960 960" fill="#808080"><path d="M480-120 0-600q96-98 220-149t260-51q137 0 261 51t219 149L480-120ZM361-353q25-18 55.5-28t63.5-10q33 0 63.5 10t55.5 28l245-245q-78-59-170.5-90.5T480-720q-101 0-193.5 31.5T116-598l245 245Z"/></svg>`,
    "wifi-2"    :`<svg xmlns="http://www.w3.org/2000/svg" height="24px" width="24px" viewBox="0 -960 960 960" fill="#808080"><path d="M480-120 0-600q96-98 220-149t260-51q137 0 261 51t219 149L480-120ZM299-415q38-28 84-43.5t97-15.5q51 0 97 15.5t84 43.5l183-183q-78-59-170.5-90.5T480-720q-101 0-193.5 31.5T116-598l183 183Z"/></svg>`,
    "wifi-3"    :`<svg xmlns="http://www.w3.org/2000/svg" height="24px" width="24px" viewBox="0 -960 960 960" fill="#808080"><path d="M480-120 0-600q96-98 220-149t260-51q137 0 261 51t219 149L480-120ZM232-482q53-38 116-59.5T480-563q69 0 132 21.5T728-482l116-116q-78-59-170.5-90.5T480-720q-101 0-193.5 31.5T116-598l116 116Z"/></svg>`,
    "wifi-4"    :`<svg xmlns="http://www.w3.org/2000/svg" height="24px" width="24px" viewBox="0 -960 960 960" fill="#808080"><path d="M480-120 0-600q95-97 219.5-148.5T480-800q136 0 260.5 51.5T960-600L480-120Z"/></svg>`,
  };
  /*====================================================================
    CORE FUNCTIONS
    ====================================================================*/
  
  /**
   * Sends a request to the `/start-wifi` endpoint with SSID, password, and protection status.
   * After saving, it waits for connection status via checkConnectionStatus().
   */
  async function connect(ssid, pass, isProtected) {
    if (isConnecting) return; // Prevent multiple connection attempts
  
    try {
      // Update flags and UI
      isConnecting = true;
      elements.connectBtn.disabled = true;
      elements.connectBtn.textContent = "connecting...";
  
      const response = await fetch('/start-wifi', {
        method: 'POST',
        headers: {'Content-Type': 'application/x-www-form-urlencoded'},
        body: new URLSearchParams({ ssid, password: pass, isProtected })
      });
  
      const message = await response.text();
  
      // If the backend responds with OK, wait for the connection to establish
      if (response.ok) {
        showMsg(message);
        setTimeout(checkConnectionStatus, 3000);
      } else {
        // Error saving network details
        showMsg(message, 'error');
        resetConnectState('Connect');
      }
    } catch (error) {
      // Handle fetch or parsing exceptions
      showMsg('Connection failed', 'error');
      resetConnectState('Connect');
    }
  }
  
  /**
   * Initiates or monitors the network scan process by calling `/start-scan` and `/scan-status`.
   */
  async function scanNetworks(skipScan) {
    try {
      // If not already scanning, request the backend to start scanning
      if (!isScanning) {
        isScanning = true;
        elements.scanBtn.classList.add('loading');
  
        if(!skipScan)
        {
          const res_startScan = await fetch('/start-scan');
          if (!res_startScan.ok)
            throw new Error('Error starting Scan');
        }
      }
  
      // Check the scan status
      const res_scanStatus = await fetch('/scan-status');
      let networks;
  
      switch (res_scanStatus.status) {
        case 202: // Scan in progress, wait 3 seconds and retry
          elements.list.innerHTML = '<div class="scanning">Scanning in progress...</div>';
          setTimeout(scanNetworks, 3000);
          return;
  
        case 500: // Scan error
          stopScan();
          throw new Error(await res_scanStatus.text());
  
        case 200: // Scan completed successfully
          networks = await res_scanStatus.json();
          stopScan();
          break;
  
        default: // Unexpected status
          stopScan();
          throw new Error(`Unexpected status code: ${res_scanStatus.status}`);
      }
  
      // If no networks were found, retry up to the maximum limit
      if (networks.length === 0) {
        scanAttempts++;
        if (scanAttempts >= maxScanAttempts) {
          elements.list.innerHTML = '<div class="error">No networks found. Please try again later.</div>';
          elements.scanBtn.classList.remove('loading');
          return;
        }
        setTimeout(scanNetworks, 3000);
        return;
      }
  
      // Networks found! Render the network list
      elements.list.innerHTML = '';
      networks.forEach(network => {
        elements.list.appendChild(createNetworkElement(network));
      });
      scanAttempts = 0; // Reset the retry counter
  
    } catch (error) {
      // Handle fetch or backend status errors
      showMsg('Scan failed', 'error');
      elements.list.innerHTML = '<div class="error">Failed to scan networks</div>';
    }
  }
  
  /**
   * Checks the connection status by calling `/wifi-status`.
   * Displays progress, success, or error messages.
   */
  async function checkConnectionStatus() {
    try {
      const response = await fetch('/wifi-status');
      const message = await response.text();
  
      switch (response.status) {
        case 202: // Connection in progress
          showMsg(message, '', 6000); // Display for a longer time
          setTimeout(checkConnectionStatus, 3000);
          break;
  
        case 500: // Connection error
          showMsg(message, 'error');
          resetConnectState('Connect');
          break;
  
        case 200: // Connection successful
          showMsg(message, 'success', 10000);
          resetConnectState('Connect');
          elements.modal.className = 'password-modal'; // Close the modal
          break;
  
        default: // Unexpected status
          showMsg(`Unexpected response: ${response.status}`, 'error');
          resetConnectState('Connect');
          break;
      }
    } catch (error) {
      // Handle exceptions during connection check
      showMsg('Error checking connection status', 'error');
      resetConnectState('Connect');
    }
  }
  
  /*====================================================================
    SUPPORTING FUNCTIONS
    ====================================================================*/
  
  /**
   * Creates an HTML element to display a found network with icon and signal info.
   * On click, calls selectNetwork() to initiate connection.
   */
  function createNetworkElement(network) {
    const div = document.createElement('div');
    div.className = 'wifi-item';

    const signalNumber = getSignalIcon(network.rssi);
    const signalIcon = icons[`wifi-${signalNumber}`];
    const lockIcon   = network.isProtected ? icons["lock-wifi"] : '';

    console.log(signalIcon);

    div.innerHTML = `
      <div class="wifi-icon">
        ${signalIcon}
      </div>
      <div class="wifi-info">
        <div class="wifi-name">
          ${network.ssid}
          ${lockIcon}
        </div>
        <div class="wifi-signal">${network.rssi}dBm</div>
      </div>
    `;
  
    div.addEventListener('click', () => selectNetwork(network.ssid, network.isProtected));
    return div;
  }
  
  /**
   * Handles user interaction when selecting a network:
   * - If open, directly calls connect() without a password.
   * - If protected, shows a modal for password input.
   */
  function selectNetwork(ssid, isProtected) {
    if (isConnecting) return; // Prevent multiple connections
  
    if (!isProtected) {
      connect(ssid, '', isProtected); // Open network
      return;
    }
  
    // Protected network: show modal for password input
    elements.netName.textContent = ssid;
    elements.modal.className = 'password-modal active';
    elements.passInput.value = '';
    elements.passInput.focus();
  
    // Replace the Connect button to remove old event listeners
    const newConnectBtn = elements.connectBtn.cloneNode(true);
    elements.connectBtn.parentNode.replaceChild(newConnectBtn, elements.connectBtn);
    elements.connectBtn = newConnectBtn;
  
    // Handle click or Enter key on password input
    elements.connectBtn.addEventListener('click', () => {
      const pass = elements.passInput.value;
      if (pass) connect(ssid, pass, isProtected);
    });
  }
  
  /*====================================================================
    UI AND UTILITY FUNCTIONS
    ====================================================================*/
  
  /**
   * Displays a message in the .status-message element for a defined timeout.
   * Type can be 'error', 'success', or empty.
   */
  function showMsg(msg, type = '', timeout = 3000) {
    elements.statusMsg.textContent = msg;
    elements.statusMsg.className = `status-message active ${type}`;
    setTimeout(() => {
      elements.statusMsg.className = 'status-message';
    }, timeout);
  }
  
  /**
   * Returns a number indicating signal strength (0-4) based on RSSI.
   * This number is used in the icon name (wifi-N.svg).
   */
  function getSignalIcon(rssi) {
    if (rssi >= -50) return 4;
    if (rssi >= -60) return 3;
    if (rssi >= -70) return 2;
    if (rssi >= -80) return 1;
    return 0;
  }
  
  /**
   * Disables a button and changes its text. To re-enable, use resetConnectState() or manually.
   */
  function disableButton(btn, newText) {
    btn.disabled = true;
    btn.textContent = newText;
  }
  
  /**
   * Resets the app to its normal connection state, enabling the Connect button and setting its text.
   */
  function resetConnectState(btnText) {
    isConnecting = false;
    elements.connectBtn.disabled = false;
    elements.connectBtn.textContent = btnText;
  }
  
  /**
   * Stops the scanning process if in progress, clearing the loading state of the button.
   */
  function stopScan() {
    isScanning = false;
    elements.scanBtn.classList.remove('loading');
  }
  
  /*====================================================================
    GENERAL EVENT LISTENERS (LOAD EVERYTHING)
    ====================================================================*/
  
  document.addEventListener("DOMContentLoaded", () => {
    const scanButton = document.getElementById('scan-button');
    scanButton.innerHTML = icons["wifi-find"];

    const favIcon = document.getElementById('favicon');
    favIcon.href = "data:image/svg+xml," + encodeURIComponent(icons["wifi-find"]);

    
  });

  // Trigger scanning on "Scan" button click
  elements.scanBtn.addEventListener('click', scanNetworks);
  
  // Handle "Cancel" button click in the password modal
  elements.cancelBtn.addEventListener('click', () => {
    if (!isConnecting) {
      elements.modal.className = 'password-modal';
    }
  });
  
  // Trigger Connect on Enter key press in password input
  elements.passInput.addEventListener('keydown', (e) => {
    if (e.key === 'Enter' && !isConnecting && elements.passInput.value) {
      elements.connectBtn.click();
    }
  });
  
  // Automatically start scanning when the page loads
  scanNetworks(true);