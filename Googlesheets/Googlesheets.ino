#include <WiFi.h>
// Replace with your SSID and Password
const char* ssid = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";
// Replace with your unique IFTTT URL resource
const char* resource = "REPLACE_WITH_YOUR_IFTTT_URL_RESOURCE";
// How your resource variable should look like, but with your own API KEY (that API KEY below is just an
example):
//const char* resource = "/trigger/DHT11_reading/with/key/nAZjOphL3d-ZO4N3k64-
1A7gTlNSrxMJdmqy3";
// Maker Webhooks IFTTT
const char* server = "maker.ifttt.com";

// Time to sleep
uint64_t uS_TO_S_FACTOR = 1000000; // Conversion factor for micro seconds to seconds
// sleep for 30 minutes = 1800 seconds
uint64_t TIME_TO_SLEEP = 1800;
#include "DHT.h"
#define DHTPIN 4 // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11 // DHT 11
DHT dht(DHTPIN, DHTTYPE);
void setup() {
Serial.begin(115200);
delay(2000);
dht.begin();
initWifi();
makeIFTTTRequest();
// enable timer deep sleep
esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
Serial.println("Going to sleep now");
// start deep sleep for 3600 seconds (60 minutes)
esp_deep_sleep_start();
}
void loop() {
// sleeping so wont get here
}
// Establish a Wi-Fi connection with your router
void initWifi() {
Serial.print("Connecting to: ");
Serial.print(ssid);
WiFi.begin(ssid, password);
int timeout = 10 * 4; // 10 seconds
while(WiFi.status() != WL_CONNECTED && (timeout-- > 0)) {
delay(250);
Serial.print(".");
}
Serial.println("");
if(WiFi.status() != WL_CONNECTED) {
Serial.println("Failed to connect, going back to sleep");
}
Serial.print("WiFi connected in: ");
Serial.print(millis());
Serial.print(", IP address: ");
Serial.println(WiFi.localIP());
}
// Make an HTTP request to the IFTTT web service
void makeIFTTTRequest() {
Serial.print("Connecting to ");
Serial.print(server);
WiFiClient client;
int retries = 5;
while(!!!client.connect(server, 80) && (retries-- > 0)) {
Serial.print(".");
}
Serial.println();

if(!!!client.connected()) {
Serial.println("Failed to connect...");
}
Serial.print("Request resource: ");
Serial.println(resource);
// Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
float humidity = dht.readHumidity();
// Read temperature as Celsius (the default)
float temperature = dht.readTemperature();
// Read temperature as Fahrenheit (isFahrenheit = true)
float fh_temperature = dht.readTemperature(true);

// Temperature in Celsius
String jsonObject = String("{\"value1\":\"") + temperature + "\",\"value2\":\"" + fh_temperature
+ "\",\"value3\":\"" + humidity + "\"}";
client.println(String("POST ") + resource + " HTTP/1.1");
client.println(String("Host: ") + server);
client.println("Connection: close\r\nContent-Type: application/json");
client.print("Content-Length: ");
client.println(jsonObject.length());
client.println();
client.println(jsonObject);
int timeout = 5 * 10; // 5 seconds
while(!!!client.available() && (timeout-- > 0)){
delay(100);
}
if(!!!client.available()) {
Serial.println("No response...");
}
while(client.available()){
Serial.write(client.read());
}
Serial.println("\nclosing connection");
client.stop();
}
