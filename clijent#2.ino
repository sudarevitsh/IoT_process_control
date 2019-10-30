#include <ESP8266WiFi.h>                                                               //ESP 8266 WiFi biblioteka
#include <DHT.h>                                                                       //biblioteka senzora temperature i vlaž.vazduha

#define DHT_PIN 13                                                                     //definisanje pina DHT senzora
#define DHT_TYPE DHT21                                                                 //vrsta DHT senzora
DHT dht(DHT_PIN, DHT_TYPE);                                                            //inicijalizacija DHT objekta

//parametri pristupne tačke na koju se klijent povezuje
const char* ssid = "Zavrsni_Rad";                                                      //naziv pristupne tačke na koju se klijent povezuje    
const char* password = "12345678";                                                     //lozinka pristupne tačke

//parametri servera na koji se šalju zahtjevi
byte port = 80;                                                                        //port preko kog se pristupa serveru     
String host_str = "8.8.8.8";                                                           //IP adresa servera
String route = "/client2/";                                                            //rezervisana ruta za klijenta 2

//inicijalizacija klijenta
WiFiClient client;                                                                     //inicijalizacija wifi klijenta 
byte id = 2;                                                                           //ID broj klijenta

//vrijednosti mjerenih veličina
float dht_temperature = 0;                                                             //vrijednost temperature
float dht_humidity = 0;                                                                //vrijednost vlažnosti vazduha
float soil_moisture = 0;                                                               //vrijednost vlažnosti zemljišta, nakon pretvaranja
int moist_value = 0;                                                                   //očitavanje senzora za vlažnost zemljišta

//promjenljive kojima se kontroliše vrijeme između dva poslata zahtjeva
unsigned long request_timer = (5*1000);                                                //vremenski interval nakon kog se šalje novi zahtjev (5 sek.)
int interval_counter = 1;                                                              //brojač vremenskih intervala (broj zahtjeva umanjen za 1)
unsigned long time_counter;                                                            //brojač vremena od početnog trenutka

//brojevi pinova sa kojih se šalju upravljački signali na regulatore
const int TEMP_REG_PIN = 14;                                                           //pin regulatora temperature
const int HUMI_REG_PIN = 12;                                                           //pin regulatora vlažnosti vazduha
const int MOIST_REG_PIN = 15;                                                          //pin regulatora vlažnosti vazduha

//vrijednosti primljene od servera, primljenim vrijednostima se reguliše samo donja granica željenih veličina
String server_response = "";                                                           //String objekat serverovog odgovora
float regulator_temeprature;                                                           //vrijednost na kojoj se reguliše temperatura
float regulator_humidity;                                                              //vrijednost na kojoj se reguliše vlažnost vazduha
float regulator_moisture;                                                              //vrijednost na kojoj se reguliše vlažnost zemlje

//--------------------------------------------------------------------------------------------------------------------------------------------

//osnovni dio programa koji se izvršava samo jednom
void setup(){
  
  //podešavanje izlaza, tj. postavljanje pinova u izlazni mod
  pinMode(LED_BUILTIN, OUTPUT);         
  pinMode(TEMP_REG_PIN, OUTPUT);
  pinMode(HUMI_REG_PIN, OUTPUT);
  pinMode(MOIST_REG_PIN, OUTPUT);
  
  dht.begin();                                                                         //pokretanje DHT senzora (objekta)
  delay(50);                                                                           //kratki zastoj zboh pokretanja senzora
  
  //podešavanje i pokretanje wifi objekta
  WiFi.mode(WIFI_STA);                                                                 //postavljanje u mod stanice
  WiFi.begin(ssid, password);                                                          //početak konekcije sa pristupnom tačkom
  
  //signalizacija da povezivanje i dalje traje...
  while(WiFi.status() != WL_CONNECTED){
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
  }  
}

//--------------------------------------------------------------------------------------------------------------------------------------------

//osnovni dio programa koji se stalno ponavlja
void loop(){
  
  dht_humidity = dht.readHumidity();                                                   //očitavanje vrijednosti temperature sa DHT senzora
  dht_temperature = dht.readTemperature();                                             //očitavanje vrijednosti vlaž.vazduha sa DHT senzora
  moist_value = analogRead(0);                                                         //očitavanje vrijednosti sa senzora vlažnosti zemlje
  soil_moisture = constrain(map (moist_value, 1024, 880, 0, 100  ), 0, 100);           //mapiranje i ograničavanje vrijednosti vlažnosti zemlje

  //slanje zahtjeva na server, ako je protekao određen vremenski period, kako je "millis()" funkcija koja daje vrijednost od početnog trenutka...
  //koristi se promjenljiva "interval_counter" kojom se množi željeni vremenski period da bi se postigao željeni vremenski interval u toku rada
  time_counter = millis();
  if(time_counter > request_timer * interval_counter){
    interval_counter += 1; 
          
    client.connect(host_str, port);                                                    //povezivanje klijenta sa serverom
      
    //formiranje zahtjeva sa ID brojem kako bi server mogao da zna o kom klijentu se radi i očitavanjima sa senzora...
    //na ovaj način nema potrebe za POST zahtjevom jer su vrijednosti svakako skrivene od korisnika
    String request = String(route + "?client_id=" + String(id) + "&temperature=" + String(dht_temperature) + "&humidity=" + String(dht_humidity) + "&soil_moisture=" + String(soil_moisture));
    
    //slanje zahtjeva na server
    client.print(String("GET " + request + " HTTP/1.1\r\n" + "Host: " + host_str + "\r\n" + "Connection: close\r\n\r\n"));
  
    //čekanje odgovora sa servera, ako čekanje traje duže od 5 sekundi, konekcija se prekida
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        client.stop();
        return;
      }
    }
    
    //čitanje odgovora sa servera, a zatim potraga za željenim dijelovima tog odgovora
    while (client.connected()){
      if (client.available()){
       
       server_response = client.readStringUntil('#');                                  //čitanje zahtjeva sve do znaka '#'
       
       int beginning = server_response.indexOf('@');                                   //indeks početnog znaka '@'
       int com1 = server_response.indexOf(',');                                        //indeks prvog veznika ','
       
       //izvlačenje vrijednosti na kojoj se reguliše temperatura iz odgovora servera
       regulator_temeprature = server_response.substring(beginning + 1, com1).toFloat();
        
       int com2 = server_response.indexOf(',', com1 + 1);                              //indeks drugog veznika ','
       
       //izvlačenje vrijednosti na kojoj se reguliše vlažnost vazduha iz odgovora servera 
       regulator_humidity = server_response.substring(com1 + 1, com2).toFloat();       
        
       int ending = server_response.indexOf('#');                                      //indeks završnog znaka '#'
       
       //izvlačenje vrijednosti na kojoj se reguliše vlažnost zemljišta iz odgovora servera
       regulator_moisture = server_response.substring(com2 + 1, ending).toFloat();
      }
    }
    client.stop();                                                                     //prekid veze sa serverom
  }
  
  //regulisanje mjerenih veličina na osnovu vrijednosti koje je korisnik unio
  
  //uključenje regulatora temperature ako je izmjerena vrijednost manja od unesene
  if (dht_temperature < regulator_temeprature){
    digitalWrite(TEMP_REG_PIN, HIGH);                                                  
  }
  
  //isključenje regulatora temperature ako ne vrijedi prethodno
  else if (dht_temperature >= regulator_temeprature){
    digitalWrite(TEMP_REG_PIN, LOW); 
  }
  
  //uključenje regulatora vlažnosti vazduha ako je izmjerena vrijednost manja od unesene
  if (dht_humidity < regulator_humidity){
    digitalWrite(HUMI_REG_PIN, HIGH);       
  }
  
  //isključenje regulatora vlažnosti vazduha ako ne vrijedi prethodno
  else if (dht_humidity >= regulator_humidity){
    digitalWrite(HUMI_REG_PIN, LOW);
  }
  
  //uključenje regulatora vlažnosti zemljišta ako je izmjerena vrijednost manja od unesene
  if (soil_moisture < regulator_moisture){
    digitalWrite(MOIST_REG_PIN, HIGH);  
  }
  
  //isključenje regulatora vlažnosti zemljišta ako ne vrijedi prethodno
  else if (soil_moisture >= regulator_moisture){
    digitalWrite(MOIST_REG_PIN, LOW);
  }
}
