#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET -1
#define ARDUINOJSON_ENABLE_ARDUINO_STRING 1

Adafruit_SSD1306 display(128, 32, &Wire, OLED_RESET);

WiFiEventHandler gotIpEventHandler, disconnectedEventHandler;

// ============================================== //
// ===== INSERT YOUR WIFI NETWORK INFO HERE ===== //
// ============================================== //
const char* ssid = "INSERT_SSID_HERE";
const char* password = "INSERT_PASSWORD_HERE";
// ============================================== //
// ============================================== //
// ============================================== //

const int httpsPort = 443;

// SHA1 fingerprint of the certificate
const char fingerprint[] PROGMEM = "10 76 19 6B E9 E5 87 5A 26 12 15 DE 9F 7D 3B 92 9A 7F 30 13";

// Define LED pins
int posLed = 14;
int negLed = 12;
int infoLed = 13;

// Define Global Progress bar val
int progVal;

// doge boot bmp
// 'boot', 128x32px
const unsigned char doge_boot [] PROGMEM = {
  0xff, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe3, 0xff, 0xff,
  0xff, 0x0f, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe1, 0xff, 0x8f,
  0xff, 0x07, 0xfc, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe9, 0xff, 0x0f,
  0xff, 0x27, 0xf8, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xec, 0xfe, 0x2f,
  0xff, 0x23, 0xf1, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xec, 0xfc, 0x6f,
  0xff, 0x30, 0x23, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xee, 0x00, 0xef,
  0xff, 0x38, 0x07, 0x3c, 0xc6, 0x43, 0x31, 0x9f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xcf, 0xc1, 0xef,
  0xfc, 0x3f, 0xff, 0x3c, 0xc6, 0x03, 0x31, 0x9f, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0xff, 0xcf,
  0xf8, 0xff, 0xff, 0x3c, 0x84, 0x01, 0x21, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x3f, 0xff, 0xcf,
  0xe1, 0xff, 0xff, 0x3c, 0x80, 0x11, 0x20, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xcf,
  0xe7, 0xff, 0xff, 0x3c, 0x00, 0x11, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xf9, 0xff, 0xff, 0xe7,
  0xcc, 0xff, 0xff, 0x9c, 0x31, 0x01, 0x0c, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xf9, 0x1f, 0xff, 0xe3,
  0xc8, 0x7c, 0xff, 0x9e, 0x31, 0x83, 0x8c, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xf3, 0x1f, 0x1f, 0xf3,
  0x98, 0xf8, 0x3f, 0xce, 0x7b, 0xc7, 0x9e, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf3, 0x1e, 0x0f, 0xf3,
  0x9c, 0xf8, 0x3f, 0xcf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe7, 0xfe, 0x0f, 0xf9,
  0x9f, 0xf8, 0x7f, 0xcf, 0xff, 0xff, 0xff, 0xef, 0xff, 0xff, 0xff, 0xff, 0xe7, 0xfe, 0x1f, 0xf9,
  0xb0, 0x7f, 0xff, 0xef, 0xff, 0xff, 0xff, 0xef, 0xff, 0xff, 0xff, 0xff, 0xe4, 0x1f, 0xff, 0xf9,
  0xb0, 0x7f, 0xff, 0xef, 0xff, 0xff, 0xff, 0xef, 0xff, 0xff, 0xff, 0xff, 0xe4, 0x0f, 0xff, 0xf9,
  0xb0, 0x7f, 0xff, 0xef, 0xfe, 0x02, 0xdc, 0x61, 0xfc, 0x40, 0x62, 0x0c, 0x64, 0x0f, 0xff, 0xf9,
  0xb0, 0x7f, 0xff, 0xef, 0xfe, 0x5a, 0xd9, 0xe5, 0xf9, 0x4b, 0x4b, 0xdb, 0x26, 0x1f, 0xff, 0xf9,
  0xb9, 0xff, 0xff, 0xef, 0xfe, 0xda, 0xdb, 0xed, 0xfb, 0x5b, 0x5b, 0xb8, 0xe7, 0xff, 0xff, 0xf9,
  0x91, 0xff, 0xff, 0xcf, 0xfe, 0xda, 0xdb, 0x6d, 0xfb, 0x5b, 0x5b, 0x7b, 0xa4, 0x1f, 0xff, 0xf9,
  0x90, 0x03, 0xff, 0xcf, 0xfe, 0xdb, 0x1c, 0xed, 0xfc, 0x5b, 0x62, 0x0c, 0x70, 0x00, 0xff, 0xf3,
  0x90, 0x03, 0xff, 0xcf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf3, 0x00, 0xff, 0xf3,
  0xcf, 0xff, 0xff, 0x9f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf1, 0xff, 0xff, 0xe3,
  0xc7, 0xff, 0xff, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf9, 0xff, 0xff, 0xe7,
  0xe3, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xcf,
  0xf1, 0xff, 0xfc, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x7f, 0xff, 0x1f,
  0xf8, 0xff, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, 0xfe, 0x3f,
  0xfc, 0x3f, 0xe1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x87, 0xf8, 0x7f,
  0xff, 0x06, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x01, 0xff,
  0xff, 0xc0, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x0f, 0xff
};

void setup()
{
  Serial.begin(115200);

  // TO-DO: Stick all initializing steps in to single sequence.
  // Intializing = LED Test, Connect to Wifi, Sync Time, etc..
  // Work on error handling.

  Serial.println('Initializing Display');

  // initialize with the I2C addr 0x3C
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  clearDisplay();  // Clear the buffer.

  // Setup LEDs
  pinMode(posLed, OUTPUT);
  pinMode(negLed, OUTPUT);
  pinMode(infoLed, OUTPUT);

  gotIpEventHandler = WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP & event)
  {
    // Progress Bar
    drawProgressbar(0, 20, 120, 10, 100);
  });

  // Draw the boot doge
  // drawBitmap(x position, y position, bitmap data, bitmap width, bitmap height, color)
  display.drawBitmap(0, 0, doge_boot, 128, 32, WHITE);
  updateDisplay();

  delay(2000);
  clearDisplay();

  // Display Text
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(1, 1);
  display.print("Initializing");
  updateDisplay();
  flashLed();
  drawProgressbar(0, 20, 120, 10, 100);
  display.display();
  delay(750);
  clearDisplay();

  infoOn(); // Turn on blue info LED

  // Connect to wireless network
  WiFi.begin(ssid, password);

  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  // Display Text
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(1, 1);
  display.print("Connecting to: ");
  display.println(ssid);

  drawProgressbar(0, 20, 120, 10, 10);
  progVal = 10;

  updateDisplay();

  int i = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500); // Wait for WiFi to connect
    Serial.print(++i); Serial.print(' ');

    // Progress Bar
    drawProgressbar(0, 20, 120, 10, progVal + 5);
    int d = progVal + 5;
    progVal = d;

    updateDisplay();

  }

  // Breifly Show WiFi Information
  showLanInfo();
  progVal = 0; //reset global progress value

  delay(1000);

  // Set time via NTP, as required for x.509 validation
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");

  clearDisplay();
  display.setCursor(1, 0);
  display.print("Syncing Time");
  updateDisplay();

  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    display.print(".");
    updateDisplay();
    now = time(nullptr);

    // Progress Bar
    drawProgressbar(0, 20, 120, 10, progVal + 5);
    int d = progVal + 5;
    progVal = d;

    updateDisplay();
  }

  progVal = 0; // reset progVal

  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);

  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));

  // Show time on display
  clearDisplay();
  display.setCursor(1, 0);
  display.print("Current Time");
  display.setCursor(1, 10);
  display.println(asctime(&timeinfo));
  updateDisplay();

}

String host ("api.cryptonator.com");
String url ("/api/ticker/doge-usd");

void loop()
{
  disconnectedEventHandler = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected & event)
  {
    Serial.println("Wi-Fi disconnected");
    // Show error on display
    displayError("Wi-Fi", "Disconnected!");
  });
  
  if (WiFi.status() == WL_CONNECTED)
  {

    WiFiClientSecure client;              // Connect to our API URL
    client.setFingerprint(fingerprint);   // Set the Fingerprint for SSL

    // If we can't connect...
    if (!client.connect(host, httpsPort)) {
      Serial.println("connection failed");

      String wifiError = "Can't connect to WiFi!";

      // Show error on display
      displayError("WiFi Error", wifiError);

      return;
    }

    // Otherwise, set headers
    client.print(String("POST ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "User-Agent: ESP8266\r\n" +
                 "Connection: close\r\n\r\n");

    // While we are connected, read the data
    while (client.connected()) {

      String line = client.readStringUntil('\n');
      if (line == "\r") {
        //Serial.println("Data Received");
        break;
      }
    }

    // Load our JSON into data variable
    String data = client.readStringUntil('\n');

    // Setup JSON Filter
    StaticJsonDocument<112> filter;
    filter["success"] = true;
    filter["error"] = true;

    // Define JSON Filter
    JsonObject filter_ticker = filter.createNestedObject("ticker");
    filter_ticker["base"] = true;
    filter_ticker["target"] = true;
    filter_ticker["price"] = true;
    filter_ticker["change"] = true;

    // Set JSON Size in Buffer
    StaticJsonDocument<256> doc;

    // Handle any deserialization errors
    DeserializationError error = deserializeJson(doc, data, DeserializationOption::Filter(filter));

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    // Set our received data variables
    JsonObject ticker = doc["ticker"];
    const char* base = ticker["base"]; // "DOGE"
    const char* target = ticker["target"]; // "USD"
    const char* price = ticker["price"]; // "0.13778104"
    const char* change = ticker["change"]; // "-0.00001018"

    bool success = doc["success"]; // true
    const char* apiError = doc["error"]; // "Lorem Ipsum"

    if (success == true) {

      // Serial Monitor
      Serial.println("==========");
      Serial.print("Price: ");
      Serial.println(price);
      Serial.print("1-Hour Change: ");
      Serial.println(change);

      // Update the display
      updatePrice(base, target, price, change);

    } else {

      // Serial Monitor
      Serial.println("==========");
      Serial.print("API Error: ");
      Serial.println(apiError);

      // Show error on display
      displayError("API ERROR", apiError);

    }
  }

  // Wait 30 seconds until we collect updated data
  delay(30000);
}

// Function update the display
void updateDisplay() {
  display.display();
}

// Function to clear the display
void clearDisplay() {
  display.clearDisplay();
}

void drawProgressbar(int x, int y, int width, int height, int progress)
{

  progress = progress > 100 ? 100 : progress;
  progress = progress < 0 ? 0 : progress;

  float bar = ((float)(width - 1) / 100) * progress;

  display.drawRect(x, y, width, height, WHITE);
  display.fillRect(x + 2, y + 2, bar , height - 4, WHITE);


  // Display progress text
  if ( height >= 15) {
    display.setCursor((width / 2) - 3, y + 5 );
    display.setTextSize(1);
    display.setTextColor(WHITE);
    if ( progress >= 50)
      display.setTextColor(BLACK, WHITE); // 'inverted' text

    display.print(progress);
    display.print("%");
  }
}

void displayError(String type, String e) {

  // Flash Red LED to Alert of Error
  flashRed();

  // Clear Display Buffer
  clearDisplay();

  // Set Title
  display.setTextColor(BLACK, WHITE); // Inverted Display White BG, BLK Text
  display.setCursor(1, 0);
  display.print(type);

  // Show Error
  display.setCursor(1, 11);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.print(e);

  display.setCursor(1, 21);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.print("Retrying in 15 seconds.");

  // Update the display
  updateDisplay();

  // Display error for 15 seconds.
  delay(15);

}

void showLanInfo() {

  // Clear display buffer
  clearDisplay();

  // Set the display
  display.setCursor(1, 0);
  display.print("Connected to: ");
  display.println(ssid);
  display.setCursor(1, 10);
  display.println("IP Address: ");
  display.setCursor(1, 20);
  display.println(WiFi.localIP());
  updateDisplay();
}

void updatePrice(String base, String target, String price, const char* change) {

  // Clear Display Buffer
  clearDisplay();

  // Set ticker
  display.setTextColor(BLACK, WHITE); // Inverted Display White BG, BLK Text
  display.setCursor(1, 0);
  display.print(base);
  display.print(" => ");
  display.print(target);

  // Set the current price
  display.setCursor(1, 9);
  display.setTextColor(WHITE); // Revert to dark BG, White text
  display.setTextSize(2);
  display.print("$");
  display.print(price.substring(0, 9));

  // Set the 1-hour change
  display.setCursor(1, 25);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.print("1-hr: $ ");
  display.println(change);

  // Update the display
  updateDisplay();

  // Update LED's
  updateLed(change);
}

void updateLed(const char* changeVal) {

  // TO-DO(maybe): Can we cast char* to int? Which is faster/more efficient?

  // If change char* begins with a '-' change is negative
  // So, turn on red led
  if (changeVal[0] == '-') {
    // change is negative
    negOn();

    // Otherwise, it's a positive value, turn on green led
  } else {
    posOn();
  }

}

// Function to run through each color of the RGB led
void flashLed() {

  allOff();

  posOn();
  drawProgressbar(0, 20, 120, 10, 25);
  display.display();
  delay(250);
  posOff();
  negOn();
  drawProgressbar(0, 20, 120, 10, 50);
  display.display();
  delay(250);
  negOff();
  infoOn();
  drawProgressbar(0, 20, 120, 10, 75);
  display.display();
  delay(250);
  infoOff();
}

// Function to turn all LEDs off
void allOff() {
  digitalWrite(posLed, LOW);
  digitalWrite(negLed, LOW);
  digitalWrite(infoLed, LOW);
}

// Green LED on
void posOn() {
  allOff();
  digitalWrite(posLed, HIGH);
}

// Green LED off
void posOff() {
  digitalWrite(posLed, LOW);
}

// Red LED on
void negOn() {
  allOff();
  digitalWrite(negLed, HIGH);
}

// Red LED off
void negOff() {
  digitalWrite(negLed, LOW);
}

// Flash Red LED 5 times
void flashRed() {

  // Turn off all LED's first.
  allOff();

  // Then flash.
  negOn();
  delay(250);
  negOff();
  delay(250);
  negOn();
  delay(250);
  negOff();
  delay(250);
  negOn();
  delay(250);
  negOff();
  delay(250);
  negOn();
  delay(250);
  negOff();
  delay(250);
  negOn();
  delay(250);
  negOff();
  delay(250);
}

// Blue LED on
void infoOn() {
  allOff();
  digitalWrite(infoLed, HIGH);
}

// Blue LED off
void infoOff() {
  digitalWrite(infoLed, LOW);
}
