#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <Finger.h>
#include <MQTTFinger.h>
#include <PubSubClient.h>
#include <Ticker.h>

// interrupt pin for finger detected
#define FINGER_INTERRUPT D5

// status leds
#define LED_GREEN D6 // on when ready for a command
#define LED_BLUE D7  // on when handling a command
#define LED_RED D8   // on during normal operation, blinking during power-up

#define MOSQUITTO_PORT 1883

static const char *ssid = "200";
static const char *pass = "1Gabelbombe";

static const char *mqtt_server = "192.168.0.26";
static const char *mqtt_clientid = "fingerprint";
static const char *mqtt_out_topic = "fingerprint_out";
static const char *mqtt_in_topic = "fingerprint_in";

// wifi client
WiFiClient espClient;

// the raw fingerprint sensor
Finger finger(D2, D3, 57600);

// mqtt client
PubSubClient mqtt_client(espClient);

// the mqtt-supproting finger
MQTTFinger mqtt_finger(&finger, &mqtt_client);

// ticker for timed callbacks, e.g. blinking leds
Ticker ticker;

// status helpers
void blink_red_led();
void signal_setup_complete();

// setup helpers
void setup_leds();
void setup_serial();
void setup_wifi();
void setup_ota();
bool setup_finger();
void setup_mqtt();

// mqtt helpers
void mqtt_callback(char *topic, byte *payload, uint16_t length);
void mqtt_connect();

void setup()
{
  setup_leds();

  ticker.attach_ms(100, blink_red_led);

  setup_serial();
  setup_wifi();
  setup_ota();
  if (!setup_finger())
  {
    delay(2000);
    ESP.restart();
  }
  setup_mqtt();

  ticker.detach();
  signal_setup_complete();
}

void loop()
{
  ArduinoOTA.handle();

  if (!mqtt_client.connected())
  {
    delay(2000);
    ESP.restart();
  }
  mqtt_client.loop();

  mqtt_finger.handle_opcode();
  digitalWrite(LED_BLUE, LOW);
  digitalWrite(LED_GREEN, HIGH);
}

void setup_leds()
{
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_BLUE, LOW);
  digitalWrite(LED_RED, LOW);
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

bool setup_finger()
{
  if (!finger.begin())
  {
    Serial.println("Did not find fingerprint sensor :(");
    return false;
  }
  Serial.println("Found fingerprint sensor!");
  finger.print_params();
  return true;
}

void setup_mqtt()
{
  mqtt_client.setServer(mqtt_server, MOSQUITTO_PORT);
  mqtt_client.setCallback(mqtt_callback);
  mqtt_connect();
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

  digitalWrite(LED_BLUE, HIGH);
  digitalWrite(LED_GREEN, LOW);
  mqtt_finger.set_opcode(opcode);
}

void mqtt_connect()
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

void blink_red_led()
{
  bool state = digitalRead(LED_RED);
  digitalWrite(LED_RED, !state);
  if (state)
  {
    // timer to turn LED back off
    ticker.attach_ms(150, blink_red_led);
  }
  else
  {
    // timer to turn LED on
    ticker.attach_ms(50, blink_red_led);
  }
}

void signal_setup_complete()
{
  // LED remains during normal operation
  digitalWrite(LED_RED, HIGH);

  // blink the green LED to signal we're done
  for (int i = 0; i < 5; ++i)
  {
    digitalWrite(LED_GREEN, HIGH);
    delay(500);
    digitalWrite(LED_GREEN, LOW);
    delay(250);
  }

  digitalWrite(LED_GREEN, HIGH);
}