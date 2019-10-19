#include <ESP8266WiFi.h> 
#include "DHT.h"

#define DHT_PIN 13
#define DHTTYPE DHT21
DHT dht(DHT_PIN, DHTTYPE);

#define AP_SSID "Zavrsni_Rad"
#define AP_PASS "12345678"
const char* ssid = AP_SSID;            
const char* password = AP_PASS;            

byte port = 80;                          
String host_str = "8.8.8.8";                   
String route = "/client2/";                   

WiFiClient client;                           
byte id = 2;                              

float dht_temp = 0;                                   
float dht_humi = 0;                                  
int soil_moist = 0;                                 
float moist_value = 0;

unsigned long req_timer = (1000);          
int interval_counter = 1;                   
unsigned long time_counter;                  

const int REG_TEMP = 14;                
const int REG_HUMI = 12;                   
const int REG_MOIST = 15;    

String server_line = "";
float reg_temp_val;
float reg_humi_val;
float reg_moist_val;

//-----------------------------------------------------------------------------------------------------------------------

void setup(){
  
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);         
  pinMode(REG_TEMP, OUTPUT);
  pinMode(REG_HUMI, OUTPUT);
  pinMode(REG_MOIST, OUTPUT);
  Serial.println("dobar");
  dht.begin();
  delay(500);
  
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
  //dht_humi = dht.readHumidity();
  //dht_temp = dht.readTemperature();
  //moist_value = analogRead(0);

  dht_humi = random(4,99);
  dht_temp = random(2,44);
  soil_moist = random(22,55);
  
  //soil_moist = constrain(map (moist_value, 1023, 700, 0, 100), 0, 100);
    
  time_counter = millis();
  if(time_counter > (req_timer * interval_counter)){
    interval_counter += 1; 
          
    client.connect(host_str, port); 
      
    String request = String(route + "?client_id=" + String(id) + "&temperature=" + String(dht_temp) + "&humidity=" + String(dht_humi) + "&moist=" + String(soil_moist));
    client.print(String("GET " + request + " HTTP/1.1\r\n" + "Host: " + host_str + "\r\n" + "Connection: close\r\n\r\n"));

    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 6000) {
        client.stop();
        return;
      }
    }
    
    while (client.connected()){
      if (client.available()){
       
       server_line = client.readStringUntil('#');
       
       int begining = server_line.indexOf('?');
       int com1 = server_line.indexOf(',');
       reg_temp_val = server_line.substring(begining + 1, com1).toFloat();
        
       int com2 = server_line.indexOf(',', com1 + 1);
       reg_humi_val = server_line.substring(com1 + 1, com2).toFloat();
        
       int ending = server_line.indexOf('#');
       reg_moist_val = server_line.substring(com2 + 1, ending).toFloat();
        
       Serial.println(reg_temp_val);
       Serial.println(reg_humi_val);
       Serial.println(reg_moist_val);
      }
    }
    client.stop(); 
  }
  
  if (dht_temp < reg_temp_val){
    digitalWrite(REG_TEMP, HIGH); 
    Serial.println("TEMP");      
  }
  else if (dht_temp >= reg_temp_val){
    digitalWrite(REG_TEMP, LOW);
    Serial.println("temp");
  }

  if (dht_humi < reg_humi_val){
    digitalWrite(REG_HUMI, HIGH); 
    Serial.println("HUMI");     
  }
  else if (dht_humi >= reg_humi_val){
    digitalWrite(REG_HUMI, LOW);
    Serial.println("humi");
  }
  
  if (soil_moist < reg_moist_val){
    digitalWrite(REG_MOIST, HIGH);  
    Serial.println("MOIST");
  }
  else if (soil_moist >= reg_moist_val){
    digitalWrite(REG_MOIST, LOW);
    Serial.println("moist");
  }
}
