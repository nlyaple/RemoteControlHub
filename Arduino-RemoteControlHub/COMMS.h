
#ifndef __COMMS_H__
#define __COMMS_H__

//#include "Arduino.h"

#include <Arduino.h>
#include <WiFi.h>

#include "FS.h"

#ifdef USE_LittleFS
    #include <LittleFS.h>
    #define FILESYS LittleFS
#else
    #include <SPIFFS.h>
    #define FILESYS SPIFFS
#endif 

#include <ESPmDNS.h>
#include <EEPROM.h>
#include <Update.h>
#include <esp_task_wdt.h>
#include <AsyncTCP.h>
#include <HTTPClient.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <ESPping.h>

#define VALID_SIG_LEN   2
#define USER_FLAGS_LEN  2
#define SSID_LENGTH     32
#define PSWD_LENGTH     64
#define IP_ADDR_STR_LEN 16
#define EMAIL_LENGTH    80

#define EEPROM_SIZE     512
#define VALID_OFFSET    0
#define UFLAGS_OFFSET   2
#define SSID_OFFSET     4
#define PSWD_OFFSET     36
#define EMAIL_OFFSET    PSWD_OFFSET + PSWD_LENGTH
//#define FZRST_OFFSET    36 + PSWD_LENGTH
//#define REFST_OFFSET    FZRST_OFFSET + 2
//#define UPLOAD_IP       REFST_OFFSET + 2
#define UPLOAD_IP       EMAIL_OFFSET + EMAIL_LENGTH
#define UPLOAD_PORT     UPLOAD_IP + IP_ADDR_STR_LEN
#define VALID_SIG0      0x6d
#define VALID_SIG1      0xb6

// User_Flags bit map values:
#define UF_WIFI_ON      0x0001
#define UF_UPLOAD_ON    0x0002
//#define UF_SSID_SET     0x0004
//#define UF_PSWD_SET     0x0008

//#define UF_TRY_CONNECT  0x0004
//#define UF_CONNECTED    0x0008

#define UC_BAT_VOLTAGE    0x80
#define UC_BME_TEMP       0x40
#define UC_ONEWIRE_TEMP   0x20


extern String   sIPAddr;
//extern IPAddress sIPAddr;
extern byte     userFlags;
extern byte     userConfig;
extern bool     tryConnecting;
extern bool     apConnected;
extern bool     FILESYS_ERROR;
extern bool     shouldReboot;

extern char     gServerIP[];
extern int      gServerPort;
extern char *   pURL;
extern char     ssid[];
extern char     localSSID[];
extern char     password[];
extern byte     devNum;


#endif // __COMMS_H__
