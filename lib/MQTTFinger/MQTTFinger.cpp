#include "MQTTFinger.h"
#include "Finger.h"
#include "PubSubClient.h"
#include <string>

MQTTFinger::MQTTFinger(Finger *finger, PubSubClient *mqtt)
    : m_finger(finger),
      m_mqtt(mqtt),
      m_opcode(""),
      m_codeAvailable(false)
{
}

void MQTTFinger::set_opcode(const char *code)
{
    strncpy(m_opcode, code, OPCODE_MAX_LENGTH);
    m_codeAvailable = true;
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

void MQTTFinger::enroll_finger()
{
    bool success;
    int16_t status, fid;

    publish("starting to enroll a new finger");
    success = m_finger->get_free_id(fid);
    if (!success)
    {
        publish("no space left on device");
        return;
    }

    publish("place finger");
    delay(1000);
    success = m_finger->read_template(status);
    publish(status);
    if (!success)
        return;

    publish("remove finger");
    delay(1000);
    publish("place same finger again");
    delay(1000);
    success = m_finger->verify_template(status);
    publish(status);
    if (!success)
        return;

    publish("validating templte");
    success = m_finger->store_model(fid, status);
    publish(status);
    if (!success)
        return;

    publish("Enrolled new finger at ID = " + std::to_string(fid));
}

void MQTTFinger::handle_opcode()
{
    if (!m_finger)
        return;

    if (!m_codeAvailable)
        return;

    if (strcmp(m_opcode, "READ") == 0)
    {
        read_fingerprint();
    }
    else if (strcmp(m_opcode, "ENROLL") == 0)
    {
        enroll_finger();
    }
    else
    {
        publish("unknown code: " + std::string(m_opcode));
    }

    m_codeAvailable = false;
}

void MQTTFinger::loop()
{
    handle_opcode();
}

void MQTTFinger::read_fingerprint()
{
    int16_t status;
    uint16_t fid, score;
    bool success = m_finger->read_fingerprint(status, fid, score);
    publish(status);
    if (!success)
        return;

    publish("ID = " + std::to_string(fid) + " | score = " + std::to_string(score));
    return;
}