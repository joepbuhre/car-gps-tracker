#define TINY_GSM_MODEM_SIM7000
#define TINY_GSM_RX_BUFFER 1024 // Set RX buffer to 1Kb

#include <TinyGsmClient.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Config.h>



// LilyGO T-SIM7000G Pinout
#define UART_BAUD   115200
#define PIN_DTR     25
#define PIN_TX      27
#define PIN_RX      26
#define PWR_PIN     4

#define LED_PIN     12

// Set serial for debug console (to Serial Monitor, default speed 115200)
#define SerialMon Serial
// Set serial for AT commands
#define SerialAT  Serial1

TinyGsm modem(SerialAT);

void setup_wifi() {
  delay(10);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PW);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

WiFiClientSecure espClient;
PubSubClient client(MQTT_SERVER, MQTT_PORT, espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP324G", MQTT_USER, MQTT_PASS)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  client.loop();
}

void setup(){
  SerialMon.begin(115200);
  setup_wifi();
  espClient.setInsecure();

  if (!client.connected()) {
      reconnect();
  }
  

  client.publish("test/gps/log", "setting up gps");

  SerialMon.println("Place your board outside to catch satelite signal");
  client.publish("test/gps/log","Place your board outside to catch satelite signal");


  // Set LED OFF
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  //Turn on the modem
  pinMode(PWR_PIN, OUTPUT);
  digitalWrite(PWR_PIN, HIGH);
  delay(300);
  digitalWrite(PWR_PIN, LOW);

  delay(1000);
  
  // Set module baud rate and UART pins
  SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  SerialMon.println("Initializing modem...");
  if (!modem.restart()) {
    Serial.println("Failed to restart modem, attempting to continue without restarting");
    client.publish("test/gps/log","Failed to restart modem, attempting to continue without restarting");
  }
  
  // Print modem info
  String modemName = modem.getModemName();
  delay(500);
  SerialMon.println("Modem Name: " + modemName);

  String modemInfo = modem.getModemInfo();
  delay(500);
  SerialMon.println("Modem Info: " + modemInfo);
}

void loop(){
  // Set SIM7000G GPIO4 HIGH ,turn on GPS power
  // CMD:AT+SGPIO=0,4,1,1
  // Only in version 20200415 is there a function to control GPS power
  modem.sendAT("+SGPIO=0,4,1,1");
  if (modem.waitResponse(10000L) != 1) {
    SerialMon.println(" SGPIO=0,4,1,1 false ");
  }

  modem.enableGPS();
  
  delay(15000);
  float lat      = 0;
  float lon      = 0;
  float speed    = 0;
  float alt     = 0;
  int   vsat     = 0;
  int   usat     = 0;
  float accuracy = 0;
  int   year     = 0;
  int   month    = 0;
  int   day      = 0;
  int   hour     = 0;
  int   min      = 0;
  int   sec      = 0;
  
  for (int8_t i = 15; i; i--) {
    SerialMon.println("Requesting current GPS/GNSS/GLONASS location");
    client.publish("test/gps/log","Requesting current GPS/GNSS/GLONASS location");

    if (modem.getGPS(&lat, &lon, &speed, &alt, &vsat, &usat, &accuracy,
                     &year, &month, &day, &hour, &min, &sec)) {
      SerialMon.println("Latitude: " + String(lat, 8) + "\tLongitude: " + String(lon, 8));
      SerialMon.println("Speed: " + String(speed) + "\tAltitude: " + String(alt));
      SerialMon.println("Visible Satellites: " + String(vsat) + "\tUsed Satellites: " + String(usat));
      SerialMon.println("Accuracy: " + String(accuracy));
      SerialMon.println("Year: " + String(year) + "\tMonth: " + String(month) + "\tDay: " + String(day));
      SerialMon.println("Hour: " + String(hour) + "\tMinute: " + String(min) + "\tSecond: " + String(sec));

      const String jsonstr = "{\"latitude\":"+ String(lat,8) +", \"longitude\": " + String(lon, 8) + "}";
      
      Serial.print("Publishing gps location:");
      Serial.println(jsonstr);

      if(!client.connected()) {
        reconnect();
      }

      client.publish("test/gps/location", jsonstr.c_str());

      break;
    } 
    else {
      SerialMon.println("Couldn't get GPS/GNSS/GLONASS location, retrying in 15s.");
      if(!client.connected()) {
        reconnect();
      }
      client.publish("test/gps/log","Couldn't get GPS/GNSS/GLONASS location, retrying in 15s.");
      delay(15000L);
    }
  }
  SerialMon.println("Retrieving GPS/GNSS/GLONASS location again as a string");
  String gps_raw = modem.getGPSraw();
  SerialMon.println("GPS/GNSS Based Location String: " + gps_raw);

  SerialMon.println("Disabling GPS");
  modem.disableGPS();

  // Set SIM7000G GPIO4 LOW ,turn off GPS power
  // CMD:AT+SGPIO=0,4,1,0
  // Only in version 20200415 is there a function to control GPS power
  modem.sendAT("+SGPIO=0,4,1,0");
  if (modem.waitResponse(10000L) != 1) {
    SerialMon.println(" SGPIO=0,4,1,0 false ");
  }

  delay(1000);

}