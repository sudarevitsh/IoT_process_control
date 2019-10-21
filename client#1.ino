#include <ESP8266WiFi.h>

const char* ssid = "Zavrsni_Rad";       
const char* password = "12345678";      

const byte port = 80;                 
String host_str = "8.8.8.8";            
String route = "/client1/";             
 
WiFiClient client;                     
byte id = 1;                      

boolean client_free = true;
String job = "";
int parts;




//-----------------------------------------------------------------------------------------------------------------------

void algorithm(){
  unsigned int ch_no;
  int opps = 0; 
  int spec_count = 0;
  int counter = 0;
  
  for (ch_no = 0; ch_no < (job.length() - 1); ch_no ++){
    if (job.charAt(ch_no) == '?' || ',' || '#'){
      spec_count += 1;
    }  
  }
  int char_place[spec_count];
  for (ch_no = 0; ch_no < (job.length() - 1); ch_no ++){
    if (job.charAt(ch_no) == '?' || ',' || '#'){
      char_place[opps] = job.indexOf(ch_no); 
      opps += 1;
    }
  } 
  for(counter = 0; counter <= spec_count; counter ++){
    for(ch_no = char_place[counter] + 1; ch_no < char_place[counter + 1]; ch_no ++){
      String delay_timer = "";
      if (job.charAt(ch_no) == 'A' || 'B' || 'C' || 'D' || 'E' || 'F'){
        if(job.charAt(ch_no + 1) == '+'){
          digitalWrite(job.charAt(ch_no), HIGH);}
        else if(job.charAt(ch_no + 1) == '-'){
          digitalWrite(job.charAt(ch_no), LOW);}
      }
      else if (isnan(String(job.charAt(ch_no)).toInt())){
        delay_timer += String(job.charAt(ch_no));
      }
      delay(delay_timer.toInt());
    }
  }
}
 
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
     
      
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 4000) {
        client.stop();
        return;
      }
    }
    
    while (client.connected() || client.available()){
      if (client.available()){
       
        String line = client.readStringUntil('#');
        int beginning = line.indexOf('?');
        int comma = line.indexOf(',');
        int ending = line.indexOf('#');
       
        parts = line.substring(beginning + 1, comma).toInt();
        job = line.substring(comma + 1 , ending);
        client_free = false;
        Serial.println(line);
        Serial.println(job);
        Serial.println(parts);
      }
    }
  }
  for(int i = 1; i <= parts; i++){
    algorithm();
  }
}
