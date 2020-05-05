
#include "settings.h"

String Settings::getDeviceKey()
{
  return this->deviceKey;
}

uint8_t Settings::getMAX_RATIO_ARGUMENT()
{
  return this->MAX_RATIO_ARGUMENT;
}

uint8_t Settings::getMAX_AXES()
{
  return this->MAX_AXES;
}
  
uint8_t Settings::getMAX_WHEELS()
{
  return this->MAX_WHEELS;
}

char Settings::getAXES_DELIMITER()
{
  return this->AXES_DELIMITER;
}

char Settings::getWHEEL_DELIMITER()
{
  return this->WHEEL_DELIMITER;
}

uint16_t Settings::getSEND_PERIOD()
{
  return this->SEND_PERIOD;
}

String Settings::getTargetServer()
{
  return this->targetServer;
}

uint16_t Settings::getTargetPort()
{
  return this->targetPort;
}

String Settings::getTargetPath()
{
  return this->targetPath;
}

String Settings::getRatioArgument()
{
  return this->ratioArgument;
}

void Settings::setRatioArgument(String ratio)
{
  this->ratioArgument = ratio;
}

String Settings::getFactoryRatioArgument()
{
  return this->factoryRatioArgument;
}

uint8_t Settings::atoi8_t(String s) 
{
    uint8_t i, n;
    n = 0;
    for (i = 0; s[i] >= '0' && s[i] <= '9'; i++)
        n = 10 * n +(s[i] - '0');
    return n;
}

uint16_t Settings::atoi16_t(String s) 
{
    uint8_t i;
    uint16_t n;
    n = 0;
    for (i = 0; s[i] >= '0' && s[i] <= '9'; i++)
        n = 10 * n +(s[i] - '0');
    return n;
}

uint32_t Settings::atoi32_t(String s) 
{
    uint8_t i;
    uint32_t n;
    n = 0;
    for (i = 0; s[i] >= '0' && s[i] <= '9'; i++)
        n = 10 * n +(s[i] - '0');
    return n;
}

String Settings::getFirstElement(String line, char delimiter)
{
  String result = line.substring(0, line.indexOf(delimiter));
  return result;
}

String Settings::getLastElement(String line, char delimiter)
{
  String result = "";
  if (line.lastIndexOf(delimiter) > -1) {
    result = line.substring(line.lastIndexOf(delimiter) + 1, line.length());
  }
  return result;
}

void Settings::calculateRatio(String ratioArgument)
{
  // only for calculating the ratio between axes per revolution of the first axis
  float myRatio = 1.0;
  uint8_t count = 0;
  String wheels[this->MAX_AXES];  // number of '-' MUST not exceed MAX_AXES 

  bool stop = false;
  count = 0;
  String wheel = "";
  for (uint8_t i = 0; i <= ratioArgument.length(); i++) {
    if ((ratioArgument[i] == this->AXES_DELIMITER) || (i == ratioArgument.length())) {
      stop = true;
    }
    else {
      wheel = wheel + ratioArgument[i];
    }
    if (stop) {
      wheels[count] = wheel;
      count +=1;
      wheel = "";
      stop = false;
    }
  }
  uint8_t bladePosition = 0;
  uint8_t wheelPosition = 1;
 
  this->blades = this->atoi8_t(wheels[bladePosition]);
  for (uint8_t i = wheelPosition; i < count; i++)
  {
    uint8_t firstWheel = this->atoi8_t(this->getFirstElement(wheels[i], this->WHEEL_DELIMITER));
    uint8_t lastWheel = this->atoi8_t(this->getLastElement(wheels[i], this->WHEEL_DELIMITER));
    if (lastWheel > 0)      // then there are axes involved
    {
      myRatio *= (firstWheel * 1.0 / lastWheel * 1.0) * 1.0;
    }
  }
  this->ratio = myRatio;                   // used for revolutions, not anymore, see pulseFactor
}

void Settings::calculatePulseFactor(String ratioArgument)
{
  // for calculating the pulse factor
  // how many pulses are neccesary to get 1 revolution of the first axis
  uint8_t count = 0;
  String axes[this->MAX_WHEELS];   // split on '-' 
  bool stop = false;
  count = 0;
  String axis = "";
  float myPulseFactor = 1.0;        // factor that goes from pulses to 1st axis
  for (uint8_t i = 0; i <= ratioArgument.length(); i++) {
    if ((ratioArgument[i] == this->WHEEL_DELIMITER) || (i == ratioArgument.length())) {
      stop = true;
    }
    else {
      axis = axis + ratioArgument[i];
    }
    if (stop) {
      axes[count] = axis;
      count +=1;
      axis = "";
      stop = false;
    }
  }
    for (uint8_t i = 0; i < count; i++)
  {
    uint8_t firstWheel = this->atoi8_t(this->getFirstElement(axes[i], this->AXES_DELIMITER));
    uint8_t lastWheel = this->atoi8_t(this->getLastElement(axes[i], this->AXES_DELIMITER));
    if (lastWheel > 0)
    {
      // if no second value then no new calculation. the previous is then ok
      myPulseFactor *= (lastWheel * 1.0 / firstWheel * 1.0) * 1.0;
    }
  }
  this->pulseFactor = myPulseFactor;      // used for revolutions in molen.cpp
}

uint16_t Settings::setupEEPROM()
{
  // It seems to help preventing ESPerror messages with mode(3,6) when using a delay 
  delay(this->WAIT_PERIOD);

  if (!this->isInitialized())
  {
    this->initNumber = this->INITCHECK;
    return this->saveSettings();
  }
  delay(this->WAIT_PERIOD);
  return this->getSettings();
}

uint16_t Settings::saveSettings()
{
  // It seems to help preventing ESPerror messages with mode(3,6) when using a delay 
  delay(this->WAIT_PERIOD);

  uint16_t firstAddress = this->address;
  uint16_t address = this->address;
 
  EEPROM.begin(this->storageSize);

  //uint32_t start = micros();
  EEPROM.put(address, this->initNumber);
  address += sizeof(this->initNumber);
  EEPROM.put(address, this->version);
  address += sizeof(this->version);
  EEPROM.put(address, this->startAsAccessPoint);
  address += sizeof(this->startAsAccessPoint);
  EEPROM.put(address, this->allowSendingDataValue);
  address += sizeof(this->allowSendingDataValue);

  char myTargetServer[33];  // one more for the null character
  strcpy(myTargetServer, this->targetServer.c_str());
  EEPROM.put(address, myTargetServer);
  address += 33;

  EEPROM.put(address, this->targetPort);
  address += sizeof(this->targetPort);
  
  char myTargetPath[17];  // one more for the null character
  strcpy(myTargetPath, this->targetPath.c_str());
  EEPROM.put(address, myTargetPath);
  address += 17;

  EEPROM.put(address, this->isOpen);
  address += sizeof(this->isOpen);
  EEPROM.put(address, this->showData);
  address += sizeof(this->showData);

  EEPROM.put(address, this->rawCounter);
  address += sizeof(this->rawCounter);

  char myRatioArgument[65];  // one more for the null character
  strcpy(myRatioArgument, this->ratioArgument.c_str());
  EEPROM.put(address, myRatioArgument);
  address += 65;

  char myDeviceKey[37];  // one more for the null character
  strcpy(myDeviceKey, this->deviceKey.c_str());
  EEPROM.put(address, myDeviceKey);
  address += 37;

  EEPROM.commit();    // with success it will return true
  EEPROM.end();       // release RAM copy of EEPROM content

  this->calculateRatio(this->getRatioArgument());
  this->calculatePulseFactor(this->getRatioArgument());

  delay(this->WAIT_PERIOD);

  return address - firstAddress;
}

bool Settings::isInitialized() {
  delay(this->WAIT_PERIOD);
  
  EEPROM.begin(sizeof(this->initNumber));
  this->initNumber = EEPROM.read(this->address);
  EEPROM.end();  // release RAM copy of EEPROM content

  delay(this->WAIT_PERIOD);
 
  return (this->initNumber == this->INITCHECK);
}

bool Settings::eraseSettings() {
  delay(this->WAIT_PERIOD);

  EEPROM.begin(this->storageSize);
  // replace values in EEPROM with 0xff
  for (uint16_t i = 0; i < this->storageSize; i++) {
    EEPROM.write(this->address + i,0xff);
  }
  bool result = EEPROM.commit();    // with success it will return true
  EEPROM.end();  // release RAM copy of EEPROM content

  delay(this->WAIT_PERIOD);

  this->setRatioArgument(this->getFactoryRatioArgument());
  this->calculateRatio(this->getFactoryRatioArgument());
  this->calculatePulseFactor(this->getFactoryRatioArgument());

  return result;
}

uint16_t Settings::initSettings()
{
  // It seems to help preventing ESPerror messages with mode(3,6) when using a delay 
  delay(this->WAIT_PERIOD);

  uint16_t firstAddress = this->address;
  uint16_t address = this->address;

  EEPROM.begin(this->storageSize);

  //uint32_t start = micros();
  EEPROM.put(address, this->factoryInitNumber);
  address += sizeof(this->factoryInitNumber);
  EEPROM.put(address, this->version);
  address += sizeof(this->version);

  EEPROM.put(address, this->factoryStartAsAccessPoint);
  address += sizeof(this->factoryStartAsAccessPoint);
  EEPROM.put(address, this->factoryAllowSendingDataValue);
  address += sizeof(this->factoryAllowSendingDataValue);

  char myFactoryTargetServer[33];  // one more for the null character
  strcpy(myFactoryTargetServer, this->factoryTargetServer.c_str());
  EEPROM.put(address, myFactoryTargetServer);
  address += 33;

  EEPROM.put(address, this->factoryTargetPort);
  address += sizeof(this->factoryTargetPort);
  
  char myFactoryTargetPath[17];  // one more for the null character
  strcpy(myFactoryTargetPath, this->factoryTargetPath.c_str());
  EEPROM.put(address, myFactoryTargetPath);
  address += 17;

  EEPROM.put(address, this->factoryIsOpen);
  address += sizeof(this->factoryIsOpen);
  EEPROM.put(address, this->factoryShowData);
  address += sizeof(this->factoryShowData);

  EEPROM.put(address, this->factoryRawCounter);
  address += sizeof(this->factoryRawCounter);

  //uint8_t myMaxRatioArgument = this->MAX_RATIO_ARGUMENT;
  char myRatioArgument[65];  // one more for the null character
  strcpy(myRatioArgument, this->factoryRatioArgument.c_str());
  EEPROM.put(address, myRatioArgument);
  address += 65;

  //uint8_t myMaxDeviceKey = sizeof(this->deviceKey);
  char myDeviceKey[37];  // one more for the null character
  strcpy(myDeviceKey, this->factoryDeviceKey.c_str());
  EEPROM.put(address, myDeviceKey);
  address += 37;

  delay(this->WAIT_PERIOD);

  EEPROM.commit();    // with success it will return true
  EEPROM.end();       // release RAM copy of EEPROM content
  delay(this->WAIT_PERIOD);

  this->setRatioArgument(this->getFactoryRatioArgument());
  this->calculateRatio(this->getFactoryRatioArgument());
  this->calculatePulseFactor(this->getFactoryRatioArgument());

  return address - firstAddress;
}

uint16_t Settings::getSettings()
{
  // It seems to help preventing ESPerror messages with mode(3,6) when using a delay 
  delay(this->WAIT_PERIOD);

  uint16_t firstAddress = this->address;
  uint16_t address = this->address;

  EEPROM.begin(this->storageSize);
  
  EEPROM.get(address, this->initNumber);
  address += sizeof(this->initNumber);
  EEPROM.get(address, this->version);
  address += sizeof(this->version);

  EEPROM.get(address, this->startAsAccessPoint);
  address += sizeof(this->startAsAccessPoint);
  EEPROM.get(address, this->allowSendingDataValue);
  address += sizeof(this->allowSendingDataValue);

  char myTargetServer[33];  // one more for the null character
  EEPROM.get(address, myTargetServer);
  this->targetServer = String(myTargetServer);
  address += 33;

  EEPROM.get(address, this->targetPort);
  address += sizeof(this->targetPort);
  
  char myTargetPath[17];  // one more for the null character
  EEPROM.get(address, myTargetPath);
  this->targetPath = String(myTargetPath);
  address += 17;

  EEPROM.get(address, this->isOpen);
  address += sizeof(this->isOpen);
  EEPROM.get(address, this->showData);
  address += sizeof(this->showData);

  EEPROM.get(address, this->rawCounter);
  address += sizeof(this->rawCounter);

  //uint8_t myMaxRatioArgument = this->MAX_RATIO_ARGUMENT;
  char myRatioArgument[65];
  EEPROM.get(address, myRatioArgument);
  this->ratioArgument = String(myRatioArgument);

  address += 65;

  //uint8_t myMaxDeviceKey = sizeof(this->deviceKey);
  char mydeviceKey[37];
  EEPROM.get(address, mydeviceKey);
  this->deviceKey = String(mydeviceKey);
  address += 37;

  EEPROM.end();  // release RAM copy of EEPROM content
  delay(this->WAIT_PERIOD);

  this->calculateRatio(this->getRatioArgument());
  this->calculatePulseFactor(this->getRatioArgument());

  return address - firstAddress;
}

uint16_t Settings::saveConfigurationSettings()
{
  // The function EEPROM.put() uses EEPROM.update() to perform the write, so does not rewrites the value if it didn't change.
  // It seems to help preventing ESPerror messages with mode(3,6) when using a delay 
  delay(this->WAIT_PERIOD);

  uint16_t firstAddress = this->address;
  uint16_t address = this->address;
 
  EEPROM.begin(this->storageSize);

  address += sizeof(this->initNumber);
  address += sizeof(this->version);
  
  //bool check_startAsAccessPoint;
  //EEPROM.get(address, check_startAsAccessPoint);
  //if (check_startAsAccessPoint != this->startAsAccessPoint) {
    EEPROM.put(address, this->startAsAccessPoint);
  //}
  address += sizeof(this->startAsAccessPoint);
  
  //bool check_allowSendingDataValue;
  //EEPROM.get(address, check_allowSendingDataValue);
  //if (check_allowSendingDataValue != this->allowSendingDataValue) {
    EEPROM.put(address, this->allowSendingDataValue);
  //}
  address += sizeof(this->allowSendingDataValue);


  //char check_myTargetServer[33];  // one more for the null character
  //EEPROM.get(address, check_myTargetServer);
  char myTargetServer[33];  // one more for the null character
  strcpy(myTargetServer, this->targetServer.c_str());
  //if (check_myTargetServer != myTargetServer) {
    EEPROM.put(address, myTargetServer);
  //}
  address += 33;

  //uint16_t check_targetPort;
  //EEPROM.get(address, check_targetPort);
  //if (check_targetPort != this->targetPort) {
    EEPROM.put(address, this->targetPort);
  //}
  address += sizeof(this->targetPort);

  //char check_myTargetPath[17];  // one more for the null character
  //EEPROM.get(address, check_myTargetPath);
  char myTargetPath[17];  // one more for the null character
  strcpy(myTargetPath, this->targetPath.c_str());
  //if (check_myTargetPath != myTargetPath) {
    EEPROM.put(address, myTargetPath);
  //}
  address += 17;

  //bool check_isOpen;
  //EEPROM.get(address, check_isOpen);
  //if (check_isOpen != this->isOpen) {
    EEPROM.put(address, this->isOpen);
  //}
  address += sizeof(this->isOpen);

  //bool check_showData;
  //EEPROM.get(address, check_showData);
  //if (check_showData != this->showData) {
    EEPROM.put(address, this->showData);
  //}
  address += sizeof(this->showData);

  //uint32_t check_rawCounter;
  //EEPROM.get(address, check_rawCounter);
  //if (check_rawCounter != this->rawCounter) {
    EEPROM.put(address, this->rawCounter);
  //}
  address += sizeof(this->rawCounter);

  //char check_myRatioArgument[65];  // one more for the null character
  //EEPROM.get(address, check_myRatioArgument);
  char myRatioArgument[65];  // one more for the null character
  strcpy(myRatioArgument, this->ratioArgument.c_str());
  //if (check_myRatioArgument != myRatioArgument) {
    EEPROM.put(address, myRatioArgument);
  //}
  address += 65;

  /*
  //char check_myDeviceKey[37];  // one more for the null character
  //EEPROM.get(address, check_myDeviceKey);
  char myDeviceKey[37];  // one more for the null character
  strcpy(myDeviceKey, this->deviceKey.c_str());
  //if (check_myDeviceKey != myDeviceKey) {
    EEPROM.put(address, myDeviceKey);
  //}
  */
  address += 37;

  EEPROM.commit();    // with success it will return true
  EEPROM.end();       // release RAM copy of EEPROM content

  this->calculateRatio(this->getRatioArgument());
  this->calculatePulseFactor(this->getRatioArgument());

  delay(this->WAIT_PERIOD);

  return address - firstAddress;
}

uint16_t Settings::getOffsetAddress()
{
  return this->addressOffset;
}

bool Settings::setOffsetAddress(uint16_t deltaAddress)
{
  if (this->getOffsetAddress() + deltaAddress > this->MAX_EEPROM_SIZE)
  {
    return false;
  }
  this->addressOffset += deltaAddress;
  return true;
}

uint32_t Settings::getFactoryCounter()
{
  return this->factoryRawCounter;
}
 
void Settings::setCounter(uint32_t counter)
{
  this->rawCounter = counter;
}

void Settings::setCounter(String counter)
{
  this->rawCounter = this->atoi32_t(counter);
}

uint32_t Settings::getCounter()
{
  return this->rawCounter;
}

bool Settings::beginAsAccessPoint()
{
  return this->startAsAccessPoint;
}

void Settings::beginAsAccessPoint(bool myBeginAsAccessPointValue)
{
  this->startAsAccessPoint = myBeginAsAccessPointValue;
}

String Settings::getFactoryStartModeWiFi()
{
  return this->factoryStartAsAccessPoint ? "ap" : "network";
}

bool Settings::allowSendingData()
{
  return this->allowSendingDataValue;
}

void Settings::allowSendingData(bool myAllowSendingDataValue)
{
  this->allowSendingDataValue = myAllowSendingDataValue;
}

bool Settings::getIsOpen()
{
  return this->isOpen;
}

void Settings::setEntree(String entree)
{
  this->isOpen = entree == "open";
}

String Settings::getFactoryEntree()
{
  return this->factoryIsOpen ? "open": "closed";
}

String Settings::getFactoryShowData()
{
  return this->factoryShowData ? "show" : "";
}

void Settings::setAllowSendData(String sendData)
{
  this->allowSendingDataValue = sendData == "true";
}

String Settings::getFactoryAllowSendData()
{
  return this->factoryAllowSendingDataValue ? "allow" : "";
}

void Settings::setShowData(String showData)
{
  this->showData = showData == "true";
}

bool Settings::getShowData()
{
  return this->showData;
}

String Settings::getTargetServerMessage()
{
  return this->targetServerMessage;
}

void Settings::setTargetServerMessage(String message)
{
   this->targetServerMessage = message;
}

String Settings::getFactoryTargetServer()
{
  return this->factoryTargetServer;
}

void Settings::setTargetServer(String targetServer)
{
  this->targetServer = targetServer;
}

uint16_t Settings::getFactoryTargetPort()
{
  return this->factoryTargetPort;
}

void Settings::setTargetPort(String targetPort)
{
  this->targetPort = this->atoi16_t(targetPort);
}

String Settings::getFactoryTargetPath()
{
  return this->factoryTargetPath;
}

void Settings::setTargetPath(String targetPath)
{
  this->targetPath = targetPath;
}
