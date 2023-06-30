// PROGRAMA DENOMINADO SD&AA.ino(Send data and activate alarms)
// PARA CAPTURAR DATOS DE LOS SENSORES USADOS EN EL BENEFICIO DEL CAFÉ
// Y ENVIAR A UNA BASE DE DATOS EN UN SERVIDOR WEB.
#include <esp_now.h>
#include <WiFi.h>
#include "DHT.h"
#include <dummy.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <BMP280_DEV.h>   
//Sensor de temperatura y humedad ambiente

#define DHTPIN 15     
#define DHTTYPE DHT11   
DHT dht(DHTPIN, DHTTYPE);
//DS18B20
#define ONE_WIRE_BUS 2
float temperature, pressure, altitude;            // Create the temperature, pressure and altitude variables
float tDH,hDH;
float tbmp;
BMP280_DEV bmp280;                        // Instantiate (create) a BMP280 object and set-up for I2C operation on pins SDA: A6, SCL: A7

// Variable to store if sending data was successful
String success;

OneWire oneWire(ONE_WIRE_BUS);

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

float incomingvar1_max;
float incomingvar2_max;
//Variables que tendrán un rango minimo y maximo según la etapa
typedef struct struct_message {
    float incomingvar1 ;  //Humedad, presión 
    float incomingvar2 ;  //temperatura ambiente, temperatura café
} struct_message;

// Create a struct_message called myData
struct_message myData;

// Create a struct_message to hold incoming sensor readings
struct_message incomingReadings;

esp_now_peer_info_t peerInfo;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status ==0){
    success = "Delivery Success :)";
  }
  else{
    success = "Delivery Fail :(";
  }
}

//Sensor de Co2

const int MQ2_PIN = 34 ;  // Pin del sensor
//float concentration = 25;
const int RL_VALUE = 15;  // Resistencia en Kilo ohms
const int R0 = 10;        // Resistencia en Kilo ohms

// Datos para lectura multiple
const int READ_SAMPLE_INTERVAL = 550;  // Tiempo entre muestras
const int READ_SAMPLE_TIMES = 12;      // Numero muestras


//Función de medición temperatura
DallasTemperature sensors(&oneWire);

// DATOS AJUSTADOS PARA MEDICIÓN DE CO 
const float X0 = 200;
const float Y0 = 5.1;     
const float X1 = 10000;
const float Y1 = 1.5;

// Puntos de la curva de concentración {X, Y}
const float punto0[] = { log10(X0), log10(Y0) };
const float punto1[] = { log10(X1), log10(Y1) };

// Calcular pendiente y coordenada abscisas
const float scope = (punto1[1] - punto0[1]) / (punto1[0] - punto0[0]);
const float coord = punto0[1] - punto0[0] * scope;

String dato;
String cade;
String line;

int Etapa = 0;
int gpio4_pin = 4;
int gpio5_pin = 5; // El GPIO5 de la tarjeta ESP32, corresponde al pin D5 identificado físicamente en la tarjeta. Este pin será utilizado para una salida de un LED.
int ID_TARJ=4; // Este dato identificará cual es la tarjeta que envía los datos, tener en cuenta que se tendrá más de una tarjeta. 
              // Se debe cambiar el dato (a 2,3,4...) cuando se grabe el programa en las demás tarjetas.
// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  Serial.print("Bytes received: ");
  Serial.println(len);
  incomingvar1_max = incomingReadings.incomingvar1;
  incomingvar2_max = incomingReadings.incomingvar2;
  
}

 
void setup() {
  //////////////////////////////////////////////////////TIPO DE PINES

  pinMode(gpio5_pin, OUTPUT);
  pinMode(gpio4_pin, OUTPUT);
  dht.begin();
  sensors.begin();
  bmp280.begin(0x76);  
  
  Serial.begin(115200);
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);
  
  // Default initialisation, place the BMP280 into SLEEP_MODE 
  bmp280.setTimeStandby(TIME_STANDBY_2000MS);     // Set the standby time to 2 seconds
  bmp280.startNormalConversion();                 // Start BMP280 continuous conversion in NORMAL_MODE
  
  
}

 
void loop() { 
  AA(); //Activated Alarms
  SBD();
}
// Ahora se guardan los valores medidos en la base de datos
void SBD(){
   
    bmp280.getMeasurements(temperature, pressure, altitude);
    tbmp = DS18B20();
    String url2 = "humedad=";
    url2 += hDH;
    url2 += "&temperatura=";
    url2 += tDH;
    url2 += "&ID_TARJ=";
    url2 += ID_TARJ;
    // Set values to send
    myData.incomingvar1 = temperature;

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
     
    if (result == ESP_OK) {
      Serial.println("Sent with success");
      digitalWrite(gpio5_pin, HIGH);
      Serial.print("Dato ENVIADO");
      delay(2000);
    }
    else {
      Serial.println("Error sending the data");
    }
    
    delay(10000);    
  }
  

void AA(){
    hDH = dht.readHumidity();
    // Read temperature as Celsius (the default)
    tDH = dht.readTemperature();
    tbmp = DS18B20();
    Serial.print("Humidity: ");
    Serial.print(hDH);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(tDH);
    Serial.print(" *C ");
    bmp280.getMeasurements(temperature, pressure, altitude);
    Serial.print(temperature);
    Serial.print(pressure);
    Serial.print(altitude);
    digitalWrite(gpio4_pin, LOW);
    
    
    
      switch (Etapa){
        case 1: //Fermentación
          break;
        case 2: //Secado
          break;
        case 3: //Almacenamiento
          cade = "Humedad max es...";
          cade += myData.incomingvar1;
          Serial.print(cade);
          cade = "Temp max es...";
          cade += myData.incomingvar2;
          Serial.print(cade);
          if (hDH > incomingReadings.incomingvar1){
             Serial.print("ALERTA HUMEDAD");
             digitalWrite(gpio4_pin, HIGH);
            }
          if (tDH > incomingReadings.incomingvar2){
             Serial.print("ALERTA TEMPERATURA");
             digitalWrite(gpio4_pin, HIGH);
            }
          break;
      }
      delay(2000);
    }
  
float DS18B20(){ 
  // call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");
  // After we got the temperatures, we can print them here.
  // We use the function ByIndex, and as an example get the temperature from the first sensor only.
  float tempC = sensors.getTempCByIndex(0);

  // Check if reading was successful
  if(tempC != DEVICE_DISCONNECTED_C) 
  {
    Serial.print("Temperature for the device 1 (index 0) is: ");
    Serial.println(tempC);
    return tempC;
  } 
  else
  {
    Serial.println("Error: Could not read temperature data");
  }
  
}
