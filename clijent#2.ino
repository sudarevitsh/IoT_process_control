#include <ESP8266WiFi.h> 
#include "DHT.h"

#define DHT_PIN 13
#define DHT_TYPE DHT21
DHT dht(DHT_PIN, DHT_TYPE);

#define AP_SSID "Zavrsni_Rad"
#define AP_PASS "12345678"
const char* ssid = AP_SSID;            
const char* password = AP_PASS;            

byte port = 80;                          
String host_str = "8.8.8.8";                   
String route = "/client2/";                   

WiFiClient client;                           
byte id = 2;                              

float dht_temperature = 0;                                   
float dht_humidity = 0;                                  
float soil_moisture = 0;                                 
int moist_value = 0;

unsigned long request_timer = (5*1000);          
int interval_counter = 1;                   
unsigned long time_counter;                  

const int TEMP_REG_PIN = 14;                
const int HUMI_REG_PIN = 12;                   
const int MOIST_REG_PIN = 15;    

String server_response = "";
float regulator_temeprature;
float regulator_humidity;
float regulator_moisture;

//-----------------------------------------------------------------------------------------------------------------------

void setup(){
  
  pinMode(LED_BUILTIN, OUTPUT);         
  pinMode(TEMP_REG_PIN, OUTPUT);
  pinMode(HUMI_REG_PIN, OUTPUT);
  pinMode(MOIST_REG_PIN, OUTPUT);
  
  dht.begin();
  delay(50);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while(WiFi.status() != WL_CONNECTED){
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
  }  
}

//-----------------------------------------------------------------------------------------------------------------------

void loop(){
  
  dht_humidity = dht.readHumidity();
  dht_temperature = dht.readTemperature();
  moist_value = analogRead(0);
  soil_moisture = constrain(map (moist_value, 1024, 880, 0, 100  ), 0, 100);

  time_counter = millis();
  if(time_counter > request_timer * interval_counter){
    interval_counter += 1; 
          
    client.connect(host_str, port); 
      
    String request = String(route + "?client_id=" + String(id) + "&temperature=" + String(dht_temperature) + "&humidity=" + String(dht_humidity) + "&soil_moisture=" + String(soil_moisture));
    client.print(String("GET " + request + " HTTP/1.1\r\n" + "Host: " + host_str + "\r\n" + "Connection: close\r\n\r\n"));
  
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        client.stop();
        return;
      }
    }
    
    while (client.connected()){
      if (client.available()){
       
       server_response = client.readStringUntil('#');
       
       int beginning = server_response.indexOf('?');
       int com1 = server_response.indexOf(',');
       regulator_temeprature = server_response.substring(beginning + 1, com1).toFloat();
        
       int com2 = server_response.indexOf(',', com1 + 1);
       regulator_humidity = server_response.substring(com1 + 1, com2).toFloat();
        
       int ending = server_response.indexOf('#');
       regulator_moisture = server_response.substring(com2 + 1, ending).toFloat();

      }
    }
    client.stop(); 
  }
  
  if (dht_temperature < regulator_temeprature){
    digitalWrite(TEMP_REG_PIN, HIGH);       
  }
  else if (dht_temperature >= regulator_temeprature){
    digitalWrite(TEMP_REG_PIN, LOW); 
  }
  if (dht_humidity < regulator_humidity){
    digitalWrite(HUMI_REG_PIN, HIGH);       
  }
  else if (dht_humidity >= regulator_humidity){
    digitalWrite(HUMI_REG_PIN, LOW);
  }
  if (soil_moisture < regulator_moisture){
    digitalWrite(MOIST_REG_PIN, HIGH);  
  }
  else if (soil_moisture >= regulator_moisture){
    digitalWrite(MOIST_REG_PIN, LOW);
  }
}
