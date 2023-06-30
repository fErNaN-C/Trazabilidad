// PROGRAMA DENOMINADO programa1.ino
// PARA CAPTURAR DATOS DE LOS SENSORES
// Y ENVIAR A UNA BASE DE DATOS EN UN SERVIDOR WEB.
#include <esp_now.h>
#include <WiFi.h>
#include <ESP8266WiFi.h>
#include <Wire.h>

// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

DHT dht(DHTPIN, DHTTYPE);
const char* ssid     = "Android1";      // SSID
const char* password = "labiii2018";      // Password
const char* host = "192.168.43.110";  // Dirección IP local o remota, del Servidor Web
const int   port = 80;            // Puerto, HTTP es 80 por defecto, cambiar si es necesario.
const int   watchdog = 2000;        // Frecuencia del Watchdog
unsigned long previousMillis = millis(); 


// Define variables to store incoming readings
float incomingTemp;
//variables

String  dato;
int     recividos;
int gpio4_pin = 4; // Se debe tener en cuenta que el GPIO4 es el pin D4, ver imagen de GPIOs de la tarjeta ESP32. Este pin será utilizado para una salida de un LED para alertas.

// Variable to store if sending data was successful
String success;

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
    String var1_max ;  //Humedad, presión 
    String var2_max ;  //temperatura ambiente, temperatura café
} struct_message;

// Create a struct_message called BME280Readings to hold sensor readings
struct_message SDRDReadings;      //send data recive data => readings of php program

// Create a struct_message to hold incoming sensor readings
struct_message incomingReadings;  //send data recive data => incoming SDRD

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
// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  Serial.print("Bytes received: ");
  Serial.println(len);
  incomingTemp = incomingReadings.temp;
} 
void setup() {
  pinMode(gpio4_pin, OUTPUT);
  Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA);
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  Serial.print("Conectando a...");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi conectado");  
  Serial.println("Dirección IP: ");
  Serial.println(WiFi.localIP());
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
}
 
void loop() {
  
}
void AA(){
    unsigned long currentMillis = millis();

      switch (Etapa){
        case 1: //Fermentación
        
          break;
        case 2: //Secado
          break;
        case 3: //Almacenamiento
          // Primero se consultan los datos maximos de temp y hum
            if ( currentMillis - previousMillis > watchdog ) {
            previousMillis = currentMillis;
            WiFiClient client;
            
            if (!client.connect(host, port)) {
              Serial.println("Conexión falló...");
              return;
            }
            
            String url = "/programas_php/proceso_eventos/programa5.php";
            // Envío de la solicitud al Servidor
            client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                       "Host: " + host + "\r\n" + 
                       "Connection: close\r\n\r\n");
            unsigned long timeout = millis();
            while (client.available() == 0) {
              if (millis() - timeout > 5000) {
                Serial.println(">>> Superado tiempo de espera!");
                return;
              }
            }
            // Lee respuesta del servidor
            while(client.available()){
              line = client.readStringUntil('\r');
              Serial.print(line);
            }
              int longitud = line.length();
              int longitud_f = longitud;
              longitud = longitud - 4;
              
              dato = line.substring(longitud,longitud_f);
              cade = "Dato recibido es...";
              cade += dato; 
              Serial.print(cade);
            
              var1_max = dato.substring(2,4);
              var2_max = dato.substring(0,2);
               
              // Lo siguiente se utiliza para pasar la cadena de texto a un flotante, para poder comparar
              char cadena1[var1_max.length()+1];
              var1_max.toCharArray(cadena1, var1_max.length()+1);
              // Set values to send
              Readings.v1_max = atof(cadena1); 
              // Lo siguiente se utiliza para pasar la cadena de texto a un flotante, para poder comparar
              char cadena2[var2_max.length()+1];
              var2_max.toCharArray(cadena2, var2_max.length()+1);
              // Set values to send
              Readings.v2_max = atof(cadena2);
              }
              break;
        // Send message via ESP-NOW
        esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &Readings, sizeof(Readings));
         
        if (result == ESP_OK) {
          Serial.println("Sent with success");
        }
        else {
          Serial.println("Error sending the data");
        }
        delay(200);
        } 
  }
              
              
  void SBD(){
    
    
   unsigned long currentMillis = millis();
   if ( currentMillis - previousMillis > watchdog ) {
    previousMillis = currentMillis;
    WiFiClient client;
  
    if (!client.connect(host, port)) {
      Serial.println("Conexión falló...");
      return;
    }

    String url2 = "/programas_php/proceso_eventos/programa1.php?humedad=";
    url2 += hDH;
    url2 += "&temperatura=";
    url2 += tDH;
    url2 += "&ID_TARJ=";
    url2 += ID_TARJ;
    
    // Envío de la solicitud al Servidor
    client.print(String("POST ") + url2 + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
    unsigned long timeout2 = millis();
    while (client.available() == 0) {
      if (millis() - timeout2 > 5000) {
        Serial.println(">>> Superado tiempo de espera!");
        client.stop();
        return;
      }
    }
  
    // Lee respuesta del servidor
    while(client.available()){
      line = client.readStringUntil('\r');
      Serial.print(line);
    }
      digitalWrite(gpio5_pin, HIGH);
      Serial.print("Dato ENVIADO");
      delay(2000);
  }
  
}
