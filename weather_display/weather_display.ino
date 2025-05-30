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

const char* host = "api.openweathermap.org";
const int httpsPort = 443;

// SHA1 fingerprint for api.openweathermap.org for https connection
//get it by running the following command in terminal:
// openssl s_client -connect api.openweathermap.org:443 -servername api.openweathermap.org
// and copy the fingerprint from the output
const char fingerprint[] = "";

String url = "/data/2.5/weather?q=London&appid=API Key here";


// Create an instance of the server
// specify the port to listen on as an argument


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
  WiFiClientSecure client;
  client.setFingerprint(fingerprint);

  //chcking the host 
    if (!client.connect(host, httpsPort)) {
        display.print(F("Connection failed \n"));
        display.display();
        return;
    }

    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");

    while (client.connected()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") {
            break; // Headers received
        }
    }

    String response = client.readString();

    display.clearDisplay();
    display.setCursor(0,0);
    //scrolling the response
    display.print(F("Response: \n"));
    for (int i = 0; i < response.length(); i += 16) {
        display.print(response.substring(i, i + 16));
        display.print("\n");
        display.display();
        delay(1000); // Adjust delay as needed for scrolling effect
        display.clearDisplay();
        display.setCursor(0,0);
    }
  // Print the IP address
//   display.print(WiFi.localIP());
//   display.display();
//   display.setCursor(0, 0);

}

void loop() {
}
