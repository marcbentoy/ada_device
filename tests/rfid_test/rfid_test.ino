#include <SPI.h>
#include <MFRC522.h>

// Constants
#define SS_PIN 5
#define RST_PIN 2
// Parameters
const int ipaddress[4] = {103, 97, 67, 25};
// Variables
byte nuidPICC[4] = {0, 0, 0, 0};
MFRC522::MIFARE_Key key;
MFRC522 rfid = MFRC522(SS_PIN, RST_PIN);

void setup()
{
    // Init Serial USB
    Serial.begin(9600);
    Serial.println(F("Initialize System"));

    // initialize rfid D8,D5,D6,D7
    SPI.begin();
    rfid.PCD_Init();
    Serial.print(F("Reader :"));
    rfid.PCD_DumpVersionToSerial();
}

void loop()
{
    String rfid = "";
    rfid = readRFID();
    if (rfid != "")
    {
        Serial.println(rfid);
    }
}

String readRFID()
{
    String rfidValue = "";
    for (byte i = 0; i < 6; i++)
    {
        key.keyByte[i] = 0xFF;
    }
    // Look for new 1 cards
    if (!rfid.PICC_IsNewCardPresent())
        return "";
    // Verify if the NUID has been readed
    if (!rfid.PICC_ReadCardSerial())
        return "";
    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++)
    {
        nuidPICC[i] = rfid.uid.uidByte[i];
    }
    Serial.print(F("RFID In dec: "));
    rfidValue = arrByteToHexString(rfid.uid.uidByte, rfid.uid.size);

    // Halt PICC
    rfid.PICC_HaltA();
    // Stop encryption on PCD
    rfid.PCD_StopCrypto1();

    return rfidValue;
}

String arrByteToHexString(byte *buffer, byte bufferSize)
{
    String hexString = "";

    for (byte i = 0; i < bufferSize; i++)
    {
        if (buffer[i] < 0x10)
        {
            hexString += '0';
            continue;
        }

        hexString += String(buffer[i], HEX);
    }

    return hexString;
}