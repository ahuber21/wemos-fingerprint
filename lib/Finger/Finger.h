#include <Arduino.h>
#include <FPM.h>
#include <SoftwareSerial.h>
#include <string>

class Finger
{
public:
    // create a new finger object using pins TX and DX
    Finger(const uint8_t TX = D2, const uint8_t = D3, uint32_t baud = 57600);

    // start up the serial communication with the server
    bool begin();

    // clear the database
    bool clear_database();

    // delete an existing finger
    bool delete_finger(int16_t fid, int16_t &status);

    // enroll a new finger at the specified fid
    bool enroll_finger(int16_t fid);

    // evaluate the status code
    bool evaluate_status(int16_t status);

    // get the next free id
    // returns true on success and updates the result argument
    bool get_free_id(int16_t &result);

    // count the number of templates in the database
    bool get_template_count(uint16_t &count, int16_t &status);

    // get the parameters of the fingerprint sensor
    FPM_System_Params get_params() { return m_params; }

    // print the fingerprint sensor's parameters
    void print_params();

    // read a fingerprint and store the ID and score in the result
    bool read_fingerprint(int16_t &status, uint16_t &fid, uint16_t &score);

    // read an image from the sensor
    bool read_image(uint8_t cycles, int16_t &status);

    // read an image and save it as template for verification
    bool read_template(int16_t &status);

    bool set_security_level(uint8_t level, int16_t &status);

    // convert the status to a string
    std::string status_to_string(const int16_t &status);

    // store the verified template as a new model at given ID
    bool store_model(int16_t fid, int16_t &status);

    // read an image and compare it against previously taken tempalte
    bool verify_template(int16_t &status);

private:
    SoftwareSerial m_serial;
    FPM m_fpm;
    uint32_t m_baud;
    FPM_System_Params m_params;
};
