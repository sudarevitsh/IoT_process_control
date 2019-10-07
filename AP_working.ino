#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define AP_SSID "FinalProject"
#define AP_PASS "12345678"

const char* ssid = AP_SSID;
const char* password = AP_PASS;

IPAddress ap_ip (8, 8, 8, 8);
IPAddress subnet_mask (255, 255, 255, 0);

ESP8266WebServer server(80);

void handleClient1(){
  if(server.hasArg("true")){
    server.send();
}  

void handleClient2(){
  float temperature = server.arg("temperature").toFloat();
  float humidity = server.arg("humidity").toFloat();
  float soil_moist = server.arg("soil_moist").toFloat();
  server.send(200, "text/html", "Collected");
}

void handleRoot(){
  server.send(200, "text/html", main_page(temperature, humidity, soil_moist));
}

void setup(){
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(ap_ip, ap_ip, subnet_mask);
  
  server.on("/client1/", HTTP_POST, handleClient1);
  server.on("/client2/", HTTP_GET, handleClient2);
  server.on("/", handleRoot);
  
  server.begin();
  }
  
void loop(){
  server.handleClient();
  }
