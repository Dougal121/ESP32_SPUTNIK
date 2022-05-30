/*
 * Compile for WEMOS LOLIN32
*/
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <WiFiUDP.h>
#include <TimeLib.h>
#include <Wire.h>
#include <EEPROM.h>
#include <stdio.h>
#include <Update.h>
#include <SPI.h>
#include <SD.h>
#include <HTTPClient.h>
#include <rom/rtc.h>
#include "StaticPages.h"
#include <Adafruit_NeoPixel.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "ds3231.h"


#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  30       /* Time ESP32 will go to sleep (in seconds) */
RTC_DATA_ATTR int bootCount = 0;


#define MYVER 0x12435678     // change this if you change the structures that hold data that way it will force a "backinthebox" to get safe and sane values from eeprom
#define BUFF_MAX 48

#define DIN_PIN1 26
#define DIN_PIN2 25
#define DIN_PIN3 17
#define DIN_PIN4 16
#define DIN_PIN5 27
#define DIN_PIN6 14

#define NUM_LEDS 10 // 60
#define DEFBRIGHTNESS 50

#define MAX_STRIPS 6
#define MAX_TEMP_SENSOR 2

#define ONE_WIRE_BUS 12   // gpio 2  
#define TEMPERATURE_PRECISION 12

#define MAX_THERMOMETERS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);  // Pass our oneWire reference to Dallas Temperature.
DeviceAddress Thermometer[MAX_THERMOMETERS];         // arrays to hold device addresses

const byte LED = 2 ; // BUILTIN_LED ;  // = 16  4 ? D4
const byte MAX_WIFI_TRIES = 60 ;
const int MAX_EEPROM = 2000 ;
const int PROG_BASE = 400 ;   // where the program specific information starts in eeprom (above GHKS maybe but be careful about creep)

byte rtc_sec ;
byte rtc_min ;
byte rtc_hour ;
byte rtc_fert_hour ;
float rtc_temp ;
long lScanCtr = 0 ;
long lScanLast = 0 ;
int iLogIndex ;                    
bool bConfig = false ;
bool hasRTC = false ;
bool hasSD = false;
bool bSaveReq = false ;
bool  bManSet = false  ;
bool bDoTimeUpdate = false ;
bool bPrevConnectionStatus = false ;
long lRebootCode = 0 ;
long iUploadPos = 0 ; 
uint8_t rtc_status ;
struct ts tc;
time_t tGoodTime ;   // try and remember the time if you root ;
time_t lLastGoodUpload = 0 ;
unsigned long lTimeNext = 0 ;     // next network retry
IPAddress DestinationIP ;     // (0,0,0,0)
unsigned int DestinationPortTx = 8052 ;
long  MyCheckSum ;
long  MyTestSum ;
long lTimePrev ;
long lTimePrev2 ;
long lTimeNextSample ;
long lLogIndex ; // which sample we is current one
uint64_t chipid;
long lMinUpTime = 0 ;
int  httpCode = 0 ; 


IPAddress MyIP ;
IPAddress MyIPC  ;
const int NTP_PACKET_SIZE = 48;       // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[NTP_PACKET_SIZE];   // buffer to hold incoming and outgoing packets
char buff[BUFF_MAX];
char buff2[BUFF_MAX];

char cssid[32] = {"LightNow_XXXXXXXX\0"} ;
char *host = "LightNow_00000000\0";                // overwrite these later with correct chip ID
char Toleo[10] = {"Ver 1.0\0"}  ;

//Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, DIN_PIN1, NEO_RGBW + NEO_KHZ800);

int NUM_PIXELS[MAX_STRIPS] = { 60 , 60 , 60 , 60 , 60 , 40 } ; // 60 leds per arm on the pentagon
Adafruit_NeoPixel strip[MAX_STRIPS] { Adafruit_NeoPixel(NUM_PIXELS[0], DIN_PIN1 , NEO_RGBW + NEO_KHZ800),Adafruit_NeoPixel(NUM_PIXELS[1], DIN_PIN2 , NEO_RGBW + NEO_KHZ800) ,Adafruit_NeoPixel(NUM_PIXELS[2], DIN_PIN3 , NEO_RGBW + NEO_KHZ800),Adafruit_NeoPixel(NUM_PIXELS[3], DIN_PIN4 , NEO_RGBW + NEO_KHZ800),Adafruit_NeoPixel(NUM_PIXELS[4], DIN_PIN5 , NEO_RGBW + NEO_KHZ800),Adafruit_NeoPixel(NUM_PIXELS[5], DIN_PIN6 , NEO_RGBW + NEO_KHZ800)  };

byte neopix_gamma[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

typedef struct __attribute__((__packed__)) {     // eeprom stuff
  unsigned int localPort = 2390;          // 2 local port to listen for NTP UDP packets
  unsigned int localPortCtrl = 8666;      // 4 local port to listen for Control UDP packets
  unsigned int RemotePortCtrl = 8664;     // 6 local port to listen for Control UDP packets
  long lNodeAddress ;                     // 10 
  float fTimeZone ;                       // 14 
  char RCIP[16] ;                         // (192,168,2,255)  30
  char NodeName[32] ;                     // 62 
  char nssid[24] ;                        // 86  
  char npassword[32] ;                    // 108
  time_t AutoOff_t ;                      // 112     auto off until time > this date   
  uint8_t lDisplayOptions  ;              // 113 
  uint8_t lNetworkOptions  ;              // 114 
  uint8_t lSpare1  ;                      // 115 
  uint8_t lSpare2  ;                      // 116 
  char timeServer[24] ;                   // 140   = {"au.pool.ntp.org\0"}
  char cpassword[32] ;                    // 172
  long lVersion  ;                        // 176
  IPAddress IPStatic ;                    // (192,168,0,123)   
  IPAddress IPGateway ;                   // (192,168,0,1)    
  IPAddress IPMask ;                      // (255,255,255,0)   
  IPAddress IPDNS ;                       // (192,168,0,15)   
  long SelfReBoot ;                       // 196    
  IPAddress IPPing  ;                     // 200
  long PingMax ;                          // 204
  long PingFreq ;                         // 208
  float latitude;                         // 212
  float longitude;                        // 216 
  char apikey[40] ;                       // 172
  char servername[32] ;
} general_housekeeping_stuff_t ;          // 

general_housekeeping_stuff_t ghks ;



typedef struct __attribute__((__packed__)) {     // eeprom stuff
  struct ts tc;            //
  float ha ;
  float sunX ;
  float sunrise ;
  float sunset ;
  float tst ;
  float solar_az_deg;      //
  float solar_el_deg;      //
  int   iDayNight ;          //
  float decl ;
  float eqtime ;
} Solar_App_stuff_t ;          

Solar_App_stuff_t SolarApp  ;

typedef struct  __attribute__((__packed__)){
  uint8_t iRed ;
  uint8_t iGreen ;
  uint8_t iBlue ;
  uint8_t iWhite ;        // default WRGB on turn on 
  uint8_t iStart ;        // position inside
  uint8_t iStop ;         // position outside
  uint8_t iGrad ;         // gradient
  uint8_t iBright ;
} Segment_t ;

#define MAX_SEGMENTS 7
#define MAX_SEGSETTINGS 8
#define MAX_FORECAST_DAYS 4

typedef struct  __attribute__((__packed__)){
  Segment_t seg[MAX_SEGMENTS] ;
  char      Description[16] ;  // description of the pattern
} Segment_array_t ;

typedef struct __attribute__((__packed__)) {     // eeprom stuff
  uint8_t bSegments ;     // five arms , one backlight , one downlight ring , alarm indicators -  8 items
  uint8_t iMode ;         // 0 normal , 1 christmas lights   etc default at turn on
  uint8_t iDefSegIndex ;  // index of the default settings to usse at turn on  
  uint8_t iCurSegIndex ;  // index of the default settings to usse at turn on  
  Segment_array_t SegMem[MAX_SEGSETTINGS] ;  // all the settings in an arragy
  int iAutoOn ;
  int iAutoOOff ;
  uint8_t bAutoDays ;  
  uint8_t bQuite ;   // reduce power so you don't need the fan
} LightNow_App_stuff_t ;          

LightNow_App_stuff_t lnas ;

typedef struct __attribute__((__packed__)) {     // memory stuff
  uint16_t wSendEmail ;
  float fTemp[MAX_TEMP_SENSOR];
  float fTempPrev[MAX_TEMP_SENSOR]; 
  uint8_t sensor[MAX_TEMP_SENSOR] ;               // which sensor is where  
  bool  bDoReboot = false ;
  bool  bDoGetWeather = false ;
  bool  bMadeWeatherDecision = false ;
  long  iPingTime = -1 ;
  time_t WForecastDate;
  int   WClouds[MAX_FORECAST_DAYS];
  float WMaxTemp[MAX_FORECAST_DAYS];
  float WMinTemp[MAX_FORECAST_DAYS];
  int   iLasthttpResponseCode;
} LightNow_App_memory_stuff_t ;         

LightNow_App_memory_stuff_t lnms ;



WiFiUDP ntpudp;
WiFiUDP scanudp;
HTTPClient http;
WebServer server(80);

void setup() {
  int i , k , j = 0;
  lMinUpTime = 0 ;

  chipid = ESP.getEfuseMac(); //The chip ID is essentially its MAC address(length: 6 bytes).
  lRebootCode = random(1,+2147483640) ;  // want to change it straight away
  
  Serial.begin(115200);
  Serial.setDebugOutput(true);  
  Serial.println("");          // new line after the startup burst
 
  pinMode(LED,OUTPUT);  //  D4 builtin LED
  EEPROM.begin(MAX_EEPROM);
  LoadParamsFromEEPROM(true);  

  if ( MYVER != ghks.lVersion ) {
    //  if ( false ) {
    Serial.println("Loading memory defaults...");
    BackInTheBoxMemory();         // load defaults if blank memory detected but dont save user can still restore from eeprom
    Serial.println("Loaded memory defaults...");
    delay(2000);
  }  

  WiFi.disconnect();
  Serial.println("Configuring soft access point...");
  WiFi.mode(WIFI_AP_STA);  // we are having our cake and eating it eee har

//  WiFi.setTxPower(WIFI_POWER_19_5dBm);
//  WiFi.setTxPower(WIFI_POWER_2dBm);  

  sprintf(cssid, "Configure_%08X\0", chipid);
  if ( cssid[0] == 0 || cssid[1] == 0 ) {  // pick a default setup ssid if none
    sprintf(ghks.cpassword, "\0");
  }
  MyIPC = IPAddress (192, 168, 5 + ((uint8_t)chipid & 0x7f ) , 1);
  WiFi.softAPConfig(MyIPC, MyIPC, IPAddress (255, 255, 255 , 0));
  Serial.println("Starting access point...");
  Serial.print("SSID: ");
  Serial.println(cssid);
  Serial.print("Password: >");
  Serial.print(ghks.cpassword);
  Serial.println("< " + String(ghks.cpassword[0]));
  if (( ghks.cpassword[0] == 0 ) || ( ghks.cpassword[0] == 0xff)) {
    WiFi.softAP((char*)cssid);                   // no passowrd
  } else {
    WiFi.softAP((char*)cssid, (char*) ghks.cpassword);
  }
  MyIPC = WiFi.softAPIP();  // get back the address to verify what happened
  Serial.print("Soft AP IP address: ");
  snprintf(buff, BUFF_MAX, ">> IP %03u.%03u.%03u.%03u <<", MyIPC[0], MyIPC[1], MyIPC[2], MyIPC[3]);
  Serial.println(buff);

  bConfig = false ;   // are we in factory configuratin mode
  if ( ghks.lNetworkOptions != 0 ) {
    WiFi.config(ghks.IPStatic, ghks.IPGateway, ghks.IPMask, ghks.IPDNS );
  }
  if ( ghks.npassword[0] == 0 ) {
    WiFi.begin((char*)ghks.nssid);                    // connect to unencrypted access point
  } else {
    WiFi.begin((char*)ghks.nssid, (char*)ghks.npassword);  // connect to access point with encryption
  }
  Serial.print("Client SSID: ");
  Serial.println(ghks.nssid);
  Serial.print("Password: >");
  Serial.print(ghks.npassword);
  while (( WiFi.status() != WL_CONNECTED ) && ( j < MAX_WIFI_TRIES )) {
    j = j + 1 ;
    delay(500);
    digitalWrite(LED, !digitalRead(LED));
  }
  if ( j >= MAX_WIFI_TRIES ) {
    bConfig = true ;
    WiFi.disconnect();
  } else {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    MyIP =  WiFi.localIP() ;
    snprintf(buff, BUFF_MAX, "%03u.%03u.%03u.%03u", MyIP[0], MyIP[1], MyIP[2], MyIP[3]);
    Serial.println(buff);

  }
  if (ghks.localPortCtrl == ghks.localPort ) {            // bump the NTP port up if they ar the same
    ghks.localPort++ ;
  }

  server.on("/", handleRoot);
  server.on("/setup", handleRoot);
  server.on("/scan", i2cScan);
  server.on("/stime", handleRoot);
  server.on("/info", handleInfo);
  server.on("/pixel", pixelscope);
  server.on("/eeprom", DisplayEEPROM);
  server.on("/backup", HTTP_GET , handleBackup);
  server.on("/backup.txt", HTTP_GET , handleBackup);
  server.on("/backup.txt", HTTP_POST,  handleRoot, handleFileUpload);
  server.on("/login", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
    Serial.printf("Display Login Page");
  });
  server.on("/update", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", updatePage);
  });
  server.on("/update", HTTP_POST, []() {   //handling uploading firmware file
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {

      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {   // flashing firmware to ESP
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });

  server.onNotFound(handleNotFound);
  server.begin();
  if ( year(now()) < 2020 ){
    if( year(tGoodTime) > 2019 ){
      setTime(tGoodTime) ;  // try and recuse a reasonable time stamp from memory just in case the NTP or network fails
    }
  }
  tc.mon = 0 ;
  tc.wday = 0 ;

  randomSeed(now());                       // now we prolly have a good time setting use this to roll the dice for reboot code
  lRebootCode = random(1, +2147483640) ;
  if (WiFi.isConnected())  {
    sendNTPpacket(ghks.timeServer); // send an NTP packet to a time server  once and hour  
  }

  sensors.begin(); // Start up the Dallas library

  Serial.print("Parasite power is: ");     // report parasite power requirements
  if (sensors.isParasitePowerMode()) 
    Serial.println("ON");
  else 
    Serial.println("OFF");

  for (i = 0 ; i < MAX_TEMP_SENSOR ; i++){
    if (sensors.getAddress(Thermometer[i], i)) {
        printAddress(Thermometer[i]);
        sensors.setResolution(Thermometer[i], TEMPERATURE_PRECISION);
        Serial.print(" Device "+String(i)+" Resolution: ");
        Serial.print(sensors.getResolution(Thermometer[i]), DEC);
        Serial.print(" ");
        sensors.requestTemperatures();
        printTemperature(Thermometer[i]);
        Serial.println();
    }
    else
      Serial.println("Unable to find address for Device "+String(i));
  }  
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" Temperature devices found.");


  Serial.println("Start strip setup");
  lnas.iCurSegIndex = lnas.iDefSegIndex ;
  for (i = 0 ; i < MAX_STRIPS ; i++){
    Serial.print(i);
    strip[i].setBrightness(lnas.SegMem[lnas.iDefSegIndex].seg[i].iBright);
    strip[i].begin();
    strip[i].show(); // Initialize all pixels to 'off'
    Serial.print(".");
  }  
  Serial.println("End of Strip Setup");
}

void loop() {
int i , j , k ;
long lTD ;

  server.handleClient();
  if (second() > 4 ) {
    if ( ntpudp.parsePacket() ) {
      processNTPpacket();
    }
  }
  lScanCtr++ ;
  if (second() != rtc_sec) {                                // do onlyonce a second
    digitalWrite(LED,!digitalRead(LED));

    rtc_sec = second();
    lScanLast = lScanCtr ;
    lScanCtr = 0 ;

    for ( i = 0 ; i < sensors.getDeviceCount() ; i++ ){
      lnms.fTemp[i] = sensors.getTempC(Thermometer[lnms.sensor[i]]) ;
    }
  }

  if (rtc_hour != hour()) {
  }



  
  // Some example procedures showing how to display to the pixels:
//  colorWipe(strip.Color(255, 0, 0), 50); // Red
//  colorWipe(strip.Color(0, 255, 0), 50); // Green
//  colorWipe(strip.Color(0, 0, 255), 50); // Blue
//  colorWipe(strip.Color(0, 0, 0, 255), 50); // White

//  whiteOverRainbow(20,75,5);  

//  pulseWhite(5); 

  // fullWhite();
  // delay(2000);

//  rainbowFade2White(3,3,1);
  for (i = 0 ; i < MAX_STRIPS ; i++){  // update all the strips
    strip[i].setBrightness(lnas.SegMem[lnas.iCurSegIndex].seg[i].iBright);
    strip[i].show(); 
  }  

  snprintf(buff, BUFF_MAX, "%d/%02d/%02d %02d:%02d:%02d", year(), month(), day() , hour(), minute(), second());
  if ( !bPrevConnectionStatus && WiFi.isConnected() ){
    Serial.println(String(buff )+ " WiFi Reconnected OK...");  
    MyIP =  WiFi.localIP() ;
  }
  if (!WiFi.isConnected())  {
    lTD = (long)lTimeNext-(long) millis() ;
    if (( abs(lTD)>40000)||(bPrevConnectionStatus)){ // trying to get roll over protection and a 30 second retry
      lTimeNext = millis() - 1 ;
/*      Serial.print(millis());
      Serial.print(" ");
      Serial.print(lTimeNext);
      Serial.print(" ");
      Serial.println(abs(lTD));*/
    }
    bPrevConnectionStatus = false;
    if ( lTimeNext < millis() ){
      Serial.println(String(buff )+ " Trying to reconnect WiFi ");
      WiFi.disconnect(false);
//      Serial.println("Connecting to WiFi...");
      WiFi.mode(WIFI_AP_STA);
      if ( ghks.lNetworkOptions != 0 ) {            // use ixed IP
        WiFi.config(ghks.IPStatic, ghks.IPGateway, ghks.IPMask, ghks.IPDNS );
      }
      if ( ghks.npassword[0] == 0 ) {
        WiFi.begin((char*)ghks.nssid);                    // connect to unencrypted access point
      } else {
        WiFi.begin((char*)ghks.nssid, (char*)ghks.npassword);  // connect to access point with encryption
      }
      lTimeNext = millis() + 30000 ;
    }
  }else{
    bPrevConnectionStatus = true ;
  }  
}                                          //  ###### BOTTOM OF LOOP   #################

// Fill the dots one after the other with a color
void colorWipe(uint8_t s,uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip[s].numPixels(); i++) {
    strip[s].setPixelColor(i, c);
    strip[s].show();
    delay(wait);
  }
}

void pulseWhite(uint8_t s,uint8_t wait) {
  for(int j = 0; j < 256 ; j++){
      for(uint16_t i=0; i<strip[s].numPixels(); i++) {
          strip[s].setPixelColor(i, strip[s].Color(0,0,0, neopix_gamma[j] ) );
        }
        delay(wait);
        strip[s].show();
      }

  for(int j = 255; j >= 0 ; j--){
      for(uint16_t i=0; i<strip[s].numPixels(); i++) {
          strip[s].setPixelColor(i, strip[s].Color(0,0,0, neopix_gamma[j] ) );
        }
        delay(wait);
        strip[s].show();
      }
}


void rainbowFade2White(uint8_t s, uint8_t wait, int rainbowLoops, int whiteLoops) {
  float fadeMax = 100.0;
  int fadeVal = 0;
  uint32_t wheelVal;
  int redVal, greenVal, blueVal;

  for(int k = 0 ; k < rainbowLoops ; k ++){
    for(int j=0; j<256; j++) { // 5 cycles of all colors on wheel
      for(int i=0; i< strip[s].numPixels(); i++) {
        wheelVal = Wheel(s,((i * 256 / strip[s].numPixels()) + j) & 255);
        redVal = red(wheelVal) * float(fadeVal/fadeMax);
        greenVal = green(wheelVal) * float(fadeVal/fadeMax);
        blueVal = blue(wheelVal) * float(fadeVal/fadeMax);
        strip[s].setPixelColor( i, strip[s].Color( redVal, greenVal, blueVal ) );
      }

      //First loop, fade in!
      if(k == 0 && fadeVal < fadeMax-1) {
          fadeVal++;
      }

      //Last loop, fade out!
      else if(k == rainbowLoops - 1 && j > 255 - fadeMax ){
          fadeVal--;
      }
      strip[s].show();
      delay(wait);
    }  
  }

  delay(500);

  for(int k = 0 ; k < whiteLoops ; k ++){
    for(int j = 0; j < 256 ; j++){
        for(uint16_t i=0; i < strip[s].numPixels(); i++) {
            strip[s].setPixelColor(i, strip[s].Color(0,0,0, neopix_gamma[j] ) );
        }
        strip[s].show();
    }
    delay(2000);
    for(int j = 255; j >= 0 ; j--){
        for(uint16_t i=0; i < strip[s].numPixels(); i++) {
            strip[s].setPixelColor(i, strip[s].Color(0,0,0, neopix_gamma[j] ) );
        }
        strip[s].show();
     }
  }
  delay(500);
}

void whiteOverRainbow(uint8_t s, uint8_t wait, uint8_t whiteSpeed, uint8_t whiteLength ) {
  
  if(whiteLength >= strip[s].numPixels()) whiteLength = strip[s].numPixels() - 1;

  int head = whiteLength - 1;
  int tail = 0;

  int loops = 3;
  int loopNum = 0;

  static unsigned long lastTime = 0;


  while(true){
    for(int j=0; j<256; j++) {
      for(uint16_t i=0; i<strip[s].numPixels(); i++) {
        if((i >= tail && i <= head) || (tail > head && i >= tail) || (tail > head && i <= head) ){
          strip[s].setPixelColor(i, strip[s].Color(0,0,0, 255 ) );
        }
        else{
          strip[s].setPixelColor(i, Wheel(s,((i * 256 / strip[s].numPixels()) + j) & 255));
        }
        
      }

      if(millis() - lastTime > whiteSpeed) {
        head++;
        tail++;
        if(head == strip[s].numPixels()){
          loopNum++;
        }
        lastTime = millis();
      }

      if(loopNum == loops) return;
    
      head%=strip[s].numPixels();
      tail%=strip[s].numPixels();
        strip[s].show();
        delay(wait);
    }
  }  
}


void fullWhite(uint8_t s) {
    for(uint16_t i=0; i<strip[s].numPixels(); i++) {
        strip[s].setPixelColor(i, strip[s].Color(0,0,0, 255 ) );
    }
    strip[s].show();
}


// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t s,uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256 * 5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip[s].numPixels(); i++) {
      strip[s].setPixelColor(i, Wheel(s,((i * 256 / strip[s].numPixels()) + j) & 255));
    }
    strip[s].show();
    delay(wait);
  }
}

void rainbow(uint8_t s,uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip[s].numPixels(); i++) {
      strip[s].setPixelColor(i, Wheel(s,(i+j) & 255));
    }
    strip[s].show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(uint8_t s,byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip[s].Color(255 - WheelPos * 3, 0, WheelPos * 3,0);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip[s].Color(0, WheelPos * 3, 255 - WheelPos * 3,0);
  }
  WheelPos -= 170;
  return strip[s].Color(WheelPos * 3, 255 - WheelPos * 3, 0,0);
}

uint8_t red(uint32_t c) {
  return (c >> 16);
}
uint8_t green(uint32_t c) {
  return (c >> 8);
}
uint8_t blue(uint32_t c) {
  return (c);
}

