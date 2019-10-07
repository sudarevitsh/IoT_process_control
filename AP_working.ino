#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define AP_SSID "FinalProject"
#define AP_PASS "12345678"

const char* ssid = AP_SSID;
const char* password = AP_PASS;

IPAddress ap_ip (8, 8, 8, 8);
IPAddress subnet_mask (255, 255, 255, 0);

ESP8266WebServer server(80);

void setup(){
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(ap_ip, ap_ip, subnet_mask);
  
  server.begin();
  }
  
void loop(){
  server.handleClient();
  }
