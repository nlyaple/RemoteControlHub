/*
   RemoteControl.h
 
   
*/

#ifndef _REMOTE_CONTROL_H
#define _REMOTE_CONTROL_H

#define RC_HUB          1

#define CC1101_RADIO    1

#define HELTEC      1
#define USE_LittleFS

#include <Arduino.h>
#include <ezTime.h>
#include "COMMS.h"
#include "queues.h"

#ifdef HELTEC
    #include "heltec.h"
#else
    #include "SSD1306Wire.h"  
#endif // Heltec    

#define VERSION_NUM "0.10"

#define DISPLAY_ATTACHED    1
//#define TEXT_PRINT          1
//120 seconds WDT
#define WDT_TIMEOUT     120

// ESP32 Pins
#define RF_RX       2  // 21
#define RF_CS       26
#define RF_TX       12

#define LED_GREEN   25  // Builtin LED.

#define GENERIC_BUF_SZ  2048        // Used for the directory list JSON response
#define DEVICE_NAME_SZ  32
#define ACTION_NAME_SZ  16

#define DEVICES_JSON_SZ     DEVICE_NAME_SZ + ACTION_NAME_SZ + 4

#define RC_MAX_EVENTS  4

#define ACTION_POOL_SIZE    5

#define ARRAY_DEPTH 2048

#define MAX_FILENAME_SZ     DEVICE_NAME_SZ + ACTION_NAME_SZ + 8

#define FILE_NAME_SZ    MAX_FILENAME_SZ

enum _STATES {
    IDLE,
    DETECT,
    VERIFY,
    SAVE,
    TRANSMIT,
    TIMED_EVENT,
    ACT_CANCEL,
    CREATE
} STATES;


typedef struct  _ACTION {
    struct queue_head    queue_member;
    byte        action;
    byte        remoteType;
    char        devName[DEVICE_NAME_SZ];
    char        actionName[ACTION_NAME_SZ];
    char        devFileName[DEVICE_NAME_SZ + ACTION_NAME_SZ + 8];
} ACTION, *PACTION;

typedef struct  _BIT_ENTRY {
    byte            POLARITY;
    unsigned short  CELL_TIME;
} BIT_ENTRY, *PBIT_ENTRY;

typedef struct  _DEVICE_CODE {
int             pulseCount;
int             remoteType;
BIT_ENTRY       bits[ARRAY_DEPTH];
} DEVICE_CODE, *PDEVICE_CODE;

typedef struct  _DEVICE_CODE_EX {
int             pulseCount;
int             remoteType;
float           frequency;
byte            repeatCnt;
BIT_ENTRY       bits[ARRAY_DEPTH];
} DEVICE_CODE_EX, *PDEVICE_CODE_EX;


typedef struct  _DEVICE_PROTO {
int             pulseCount;
int             remoteType;
float           frequency;
short           bitCellTime;
byte            unitNumber;
byte            unitNumBits;
byte            command;
byte            cmdNumBits;
byte            packetByteLen;
byte            protoData[];
} DEVICE_PROTO, *PDEVICE_PROTO;

extern const char * eventFileName;

extern DEVICE_CODE  txCode;

extern bool    clearDisplay;
extern bool    cancelDetecting;
extern int     devicesCount;


#endif // _REMOTE_CONTROL_H
