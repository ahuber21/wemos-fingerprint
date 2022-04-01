#include <Fingerprint.h>

Finger::Finger(const uint8_t TX, const uint8_t RX, uint32_t baud)
    : m_serial(TX, RX),
      m_fpm(&m_serial),
      m_baud(baud)
{
}

bool Finger::begin()
{
    // start the software serial
    m_serial.begin(m_baud);
    // start the finger sensor
    m_fpm.begin();
    // read the parameters
    m_fpm.readParams(&m_params);

    return true;
}

void Finger::print_params()
{
    Serial.print("Capacity: ");
    Serial.println(m_params.capacity);
    Serial.print("Packet length: ");
    Serial.println(FPM::packet_lengths[m_params.packet_len]);
}

bool Finger::get_image()
{
    uint16_t status;
    for (int i = 0; i < 10; ++i)
    {
        status = m_fpm.getImage();
        bool success = evaluate_status(status);
        if (success)
        {
            return success;
        }
        yield();
    }
    Serial.println("Failed to get image");
    return false;
}

int16_t Finger::enroll_finger(int16_t fid)
{
    Serial.println("Waiting for valid finger to enroll");
    get_image();

    int16_t status = m_fpm.image2Tz(1);
    if (!evaluate_status(status))
        return status;

    Serial.println("Remove finger");
    delay(500);
    get_image();

    Serial.println("Place same finger again");
    get_image();

    status = m_fpm.image2Tz(2);
    if (!evaluate_status(status))
        return status;

    status = m_fpm.createModel();
    if (!evaluate_status(status))
        return status;

    Serial.print("ID ");
    Serial.println(fid);
    status = m_fpm.storeModel(fid);
    if (!evaluate_status(status))
        return status;

    Serial.println("Enroll finished successfully");
    return status;
}

bool Finger::get_free_id(int16_t &result)
{
    int16_t status;
    for (int page = 0; page < (m_params.capacity / FPM_TEMPLATES_PER_PAGE) + 1; page++)
    {
        status = m_fpm.getFreeIndex(page, &result);

        if (!evaluate_status(status))
        {
            // read error
            return false;
        }

        if (result != FPM_NOFREEINDEX)
        {
            // found a free slot
            Serial.print("Free slot at ID ");
            Serial.println(result);
            return true;
        }

        // nothing found on this page, move on to next
        yield();
    }

    Serial.println("No free slots!");
    return false;
}

bool Finger::clear_database()
{
    int16_t status = m_fpm.emptyDatabase();
    return evaluate_status(status);
}

bool Finger::evaluate_status(int16_t status)
{
    switch (status)
    {
    case FPM_OK:
        Serial.println("Image taken");
        return true;
    case FPM_IMAGEMESS:
        Serial.println("Image too messy");
        break;
    case FPM_FEATUREFAIL:
        Serial.println("Could not find fingerprint features");
        break;
    case FPM_INVALIDIMAGE:
        Serial.println("Could not find fingerprint features");
        break;
    case FPM_READ_ERROR:
        Serial.println("Got wrong PID or length!");
        break;
    case FPM_NOFINGER:
        Serial.println(".");
        break;
    case FPM_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
    case FPM_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
    case FPM_TIMEOUT:
        Serial.println("Timeout!");
        break;
    case FPM_ENROLLMISMATCH:
        Serial.println("Fingerprints did not match");
        break;
    case FPM_BADLOCATION:
        Serial.println("Could not store in that location");
        break;
    case FPM_FLASHERR:
        Serial.println("Error writing to flash");
        break;
    case FPM_DBCLEARFAIL:
        Serial.println("Could not clear database!");
        break;
    default:
        Serial.print("Unknown error: ");
        Serial.println(status);
        break;
    }
    return false;
}
