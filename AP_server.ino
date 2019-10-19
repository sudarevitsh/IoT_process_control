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
  
float reg_temp = 20;
float reg_humi = 60;
float reg_moist = 30;

String process = "";
String x = "";
int process_index = 0;
  
//-----------------------------------------------------------------------------------------------------------------------

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
      
      <p>Broj ponavljanja:<input class="input_class" name="x" style=width:15%; type="number" min="1" max="9"></input></p>
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

//-----------------------------------------------------------------------------------------------------------------------

void handleClient1(){
  if(server.arg("client_id") == "1" && server.arg("client_free" == "1")){
//    String response_1 = ("repeat=" + String(x) + "&process=" + String(process) + "##");
  //  server.send(200, "text/plain", response_1);                
  }
}  

void handleClient2(){
  if (server.arg("client_id") == "2"){                      
    temperature = server.arg("temperature").toFloat();     
    humidity = server.arg("humidity").toFloat();           
    soil_moist = server.arg("soil_moist").toFloat();  
    String response_2 = "?" + String(reg_temp) + "," + String(reg_humi) + "," + String(reg_moist) + "#";
    server.send(200, "text/plain", response_2);     
  }
}


void handleRoot(){
  server.send(200, "text/html", webpage(temperature, humidity, soil_moist, reg_temp, reg_humi, reg_moist)); 
}

void handleNotFound(){
  server.send(404, "text/plain", "Nije pronadjeno");
}
 
void handleInput(){
  process = server.arg("process");
  x = server.arg("x");
  server.send(200, "text/plain", "Proces je poslan na server");
}

void handleRegulation(){
  if (server.arg("reg_temp") != ""){
    reg_temp = server.arg("reg_temp").toFloat();
  }
  if (server.arg("reg_humi") != ""){
    reg_humi = server.arg("reg_humi").toFloat();
  }
  if (server.arg("reg_moist") != ""){
    reg_moist = server.arg("reg_moist").toFloat();
  }
  server.send(200, "text/plain", "Vrijednosti su promjenjene!");
}

//-----------------------------------------------------------------------------------------------------------------------

void setup(){
  WiFi.softAP(ssid, password);                                
  WiFi.softAPConfig(ap_ip, ap_ip, subnet_mask);       
   
  server.on("/client1/", HTTP_GET, handleClient1);          
  server.on("/client2/", HTTP_GET, handleClient2);          
  server.on("/input", HTTP_GET, handleInput);
  server.on("/regulation", HTTP_GET, handleRegulation);
  server.on("/", handleRoot);                                 
  
  server.begin();                                            
}

//-----------------------------------------------------------------------------------------------------------------------

void loop(){
  server.handleClient();  
}
