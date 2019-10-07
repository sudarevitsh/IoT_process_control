#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define AP_SSID "Zavrsni_Rad"
#define AP_PASS "12345678"

const char* ssid = AP_SSID;
const char* password = AP_PASS;

IPAddress ap_ip (8, 8, 8, 8);
IPAddress subnet_mask (255, 255, 255, 0);

ESP8266WebServer server(80);

float temperature = 0;
float humidity = 0;
float soil_moist = 0;

String webpage(float temperature, float humidity, float soil_moist){
String html = R"=====(
<!DOCTYPE html><html>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <head>
  <title>Zavrsni projekat</title>
  <style>
    body {background-color: #CFCCCC ; text-align: center; color: black; font-family: Arial, serif;}
    
    .input_class{width: 90%; height: 80px; padding 10px 10px; margin: 5px 5px; box-sizing: border-box;
    border: 2px #854D41; border-radius: 5px; background-color: white; color: #854D41;}
    
    .button_class{width: 90%; height:35px; padding 10px 10px; margin: 5px 5px; box-sizing: border-box;
    border: 2px #854D41; border-radius: 5px; background-color: white; color: #854D41;}
  </style>
  </head>
  
  
  <body>
    <h2>ZAVRSNI RAD</h2>
    <h3>Klijent #1</h3>
    <p>Klijent koji upravlja izvrsavanjem nekog industrijskog procesa,
    u ovom slucaju se koriste LE diode za simulaciju upravljanja izvrsnim elementima.</p>
    <p>Postoje 3 vec definisana procesa koja mogu da se pozovu, 
    ali i polje za unos kako bi se izmjenio neki od njih.</p>
  
    <p><button class="button_class" href="/process1" id=1>Proces #1</button></p>
    <p><button class="button_class" href="/process2" id=2>Proces #2</button></p>
    <p><button class="button_class" href="/process3" id=3>Proces #3</button></p>
    <p><input class="input_class"></input></p>
    <p><button class="button_class" style="width:50%;">Izmjeni proces</button></p>
  
    <h3>Klijent #2</h3>
    <p>Klijent koji upravlja sa mikroklimom nekog objekta i prati vrijednost odredjenih velicina.</p>
    <p>Mikrokontroler prati vrijednosti ovih velicina i na osnovu njih pokrece sisteme za njihovu regulaciju.</p>
    <p>Sistemi za regulaciju takodje ce biti simulirani LE diodama.</p>
    )=====";

    html += "<h4>Temperatura:<span>";
    html += (float)temperature;
    html += "</span>C</h4>\n";
    html += "<h4>Vlaznost vazduha:<span>";
    html += (float)humidity;
    html += "</span>[%]</h4>\n";
    html += "<h4>Vlaznost zemljista:<span id=\"soil_moist\">";
    html += (float)soil_moist;
    html += "</span>[%]</h4>\n";
  
    html += "</body>\n";
    html += "</html>\n";

  return html;
}

/*void handleClient1(){
  if(server.arg("free") == true && server.arg("client_id") == "1"){
    server.send();
}  */

void handleClient2(){
  if (server.arg("client_id") == "2"){
  temperature = server.arg("temperature").toFloat();
  humidity = server.arg("humidity").toFloat();
  soil_moist = server.arg("soil_moist").toFloat();
  server.send(200, "text/plain", "Collected");
  }
}

void handleRoot(){
  server.send(200, "text/html", webpage(temperature, humidity, soil_moist));
}

void setup(){
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(ap_ip, ap_ip, subnet_mask);
  
//  server.on("/client1/", HTTP_GET, handleClient1);
  server.on("/client2/", HTTP_GET, handleClient2);
  server.on("/", handleRoot);
  
  server.begin();
  }
  
void loop(){
  server.handleClient();
  }
