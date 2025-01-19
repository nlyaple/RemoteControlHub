// Importing necessary libraries
// Requires AsyncTCP and ESPAsyncWebServer libraries that are not in the Arduino Libray Manager
// https://github.com/me-no-dev/AsyncTCP
// https://github.com/me-no-dev/ESPAsyncWebServer

#include "COMMS.h"

// Create the AsyncWebServer object 
AsyncWebServer server(80);

File    gFileHandle;
//flag to use from web update to reboot the ESP
//bool    shouldReboot = false;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Nelsony Remote Control Hub</title>
</head>
<body>
  <h1>Nelsony Remote Control Hub</h1>
  <h2>Remote Control Hub has an internal error. Please contact support.</h2>
</body>
</html>
)rawliteral";


// Replaces placeholder with button section in your web page
String processor(const String& var)
{
    if (var == "WIFISTATE")
    {
        String value = "";
        if (!(userFlags & UF_WIFI_ON))
            value += "disabled";
        return value;
    }
    else if (var == "WIFIBUTTON")
    {
        String buttons = "";
        if (userFlags & UF_WIFI_ON)
        {
            buttons += "checked";
        }
        return buttons;
    }    
    else if (var == "SSID_VALUE")
    {  
        String value = String(ssid);
        return value;
    }
    else if (var == "PSWD_VALUE")
    {  
        String value = String(password);
        return value;
    }
    else if (var == "IPADDRESS")
    {
        return sIPAddr;
    }
    else if (var == "VERSION_NUMBER")
    {  
        String value = String(VERSION_NUM);
        return value;
    }
    return String();
}


void 
ServerInit()
{
    if (!FILESYS.begin(true))
    {
        Serial.println("An Error has occurred while mounting FILESYS");
        FILESYS_ERROR = true;
    }

    // API to read devices file list
    server.on("/devices", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        //Serial.println("Recieved API GET request");
        //Serial.print("X");
        if(request->hasHeader("x-api-key"))
        {
            AsyncWebHeader* h = request->getHeader("x-api-key");
            if (strstr(h->value().c_str(), "69c45815-ca2e-45e5-a2ea-e07aafb37595") != NULL)
            {
                char *      pBuf = NULL;
                int         listSize;

                listSize = listDir((char *)fileType, NULL, 0);
                Serial.print("Directory List size: ");
                Serial.println(listSize);
                pBuf = (char *)malloc(listSize);
                if (pBuf != NULL)
                {
                    listDir((char *)fileType, pBuf, listSize);
                    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", (char *)pBuf);
                    request->send(response);
                    free(pBuf);
                }
                else
                {
                    request->send(404, "text/plain", "Allocation fault");
                }
            }
            else
            {
                Serial.println("Recieved API GET request bad key");
                request->send(401, "text/plain", "Unauthorized");  
            }
        }
        else
        {
            Serial.println("Recieved API GET request No API Key");
            request->send(404, "text/plain", "Not Found");
        }
    });

    // API to read devices file
    server.on("/deviceList", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        Serial.println("Recieved API GET read deviceList request");
        //Serial.print("X");
        if(request->hasHeader("x-api-key"))
        {
            AsyncWebHeader* h = request->getHeader("x-api-key");
            if (strstr(h->value().c_str(), "69c45815-ca2e-45e5-a2ea-e07aafb37595") != NULL)
            {
                char    fileName[DEVICE_NAME_SZ];
                char *  pFileName = &fileName[0];
                char *  pReqBuf;
                int     bytesRead;
                
                strncpy(pFileName, "/Devices.JSON", DEVICE_NAME_SZ);
                pReqBuf = (char *)fileReadBuffer(pFileName, (int *)&bytesRead);
                if (pReqBuf != NULL)
                {
                    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", (char*)pReqBuf);
                    request->send(response);
                    free(pReqBuf);
                }
                else
                {
                    request->send(404, "text/plain", "Allocation fault");
                }
            }
            else
            {
                Serial.println("Recieved API GET request bad key");
                request->send(401, "text/plain", "Unauthorized");  
            }
        }
        else
        {
            Serial.println("Recieved API GET request No API Key");
            request->send(404, "text/plain", "Not Found");
        }
    });

    // Route for activating a device
    server.on("/devaction", HTTP_POST, [](AsyncWebServerRequest * request)
    {}, NULL, [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) 
    {
        PACTION pAction;

        data[len] = 0;  // Null terminate the data.
        pAction = getNextAvailAction();
        if (pAction != NULL)
        {
            convertActionNameToFile((const char *)data, len, pAction->devFileName);
            Serial.print("Activating device: ");
            Serial.println(pAction->devFileName);
            pAction->action = TRANSMIT;
            request->send(200, "text/plain");
            submitAction(pAction);
        }
        else
        {
            Serial.println("Recieved action request, system busy");
            request->send(304, "text/plain", "Busy");
        }
    });

      
    // Route for Removing a device
    server.on("/devrem", HTTP_POST, [](AsyncWebServerRequest *request)
    {}, NULL, [](AsyncWebServerRequest * request, uint8_t *idata, size_t len, size_t index, size_t total) 
    {
        idata[len] = 0;  // Null terminate the data.
        Serial.print("Removing device: ");
        Serial.println((char *)idata);
        updateDevicesFile((char *)idata, 0, 0);
        removeFiles((char *)idata);
        request->send(200, "text/plain");
    });

    // Route for Adding a new device
    server.on("/devadd", HTTP_POST, [](AsyncWebServerRequest *request)
    {}, NULL, [](AsyncWebServerRequest * request, uint8_t *idata, size_t len, size_t index, size_t total) 
    {
        idata[len] = 0;  // Null terminate the data.

        Serial.print(F("Raw data: "));
        Serial.println((char *)idata);
        JsonDocument doc;  // DynamicJsonDocument doc(len * 2);
        // Deserialize the JSON document
        DeserializationError error = deserializeJson(doc, idata);

        // Test if parsing succeeds.
        if (error) 
        {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            cancelDetecting = true;
        }
        else
        {
            PACTION pAction = getNextAvailAction();
            if (pAction != NULL)
            {
                const char * devName = doc["device"];
                const char * reqAction = doc["action"];
                const char * actionName = doc["actionName"];

                pAction->remoteType = doc["remoteType"];
                strncpy(pAction->devName, devName, DEVICE_NAME_SZ);
                strncpy(pAction->actionName, actionName, ACTION_NAME_SZ);

                Serial.print("deviceName: ");
                Serial.println(pAction->devName);
                Serial.print("remoteType: ");
                Serial.println(pAction->remoteType);
                Serial.print("action: ");
                Serial.println(reqAction);
                Serial.print("actionName: ");
                Serial.println(pAction->actionName);
            
                cancelDetecting = false;
                convertAddDeviceNameToFile(devName, actionName, pAction->devFileName, MAX_FILENAME_SZ);

                if (strstr(reqAction, "Detect") != NULL)
                {
                    pAction->action = DETECT;
                }
                else if (strstr(reqAction, "Verify") != NULL)
                {
                    pAction->action = VERIFY;
                }
                else if (strstr(reqAction, "Create") != NULL)
                {
                    pAction->action = CREATE;
                }
                submitAction(pAction);
            }
            else
            {
                Serial.println(F("FAILED to allocate a new action from getNextAvailAction."));
            }
        }
        request->send(200, "text/plain");
    });


    // API to get an events JSON file from FILESYS
    server.on("/getevent", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        Serial.println("Recieved Event GET request");
        
        if(request->hasHeader("x-api-key"))
        {
            AsyncWebHeader* h = request->getHeader("x-api-key");
            if (strstr(h->value().c_str(), "69c45815-ca2e-45e5-a2ea-e07aafb37595") != NULL)
            {
                if (request->hasParam("event")) 
                {
                    const char *    fn;
                    char            fileName[30];
                    
                    fn = request->getParam("event")->value().c_str();
                    if (strlen(fn) < 2)
                    {
                        sprintf((char *)&fileName, "/EVENT%s.JSON", fn);
                        Serial.print("Recieved Get Event, requesting filename: ");
                        Serial.println((char *)&fileName);
                        File file = FILESYS.open((char *)&fileName, FILE_READ);
                        if (file && !file.isDirectory())
                        {
                            file.close();
                            // Reset the Watchdog timer
                            esp_task_wdt_reset();
                            AsyncWebServerResponse *response = request->beginResponse(FILESYS, (char *)&fileName, "application/json");
                            response->addHeader("Content-Encoding", "application/json");
                            request->send(response);
                        }
                        else
                        {
                            Serial.println("Recieved Get Event, file not found");
                            request->send(401, "text/plain", "File Not Found");  
                        }
                    }
                    else
                    {
                        Serial.println("Recieved Get Event, no filename");
                        request->send(401, "text/plain", "Bad Parameter");  
                    }
                }
                else
                {
                    Serial.println("Recieved Get Event, no filename");
                    request->send(401, "text/plain", "Bad Parameter");  
                }
            }
            else
            {
                Serial.println("Recieved Get Event, bad key");
                request->send(401, "text/plain", "Unauthorized");  
            }
        }
        else
        {
            Serial.println("Recieved Get Event, No API Key");
            request->send(404, "text/plain", "Not Found");
        }
    });


    // Route for Setting an event
    server.on("/setevent", HTTP_POST, [](AsyncWebServerRequest *request)
    {}, NULL, [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) 
    {
        data[len] = 0;  // Null terminate the data.
        JsonDocument doc;  //  DynamicJsonDocument doc(len * 2); //1024);
        // Deserialize the JSON document
        DeserializationError error = deserializeJson(doc, data);

        // Test if parsing succeeds.
        if (error) 
        {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            cancelDetecting = true;
        }
        else
        {
            byte *      pSerialBuf;
            char        fileName[16];
            int         eventNumInt;
            int         eventHour;
            int         eventMin;
            size_t      bytesSerialized;
            const char * eventNum = doc["eventNum"];
            
            //const char * eventTime = doc["time"];
            //const char * eventNames = doc["events"][0];

            //Serial.print("Event number: ");
            //Serial.print(eventNum);
            //Serial.print(", time: ");
            //Serial.print(eventTime);
            //Serial.print(", events: ");
            //Serial.println(eventNames);
            eventNumInt = atoi(eventNum);
            cancelDetecting = false;
            sprintf((char *)&fileName, eventFileName, eventNumInt);
            
            Serial.print("Saving to file: ");
            Serial.println(fileName);
            pSerialBuf = (byte *)malloc(len * 2);
            if (pSerialBuf != NULL)
            {
                bytesSerialized = serializeJson(doc, pSerialBuf, (len * 2));
                if (bytesSerialized > 0)
                {
                    fileWrite((char *)&fileName, pSerialBuf, bytesSerialized);
                    getEventTime(eventNumInt, (int *)&eventHour, (int *)&eventMin);
                    nextIntervalTime(calcTimeDiff(eventHour, eventMin), eventNumInt);
                }
                free(pSerialBuf);
            }
        }
        request->send(200, "text/plain");
    });

    
    // Route for WiFi configuration
    server.on("/set", HTTP_POST, [](AsyncWebServerRequest *request)
    {
        int params = request->params();
        Serial.print("Recieved set POST with params = ");
        Serial.println(params);
        userFlags = 0;
        tryConnecting = true;
        
        for (int i=0; i < params; i++)
        {
            AsyncWebParameter* p = request->getParam(i);
            if (p->isPost())
            {
                Serial.printf("POST set[%s]: %s\n", p->name().c_str(), p->value().c_str());
                if (strstr(p->name().c_str(), "WiFi") != NULL)
                {
                    if (strstr(p->value().c_str(), "on") != NULL)
                    {
                        userFlags |= UF_WIFI_ON;
                    }
                }
                else if (strstr(p->name().c_str(), "SSID") != NULL)
                {
                    strncpy(ssid, p->value().c_str(), 32);
                    Serial.print("SSID set to ");
                    Serial.println(ssid);
                    for (int i= 0; i < SSID_LENGTH; i++)
                    {
                        EEPROM.write(SSID_OFFSET + i, ssid[i]);
                    }
                    userFlags |= UF_WIFI_ON;
                }
                else if (strstr(p->name().c_str(), "PSWD") != NULL) 
                {
                    strncpy(password, p->value().c_str(), 64);
                    Serial.print("Password set to ");
                    Serial.println(password);
                    for (int i= 0; i < PSWD_LENGTH; i++)
                    {
                        EEPROM.write(PSWD_OFFSET + i, password[i]);
                    }
                    userFlags |= UF_WIFI_ON;
                }
            } 
        }        
        EEPROM.write(UFLAGS_OFFSET, userFlags);
        EEPROM.commit();
        request->send(FILESYS, "/index.html", String(), false, processor);
    });

    // API to read file from FILESYS
    server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        Serial.println("Recieved Download GET request");
        
        if(request->hasHeader("x-api-key"))
        {
            AsyncWebHeader* h = request->getHeader("x-api-key");
            if (strstr(h->value().c_str(), "69c45815-ca2e-45e5-a2ea-e07aafb37595") != NULL)
            {
                if (request->hasParam("filename")) 
                {
                    const char *  fn;

                    fn = request->getParam("filename")->value().c_str();
                    Serial.print("Recieved Download, requesting filename: ");
                    Serial.println(fn);
                    File file = FILESYS.open(fn, FILE_READ);
                    if (file && !file.isDirectory())
                    {
                        file.close();
                        // Reset the Watchdog timer
                        esp_task_wdt_reset();
                        AsyncWebServerResponse *response = request->beginResponse(FILESYS, fn, "application/octet-stream");
                        response->addHeader("Content-Encoding", "application/octet-stream");
                        request->send(response);
                    }
                    else
                    {
                        Serial.println("Recieved Get Event, file not found");
                        request->send(401, "text/plain", "File Not Found");  
                    }
                }
                else
                {
                    Serial.println("Recieved Download, no filename");
                    request->send(401, "text/plain", "Bad Parameter");  
                }
            }
            else
            {
                Serial.println("Recieved Download, bad key");
                request->send(401, "text/plain", "Unauthorized");  
            }
        }
        else
        {
            Serial.println("Recieved Download, No API Key");
            request->send(404, "text/plain", "Not Found");
        }
    });

    server.onNotFound([](AsyncWebServerRequest *request) 
    {
        if (request->method() == HTTP_OPTIONS) 
        {
            Serial.println("Recieved OPTIONS request");
            request->send(200);
        } 
        else 
        {
            Serial.println("Recieved onNotFound request");
            request->send(404);
        }
    });

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        Serial.println("Recieved default page GET request");
        if (FILESYS_ERROR)
        {
            request->send_P(200, "text/html", index_html); //, processor);
        }
        else
        {
            if (devicesCount > 0)
                request->send(FILESYS, "/index.html", String(), false, processor);
            else
                request->send(FILESYS, "/adddevice.html", String(), false, processor);
        }
    });

    // Route for root / web page
    server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send(FILESYS, "/index.html", String(), false, processor);
    });

    // Route for style sheet
    server.on("/style.min.css", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        Serial.println("Recieved GET style.min.css request");
        request->send(FILESYS, "/style.min.css");
    });

    // Route for settings page
    server.on("/settings.html", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        Serial.println("Recieved GET settings.html request");
        cancelDetecting = true;
        request->send(FILESYS, "/settings.html", String(), false, processor);
    });

    // Route for settings icon
    server.on("/settings-icon.png", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        Serial.println("Recieved GET settings-icon.png request");
        request->send(FILESYS, "/settings-icon.png");
    });

    // Route for add devices page
    server.on("/adddevice.html", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        Serial.println("Recieved GET adddevice.html request");
        request->send(FILESYS, "/adddevice.html", String(), false, processor);
    });

    // Route for remove devices page
    server.on("/remdevice.html", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        Serial.println("Recieved GET remdevice.html request");
        request->send(FILESYS, "/remdevice.html", String(), false, processor);
    });

    // Route for events page
    server.on("/events.html", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        Serial.println("Recieved GET events.html request");
        request->send(FILESYS, "/events.html", String(), false, processor);
    });

    // Simple Firmware Update Form
    server.on("/fota", HTTP_GET, [](AsyncWebServerRequest *request) 
    {
        Serial.println("Recieved GET fota.html request");
        request->send(FILESYS, "/fota.html", String(), false, processor);
    });

    // Route for favicon.ico page
    server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        Serial.println("Recieved GET favicon.ico request");
        request->send(FILESYS, "/favicon.ico", String(), false, processor);
    });

    server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) 
    {
        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain");
        Serial.println("Recieved upload POST request");
        response->addHeader("Connection", "close");
        request->send(response);
    }
    ,[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) 
    {
        char    fileName[128];
        
        if ((strstr(filename.c_str(), ".html") != NULL) ||
            (strstr(filename.c_str(), ".rcs") != NULL) ||
            (strstr(filename.c_str(), ".css") != NULL) ||
            (strstr(filename.c_str(), ".JSON") != NULL))
        {
            if (!index) 
            {
                strcpy((char *)&fileName[0], "/");
                strcat((char *)&fileName[0], filename.c_str());
                strcat((char *)&fileName[0], "-update");
                Serial.printf("Upload Start: %s\n", fileName);
                // Reset the Watchdog timer
                esp_task_wdt_reset();
                gFileHandle = FILESYS.open(fileName, FILE_WRITE);
                if (!gFileHandle)
                {
                    Serial.println("There was an error opening the file for writing");
                    return -1;
                }        
            }
            // Reset the Watchdog timer
            esp_task_wdt_reset();
            if (gFileHandle.write(data, len))
            {
                Serial.println("File was written");
            } else {
                Serial.println("File write failed");
            }
            if (final) 
            {
                // Reset the Watchdog timer
                esp_task_wdt_reset();
                gFileHandle.close();
                Serial.println(" DONE");
                shouldReboot = true;
                request->send(FILESYS, "/index.html", String(), false, processor);
            }
        }
        else
        {
            Serial.printf("Unknown update file type:%s, %u B\n", filename.c_str(), len);
        }
        return 0;
    });
    
   
    server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) 
    {
        shouldReboot = !Update.hasError();
        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", shouldReboot?"OK":"FAIL");
        response->addHeader("Connection", "close");
        request->send(response);
    }
    ,[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) 
    {
        if (strstr(filename.c_str(), ".bin") != NULL)
        {
            if (!index) 
            {
                Serial.printf("Update Start: %s\n", filename.c_str());
                //Update.runAsync(true);
                // Reset the Watchdog timer
                esp_task_wdt_reset();
                if(!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000))
                {
                    Update.printError(Serial);
                }
            }
            if (!Update.hasError())
            {
                // Reset the Watchdog timer
                esp_task_wdt_reset();
                Serial.print("X");
                if (Update.write(data, len) != len) 
                {
                    Update.printError(Serial);
                }
            }
            if (final) 
            {
                // Reset the Watchdog timer
                esp_task_wdt_reset();
                Serial.println(" DONE");
                shouldReboot = !Update.hasError();
                if (Update.end(true)) 
                {
                    Serial.printf("Update Success: %uB\n", index+len);
                } 
                else 
                {
                    Update.printError(Serial);
                }
            }
        }
        else
        {
            Serial.printf("Unknown update file type:%s, %u B\n", filename.c_str(), len);
            shouldReboot = false;
            if (final) 
            {
                request->send(FILESYS, "/index.html", String(), false, processor);
            }
        }
    });

    // Start server
    server.begin();
}
