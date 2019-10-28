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
int job_len;
String delay_timer = "";

//-----------------------------------------------------------------------------------------------------------------------

int char_to_pin(char led_char){
  int led_pin;
  switch (led_char){
    case 'A':
    led_pin = 14;
    case 'B':
    led_pin = 12;
    case 'C':
    led_pin = 13;
    case 'D':
    led_pin = 15;
    case 'E':
    led_pin = 3;
    case 'F':
    led_pin = 1;
  }
  return led_pin;
}

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
    delay_timer = "";
    for(ch_no = char_place[counter] + 1; ch_no < char_place[counter + 1]; ch_no ++){
      char com = job.charAt(ch_no);
      Serial.print("timer: ");Serial.println(delay_timer);
      Serial.print("znak ");Serial.println(com);
      if (com == 'A' || com == 'B' || com == 'C' || com == 'D' || com == 'E' || com == 'F'){
        if(job.charAt(ch_no + 1) == '+'){
          Serial.println("HIGH"); 
          digitalWrite(char_to_pin(com), HIGH);
        }
        else if(job.charAt(ch_no + 1) == '-'){
          Serial.println("LOW");
          digitalWrite(char_to_pin(com), LOW);
        }
      }
      else if (com == '1' || com == '2' ||  com == '3' || com == '4' || com == '5' || com == '6' || com == '7' || com == '8' || com == '9' || com == '0'){
        delay_timer += String(com);
      }
           
    }
    Serial.print("Vremenski period je:");Serial.println(delay_timer.toInt());
    delay(delay_timer.toInt());
  }
  spec_count = 0;
  Serial.print("Dijelova napravljeno");Serial.println(parts);
  Serial.println("Cekanje serveraaaaa"); 
  }
}
  
 
void setup(){
Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(15, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(1, OUTPUT);
  
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
      if (millis() - timeout > 5000) {
        client.stop();
        return;
      }
    }
    
    while (client.connected()){
      if (client.available()){
       
        String line = client.readStringUntil('$');
        Serial.print("OD SERVERA DOSLO: ");
        Serial.println(line);
        delay(2000);
        unsigned int beginning = line.indexOf('@');
        Serial.print("pocetni indeks ");Serial.println(beginning);
        unsigned int mid = line.indexOf('x', beginning);
        Serial.print("srednji indeks ");Serial.println(mid);
        unsigned int ending = line.indexOf('#', beginning );
        Serial.print("krajnji indeks ");Serial.println(ending);
        delay(3000);
        
        parts = line.substring(beginning + 1, mid).toInt();
        job = line.substring(mid + 1 , ending + 1);
        client.flush();
        //Serial.println(line);
        Serial.print("Zadatak je ovaj:");Serial.println(job);
        Serial.print("Dijelova treba:");Serial.println(parts);
        delay(3000);          
      }
    }
    client_free = false;
  }
  while (!client_free){
      algorithm();
      client_free = true;
  }
   
}
