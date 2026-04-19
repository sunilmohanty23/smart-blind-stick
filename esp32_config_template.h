/**
 * ============================================
 * CONFIGURATION TEMPLATE - Smart Blind Stick
 * ============================================
 * 
 */

#ifndef ESP32_CONFIG_H
#define ESP32_CONFIG_H

// ============================================
// 1. THINGSPEAK SETTINGS
// ============================================
// Get your free API key from: https://thingspeak.com
// Create a channel, then copy the Write API Key
const char* THINGSPEAK_API_KEY = "YOUR_THINGSPEAK_WRITE_API_KEY";

// ============================================
// 2. GPRS APN SETTINGS (for SIM800L)
// ============================================
// VI (Vodafone Idea): apn = "www"
// Airtel: apn = "airtelgprs.com"
// BSNL: apn = "bsnlnet"
const char* GPRS_APN = "www";           // Change based on your SIM
const char* GPRS_USER = "";              // Leave empty for VI/Airtel
const char* GPRS_PASS = "";              // Leave empty for VI/Airtel

// ============================================
// 3. EMERGENCY CONTACT NUMBERS (4 Numbers)
// ============================================
// Format: With country code, no spaces or dashes
// India: +91 followed by 10-digit number
// Example: +911234567890
const char* EMERGENCY_NUMBERS[] = {
  "+911234567890",   // Contact 1 - Primary guardian
  "+911234567891",   // Contact 2 - Secondary guardian
  "+911234567892",   // Contact 3 - Family member
  "+911234567893"    // Contact 4 - Friend/Neighbor
};

// Number of emergency contacts
const int NUM_EMERGENCY_CONTACTS = 4;

// ============================================
// 4. CALL SETTINGS
// ============================================
// Duration to let the phone ring before hanging up (milliseconds)
const unsigned long CALL_RING_DURATION = 15000;   // 15 seconds

// ============================================
// 5. THINGSPEAK UPDATE INTERVAL
// ============================================
// How often to send GPS and battery data to cloud (milliseconds)
const unsigned long THINGSPEAK_SEND_INTERVAL = 15000;   // 15 seconds

// ============================================
// 6. BATTERY MONITORING SETTINGS
// ============================================
// Voltage divider ratio (R1 = R2 = 100kΩ)
const float VOLTAGE_DIVIDER_RATIO = 2.0;

// Battery voltage range for percentage calculation
const float BATTERY_MAX_VOLTAGE = 4.2;   // Fully charged (100%)
const float BATTERY_MIN_VOLTAGE = 3.3;   // Discharged (0%)

// How often to read battery voltage (milliseconds)
const unsigned long BATTERY_READ_INTERVAL = 10000;   // 10 seconds

// ============================================
// 7. GPS SETTINGS
// ============================================
// GPS module baud rate (NEO-6M default is 9600)
const long GPS_BAUD_RATE = 9600;

// Maximum time to wait for GPS fix before sending SMS (milliseconds)
const unsigned long GPS_FIX_TIMEOUT = 30000;   // 30 seconds

// ============================================
// 8. SERIAL MONITOR SETTINGS
// ============================================
const long SERIAL_BAUD_RATE = 115200;

// ============================================
// 9. PIN MAPPINGS (Hardware Connections)
// ============================================
// GPS connections (UART2)
#define GPS_RX_PIN 16
#define GPS_TX_PIN 17

// GSM connections (UART1)
#define GSM_RX_PIN 26
#define GSM_TX_PIN 27

// SOS Button
#define SOS_BUTTON_PIN 5

// Battery ADC pins
#define BATTERY_1_ADC_PIN 34
#define BATTERY_2_ADC_PIN 35

#endif  // ESP32_CONFIG_H