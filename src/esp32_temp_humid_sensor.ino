#include <Arduino.h>
#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include "time.h"
#include <ESP_Google_Sheet_Client.h>

// OLED Libraries
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// For SD/SD_MMC mounting helper
#include <GS_SDHelper.h>

#define WIFI_SSID "hotspotngpogi"
#define WIFI_PASSWORD "anlakingtitiko"

// Google Project ID
#define PROJECT_ID "iot-dataloggeresp32"

// Service Account's client email
#define CLIENT_EMAIL "esp32-datalogger@iot-dataloggeresp32.iam.gserviceaccount.com"

// Service Account's private key
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\n\
MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDXJ+10TvjCRUUO\n\
3pumJWgZLfe75sjNL9rUhxt5rZ30rDvgxLA3F491YtSAy/PHkUfFR1ERRyUnS2WT\n\
kcaSYfEUfcBVKjg9N3jHRO0K17Wln93aboDJx7v9KdbXXJAtkHwEdUQjV0s2ygcv\n\
wlEpIizp2W1R83wfnov0o5kd97stp9nis/vjyKG7QAHgBtnuXlS1Z8UsiZQ38BeX\n\
F9XJrPMDPzvtK4STA5QBu0IFXnh/UZ6S7GLghd4m5nxN5ACgv59CF6YU67WeIaC3\n\
DvphrENBEur98lYzY8fcY8blcEpyQvBaAK+6NeKQRq4YpF1zEK7pdxbb/qh+mKR1\n\
LZvh7ednAgMBAAECggEATVz4/+JJhik8LJ2UoAhBRxSFSJYyS71SVSsDkRwBePHZ\n\
UffHWPfqyI2x1WIAlRjDEseS+cB4Nongy1AWc5ouAm05FCZrNG/WyOgax9RKZ3R1\n\
ZE5plqRwDxbVPnj1Cr0yKrQzvPVtgRThwqV/Y2f7eFaK/XWsgv4SWvPcYa8rKb3R\nPcHTmtYo6jMtKDFDNOkncPKscFZ+09kpr1U1xNV/jJ4O6hnmFf2r9+Ag4j+GP4yD\n\
UShaF0PQ94D6W7HY/HP9StQM270jODiZtLyPr9qX/VJWRluxYBwQV6PIjSEpL+uC\noJtgCCXFlRLB5S0WEg+KpnwsfdN6szQ3yw0p4huZ6QKBgQDxW/C32qXX7WSLfhyA\n\
CVMeK6BahcLOmkxjLQmBn8UjAxwdnyuIuRwrAkodyenxiiu/YUPWcaUxerCViIub\nzJ+dVuwf4aL6xkrCF3ZcnQUs12oSnzAQA6OpPOVr8sZyq5q+UhmAkYU2LBGKjKOu\nnJbEk0Afxre3eVJrD8itO3yhtQKBgQDkNRQvPxkJF2RLyxTCGJ6/Uf5Ix1tlBihw\n\
3r4h/sj9B12fsCS47AQiRSF1ugWuSG4/FFSSCvot/TiFZLYr8ZZY3Jvu1ywElMUF\nha8DKgoRNcK/kmpeesfGBeGf3glicA5RYcFv7sdQ5m+0ZwX1wOqQF8MP36/8ihfb\nyCI7CjKGKwKBgQCgq06cZW4fRsW24lXAmfR90hbmC8M525dcMf/xDVWjUA+oXGwT\nkP6CVvzVxbL5erxSo0IQgAiy3nSspoAhT981U0bOllrzS4s6l3nQfyqRxjizesr4\n7iNFpucmrC+U6E2Twn19i+G8xStMKwFPXKg05b07KgLknVvTL2esgjwePQKBgHsQ\nnLf14PxkHvQ4qhMZz9IA145L49+Q+JIVrJcMnTrGlBifls4aiQgqG6cvEA0yhjGC\nNzEitlPCsI5PB2afO6LkJTsh0l0OFUmrE/wy2Yb6ZPGGddJJiB4j4c6ioJDzOOVu\nQDCqpdXczvSMckxbIqNeDRUsXe1kUwV04fDQyPpNAoGBANJIoKmJbb95kIMBKvfV\nQhAbQeB+2UHbs0U5ct4tXur6aY+UL1R112LH5i541nYiLsNUsIbdkDmf4Fd1THKA\neQNl23QjXkfYC2cvx/Rt3wLjzqjcSA5UUlzq3Zw25yx0V/s/CZieDFy/GgqauQOf\n4HNh+7qA8ICnZ2k2YyTpdmio\n-----END PRIVATE KEY-----\n";

const char spreadsheetId[] = "1WJBArbC8w7pP_yYIglzAlfPJ79kAFie2PfOt-ak1grQ";

// DHT11 Config
#define DHTPIN 14     
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);

// OLED Dimensions & Initialization
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1 // Sharing architecture reset pin
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Timing variables
unsigned long lastTime = 0;  
unsigned long timerDelay = 30000; 

unsigned long lastOledUpdate = 0;
unsigned long oledDelay = 1000; // Refreshes screen metrics every 1 second

void tokenStatusCallback(TokenInfo info);

float temp;
float hum;
String formattedTime; 

const char* ntpServer = "ph.pool.ntp.org";

String getTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Time Error";
  }
  char timeStringBuff[50];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timeStringBuff);
}

// Function dedicated to drawing text layouts on your OLED screen
void updateOLEDDisplay(String statusMsg) {
  display.clearDisplay();
  
  // Header Zone
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("--- MONITOR SYSTEM ---");
  
  // Print Temperature
  display.setCursor(0, 16);
  display.print("Temp: ");
  display.setTextSize(2); // Make value distinctively larger
  if(isnan(temp)) display.print("--");
  else display.print(temp, 1);
  display.setTextSize(1);
  display.println(" C");
  
  // Print Humidity
  display.setCursor(0, 36);
  display.print("Humid: ");
  display.setTextSize(2);
  if(isnan(hum)) display.print("--");
  else display.print(hum, 0);
  display.setTextSize(1);
  display.println(" %");

  // Bottom Status Text
  display.setCursor(0, 56);
  display.setTextSize(1);
  display.print(statusMsg);
  
  display.display(); // Push layout matrix to hardware
}

void setup(){
    Serial.begin(115200);

    // Initialize OLED Screen
    // 0x3C is the standard I2C address for 0.96 inch OLED screens
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
        Serial.println(F("SSD1306 allocation failed"));
        for(;;); // Freeze here if wiring layout is incorrect
    }
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,20);
    display.println("Booting System...");
    display.display();

    dht.begin();

    GSheet.printf("ESP Google Sheet Client v%s\n\n", ESP_GOOGLE_SHEET_CLIENT_VERSION);

    WiFi.setAutoReconnect(true);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
    }

    configTime(28800, 0, ntpServer); 
    
    struct tm timeinfo;
    while (!getLocalTime(&timeinfo)) {
      delay(500);
    }

    display.clearDisplay();
    display.setCursor(0,20);
    display.println("System Ready!");
    display.display();
    delay(1000);

    GSheet.setTokenCallback(tokenStatusCallback);
    GSheet.setPrerefreshSeconds(10 * 60);
    GSheet.begin(CLIENT_EMAIL, PROJECT_ID, PRIVATE_KEY);
}

void loop(){
    bool ready = GSheet.ready();
    
    // Non-blocking snapshot reading
    float t_read = dht.readTemperature();
    float h_read = dht.readHumidity();
    if (!isnan(t_read)) temp = t_read;
    if (!isnan(h_read)) hum = h_read;

    // Fast Loop: Refresh screen metrics locally every 1 second
    if (millis() - lastOledUpdate > oledDelay) {
        lastOledUpdate = millis();
        String current_time = getTime();
        // Discards date segment, crops to just HH:MM
        String time_short = current_time.substring(11, 16); 
        updateOLEDDisplay("Time: " + time_short);
    }

    // Slow Loop: Append payload block to Google Sheets every 30 seconds
    if (ready && millis() - lastTime > timerDelay){
        lastTime = millis();

        if (isnan(temp) || isnan(hum)) {
            Serial.println("Failed to read from DHT sensor! Skipping sheet append.");
            return; 
        }

        updateOLEDDisplay("Uploading...");

        FirebaseJson response;
        FirebaseJson valueRange;
        formattedTime = getTime();

        valueRange.add("majorDimension", "COLUMNS");
        valueRange.set("values/[0]/[0]", formattedTime);
        valueRange.set("values/[1]/[0]", temp);
        valueRange.set("values/[2]/[0]", hum);

        bool success = GSheet.values.append(&response, spreadsheetId, "Sheet1!A1", &valueRange);
        if (success){
            response.toString(Serial, true);
            valueRange.clear();
            updateOLEDDisplay("Upload Success!");
        }
        else{
            Serial.println(GSheet.errorReason());
            updateOLEDDisplay("Upload Failed!");
        }
        delay(1000); // Small visual hold for state change
    }
}

void tokenStatusCallback(TokenInfo info){
    // Keep callback logic running silently in terminal background
}