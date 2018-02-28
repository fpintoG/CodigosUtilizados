#include <LiquidCrystal.h>
#include <WireExt.h>


#include "WireExt.h"
#include <Wire.h>

#define D6T_addr 0x0A
#define D6T_cmd 0x4C

///////////////////////////////RTC libs////////////////
#include "RTClib.h"
RTC_DS1307 RTC;
///////////////////////////////////////////////////////

////////////////////////////SD lib /////////////////////
#include <SD.h>
File myFile;
char logFile[80];
char logDir[80];
////////////////////////////////////////////////////////

int numbytes = 19; 
int numel = 8;
int rbuf[19]; // Actual raw data is coming in as 35 bytes. Hence the needed for WireExt so that we can handle more than 32 bytes
float tdata[8]; // The data comming from the sensor is in 8 elements
float t_PTAT;

void setup()
{
  Wire.begin();
  RTC.begin(); // Inicia la comunicación con el RTC 
  //RTC.adjust(DateTime(__DATE__, __TIME__)); // Establece la fecha y hora (Comentar una vez establecida la hora)
  Serial.begin(9600);
  ///////////////////////initialize SD card //////////////////////
  Serial.print("Initializing SD card...");
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin 
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
  // or the SD library functions will not work. 
  pinMode(53, OUTPUT);
   
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  
  /////////////////////////////////////////////////////////////////
  pinMode(13, OUTPUT);
}
 
void loop()
{
  int i;
      // Step one - send commands to the sensor
      Wire.begin();
      Wire.beginTransmission(D6T_addr);
      Wire.write(D6T_cmd);
      Wire.endTransmission();

      delay(70); // Delay between instruction and data acquisition

      // Read from the sensor
      // Need to send a read command here - but how will this work with 7 bit addressing?
      Wire.requestFrom(D6T_addr,numbytes); // D6T-8 returns 19 bytes 

      // Receive the data

      if (WireExt.beginReception(D6T_addr) >= 0) { // If there is data still left in buffer
        i = 0;
        for (i=0; i < 19; i++) {
          rbuf[i] = WireExt.get_byte();
        }
        WireExt.endReception();
        t_PTAT = (rbuf[0] + (rbuf[1] << 8) ) * 0.1;
//        if (WireExt.beginReception(D6T_addr) >= 0) {
//          i = 0;
//          for (i = 0; i < 19; i++) {
//            rbuf[i] = WireExt.get_byte();
//         }
//          WireExt.endReception();
// 
//          t_PTAT = (rbuf[0]+(rbuf[1]<<8))*0.1;
//        
//        }
        ///////////////////////////////////////RTC read///////////////////////////
        DateTime now = RTC.now(); // Obtiene la fecha y hora del RTC
        sprintf(logDir, "%d/%d", now.month(), now.day());  
        sprintf(logFile, "%s/hour_%d.csv", logDir, now.hour()); 
        SD.mkdir(logDir);
        myFile = SD.open(logFile, FILE_WRITE);
        if(myFile) {
          
          Serial.print(now.year(), DEC); // Año
          myFile.print(now.year(), DEC);
          Serial.print('/');
          myFile.print('/');
          Serial.print(now.month(), DEC); // Mes
          myFile.print(now.month(), DEC);
          Serial.print('/');
          myFile.print('/');
          Serial.print(now.day(), DEC); // Dia
          myFile.print(now.day(), DEC);
          Serial.print(' ');
          myFile.print(' ');
          Serial.print(now.hour(), DEC); // Horas
          myFile.print(now.hour(), DEC);
          Serial.print(':');
          myFile.print(':');
          Serial.print(now.minute(), DEC); // Minutos
          myFile.print(now.minute(), DEC);
          Serial.print(':');
          myFile.print(':');
          Serial.print(now.second(), DEC); // Segundos
          myFile.print(now.second(), DEC);
          Serial.print(' ');
          myFile.print(' ');
          /////////////////////////////////////////////////////////////////////////
          for (i = 0; i < numel; i++) {
            tdata[i] = (rbuf[(i*2+2)] + (rbuf[(i*2+3)] << 8 )) * 0.1;
            Serial.print(tdata[i]);
            myFile.print(tdata[i]);
            Serial.print(",");
            myFile.print(",");
          }
          Serial.print("\n");
          myFile.print("\n");
          myFile.close();
        }
        
      }

          delay(100);          
}
