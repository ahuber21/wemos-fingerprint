#include <string>

class Finger;
class PubSubClient;

#define OPCODE_MAX_LENGTH 20

class MQTTFinger
{
public:
    MQTTFinger(Finger *finger = nullptr, PubSubClient *mqtt = nullptr);

    // set the next opcode
    void set_opcode(const char *code);

    // handle the next opcode
    void handle_opcode();

    // the function to be called in the main loop
    void loop();

protected:
    // count the number of templates in the database
    bool count();

    // enroll a new finger
    bool enroll_finger();

    // publish a message using the PubSubClient
    void publish(std::string msg) { publish(msg.c_str()); }
    void publish(uint16_t status);
    void publish(const char *msg);

    // read a fingerprint
    bool read_fingerprint();

private:
    void _mqtt_callback(char *topic, uint8_t *payload, unsigned int length);

    Finger *m_finger;
    PubSubClient *m_mqtt;

    char m_opcode[OPCODE_MAX_LENGTH];
    bool m_codeAvailable;
};