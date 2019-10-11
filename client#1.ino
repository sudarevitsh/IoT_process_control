#include <ESP8266WiFi.h>


//parametri pristupne tačke
  const char ssid = "Zavrsni_Rad";       //naziv mreže
  const char password = "12345678";      //lozinka

//parametri webservera
  const byte port = 80;               //broj porta      
  IPAddress host (8,8,8,8);               //ip adresa webservera
  String host_str = "8.8.8.8";            //ista adresa samo u obliku znakovnog niza
  String route = "/client1/";             //ruta po kojoj klijent ostvaruje komunikaciju sa serverom
 
//definisanje wifi klijenta 
  WiFiClient client;                      //inicijalizacija klijenta
  String byte = 1;                        //identifikacioni broj koji server zahtjeva od klijenta za komunikaciju

//aktuatorski niz sa brojem pinova kojima se aktuatorima šalju upravljački signali
  #define ACT_NUMBER 6                    //dužina aktuatorskog niza
  int act[ACT_NUMBER] = {1, 3, 15, 13, 12, 14};

//-----------------------------------------------------------------------------------------------------------------------

void setup(){
Serial.begin(115200);

//definisanje izlaza
    pinMode(LED_BUILTIN, OUTPUT);
    for (int i = 0; i < ACT_NUMBER; i++){
      pinMode(act[i], OUTPUT);
    }

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
}

//-----------------------------------------------------------------------------------------------------------------------

void loop(){
  
//definisanje stanja klijenta, da li je slobodan za novi posao ili ne
    boolean client_free = true;
    String new_job;
  
//povezivanje klijenta sa serverom, a zatim traženje novog posla za izvršavanje
    if(client_free == true){
      client.connect(host_str, port);       //povezivanje sa serverom i slanje zahtjeva za novi posao

  //formiranje i slanje HTTP zahtjeva na server    
      String request = String(route + "?client_id=" + String(id) + "client_free=" + String(client_free));
      client.print(String("GET " + request + " HTTP/1.1\r\n" + "Host: " + host_str + "\r\n" + "Connection: close\r\n\r\n"));
      delay(30);

  //čekanje odgovora od servera, ne više od 5 sekundi
      unsigned long timeout = millis();
      while (client.available() == 0) {
        if (millis() - timeout > 5000) {
          client.stop();
          return;
        }
      }

  //očitavanje odgovora sa servera, tj. novog posla za klijenta
      while (client.available()) {
        String new_job = client.readStringUntil('\r'); 
        client.stop();                      
      }
    
      client_free = false;                  //klijent je zauzet
  }
}
  
  //DO NEW JOB
  
  //WHEN DONE SET FREE = TRUE
  
  
