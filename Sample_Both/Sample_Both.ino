#ifdef ENABLE_DEBUG
#define DEBUG_ESP_PORT Serial
#define NODEBUG_WEBSOCKETS
#define NDEBUG
#endif

#include <Arduino.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif
#ifdef ESP32
#include <WiFi.h>
#endif

// SinricPro
#include "SinricPro.h"
#include "SinricProSwitch.h"
#include <map>

// SmartNest
#include <PubSubClient.h>
#include <WiFiClient.h>

#define WIFI_SSID "Enter WIFI Name"             // Your Wifi Network name
#define WIFI_PASS "Enter WIFI Password"         // Your Wifi network password

// SinricPro
#define APP_KEY "Enter APP_KEY" // Should look like "de0bxxxx-1x3x-4x3x-ax2x-5dabxxxxxxxx"
#define APP_SECRET "Enter APP_SECRET" // Should look like "5f36xxxx-x3x7-4x3x-xexe-e86724a9xxxx-4c4axxxx-3x3x-x5xe-x9x3-333d65xxxxxx"
#define DEVICE_ID_1 "Enter 1st Device Id" // Should look like "5dc1564130xxxxxxxxxxxxxx"
// #define DEVICE_ID_2 "Enter 2nd Device Id" // Should look like "5dc1564130xxxxxxxxxxxxxx"
// #define DEVICE_ID_3 "Enter 3rd Device Id" // Should look like "5dc1564130xxxxxxxxxxxxxx"

// SmartNest
#define MQTT_BROKER "smartnest.cz"              // Broker host
#define MQTT_PORT 1883                          // Broker port
#define MQTT_USERNAME ""                        // Username from Smartnest
#define MQTT_PASSWORD ""                        // Password from Smartnest (or API key)
#define MQTT_CLIENT ""                          // Device Id from smartnest
#define FIRMWARE_VERSION "1.0.0"                // Custom name for this program

#define BAUD_RATE 9600                          // Change baudrate to your need
#define LED_PIN 12                              // GPIO for LED (inverted)

// setup function for WiFi connection
void startWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.println("Connecting ...");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 10) {
    attempts++;
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println('\n');
    Serial.print("Connected to ");
    Serial.println(WiFi.SSID());
    Serial.print("IP address:\t");
    Serial.println(WiFi.localIP());

  } else {
    Serial.println('\n');
    Serial.println('I could not connect to the wifi network after 10 attempts \n');
  }

  delay(500);
}

// ********** SmartNest **********
WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length) {  //A new message has been received
  Serial.print("Topic:");
  Serial.println(topic);
  int tokensNumber = 10;
  char* tokens[tokensNumber];
  char message[length + 1];
  splitTopic(topic, tokens, tokensNumber);
  sprintf(message, "%c", (char)payload[0]);
  for (int i = 1; i < length; i++) {
    sprintf(message, "%s%c", message, (char)payload[i]);
  }
  Serial.print("Message:");
  Serial.println(message);

  //------------------ACTIONS HERE---------------------------------
  if (strcmp(tokens[1], "directive") == 0 && strcmp(tokens[2], "powerState") == 0) {
    if (strcmp(message, "ON") == 0) {
      digitalWrite(LED_PIN, LOW);
      sendToBroker("report/powerState", "ON");

    } else if (strcmp(message, "OFF") == 0) {
      digitalWrite(LED_PIN, HIGH);
      sendToBroker("report/powerState", "OFF");
    }
  }
}

void startMqtt() {
  client.setServer(MQTT_BROKER, MQTT_PORT);
  client.setCallback(callback);

  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");

    if (client.connect(MQTT_CLIENT, MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.println("connected");
    } else {
      if (client.state() == 5) {
        Serial.println("Connection not allowed by broker, possible reasons:");
        Serial.println("- Device is already online. Wait some seconds until it appears offline for the broker");
        Serial.println("- Wrong Username or password. Check credentials");
        Serial.println("- Client Id does not belong to this username, verify ClientId");

      } else {
        Serial.println("Not possible to connect to Broker Error code:");
        Serial.print(client.state());
      }

      delay(0x7530);
    }
  }

  char subscibeTopic[100];
  sprintf(subscibeTopic, "%s/#", MQTT_CLIENT);
  client.subscribe(subscibeTopic);  //Subscribes to all messages send to the device

  sendToBroker("report/online", "true");  // Reports that the device is online
  delay(100);
  sendToBroker("report/firmware", FIRMWARE_VERSION);  // Reports the firmware version
  delay(100);
  sendToBroker("report/ip", (char*)WiFi.localIP().toString().c_str());  // Reports the ip
  delay(100);
  sendToBroker("report/network", (char*)WiFi.SSID().c_str());  // Reports the network name
  delay(100);

  char signal[5];
  sprintf(signal, "%d", WiFi.RSSI());
  sendToBroker("report/signal", signal);  // Reports the signal strength
  delay(100);
}

int splitTopic(char* topic, char* tokens[], int tokensNumber) {
  const char s[2] = "/";
  int pos = 0;
  tokens[0] = strtok(topic, s);

  while (pos < tokensNumber - 1 && tokens[pos] != NULL) {
    pos++;
    tokens[pos] = strtok(NULL, s);
  }

  return pos;
}

void checkMqtt() {
  if (!client.connected()) {
    startMqtt();
  }
}

void sendToBroker(char* topic, char* message) {
  if (client.connected()) {
    char topicArr[100];
    sprintf(topicArr, "%s/%s", MQTT_CLIENT, topic);
    client.publish(topicArr, message);
  }
}

// *******************************

// ********** SinricPro **********
bool myPowerState = false;

bool onPowerState(const String &deviceId, bool &state) {
  Serial.printf("Device %s turned %s (via SinricPro) \r\n", deviceId.c_str(), state ? "on" : "off");
  myPowerState = state;
  digitalWrite(LED_PIN, myPowerState ? LOW : HIGH);
  return true; // request handled properly
}

// setup function for SinricPro
void setupSinricPro() {
  // add device to SinricPro
  SinricProSwitch& mySwitch = SinricPro[DEVICE_ID_1];

  // set callback function to device
  mySwitch.onPowerState(onPowerState);

  // setup SinricPro
  SinricPro.onConnected([]() {
    Serial.printf("Connected to SinricPro\r\n");
  });
  SinricPro.onDisconnected([]() {
    Serial.printf("Disconnected from SinricPro\r\n");
  });
  SinricPro.begin(APP_KEY, APP_SECRET);
}

// ********************************

// ********** void setup and loop **********
void setup() {
  Serial.begin(BAUD_RATE);
  pinMode(LED_PIN, OUTPUT); // define LED GPIO as output
  digitalWrite(LED_PIN, HIGH); // turn off LED on bootup
  startWifi();
  setupSinricPro();
  startMqtt();
}

void loop() {
  SinricPro.handle();
  client.loop();
  checkMqtt();
}