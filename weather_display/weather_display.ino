#include <ArduinoJson.h>
#include <splash.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#ifndef STASSID
#define STASSID "WIFI SSID"
#define STAPSK "WIFI PASSWORD"
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
const char fingerprint[] = "Fingerprint of the server SHA-1 hash";

String url = "/data/2.5/weather?q=Lucknow&appid=API_KEY";


// Create an instance of the server
// specify the port to listen on as an argument

void clearDisplay() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.display();
}

void printToDisplay(const String& message) {
  clearDisplay();
  display.print(message);
  display.display();
}

void wifiConnect(char* ssid, char* pass) {
  //initilaising the connection

  //going in station mode
  WiFi.mode(WIFI_STA);

  //initialising the connection
  WiFi.begin(ssid, password);


  //printing the connection status on the display
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(F("Connecting to \n"));
  display.print(ssid);
  display.display();


  //busy wait and printing the dots on the display
  display.print("\n");
  int dotCount = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    display.print(F("#"));
    dotCount++;

    //dot clearance after passing the display width
    if (dotCount % 16 == 0) {
      display.print("\n");
      display.setCursor(0, 0);
    }
    display.display();

    if (dotCount * 2 >= display.height()) {
      // If the number of dots exceeds the display height, clear the display
      display.clearDisplay();
      display.setCursor(0, 0);
    }

    //timeout condition to prevent infinite loop
    if (dotCount >= 200) {
      break;
    }
  }

  //checking the connection status
  if (WiFi.status() != WL_CONNECTED) {
    printToDisplay(F("Connection failed \n"));
    display.display();
    return;
  } else {
    printToDisplay(F("WiFi connected \n"));
    delay(1000);
  }
}

void initDisplayText() {
  //starting the display output
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32

  //initialising the display
  display.display();
  delay(1000);

  // Clear the buffer.
  display.clearDisplay();
  display.display();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
}



void setup() {
  Serial.begin(115200);

  initDisplayText();
  wifiConnect((char*)ssid, (char*)password);

  printToDisplay(F("IP address\n"));
  display.print(WiFi.localIP());
  display.display();
  delay(1000);
  clearDisplay();


  // Start the server
  WiFiClientSecure client;
  client.setFingerprint(fingerprint);

  //chcking the host
  if (!client.connect(host, httpsPort)) {
    display.print(F("Connection failed \n"));
    display.display();
    return;
  } else {
    printToDisplay(F("Connected to server \n"));
    delay(1000);
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
  display.setCursor(0, 0);
  //scrolling the response
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, response);

  if (error) {
    display.print(F("Parse fail"));
    display.display();
    return;
  }

  const char* city = doc["name"];  // "London"
  float tempK = doc["main"]["temp"];  // in Kelvin
  int tempC = (int)(tempK - 273.15);  // Convert to Celsius
  const char* weather = doc["weather"][0]["main"];  // e.g. "Clear"
  int humidity = doc["main"]["humidity"];  // e.g. 81
  int feelLike = (int)((int)doc["main"]["feels_like"] - 273.15);  // Convert to Celsius
  const char* wind = doc["wind"]["speed"];  // e.g. 4.1 m/s
  const char* uvi = doc["uvi"];  // e.g. 0.0
  const char* description = doc["weather"][0]["description"];  // e.g. "clear sky"

  display.print(city);
  display.print("\nTemp/Feel: ");
  display.print(tempC);
  display.print("/");
  display.print(feelLike);
  display.print(" C\n");
  display.print("Weather: ");
  display.print(weather);
  display.print("\nHumidity: ");
  display.print(humidity);
  display.print("%\n");
  display.display();



  //   display.print(F("Response: \n"));
  //   for (int i = 0; i < response.length(); i += 16) {
  //       display.print(response.substring(i, i + 16));
  //       display.print("\n");
  //       display.display();
  //       delay(1000); // Adjust delay as needed for scrolling effect
  //       display.clearDisplay();
  //       display.setCursor(0,0);
  //   }
  ////  Print the IP address
  //  display.print(WiFi.localIP());
  //  display.display();
  //  display.setCursor(0, 0);

}

void loop() {
}
