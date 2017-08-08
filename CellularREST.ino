


/*
      XBee LTE Cellular IoT Solution
      Copyright (C) 2017  Mouser Electronics

      This program is free software: you can redistribute it and/or modify
      it under the terms of the GNU General Public License as published by
      the Free Software Foundation, either version 3 of the License, or
      any later version.

      This program is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
      GNU General Public License for more details.

      You should have received a copy of the GNU General Public License
      along with this program.  If not, see <http://www.gnu.org/licenses/>.

      Created using XBee LTE Cellular Module and Arduino Mega-2560
      http://www.mouser.com/ProjectManager/ProjectDetail.aspx?AccessID=9a53714c1f

      
      Serial uses pins 0 and 1
      SoftwareSerial uses pins 2 and 3

      
*/

    //variable setup

    #include <SPI.h>
    #include <TH02_dev.h>
    #include <stdlib.h>
    #include <SoftwareSerial.h>
    

    #define sensor1 A0 // Ambient Light
    #define sensor2 A3 // UV 
    #define sensor3 temper
    #define sensor4 humidity
    #define RELAY_PIN 5
    #define PIN_RX 2
    #define PIN_TX 3

    SoftwareSerial monitorSerial = SoftwareSerial(PIN_TX, PIN_RX);

    String inputString = "";
    char input[2048];
    boolean stringComplete = false;
    int timeout = 0;

    float sensorValue1;
    float sensorValue2;
    float temper;
    float humidity;
    float fertemp;
    float relay;


    //Ubidots API information

    char ubidotstoken[] = "<Place Token Here>"; //your token here
    char idlight[] = "<Place Variable ID Here>"; // Ambient Light Sensor ID
    char idUV[] = "<Place Variable ID Here>"; // UV Sensor ID
    char idTemp[] = "<Place Variable ID Here>"; // Temp sensor ID
    char idHumi[] = "<Place Variable ID Here>"; // Humidity sensor ID
    char idRelay [] = "<Place Variable ID Here>"; // Relay ID


    //XBee AT commands

    char chCmd[] = "+++"; // Enter command mode
    char ipCmd[] = "ATIP"; // IP protocol command (should read 1 for TCP)
    char aiCmd[] = "ATAI"; // Association Indication
    char apCmd[] = "ATAP"; // Verify API mode
    char cnCmd[] = "ATCN"; // Exit command
    char apRsp[] = "0"; // API disabled
    char chRsp[] = "OK"; // Command mode response
    char ipRsp[] = "1"; // IP protocol response
    char aiRsp[] = "0"; // Connected to the internet



    void setup() {
      
      //Configure baud rate for Serial ports, verify XBee setup, and Ubidots Connection.
      Serial.begin(9600);
      monitorSerial.begin(9600);
      Serial.flush();
      monitorSerial.flush();
      pinMode(sensor1, INPUT);
      pinMode(sensor2, INPUT);
      pinMode(RELAY_PIN, OUTPUT);
      TH02.begin();
      delay(2000);
      verifyXbee();
      delay(10000);      
      checkUbidots();
      delay(10000);
      
    }

    void loop() {

      AmbientLight();
      UVSensor();
      TempSensor();
      HumiSensor();  
      //RelayActuate(idRelay);  

    }

    void checkUbidots(){
      monitorSerial.println();
      monitorSerial.println("Checking connection to Ubidots");
      Serial.println("GET /api/v1.6/variables HTTP/1.1");
      Serial.println("Host: things.ubidots.com");
      Serial.println("X-Auth-Token: HpmUoreWuuX8igkKwhqzDIWjvY6vWo");
      Serial.println();
      delay(5000);
      checkResponse(10000);
    } 

    void AmbientLight(){

      sensorValue1 = analogRead(sensor1);
      send2Ubidots(sensorValue1, idlight);
      delay(2000);
      if (sensorValue1 < 100){
        digitalWrite(RELAY_PIN, HIGH);
      }
      if (sensorValue1 > 200){
        digitalWrite(RELAY_PIN, LOW);
      }
      send2Ubidots(RELAY_PIN, idRelay);
      delay(5000);
      
    }

    void UVSensor(){

      sensorValue2 = analogRead(sensor2);
      send2Ubidots(sensorValue2, idUV);
      delay(5000);
      
    }

    void TempSensor(){

      temper = TH02.ReadTemperature();
      fertemp = (temper * 9/5)+32;
      send2Ubidots(fertemp, idTemp);
      delay(5000);
      
    }

    void HumiSensor(){

      humidity = TH02.ReadHumidity();
      send2Ubidots(humidity, idHumi);
      delay(5000);
      
    }

    void RelayActuate(char* id){

      //flushSerial();
      monitorSerial.println();
      monitorSerial.println("Sending data to Ubidots");
      char url1[] = "GET /api/v1.6/variables/";
      char* url2 = id;
      char url3[] = "/values?format=csv HTTP/1.1";
      int lurl = strlen(url1) + strlen(url2) + strlen(url3);
      char url[lurl];
      sprintf(url,"%s%s%s",url1,url2,url3);
      Serial.println(url);
      Serial.println("X-Auth-Token: hmxb7K9CQ4B1v1jJXTghtFgUlb2OyN");
      Serial.println("Host: things.ubidots.com");
      Serial.println();
      delay(5000);
      checkResponse(10000);

      
    }
    
    void send2Ubidots(int value, char* id){
      //Send one value to ubidots
      flushSerial();
      Serial.flush();
      monitorSerial.println();
      monitorSerial.println("Sending data to Ubidots");
      
      char url1[] = "POST /api/v1.6/variables/";
      char* url2 = id;
      char url3[] = "/values HTTP/1.1";
      int lurl = strlen(url1) + strlen(url2) + strlen(url3);
      char url[lurl];
      sprintf(url,"%s%s%s",url1,url2,url3);
      Serial.println(url);
      
      Serial.println("X-Auth-Token: hmxb7K9CQ4B1v1jJXTghtFgUlb2OyN");
      Serial.println("Host: things.ubidots.com");
      Serial.println("connection: close");
      Serial.print("Content-Length: ");
      
      char data1[] = "{\"value\":";
      char data2[get_int_len(value)+1];
      char data3[] = "}";
      itoa(value,data2,10);
      int ldata = strlen(data1) + strlen(data2) + strlen(data3);
      char data[ldata];
      sprintf(data,"%s%s%s",data1,data2,data3);
      
      Serial.println(ldata);
      Serial.println("Content-Type: application/json\n");
      Serial.println(data);
      Serial.println(); 
      delay(5000);
      checkResponse(10000);

    }


    void verifyXbee(){
      //Send AT commands and verify response from XBee Module
      
      monitorSerial.println("Verify XBee Module Configuration.");
      while(!SendCMCommand(chCmd, chRsp));
      while(!SendATCommand(ipCmd, ipRsp));
      while(!SendATCommand(aiCmd, aiRsp));
      while(!SendATCommand(apCmd, apRsp));
      SendATCommand(cnCmd);
      monitorSerial.println("XBee module verified!");
      delay(10000);
    }

    boolean SendCMCommand(char* data, char* response){
      flushSerial();
      monitorSerial.print("ATCommand: ");
      delay(2000);
      Serial.print(data);
      monitorSerial.println(data);
      delay(2000);
      Serial.println("");
      return checkResponse(response);
    }

    //This method sends an ATCommand to the XBee
    
    void SendATCommand(char* data){
      flushSerial();
      monitorSerial.print("ATCommand: ");
      delay(2000);
      Serial.println(data);
      monitorSerial.println(data);
      delay(2000);
      checkResponse();
    }


    //This method sends an ATCommand and checks for a response
    
    boolean SendATCommand(char* data, char* response){
      flushSerial();
      monitorSerial.print("ATCommand: ");
      delay(2000);
      Serial.println(data);
      monitorSerial.println(data);
      delay(2000);
      return checkResponse(response);
    }

    //Check if there's a response in the internal buffer
    
    void checkResponse(){
      long contador = 1;
      int cont = 0;
      while(contador < 10000){
        while(Serial.available() > 0){
          char inChar = (char)Serial.read();
          //inputString += inChar;
          if (Serial1.available() == 0) {
            stringComplete = true;
            break;
          }
          else{
            input[cont] = inChar;
            cont++; 
          }
        }
        if(stringComplete) {
          monitorSerial.print("Response output: ");
          monitorSerial.println(inputString);
          resetInput();
          cont = 0;
          stringComplete = false;
        }
        contador++;
      }
    }

    //Check if there's a specific response in the internal buffer

    boolean checkResponse(char* data){
      long contador = 1;
      int cont = 0;
      while(contador < 100000){
        while(Serial.available() > 0){
          char inChar = (char)Serial.read();
          if (Serial.available() == 0) {
            stringComplete = true;
            break;
          }else{
          input[cont] = inChar;
          cont++;
          }
        }
        if(stringComplete) {
          monitorSerial.print(input);
          monitorSerial.print(" Compared with ");
          monitorSerial.println(data);
          int a = strcmp(input, data);
          if(strncmp(input, data, strlen(data)) == 0){
            resetInput();
            cont = 0;
            stringComplete = false;
            return true;
          }
          resetInput();
          cont = 0;
          stringComplete = false;
        }
        contador++;
      }
      return false;
    }

    //Check if there's a response in the internal buffer with a specific timeout
    
    void checkResponse(long timeout){
      long contador = 1;
      while(contador < timeout){
        while(Serial.available() > 0){
          char inChar = (char)Serial.read();
          inputString += inChar;
          if (Serial.available() == 0){
            stringComplete = true;
          } 
        }
        if(stringComplete) {
          monitorSerial.print("Response from Ubidots: ");
          monitorSerial.println(inputString);
          inputString = "";
          stringComplete = false;
          return;
        }        
        contador++;
      }
    }

    //flush the serial input
    void flushSerial() {
        while (Serial.available()) 
        Serial.read();
    }

    //reset the temporal buffer to read data
    void resetInput(){
      for(int i=0; i<255; i++){
        input[i]=0;
      }
    }

    //used to count the number of chars in a value
    long get_int_len (long value){
      int l=1;
      while(value>9){ l++; value/=10; }
      return l;
    }

    
