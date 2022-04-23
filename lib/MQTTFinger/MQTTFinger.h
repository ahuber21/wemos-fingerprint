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

    // print the system params to MQTT
    void print_system_params();

protected:
    // count the number of templates in the database
    bool count();

    // get the number in the opcode
    int16_t get_number_in_opcode();

    // delete the requested finger
    bool delete_finger(int16_t fid);

    // enroll a new finger
    bool enroll_finger();

    // set the security level
    bool set_security_level(uint8_t level);

    // publish a message using the PubSubClient
    void publish(std::string msg) { publish(msg.c_str()); }
    void publish(uint16_t status);
    void publish(const char *msg);

    // read a fingerprint
    bool read_fingerprint();

private:
    Finger *m_finger;
    PubSubClient *m_mqtt;

    char m_opcode[OPCODE_MAX_LENGTH];
    bool m_codeAvailable;
};