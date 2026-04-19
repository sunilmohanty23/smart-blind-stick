/**
 * ============================================
 * SMART BLIND STICK DASHBOARD - JAVASCRIPT
 * Fetches live data from ThingSpeak API
 * Updates: Location Map, Battery Status, Device Status
 * ============================================
 */

// ========== CONFIGURATION – REPLACE WITH YOUR ACTUAL VALUES ==========
const CHANNEL_ID = "YOUR_CHANNEL_ID";           // ThingSpeak Channel ID (e.g., "3316526")
const READ_API_KEY = "YOUR_READ_API_KEY";       // ThingSpeak Read API Key (e.g., "CFBB3XHOQOCRIL9H")
const STALE_MS = 45000;                         // 45 seconds threshold (device considered stale after this)


// ========== DOM ELEMENT REFERENCES ==========
// Map elements
const mapFrame = document.getElementById('mapFrame');
const coordOverlay = document.getElementById('coordOverlay');

// Status display elements
const latField = document.getElementById('latField');
const lonField = document.getElementById('lonField');
const deviceStatusField = document.getElementById('deviceStatusField');
const lastUpdateField = document.getElementById('lastUpdateField');
const staleHintSpan = document.getElementById('staleHint');

// Status badge elements
const statusLed = document.getElementById('statusLed');
const statusTextSpan = document.getElementById('statusText');

// Live tag elements (map overlay)
const liveTag = document.getElementById('liveTag');
const livePulse = document.getElementById('livePulse');
const liveTagText = document.getElementById('liveTagText');

// Battery elements
const batteryPercentSpan = document.getElementById('batteryPercent');
const batteryFill = document.getElementById('batteryFill');
const batteryTag = document.getElementById('batteryTag');

// ========== HELPER FUNCTIONS ==========

/**
 * Updates the live status tag on the map overlay
 * @param {boolean} isOnline - True if device is online
 * @param {boolean} isStale - True if data is stale (old)
 */
function updateLiveTag(isOnline, isStale = false) {
  if (isOnline) {
    liveTag.className = 'live-tag online';
    livePulse.className = 'pulse';
    liveTagText.innerText = 'LIVE';
  } else if (isStale) {
    liveTag.className = 'live-tag warning';
    livePulse.className = 'pulse stopped';
    liveTagText.innerText = 'STALE';
  } else {
    liveTag.className = 'live-tag offline';
    livePulse.className = 'pulse stopped';
    liveTagText.innerText = 'OFFLINE';
  }
}

/**
 * Updates the battery UI (percentage bar and status text)
 * @param {number} percentage - Battery percentage (0-100)
 */
function updateBatteryUI(percentage) {
  let p = Math.min(100, Math.max(0, parseInt(percentage) || 0));
  batteryPercentSpan.innerText = p + '%';
  batteryFill.style.width = p + '%';
  
  let statusMsg = '';
  if (p >= 70) {
    statusMsg = 'Excellent';
  } else if (p >= 40) {
    statusMsg = 'Good';
  } else if (p >= 15) {
    statusMsg = 'Low';
  } else {
    statusMsg = 'Critical - Charge Now!';  // Shows for 0% battery
  }
  batteryTag.innerText = statusMsg;
}

/**
 * Updates the entire dashboard with new data
 * @param {number} lat - Latitude
 * @param {number} lon - Longitude
 * @param {string} timestampISO - ISO timestamp of last update
 * @param {number} batteryPercent - Battery percentage
 */
function updateDashboard(lat, lon, timestampISO, batteryPercent) {
  // Format coordinates to 6 decimal places
  const latFixed = parseFloat(lat).toFixed(6);
  const lonFixed = parseFloat(lon).toFixed(6);
  
  // Update text displays
  latField.innerText = latFixed;
  lonField.innerText = lonFixed;
  coordOverlay.innerText = `📍 Lat: ${latFixed} | Lon: ${lonFixed}`;
  
  // Update Google Maps iframe (only if coordinates changed)
  const newMapSrc = `https://maps.google.com/maps?q=${latFixed},${lonFixed}&z=15&output=embed`;
  if (!mapFrame.src.includes(`q=${latFixed},${lonFixed}`)) {
    mapFrame.src = newMapSrc;
  }
  
  // Update battery display
  updateBatteryUI(batteryPercent);
  
  // Process timestamp and determine device status
  let lastDate = timestampISO ? new Date(timestampISO) : null;
  let now = new Date();
  
  if (lastDate && !isNaN(lastDate.getTime())) {
    const diffMs = now - lastDate;
    const isFresh = diffMs <= STALE_MS;
    lastUpdateField.innerText = lastDate.toLocaleString();
    
    if (isFresh) {
      // Device is online with fresh data
      deviceStatusField.innerHTML = '✅ Online';
      statusLed.className = 'led online';
      statusTextSpan.innerText = 'Device Online';
      staleHintSpan.innerHTML = '';
      updateLiveTag(true, false);
    } else {
      // Data is stale (device may be offline)
      const secAgo = Math.floor(diffMs / 1000);
      deviceStatusField.innerHTML = `⚠️ Stale (${secAgo}s ago)`;
      statusLed.className = 'led warning';
      statusTextSpan.innerText = 'Stale data';
      staleHintSpan.innerText = 'Data older than 45 sec';
      updateLiveTag(false, true);
    }
  } else {
    // No valid timestamp
    lastUpdateField.innerText = 'No timestamp';
    deviceStatusField.innerHTML = '❓ No data';
    statusLed.className = 'led offline';
    statusTextSpan.innerText = 'Offline';
    staleHintSpan.innerText = 'Waiting for ThingSpeak';
    updateLiveTag(false, false);
  }
}

/**
 * Fallback function – uses default coordinates and 0% battery when API fails
 */
function fallbackToDefault() {
  // Use default coordinates and 0% battery
  updateDashboard(DEFAULT_LAT, DEFAULT_LON, null, 0);   // Battery set to 0%
  lastUpdateField.innerText = 'Default location';
  deviceStatusField.innerHTML = '⚠️ Fallback mode (No data)';
  statusLed.className = 'led offline';
  statusTextSpan.innerText = 'API Error';
  staleHintSpan.innerText = 'Using default coordinates - Battery 0%';
  updateLiveTag(false, false);
  
  // Update map to default location
  const defaultSrc = `https://maps.google.com/maps?q=${DEFAULT_LAT.toFixed(6)},${DEFAULT_LON.toFixed(6)}&z=15&output=embed`;
  if (!mapFrame.src.includes(`${DEFAULT_LAT.toFixed(6)},${DEFAULT_LON.toFixed(6)}`)) {
    mapFrame.src = defaultSrc;
  }
}

// ========== THINGSPEAK API FETCH ==========

/**
 * Fetches the latest data from ThingSpeak API
 * Maps: field1 → Latitude, field2 → Longitude, field3 → Battery %
 */
async function fetchThingSpeakData() {
  const apiUrl = `https://api.thingspeak.com/channels/${CHANNEL_ID}/feeds/last.json?api_key=${READ_API_KEY}`;
  
  try {
    // Setup abort controller for timeout (10 seconds)
    const controller = new AbortController();
    const timeoutId = setTimeout(() => controller.abort(), 10000);
    
    const res = await fetch(apiUrl, { signal: controller.signal });
    clearTimeout(timeoutId);
    
    if (!res.ok) throw new Error(`HTTP ${res.status}`);
    const data = await res.json();
    
    // Extract data from ThingSpeak fields
    let lat = parseFloat(data.field1);      // Latitude from field1
    let lon = parseFloat(data.field2);      // Longitude from field2
    let battery = parseInt(data.field3);    // Battery % from field3
    
    // Validate data
    if (isNaN(lat) || isNaN(lon)) throw new Error('Invalid coordinates');
    if (isNaN(battery)) battery = 0;        // Default battery to 0% if missing
    
    const createdAt = data.created_at;      // ISO timestamp
    updateDashboard(lat, lon, createdAt, battery);
    
  } catch (err) {
    console.warn('ThingSpeak fetch error:', err);
    fallbackToDefault();  // Shows 0% battery when API fails
  }
}

// ========== USER INTERACTIONS ==========

/**
 * Manual refresh triggered by button click
 */
async function refreshData() {
  statusLed.className = 'led warning';
  statusTextSpan.innerText = 'Fetching...';
  await fetchThingSpeakData();
}

/**
 * Opens Google Maps in a new tab with current coordinates
 */
function openFullMap() {
  let lat = latField.innerText;
  let lon = lonField.innerText;
  
  // Use default coordinates if current values are invalid
  if (lat === '--' || lon === '--' || isNaN(parseFloat(lat))) {
    lat = DEFAULT_LAT;
    lon = DEFAULT_LON;
  }
  window.open(`https://www.google.com/maps?q=${lat},${lon}`, '_blank');
}

// ========== AUTO-REFRESH TIMER ==========

let refreshInterval;

/**
 * Starts automatic refresh every 15 seconds
 */
function startAutoRefresh() {
  if (refreshInterval) clearInterval(refreshInterval);
  fetchThingSpeakData();                    // Initial fetch
  refreshInterval = setInterval(fetchThingSpeakData, 15000); // 15 seconds
}

// ========== EVENT LISTENERS ==========
document.getElementById('refreshBtn').addEventListener('click', refreshData);
document.getElementById('fullMapBtn').addEventListener('click', openFullMap);

// ========== INITIALIZATION ==========
startAutoRefresh();