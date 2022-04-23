#include "MQTTFinger.h"
#include "Finger.h"
#include "PubSubClient.h"
#include "StatusLEDs.h"
#include <string>

MQTTFinger::MQTTFinger(Finger *finger, PubSubClient *mqtt)
    : m_finger(finger),
      m_mqtt(mqtt),
      m_opcode(""),
      m_codeAvailable(false)
{
}

bool MQTTFinger::clear()
{
    int16_t status;

    bool success = m_finger->clear_database(status);
    publish(status);
    return success;
}

bool MQTTFinger::count()
{
    uint16_t count;
    int16_t status;

    bool success = m_finger->get_template_count(count, status);
    if (success)
        publish("Number of templates on device = " + std::to_string(count));
    else
        publish(status);

    return success;
}

bool MQTTFinger::enroll_finger()
{
    bool success;
    int16_t status, fid;

    publish("starting to enroll a new finger");
    success = m_finger->get_free_id(fid);
    if (!success)
    {
        publish("no space left on device");
        return false;
    }

    publish("place finger");
    delay(1000);
    success = m_finger->read_template(status);
    publish(status);
    if (!success)
        return false;

    publish("remove finger");
    digitalWrite(LED_BLUE, LOW);
    blink_led(LED_BLUE, 3, 100);
    delay(1000);
    publish("place same finger again");
    digitalWrite(LED_BLUE, HIGH);
    delay(1000);
    success = m_finger->verify_template(status);
    publish(status);
    if (!success)
        return false;

    publish("validating templte");
    success = m_finger->store_model(fid, status);
    publish(status);
    if (!success)
        return false;

    publish("Enrolled new finger at ID = " + std::to_string(fid));
    return true;
}

void MQTTFinger::handle_opcode()
{
    if (!m_finger)
        return;

    if (!m_codeAvailable)
        return;

    digitalWrite(LED_BLUE, HIGH);

    bool success = false;

    if (strcmp(m_opcode, "READ") == 0)
    {
        // try to read a fingerprint
        success = read_fingerprint();
    }
    else if (strcmp(m_opcode, "ENROLL") == 0)
    {
        // enroll a new finger
        success = enroll_finger();
    }
    else if (strcmp(m_opcode, "RESET") == 0)
    {
        // reset the ESP
        ESP.reset();
    }
    else if (strcmp(m_opcode, "CLEAR") == 0)
    {
        // clear the fingerprint database
        success = clear();
    }
    else if (strcmp(m_opcode, "COUNT") == 0)
    {
        success =
    }
    else
    {
        publish("unknown code: " + std::string(m_opcode));
    }

    m_codeAvailable = false;

    digitalWrite(LED_BLUE, LOW);
    blink_led(success ? LED_GREEN : LED_RED, 3, 100);
}

void MQTTFinger::loop()
{
    handle_opcode();
}

void MQTTFinger::publish(const char *msg)
{
    if (!m_mqtt)
        return;

    m_mqtt->publish("fingerprint_out", msg);
}

void MQTTFinger::publish(uint16_t status)
{
    if (!m_finger)
        return;
    publish(m_finger->status_to_string(status));
}

bool MQTTFinger::read_fingerprint()
{
    int16_t status;
    uint16_t fid, score;
    bool success = m_finger->read_fingerprint(status, fid, score);
    publish(status);
    if (!success)
        return false;

    publish("ID = " + std::to_string(fid) + " | score = " + std::to_string(score));
    return true;
}

void MQTTFinger::set_opcode(const char *code)
{
    strncpy(m_opcode, code, OPCODE_MAX_LENGTH);
    m_codeAvailable = true;
}
