#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <DNSServer.h>
#include <HTTPClient.h>

// Constants for Access Point mode
const char *ssidAP = "ESP32_Config";
const char *passwordAP = "12345678";
const byte DNS_PORT = 53;

// Create DNS and WebServer objects
DNSServer dnsServer;
WebServer server(80);

// Variables to store WiFi and server settings
String localServerURL;
String wifiSSID;
String wifiPassword;

void setup()
{
    Serial.begin(115200);
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
    }
    server.handleClient();
}

void startConfigPortal()
{
    WiFi.softAP(ssidAP, passwordAP);
    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

    setupWebServerRoutes();
    server.begin();

    Serial.println("AP Mode: Config page served");
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
    html += "<input type='submit' value='Save'>";
    html += "</form>";
    html += "</body></html>";
    server.send(200, "text/html", html);
}

void handleSave()
{
    if (server.method() == HTTP_POST)
    {
        if (server.hasArg("ssid") && server.hasArg("password") && server.hasArg("url"))
        {
            wifiSSID = server.arg("ssid");
            wifiPassword = server.arg("password");
            localServerURL = server.arg("url");

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
    EEPROM.commit();
}

void loadCredentialsFromEEPROM()
{
    wifiSSID = EEPROM.readString(0);
    wifiPassword = EEPROM.readString(32);
    localServerURL = EEPROM.readString(64);
}

// Optional: Function to send a request to the local server
void sendRequestToServer()
{
    if (WiFi.status() == WL_CONNECTED && localServerURL.length() > 0)
    {
        HTTPClient http;
        http.begin(localServerURL);
        int httpCode = http.GET();
        if (httpCode > 0)
        {
            String payload = http.getString();
            Serial.println(payload);
        }
        http.end();
    }
}
