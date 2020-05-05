#ifndef HANDLEWEBSERVER_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define HANDLEWEBSERVER_H
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include "settings.h"
#include "WiFiSettings.h"

/* show homepage to see the counter */
void homePage(ESP8266WebServer &server, Settings * pSettings);
/* page not found message */
void notFound(ESP8266WebServer &server, Settings * pSettings);
/* get and handle arguments on the settings-page */
void arguments(ESP8266WebServer &server, Settings * pSettings);
/* show the help page */
void help(ESP8266WebServer &server, Settings * pSettings);
/* show saved setting values from EEPROM */
void showSavedSettings(ESP8266WebServer &server, Settings * pSettings);
/* choose settings for the device and the target server */
void device(ESP8266WebServer &server, Settings * pSettings);
/* choose wifi connection, (Access Point or Station -todo: get SSID for Station-) */
void wifi(ESP8266WebServer &server, Settings * pSettings, WiFiSettings * pWifiSettings);
/* sending data through sse */
void sse(ESP8266WebServer &server, Settings * pSettings, uint32_t revolutions, uint32_t viewPulsesPerMinute);

#endif