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
unsigned int ch_no = 0;
int spec_count = 0;
int opps = 0;
int counter = 0;
boolean client_free = true;
int job_len;



//-----------------------------------------------------------------------------------------------------------------------

void algorithm(){  
  
  for(int part_count = 1; part_count <= parts; part_count++){
    job_len = job.length();
    for (ch_no = 0; ch_no < job_len ; ch_no ++){
      char com = job.charAt(ch_no);
      if ( com == '?' || com == ',' || com == '#'){
        spec_count += 1;
      }
  }

  int char_place[spec_count];
  memset(char_place, 0, sizeof(char_place));
  int opps = 0; 
  
  
  for (int ch_no = 0; ch_no < job_len; ch_no ++){
    char com = job.charAt(ch_no);
    if (com == '?' || com == ',' || com == '#'){
      char_place[opps] = ch_no;  
      opps += 1;  
    }

  } 
  Serial.println("prebrojavanje i indeksi su dobri!");
  for(counter = 0; counter < spec_count - 1; counter ++){
    Serial.print("Counter je: ");Serial.println(counter);
    Serial.print("poredmimo sa: "); Serial.println(spec_count);
    String delay_timer = "";
    for(ch_no = char_place[counter] + 1; ch_no < char_place[counter + 1]; ch_no ++){
      char com = job.charAt(ch_no);
      Serial.print("timer: ");Serial.println(delay_timer);
      Serial.print("znak ");Serial.println(com);
      Serial.print("broj je: ");
      Serial.println(isnan(String(job.charAt(ch_no)).toInt()));
      if (com == 'A' || com == 'B' || com == 'C' || com == 'D' || com == 'E' || com == 'F'){
        if(job.charAt(ch_no + 1) == '+'){
          Serial.println("HIGH"); 
          digitalWrite(job.charAt(ch_no), HIGH);}
        else if(job.charAt(ch_no + 1) == '-'){
          Serial.println("LOW");
          digitalWrite(job.charAt(ch_no), LOW);}
      }
      else if (com == '1' || com == '2' ||  com == '3' || com == '4' || com == '5' || com == '6' || com == '7' || com == '8' || com == '9' || com == '0'){
        delay_timer += String(com);
      }
           
    }
    Serial.print("Vremenski period je:");Serial.println(delay_timer.toInt());
    delay(delay_timer.toInt());
  }
  spec_count = 0;
 }
 Serial.print("Dijelova napravljeno");Serial.println(parts);
 Serial.println("Cekanje serveraaaaa");
 delay(3000);  
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
    
    while (client.connected()){
      if (client.available()){
       
        String line = client.readStringUntil('#');
        int beginning = line.indexOf('?');
        int mid = line.indexOf('x');
        int ending = line.indexOf('#');
       
        parts = line.substring(beginning + 1, mid).toInt();
        job = line.substring(mid + 1 , ending);
        client_free = false;
        Serial.println(line);
        Serial.println(job);
        Serial.println(parts);
      }
    }
  }
  while (!client_free){
    algorithm();
  }
  client_free = true;  
}
