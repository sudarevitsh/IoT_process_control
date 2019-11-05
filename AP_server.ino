#include <ESP8266WiFi.h>                                                               //ESP8266 WiFi biblioteka
#include <ESP8266WebServer.h>                                                          //ESP8266 Server biblioteka

//podešavanja pristupne tačke (Access Point) 
#define AP_SSID "Zavrsni_Rad"                                                          //SSID pristupne tačke
#define AP_PASS "12345678"                                                             //Lozinka pristupne tačke
const char* ssid = AP_SSID;                                                              
const char* password = AP_PASS;                             

IPAddress ap_ip (8, 8, 8, 8);                                                          //IP adresa pristupne tačke
IPAddress subnet_mask (255, 255, 255, 0);                                              //Maska podmreže

//podešavanje servera na pristupnoj tački
ESP8266WebServer server(80);                                                           //pokretanje servera, sa portom 80

//vrijednosti veličina koje mjeri klijent 2 se čuvaju u ovim promjenljivim
float temperature = 0;                                                                 //izmjerena temperatura
float humidity = 0;                                                                    //izmjerena vlažnost vazduha
float soil_moist = 0;                                                                  //izmjerena vlažnost zemljišta

//početne (default) vrijednosti parametara regulatora koje se šalju klijentu 2, korisnik može da ih izmjeni preko telefona
float reg_temp_bot = 20;                                                               //donja vrijednost na kojoj se reguliše temperatura
float reg_temp_top = 30;                                                               //gornja vrijednost na kojoj se reguliše temepratura
float reg_humi_bot = 40;                                                               //donja vrijednost na kojoj se reguliše vlaž.vazduha
float reg_humi_top = 70;                                                               //gornja vrijednost na kojoj se reguliše vlaž. vazduha
float reg_moisture = 30;                                                               //vrijednost na kojoj se reguliše vlažnost zemlje 

//nizovi u kojima se čuvaju uneseni poslovi i ponavljanja, prije nego što se pošalju ka klijentu 1 na izvršavanje
String process[5] = {"", "", "", "", ""};                                              //niz procesa koji se nalaze na serveru
String parts[5] = {"","","","",""};                                                    //niz broja ponavljanja procesa 

unsigned int process_number_in = 0;                                                    //indeks unesenog procesa
unsigned int process_number_out = 0;                                                   //indeks poslatog procesa

//----------------------------------------------------------------------------------------------------------------------------------

//web stranica definisana u obliku funkcije sa parametrima temperature, vlažnosti i vrijednosti na kojima se reguliše
//detaljnije objašnjena u radu
String webpage(float TEMPERATURE, float HUMIDITY, float SOIL_MOIST, float REG_TEMP_BOT, float REG_HUMI_BOT, float REG_MOIST, float REG_TEMP_TOP, float REG_HUMI_TOP){
  String html = R"=====(
    <!DOCTYPE html><html>
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <head>
      <title>Zavrsni projekat</title>
      <style>
        body {background-color: white ; text-align: left; color: black; font-family: candara; margin: 0px 20px; padding: 0px;}
        h1 {padding: 0px; margin: 0px;}
        
        .div_heading { margin: 100px 0px; font-family: rockwell;}
        
        .div_client { margin: 10px 0px; font-family: rockwell;}
        
        .div_text {font-family: candara; font-size: 14px;}
        
        .input_class { width:195px; height: 25px; padding 10px 10px; margin: 5px 5px; box-sizing: border-box; border: 1px solid #ccc; border-radius: 8px; background-color: white; color: #854D41; text-align: center; font-size: 12px;}
        
        .button_class { width: 250px; height:50px; padding 10px 10px; margin: 5px 5px; box-sizing: border-box; border: 2px #FFFFFF; border-radius: 8px; background-color: black; color: white; font-family: rockwell; font-size: 12px;}
        
        .field_class{ width: 265px; border-color: black; padding: 20px; text-align: center; margin-bottom: 30px; font-size: 14px; border-radius: 12px;}
      </style>
      </head>
      <body>
      
      <div class="div_heading">
        <h1>ZAVRSNI</h2>
        <h1>RAD</h2>
      </div>
      
      <hr>
      
      <div class="div_client">
        <h3>Klijent #1</h3>
      </div>
      
      <div class="div_text">
        <p>Klijent koji upravlja sa radom 6 LE dioda.<br>
        U polje se unosi nacin rada lampica i broj ponavljanja.</p>
        <p>Primjer:<br>A+, B+, C+3000, A-, D+2000, E+, F+3000, B-2000</p>
      </div>
     
    <div> 
    <fieldset class="field_class">
    <legend>Unos zadatka</legend>
    <form action="/input" method="GET">
        <p>Proces:<input class="input_class" name="process"></input></p>
        <p>Broj ponavljanja:<input class="input_class" name="x" style=width:140px; type="number" min="1" max="9"></input></p>
        <button class="button_class" value="Submit"><b>POSALJI PROCES</b></button>
    </form>
    </fieldset>
    </div>
    
    <hr>
    
    <div class="div_client">
      <h3>Klijent #2</h3>
    </div>    
    
    <div class="div_text">
        <p>Klijent koji prati i upravlja sa mikroklimom prostorije.</p>
        <p>U polja ispod se unose intervali unutar kojih se odrzavaju mjerene velicine.</p>
    </div>    

        <fieldset class="field_class"><legend>Stanje mikroklime</legend><form action="/regulation" method="GET">
        <h4>Temperatura:
        TEMPERATURE
        C</h4><h5>Podesavanje regulatora temperature:
        REG_TEMP
        C<input class="input_class" style=width:10%; name="reg_temp" type="number" min="0" max="50" step=".01"></h5>
        <h4>Vlaznost vazduha:
        HUMIDITY
        %</h4><h5>Podesavanje regulatora vlaznosti vazduha:
        REG_HUMI
        %<input class="input_class" style=width:10%; name= "reg_humi" type="number" min="0" max="100" step=".01"></h5>
        <h4>Vlaznost zemljista:
        SOIL_MOIST
        %</h4><h5>Podesavanje regulatora vlaznosti zemljista:
        REG_MOIST
        %<input class="input_class" style=width:10%; name="reg_moist" type="number" min="0" max="100" step=".01"></h5>
        <p><button class="button_class" value="Submit">Izmjeni vrijednosti</button></p>

        </form></fieldset></body>
        </html>)=====";

  return html;
}

//----------------------------------------------------------------------------------------------------------------------------------

//funkcija koja se pokreće prilikom zahtjeva klijenta 1
void handleClient1(){
    if(server.arg("client_id") == "1" && server.arg("client_free" == "1")){            //provjerava se ID i stanje klijenta
      process_number_out %= 5; 
      if(process[process_number_out] == "" ){                                          //slanje odgovora kada nema procesa/posla
        server.send(102, "text/plain", "Nema procesa na serveru");
        goto skip;                                                                     //izlazak iz petlje
      }
      else{
        if(parts[process_number_out] == ""){                                           //slanje odgovora kada nema broja komada
          server.send(102, "text/plain", "Proces nema ponavljanje");
          goto skip;
        }
        else{                                                                          //slanje procesa kada su svi uslovi ispunjeni
          String response_1 = ("@" + parts[process_number_out] + "x?" + process[process_number_out] + "#$"); 
          server.send(200, "text/plain", response_1);
        }
      }
      process[process_number_out] = "";                                                //brisanje procesa koji je upravo poslan 
      parts[process_number_out] = "";                                                  //brisanje broja komada poslanog procesa
      process_number_out += 1;                                                         //povećavanje indeksa za 1
      skip:;                                                                           //izlaz iz petlje
    }
}

//----------------------------------------------------------------------------------------------------------------------------------

//funkcija koja se pokreće prilikom zahtjeva klijenta 2
void handleClient2(){
    if (server.arg("client_id") == "2"){                                               //provjeranja klijenta
      temperature = server.arg("temperature").toFloat();                               //očitavanje temperature sa klijenta 2
      humidity = server.arg("humidity").toFloat();                                     //očitavanje vlaž.vazduha sa klijenta 2
      soil_moist = server.arg("soil_moist").toFloat();                                 //očitavanje vlaž.zemljišta sa klijenta 2
      
      //slanje novih vrijednosti na kojima se regulišu izmjerene veličine na klijentu 2
      String response_2 = "@"+String(reg_temp_bot)+","+String(reg_temp_top)+","+String(reg_humi_bot)+","+String(reg_humi_top)+","+String(reg_moisture)+"#";
      server.send(200, "text/plain", response_2);     
    }
}

//----------------------------------------------------------------------------------------------------------------------------------

//funkcija koja se pokreće kada se otvori stranica i koja šalje funkciju web stranice kao odgovor
void handleRoot(){
    server.send(200, "text/html", webpage(temperature, humidity, soil_moist, reg_temp_bot, reg_temp_top, reg_humi_bot, reg_humi_top, reg_moisture)); 
}

//----------------------------------------------------------------------------------------------------------------------------------

//funkcija koja se pokreće kada se unese nepoznata ruta na server
void handleNotFound(){
    server.send(404, "text/plain", "Nije pronadjeno!");
}

//----------------------------------------------------------------------------------------------------------------------------------

//funkcija koja se pokreće prilikom korisnikovog unosa procesa preko web stranice
void handleInput(){
    process_number_in %= 5;                                                            //indeks na koji se unose vrijednosti
    process[process_number_in] = server.arg("process");                                //spremanje procesa
    parts[process_number_in] = server.arg("x");                                        //spremanje broja komada
    process_number_in += 1;                                                            //povećanje indeksa za 1
    server.send(200, "text/plain", "Proces je postavljen na server");
}

//----------------------------------------------------------------------------------------------------------------------------------

//funkcija koja se pokreće prilikom unosa novih vrijednosti na kojima se odvija regulacija veličina (klijent 2)
void handleRegulation(){
    if (server.arg("reg_temp_bot") != ""){                                                //nastavlja ako unos nije prazno polje...
      reg_temp_bot = server.arg("reg_temp_bot").toFloat();                             //očitavanje nove donje temperature za regulisanje
    }
    if (server.arg("reg_humi_bot") != ""){
      reg_humi_bot = server.arg("reg_humi_bot").toFloat();                                //očitavanje nove vlaž.vazduha za regulisanje
    }
    if (server.arg("reg_moist") != ""){
      reg_moisture = server.arg("reg_moist").toFloat();                                   //očitavanje nove vlaž.zemlje za regulisanje  
    }
    if (server.arg("reg_temp_top") != ""){
      reg_temp_top = server.arg("reg_temp_top").toFloat();                                //očitavanje nove gornje temperature  
    }
    if (server.arg("reg_humi_top") != ""){
      reg_humi_top = server.arg("reg_humi_top").toFloat();                                //očitavanje nove gornje vlažnosti vazduha  
    }
    server.send(200, "text/plain", "Vrijednosti regulatora su postavljene.");
}

//----------------------------------------------------------------------------------------------------------------------------------

//osnovni dio programa koji se pokreće samo jednom, koristimo ga za podešavanje 
void setup(){
    WiFi.softAP(ssid, password);                                                       //pokretanje pristupne tačke
    WiFi.softAPConfig(ap_ip, ap_ip, subnet_mask);                                      //podešavanje parametara pristupne tačke

    server.on("/client1/", HTTP_GET, handleClient1);                                   //ruta klijenta 1
    server.on("/client2/", HTTP_GET, handleClient2);                                   //ruta klijenta 2
    server.on("/input", HTTP_GET, handleInput);                                        //ruta unosa procesa
    server.on("/regulation", HTTP_GET, handleRegulation);                              //ruta unosa novih vrijednosti regulatora
    server.on("/", handleRoot);                                                        //osnovna ruta (korijen), tj. web stranica

    server.begin();                                                                    //server prati dolazeće zahtjeve
}

//----------------------------------------------------------------------------------------------------------------------------------

//osnovni dio programa koji se stalno ponavlja
void loop(){
    server.handleClient();                                                              //server "barata" sa zahtjevima
}
