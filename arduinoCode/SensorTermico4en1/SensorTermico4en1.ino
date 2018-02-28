#include <LiquidCrystal.h>
#include <WireExt.h>


#include "WireExt.h"
#include <Wire.h>

#define D6T_addr 0x0A
#define D6T_cmd 0x4C

///////////////////humidity and temperature sensor//////////////////////
#include <DHT.h>
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
unsigned long previousMillis = 0;
const long interval = 10000;
float DHT11data[5]; 
int sendDHT11Data = 0;
/////////////////////////////////////////////////////////////////////////


///////////////////////////gas sensor////////////////////////////////////
const int calibrationLed = 13;                      //when the calibration start , LED pin 13 will light up , off when finish calibrating
const int MQ_PIN=A0;                                //define which analog input channel you are going to use
int RL_VALUE=5;                                     //define the load resistance on the board, in kilo ohms
float RO_CLEAN_AIR_FACTOR=9.83;                     //RO_CLEAR_AIR_FACTOR=(Sensor resistance in clean air)/RO,
                                                    //which is derived from the chart in datasheet
 
/***********************Software Related Macros************************************/
int CALIBARAION_SAMPLE_TIMES=50;                    //define how many samples you are going to take in the calibration phase
int CALIBRATION_SAMPLE_INTERVAL=500;                //define the time interal(in milisecond) between each samples in the
                                                    //cablibration phase
int READ_SAMPLE_INTERVAL=50;                        //define how many samples you are going to take in normal operation
int READ_SAMPLE_TIMES=5;                            //define the time interal(in milisecond) between each samples in 
                                                    //normal operation
 
/**********************Application Related Macros**********************************/
#define         GAS_LPG             0   
#define         GAS_CO              1   
#define         GAS_SMOKE           2    
 
/*****************************Globals***********************************************/
float           LPGCurve[3]  =  {2.3,0.21,-0.47};   //two points are taken from the curve. 
                                                    //with these two points, a line is formed which is "approximately equivalent"
                                                    //to the original curve. 
                                                    //data format:{ x, y, slope}; point1: (lg200, 0.21), point2: (lg10000, -0.59) 
float           COCurve[3]  =  {2.3,0.72,-0.34};    //two points are taken from the curve. 
                                                    //with these two points, a line is formed which is "approximately equivalent" 
                                                    //to the original curve.
                                                    //data format:{ x, y, slope}; point1: (lg200, 0.72), point2: (lg10000,  0.15) 
float           SmokeCurve[3] ={2.3,0.53,-0.44};    //two points are taken from the curve. 
                                                    //with these two points, a line is formed which is "approximately equivalent" 
                                                    //to the original curve.
                                                    //data format:{ x, y, slope}; point1: (lg200, 0.53), point2: (lg10000,  -0.22)                                                     
float           Ro           =  10;                 //Ro is initialized to 10 kilo ohms
/////////////////////////////////////////////////////////////////////////



int rbuf[35];
float tdata[17];
float t_PTAT;
int nunPin1 = 8;                // Digital pin to select nunchuck1
int nunPin2 = 9;                // Digital pin to select nunchuck2
int nunPin3 = 10;                // Digital pin to select nunchuck1
int nunPin4 = 11;                // Digital pin to select nunchuck2
int led1 = 2;                   // Pin connected to LED1 (turns off when nunchuck1's z_button is pushed
int led2 = 3;                   // pin connected to LED2 (turns off when nunchuck2's z_button is pushed
int state = 1;
int state2;
char serialIn;
int sendFlag = 0;
 
void setup()
{
  Wire.begin();
  dht.begin();
  Serial.begin(9600);
  Serial.flush();
  
  pinMode(nunPin1, OUTPUT);
  pinMode(nunPin2, OUTPUT);
  pinMode(nunPin3, OUTPUT);
  pinMode(nunPin4, OUTPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  digitalWrite(nunPin2, HIGH);   // both nunchucks must start out high
  digitalWrite(nunPin1, LOW);   //in order to initiate both at once
  digitalWrite(nunPin3, LOW);   // both nunchucks must start out high
  digitalWrite(nunPin4, LOW);   //in order to initiate both at once
  
  pinMode(17,OUTPUT);    //
  digitalWrite(17,HIGH);
  pinMode(16,OUTPUT);    //
  digitalWrite(16,LOW);
////////////////////////////////////// gas sensor calibration phase /////////////////////////
  Ro = MQCalibration(MQ_PIN);  //Calibrating the sensor. Please make sure the sensor is in clean air         
  delay(3000);
}


void
change_sensor()      // Swaps between nunchucks
{
  if (state == 1)
  {
    digitalWrite(nunPin1, HIGH);
    digitalWrite(nunPin2, LOW);
    digitalWrite(nunPin3, LOW);
    digitalWrite(nunPin4, LOW);
    //state = state + 1;
  }
  else
  {
    if (state == 2)
    {
      digitalWrite(nunPin1, LOW);
      digitalWrite(nunPin2, HIGH);
      digitalWrite(nunPin3, LOW);
      digitalWrite(nunPin4, LOW);
      //state = state + 1;
    }
    else
    {
        if (state == 3)
        {
          digitalWrite(nunPin1, LOW);
          digitalWrite(nunPin2, LOW);
          digitalWrite(nunPin3, HIGH);
          digitalWrite(nunPin4, LOW);
          //state = state + 1; 
        }
        else
        {
          if(state == 4)
          {
                digitalWrite(nunPin1, LOW);
                digitalWrite(nunPin2, LOW);
                digitalWrite(nunPin3, LOW);
                digitalWrite(nunPin4, HIGH);
                //state = 1; 
          }
        }
    }
  }
}

void readDHT11(){
    // Leemos la humedad relativa
    DHT11data[0] = dht.readHumidity();
    // Leemos la temperatura en grados centígrados (por defecto)
    DHT11data[1] = dht.readTemperature();
    // Leemos la temperatura en grados Fahreheit
    DHT11data[2] = dht.readTemperature(true);
   
    // Comprobamos si ha habido algún error en la lectura
    if (isnan(DHT11data[0]) || isnan(DHT11data[1]) || isnan(DHT11data[2])) {
      Serial.println("Error obteniendo los datos del sensor DHT11");
      return;
    }
   
    // Calcular el índice de calor en Fahreheit
    DHT11data[3] = dht.computeHeatIndex(DHT11data[2], DHT11data[0]);
    // Calcular el índice de calor en grados centígrados
    DHT11data[4] = dht.computeHeatIndex(DHT11data[1], DHT11data[0], false);
   
}

void outputDHT11_csv() {
    //Serial.print("Humedad: ");
    Serial.print(DHT11data[0]);
    Serial.print(",");
    //Serial.print("Temperatura: ");
    Serial.println(DHT11data[1]);
    //Serial.print(" *C ");
    //Serial.print(DHT11data[2]);
    //Serial.print(" *F\t");
    //Serial.print("Indice de calor: ");
    //Serial.print(DHT11data[3]);
    //Serial.print(" *C ");
    //Serial.print(DHT11data[4]);
    //Serial.println(" *F"); 
}

void output_csv() {
  for (int i = 0; i < 9; i++) {
    Serial.print(tdata[i]);
    if (i < 8) {
      Serial.print(",");
    } 
    else {
      Serial.println();
    }
  }
}
 
void output_json() {
  Serial.println("{");
  Serial.print("\"PTAT\":");
  Serial.print(tdata[0]);
  Serial.println(",");
  Serial.print("\"TEMP\":[");
  for (int i = 1; i < 7; i++) {
    Serial.print(tdata[i]);
    Serial.print(",");
  }
  Serial.print(tdata[15]);
  Serial.println("]}");
}
  /////////////////////////////////////////////////////////////// gas sensor functions///////////////////////////////////
void readGasSensor()
{
  long iPPM_LPG = 0;
  long iPPM_CO = 0;
  long iPPM_Smoke = 0;

  iPPM_LPG = MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_LPG);
  iPPM_CO = MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_CO);
  iPPM_Smoke = MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_SMOKE);
  
  Serial.print("LPG: ");
  Serial.print(iPPM_LPG);
  Serial.println(" ppm");   
   

  Serial.print("CO: ");
  Serial.print(iPPM_CO);
  Serial.println(" ppm");    

  Serial.print("Smoke: ");
  Serial.print(iPPM_Smoke);
  Serial.println(" ppm");  

}

float MQResistanceCalculation(int raw_adc)
{
  return ( ((float)RL_VALUE*(1023-raw_adc)/raw_adc));
}

float MQCalibration(int mq_pin)
{
  int i;
  float val=0;

  for (i=0;i<CALIBARAION_SAMPLE_TIMES;i++) {            //take multiple samples
    val += MQResistanceCalculation(analogRead(mq_pin));
    delay(CALIBRATION_SAMPLE_INTERVAL);
  }
  val = val/CALIBARAION_SAMPLE_TIMES;                   //calculate the average value
  val = val/RO_CLEAN_AIR_FACTOR;                        //divided by RO_CLEAN_AIR_FACTOR yields the Ro                                        
  return val;                                                      //according to the chart in the datasheet 

} 

float MQRead(int mq_pin)
{
  int i;
  float rs=0;
 
  for (i=0;i<READ_SAMPLE_TIMES;i++) {
    rs += MQResistanceCalculation(analogRead(mq_pin));
    delay(READ_SAMPLE_INTERVAL);
  }
 
  rs = rs/READ_SAMPLE_TIMES;
 
  return rs;  
}

long MQGetGasPercentage(float rs_ro_ratio, int gas_id)
{
  if ( gas_id == GAS_LPG ) {
     return MQGetPercentage(rs_ro_ratio,LPGCurve);
  } else if ( gas_id == GAS_CO ) {
     return MQGetPercentage(rs_ro_ratio,COCurve);
  } else if ( gas_id == GAS_SMOKE ) {
     return MQGetPercentage(rs_ro_ratio,SmokeCurve);
  }    
 
  return 0;
}
 
long  MQGetPercentage(float rs_ro_ratio, float *pcurve)
{
  return (pow(10,( ((log(rs_ro_ratio)-pcurve[1])/pcurve[2]) + pcurve[0])));
} 
////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop()
{
  int i;
 
  //if (Serial.available() > 0) {     // 
    //int inByte = Serial.read();     // 
    if (true) {         //
 
      Wire.beginTransmission(D6T_addr);
      Wire.write(D6T_cmd);
      Wire.endTransmission();
 
      if (WireExt.beginReception(D6T_addr) >= 0) {
        i = 0;
        for (i = 0; i < 19; i++) {
          rbuf[i] = WireExt.get_byte();
        }
        WireExt.endReception();
 
        t_PTAT = (rbuf[0]+(rbuf[1]<<8))*0.1;
        for (i = 0; i < 8; i++) {
          tdata[i]=(rbuf[(i*2+2)]+(rbuf[(i*2+3)]<<8))*0.1;
        }
        
        if (state == 1)
        {
          tdata[8] = 1;
        }
        
        if (state == 2)
        {
          tdata[8] = 2;
        }
        
        if (state == 3)
        {
          tdata[8] = 3;
        }
        
        if (state == 4)
        {
          tdata[8] = 4;
        }        
        if (sendFlag)
        {
          output_csv();  //
          sendFlag = 0;
        }
      }
    }

  //}
  //delay(3000);
  //delay(0.05);

  Serial.flush();
  state2 = 0;
  state2 = Serial.available();
  //Serial.print(state2);
  serialIn = Serial.read();
  //Serial.print(serialIn);
  //Serial.print(",");
  state2 = int(serialIn);
  //Serial.print(state2);
  
  if (state2 == 49)
  {
    state = 1;
    change_sensor();
    sendFlag = 1;
  }
    if (state2 == 50)
  {
    state = 2;
    change_sensor();
    sendFlag = 1;
  }
    if (state2 == 51)
  {
    state = 3;
    change_sensor();
    sendFlag = 1;
  }
  if (state2 == 52)
  {
    state = 4;
    change_sensor();
    sendFlag = 1;
  }
////////////////////////////////////////////////////////////
  //10 segundos entre medidas
 unsigned long currentMillis = millis();
  if (state2 == 53/*currentMillis - previousMillis >= interval*/) {
    //previousMillis = currentMillis;
    //request for humidity and temperature data
    readDHT11();
    outputDHT11_csv();
  }

  if (state == 54) 
  {
    //request for gas sensor readings
    readGasSensor();
  }
 
}

 

