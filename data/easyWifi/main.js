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
    div.innerHTML = `
      <div class="wifi-icon">
        <img src="easyWifi/icons/wifi-${getSignalIcon(network.rssi)}.svg" alt="Signal" width="24" height="24">
      </div>
      <div class="wifi-info">
        <div class="wifi-name">
          ${network.ssid}
          ${network.isProtected ? '<img src="easyWifi/icons/lock-wifi.svg" alt="Locked" width="16" height="16">' : ''}
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