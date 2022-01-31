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

const char* ssid = "Jones";
const char* password = "Jhome@10820";

const int httpsPort = 443;

// SHA1 fingerprint of the certificate
const char fingerprint[] PROGMEM = "10 76 19 6B E9 E5 87 5A 26 12 15 DE 9F 7D 3B 92 9A 7F 30 13";

// Define LEDS
int posLed = 14;
int negLed = 12;
int infoLed = 13;

void setup()
{
  Serial.begin(115200);
  // initialize with the I2C addr 0x3C
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  //Serial.println('Initializing Display');

  clearDisplay();  // Clear the buffer.

  // Setup LEDs
  pinMode(posLed, OUTPUT);
  pinMode(negLed, OUTPUT);
  pinMode(infoLed, OUTPUT);
  flashLed();

  // Display Text
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(1, 1);
  display.print("Initializing");
  updateDisplay();
  delay(1000);
  clearDisplay();
  Serial.println('\n');

  WiFi.begin(ssid, password);

  infoOn();

  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  // Display Text
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(1, 1);
  display.print("Connecting to: ");
  display.println(ssid);
  updateDisplay();

  int i = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(++i); Serial.print(' ');
    display.print("..");
    updateDisplay();
  }

  // Breifly Show WiFi Information
  showLanInfo();
  delay(2000);

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
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
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
  if (WiFi.status() == WL_CONNECTED)
  {

    WiFiClientSecure client;              // Connect to our API URL
    client.setFingerprint(fingerprint);   // Set the Fingerprint for SSL

    // If we can't connect...
    if (!client.connect(host, httpsPort)) {
      Serial.println("connection failed");
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
      displayError(apiError);

    }
  }

  // Wait 30 seconds until we collect updated data
  delay(30000);
}

void displayError(String e) {

  // Flash Red LED to Alert of API Error
  flashRed();
  
  // Clear Display Buffer
  clearDisplay();

  // Set Title
  display.setTextColor(BLACK, WHITE); // Inverted Display White BG, BLK Text
  display.setCursor(1, 0);
  display.print("API ERROR");

  // Show Error
  display.setCursor(1, 25);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.print(e);

  // Update the display
  updateDisplay();

  // Display error for 30 seconds.
  delay(30000);
  
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

  if (changeVal[0] == '-') {
    // change is negative
    Serial.println("val < 0");
    negOn();

  } else {
    Serial.println("val >= 0");
    posOn();
  }

}

void updateDisplay() {
  display.display();
}

void clearDisplay() {
  display.clearDisplay();
}

void flashLed() {
  
  allOff();
  
  posOn();
  delay(250);
  posOff();
  negOn();
  delay(250);
  negOff();
  infoOn();
  delay(250);
  infoOff();
}

void allOff() {
  digitalWrite(posLed, LOW);
  digitalWrite(negLed, LOW);
  digitalWrite(infoLed, LOW);
}

// Green LED
void posOn() {
  allOff();
  digitalWrite(posLed, HIGH);
}

void posOff() {
  digitalWrite(posLed, LOW);
}

// Red LED
void negOn() {
  allOff();
  digitalWrite(negLed, HIGH);
}

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

// Blue LED
void infoOn() {
  allOff();
  digitalWrite(infoLed, HIGH);
}

void infoOff() {
  digitalWrite(infoLed, LOW);
}
