#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <Fingerprint.h>

Finger finger(D2, D3, 57600);
const char *ssid = "200";
const char *pass = "1Gabelbombe";

void setup()
{
  Serial.begin(115200);

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

  ArduinoOTA.setHostname("sauffinger");
  ArduinoOTA.setPassword("prost");

  ArduinoOTA.begin();
  Serial.println("Arduino OTA ready");

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

void loop()
{
  ArduinoOTA.handle();

  if (Serial.available() > 0)
  {

    Serial.println("Searching for a free slot to store the template...");
    int16_t fid;

    if (finger.get_free_id(fid))
    {
      for (int i = 0; i < 10; ++i)
      {
        if (finger.enroll_finger(fid))
        {
          break;
        }
        delay(200);
        Serial.println("Enroll failed - please try again");
        delay(500);
      }
    }
    else
    {
      Serial.println("No free slot in flash library!");
    }

    while (Serial.read() != -1)
    {
      // clear buffer
    }
  }
}
