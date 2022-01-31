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

const char* ssid = "SSID_HERE";
const char* password = "PASSWORD";

const int httpsPort = 443;

// SHA1 fingerprint of the certificate
const char fingerprint[] PROGMEM = "10 76 19 6B E9 E5 87 5A 26 12 15 DE 9F 7D 3B 92 9A 7F 30 13";

// Define LEDS
int posLed = 14;
int negLed = 12;

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
  allOn();

  // Display Text
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(1, 1);
  display.print("Initializing");
  updateDisplay();
  delay(1000);
  clearDisplay();
  Serial.println('\n');

  allOff();

  WiFi.begin(ssid, password);

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

    // Set our data variable
    String data = client.readStringUntil('\n');

    // Set Buffer Size
    const size_t capacity = JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + 150;
    DynamicJsonBuffer jsonBuffer(capacity);

    // Parse the JSON
    JsonObject& root = jsonBuffer.parseObject(data);

    // Set Vars
    JsonObject& ticker = root["ticker"];
    const char* ticker_base = ticker["base"]; // "DOGE"
    const char* ticker_target = ticker["target"]; // "USD"
    const char* ticker_price = ticker["price"]; // "0.14231596"
    const char* ticker_volume = ticker["volume"]; // "205965275.15992999"
    const char* ticker_change = root["ticker"]["change"]; // "0.00026863"
    //const char* ticker_change = "-0.00012345"; // "0.00026863"
    
    long timestamp = root["timestamp"]; // 1643508362
    bool success = root["success"]; // true
    const char* error = root["error"]; // ""

    // CONVERT ticker_change char* TO Int
    //int change_int = atoi(ticker_change);

    // ===============================
    // ============ DEBUG ============
    // ===============================
    Serial.println("==========");
    Serial.print("Price: ");
    Serial.println(ticker_price);
    Serial.print("1-Hour Change: ");
    Serial.println(ticker_change);
    Serial.println("==========");
    // ===============================
    // ===============================
    // ===============================

    // Now update the display
    updatePrice(ticker_price, ticker_change);


  }

  // Wait 30 seconds until we collect updated data
  delay(30000);
}

void showLanInfo(){
  
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

void updatePrice(String price, const char* change) {

  // Clear Display Buffer
  clearDisplay();

  // Set ticker
  display.setTextColor(BLACK, WHITE); // Inverted Display White BG, BLK Text
  display.setCursor(1, 0);
  display.print("DOGE/USD");

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
  //updateLed(change);
}

//void updateLed(const char* changeVal){

  //int val = changeVal.toInt();

  //char tmp[10] = changeVal;
  //int v = tmp.toInt();
  
//  Serial.print("changeVal var: ");
//  Serial.println(changeVal);
//  
//  Serial.print("atoi v var: ");
//  Serial.println(v);
//
//  if (v > 0){
//    Serial.println("num>0");
//    negOff();
//    posOn();
//  } else {
//    Serial.println("num<0");
//    posOff();
//    negOn();
//  }
//  
//}

void updateDisplay() {
  display.display();
}

void clearDisplay() {
  display.clearDisplay();
}

void allOn(){
  digitalWrite(posLed, HIGH);
  digitalWrite(negLed, HIGH);
}

void allOff(){
  digitalWrite(posLed, LOW);
  digitalWrite(negLed, LOW);
}

void posOn(){
  digitalWrite(posLed, HIGH);
}

void posOff(){
  digitalWrite(posLed, LOW);
}

void negOn(){
  digitalWrite(negLed, HIGH);
}

void negOff(){
  digitalWrite(negLed, LOW);
}
