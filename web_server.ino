#include <splash.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#ifndef STASSID
#define STASSID ""
#define STAPSK ""
#endif
#include <WiFiClientSecure.h>

const char* ssid = STASSID;
const char* password = STAPSK;

//setting up the dispaly
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#if defined(ESP8266)
  #define BUTTON_A  0
  #define BUTTON_B 16
  #define BUTTON_C  2
  #define WIRE Wire
#elif defined(ESP32)
  #define BUTTON_A 15
  #define BUTTON_B 32
  #define BUTTON_C 14
  #define WIRE Wire
#else // 32u4, M0, M4, nrf52840 and 328p
  #define BUTTON_A  9
  #define BUTTON_B  6
  #define BUTTON_C  5
  #define WIRE Wire
#endif

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &WIRE);




// Create an instance of the server
// specify the port to listen on as an argument

//this is the port of the server
WiFiServer server(80);

void setup() {
  Serial.begin(115200);

  //starting the display output
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);

  // initialising the display
  display.display();
  delay(1000);

  // Clear the buffer.
  display.clearDisplay();
  display.display();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  

  // Connect to WiFi network
  display.setCursor(0,0);
  display.print(F("Connecting to \n"));
  display.print(ssid);
  display.display();

  //initilaising the connection
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  //busy wait
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    display.print(F("."));
    display.display();
  }

  display.clearDisplay();
  display.setCursor(0,0);
  display.display();
  display.print(F("WiFi connected \n"));

  // Start the server
  server.begin();
  display.print(F("Server started \n"));
  // Print the IP address
  display.print(WiFi.localIP());
  display.display();
}

void loop() {
  // Check if a client has connected
  
  WiFiClient client = server.accept();
  if (!client) { return; }
  Serial.println(F("new client"));

  client.setTimeout(5000);  // default is 1000

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(F("request: "));
  Serial.println(req);

  // Match the request
  int val;
  if (req.indexOf(F("/gpio/0")) != -1) {
    val = 0;
  } else if (req.indexOf(F("/gpio/1")) != -1) {
    val = 1;
  } else {
    Serial.println(F("invalid request"));
    val = digitalRead(LED_BUILTIN);
  }

  // Set LED according to the request
  digitalWrite(LED_BUILTIN, val);

  // read/ignore the rest of the request
  // do not client.flush(): it is for output only, see below
  while (client.available()) {
    // byte by byte is not very efficient
    client.read();
  }

  // Send the response to the client
  // it is OK for multiple small client.print/write,
  // because nagle algorithm will group them into one single packet
  client.print(F("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nGPIO is now "));
  client.print((val) ? F("high") : F("low"));
  client.print(F("<br><br>Click <a href='http://"));
  client.print(WiFi.localIP());
  client.print(F("/gpio/1'>here</a> to switch LED GPIO on, or <a href='http://"));
  client.print(WiFi.localIP());
  client.print(F("/gpio/0'>here</a> to switch LED GPIO off.</html>"));

  // The client will actually be *flushed* then disconnected
  // when the function returns and 'client' object is destroyed (out-of-scope)
  // flush = ensure written data are received by the other side
  Serial.println(F("Disconnecting from client"));
}