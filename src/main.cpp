#include <Arduino.h>
#include <Fingerprint.h>

Finger finger(D2, D3, 57600);

void setup()
{
  Serial.begin(115200);

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

  // clear the database
  finger.clear_database();
}

void loop()
{
  Serial.println("Send any character to enroll a finger...");
  while (Serial.available() == 0)
    yield();
  Serial.println("Searching for a free slot to store the template...");
  int16_t fid;
  if (finger.get_free_id(fid))
    finger.enroll_finger(fid);
  else
    Serial.println("No free slot in flash library!");
  while (Serial.read() != -1)
  {
    // clear buffer
  }
}
