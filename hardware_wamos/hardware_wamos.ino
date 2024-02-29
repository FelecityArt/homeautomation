#include <SoftwareSerial.h>
// IMPORT ALL REQUIRED LIBRARIES
#include <NewPing.h>
#include <ArduinoJson.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <SPI.h>
#include <Wire.h>
//**********ENTER IP ADDRESS OF SERVER******************//

#define HOST_IP     "10.22.16.191"       // REPLACE WITH IP ADDRESS OF SERVER ( IP ADDRESS OF COMPUTER THE BACKEND IS RUNNING ON) 
#define HOST_PORT   "8080"            // REPLACE WITH SERVER PORT (BACKEND FLASK API PORT)
#define route       "api/update"      // LEAVE UNCHANGED 
#define idNumber    "620151149"       // REPLACE WITH YOUR ID NUMBER 

// WIFI CREDENTIALS
#define SSID        "MonaConnect"      // "REPLACE WITH YOUR WIFI's SSID"   
#define password    ""  // "REPLACE WITH YOUR WiFi's PASSWORD" 

#define stay        100
 
//**********PIN DEFINITIONS******************//

 
#define espRX         10
#define espTX         11
#define espTimeout_ms 300

#define TRIG_PIN 3  // Trigger pin
#define ECHO_PIN    4 // Echo pin
#define Max_Height 77.763 // Maximum water level/height from the base of the tank
#define Max_Capacity  1000 // Maximum capacity of the tank in US Gallons
#define Tank_Height 94.5 // Ultrasonic sensor height from the base of the tank
#define Tank_Diameter 61.5 // Diameter of the tank


 
/* Declare your functions below */
void espSend(char command[]);
void espUpdate(char mssg[]);
void espInit();
double getWaterHeight(double radar);
double getRadar();
double getWaterReserve(double height);


SoftwareSerial esp(espRX, espTX); 
NewPing sonar(TRIG_PIN, ECHO_PIN,1000); // NewPing setup of pins and maximum distance.

void setup(){

  Serial.begin(115200); 
  // Configure GPIO pins here
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
 

  espInit();  
 
}

void loop(){ 
   
  // send updates with schema ‘{"id": "student_id", "type": "ultrasonic", "radar": 0, "waterheight": 0, "reserve": 0, "percentage": 0}’
  double radar = sonar.ping_in();
  double WaterHeight = getWaterHeight(radar);

  double reserve = getWaterReserve(WaterHeight); // Calculate water reserve
  double percentage = (WaterHeight / Max_Height) * 100; // Calculate percentage of water in the tank

  StaticJsonDocument<290> doc;
  char mssg[290] = {0};

  Serial.println(radar);

  doc["id"] = "620151149";
  doc["type"] = "ultrasonic";
  doc["radar"] = radar;
  doc["waterheight"] = WaterHeight;
  doc["reserve"] = reserve;
  doc["percentage"] = percentage;

  serializeJson(doc, mssg);

  // snprintf(mssg,sizeof(mssg),"{\"id\": \"%s\", \"type\": \"ultrasonic\", \"radar\": 0, \"waterheight\": %.2f, \"reserve\": %.2f, \"percentage\": %.2f}",idNumber,WaterHeight,reserve,percentage);

  // Serial.println(mssg);
  espUpdate(mssg);
  


  delay(1000);  
}

 
void espSend(char command[] ){   
    esp.print(command); // send the read character to the esp    
    while(esp.available()){ Serial.println(esp.readString());}    
}


void espUpdate(char mssg[]){ 
    char espCommandString[50] = {0};
    char post[290]            = {0};

    snprintf(espCommandString, sizeof(espCommandString),"AT+CIPSTART=\"TCP\",\"%s\",%s\r\n",HOST_IP,HOST_PORT); 
    espSend(espCommandString);    //starts the connection to the server
    delay(stay);

    // GET REQUEST 
    // snprintf(post,sizeof(post),"GET /%s HTTP/1.1\r\nHost: %s\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n%s\r\n\r\n",route,HOST_IP,strlen(mssg),mssg);

    // POST REQUEST
    snprintf(post,sizeof(post),"POST /%s HTTP/1.1\r\nHost: %s\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n%s\r\n\r\n",route,HOST_IP,strlen(mssg),mssg);
  
    snprintf(espCommandString, sizeof(espCommandString),"AT+CIPSEND=%d\r\n", strlen(post));
    espSend(espCommandString);    //sends post length
    delay(stay);
    Serial.println(post);
    espSend(post);                //sends POST request with the parameters 
    delay(stay);
    espSend("AT+CIPCLOSE\r\n");   //closes server connection
   }

void espInit(){
    char connection[100] = {0};
    esp.begin(115200); 
    Serial.println("Initiallizing");
    esp.println("AT"); 
    delay(1000);
    esp.println("AT+CWMODE=1");
    delay(1000);
    while(esp.available()){ Serial.println(esp.readString());} 

    snprintf(connection, sizeof(connection),"AT+CWJAP=\"%s\",\"%s\"\r\n",SSID,password);
    esp.print(connection);

    delay(3000);  //gives ESP some time to get IP

    if(esp.available()){   Serial.print(esp.readString());}
    
    Serial.println("\nFinish Initializing");    
   
}

//***** Design and implement all util functions below ******
double getWaterHeight(double radar){
  // Calculate water height using the radar measurement
  double val = radar + 16.737;
  return Tank_Height - val;
}

double getWaterReserve(double height){
  // Calculate water reserve using the radar measurement
  double tank_radius = Tank_Diameter / 2.0;
  // Convert height to gallons using the volume formula for a cylinder
  double volume = M_PI * tank_radius * tank_radius * height / 231.0; // 231 cubic inches in a gallon

  return volume;
}
