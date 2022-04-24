#include "MQTTFinger.h"
#include "Finger.h"
#include "PubSubClient.h"
#include "StatusLEDs.h"
#include <string>

MQTTFinger::MQTTFinger(Finger *finger, PubSubClient *mqtt)
    : m_finger(finger),
      m_mqtt(mqtt),
      m_opcode(""),
      m_codeAvailable(false) {}

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

bool MQTTFinger::delete_finger(int16_t fid)
{
    int16_t status;

    bool success = m_finger->delete_finger(fid, status);
    if (!success)
        publish(status);
    else
        publish("Deleted finger with ID = " + std::to_string(fid));

    return success;
}

bool MQTTFinger::enroll_finger()
{
    bool success;
    int16_t status, fid;

    publish("starting to enroll a new finger", "fingerprint_debug");
    success = m_finger->get_free_id(fid);
    if (!success)
    {
        publish("no space left on device");
        return false;
    }

    publish("place finger to start enrolling at ID = " + std::to_string(fid), "fingerprint_debug");
    delay(1000);
    success = m_finger->read_template(status);
    if (!success)
    {
        publish(status);
        return false;
    }

    publish("remove finger", "fingerprint_debug");
    digitalWrite(LED_BLUE, LOW);
    blink_led(LED_BLUE, 3, 100);
    delay(1000);
    publish("place same finger again", "fingerprint_debug");
    digitalWrite(LED_BLUE, HIGH);
    delay(200);
    success = m_finger->verify_template(status);
    if (!success)
    {
        publish(status, "fingerprint_debug");
        return false;
    }

    publish("validating template", "fingerprint_debug");
    success = m_finger->store_model(fid, status);
    if (!success)
    {
        publish(status, "fingerprint_debug");
        return false;
    }
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
    else if (strncmp(m_opcode, "DELETE", 6) == 0)
    {
        // delete the requested fingerprint from the index
        success = delete_finger(get_number_in_opcode());
    }
    else if (strcmp(m_opcode, "CLEAR") == 0)
    {
        // clear the fingerprint database
        success = m_finger->clear_database();
    }
    else if (strcmp(m_opcode, "COUNT") == 0)
    {
        success = count();
    }
    else if (strcmp(m_opcode, "PARAMS") == 0)
    {
        // print the system parameters to MQTT
        print_system_params();
        success = true;
    }
    else if (strncmp(m_opcode, "SECURITY", 8) == 0)
    {
        success = set_security_level(get_number_in_opcode());
    }
    else
    {
        publish("unknown code: " + std::string(m_opcode), "fingerprint_debug");
    }

    m_codeAvailable = false;

    digitalWrite(LED_BLUE, LOW);
    blink_led(success ? LED_GREEN : LED_RED, 3, 50);
}

int16_t MQTTFinger::get_number_in_opcode()
{
    const char *ptr = strchr(m_opcode, ' ');
    if (ptr && *(ptr + 1))
    {
        return std::atoi(ptr + 1);
    }
    else
    {
        return 0;
    }
}

void MQTTFinger::loop()
{
    handle_opcode();
}

void MQTTFinger::publish(const char *msg, const char *topic)
{
    if (!m_mqtt)
        return;

    m_mqtt->publish(topic, msg);
}

void MQTTFinger::publish(uint16_t status, const char *topic)
{
    if (!m_finger)
        return;
    publish(m_finger->status_to_string(status), topic);
}

bool MQTTFinger::read_fingerprint()
{
    int16_t status;
    uint16_t fid, score;
    bool success = m_finger->read_fingerprint(status, fid, score);
    if (!success)
    {
        publish(status, "fingerprint_debug");
        return false;
    }

    publish("ID = " + std::to_string(fid) + " | score = " + std::to_string(score));
    return true;
}

void MQTTFinger::set_opcode(const char *code)
{
    strncpy(m_opcode, code, OPCODE_MAX_LENGTH);
    m_codeAvailable = true;
}

bool MQTTFinger::set_security_level(uint8_t level)
{
    if (level < 1 || level > 5)
    {
        publish("Security level must be between 1 and 5", "fingerprint_debug");
        return false;
    }
    int16_t status;
    bool success = m_finger->set_security_level(level, status);
    if (success)
        publish("Security level set to " + std::to_string(level));
    else
        publish(status);
    return success;
}

void MQTTFinger::print_system_params()
{
    FPM_System_Params params = m_finger->get_params();
    publish("Fingerprint parameters", "fingerprint_debug");
    publish("status_reg   " + std::to_string(params.status_reg), "fingerprint_debug");
    publish("system_id    " + std::to_string(params.system_id), "fingerprint_debug");
    publish("capacity     " + std::to_string(params.capacity), "fingerprint_debug");
    publish("security lvl " + std::to_string(params.security_level), "fingerprint_debug");
    publish("device_addr  " + std::to_string(params.device_addr), "fingerprint_debug");
    publish("packet len   " + std::to_string(params.packet_len), "fingerprint_debug");
    publish("baud rate    " + std::to_string(params.baud_rate), "fingerprint_debug");
}