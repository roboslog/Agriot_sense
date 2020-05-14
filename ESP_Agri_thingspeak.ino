/*------------------------------------------------------------------------------
  10/05/2020
  Author: Narendra,Shivam @ROBOSLOG TEAM
  Platforms: ESP8266
  Language: C++/Arduino
  File: ESP_Agri_thingspeak.ino
------------------------------------------
  Please consider buying products from ROBOSLOF to help fund future
  Open-Source projects like this! We'll always put our best effort in every
  project, and release all our design files and code for you to use.

  https://roboslog.in/agri
  ------------------------------------------------------------------------------
  License:
  Please see attached LICENSE.txt file for details.   
 ----------------------------------------------------------------------------- */
  


#include <SoftwareSerial.h>       //Software Serial library
SoftwareSerial espSerial(4, 3);   //Pin 2 and 3 act as RX and TX. Connect them to TX and RX of ESP8266      
#define DEBUG true
String mySSID = "Robo";       // WiFi SSID
String myPWD = "lopklopk"; // WiFi Password
String myAPI = "0GATARERP787MAFL";   // API Key
String myHOST = "api.thingspeak.com";
String myPORT = "80";


#include <String.h>

#include <DallasTemperature.h>
#include <OneWire.h>
#include <dht.h>
#define DHT11_PIN 11
dht DHT;
const int airPressurePin = 3;  // Sensor connected to A0
int sensorValue = 0;        // value read from the pressure sensor via the amplifier stage
float outputValue = 0;  // value output to the Serial port
float baseValue = 1013.25;
const int soilMoisturePin = A1;
int moistureData;
// int moisturePercent;

const int analogph = A5; 
unsigned long int avgValue; 
float b;
int buf[10],temp;
float phValue;
float airp;

int ph;
int N,P,k;
const int numReadingsN = 10;

int readingsN[numReadingsN];      // the readings from the analog input
int readIndexN = 0;              // the index of the current reading
int totalN = 0;                  // the running total
int averageN = 0;                // the average


const int numReadingsP = 10;

int readingsP[numReadingsP];      // the readings from the analog input
int readIndexP = 0;              // the index of the current reading
int totalP = 0;                  // the running total
int averageP = 0;                // the average


const int numReadingsK = 10;

int readingsK[numReadingsK];      // the readings from the analog input
int readIndexK = 0;              // the index of the current reading
int totalK = 0;                  // the running total
int averageK = 0;                // the average

int inputPinN = A3;
int inputPinP = A2;
int inputPinK = A1;
 
int R1= 1000;
int Ra=25; //Resistance of powering Pins
int ECPin= A0;
//int ECGround=A1;
int ECPower =7;
 
float PPMconversion=0.7;
float TemperatureCoef = 0.019; //this changes depending on what chemical we are measuring
float K=0.52;
 
//************ Temp Probe Related *********************************************//
#define ONE_WIRE_BUS 8          // Data wire For Temp Probe is plugged into pin 10 on the Arduino
const int TempProbePossitive =10;  //Temp Probe power connected to pin 9
const int TempProbeNegative=9;    //Temp Probe Negative connected to pin 8
 
OneWire oneWire(ONE_WIRE_BUS);// Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire);// Pass our oneWire reference to Dallas Temperature.
 
 
float tempC=10;
float EC=0;
float EC25 =0;
int ppm =0;
 
 
float raw= 0;
float Vin= 5;
float Vdrop= 0;
float Rc= 0;
float buffer=0;


 

void setup()
{
  
  Serial.begin(9600);
  espSerial.begin(115200);
  espData("AT+RST", 1000, DEBUG);                      //Reset the ESP8266 module
  espData("AT+CWMODE=3", 1000, DEBUG);    
//  espData("AT+CWSAP=”ESP”,”password”,1,4");//Set the ESP mode as station mode
  espData("AT+CWJAP=\""+ mySSID +"\",\""+ myPWD +"\"", 1000, DEBUG);   //Connect to WiFi network
  delay(2000);
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  digitalWrite(13,1);
  digitalWrite(12,0);
  pinMode(TempProbeNegative , OUTPUT ); //seting ground pin as output for tmp probe
  digitalWrite(TempProbeNegative , LOW );//Seting it to ground so it can sink current
  pinMode(TempProbePossitive , OUTPUT );//ditto but for positive
  digitalWrite(TempProbePossitive , HIGH );
  pinMode(ECPin,INPUT);
  pinMode(ECPower,OUTPUT);//Setting pin for sourcing current
 // pinMode(ECGround,OUTPUT);//setting pin for sinking current
//  digitalWrite(ECGround,LOW);//We can leave the ground connected permanantly
 
  delay(100);// gives sensor time to settle
  sensors.begin();
  delay(100);
  //** Adding Digital Pin Resistance to [25 ohm] to the static Resistor *********//
  // Consule Read-Me for Why, or just accept it as true
  R1=(R1+Ra);// Taking into acount Powering Pin Resitance
  Serial.println("EC-PPM measurments");
  pinMode(airPressurePin, INPUT);
  pinMode(soilMoisturePin, INPUT);
  Serial.begin(9600);
  sensors.begin();
  Serial.print("EC-start");
  delay(1000);
  
  for (int thisReadingN = 0; thisReadingN < numReadingsN; thisReadingN++) {
    readingsN[thisReadingN] = 0;
}

}
 
void loop()
{
  Serial.println("SenSe Start");
  GetEC(); 
  tempC=0;//Calls Code to Go into GetEC() Loop [Below Main Loop] dont call this more that 1/5 hhz [once every five seconds] or you will polarise the water
  PrintReadings();  // Cals Print routine [below main loop]
  pH();   
  Nf();
  Pf();
  ph=phValue;
                                         //AIR TEMPERAURE AND HUMIDITY
  int chk = DHT.read11(DHT11_PIN);
                                              //AIR PRESSURE
 sensorValue = analogRead(airPressurePin);            
 outputValue = map(sensorValue, 10, 1023, 0, 100); 
 airp=outputValue + baseValue;
                                     //SOIL MOISTURE
  moistureData = analogRead(soilMoisturePin);
  delay(1000);
int sendVal=0;
Serial.println("Sending...");
 String sendData = "GET /update?api_key="+ myAPI +"&field1="+String(tempC)+"&field2="+String(DHT.temperature)+"&field3="+String(DHT.humidity)+"&field4="+String(airp)+"&field5="+String(moistureData)+"&field6="+String(ph);
    //  String sendData = "GET /update?api_key="+ myAPI +"&field1="+String(sendVal)+"&field2="+String(sendVal)+"&field3="+String(sendVal)+"&field4="+String(sendVal)+"&field5="+String(sendVal)+"&field6="+String(sendVal);
  
    espData("AT+CIPMUX=1", 1000, DEBUG);       //Allow multiple connections
    espData("AT+CIPSTART=0,\"TCP\",\""+ myHOST +"\","+ myPORT, 1000, DEBUG);
    espData("AT+CIPSEND=0," +String(sendData.length()+4),1000,DEBUG);  
    espSerial.find(">"); 
    espSerial.println(sendData);     
    espData("AT+CIPCLOSE=0",1000,DEBUG);
    delay(2000);
}


 
void pH() {
 for(int i=0;i<10;i++) 
 { 
  buf[i]=analogRead(analogph);
  delay(10);
 }
 for(int i=0;i<9;i++)
 {
  for(int j=i+1;j<10;j++)
  {
   if(buf[i]>buf[j])
   {
    temp=buf[i];
    buf[i]=buf[j];
    buf[j]=temp;
   }
  }
 }
 avgValue=0;
 for(int i=2;i<8;i++)
 avgValue+=buf[i];
 float pHVol=(float)avgValue*5.0/1024/6;
 phValue = -5.70 * pHVol + 21.34;
 Serial.print("sensor = ");
 Serial.println(phValue);
 delay(20);
} 

void Nf()
{
  totalN = totalN - readingsN[readIndexN];
  // read from the sensor:
  readingsN[readIndexN] = analogRead(inputPinN);
  readingsN[readIndexN]  = map(readingsN[readIndexN], 0, 1023, 4, 15);
  // add the reading to the total:
  totalN = totalN + readingsN[readIndexN];
  // advance to the next position in the array:
  readIndexN = readIndexN + 1;

  // if we're at the end of the array...
  if (readIndexN >= numReadingsN) {
    // ...wrap around to the beginning:
    readIndexN = 0;
  }

  // calculate the average:
  averageN = totalN / numReadingsN;
  // send it to the computer as ASCII digits
   Serial.println("N== ");
  Serial.println(averageN);
  N=averageN;
  delay(1);        // delay in between reads for stability
  
}
void Pf()
{
  totalP = totalP - readingsP[readIndexP];
  // read from the sensor:
  readingsP[readIndexP] = analogRead(inputPinP);
  readingsP[readIndexP]  = map(readingsP[readIndexP], 0, 1023, 45, 65);
  // add the reading to the total:
  totalP = totalP + readingsP[readIndexP];
  // advance to the next position in the array:
  readIndexP = readIndexP + 1;

  // if we're at the end of the array...
  if (readIndexP >= numReadingsP) {
    // ...wrap around to the beginning:
    readIndexP = 0;
  }

  // calculate the average:
  averageP = totalP / numReadingsP;
  // send it to the computer as ASCII digits
   Serial.println("& P == ");
  Serial.println(averageP);
  P=averageP;
  delay(1);        // delay in between reads for stability
  
}
void Kf()
{
  totalK = totalK - readingsP[readIndexK];
  // read from the sensor:
  readingsK[readIndexK] = analogRead(inputPinK);
  readingsK[readIndexK]  = map(readingsP[readIndexK], 0, 1023, 45, 65);
  // add the reading to the total:
  totalK = totalK + readingsP[readIndexK];
  // advance to the next position in the array:
  readIndexK = readIndexK + 1;

  // if we're at the end of the array...
  if (readIndexK >= numReadingsK) {
    // ...wrap around to the beginning:
    readIndexK = 0;
  }

  // calculate the average:
  averageK = totalK / numReadingsK;
  // send it to the computer as ASCII digits
   Serial.println("& P == ");
  Serial.println(averageK);
  K=averageK;
  delay(1);        // delay in between reads for stability
  
}

void GetEC(){
  
//*********Reading Temperature Of Solution *******************//
sensors.requestTemperatures();// Send the command to get temperatures
tempC=sensors.getTempCByIndex(0); //Stores Value in Variable

//************Estimates Resistance of Liquid ****************//
digitalWrite(ECPower,HIGH);
raw= analogRead(ECPin);
raw= analogRead(ECPin);// This is not a mistake, First reading will be low beause if charged a capacitor
digitalWrite(ECPower,LOW);

//***************** Converts to EC **************************//
Vdrop= (Vin*raw)/1024.0;
Rc=(Vdrop*R1)/(Vin-Vdrop);
Rc=Rc-Ra; //acounting for Digital Pin Resitance
EC = 1000/(Rc*K);

//*************Compensating For Temperaure********************//
EC25  =  EC/ (1+ TemperatureCoef*(tempC-25.0));
ppm=(EC25)*(PPMconversion*1000);

;}
//************************** End OF EC Function ***************************//
 
 
//***This Loop Is called From Main Loop- Prints to serial usefull info ***//
void PrintReadings(){
Serial.print("Rc: ");
Serial.print(Rc);
Serial.print(" EC: ");
Serial.print(EC25);
Serial.print(" Simens  ");
Serial.print(ppm);
Serial.print(" ppm  ");
Serial.print(tempC);
Serial.println(" *C ");
 
 
/*
//********** Usued for Debugging ************
Serial.print("Vdrop: ");
Serial.println(Vdrop);
Serial.print("Rc: ");
Serial.println(Rc);
Serial.print(EC);
Serial.println("Siemens");
//********** end of Debugging Prints *********
*/
};

 String espData(String command, const int timeout, boolean debug)
{
  Serial.print("AT Command ==> ");
  Serial.print(command);
  Serial.println("     ");
  
  String response = "";
  espSerial.println(command);
  long int time = millis();
  while ( (time + timeout) > millis())
  {
    while (espSerial.available())
    {
      char c = espSerial.read();
      response += c;
    }
  }
  if (debug)
  {
    //Serial.print(response);
  }
  return response;
}
