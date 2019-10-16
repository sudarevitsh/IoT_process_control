#include <ESP8266WiFi.h> 
#include "DHT.h"

#define DHT_PIN 13
#define DHTTYPE DHT21
  
DHT dht(DHT_PIN, DHTTYPE);

//parametri pristupne tačke
  const char* ssid = "Zavrsni_Rad";              //naziv mreže
  const char* password = "12345678";             //lozinka

//parametri webservera
  const byte port = 80;                          //broj porta
  String host_str = "8.8.8.8";                   //IP webservera 
  String route = "/client2/";                    //ruta po kojoj klijent ostvaruje komunikaciju sa serverom

//definisanje wifi klijenta.
  WiFiClient client;                             //inicijalizacija klijenta
  byte id = 2;                               //identifikacioni broj koji server zahtjva od klijenta za komunikaciju
  
//promjenljive u koje se unose rezultati senzorskih mjerenja
  float dht_temp = 0;                                    //temperatura
  float dht_humi = 0;                                    //vlažnost vazduha
  int soil_moist = 0;                                  //vlažnost zemljišta
  int moist_value = 0;

//promjenljive kojima se reguliše vremenski interval slanja zahtjeva na server
unsigned long req_timer = (1000);           //koliko često će se slati zahtjev i podaci.
  int interval_counter = 1;                      //brojač proteklih intervala, odgovara i broju poslanih zahtjeva plus 1
  unsigned long time_counter;                   //brojač vremena, ispisuje rezultat funkcije millis()

  const int REG_TEMP = 14;                //GPIO14 = D5, pin regulatora temperature
  const int REG_HUMI = 12;                   //GPIO12 = D6, pin regulatora vlažnosti vazduha
  const int REG_MOIST = 15;                  //GPIO13 = D7, pin regulatora vlažnosti zemljišta
  

//-----------------------------------------------------------------------------------------------------------------------

void setup(){
//definisanje izlaza
    pinMode(LED_BUILTIN, OUTPUT);         
    pinMode(REG_TEMP, OUTPUT);
    pinMode(REG_HUMI, OUTPUT);
    pinMode(REG_MOIST, OUTPUT);
    

    Serial.begin(115200);
    dht.begin();
    delay(500);
    Serial.println("DHTPOCO");
//podešavanje klijenta (stanice) i povezivanje na pristupnu tačku
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
  
//rutina blinkanja ugrađene lampice dok se klijent ne poveže sa serverom, prestaje nakon povezivanja  
    while(WiFi.status() != WL_CONNECTED){
      digitalWrite(LED_BUILTIN, LOW);
      delay(200);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(200);
    }  
    Serial.println("POVEZO");
}

//-----------------------------------------------------------------------------------------------------------------------

void loop(){

//praćenje vremena rada klijenta, na osnovu njega se određuje vremenski interval slanja zahtjeva na server
    

//dodjela vrijednosti mjerenim veličinama
    dht_humi = dht.readHumidity();
    dht_temp = dht.readTemperature();
    moist_value = analogRead(0);
    Serial.println(moist_value);
    soil_moist = constrain(map (moist_value, 1023, 700, 0, 100), 0, 100);
    
    Serial.println(soil_moist);
    Serial.println("OCITAO");
    delay(500);
    time_counter = millis();
//regulisanje vremenskog intervala zahtjeva, objasniti dodatno...
    if(time_counter > (req_timer * interval_counter)){
      interval_counter += 1; 
          
  //povezivanje klijenta sa serverom, a zatim ormiranje i slanje zahtjeva u kom se nalaze podaci očitavanja senzora 
      client.connect(host_str, port); 
    
  //način slanja podataka: ruta + id + temperatura + vlažnost vazduha + vlažnost zemljišta
      String request = String(route + "?client_id=" + String(id) + "&temperature=" + String(dht_temp) + "&humidity=" + String(dht_humi) + "&moist=" + String(soil_moist));
 
  //standardni način slanja HTTP zahtjeva na server  
      client.print(String("GET " + request + " HTTP/1.1\r\n" + "Host: " + host_str + "\r\n" + "Connection: close\r\n\r\n"));
          
      Serial.println(request);       
      Serial.println("POSLO");
      delay(500);
      client.stop(); 
   }
  
//pokretanje regulatora temperature na osnovu neke željene vrijednosti temperature  
    if (dht_temp < 5){
      digitalWrite(REG_TEMP, HIGH);       //slanje upravljačkog signala na definisani pin
    }
    else if (dht_temp >= 5){
      digitalWrite(REG_TEMP, LOW);
    }

//pokretanje regulatora vlažnosti vazduha na osnovu zadane vrijednosti vlažnosti vazduha
    if (dht_humi < 50){
      digitalWrite(REG_HUMI, HIGH);          //slanje upravljačkog signala na definisani pin
    }
    else if (dht_humi >= 50){
      digitalWrite(REG_HUMI, LOW);
    }
  
//pokretanje regulatora vlažnosti zemljišta na osnovu zadane vrijednosti vlažnosti zemljišta
    if (soil_moist < 40){
      digitalWrite(REG_MOIST, HIGH);          //slanje upravljačkog signala na definisani pin  
    }
    else if (soil_moist >= 40){
      digitalWrite(REG_MOIST, LOW);
    }
}
