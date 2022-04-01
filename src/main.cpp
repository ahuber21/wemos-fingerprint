#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <Finger.h>
#include <MQTTFinger.h>
#include <PubSubClient.h>

#define MOSQUITTO_PORT 1883

const char *ssid = "200";
const char *pass = "1Gabelbombe";

const char *mqtt_server = "192.168.0.26";
const char *mqtt_clientid = "fingerprint";
const char *mqtt_out_topic = "fingerprint_out";
const char *mqtt_in_topic = "fingerprint_in";

Finger finger(D2, D3, 57600);
WiFiClient espClient;
PubSubClient mqtt_client(espClient);
MQTTFinger mqtt_finger(&finger, &mqtt_client);

// setup helpers
void setup_serial();
void setup_wifi();
void setup_ota();
void setup_finger();
void setup_mqtt();

// mqtt helpers
void mqtt_callback(char *topic, byte *payload, uint16_t length);
void mqtt_reconnect();

void setup()
{
  setup_serial();
  setup_wifi();
  setup_ota();
  setup_mqtt();
  setup_finger();
}

void loop()
{
  ArduinoOTA.handle();

  if (!mqtt_client.connected())
    mqtt_reconnect();
  mqtt_client.loop();

  mqtt_finger.handle_opcode();
}

void setup_serial()
{
  Serial.begin(115200);
}

void setup_wifi()
{
  // set up wifi and OTA
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  Serial.print("WiFi connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("success");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup_ota()
{
  ArduinoOTA.setHostname("sauffinger");
  ArduinoOTA.setPassword("prost");

  ArduinoOTA.begin();
  Serial.println("Arduino OTA ready");
}

void setup_finger()
{
  if (finger.begin())
  {
    Serial.println("Found fingerprint sensor!");
    finger.print_params();
  }
  else
  {
    Serial.println("Did not find fingerprint sensor :(");
    while (1)
      yield();
  }
}

void setup_mqtt()
{
  mqtt_client.setServer(mqtt_server, MOSQUITTO_PORT);
  mqtt_client.setCallback(mqtt_callback);
}

void mqtt_callback(char *topic, byte *payload, uint16_t length)
{
  char opcode[length + 1];
  for (uint16_t i = 0; i < length; ++i)
  {
    opcode[i] = payload[i];
  }
  opcode[length] = '\0';

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(opcode);

  mqtt_finger.set_opcode(opcode);
}

void mqtt_reconnect()
{
  int count = 0;
  // Loop until we're reconnected
  while (!mqtt_client.connected() and count < 10)
  {
    count++;
    Serial.print("Attempting MQTT connection (");
    Serial.print(count);
    Serial.print("/10) ");
    // Attempt to connect
    if (mqtt_client.connect(mqtt_clientid))
    {
      Serial.println("connected");
      mqtt_client.publish(mqtt_out_topic, "ready");
      // ... and resubscribe
      mqtt_client.subscribe(mqtt_in_topic);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.println(mqtt_client.state());
      // Wait 5 seconds before retrying
      delay(1000);
    }
  }
}
