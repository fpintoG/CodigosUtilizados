#include <WireExt.h>


#include "WireExt.h"
#include <Wire.h>

#define D6T_addr 0x0A
#define D6T_cmd 0x4C


#include "WiFiEsp.h"

// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
#include <NeoSWSerial.h>
NeoSWSerial Serial1( 6, 7 );
#endif

char ssid[] = "-_-";            // your network SSID (name)
char pass[] = "royale0059";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status
char server[] = "200.14.68.107";

// Initialize the Ethernet client object
WiFiEspClient client;

unsigned long actualConnectionTime = 0; 
unsigned long lastConnectionTime = 0;         // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 300000; // delay between updates, in milliseconds


///////////////////humidity and temperature sensor//////////////////////
#include <DHT.h>
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
//unsigned long previousMillis = 0;
//const long interval = 10000;
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



void setup()
{
  Wire.begin();
  dht.begin();
  // initialize serial for debugging
  Serial.begin(9600);
  Serial.flush();
  // initialize serial for ESP module
  Serial1.begin(9600);
  // initialize ESP module
  WiFi.init(&Serial1);

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    //Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  // you're connected now, so print out the data
  //Serial.println("You're connected to the network");

  //printWifiStatus();

  Serial.println();
  Serial.println("Starting connection to server...");
//  // if you get a connection, report back via serial
 
  Ro = MQCalibration(MQ_PIN);  //Calibrating the sensor. Please make sure the sensor is in clean air         
  delay(3000);
}

void loop()
{
  //////////////////////////////reading GAS//////////////////////
  long iPPM_LPG = 0;
  long iPPM_CO = 0;
  long iPPM_Smoke = 0;

  iPPM_LPG = MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_LPG);
  iPPM_CO = MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_CO);
  iPPM_Smoke = MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_SMOKE);
  /////////////////////////////////////////////////////////////////
  //////////////////////////////////////////reading DHT////////////
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  /////////////////////////////////////////////////////////////////
  actualConnectionTime = millis();
  if (actualConnectionTime  - lastConnectionTime > postingInterval) {
    lastConnectionTime = actualConnectionTime;
    sendToServerDHT(temperature, humidity);
  }
      
  // if there are incoming bytes available
  // from the server, read them and print them
  while (client.available()) {
    char c = client.read();
    //Serial.write(c);
  }

  // if the server's disconnected, stop the client
  if (!client.connected()) {
      delay(100);
      Serial.println();
      //Serial.println("Disconnecting from server...");
      client.stop();
  
    while ( status != WL_CONNECTED) {
      Serial.print("Attempting to connect to WPA SSID: ");
      Serial.println(ssid);
      // Connect to WPA/WPA2 network
      status = WiFi.begin(ssid, pass);
      Serial.println("Trying to reconnect...");    
    }

  }
}

void sendToServerGAS(long iPPM_LPG, long iPPM_CO, long iPPM_Smoke)
{
  if (client.connect(server, 80)) {
    Serial.println();
    Serial.println("Starting connection to server...");    
    Serial.println("Connected to server");
    // Make a HTTP request
    //String content = "{\"valor\":\""+String(2345)+"\"}";
    String content = String(iPPM_LPG)+';'+String(iPPM_CO)+';'+String(iPPM_Smoke);
    client.println("POST /modulotomas.php?valor=" + String(content)+ " HTTP/1.1");
    client.println("Host: 200.14.68.107:80");
    client.println("Accept: */*");
    client.println("Content-Length: " + String(content.length()));
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println();
    client.println(content);
  }  
}

void sendToServerDHT(float temperature, float humidity)
{
  if (client.connect(server, 80)) {
    Serial.println();
    Serial.println("Starting connection to server...");    
    Serial.println("Connected to server");
    // Make a HTTP request
    //String content = "{\"valor\":\""+String(2345)+"\"}";
    String content = String(temperature)+';'+String(humidity);
    client.println("POST /modulotomas.php?valor=" + String(content)+ " HTTP/1.1");
    client.println("Host: 200.14.68.107:80");
    client.println("Accept: */*");
    client.println("Content-Length: " + String(content.length()));
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println();
    client.println(content);
  }  
}

// this method makes a HTTP connection to the server
void httpRequest(float temperature, float humidity)
{
  Serial.println();
    
  // close any connection before send a new request
  // this will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection
  if (client.connect(server, 80)) {
    Serial.println("Connecting...");
    
    // send the HTTP PUT request
    String content = String(temperature)+';'+String(humidity);
    client.println("GET /modulotomas.php?valor=" + String(content)+ " HTTP/1.1");
    client.println("Host: 200.14.68.107:80");
    client.println("Accept: */*");
    client.println("Connection: close");
    client.println();

    // note the time that the connection was made
//    lastConnectionTime = millis();
  }
  else {
    // if you couldn't make a connection
    Serial.println("Connection failed");
  }
  Serial.println("Closed");
}


void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
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
//////////////////////////////////////////////////////////////////////////////////
