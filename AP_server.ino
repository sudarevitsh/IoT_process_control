#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

//definisanje parametara pristupne tačke (AP)
  #define AP_SSID "Zavrsni_Rad"                               //naziv mreže 
  #define AP_PASS "12345678"                                  //lozinka mreže

//dodjeljivanje vrijednosti parametara promjenljivima
  const char* ssid = AP_SSID;                                 //ukazivanje na naziv mreže
  const char* password = AP_PASS;                             //ukazivanje na lozinku

//definisanje IP adrese (koja je i gateway) i maske pristupne tačke
  IPAddress ap_ip (8, 8, 8, 8);                               //IP adresa pristupne tačke
  IPAddress subnet_mask (255, 255, 255, 0);                   //podmrežna maska 

//inicijalizacija webservera i porta za komunikaciju
  ESP8266WebServer server(80);

//vrijednosti veličina koje mjeri i šalje klijent#2
  float temperature = 0;                                      //promjenljiva temperature koju pošalje klijent
  float humidity = 0;                                         //promjenljiva vlažnosti vazduha koju pošalje klijent
  float soil_moist = 0;                                       //promjenljiva vlažnosti zemljišta koju pošalje klijent  
  
  float reg_temp = 0;
  float reg_humi = 0;
  float reg_moist = 0;

  String process = "";
  String x = "";

  
//-----------------------------------------------------------------------------------------------------------------------

//funkcija u kojoj je definisan izgled web stranice koja se prikazuje prilikom pristupa serveru
//funkcija za parametre uzima vrijednosti primljene sa klijenta#2

  String webpage(float TEMPERATURE, float HUMIDITY, float SOIL_MOIST, float REG_TEMP, float REG_HUMI, float REG_MOIST){
  String html = R"=====(
  <!DOCTYPE html><html>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <head>
    <title>Zavrsni projekat</title>
    <style>
      body {background-color: #D8D8D8 ; text-align: center; color: black; font-family: Arial, serif;}
    
      .input_class{width: 70%; height: 30px; padding 10px 10px; margin: 5px 5px; box-sizing: border-box;
      border: 2px; border-color: black; border-radius: 5px; background-color: white; color: #854D41; text-align: center;}
          
      .button_class{width: 35%; height:35px; padding 10px 10px; margin: 5px 5px; box-sizing: border-box;
      border: 2px #FFFFFF; border-radius: 5px; background-color: white; color: #854D41;}
      
      .field_class{border-color: black;}
    </style>
    </head>
  
    <body>
      <h2>ZAVRSNI RAD</h2>
      <h3>Klijent #1</h3>
      <p>Klijent koji upravlja sa 6 LE dioda. LED se koriste kako bi se samo simulirao rad nekog aktuatora. Umjesto njih mogu biti releji koji upravljaju sa nekim drugim aktuatorima ili slicno.</p>
      <p>Način unosenja zadataka nekoj od lampica:</p>
      <p>A+0,A-15,A+25,A-30...</p>
      <p>Na ovaj način bi se lampica A upalila u početnom trenutku, a nakon 15 sekundi ugasila, upalila na 25. sekundi, pa ugasila na 30. </p>
  <fieldset class="field_class">
  <legend>Unesi novi proces</legend>
  <form action="/input" method="GET">
      <p>Proces:<input class="input_class" name="process"></input></p>
      
      <p>Broj ponavljanja:<input class="input_class" name="x" style=width:15%;></input></p>
      <p><button class="button_class" value="Submit">Posalji proces na server</button></p>
  </form>
  </fieldset>
      <h3>Klijent #2</h3>
      <p>Klijent koji upravlja sa mikroklimom nekog objekta i prati vrijednost odredjenih velicina.</p>
      <p>Mikrokontroler prati vrijednosti ovih velicina i na osnovu njih pokrece sisteme za njihovu regulaciju.</p>
      <p>Sistemi za regulaciju takodje ce biti simulirani LE diodama.</p>
      )=====";
    
      html +="<fieldset class=\"field_class\"><legend>Izmjerene vrijednosti</legend><form action=\"/regulation\" method=\"GET\">\n";
      html += "<h4>Temperatura:";
      html += (float)TEMPERATURE;
      html += "C</h4><h5>Podesavanje regulatora temperature:";
      html += (float)REG_TEMP;
      html += "C /<input class=\"input_class\" style=width:10%; name=\"reg_temp\"></h5>\n";
      html += "<h4>Vlaznost vazduha:";
      html += (float)HUMIDITY;
      html += "%</h4><h5>Podesavanje regulatora vlaznosti vazduha:";
      html += (float)REG_HUMI;
      html += "% /<input class=\"input_class\" style=width:10%; name= \"reg_humi\"></h5>\n";
      html += "<h4>Vlaznost zemljista:";
      html += (float)SOIL_MOIST;
      html += "%</h4><h5>Podesavanje regulatora vlaznosti zemljista:";
      html += (float)REG_MOIST;
      html += "% /<input class=\"input_class\" style=width:10%; name=\"reg_moist\"></h5>\n";
      html += "<p><button class=\"button_class\" value=\"Submit\">Izmjeni vrijednosti</button></p>";
  
      html += "</form></fieldset></body>\n";
      html += "</html>\n";

    return html;
}

//-----------------------------------------------------------------------------------------------------------------------

//obrada i odgovor na zahtjev klijenta#1
  String new_job;
//provjera ID-a i zauzetosti klijenta, ako je slobodan dobiće novi posao
  void handleClient1(){
    if(server.arg("client_id") == "1" && server.arg("client_free" == "1")){
      server.send(200, "text/plain", "new_job");                //odgovor servera na zahtjev klijenta sa novim poslom
    }
  }  

//obrada i odgovor na zahtjev klijenta#2
  void handleClient2(){
    if (server.arg("client_id") == "2"){                      //provjera ID-a klijenta
      temperature = server.arg("temperature").toFloat();      //očitavanje vrijednosti temperature iz zahtjeva
      humidity = server.arg("humidity").toFloat();            //očitavanje vrijednosti vlažnosti vazduha iz zahtjeva
      soil_moist = server.arg("soil_moist").toFloat();        //očitavanje vrijednosti vlažnosti zemljišta iz zahtjeva
      server.send(200, "text/plain", " :) ");                 //odgovor servera na zahtjev klijenta (200 = OK)
    }
  }

//odgovor kada se zatraži samo IP adresa servera (web stranica)
  void handleRoot(){
    server.send(200, "text/html", webpage(temperature, humidity, soil_moist, reg_temp, reg_humi, reg_moist)); //odgovor servera web stranicom
  }

//kada se zatraži neka nepoznara ruta na serveru
  void handleNotFound(){
    server.send(404, "text/plain", "Nije pronadjeno");
  }
 
  void handleInput(){
    process = server.arg("process");
    x = server.arg("x");
    server.send(200, "text/plain", "Unešeni proces je dodan u listu poslova");
  }

  void handleRegulation(){
    reg_temp = server.arg("reg_temp").toFloat();
    reg_humi = server.arg("reg_humi").toFloat();
    reg_moist = server.arg("reg_moist").toFloat();
    server.send(200, "text/plain", "Vrijednosti promjenjene!");
  }
//-----------------------------------------------------------------------------------------------------------------------

void setup(){
//pkretanje i podešavanje pristupne tačke  
  WiFi.softAP(ssid, password);                                //ime i lozinka pristupne tačke
  WiFi.softAPConfig(ap_ip, ap_ip, subnet_mask);               //IP, gateway, subnet maks
  
//definisanje ruta koje server prepoznaje i vrsta zahtjeva koje obrađuje  
  server.on("/client1/", HTTP_GET, handleClient1);          //klijent#1
  server.on("/client2/", HTTP_GET, handleClient2);          //klijent#2
  server.on("/input", HTTP_GET, handleInput);
  server.on("/regulation", HTTP_GET, handleRegulation);
  server.on("/", handleRoot);                                 //bilo koji klijent koji otvori IP adresu
  
  server.begin();                                             //pokretanje servera
}

//-----------------------------------------------------------------------------------------------------------------------

void loop(){
  server.handleClient();                                      //održavanje servera
}