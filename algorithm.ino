  String job = "?A+250,B+220,C+,A-400#";
  int parts = 4;
  unsigned int ch_no = 0;
  int spec_count = 0;
  int counter = 0;
  boolean client_free = false;
  int job_len;
  

  void setup(){
    Serial.begin(9600);
  }
  
  void loop(){
    
  while(!client_free){
  for(int part_count = 1; part_count <= parts; part_count++){ 
    job_len = job.length();
    for (ch_no = 0; ch_no < job_len ; ch_no ++){
      char com = job.charAt(ch_no);
      if ( com == '?' || com == ',' || com == '#'){
        spec_count += 1;
      }
  }

  int char_place[spec_count];
  memset(char_place, 0, sizeof(char_place));
  int opps = 0; 
  
  
  for (int ch_no = 0; ch_no < job_len; ch_no ++){
    char com = job.charAt(ch_no);
    if (com == '?' || com == ',' || com == '#'){
      char_place[opps] = ch_no;  
      opps += 1;  
    }

  } 
  Serial.println("prebrojavanje i indeksi su dobri!");
  for(counter = 0; counter < spec_count - 1; counter ++){
    Serial.print("Counter je: ");Serial.println(counter);
    Serial.print("poredmimo sa: "); Serial.println(spec_count);
    String delay_timer = "";
    for(ch_no = char_place[counter] + 1; ch_no < char_place[counter + 1]; ch_no ++){
      char com = job.charAt(ch_no);
      Serial.print("timer: ");Serial.println(delay_timer);
      Serial.print("znak ");Serial.println(com);
      Serial.print("broj je: ");
      Serial.println(isnan(String(job.charAt(ch_no)).toInt()));
      if (com == 'A' || com == 'B' || com == 'C' || com == 'D' || com == 'E' || com == 'F'){
        if(job.charAt(ch_no + 1) == '+'){
          Serial.println("HIGH"); 
          digitalWrite(job.charAt(ch_no), HIGH);}
        else if(job.charAt(ch_no + 1) == '-'){
          Serial.println("LOW");
          digitalWrite(job.charAt(ch_no), LOW);}
      }
      else if (com == '1' || com == '2' ||  com == '3' || com == '4' || com == '5' || com == '6' || com == '7' || com == '8' || com == '9' || com == '0'){
        delay_timer += String(com);
      }
           
    }
    Serial.print("Vremenski period je:");Serial.println(delay_timer.toInt());
    delay(delay_timer.toInt());
  }
  spec_count = 0;
  client_free = true;
 }
 }
 Serial.print("Dijelova napravljeno");Serial.println(parts);
 Serial.println("Cekanje serveraaaaa");
 
 delay(5000);}

