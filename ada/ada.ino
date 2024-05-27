#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <DNSServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <string.h>
#include <stdio.h>

#define SS_PIN 5
#define RST_PIN 2
const int ipaddress[4] = {103, 97, 67, 25};
byte nuidPICC[4] = {0, 0, 0, 0};
MFRC522::MIFARE_Key key;
MFRC522 rfid = MFRC522(SS_PIN, RST_PIN);

#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 32    // OLED display height, in pixels
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Constants for Access Point mode
const char *ssidAP = "ADA_Config";
const char *passwordAP = "12345678";
const byte DNS_PORT = 53;

// Create DNS and WebServer objects
DNSServer dnsServer;
WebServer server(80);

// Variables to store WiFi and server settings
String localServerURL;
String wifiSSID;
String wifiPassword;

String roomName;
String roomRealName;

const unsigned char ada_logo_bitmap[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x07, 0xfe, 0x00, 0x01, 0xfc, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x1f, 0xff, 0x80, 0x03, 0xfe, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0x00, 0x3f, 0xff, 0xe0, 0x07, 0xff, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0x80, 0x7f, 0xff, 0xf0, 0x07, 0xff, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0x80, 0x78, 0x01, 0xf8, 0x0f, 0x8f, 0x80, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0x80, 0x70, 0x00, 0x7c, 0x0f, 0x8f, 0x80, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0x80, 0x70, 0x00, 0x3c, 0x0f, 0x07, 0x80, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0x80, 0x70, 0x00, 0x1e, 0x0f, 0x07, 0x80, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0x00, 0x70, 0x00, 0x1e, 0x1f, 0x07, 0xc0, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x70, 0x00, 0x1e, 0x1e, 0x03, 0xc0, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x70, 0x00, 0x1e, 0x1e, 0x03, 0xc0, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x1e, 0x3f, 0xff, 0xe0, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x1e, 0x3f, 0xff, 0xe0, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xe0, 0x70, 0x00, 0x1e, 0x3f, 0xff, 0xe0, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xf0, 0x70, 0x00, 0x1e, 0x7f, 0xff, 0xf0, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xf8, 0x70, 0x00, 0x3c, 0x7c, 0x01, 0xf0, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xf8, 0x70, 0x00, 0x7c, 0x7c, 0x01, 0xf0, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xfc, 0x78, 0x01, 0xf8, 0xf8, 0x00, 0xf8, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x7c, 0x7f, 0xff, 0xf0, 0xf8, 0x00, 0xf8, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xfd, 0xff, 0x7e, 0x3f, 0xff, 0xe0, 0xf0, 0x00, 0x78, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xfd, 0xff, 0x7e, 0x1f, 0xff, 0x80, 0xf0, 0x00, 0x78, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x7d, 0xff, 0x7c, 0x07, 0xfe, 0x00, 0x60, 0x00, 0x30, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

void setup()
{
    Serial.begin(9600);

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Don't proceed, loop forever
    }
    display.clearDisplay();

    // initialize rfid D8,D5,D6,D7
    SPI.begin();
    rfid.PCD_Init();
    Serial.print(F("Reader :"));
    rfid.PCD_DumpVersionToSerial();

    // TODO: show ada logo
    display.drawBitmap(0, 0, ada_logo_bitmap, 128, 32, 1);
    display.display();
    delay(2000);

    EEPROM.begin(512); // Initialize EEPROM with 512 bytes

    // Load saved credentials from EEPROM
    loadCredentialsFromEEPROM();

    // If credentials are available, try to connect to WiFi
    if (wifiSSID.length() > 0 && wifiPassword.length() > 0)
    {
        connectToWiFi();
    }
    else
    {
        startConfigPortal();
    }
}

void loop()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        dnsServer.processNextRequest();
        server.handleClient();
    }
    else
    {
        server.handleClient();
        // read rfid
        String rfid = "";
        rfid = readRFID();
        if (rfid != "")
        {
            display.clearDisplay();
            display.setTextSize(1);              // Normal 1:1 pixel scale
            display.setTextColor(SSD1306_WHITE); // Draw white text
            display.setCursor(0, 0);             // Start at top-left corner
            display.cp437(true);                 // Use full 256 char 'Code Page 437' font

            display.println("Please wait...");
            display.display();

            Serial.println(rfid);

            // request student info based on rfid value
            display.clearDisplay();
            getStudentInfo(rfid);

            // display feedback: student info, entry status, rfid value
            // displayRFIDValue(rfid); // TODO: update display, move to bottom

            // if data is valid
            // display student info at the top
            // get last student record filtered with room name
            // if no record or the last record is out
            // post new room-in record
            // if record is in
            // post new room-out record

            // display room entry at middle part

            // else if invalid
            // display not yet registered
        }
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

void displayRFIDValue(String rfidValue)
{

    display.println(rfidValue);

    display.display();
}

void loadCredentialsFromEEPROM()
{
    wifiSSID = EEPROM.readString(0);
    wifiPassword = EEPROM.readString(32);
    localServerURL = EEPROM.readString(64);
    roomName = EEPROM.readString(96);
    roomRealName = EEPROM.readString(128);
}

void connectToWiFi()
{
    WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
    int attempt = 0;
    while (WiFi.status() != WL_CONNECTED && attempt < 20)
    {
        delay(500);
        Serial.print(".");
        attempt++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\nConnected to WiFi");
        // Start web server to allow configuration updates
        setupWebServerRoutes();
        server.begin();
        Serial.print("Server started at ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        Serial.println("\nFailed to connect. Starting AP mode for configuration");
        startConfigPortal();
    }
}

void startConfigPortal()
{
    WiFi.softAP(ssidAP, passwordAP);
    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

    setupWebServerRoutes();
    server.begin();

    Serial.println("AP Mode: Config page served");
}

void setupWebServerRoutes()
{
    server.on("/", handleRoot);
    server.on("/save", handleSave);
}

void handleRoot()
{
    String html = "<html><body>";
    html += "<form action='/save' method='POST'>";
    html += "SSID: <input type='text' name='ssid'><br>";
    html += "Password: <input type='text' name='password'><br>";
    html += "Server URL: <input type='text' name='url'><br>";
    html += "Room Name: <input type='text' name='roomName'><br>";
    html += "Room Real Name: <input type='text' name='roomRealName'><br>";
    html += "<input type='submit' value='Save'>";
    html += "</form>";
    html += "</body></html>";
    server.send(200, "text/html", html);
}

void handleSave()
{
    if (server.method() == HTTP_POST)
    {
        if (server.hasArg("ssid") && server.hasArg("password") && server.hasArg("url") && server.hasArg("roomName") && server.hasArg("roomRealName"))
        {
            wifiSSID = server.arg("ssid");
            wifiPassword = server.arg("password");
            localServerURL = server.arg("url");
            roomName = server.arg("roomName");
            roomRealName = server.arg("roomRealName");

            saveCredentialsToEEPROM();

            server.send(200, "text/html", "Settings saved. Rebooting...");
            delay(3000);
            ESP.restart();
        }
        else
        {
            server.send(400, "text/html", "Invalid Input");
        }
    }
}

void saveCredentialsToEEPROM()
{
    EEPROM.writeString(0, wifiSSID);
    EEPROM.writeString(32, wifiPassword);
    EEPROM.writeString(64, localServerURL);
    EEPROM.writeString(96, roomName);
    EEPROM.writeString(128, roomRealName);
    EEPROM.commit();
}

void getStudentInfo(String rfid)
{
    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;

        String studentsCN = "students/records";

        // Construct the full URL dynamically including the expand parameter
        String endpoint = "/api/collections/" + studentsCN + constructStudentQuery(rfid);
        String serverPath = localServerURL + endpoint;

        http.begin(serverPath.c_str());

        int httpResponseCode = http.GET();

        if (httpResponseCode > 0)
        {
            String payload = http.getString();
            Serial.println(httpResponseCode);
            Serial.println(payload);

            // Process the JSON response
            DynamicJsonDocument doc(2048);
            DeserializationError error = deserializeJson(doc, payload);

            if (!error)
            {
                int totalItems = doc["totalItems"];
                if (totalItems == 0)
                {
                    display.println("Not yet registered");
                    display.println("");
                    display.println(rfid);
                    display.display();
                    return;
                }

                JsonObject studentjson = doc["items"][0];
                const char *id = studentjson["id"];
                const char *studentName = studentjson["name"];
                const char *schoolId = studentjson["schoolId"];

                // TODO: get record of latest filtered with student record id and room name
                getRecordInfo(id, studentName, rfid);

                Serial.print("Record ID: ");
                Serial.println(id);
                Serial.print("Student Name: ");
                Serial.println(studentName);
                Serial.print("School ID: ");
                Serial.println(schoolId);
            }
            else
            {
                Serial.print("deserializeJson() failed: ");
                Serial.println(error.c_str());
            }
        }
        else
        {
            display.println("Entry failed");
            display.println("Retry again");
            display.display();

            Serial.print("Error on HTTP request: ");
            Serial.println(httpResponseCode);
        }

        http.end();
    }
}

void getRecordInfo(String studentRecordId, String studentName, String rfid)
{
    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;

        String recordsCollectionName = "records/records";

        // Construct the full URL dynamically including the expand parameter
        String endpoint = "/api/collections/" + recordsCollectionName + constructRecordQuery(studentRecordId);
        String serverPath = localServerURL + endpoint;

        http.begin(serverPath.c_str());

        int httpResponseCode = http.GET();

        if (httpResponseCode > 0)
        {
            String payload = http.getString();
            Serial.println(httpResponseCode);
            // Serial.println(payload);

            // Process the JSON response
            DynamicJsonDocument doc(2048);
            DeserializationError error = deserializeJson(doc, payload);

            if (!error)
            {
                int totalItems = doc["totalItems"];

                if (totalItems == 0)
                {
                    display.println(studentName);
                    String roomEntry = roomRealName + " -> IN";
                    display.println(roomEntry);
                    display.println(rfid);
                    display.display();

                    Serial.println("No records, creating entry..");

                    postEntryRequest(studentRecordId, "in");
                    return;
                }

                JsonObject record = doc["items"][0];
                const char *id = record["id"];
                String room = record["room"];
                const char *entry = record["entry"];

                Serial.println("Record entry value: ");
                Serial.println(entry);

                if (strcmp(entry, "in") == 0)
                {
                    display.println(studentName);
                    String roomEntry = room + " -> OUT";
                    display.println(roomEntry);
                    display.println(rfid);
                    display.display();

                    Serial.println("Entry out, creating post request..");

                    postEntryRequest(studentRecordId, "out");
                }
                else
                {
                    display.println(studentName);
                    String roomEntry = room + " -> IN";
                    display.println(roomEntry);
                    display.println(rfid);
                    display.display();

                    Serial.println("Entry in, creating post request..");

                    postEntryRequest(studentRecordId, "in");
                }
            }
            else
            {
                Serial.print("deserializeJson() failed: ");
                Serial.println(error.c_str());
            }
        }
        else
        {
            Serial.print("Error on HTTP request: ");
            Serial.println(httpResponseCode);
        }

        http.end();
    }
}

String constructStudentQuery(String rfid)
{
    return "?filter=rfid='" + rfid + "'";
}

String constructRecordQuery(String studentRecordId)
{
    return "?filter=%28student%3D%22" + studentRecordId + "%22%26%26room%3D%22" + roomName + "%22" + "%29&sort=-created,-entryDate&limit=1";
}

String constructQuery(String rfid, String expand)
{
    return "?filter=student.rfid%3D'\"" + rfid + "\"'&expand=\"" + expand + "\"&sort=-created&limit=1";
}

void postEntryRequest(String studentCollectionId, String entry)
{
    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;

        String recordsCollectionName = "records/records";

        // Construct the full URL dynamically including the expand parameter
        String endpoint = "/api/collections/" + recordsCollectionName;
        String serverPath = localServerURL + endpoint;

        http.begin(serverPath.c_str());
        http.addHeader("Content-Type", "application/json");

        String httpRequestData = "{\"student\":\"" + studentCollectionId + "\", \"entry\":\"" + entry + "\", \"room\":\"" + roomRealName + "\"}";

        int httpResponseCode = http.POST(httpRequestData);

        if (httpResponseCode > 0)
        {
            String response = http.getString();
            Serial.println(httpResponseCode);
            Serial.println(response);
        }
        else
        {
            Serial.print("Error on sending POST: ");
            Serial.println(httpResponseCode);
        }

        http.end();
    }
    else
    {
        Serial.println("WiFi Disconnected");
    }
}
