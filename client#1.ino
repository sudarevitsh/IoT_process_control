#include <ESP8266WiFi.h>                                                               //ESP 8266 WiFi biblioteka

//parametri pristupne tačke na koju se klijent povezuje
const char* ssid = "Zavrsni_Rad";                                                      //naziv pristupne tačke na koju se klijent povezuje
const char* password = "12345678";                                                     //lozinka pristupne tačke

//parametri servera na koji se šalju zahtjevi
const byte port = 80;                                                                  //port preko kog se pristupa serveru
String host_str = "8.8.8.8";                                                           //IP adresa servera
String route = "/client1/";                                                            //rezervisana ruta za klijenta 1

//inicijalizacija klijenta
WiFiClient client;                                                                     //inicijalizacija wifi klijenta 
byte id = 1;                                                                           //ID broj klijenta

//promjenljive vezane za trenutni posao koji se izvršava
boolean client_free = true;                                                            //stanje klijenta (zauzet - false)
String job = "";                                                                       //String objekat posla koji dolazi sa servera
int parts;                                                                             //broj ponavljanja posla

//promjenljive koje se koriste u algoritmu za razradu posla/procesa iz dobijenog odgovora sa servera 
unsigned int char_number = 0;                                                          //indeks znaka iz niza              
int break_count = 0;                                                                   //broj određenih prelomnih znakova (?,#)
int operations;                                                                        //broj operacija (+1)
int job_length;                                                                        //broj znakova unutar posla
String delay_timer = "";                                                               //vrijeme trajanja stanja u procesu 
int counter;                                                                           //brojač radnji u toku procesa

//--------------------------------------------------------------------------------------------------------------------------------------------

//funkcija kojom se znak lampice pretvara u odgovarajući broj pina  
int char_to_pin(char led_char){
  int led_pin;
  switch (led_char){
    case 'A':                                                                          //znak A se pretvara u GPIO14
    led_pin = 14;
    break;
    case 'B':                                                                          //analogno prethodnom...
    led_pin = 12;
    break;
    case 'C':
    led_pin = 13;
    break;
    case 'D':
    led_pin = 15;
    break;
    case 'E':
    led_pin = 3;
    break;
    case 'F':
    led_pin = 2;
    break;
  }
  return led_pin;
}

//--------------------------------------------------------------------------------------------------------------------------------------------

//funkcija kojom se sva stanja na pinovima resetuju, tj. resterećuju
void pin_reset(){
  digitalWrite(14, LOW);
  digitalWrite(12, LOW);
  digitalWrite(13, LOW);
  digitalWrite(15, LOW);
  digitalWrite(3, LOW);
  digitalWrite(2, LOW);
  delay(50);
}

//--------------------------------------------------------------------------------------------------------------------------------------------

//algoritam kojim se iz String objekta formira proces koji je korisnik unio i paralelno izvršava
void algorithm(){  
  for(int part_count = 1; part_count <= parts; part_count++){                          //proces se ponavlja uneseni broj puta 
    delay(500);
    job_length = job.length();                                                         //broj znakova u primljenom odgovoru 
    for (char_number = 0; char_number < job_length ; char_number ++){                  //traženje određenih znakova u nizu
      char com = job.charAt(char_number);
      if ( com == '?' || com == ',' || com == '#'){
        break_count += 1;                                                              //brojač pronađenih željenih znakova
      }
  }

  int char_place[break_count];                                                         //formiranje niza za indeks željenih znakova
  memset(char_place, 0, sizeof(char_place));                                           //postavljanje vrijednosti elemenata na 0                     
  operations = 0;
  counter = 0;
  
  for (int char_number = 0; char_number < job_length; char_number ++){                 //prebrojavanje slično prvom
    char com = job.charAt(char_number);
    if (com == '?' || com == ',' || com == '#'){
      char_place[operations] = char_number;                                            //ovaj put se spremaju indeksi željenih znakova
      operations += 1;  
    }
  } 
  pin_reset();                                                                         //resetovanje pinova  
  
  for(counter = 0; counter < break_count - 1; counter ++){                             //brojač znakova u jednoj radnji
    delay_timer = "";                                                                  //resetovanje tajmera trajanja stanja
     
 
    //brojač indeksa svakog od znakova unutar jedne radnje
    for(char_number = char_place[counter] + 1; char_number < char_place[counter + 1]; char_number ++){
      char com = job.charAt(char_number);                                              //znak na mjestu broja brojača
    
      //poređenje znaka, da li je slovo ili neki broj
      if (com == 'A' || com == 'B' || com == 'C' || com == 'D' || com == 'E' || com == 'F'){
        
        if(job.charAt(char_number + 1) == '+'){                                        //ako je slovo, provjerava se i naredni znak
          digitalWrite(char_to_pin(com), HIGH);                                        //na osnovu tih znakova se određuje novo stanje lampice
        }
        else if(job.charAt(char_number + 1) == '-'){                                   //ako je minus, lampica se gasi...
          digitalWrite(char_to_pin(com), LOW); 
        }
      }
      else if (isDigit(com)){
        delay_timer += String(com);                                                    //ako se radi o broju, on se koncentriše sa dosadašnjim brojevma
      }       
    }
    delay(delay_timer.toInt());                                                        //niz brojeva se pretvara u cjelobrojnu vrijednost i čeka se
  }
  break_count = 0;                                                                     //resetovanje brojača
  }
}

//--------------------------------------------------------------------------------------------------------------------------------------------

//osnovni dio programa koji se pokreće samo jednom, koristimo ga za podešavanje
void setup(){
  
  //podešavanje izlaznih pinova, tj. postavljanje pinova u izlazni mod
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(12, OUTPUT); 
  pinMode(13, OUTPUT);
  pinMode(15, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(2, OUTPUT);

  pin_reset();                                                                         //resetovanje pinova
   
  //podešavanje i pokretanje wifi objekta
  WiFi.mode(WIFI_STA);                                                                 //postavljanje u mod stanice
  WiFi.begin(ssid, password);                                                          //početak konekcije sa pristupnom tačkom
  
  //signalizacija da povezivanje i dalje traje...
  while(WiFi.status() != WL_CONNECTED){
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
  }
}

//--------------------------------------------------------------------------------------------------------------------------------------------

//osnovni dio programa koji se stalno ponavlja
void loop(){
 
  //slanje zahtjeva serveru za novi posao, na osnovu stanja klijenta
  if(client_free){
    client.connect(host_str, port);                                                   //ako je klijent slobodan, povezuje se sa serverom
    client_free = false;                                                              //promjena stanja klijenta u "zauzet"
    
    //formiranje zahtjeva, sa stanjem i ID brojem kako bi server mogao da zna o kom klijentu se radi
    String request = String(route + "?client_id=" + String(id) + "&client_free=" + String(client_free));
   
    //slanje zahtjeva na server
    client.print(String("GET " + request + " HTTP/1.1\r\n" + "Host: " + host_str + "\r\n" + "Connection: keep-alive\r\n\r\n"));   
    
    //čekanje odgovora sa servera, ako čekanje traje duže od 5 sekundi, veza se prekida
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
       
        String line = client.readStringUntil('$');                                     //čitanje serverovog odgovora do znaka '$'
               
        unsigned int beginning = line.indexOf('@');                                    //indeks početnog znaka '@'
        unsigned int mid = line.indexOf('x', beginning);                               //indeks znaka 'x' koji razdvaja odgovor  
        unsigned int ending = line.indexOf('#', beginning );                           //indeks posljednjeg željenog znaka '#'
        
        //izvlačenje željenih vrijednosti iz odgovora
        parts = line.substring(beginning + 1, mid).toInt();                            //broj komada, tj. ponavljanja procesa
        job = line.substring(mid + 1 , ending + 1);                                    //proces koji je klijent unio
        job.toUpperCase();
        delay(50);
        client.flush();                                                                //brisanje svih neočitanih bitova        
      }
    }
    
  }
  
  //izvršavanje procesa kroz već opisani algoritam
  while (!client_free){
      algorithm();
      pin_reset();
      delay(2000);
      //kada se proces/posao obavi, klijent je ponovo slobodan i može da zahtjeva novi posao od servera
      client_free = true;                                                               
  } 
}
