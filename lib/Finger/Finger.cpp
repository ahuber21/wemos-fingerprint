#include <Finger.h>

Finger::Finger(const uint8_t TX, const uint8_t RX, uint32_t baud)
    : m_serial(TX, RX),
      m_fpm(&m_serial),
      m_baud(baud)
{
}

bool Finger::begin()
{
    bool status = true;
    // start the software serial
    m_serial.begin(m_baud);
    // start the finger sensor
    status &= m_fpm.begin();
    // read the parameters
    status &= evaluate_status(m_fpm.readParams(&m_params));

    return true;
}

void Finger::print_params()
{
    Serial.print("Capacity: ");
    Serial.println(m_params.capacity);
    Serial.print("Packet length: ");
    Serial.println(FPM::packet_lengths[m_params.packet_len]);
}

bool Finger::read_image(uint8_t cycles)
{
    uint16_t status;
    for (uint8_t i = 0; i < cycles; ++i)
    {
        status = m_fpm.getImage();
        if (status == FPM_OK)
        {
            return true;
        }
        // yield();
    }
    Serial.println("Failed to get image");
    return false;
}

bool Finger::read_template(int16_t &status)
{
    if (!read_image())
    {
        return false;
    }
    status = m_fpm.image2Tz(1);
    return status == FPM_OK;
}

bool Finger::verify_template(int16_t &status)
{
    if (!read_image())
    {
        return false;
    }
    status = m_fpm.image2Tz(2);
    return status == FPM_OK;
}

bool Finger::store_model(int16_t fid, int16_t &status)
{
    status = m_fpm.createModel();
    if (!evaluate_status(status))
        return false;

    status = m_fpm.storeModel(fid);
    if (!evaluate_status(status))
        return false;

    return true;
}

bool Finger::enroll_finger(int16_t fid)
{
    Serial.println("Waiting for valid finger to enroll");
    read_image();

    int16_t status = m_fpm.image2Tz(1);
    if (!evaluate_status(status))
        return false;

    Serial.println("Remove finger");
    delay(1000);

    Serial.println("Place same finger again");
    delay(1000);

    // give the user 5 attempts to place the finger correctly
    read_image();
    status = m_fpm.image2Tz(2);
    if (!evaluate_status(status))
        return false;

    status = m_fpm.createModel();
    if (!evaluate_status(status))
        return false;

    Serial.print("ID ");
    Serial.println(fid);
    status = m_fpm.storeModel(fid);
    if (!evaluate_status(status))
        return false;

    Serial.println("Enroll finished successfully");
    return true;
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
        // yield();
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
        Serial.print(".");
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
        Serial.println("Fingers did not match");
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

std::string Finger::status_to_string(const int16_t &status)
{
    switch (status)
    {
    case FPM_OK:
        return "FPM_OK";
    case FPM_IMAGEMESS:
        return "FPM_IMAGEMESS";
    case FPM_FEATUREFAIL:
        return "FPM_FEATUREFAIL";
    case FPM_INVALIDIMAGE:
        return "FPM_INVALIDIMAGE";
    case FPM_READ_ERROR:
        return "FPM_READ_ERROR";
    case FPM_NOFINGER:
        return "FPM_NOFINGER";
    case FPM_PACKETRECIEVEERR:
        return "FPM_PACKETRECIEVEERR";
    case FPM_IMAGEFAIL:
        return "FPM_IMAGEFAIL";
    case FPM_TIMEOUT:
        return "FPM_TIMEOUT";
    case FPM_ENROLLMISMATCH:
        return "FPM_ENROLLMISMATCH";
    case FPM_BADLOCATION:
        return "FPM_BADLOCATION";
    case FPM_FLASHERR:
        return "FPM_FLASHERR";
    case FPM_DBCLEARFAIL:
        return "FPM_DBCLEARFAIL";
    default:
        char buf[25];
        sprintf(buf, "UNKNOWN STATUS %d", status);
        return buf;
    }
}