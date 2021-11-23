#ifdef ENABLE_DEBUG
#define DEBUG_ESP_PORT Serial
#define NODEBUG_WEBSOCKETS
#define NDEBUG
#endif

// wifi manager for web
#include <WiFiManager.h>

#include <Arduino.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif
#ifdef ESP32
#include <WiFi.h>
#endif

#include <SinricPro.h>
#include <SinricProLight.h>
#include <map>

// change as per your credentials
#define WIFI_SSID "Enter WIFI Name"             // Your Wifi Network name
#define WIFI_PASS "Enter WIFI Password"         // Your Wifi network password

#define APP_KEY "Enter APP_KEY"                 // Should look like "de0bxxxx-1x3x-4x3x-ax2x-5dabxxxxxxxx"
#define APP_SECRET "Enter APP_SECRET"           // Should look like "5f36xxxx-x3x7-4x3x-xexe-e86724a9xxxx-4c4axxxx-3x3x-x5xe-x9x3-333d65xxxxxx"
#define DEVICE_ID_1 "Enter 1st Device Id"       // Should look like "5dc1564130xxxxxxxxxxxxxx"
#define DEVICE_ID_2 "Enter 2nd Device Id"       // Should look like "5dc1564130xxxxxxxxxxxxxx"
#define DEVICE_ID_3 "Enter 3rd Device Id"       // Should look like "5dc1564130xxxxxxxxxxxxxx"

// pins of nodemcu
#define BAUD_RATE 115200
#define D0 D0
#define D1 D1
#define D2 D2
#define D3 D3
#define D4 D4
#define D5 D5
#define D6 D6
#define D7 D7
#define D8 D8

struct Color {
  int r;
  int g;
  int b;
};

// Colortemperature lookup table
using ColorTemperatures = std::map<int, Color>;
ColorTemperatures colorTemperatures{
  //   {Temperature value, {color r, g, b}}
  {2000, {255, 138, 18}},
  {2200, {255, 147, 44}},
  {2700, {255, 169, 87}},
  {3000, {255, 180, 107}},
  {4000, {255, 209, 163}},
  {5000, {255, 228, 206}},
  {5500, {255, 236, 224}},
  {6000, {255, 243, 239}},
  {6500, {255, 249, 253}},
  {7000, {245, 243, 255}},
  {7500, {235, 238, 255}},
  {9000, {214, 225, 255}}};

// device list
struct
{
  const char* deviceId = DEVICE_ID_1;
  bool powerState = false;
  Color color = colorTemperatures[9000];
  int colorTemperature = 9000;
  int brightness = 100;
  const int r_pin = D0;
  const int g_pin = D1;
  const int b_pin = D2;
} DEVICE1;

struct
{
  const char* deviceId = DEVICE_ID_2;
  bool powerState = false;
  Color color = colorTemperatures[9000];
  int colorTemperature = 9000;
  int brightness = 100;
  const int r_pin = D3;
  const int g_pin = D4;
  const int b_pin = D5;
} DEVICE2;

struct
{
  const char* deviceId = DEVICE_ID_3;
  bool powerState = false;
  Color color = colorTemperatures[9000];
  int colorTemperature = 9000;
  int brightness = 100;
  const int r_pin = D6;
  const int g_pin = D7;
  const int b_pin = D8;
} DEVICE3;

/* starting the wifi connection
  ~no params~
*/
void startWifi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 10)
  {
    attempts++;
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  delay(500);
}

/* light led based on device id
  deviceid = deviceid of the strip
*/
void lightLED(const String &deviceId)
{
  if (strcmp(deviceId.c_str(), DEVICE1.deviceId) == 0)
  {
    analogWrite(DEVICE1.r_pin, map(DEVICE1.color.r * DEVICE1.brightness, 0, 255 * 100, 0, 255));
    analogWrite(DEVICE1.g_pin, map(DEVICE1.color.g * DEVICE1.brightness, 0, 255 * 100, 0, 255));
    analogWrite(DEVICE1.b_pin, map(DEVICE1.color.b * DEVICE1.brightness, 0, 255 * 100, 0, 255));
  }
  else if (strcmp(deviceId.c_str(), DEVICE2.deviceId) == 0)
  {
    analogWrite(DEVICE2.r_pin, map(DEVICE2.color.r * DEVICE2.brightness, 0, 255 * 100, 0, 255));
    analogWrite(DEVICE2.g_pin, map(DEVICE2.color.g * DEVICE2.brightness, 0, 255 * 100, 0, 255));
    analogWrite(DEVICE2.b_pin, map(DEVICE2.color.b * DEVICE2.brightness, 0, 255 * 100, 0, 255));
  }
  else if (strcmp(deviceId.c_str(), DEVICE3.deviceId) == 0)
  {
    analogWrite(DEVICE3.r_pin, map(DEVICE3.color.r * DEVICE3.brightness, 0, 255 * 100, 0, 255));
    analogWrite(DEVICE3.g_pin, map(DEVICE3.color.g * DEVICE3.brightness, 0, 255 * 100, 0, 255));
    analogWrite(DEVICE3.b_pin, map(DEVICE3.color.b * DEVICE3.brightness, 0, 255 * 100, 0, 255));
  }
}

/* set RGB values to the either of the three devices based on device id
  deviceid = deviceid of the strip
  r = red value
  g = green value
  b = blue value
*/
void setLED(const String &deviceId, int r, int g, int b, int brightness)
{
  if (r > -1 && g > -1 && b > -1 && brightness > -1)
  {
    if (strcmp(deviceId.c_str(), DEVICE1.deviceId) == 0)
    {
      DEVICE1.color.r = r;
      DEVICE1.color.g = g;
      DEVICE1.color.b = b;
      DEVICE1.brightness = brightness;
    }
    else if (strcmp(deviceId.c_str(), DEVICE2.deviceId) == 0)
    {
      DEVICE2.color.r = r;
      DEVICE2.color.g = g;
      DEVICE2.color.b = b;
      DEVICE2.brightness = brightness;
    }
    else if (strcmp(deviceId.c_str(), DEVICE3.deviceId) == 0)
    {
      DEVICE3.color.r = r;
      DEVICE3.color.g = g;
      DEVICE3.color.b = b;
      DEVICE3.brightness = brightness;
    }
  }
  else if (r == -1 && g == -1 && b == -1)
  {
    if (strcmp(deviceId.c_str(), DEVICE1.deviceId) == 0)
    {
      DEVICE1.brightness = brightness;
    }
    else if (strcmp(deviceId.c_str(), DEVICE2.deviceId) == 0)
    {
      DEVICE2.brightness = brightness;
    }
    else if (strcmp(deviceId.c_str(), DEVICE3.deviceId) == 0)
    {
      DEVICE3.brightness = brightness;
    }
  }
  else if (brightness == -1)
  {
    if (strcmp(deviceId.c_str(), DEVICE1.deviceId) == 0)
    {
      DEVICE1.color.r = r;
      DEVICE1.color.g = g;
      DEVICE1.color.b = b;
    }
    else if (strcmp(deviceId.c_str(), DEVICE2.deviceId) == 0)
    {
      DEVICE2.color.r = r;
      DEVICE2.color.g = g;
      DEVICE2.color.b = b;
    }
    else if (strcmp(deviceId.c_str(), DEVICE3.deviceId) == 0)
    {
      DEVICE3.color.r = r;
      DEVICE3.color.g = g;
      DEVICE3.color.b = b;
    }
  }
}

/* switch led function for the SinricPro
  deviceid = deviceid of the strip
  state = state of the switch
*/
bool switchLed(const String &deviceId, bool &state)
{
  if (state)
    setLED(deviceId, 255, 255, 255, 100);
  else
    setLED(deviceId, 0, 0, 0, 0);
  lightLED(deviceId);
  return true;
}

/* set RGB values to the either of the three devices based on device id
  deviceid = deviceid of the strip
  brightness = the brightness value
*/
bool changeBrightness(const String &deviceId, int &brightness)
{
  setLED(deviceId, -1, -1, -1, brightness);
  lightLED(deviceId);
  return true;
}

/* change color of the strip
  deviceid = deviceid of the strip
  r = red value
  g = green value
  b = blue value
*/
bool changeColor(const String &deviceId, byte &r, byte &g, byte &b)
{
  setLED(deviceId, r, g, b, -1);
  lightLED(deviceId);
  return true;
}

/* change color temperature of the strip
  deviceid = deviceid of the strip
  colorTemp = color temperature
*/
bool changeTemp(const String &deviceId, int &colorTemp)
{
  setLED(deviceId, colorTemperatures[colorTemp].r, colorTemperatures[colorTemp].g, colorTemperatures[colorTemp].b, -1);
  lightLED(deviceId);
  return true;
}

// setup function for sinric pro
void setupSinricPro()
{
  SinricProLight &LEDLight1 = SinricPro[DEVICE_ID_1];
  SinricProLight &LEDLight2 = SinricPro[DEVICE_ID_2];
  SinricProLight &LEDLight3 = SinricPro[DEVICE_ID_3];

  // change state
  LEDLight1.onPowerState(switchLed);
  LEDLight2.onPowerState(switchLed);
  LEDLight3.onPowerState(switchLed);

  // change brightness 
  LEDLight1.onBrightness(changeBrightness);
  LEDLight2.onBrightness(changeBrightness);
  LEDLight3.onBrightness(changeBrightness);

  // change color
  LEDLight1.onColor(changeColor);
  LEDLight2.onColor(changeColor);
  LEDLight3.onColor(changeColor);

  //change color temp
  LEDLight1.onColorTemperature(changeTemp);
  LEDLight2.onColorTemperature(changeTemp);
  LEDLight3.onColorTemperature(changeTemp);
  
  SinricPro.begin(APP_KEY, APP_SECRET);
}

void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);
  pinMode(D8, OUTPUT);
  digitalWrite(D0, LOW);
  digitalWrite(D1, LOW);
  digitalWrite(D2, LOW);
  digitalWrite(D3, LOW);
  digitalWrite(D4, LOW);
  digitalWrite(D5, LOW);
  digitalWrite(D6, LOW);
  digitalWrite(D7, LOW);
  digitalWrite(D8, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  /* if the saved wifi credentials do not connect automatically connect to the hotspot mentioned here
    open browser to 192.168.4.1 and enter wifi credentials
  */
  WiFiManager wifiManager;
  wifiManager.autoConnect("SinricPro NodeMCU", "password");
  startWifi();
  setupSinricPro();
}

void loop()
{
  SinricPro.handle();
}
