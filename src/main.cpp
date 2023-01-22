#define TINY_GSM_MODEM_SIM7000SSL
#define TINY_GSM_RX_BUFFER 1024 // Set RX buffer to 1Kb

// Set serial for debug console (to Serial Monitor, default speed 115200)
#define SerialMon Serial
// Set serial for AT commands
#define SerialAT  Serial1

#include <TinyGsmClient.h>
#include <WiFiClientSecure.h>
#include <Config.h>



// LilyGO T-SIM7000G Pinout
#define UART_BAUD   115200
#define PIN_DTR     25
#define PIN_TX      27
#define PIN_RX      26
#define PWR_PIN     4

#define LED_PIN     12

TinyGsm modem(SerialAT);
TinyGsmClientSecure LTEclient(modem);

#include <PubSubClient.h>



WiFiClientSecure WIFIclient;
PubSubClient mqtt;

long lastMsg = 0;
char msg[50];
int value = 0;

boolean setup_wifi() {

    // We start by connecting to a WiFi network    
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(WIFI_SSID);

    
    
    WiFi.begin(WIFI_SSID, WIFI_PW);

    // Wait for 10 seconds 
    for (int i = 0; i < 100; i++)
    {
        if(WiFi.status() == WL_CONNECTED) {
            break;
        }
        
        delay(100);
    }
    
    

    return WiFi.status() == WL_CONNECTED;
}

boolean setup_lte() {
    modem.setNetworkMode(2);
    Serial.println("Setting up LTE");

    const char apn[] = GSM_APN; // SET TO YOUR APN
    const char gprsUser[] = GSM_GPRS_USER;
    const char gprsPass[] = GSM_GPRS_PASS;

    modem.waitForNetwork(600000L);

    if (!modem.gprsConnect(apn, gprsUser, gprsPass))
    {
        delay(10000);
    }
    
    if (modem.isGprsConnected())
    {
        Serial.println("connected");
        return true;
    }
    else
    {
        Serial.println("not connected");
        return false;
    }
}

boolean setup_internet()
{
    if(!setup_wifi()) {
        Serial.println("Not connected, trying to connect to LTE now");
        setup_lte();
    } else {
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
    }
    return false;
}

void setup_mqtt() {
    if(WiFi.status() == WL_CONNECTED) {
        mqtt.setClient(WIFIclient);
    } else if (modem.isGprsConnected()) {
        mqtt.setClient(LTEclient);
    } else {
        Serial.print("No connection to internet has been made");
        return;
    }
    mqtt.setServer(MQTT_SERVER, MQTT_PORT);
}

void modem_start() {
    // Set LED OFF
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    //Turn on the modem
    pinMode(PWR_PIN, OUTPUT);
    digitalWrite(PWR_PIN, HIGH);
    delay(300);
    digitalWrite(PWR_PIN, LOW);
}

void publish(char *topic, char const *message)
{
    Serial.println("hi");


    // Loop until we're reconnected
    while (!mqtt.connected())
    {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (mqtt.connect("ESP324G", MQTT_USER, MQTT_PASS))
        {
            Serial.println("connected");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(mqtt.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
    mqtt.loop();

    mqtt.publish(topic, message);
}

void setup()
{
    Serial.begin(115200);
    // WIFIclient.setInsecure();


    modem_start();

    delay(1000);

    // Set module baud rate and UART pins
    SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);

    // Restart takes quite some time
    // To skip it, call init() instead of restart()
    Serial.println("Initializing modem...");
    if (!modem.restart())
    {
        Serial.println("Failed to restart modem, attempting to continue without restarting");
        // client.publish("test/gps/log", "Failed to restart modem, attempting to continue without restarting");
    }

    // Print modem info
    String modemName = modem.getModemName();
    delay(500);
    Serial.println("Modem Name: " + modemName);

    String modemInfo = modem.getModemInfo();
    delay(500);
    Serial.println("Modem Info: " + modemInfo);

    // Setup the network connection
    setup_internet();
    setup_mqtt();

    publish("test/gps/log", "setting up gps");

    Serial.println("Place your board outside to catch satelite signal");
    publish("test/gps/log", "Place your board outside to catch satelite signal");
}

void loop()
{
    // Set SIM7000G GPIO4 HIGH ,turn on GPS power
    // CMD:AT+SGPIO=0,4,1,1
    // Only in version 20200415 is there a function to control GPS power
    while (true)
    {
    }

    modem.sendAT("+SGPIO=0,4,1,1");
    if (modem.waitResponse(10000L) != 1)
    {
        Serial.println(" SGPIO=0,4,1,1 false ");
    }

    modem.enableGPS();

    delay(15000);
    float lat = 0;
    float lon = 0;
    float speed = 0;
    float alt = 0;
    int vsat = 0;
    int usat = 0;
    float accuracy = 0;
    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
    int min = 0;
    int sec = 0;

    for (int8_t i = 15; i; i--)
    {
        Serial.println("Requesting current GPS/GNSS/GLONASS location");
        publish("test/gps/log", "Requesting current GPS/GNSS/GLONASS location");

        if (modem.getGPS(&lat, &lon, &speed, &alt, &vsat, &usat, &accuracy,
                         &year, &month, &day, &hour, &min, &sec))
        {
            Serial.println("Latitude: " + String(lat, 8) + "\tLongitude: " + String(lon, 8));
            Serial.println("Speed: " + String(speed) + "\tAltitude: " + String(alt));
            Serial.println("Visible Satellites: " + String(vsat) + "\tUsed Satellites: " + String(usat));
            Serial.println("Accuracy: " + String(accuracy));
            Serial.println("Year: " + String(year) + "\tMonth: " + String(month) + "\tDay: " + String(day));
            Serial.println("Hour: " + String(hour) + "\tMinute: " + String(min) + "\tSecond: " + String(sec));

            const String jsonstr = "{\"latitude\":" + String(lat, 8) + ", \"longitude\": " + String(lon, 8) + "}";

            Serial.print("Publishing gps location:");
            Serial.println(jsonstr);
        
            publish("test/gps/location", jsonstr.c_str());

            break;
        }
        else
        {
            Serial.println("Couldn't get GPS/GNSS/GLONASS location, retrying in 15s.");
            publish("test/gps/log", "Couldn't get GPS/GNSS/GLONASS location, retrying in 15s.");
            delay(15000L);
        }
    }
    Serial.println("Retrieving GPS/GNSS/GLONASS location again as a string");
    String gps_raw = modem.getGPSraw();
    Serial.println("GPS/GNSS Based Location String: " + gps_raw);

    Serial.println("Disabling GPS");
    modem.disableGPS();

    // Set SIM7000G GPIO4 LOW ,turn off GPS power
    // CMD:AT+SGPIO=0,4,1,0
    // Only in version 20200415 is there a function to control GPS power
    modem.sendAT("+SGPIO=0,4,1,0");
    if (modem.waitResponse(10000L) != 1)
    {
        Serial.println(" SGPIO=0,4,1,0 false ");
    }

    delay(1000);
}