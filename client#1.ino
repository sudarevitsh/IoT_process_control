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
unsigned int char_number = 0;
int break_count = 0;
int operations = 0;
int counter = 0;
int job_lengthgth;
String delay_timer = "";

//-----------------------------------------------------------------------------------------------------------------------

int char_to_pin(char led_char){
  int led_pin;
  switch (led_char){
    case 'A':
    led_pin = 14;
    break;
    case 'B':
    led_pin = 12;
    break;
    case 'C':
    led_pin = 13;
    break;
    case 'D':
    led_pin = 15;
    break;
    case 'E':
    led_pin = 3;
    break;
    case 'F':
    led_pin = 2;
    break;
  }
  return led_pin;
}

//-----------------------------------------------------------------------------------------------------------------------

void pin_reset(){
  digitalWrite(14, LOW);
  digitalWrite(12, LOW);
  digitalWrite(13, LOW);
  digitalWrite(15, LOW);
  digitalWrite(3, LOW);
  digitalWrite(2, LOW);
}

void algorithm(){  
  for(int part_count = 1; part_count <= parts; part_count++){
    job_lengthgth = job.length();
    for (char_number = 0; char_number < job_lengthgth ; char_number ++){
      char com = job.charAt(char_number);
      if ( com == '?' || com == ',' || com == '#'){
        break_count += 1;
      }
  }

  int char_place[break_count];
  memset(char_place, 0, sizeof(char_place));
  int operations = 0; 
  
  
  for (int char_number = 0; char_number < job_lengthgth; char_number ++){
    char com = job.charAt(char_number);
    if (com == '?' || com == ',' || com == '#'){
      char_place[operations] = char_number;  
      operations += 1;  
    }

  } 
  Serial.println("prebrojavanje i indeksi su dobri!");
  for(counter = 0; counter < break_count - 1; counter ++){
    Serial.print("Counter je: ");Serial.println(counter);
    delay_timer = "";
    for(char_number = char_place[counter] + 1; char_number < char_place[counter + 1]; char_number ++){
      char com = job.charAt(char_number);
      if (com == 'A' || com == 'B' || com == 'C' || com == 'D' || com == 'E' || com == 'F'){
        if(job.charAt(char_number + 1) == '+'){
          digitalWrite(char_to_pin(com), HIGH);          
        }
        else if(job.charAt(char_number + 1) == '-'){
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
  break_count = 0;
  pin_reset();
  Serial.print("Dijelova napravljeno");Serial.println(part_count);
  
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
  pinMode(2, OUTPUT);

  pin_reset();
   
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
   
  while(WiFi.status() != WL_CONNECTED){
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
  }
  Serial.println("Connected!");
}

//-----------------------------------------------------------------------------------------------------------------------

void loop(){
   
  Serial.println("Pinovi resetovani");
  delay(5);
  if(client_free){
    client.connect(host_str, port);
      
    String request = String(route + "?client_id=" + String(id) + "&client_free=" + String(client_free));
    client.print(String("GET " + request + " HTTP/1.1\r\n" + "Host: " + host_str + "\r\n" + "Connection: keep-alive\r\n\r\n"));   
    Serial.println("Send!");
      
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
        Serial.println(line);
        
        unsigned int beginning = line.indexOf('@');
        Serial.print("pocetni indeks ");Serial.println(beginning);
        unsigned int mid = line.indexOf('x', beginning);
        Serial.print("srednji indeks ");Serial.println(mid);
        unsigned int ending = line.indexOf('#', beginning );
        Serial.print("krajnji indeks ");Serial.println(ending);
        delay(30);
        
        parts = line.substring(beginning + 1, mid).toInt();
        job = line.substring(mid + 1 , ending + 1);
        
        client.flush();
        
        Serial.print("Zadatak je ovaj:");Serial.println(job);
        Serial.print("Dijelova treba:");Serial.println(parts);
        delay(30);          
      }
    }
    client_free = false;
  }
  
  while (!client_free){
      algorithm();
      Serial.println("Algoritam gotov!");
      client_free = true;
      Serial.println("Trazi se novi posao, pin reset!");
  } 
}
