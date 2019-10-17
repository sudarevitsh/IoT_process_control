#include <ESP8266WiFi.h>

const char* ssid = "Zavrsni_Rad";       
const char* password = "12345678";      

const byte port = 80;                 
String host_str = "8.8.8.8";            
String route = "/client1/";             
 
WiFiClient client;                     
byte id = 1;                      

boolean client_free = true;

//-----------------------------------------------------------------------------------------------------------------------

void setup(){
Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT); 
  
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
     
  if(client_free){
    client.connect(host_str, port);
      
    String request = String(route + "?client_id=" + String(id) + "&client_free=" + String(client_free));
    client.print(String("GET " + request + " HTTP/1.1\r\n" + "Host: " + host_str + "\r\n" + "Connection: keep-alive\r\n\r\n"));
    delay(5); 
      
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        client.stop();
        return;
      }
    }
    
    while (client.connected() || client.available()){
      if (client.available()){
       
       String line = client.readStringUntil('\n');
       Serial.println(line);
      }
    }
      client_free = false; 
      client_free = true;                 
  }
}
