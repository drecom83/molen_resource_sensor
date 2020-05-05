#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)

//#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
//#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
//#include <ArduinoOTA.h>
//#include <EEPROM.h>               // used to store and read settings
#include "ota.h"
#include "settings.h"
#include "handleWebServer.h"
#include "handleHTTPClient.h"
#include "WiFiSettings.h"

//#include "webServer.h"  zodra alle webserver-zaken zijn verplaatst naar lib
// schema on https://wiki.wemos.cc/_media/products:d1:sch_d1_mini_v3.0.0.pdf

/*
const uint8 MAX_AXES = 32;        // Maximun number of axes for calculating the ratio
const uint8 MAX_WHEELS = 32;      // Maximum number of wheels for calculating the pulse factor
const char AXES_DELIMITER = '-';  // Delimiter for axes
const char WHEEL_DELIMITER = '.'; // Delimiter for connected wheels whithout an ax
const uint8 MAX_RATIO_ARGUMENT = 128; // Maximum length of ratioArgument
*/

// WIFI URL: http://192.168.4.1/ or http://molen.local/
/////////////////////
// Pin Definitions //
/////////////////////
// On a ESP8266-12 GPIO0 is used, physical name is pin D0
// On a ESP8266-12 GPIO5 is used, physical name is pin D5

const uint8_t IR_RECEIVE_1 = D5;    // Digital pin to read an incoming signal
const uint8_t IR_RECEIVE_2 = D6;    // Digital pin to read an incoming signal
const uint8_t OUTPUT_LED = D0;
const uint8_t IR_SEND = D1;         // switch for IR send LED. 0 = off, 1 = on

//const uint8_t DIM = 30;             // maximum acceleration and deceleration of viewPulsesPerMinute
//const uint32_t WAIT_PERIOD = 1000;  // wait period in the loop for energy saving, in milliseconds
const uint32_t RELAX_PERIOD = 2;   // wait period in the loop for letting the microcontroller come into a state.
                                    // Is also a small energy saving, in milliseconds
const uint32_t TOO_LONG = 60000;    // after this period the pulsesPerMinute = 0 (in milliseconds)
bool permissionToDetect = false;    // all sensors must have had a positive value 

uint32_t startPulse = millis();     // set the offset time for a passing a pulse
uint32_t pulsesPerMinute = 0;       // holds the value of pulses per minute
uint32_t revolutions = 0;           // holds the value of revolutions of the first axis, calculated with ratio
uint32_t viewPulsesPerMinute = 0;   // holds the value of ends per minute calculated with ratio

Settings settings = Settings();
Settings* pSettings = &settings;
//////////////////////
// WiFi Definitions //
//////////////////////


WiFiSettings wifiSettings = WiFiSettings(pSettings);
WiFiSettings* pWifiSettings = &wifiSettings;

//char ratioTestArgument[] = "4-72.99.33-80.24";

// Forward declaration
//void handleArguments();
void setupWiFi();
void showSettings();
void switchToAccessPoint();
//-void detectPulse();
//void setupArduinoOTA();
//void checkmDNS();

void initServer();

void sendExample();

void ICACHE_RAM_ATTR detectPulse();
void echoInterruptOn();
void echoInterruptOff();


/*
2^8 = 256
2^16 = 65536
2^32 = 4294967296
2^64 = 18446744073709551616
*/

// The SSE retry value could be influenced by timing delays
ESP8266WebServer server(80);
//WiFiManager wifiManager;
WiFiClient wifiClient;

MDNSResponder mdns;

void debugMessage(String message) {
  Serial.println(message);
}


// start Settings and EEPROM stuff
void saveSettings() {
  pSettings->saveSettings();
  showSettings();
}

void getSettings() {
  pSettings->getSettings();
  showSettings();
}

void eraseSettings() {
  pSettings->eraseSettings();
  pSettings->getSettings();   // otherwise the previous values of Settings are used
  showSettings();
}

void initSettings() {
  pSettings->initSettings();
  pSettings->getSettings();   // otherwise the previous values of Settings are used
  showSettings();
}
// end Settings and EEPROM stuff

void setupWiFi(){
  echoInterruptOff();  // to prevent error with Delay
  WiFi.mode(WIFI_AP);

  pSettings->allowSendingData(false);  // because there is no internetConnection

/*
  Serial.println("Sector size info:");
  Serial.println(SPI_FLASH_SEC_SIZE);
  Serial.println(FLASH_SECTOR_SIZE);
  pWifiSettings->setAccessPointSSID(String(ssid));
  pWifiSettings->setAccessPointPassword(String(pass));
  pWifiSettings->saveAuthorizationAccessPoint();
*/

  String myssid = pWifiSettings->readAccessPointSSID();
  if (myssid == "")
  {
    myssid = "ESP-" + WiFi.macAddress();
  }
  String mypass = pWifiSettings->readAccessPointPassword();
  /*
  Serial.println("myssid in class");
  Serial.println(pWifiSettings->getAccessPointSSID());
  Serial.println("password in class");
  Serial.println(pWifiSettings->getAccessPointPassword());
  Serial.println("adres van eerste karakter van accesspointer ssid");
  Serial.println(&myssid[0]);
  Serial.println("myssid in EEPROM");
  Serial.println(myssid);
  Serial.println(myssid[0]);
  Serial.println(myssid[1]);
  Serial.println(myssid[2]);
  Serial.println("password in EEPROM");
  Serial.println(mypass);
  */

  //WiFi.begin();  // alleen voor het maken van een connectie in station mode
  //WiFi.mode(WIFI_AP_STA);
  //IPAddress local_IP(10,0,0,61);
  //IPAddress gateway(10,0,0,1);
  //IPAddress subnet(255,255,255,0);
  IPAddress local_IP(192,168,4,1);
  IPAddress gateway(192,168,4,1);
  IPAddress subnet(255,255,255,0);


  Serial.print("Setting soft-AP ... ");
  //Serial.println(WiFi.softAP(ssid,pass,3,0) ? "Ready" : "Failed!");
  Serial.println(WiFi.softAP(myssid,mypass,3,0) ? "Ready" : "Failed!");
  Serial.print("Setting soft-AP configuration ... ");
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");
  Serial.print("Connecting to AP mode");
  /*
  if (WiFi.getMode() != WIFI_AP) {
    while (WiFi.reconnect() != true)
    {
      delay(500);
      Serial.print(".");
    }
  }
  Serial.println();
  */
  
  delay(500);
  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());
  Serial.println(WiFi.softAPmacAddress());

  echoInterruptOn();  // to prevent error with Delay

}

/*
// start WiFiManager
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entering configCallback");
  //setupWiFi();
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
}
*/
void setupWiFiManager () {
  bool networkConnected = false;
  echoInterruptOff();  // to prevent error with Delay
  //WiFi.mode(WIFI_STA);

  //IPAddress local_IP(10,0,0,55);
  //IPAddress gateway(10,0,0,1);
  //IPAddress subnet(255,255,255,0);

  String mynetworkssid = pWifiSettings->readNetworkSSID();
  if (mynetworkssid != "") {
    String mynetworkpass = pWifiSettings->readNetworkPassword();
    //WiFi.mode(WIFI_AP_STA);
    WiFi.mode(WIFI_STA);
    WiFi.begin(mynetworkssid, mynetworkpass); 

    //Serial.print("Setting soft station ... ");
    //Serial.println(WiFi.softAP(networkssid,networkpass,3,0) ? "Ready" : "Failed!");
    //Serial.print("Setting soft-AP configuration ... ");
    //Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");
    Serial.print("Connecting to a WiFi Network");
    int toomuch = 30;  //gives 30 seconds to connect to a Wifi network
    while ((WiFi.status() != WL_CONNECTED) && (toomuch > 0))
    {
      delay(1000);
      Serial.print(".");
      toomuch -=1;
    }
    if (toomuch > 0) {
      Serial.println();

      Serial.print("Connected, IP address: ");
      Serial.println("local ip address");
      Serial.println(WiFi.localIP());
      Serial.println(WiFi.gatewayIP());
      Serial.println(WiFi.macAddress());

      //Serial.print("Soft-AP IP address = ");
      //Serial.println(WiFi.softAPIP());
      //Serial.println(WiFi.softAPmacAddress());
    
      echoInterruptOn();  // to prevent error with Delay
      networkConnected = true;
      //* TODO: change this... it's working in the during development phase, in production it is remarked */
      //pSettings->allowSendingData(true);

      //dnsServer.reset(new DNSServer());
      }
    }
    if (networkConnected == false) {
    // no network found, fall back to access point
    Serial.println("no network SSID found/selected, fall back to AccessPoint mode");
    switchToAccessPoint();
  }
}


void resetWiFiManagerToFactoryDefaults () {
  echoInterruptOff();  // to prevent error with Delay

 // WiFi.disconnect(true);  // true argument should also erase ssid and password
  
  // https://www.pieterverhees.nl/sparklesagarbage/esp8266/130-difference-between-esp-reset-and-esp-restart
  Serial.println("try to disconnect works only when WiFi.begin() was successfully called");
  int toomuch = 2;
  while (toomuch > 0) {
    int getValue = WiFi.disconnect(true);
    if (getValue != 0) {
      toomuch = 0;
    }
    Serial.println(String(getValue));
    delay(3000);
    Serial.println("waited 3 seconds");
    toomuch -= 1;
  }

  //reset and try again, or maybe put it to deep sleep
  //ESP.reset();
  //delay(5000);

  //ESP.reset();   // or ESP.restart()? or just nothing?
  //ESP.restart();   // or ESP.restart()? or just nothing?
  //delay(5000);
  //hardwareReset();
  echoInterruptOn();  // to prevent error with Delay

}
// end WiFiManager


void switchToAccessPoint() {
  echoInterruptOff();  // to prevent error with Delay
  showSettings();
  server.close();
  resetWiFiManagerToFactoryDefaults();
  delay(pSettings->WAIT_PERIOD);
  setupWiFi();
  delay(pSettings->WAIT_PERIOD);

//setupArduinoOTA();  // geeft cause error, dus maar even weglaten
//delay(WAIT_PERIOD);

initServer();
// start domain name server check
//void checkmDNS() {
  //mdns.close();
 // while (mdns.begin("molen", WiFi.softAPIP())) {
 //   Serial.println("MDNS responder started");
 //   mdns.addService("http", "tcp", 80);
  //}
//}
// end domain name server check
  echoInterruptOn();  // to prevent error with Delay

}

void switchToNetwork() {
  echoInterruptOff();  // to prevent error with Delay
  showSettings();
  server.close();
  resetWiFiManagerToFactoryDefaults();
  delay(pSettings->WAIT_PERIOD);
  setupWiFiManager();

  delay(pSettings->WAIT_PERIOD);
  setupArduinoOTA();
  delay(pSettings->WAIT_PERIOD);
  //checkmDNS();
  delay(pSettings->WAIT_PERIOD);
  initServer();
  // start domain name server check
  ////void checkmDNS() {
  //mdns.enableArduino voor ota
  mdns.close();

  while (mdns.begin("molen", WiFi.localIP())) {
    Serial.println("MDNS responder started");
    mdns.addService("http", "tcp", 80);
  }
//}
// end domain name server check
  echoInterruptOn();  // to prevent error with Delay
}

// start domain name server check
//void checkmDNS() {
//  if (mdns.begin("molen", WiFi.localIP())) {
//    Serial.println("MDNS responder started");
//    mdns.addService("http", "tcp", 80);
//  }
//}
// end domain name server check


void writeResult(WiFiClient wifiClient, String result) {
  wifiClient.print(result);
  wifiClient.flush();
}

/* flashes PIN, unit is milliseconds (0-256) */
void flashPin(uint8_t pin, uint8_t ms) {
  digitalWrite(pin, HIGH);
  for (uint16_t i = 0; i <= ms; i++)
  {
    delayMicroseconds(250);   // delay in the loop could cause an exception (9) when using interrupts
    delayMicroseconds(250);   // delay in the loop could cause an exception (9) when using interrupts
    delayMicroseconds(250);   // delay in the loop could cause an exception (9) when using interrupts
    delayMicroseconds(250);   // delay in the loop could cause an exception (9) when using interrupts
  }
  digitalWrite(pin, LOW);
}

/*
uint32_t dimViewPulsesPerMinute( uint32_t previousViewPulsesPerminute)
// slows down the acceleration
// sets value in global viewPulsesPerMinute
{
  if (viewPulsesPerMinute - previousViewPulsesPerminute > DIM) {
    viewPulsesPerMinute = previousViewPulsesPerminute + DIM;
  }
  return viewPulsesPerMinute;
}
*/
void checkGlobalPulseInLoop() {
  // sets value to 0 after a period of time
  uint32_t elapsedTime;
  if (millis() > startPulse) {  // check for overflow
    
    elapsedTime = millis() - startPulse;
    if (elapsedTime * pSettings->ratio > TOO_LONG) {
      pulsesPerMinute  = 0;
      viewPulsesPerMinute = 0;
    }   
  }
}

void setGlobalPulsesPerMinute() {
  /*
  start = millis();
  Returns the number of milliseconds since the Arduino board began
  running the current program. This number will overflow (go back to zero),
  after approximately 50 days.
  */
  // use previous value if an overflow of millis() occurs,
  // it does not have to be too precise
  uint32_t elapsedTime;
  if (millis() > startPulse) {  // check for overflow
    elapsedTime = millis() - startPulse;

    // measuremens shorter then the delay time are invalid (with an extra 50 ms to be sure)
    //if (elapsedTime > START_PERIOD) {
      if (elapsedTime > 0) {
        // get duration to get 1 pulse
        //pulsesPerMinute = (uint32_t) round((60 * 1000)/elapsedTime);
        pulsesPerMinute = (uint32_t) round(60000 / elapsedTime);
      }
      else {
        pulsesPerMinute = 0;    // maybe slow movement, but rounded to 0
      }
   //}
    startPulse = millis();
  }
}

void delayInMillis(uint8_t ms)
{
  for (uint8_t i = 0; i <= ms; i++)
  {
    delayMicroseconds(250);   // delay in the loop could cause an exception (9) when using interrupts
    delayMicroseconds(250);   // delay in the loop could cause an exception (9) when using interrupts
    delayMicroseconds(250);   // delay in the loop could cause an exception (9) when using interrupts
    delayMicroseconds(250);   // delay in the loop could cause an exception (9) when using interrupts
  }
}

void ICACHE_RAM_ATTR detectPulse() {  // ICACHE_RAM_ATTR is voor interrupts
  // this function is called after a change of every sensor-value
  // wait until both sensors are true, then permissionToDetect = true
  // if both sensors are false and permissionToDetect == true then it counts as a valid pulse
  // after a valid pulse the value of permissionToDetect is set to false to start over again
  echoInterruptOff();
    // for energy savings a delay is added of n milliseconds
  delayInMillis(RELAX_PERIOD);


  uint8_t result = 0;
  if ( (digitalRead(IR_RECEIVE_1) == true) && 
      (digitalRead(IR_RECEIVE_2) == true) &&
      (permissionToDetect == false) )
  {
    permissionToDetect = true;
  }

  if ( (digitalRead(IR_RECEIVE_1) == false) && 
      (digitalRead(IR_RECEIVE_2) == false) && 
      (permissionToDetect == true) )
  {
    result = 1;
    permissionToDetect = false;  // start over again
    flashPin(OUTPUT_LED, 1);

    //debugMessage("detectpulse aangeroepen");
    //uint8_t result = getEchoResult();
    //debugMessage("er mag weer gemeten worden");
    pSettings->setCounter(pSettings->getCounter() + result);
    setGlobalPulsesPerMinute();
    // calculate with ratio
    if (pSettings->blades < 1) {
      pSettings->blades = 1;
    }
    // slow down acceleration or deceleration, part 1 of 2
    //uint32_t previousViewPulsesPerminute = viewPulsesPerMinute;
    // viewPulsePerMinute == aantal enden
    viewPulsesPerMinute = round(pulsesPerMinute / pSettings->pulseFactor);
    // slow down acceleration or deceleration, part 2 of 2
    //viewPulsesPerMinute = dimViewPulsesPerMinute(previousViewPulsesPerminute);  // sets value in global viewPulsesPerMinute
    //revolutions = floor(pSettings->getCounter() / pSettings->ratio);
    revolutions = floor(pSettings->getCounter() / pSettings->pulseFactor);
  }
  echoInterruptOn();
}

void echoInterruptOn() {
  //attachInterrupt(digitalPinToInterrupt(IR_RECEIVE_1), detectPulse, CHANGE);
  // 0 = ir_light, 1 is no ir_light
  attachInterrupt(IR_RECEIVE_1, detectPulse, CHANGE);
  attachInterrupt(IR_RECEIVE_2, detectPulse, CHANGE);
}

void echoInterruptOff() {
  detachInterrupt(IR_RECEIVE_1);
  detachInterrupt(IR_RECEIVE_2);
}

void handlePage() {
  homePage(server, pSettings);

  //writeResult(client, result);
  flashPin(OUTPUT_LED, 100);
  debugMessage("Someone has entered the 'Count' page");

}

void handleWiFi() {
  wifi(server, pSettings, pWifiSettings);
}

void handleDevice() {
  device(server, pSettings);
}

void handleSse() {
  sse(server, pSettings, revolutions, viewPulsesPerMinute);
}
/*
void handleJson() {
  String result = "{\r\n";
  result += "  data: {\r\n";
  result += "    \"revolutions\":";
  result += String(revolutions);
  result += ",\r\n";
  result += "    \"rawCounter\":";
  result += String(pSettings->rawCounter);
  result += ",\r\n";
  result += "    \"viewPulsesPerMinute\":";
  result += String(viewPulsesPerMinute);
  result += "\r\n";
  result += "  }\r\n";
  result += "}\r\n";
   server.sendHeader("Cache-Control", "no-cache");
   server.sendHeader("Connection", "keep-alive");
   server.sendHeader("Pragma", "no-cache");
   server.send(200, "application/json", result);
}
*/

void handleArguments() {
  arguments(server, pSettings);
  showSettings();
}

void mydebug() {
  Serial.println("wifi gegevens");
  Serial.print("readAccessPointSSID: ");
  Serial.println(pWifiSettings->readAccessPointSSID());
  Serial.print("readAccessPointPassword: ");
  Serial.println(pWifiSettings->readAccessPointPassword());
  Serial.print("readNetworkSSID: ");
  Serial.println(pWifiSettings->readNetworkSSID());
  Serial.print("readNetworkPassword: ");
  Serial.println(pWifiSettings->readNetworkPassword());
}

void showSettings() {
  showSavedSettings(server, pSettings);
}

void handleHelp() {
  help(server, pSettings);
}

void handleNetworkSSID() {
  // creates a list of {ssid, including input field , dBm}
  String result = "";
  int numberOfNetworks = WiFi.scanNetworks();
  for(int i =0; i<numberOfNetworks; i++){ 
    //Serial.print("Network name: ");
    //Serial.println(WiFi.SSID(i));
    //Serial.print("Signal strength: ");
    //Serial.println(WiFi.RSSI(i));
    //Serial.println("-----------------------");
    if (i > 0) {
      result += ",";
    }
    result += "{ssid:";
    result += "'<span><input type=\"radio\" name=\"networkSSID\" onclick=\"selectNetworkSSID(this)\" value=\"";
    result += WiFi.SSID(i);
    result += "\">";
    result += WiFi.SSID(i);
    result += "</span>";
    result += "',";
    result += "dBm:'";
    result += WiFi.RSSI(i);
    result += "'}";
    //result += "<br><br>\r\n";

    /*
    result += WiFi.SSID(i);
    result += "<br>\r\n";
    result += "Strength: ";
    result += WiFi.RSSI(i);
    result += "<br>\r\n";
    result += "channel: ";
    result += WiFi.channel(i);
    result += "<br>\r\n";
    result += "encryption: ";
    result += WiFi.encryptionType(i);
    result += "<br>\r\n";
    */
  }
  server.sendHeader("Cache-Control", "no-cache");
  server.sendHeader("Connection", "keep-alive");
  server.sendHeader("Pragma", "no-cache");
  server.send(200, "text/html", result);
}

void handleWifiConnect() {
  uint8_t argumentCounter = 0;
  String result = "";
  //result += ( server.method() == HTTP_GET)?"GET":"POST";
  //result += "\nArguments: ";
  //result +=  server.args();
  //result += "\n";
  if (server.method() == HTTP_POST)
  {
    argumentCounter = server.args();  // if argumentCounter > 0 then save
    String name = "";
    String ssid = "";
    String password = "";
    String target = "";                // for action Erase
    for (uint8_t i=0; i< server.args(); i++){
      //result += " " +  server.argName(i) + ": " +  server.arg(i) + "\n";
      if (server.argName(i) == "name") {
        name = server.arg(i);
      }
      if (server.argName(i) == "ssid") {
        ssid = server.arg(i);
      }
      if (server.argName(i) == "password") {
        password = server.arg(i);
      }
      if (server.argName(i) == "target") {
        target = server.arg(i);
      }
    }
    // zoek name (is ap of network en dan de ssid en password)
    if (name == "ap")
    {
      pWifiSettings->setAccessPointSSID(ssid);
      pWifiSettings->setAccessPointPassword(password);
      if (argumentCounter > 0) {
        pWifiSettings->saveAuthorizationAccessPoint();
        result += "Access Point data has been saved\n";
      }
    }
    if (name == "network")
    {
      pWifiSettings->setNetworkSSID(ssid);
      pWifiSettings->setNetworkPassword(password);
      if (argumentCounter > 0) {
        pWifiSettings->saveAuthorizationNetwork();
        result += "Network connection data has been saved\n";
      }
    }
    if (name == "erase")
    {
      if (argumentCounter > 0) {
        if (target == "eraseAPData")
        {
          pWifiSettings->eraseAccessPointSettings();
          result += "Access Point data has been erased\n";
        }
        if (target == "eraseNetworkData")
        {
          pWifiSettings->eraseNetworkSettings();
          result += "Network connection data has been erased\n";
        }
        if (target == "eraseWiFiData")
        {
          pWifiSettings->eraseWiFiSettings();
          result += "Access Point data and Network connection data has been erased\n";
        }
      }
    }
  }
  server.send(200, "text/plain", result);
  Serial.println(result);
}

void handleDeviceSettings()
{
  uint8_t argumentCounter = 0;
  String result = "";
  //result += ( server.method() == HTTP_GET)?"GET":"POST";
  //result += "\nArguments: ";
  //result +=  server.args();
  //result += "\n";
  if (server.method() == HTTP_POST)
  {
    // extract the settings-data and take action
    argumentCounter = server.args();  // if argumentCounter > 0 then saveConfigurationSettings
    String _name = "";
    String _startWiFiMode = "";
    String _counter = "";
    String _ratio = "";
    String _targetServer = "";
    String _targetPort = "";
    String _targetPath = "";
    String _allowSendingData = "";
    String _isOpen = "";
    String _showData = "";
    String _message = "";
    for (uint8_t i=0; i< server.args(); i++){
      result += " " +  server.argName(i) + ": " +  server.arg(i) + "\n";
      if (server.argName(i) == "name") {
        _name = server.arg(i);
      }
      if (server.argName(i) == "startWiFiMode") {
        _startWiFiMode = server.arg(i);
      }
      if (server.argName(i) == "counter") {
        _counter = server.arg(i);
      }
      if (server.argName(i) == "ratio") {
        _ratio = server.arg(i);
      }
      if (server.argName(i) == "targetServer") {
        _targetServer = server.arg(i);
      }
      if (server.argName(i) == "targetPort") {
        _targetPort = server.arg(i);
      }
      if (server.argName(i) == "targetPath") {
        _targetPath = server.arg(i);
      }
      if (server.argName(i) == "allowSendingData") {
        _allowSendingData = server.arg(i);
      }
      if (server.argName(i) == "isOpen") {
        _isOpen = server.arg(i);
      }
      if (server.argName(i) == "showData") {
        _showData = server.arg(i);
      }
      if (server.argName(i) == "message") {
        _message = server.arg(i);
      }
    }
    // zoek name (is device, targetServer of targetserverData en dan de andere parameters)
    if (_name == "device")
    {
      if (_startWiFiMode == "ap") {
        pSettings->beginAsAccessPoint(true);
      }
      if (_startWiFiMode == "network") {
        pSettings->beginAsAccessPoint(false);
      }
      pSettings->setCounter(_counter);
      pSettings->setRatioArgument(_ratio);
    }
    if (_name == "targetServer")
    {
      pSettings->setTargetServer(_targetServer);
      pSettings->setTargetPort(_targetPort);
      pSettings->setTargetPath(_targetPath);
    }
    if (_name == "targetServerData")
    {
      pSettings->setAllowSendData(_allowSendingData);
      pSettings->setEntree(_isOpen);
      pSettings->setShowData(_showData);
      pSettings->setTargetServerMessage(_message);  // message will not be saved in EEPROM
    }
    if (argumentCounter > 0) {
      pSettings->saveConfigurationSettings();
      result += "Device data has been saved\n";
    }
  }
  server.send(200, "text/plain", result);
  Serial.println(result);
}

void handleNotFound(){
  notFound(server, pSettings);

}



void initHardware()
{
  Serial.begin(115200);
  pinMode(IR_SEND, OUTPUT);
  pinMode(IR_RECEIVE_1, INPUT);  // default down
  pinMode(IR_RECEIVE_2, INPUT);  // default down
  //pinMode(VALID_DISTANCE, OUTPUT);
  pinMode(OUTPUT_LED, OUTPUT);
}

void initServer()
{
  server.close();
  // start webserver
   server.on("/count/", handlePage);
  //server.on("/pulse/", handlePulse);
   server.on("/debug/", mydebug);
   server.on("/wifi/", handleWiFi);
   server.on("/ap/", switchToAccessPoint);
   server.on("/network/", switchToNetwork);
   server.on("/device/", handleDevice);
   server.on("/data.sse/", handleSse);
  //server.on("/data.json/", handleJson);
   server.on("/settings/", handleArguments);
   server.on("/eraseSettings/", eraseSettings);
   server.on("/initSettings/", initSettings);
   server.on("/getSettings/", getSettings);
   server.on("/saveSettings/", saveSettings);
   server.on("/reset/", resetWiFiManagerToFactoryDefaults);
   server.on("/help/", handleHelp);
   server.on("/networkssid/", handleNetworkSSID);
   server.on("/wifiConnect/", handleWifiConnect);
   server.on("/deviceSettings/", handleDeviceSettings);

  //server.onNotFound(handleNotFound);
   server.onNotFound(handleHelp);

   server.begin();
  Serial.println("HTTP server started");
}

void setup()
{
  /* It seems to help preventing ESPerror messages with mode(3,6) when
  using a delay */
  initHardware();
  digitalWrite(IR_SEND, HIGH);
  delay(pSettings->WAIT_PERIOD);

  // see https://forum.arduino.cc/index.php?topic=121654.0 voor circuit brownout
  delay(pSettings->WAIT_PERIOD);
  // use EITHER setupWiFi OR setupWiFiManager
  
  if (pSettings->beginAsAccessPoint())
  {
    setupWiFi();        // local network as access point
  }
  else
  {
    setupWiFiManager();   // part of local network as station
  }
  delay(pSettings->WAIT_PERIOD);
  setupArduinoOTA();
  delay(pSettings->WAIT_PERIOD);
  initServer();
  delay(pSettings->WAIT_PERIOD);
  echoInterruptOn();
  //setupWiFiManager();
  //initEcho();  // interrupt lijkt foutmelding te veroorzaken, kan liggen aan combinatie met eeprom, wifi(manager)
  // volgens bronnen (https://circuits4you.com/2017/12/19/esp8266-fatal-exception-wdt-reset/)
  // kan het aan delay liggen dat ook in eeprom of wifimanager wordt gebruikt
//echoInterruptOff();
}

void loop()
{
  // for setupArduinoOTA
  otaHandle();
  //ArduinoOTA.handle();

  mdns.update();

  // For ESP8266WebServer
  server.handleClient();
  
  // For handleHTTPClient
  if (pSettings->allowSendingData() == true)
  {
    /* send data to target server using ESP8266HTTPClient */
    handleHTTPClient(wifiClient, pSettings, revolutions, viewPulsesPerMinute);
  }

  checkGlobalPulseInLoop();
}
