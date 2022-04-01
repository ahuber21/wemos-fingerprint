#include <Arduino.h>
#include <FPM.h>
#include <SoftwareSerial.h>

class Finger
{
public:
    // create a new finger object using pins TX and DX
    Finger(const uint8_t TX = D2, const uint8_t = D3, uint32_t baud = 57600);

    // start up the serial communication with the server
    bool begin();

    // enroll a new finger at the specified fid
    int16_t enroll_finger(int16_t fid);

    // clear the database
    bool clear_database();

    // get the next free id
    // returns true on success and updates the result argument
    bool get_free_id(int16_t &result);

    // get the parameters of the fingerprint sensor
    FPM_System_Params get_params() { return m_params; }

    // print the fingerprint sensor's parameters
    void print_params();

protected:
    bool get_image();
    bool evaluate_status(int16_t status);

private:
    SoftwareSerial m_serial;
    FPM m_fpm;
    uint32_t m_baud;
    FPM_System_Params m_params;
};
