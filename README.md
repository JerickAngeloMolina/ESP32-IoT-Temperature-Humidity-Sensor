# ESP32-IoT-Temperature-Humidity-Sensor

A smart, network-connected IoT sensor node that monitors real-time temperature and humidity and logs the data directly to a cloud-hosted Google Sheet. 

This project bypasses the need for a dedicated backend database by leveraging **Google Apps Script** as a serverless API endpoint, showcasing full-stack IoT integration, secure HTTPS requests, and low-power hardware design.

---

## 🏗️ System Architecture

The data flows through a 3-stage pipeline to achieve serverless logging:

1. **Edge (Hardware):** The microcontroller reads ambient temperature and humidity.
2. **Network (HTTPS):** The node connects to local Wi-Fi and sends a secure HTTP POST/GET request containing the sensor payload.
3. **Cloud (API & Database):** A custom Google Apps Script Web App receives the payload, parses the JSON/Query parameters, and appends a new row with a timestamp to Google Sheets.

---

## 🛠️ Hardware Specifications

* **Microcontroller:** [ESP32] (chosen for onboard Wi-Fi capabilities)
* **Sensor:** [DHT11] (I2C/One-Wire communication)
* **Power Source:** [USB Power]

### Pin Connection Diagram

| Device Pin | Microcontroller Pin | Function |
|---|---|---|
| **VCC** | 3.3V | Power Supply |
| **GND** | GND | Ground |
| **DATA** | [GPIO 14] | Sensor Data Signal |

---

## ☁️ Cloud & API Setup (Google Apps Script)

To interface the hardware with Google Sheets securely, a Google Apps Script was deployed as a Web App to act as the API gateway.

### The Deployment Process:
1. Created a dedicated Google Sheet with headers: `Timestamp | Temperature (°C) | Humidity (%)`.
2. Opened **Extensions > Apps Script** and implemented the following handler to receive incoming sensor data:

```javascript
function doGet(e) {
  var sheet = SpreadsheetApp.getActiveSpreadsheet().getActiveSheet();
  
  // Extract parameters sent by the microcontroller
  var temp = e.parameter.temp;
  var humid = e.parameter.humid;
  
  if (temp !== undefined && humid !== undefined) {
    // Append data along with a server-side timestamp
    sheet.appendRow([new Date(), temp, humid]);
    return ContentService.createTextOutput("Success");
  }
  return ContentService.createTextOutput("Error: Missing Parameters");
}
