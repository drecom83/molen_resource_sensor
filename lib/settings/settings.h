#ifndef SETTINGS_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define SETTINGS_H
#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <EEPROM.h>               // used to store and read settings

class Settings
{
public:
  /* time between two sse pushes in ms */
  const uint16_t SSE_RETRY = 1000;

  /* wait period in milliseconds */
  uint32_t WAIT_PERIOD = 200;

private:
  /* start as Access Point or as Network client */
  bool startAsAccessPoint = true;

  /* TODO factoryStartAsAccessPoint, is prepared for in storageSize for EEPROM */
  bool factoryStartAsAccessPoint = true;

  /* maximum length of ratioArgument string */
  uint8_t MAX_RATIO_ARGUMENT = 64;
  
  /* maximun number of axes for calculating the ratio */
  const uint8_t MAX_AXES = 32;
  
  /* maximum number of wheels for calculating the pulse factor */
  const uint8_t MAX_WHEELS = 32;

  /* delimiter for axes */
  const char AXES_DELIMITER = '-';

  /* delimiter for connected wheels whithout an ax */
  const char WHEEL_DELIMITER = '.';

  /* interval for sending data to the target server */
  uint16_t SEND_PERIOD = 3000;

  /* target server, max size = 32 */
  String targetServer = "http://10.0.0.10";
  /* TODO factoryTargetServer, is prepared for in storageSize for EEPROM */
  String factoryTargetServer = "http://10.0.0.40"; //"http://meandmy.info";

  /* target port */
  uint16_t targetPort = 9090;
  /* TODO factoryTargetPort server, is prepared for in storageSize for EEPROM */
  uint16_t factoryTargetPort = 80;

  /* target path, max size = 16 */
  String targetPath = "/feed";
  /* TODO factoryTargetPath server, is prepared for in storageSize for EEPROM */
  String factoryTargetPath = "/feed/";

  /* show on the server if the place is open for visitors */
  bool isOpen = true;
  /* TODO factoryIsOpen, is prepared for in storageSize for EEPROM */
  bool factoryIsOpen = true;

  /* show on the server the sent data */
  bool showData = true;
  /* TODO factoryShowData server, is prepared for in storageSize for EEPROM */
  bool factoryShowData = true;

  /* show on the server the sent data */
  bool allowSendingDataValue = false;
  /* TODO factoryShowData server, is prepared for in storageSize for EEPROM */
  bool factoryAllowSendingDataValue = false;

  /* message that is shown on the targetServer, comes with the data, is not stored */
  String targetServerMessage = "";

  /* MAX_RATIO_ARGUMENT bytes to store, user-entered ratio as argument, example: "4-72.99.33-80.24" */
  String ratioArgument = "4-72.99.33-80"; //"4-4"; // keeps ratioArgument, for future use in a form 

  /* MAX_RATIO_ARGUMENT bytes to store, factory setting, user-entered ratio as argument, example: "4-72.99.33-80.24" */
  String factoryRatioArgument = "4-4"; // keeps ratioArgument, for future use in a form 

  /* 4 bytes to store, version of this data */
  float version = 0.1f;

  /* Maximum size of EEPROM, SPI_FLASH_SEC_SIZE comes from spi_flash.h */
  const uint16_t MAX_EEPROM_SIZE = SPI_FLASH_SEC_SIZE;
  
  /* first address for Settings storage */
  const uint16_t address = 0;

  /* first available address for Settings storage, for use in other functions or classes */
  uint16_t addressOffset = 0;

  /* check for first saved initialization */
  const uint8_t INITCHECK = 61;

  /* 1 byte to store, holds check for first initialization */
  uint8_t initNumber = 0;

  /* 1 byte to store, factory setting, holds check for first initialization */
  uint8_t factoryInitNumber = 0;

  /* 37 bytes to store, together with the MAC address, the identification of a device */
  String deviceKey = "88888888-4444-4444-4444-121212121212";

  /* 37 bytes to store, factory setting, together with the MAC address, the identification of a device */
  String factoryDeviceKey = "88888888-4444-4444-4444-121212121212";

  /* 4 bytes to store, counts every pulse, range 0 - 2^32 = 4294967296 */
  uint32_t rawCounter = 0;

  /* 4 bytes to store, factory setting, scounts every pulse, range 0 - 2^32 = 4294967296 */
  uint32_t factoryRawCounter = 0;

  /* sizeof of serialized variable, marked as 'to store' */
  uint16_t storageSize;

public:

  /* hold the ratio between measured point and the number of revolutions of the desired object 
     > 1 is accelaration 
     < 1 is decelleration 
  */
  float ratio = 1.0f;           

  /* a factor to calculate the measured pulses to the number of pulses the first axis has, opposite of ratio */
  float pulseFactor = 1.0f;
  
  /* number of blades */
  uint8_t blades = 4;


public:
  Settings()
  {
    storageSize = sizeof(initNumber) + 
                  sizeof(version) + 
                  sizeof(startAsAccessPoint) +
                  sizeof(allowSendingDataValue) +
                  33 +                  // max size targetServer + 1
                  sizeof(targetPort) + 
                  17 +                  // max size of targetPath + 1
                  sizeof(isOpen) + 
                  sizeof(showData) + 
                  sizeof(rawCounter) + 
                  37 +                  // MAX_DEVICEKEY + 1
                  129;                  // MAX_RATIO_ARGUMENT + 1


    /* set new address offset */
    //this->initSettings();
    //this->setOffsetAddress(storageSize);
    this->addressOffset = this->address + storageSize;
    //eraseSettings();
    setupEEPROM();

    /* TODO: change this... it's used in the during development phase */
    //this->startAsAccessPoint = false;
  };

  ~Settings()
  {
  };

private:
  /* converts a string of numbers to an integer */
  uint8_t atoi8_t(String s);

  /* converts a string of numbers to an integer */
  uint16_t atoi16_t(String s);

  /* converts a string of numbers to an unsigned 32 bits number */
  uint32_t atoi32_t(String s);

  String getFirstElement(String line, char delimiter);
  String getLastElement(String line, char delimiter);

  //* check to see if the EEPROM settings are already there */
  bool isInitialized();


  
public:
  /* calculate the ratio using the ratioArgument and store the result in this->ratio */
  void calculateRatio(String ratioArgument);

  /* calculate the pulsefactor using the ratioArgument and store the result in this-> pulseFactor */
  void calculatePulseFactor(String ratioArgument);

  /* does the initial setup of the settings and saves the values on EEPROM-address (default start= 0), returns length of saved bytes */
  uint16_t setupEEPROM();

  /* saves settings in EEPROM starting on EEPROM-address (default = 0), returns length of saved bytes */
  uint16_t saveSettings();

  /* erase settings, set value ff on every EEPROM Settings address, returns true if it succeeds */
  bool eraseSettings();

  /* set Settings value to factory values and saves the values on EEPROM-address (default start= 0), returns length of saved bytes */
  uint16_t initSettings();

  /* get Settings from EEPROM */
  uint16_t getSettings();

  /* saves only Changed Configuration Settings in EEPROM starting on EEPROM-address (default = 0), returns length of saved bytes */
  uint16_t saveConfigurationSettings();

  /* return deviceKey */
  String getDeviceKey();

  /* maximum length of ratioArgument string */
  uint8_t getMAX_RATIO_ARGUMENT();
  
  /* maximun number of axes for calculating the ratio */
  uint8_t getMAX_AXES();
  
  /* maximum number of wheels for calculating the pulse factor */
  uint8_t getMAX_WHEELS();

  /* delimiter for axes */
  char getAXES_DELIMITER();

  /* delimiter for connected wheels whithout an ax */
  char getWHEEL_DELIMITER();

  /* period for sending data to the target server */
  uint16_t getSEND_PERIOD();

  /* return factory setting for targetServer */
  String getFactoryTargetServer();

  /* targetServer */
  String getTargetServer();

  /* set target server */
  void setTargetServer(String targetServer);

  /* return factory setting for targetPort */
  uint16_t getFactoryTargetPort();

  /* targetPort */
  uint16_t getTargetPort();

  /* set target port */
  void setTargetPort(String port);

  /* return factory setting for targetPath */
  String getFactoryTargetPath();

  /* targetPath */
  String getTargetPath();

  /* set target path */
  void setTargetPath(String targetPath);

  /* return the factorySetting of rawCounter */
  uint32_t getFactoryCounter();
  /* set value to pulse counter, also known as rawCounter */
  void setCounter(uint32_t counter);
  void setCounter(String counter);

  /* set value to pulse counter, also known as rawCounter */
  uint32_t getCounter();

  /* MAX_RATIO_ARGUMENT bytes to store, user-entered ratio as argument, example: "4-72.99.33-80.24" */
  String getRatioArgument(); // keeps ratioArgument, for future use in a form 

  /* MAX_RATIO_ARGUMENT bytes to store, user-entered ratio as argument, example: "4-72.99.33-80.24" */
  void setRatioArgument(String ratio); // keeps ratioArgument, for future use in a form 

  /* MAX_RATIO_ARGUMENT bytes to store, factory setting, user-entered ratio as argument, example: "4-72.99.33-80.24" */
  String getFactoryRatioArgument(); // keeps ratioArgument, for future use in a form 

  /* EEPROM Offset Address, for use in other functions or classes */
  uint16_t getOffsetAddress();

  /* EEPROM set new value for Offset Address, for use in other functions or classes */
  bool setOffsetAddress(uint16_t deltaAddress);

  /* returns factory setting beginAs AccessPoint for WiFi start-mode, translated to "ap" or "network" */
  String getFactoryStartModeWiFi();

  /* return start as Access point or as network client */
  bool beginAsAccessPoint();

  /* set start as Access point or as network client */
  void beginAsAccessPoint(bool beginAsAccessPointValue);

  /* for target server */
  bool allowSendingData();

  /* for target server */
  void allowSendingData(bool allowSendingDataValue);

  /* for target server */
  bool getIsOpen();

  /* return factory setting isOpen, translated to "open" or "closed" */
  String getFactoryEntree();

  /* set isOpen, translated from "open" or "closed" */
  void setEntree(String entree);

  /* return factory setting showData, translated to "show" of "" */
  String getFactoryShowData();

  /* set showData, translated from "allow" or "" */
  void setAllowSendData(String allowSendData);

  /* return factory setting showData, translated to "allow" of "" */
  String getFactoryAllowSendData();

  /* set showData, translated from "show" or "" */
  void setShowData(String showData);

  /* for target server */
  bool getShowData();

  /* for target server */
  String getTargetServerMessage();

  /* for target server */
  void setTargetServerMessage(String message);
};
#endif