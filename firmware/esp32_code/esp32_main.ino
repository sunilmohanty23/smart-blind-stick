/**
 * ============================================
 * SMART BLIND STICK - GPRS VERSION
 * Features:
 * - GPS (NEO-6M) with real-time location
 * - 2 Battery monitoring (average percentage)
 * - ThingSpeak upload via GPRS (Lat, Lon, Avg Battery %)
 * - Emergency button (GPIO5) sends SMS + Call to 4 numbers
 * - SMS contains Google Maps link with exact GPS location
 * - Call rings for 15 seconds then hangs up
 * ============================================
 */

#define TINY_GSM_MODEM_SIM800
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <TinyGsmClient.h>

// ========== GPRS APN for Vi (Vodafone Idea) ==========
// REPLACE WITH YOUR APN SETTINGS
const char apn[]  = "XXX";           //Replace with Actual APN and use 2G Enable Sim Card
const char gprsUser[] = "";          // Leave empty for Vi
const char gprsPass[] = "";          // Leave empty for Vi

// ========== ThingSpeak Settings ==========
const char server[] = "api.thingspeak.com";
const String writeAPIKey = "YOUR_THINGSPEAK_WRITE_API_KEY";  // REPLACE THIS
const unsigned long sendInterval = 15000;   // 15 seconds

// ========== GPS Pins (HardwareSerial UART2) ==========
#define GPS_RX 16
#define GPS_TX 17
HardwareSerial gpsSerial(2);
TinyGPSPlus gps;

// ========== GSM/GPRS Pins (HardwareSerial UART1) ==========
#define GSM_RX 26
#define GSM_TX 27
HardwareSerial gsmSerial(1);
TinyGsm modem(gsmSerial);
TinyGsmClient client(modem);

// ========== Emergency Button ==========
#define BUTTON_PIN 5
bool emergencyTriggered = false;
unsigned long lastDebounce = 0;
const unsigned long debounceDelay = 200;

// ========== Emergency Contacts (4 Numbers) ==========
// REPLACE WITH ACTUAL PHONE NUMBERS (with country code)
const char* phoneNumbers[] = {
  "+911234567890",   // Contact 1
  "+911234567891",   // Contact 2
  "+911234567892",   // Contact 3
  "+911234567893"    // Contact 4
};
const int numNumbers = 4;

// ========== 2 Battery Monitoring (Average) ==========
#define BAT1_PIN 34
#define BAT2_PIN 35
#define VOLTAGE_DIVIDER 2.0    // R1=R2=100kΩ
#define MAX_VOLTAGE 4.2
#define MIN_VOLTAGE 3.3

float bat1Voltage = 0, bat2Voltage = 0;
int bat1Percent = 0, bat2Percent = 0;
int avgBattery = 0;
unsigned long lastBatteryRead = 0;
const unsigned long batteryReadInterval = 10000;   // every 10 sec

// ========== Location Variables ==========
float currentLat = 0, currentLon = 0;
bool hasGpsFix = false;
unsigned long lastThingSpeakSend = 0;
unsigned long lastGpsCheck = 0;

// ========== Non-blocking Call State Machine ==========
enum CallState { CALL_IDLE, CALL_DIALING, CALL_RINGING, CALL_HANGUP };
CallState callState = CALL_IDLE;
unsigned long callRingStart = 0;
const unsigned long callRingDuration = 15000;   // 15 seconds
int currentCallIndex = 0;
bool emergencySmsSent = false;

// ========== SETUP ==========
void setup() {
  Serial.begin(115200);
  
  // Initialize GPS
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  
  // Initialize GSM
  gsmSerial.begin(9600, SERIAL_8N1, GSM_RX, GSM_TX);
  
  // Initialize Battery ADC
  analogReadResolution(12);
  pinMode(BAT1_PIN, INPUT);
  pinMode(BAT2_PIN, INPUT);
  
  // Initialize Button
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  Serial.println("\n================================================");
  Serial.println("Smart Blind Stick - GPRS Version");
  Serial.println("================================================\n");
  
  // Initialize GPRS connection
  initGPRS();
  
  Serial.println("GPS initialized. Waiting for fix...");
  Serial.println("Emergency button ready (GPIO5). Press to send SOS.");
}

// ========== GPRS INITIALIZATION ==========
void initGPRS() {
  Serial.println("Initializing GSM modem...");
  if (!modem.restart()) {
    Serial.println("❌ Modem restart failed. Check wiring & power!");
    while (true) delay(1000);
  }
  Serial.println("✅ Modem restarted");

  // Optional: Unlock SIM if PIN protected
  // if (modem.getSimStatus() != 3) { modem.simUnlock("1234"); }

  Serial.print("Waiting for network registration");
  bool registered = false;
  for (int i = 0; i < 30; i++) {
    gsmSerial.println("AT+CREG?");
    delay(500);
    String resp = "";
    unsigned long start = millis();
    while (millis() - start < 2000) {
      while (gsmSerial.available()) {
        resp += (char)gsmSerial.read();
      }
    }
    if (resp.indexOf("+CREG: 0,1") != -1 || resp.indexOf("+CREG: 0,5") != -1) {
      registered = true;
      break;
    }
    Serial.print(".");
    delay(1000);
  }
  
  if (registered) {
    Serial.println("\n✅ Registered on network");
  } else {
    Serial.println("\n❌ Registration failed. Check SIM, antenna, and 2G signal.");
    while (true) delay(1000);
  }

  Serial.print("Connecting to GPRS (APN: ");
  Serial.print(apn);
  Serial.print(")...");
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    Serial.println(" fail. Check APN & data plan.");
    while (true) delay(1000);
  }
  Serial.println(" OK");
  Serial.print("GPRS connected. Local IP: ");
  Serial.println(modem.getLocalIP());
}

// ========== MAIN LOOP ==========
void loop() {
  // Read GPS continuously
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }
  
  // Update GPS status
  hasGpsFix = gps.location.isValid();
  if (hasGpsFix) {
    currentLat = gps.location.lat();
    currentLon = gps.location.lng();
    lastGpsCheck = millis();
  }
  
  // Read batteries periodically
  if (millis() - lastBatteryRead >= batteryReadInterval) {
    readBatteries();
    lastBatteryRead = millis();
  }
  
  // Send to ThingSpeak every 15 seconds (if GPRS connected)
  if (modem.isGprsConnected() && (millis() - lastThingSpeakSend >= sendInterval)) {
    sendToThingSpeak(currentLat, currentLon, avgBattery);
    lastThingSpeakSend = millis();
  }
  
  // Check GPRS connection health
  static unsigned long lastGprsCheck = 0;
  if (millis() - lastGprsCheck >= 30000) {
    if (!modem.isGprsConnected()) {
      Serial.println("⚠️ GPRS lost, reconnecting...");
      modem.gprsConnect(apn, gprsUser, gprsPass);
    }
    lastGprsCheck = millis();
  }
  
  // Emergency button detection
  bool buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW && !emergencyTriggered && (millis() - lastDebounce > debounceDelay)) {
    emergencyTriggered = true;
    lastDebounce = millis();
    Serial.println("\n⚠️⚠️⚠️ EMERGENCY BUTTON PRESSED! ⚠️⚠️⚠️");
    startEmergencySequence();
    emergencyTriggered = false;
  }
  
  // Non-blocking call handler
  handleCallState();
  
  // Print status every 10 seconds
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint >= 10000) {
    printStatus();
    lastPrint = millis();
  }
  
  delay(10);
}

// ========== BATTERY READING (2 batteries, average) ==========
void readBatteries() {
  // Battery 1
  int adc1 = analogRead(BAT1_PIN);
  float pinV1 = (adc1 / 4095.0) * 3.3;
  bat1Voltage = pinV1 * VOLTAGE_DIVIDER;
  bat1Percent = (bat1Voltage - MIN_VOLTAGE) * 100 / (MAX_VOLTAGE - MIN_VOLTAGE);
  bat1Percent = constrain(bat1Percent, 0, 100);
  
  // Battery 2
  int adc2 = analogRead(BAT2_PIN);
  float pinV2 = (adc2 / 4095.0) * 3.3;
  bat2Voltage = pinV2 * VOLTAGE_DIVIDER;
  bat2Percent = (bat2Voltage - MIN_VOLTAGE) * 100 / (MAX_VOLTAGE - MIN_VOLTAGE);
  bat2Percent = constrain(bat2Percent, 0, 100);
  
  // Average
  avgBattery = (bat1Percent + bat2Percent) / 2;
}

// ========== THINGSPEAK UPLOAD ==========
void sendToThingSpeak(float lat, float lon, int battery) {
  if (!client.connect(server, 80)) {
    Serial.println("❌ ThingSpeak connection failed");
    return;
  }
  
  String url = "/update?api_key=" + writeAPIKey +
               "&field1=" + String(lat, 6) +
               "&field2=" + String(lon, 6) +
               "&field3=" + String(battery);
  
  client.print("GET ");
  client.print(url);
  client.println(" HTTP/1.1");
  client.print("Host: ");
  client.println(server);
  client.println("Connection: close");
  client.println();
  
  unsigned long timeout = millis() + 5000;
  while (client.connected() && millis() < timeout) {
    if (client.available()) {
      String line = client.readStringUntil('\n');
      if (line.indexOf("HTTP/1.1 200") != -1) {
        Serial.println("✅ ThingSpeak updated");
      }
    }
  }
  client.stop();
}

// ========== EMERGENCY SEQUENCE (SMS + Call to all 4 numbers) ==========
void startEmergencySequence() {
  emergencySmsSent = false;
  currentCallIndex = 0;
  
  // First, send SMS to all 4 numbers with current location
  sendEmergencySMSToAll();
  
  // Then start calling the first number
  if (callState == CALL_IDLE) {
    startCall(phoneNumbers[0]);
  }
}

void sendEmergencySMSToAll() {
  // Use current GPS location (no fallback – wait for fix if needed)
  float lat = currentLat;
  float lon = currentLon;
  String locationText;
  
  if (hasGpsFix && lat != 0 && lon != 0) {
    locationText = "https://maps.google.com/?q=" + String(lat, 6) + "," + String(lon, 6);
  } else {
    // If no GPS fix, wait a moment and try again
    Serial.println("Waiting for GPS fix before sending SMS...");
    unsigned long waitStart = millis();
    while (!hasGpsFix && (millis() - waitStart < 30000)) {
      while (gpsSerial.available()) {
        gps.encode(gpsSerial.read());
      }
      hasGpsFix = gps.location.isValid();
      if (hasGpsFix) {
        lat = gps.location.lat();
        lon = gps.location.lng();
        break;
      }
      delay(500);
    }
    
    if (hasGpsFix && lat != 0 && lon != 0) {
      locationText = "https://maps.google.com/?q=" + String(lat, 6) + "," + String(lon, 6);
    } else {
      locationText = "GPS signal lost - approximate location";
    }
  }
  
  String message = "🚨 EMERGENCY! Smart blind stick activated.\nLocation: " + locationText +
                   "\nAverage Battery: " + String(avgBattery) + "%";
  
  for (int i = 0; i < numNumbers; i++) {
    Serial.print("Sending SMS to ");
    Serial.println(phoneNumbers[i]);
    sendSMS(phoneNumbers[i], message);
    delay(1000);
  }
  emergencySmsSent = true;
}

// ========== SMS FUNCTION ==========
void sendSMS(const char* number, String message) {
  // Flush input buffer
  while (gsmSerial.available()) gsmSerial.read();
  
  gsmSerial.println("AT+CMGS=\"" + String(number) + "\"");
  delay(500);
  
  // Wait for '>' prompt
  unsigned long start = millis();
  bool prompt = false;
  while (millis() - start < 5000) {
    while (gsmSerial.available()) {
      if (gsmSerial.read() == '>') {
        prompt = true;
        break;
      }
    }
    if (prompt) break;
  }
  
  if (!prompt) {
    Serial.println("    ❌ No SMS prompt");
    return;
  }
  
  gsmSerial.print(message);
  delay(200);
  gsmSerial.write(26);   // Ctrl+Z
  
  if (waitResponse("+CMGS:", 10000))
    Serial.println("    ✅ SMS sent");
  else
    Serial.println("    ❌ SMS failed");
  
  delay(500);
  while (gsmSerial.available()) gsmSerial.read();
}

// ========== CALL FUNCTIONS (Non-blocking) ==========
void startCall(const char* number) {
  // Hang up any previous call
  gsmSerial.println("ATH");
  delay(500);
  while (gsmSerial.available()) gsmSerial.read();
  
  gsmSerial.println("ATD" + String(number) + ";");
  callState = CALL_DIALING;
  Serial.print("    Dialing ");
  Serial.println(number);
}

void handleCallState() {
  if (callState == CALL_IDLE) {
    // After call ends, move to next number if any
    if (emergencySmsSent && currentCallIndex < numNumbers - 1) {
      currentCallIndex++;
      startCall(phoneNumbers[currentCallIndex]);
    }
    return;
  }
  
  // Read responses from GSM module
  while (gsmSerial.available()) {
    String resp = gsmSerial.readString();
    if (resp.indexOf("OK") != -1 && callState == CALL_DIALING) {
      callState = CALL_RINGING;
      callRingStart = millis();
      Serial.println("    ✅ Call dialed, ringing...");
    }
    if (resp.indexOf("NO CARRIER") != -1 || resp.indexOf("BUSY") != -1) {
      Serial.println("    ❌ Call failed (no carrier/busy)");
      callState = CALL_IDLE;
      return;
    }
  }
  
  // Ringing phase – wait for duration then hang up
  if (callState == CALL_RINGING && (millis() - callRingStart >= callRingDuration)) {
    gsmSerial.println("ATH");
    Serial.println("    ✅ Call ended after ringing");
    callState = CALL_HANGUP;
  }
  
  // Small delay to ensure hangup is processed
  if (callState == CALL_HANGUP && (millis() - callRingStart >= callRingDuration + 500)) {
    callState = CALL_IDLE;
  }
}

// ========== HELPER FUNCTION ==========
bool waitResponse(String expected, int timeout) {
  unsigned long start = millis();
  while (millis() - start < timeout) {
    if (gsmSerial.available()) {
      String resp = gsmSerial.readString();
      if (resp.indexOf(expected) != -1) return true;
    }
  }
  return false;
}

// ========== STATUS PRINT ==========
void printStatus() {
  Serial.println("-----------------------------------------");
  if (hasGpsFix) {
    Serial.println("✅ GPS: FIXED");
    Serial.printf("   Lat: %.6f, Lon: %.6f\n", currentLat, currentLon);
    Serial.printf("   Satellites: %d\n", gps.satellites.value());
  } else {
    Serial.println("❌ GPS: NO FIX - Waiting for signal");
    Serial.println("   Emergency SMS will wait for GPS fix");
  }
  Serial.printf("🔋 Battery 1: %.2fV (%d%%) | Battery 2: %.2fV (%d%%)\n", 
                bat1Voltage, bat1Percent, bat2Voltage, bat2Percent);
  Serial.printf("🔋 Average Battery: %d%%\n", avgBattery);
  Serial.printf("📡 GPRS: %s\n", modem.isGprsConnected() ? "Connected" : "Disconnected");
  Serial.printf("📞 Call State: %d\n", callState);
  Serial.println("-----------------------------------------\n");
}