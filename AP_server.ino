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
float reg_temperature = 20;                                                            //vrijednost na kojoj se reguliše temperatura
float reg_humidity = 60;                                                               //vrijednost na kojoj se reguliše vlažnost vazduha
float reg_moisture = 30;                                                               //vrijednost na kojoj se reguliše vlažnost zemlje 

//nizovi u kojima se čuvaju uneseni poslovi i ponavljanja, prije nego što se pošalju ka klijentu 1 na izvršavanje
String process[5] = {"", "", "", "", ""};                                              //niz procesa koji se nalaze na serveru
String parts[5] = {"","","","",""};                                                    //niz broja ponavljanja procesa 

unsigned int process_number_in = 0;                                                    //indeks unesenog procesa
unsigned int process_number_out = 0;                                                   //indeks poslatog procesa

//----------------------------------------------------------------------------------------------------------------------------------

//web stranica definisana u obliku funkcije sa parametrima temperature, vlažnosti i vrijednosti na kojima se reguliše
//detaljnije objašnjena u radu
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
        <p>Klijent koji upravlja sa 6 LE dioda. U polje ispod se unosi nacin rada lampica i broj ponavljanja.</p>
        <p>Primjer: A+,B+,C+3000,A-,D+2000,E+,F+3000,B-2000</p>
        <p></p>
    <fieldset class="field_class">
    <legend>Unesi novi proces</legend>
    <form action="/input" method="GET">
        <p>Proces:<input class="input_class" name="process"></input></p>

        <p>Broj ponavljanja:<input class="input_class" name="x" style=width:15%; type="number" min="1" max="9"></input></p>
        <p><button class="button_class" value="Submit">Posalji proces na server</button></p>
    </form>
    </fieldset>
        <h3>Klijent #2</h3>
        <p>Klijent koji upravlja sa mikroklimom nekog objekta i prati vrijednost odredjenih velicina.</p>
        <p>Mikrokontroler prati vrijednosti ovih velicina i poredi ih sa zeljenim velicinama koje korisnik unese.</p>
        )=====";

        html +="<fieldset class=\"field_class\"><legend>Izmjerene vrijednosti</legend><form action=\"/regulation\" method=\"GET\">\n";
        html += "<h4>Temperatura:";
        html += TEMPERATURE;
        html += "C</h4><h5>Podesavanje regulatora temperature:";
        html += REG_TEMP;
        html += "C /<input class=\"input_class\" style=width:10%; name=\"reg_temp\" type=\"number\" min=\"0\" max=\"50\" step=\".01\"></h5>\n";
        html += "<h4>Vlaznost vazduha:";
        html += HUMIDITY;
        html += "%</h4><h5>Podesavanje regulatora vlaznosti vazduha:";
        html += REG_HUMI;
        html += "% /<input class=\"input_class\" style=width:10%; name= \"reg_humi\" type=\"number\" min=\"0\" max=\"100\" step=\".01\"></h5>\n";
        html += "<h4>Vlaznost zemljista:";
        html += SOIL_MOIST;
        html += "%</h4><h5>Podesavanje regulatora vlaznosti zemljista:";
        html += REG_MOIST;
        html += "% /<input class=\"input_class\" style=width:10%; name=\"reg_moist\" type=\"number\" min=\"0\" max=\"100\" step=\".01\"></h5>\n";
        html += "<p><button class=\"button_class\" value=\"Submit\">Izmjeni vrijednosti</button></p>";

        html += "</form></fieldset></body>\n";
        html += "</html>\n";

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
      String response_2 = "?" + String(reg_temperature) + "," + String(reg_humidity) + "," + String(reg_moisture) + "#";
      server.send(200, "text/plain", response_2);     
    }
}

//----------------------------------------------------------------------------------------------------------------------------------

//funkcija koja se pokreće kada se otvori stranica i koja šalje funkciju web stranice kao odgovor
void handleRoot(){
    server.send(200, "text/html", webpage(temperature, humidity, soil_moist, reg_temperature, reg_humidity, reg_moisture)); 
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
    if (server.arg("reg_temp") != ""){                                                 //nastavlja ako unos nije prazno polje...
      reg_temperature = server.arg("reg_temp").toFloat();                              //očitavanje nove temperature za regulisanje
    }
    if (server.arg("reg_humi") != ""){
      reg_humidity = server.arg("reg_humi").toFloat();                                 //očitavanje nove vlaž.vazduha za regulisanje
    }
    if (server.arg("reg_moist") != ""){
      reg_moisture = server.arg("reg_moist").toFloat();                                //očitavanje nove vlaž.zemlje za regulisanje  
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
