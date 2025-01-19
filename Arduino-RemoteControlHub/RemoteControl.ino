/*
   Set Board to Heltec Wifi Kit 32 or Generic ESP32 Dev Module, 
   set baud rate to 223,400 for serial debugging.
    
*/

#include "RemoteControl.h"

const char* RCHUB_ID =      "05"; 
const char* RCHUB_NAME =   "RemoteCntlHub";

const char * fileType =     ".rcs\0";

const char * devicesFile =  "/Devices.JSON";

int         devicesCount = 0;

#ifndef HELTEC
// Initialize the OLED display using Arduino Wire:
#define SCREEN_ADDRESS  0x3c ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define I2C_SDA         4 
#define I2C_SCL         15
#define OLED_RST        16
SSD1306Wire display(SCREEN_ADDRESS, I2C_SDA, I2C_SCL);  // 16, GEOMETRY_128_64);   // ADDRESS, SDA, SCL
#endif // HELTEC

unsigned long   lastUpdateTime;
unsigned long   lastPingTime;

Timezone currentTZ;

bool    clearDisplay = false;
bool    cancelDetecting = false;

struct queue_head    actionList;

// Specify IP address or hostname, in our case use the gateway.
IPAddress hostName (8, 8, 8, 8);

//~~ Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*!
    \brief  Function to setup and initialize the system - Standard Arduino call
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void 
setup() 
{
    Serial.begin(230400);
    delay(100);
    Serial.println(" ");
    delay(100);
    Serial.println(F(""));    
    delay(100);    
    Serial.println(F("Nelsony Remote Control Hub"));
    Serial.println(F(""));    

    pinMode(LED_GREEN, OUTPUT);
    digitalWrite(LED_GREEN, LOW);   // turn on the Green LED 

    // Setup the Watchdog timer
    esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
    esp_task_wdt_add(NULL); //add current thread to WDT watch   

#ifdef HELTEC
    Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, false /*Serial Enable*/);
    //Heltec.display->flipScreenVertically();
    Heltec.display->setFont(ArialMT_Plain_16);
    //Heltec.display->setContrast(100); //255);
    Heltec.display->setContrast(10, 5, 0); //255);
    Heltec.display->setBrightness(255);

    Heltec.display->clear();
    Heltec.display->drawString( 0, 0, "Connecting...");
    Heltec.display->display();
#else
    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, LOW); // set GPIO16 low to reset OLED
    delay(50);
    digitalWrite(OLED_RST, HIGH); // while OLED is running, must set GPIO high

    display.init();
    display.flipScreenVertically();
    //display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_16);
    display.clear();
    display.drawString( 0, 0, "Connecting...");
    display.display();
#endif    
    lastUpdateTime = millis();

    INIT_QUEUE_HEAD(&actionList);

    initializeRadio();    

    initializeComms(RCHUB_NAME, RCHUB_ID);

    ServerInit();

    if (apConnected)
    {        
        waitForSync(15);
#ifdef HELTEC
        Heltec.display->clear();
        Heltec.display->drawString( 0, 0, "Connected");
        Heltec.display->drawString( 0, 20, (String)sIPAddr);
        Heltec.display->display();
#else
        display.clear();
        display.drawString( 0, 0, "Connected");
        display.drawString( 0, 20, sIPAddr);
        display.display();
#endif    
        lastUpdateTime = millis();

        Serial.println("UTC: " + UTC.dateTime());
  
        currentTZ.setLocation("America/Los Angeles");
        currentTZ.setDefault();
        Serial.println("Local time: " + currentTZ.dateTime());
        updateEventSchedules();
    }
    else
    {
        Serial.println("Time service not available in AdHoc mode");
    }
    // Do a count of the devices so far
    devicesCount = listDir((char *)fileType, NULL, 0);
}


//~~ Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*!
    \brief  Function main loop thread - Standard Arduino system call
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void 
loop() 
{
    PACTION pAction;
 
    // Reset the Watchdog timer
    esp_task_wdt_reset();

    pAction = getNextAction();
    if (pAction != NULL)
    {
        Serial.print("Processing action: ");
        Serial.println(pAction->action);
        switch(pAction->action)
        {
            case DETECT:
                if (strlen(pAction->devFileName) > 0)
                {
                    // clear the display
#ifdef HELTEC
                    Heltec.display->clear();
                    Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
                    Heltec.display->drawString(64, 0, "Detecting");
                    Heltec.display->display();
#else
                    display.clear();
                    display.setTextAlignment(TEXT_ALIGN_CENTER);
                    display.drawString(64, 0, "Detecting");
                    display.display();
#endif    
                    //if (detectDevice(&txCode) > 150)
                    //{
                    //    Serial.print("Saving to file: ");
                    //    Serial.println(pAction->pDevFileName);
                    //    fileWrite(pAction->devFileName, (unsigned char *)&txCode, sizeof(DEVICE_CODE));
                    if (detectDevice(pAction) == 0)
                    {
                        updateDevicesFile(pAction->devName, pAction->remoteType, 1);
#ifdef HELTEC
                        Heltec.display->clear();
                        Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
                        Heltec.display->drawString(64, 0, "Success");
                        Heltec.display->display();
#else
                        display.clear();
                        display.setTextAlignment(TEXT_ALIGN_CENTER);
                        display.drawString(64, 0, "Success");
                        display.display();
#endif    
                        lastUpdateTime = millis();
                    }
                    else
                    {
#ifdef HELTEC
                        Heltec.display->clear();
                        Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
                        Heltec.display->drawString(64, 0, "Failed");
                        Heltec.display->display();
#else
                        display.clear();
                        display.setTextAlignment(TEXT_ALIGN_CENTER);
                        display.drawString(64, 0, "Failed");
                        display.display();
#endif    
                        lastUpdateTime = millis();
                    }
                }
                break;
            case VERIFY:
#ifdef HELTEC
                Heltec.display->clear();
                Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
                Heltec.display->drawString(64, 0, "Verifying");
                Heltec.display->display();
#else
                display.clear();
                display.setTextAlignment(TEXT_ALIGN_CENTER);
                display.drawString(64, 0, "Verifying");
                display.display();
#endif    
                lastUpdateTime = millis();
                transmitCode(pAction);
                clearDisplay = true;
                break;
            case TRANSMIT:
                transmitCode(pAction);
                break;
            case CREATE:
                if ((strlen(pAction->devFileName) > 0) &&
                    (pAction->remoteType == 4))
                {
                    Serial.print("Creating device: ");
                    Serial.print(pAction->devName);
                    Serial.print(", for unit number: ");
                    Serial.println(pAction->actionName);
                    createMinkeRemoteFiles(pAction->devName, atoi(pAction->actionName));
                }
                break;          
        }
        pAction = releaseAction(pAction);
    }
    if (tryConnecting)
    {
        connectWifi();
        if (apConnected)
        {        
            waitForSync(15);
#ifdef HELTEC
            Heltec.display->clear();
            Heltec.display->drawString( 0, 0, "Connected");
            Heltec.display->drawString( 0, 20, (String)sIPAddr);
            Heltec.display->display();
#else
            display.clear();
            display.drawString( 0, 0, "Connected");
            display.drawString( 0, 20, (String)sIPAddr);
            display.display();
#endif    
            lastUpdateTime = millis();
            lastPingTime = millis();
            Serial.println("UTC: " + UTC.dateTime());
            currentTZ.setLocation("America/Los Angeles");
            currentTZ.setDefault();
            Serial.println("Local time: " + currentTZ.dateTime());
            updateEventSchedules();
        }
        else
        {
            Serial.println("Time service not available in AdHoc mode");
        }
    }
    if (apConnected && (WiFi.status() != WL_CONNECTED))
    {
        tryConnecting = true; 
        apConnected = false;
    }
    
    if (shouldReboot)
    {
        Serial.println("Rebooting...");
        delay(2000);
        ESP.restart();
    }
    if (((millis() - lastUpdateTime) > 15000) ||
        clearDisplay)
    {
        //Serial.println("Clearing display");
#ifdef HELTEC
        Heltec.display->clear();
        Heltec.display->display();
#else
        display.clear();
        display.display();
#endif    
        lastUpdateTime = millis();
        clearDisplay = false;
    }
    if (((millis() - lastPingTime) > 120000) &&
        apConnected)
    {
        // Make sure we can talk to the network
        Serial.println(F("Pinging gateway..."));    
        if (Ping.ping(hostName) == false)
        {
            Serial.println(F("Ping failed."));    
            tryConnecting = true; 
            apConnected = false;
        }
        lastPingTime = millis();
    }
    // Make sure ezTime has some servicing.
    events();
}


//~~ Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*!
    \brief  
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int
convertFileNameToDisplay(
    char *  src,
    char *  dst)
{
    int     srcLen;
    int     noChars = 0;

    srcLen = strlen(src);
    src++;  // skip past path delimiter
    strReplace(src, dst, '_', ' ');
    strReplace(dst, dst, '.', 0);
    return noChars;
}


//~~ Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*!
    \brief  Converts Web request action name to file name action (devaction)
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int
convertActionNameToFile(
    const char *  src,
    int     srcLen,
    char *  dst)
{
    int     noChars = 0;
    
    *dst++ = '/';   // Add in the path delimiter
    dst = strReplace(src, dst, ' ', '_');
    strcat(dst, fileType);
    noChars += strlen(fileType);
    return noChars;
}


//~~ Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*!
    \brief  
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int
convertAddDeviceNameToFile(
    const char *  src,
    const char *  aType,
    char *  pDst,
    int     dstLen)
{
    int     noChars = 0;
    char *  dst = pDst;

    int srcLen = strlen(src);
    
    *dst++ = '/';   // Add in the path delimiter
    dst = strReplace(src, dst, ' ', '_');
    *dst++ = '~';
    dst = strReplace(aType, dst, ' ', '_');
    strcat(dst, fileType);
    noChars += strlen(fileType);
    return noChars;
}


//~~ Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*!
    \brief  Update the devices file with the new device or ignore if duplicate
    \param  device name to add or delete - Not the device file name
    \param  type of remote
    \param  add = 1, 0r 0 to delete.
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int
updateDevicesFile(
    const char *    pDevName, 
    int             remoteType,
    int             opType)
{
   
    char *  pReqBuf = NULL;
    int     bytesRead;
    int     retVal = 0;
    
    pReqBuf = (char *)fileReadBuffer((char *)devicesFile, (int *)&bytesRead);
    if ((pReqBuf != NULL) && (bytesRead > 0))
    {
        Serial.println("Devices.JSON store before changes: ");
        Serial.println((char *)pReqBuf);
        JsonDocument doc;   // DynamicJsonDocument doc(DEVICES_JSON_SZ + (bytesRead * 2)); 
        // Deserialize the JSON document
        DeserializationError error = deserializeJson(doc, pReqBuf);

        // Test if parsing succeeds.
        if (error) 
        {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
        }
        else
        {
            if (opType == 0)
            {
                Serial.print("Deleting device entry: ");
                Serial.println(pDevName);
                //JsonObject device = doc[pDevName];
                doc.remove(pDevName);
                retVal = 1;
            }
            else if (opType == 1)
            {
                char    iBuf[4];
                
                Serial.print("Adding device to JSON store: ");
                Serial.print(pDevName);
                itoa(remoteType, iBuf, 10);
                Serial.print(" with remoteType: ");
                Serial.println(iBuf);
                if (doc[pDevName].set(iBuf))
                    retVal = 1;
            }
            if (retVal)
            {
                byte *      pSerialBuf;
                
                pSerialBuf = (byte *)malloc(DEVICES_JSON_SZ + bytesRead + 2);
                if (pSerialBuf != NULL)
                {
                    bytesRead = serializeJson(doc, pSerialBuf, (DEVICES_JSON_SZ + bytesRead + 2));
                    Serial.print("serializeJson returns: ");
                    Serial.println(bytesRead);
                    fileWrite((char *)devicesFile, pSerialBuf, bytesRead);
                    Serial.println("Devices.JSON store after write: ");
                    Serial.println((char *)pSerialBuf);
                    free(pSerialBuf);
                }
            }
        }
        free(pReqBuf);
    }
    return 0;
}


//~~ Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*!
    \brief  Returns the next avaialable action struct in the table 
            or NULL if full
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PACTION
getNextAvailAction()
{
    PACTION pAction = NULL;

    pAction = (PACTION)malloc(sizeof(ACTION));
    if (pAction != NULL)
    {
        INIT_QUEUE_HEAD(&pAction->queue_member);
    }
    return pAction;
}


//~~ Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*!
    \brief  Returns the next action if any pending else NULL
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PACTION
getNextAction()
{
    PACTION pAction = NULL;

    if (!queue_empty(&actionList))
    {
        pAction = queue_entry(actionList.next, ACTION, queue_member);
    }
    return pAction;
}


//~~ Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*!
    \brief  Submits (completes) the action, ready for processing. 
        Returns 0 if successful
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int
submitAction(
    PACTION     pAction)
{
    queue_add_tail(&pAction->queue_member, &actionList);
    return 0;
}

//~~ Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*!
    \brief  Returns the current action to the pool
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PACTION
releaseAction(
    PACTION     pAction)
{
    queue_del(&pAction->queue_member);
    free(pAction);
    return NULL;
}  


//~~ Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*!
    \brief  Replace target character in string with rep_char and save to dst
    \note   String is null terminated, returned pointer is at the termination
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
char *
strReplace(
    const char *  src,
    char *  pDst,
    char    target,
    char    rep_char)
{
    char *  dst = pDst;

    int srcLen = strlen(src);

    if ((pDst == NULL) ||
        (srcLen == 0))
        return 0;
        
    for (int i=0; i < srcLen; i++)
    {
        // Find the target char to replace
        if (*src == target)
        {
            // Replace it in the destination string only
            *dst++ = rep_char; 
            src++;
        }
        else if (*src != 0)
        {
            *dst++ = *src++;
        }
        else
        {
            break;
        }
    }
    *dst = 0;
    return dst;
}
