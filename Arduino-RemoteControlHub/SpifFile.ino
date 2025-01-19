/*
 *    Nelsony.com - FILESYS File functions
 *    
 *    Last update 5/4/2022
*/

#include "COMMS.h"


//~~ Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*!
    \brief  Build directory list as JSON response
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int
listDir(
    char *  filterBy,
    char *  responseBuf,
    int     respBufLen)
{
    const char *  src;
    char *  pch = NULL;
    int     srcLen = 0;
    int     entries = 0;
    
    Serial.println("directory: ");

    File root = FILESYS.open("/");
    
    if (!root){
        Serial.println("- failed to open directory");
        return 0;
    }
    if (!root.isDirectory()){
        Serial.println(" - not a directory");
        return 0;
    }
    if ((filterBy != NULL) &&
        (responseBuf != NULL) && 
        (respBufLen > 2))
    {
        pch = responseBuf;
        //strcpy(responseBuf, "[");
        *pch++ = '[';
        respBufLen -= 2;
        File file = root.openNextFile();
        while (file)
        {
            if (!file.isDirectory())
            {
                src = file.name();
                srcLen = strlen(file.name());
                if (srcLen < respBufLen)
                {
                    //Serial.print("  FILE: ");
                    //Serial.println(file.name());
                    if (strstr(src, filterBy) != NULL)
                    {
                        src++;
                        *pch++ = 0x22;  // Double quote
                        for (int i=0; i < srcLen; i++)
                        {
                            if (*src == '_')
                            {
                                *pch++ = ' ';
                                src++;
                            }
                            else if (*src != '.')
                                *pch++ = *src++;
                            else
                                break;
                        }
                        *pch++ = 0x22;  // Double quote
                        *pch++ = ',';
                        entries++;
                    }
                }
            }
            file = root.openNextFile();
        }
        if (entries)
            pch--;
        *pch++ = ']';
        *pch++ = 0;
        Serial.print("result: ");
        Serial.println(responseBuf);
        return entries;
    }
    else
    {
        File file = root.openNextFile();
        while (file)
        {   
            if (file.isDirectory()){
                Serial.print("  DIR : ");
                Serial.println(file.name());
            } 
            else 
            {
                if (filterBy != NULL) 
                {
                    src = file.name();
                    if (strstr(src, filterBy) != NULL)
                    {
                        Serial.print("  FILE: ");
                        Serial.print(file.name());
                        Serial.print("\tSIZE: ");
                        Serial.println(file.size());
                        srcLen += strlen(file.name());
                    }
                }
                else
                {
                    Serial.print("  FILE: ");
                    Serial.print(file.name());
                    Serial.print("\tSIZE: ");
                    Serial.println(file.size());
                    srcLen += strlen(file.name());
                }
            }
            file = root.openNextFile();
        }
    }
    return srcLen;
}

//~~ Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*!
    \brief  Write the contents of wrData to the file name indicated
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int
fileWrite(
    char *          fileName,
    byte *          wrData,
    int             dataLen)
{
    File file = FILESYS.open(fileName, FILE_WRITE);
    
    if (file && !file.isDirectory())
    {
        if (file.write(wrData, dataLen))
        {
            Serial.println("File was written");
        } 
        else 
        {
            Serial.println("File write failed, file: ");
            Serial.println(fileName);
        }
        file.close();
    }
    else
    {
        Serial.print("There was an error opening the file for writing, file: ");
        Serial.println(fileName);
        return -1;
    }        
    return 0;
}


#if 0
//~~ Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*!
    \brief  Attempts to read the file indicated into rdData buffer
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int
fileRead(
    char *      fileName,
    byte *      rdData,
    int         bufLen)
{
    int     bytesRead = 0;

//#ifdef USE_LittleFS
//#else   
    File file = FILESYS.open(fileName, FILE_READ);
//#endif // USE_LittleFS vs FILESYS
    
    if (file && !file.isDirectory())
    {
        if (file.size() > bufLen)
        {
            file.close();
            Serial.print("The buffer was too small for reading, file: ");
            Serial.println(fileName);
            return 0;
        }
        bytesRead = file.readBytes((char *)rdData, bufLen);
        if (bytesRead > 0)
        {
            Serial.print("File read bytes: ");
            Serial.println(bytesRead);
            rdData[bytesRead] = 0;
        } else {
            Serial.print("File read failed, file: ");
            Serial.println(fileName);
        }
        file.close();
    }
    else
    {
        Serial.print("There was an error opening the file for reading, file: ");
        Serial.println(fileName);
        return -1;
    }        
    return bytesRead;
}
#endif // 0


//~~ Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*!
    \brief  Attempts to read the file indicated into buffer allocated based 
            on the file size.  
    \Notice The buffer must be freed after the usage is completed.
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
byte *
fileReadBuffer(
    char *      fileName,
    int *       bufLen)
{
    byte *  pReadBuffer = NULL;
    int     reqLen = 0;
    int     bytesRead = 0;
    
    Serial.print("Read file: ");
    Serial.println(fileName);

    File file = FILESYS.open(fileName, FILE_READ);
    if (file && !file.isDirectory())
    {
        reqLen = file.size() + 4;
        Serial.print("Allocating buffer size: ");
        Serial.println(reqLen);
        pReadBuffer = (byte *)malloc(reqLen);
        if (pReadBuffer == NULL)
        {
            file.close();
            Serial.print("The buffer allocation failed, file: ");
            Serial.println(fileName);
        }
        else
        {
            bytesRead = file.readBytes((char *)pReadBuffer, reqLen);
            if (bytesRead > 0)
            {
                Serial.print("File read bytes: ");
                Serial.println(bytesRead);
                pReadBuffer[bytesRead] = 0;
                *bufLen = bytesRead;
            } else {
                Serial.print("File read failed, file: ");
                Serial.println(fileName);
            }
            file.close();
        }
    }
    else
    {
        Serial.print("There was an error opening the file for reading, file: ");
        Serial.println(fileName);
    }        
    return pReadBuffer;
}


//~~ Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*!
    \brief  Attempts to remove files that containd the name
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int
removeFiles(
    char *      fileName)
{
    const char *  pSrc;
    int     entries = 0;
    
    File root = FILESYS.open("/");

    if (!root){
        Serial.println("- failed to open directory");
        return 0;
    }
    if (!root.isDirectory()){
        Serial.println(" - not a directory");
        return 0;
    }
    if (fileName != NULL) 
    {
        File file = root.openNextFile();
        while (file)
        {
            if (!file.isDirectory())
            {
                pSrc = file.name();
                if (strstr(pSrc, fileName) != NULL)
                {
                    Serial.print("  FILE: ");
                    Serial.print(pSrc);
                    if (FILESYS.remove(pSrc))
                    {
                        Serial.println("- file deleted");
                        entries++;
                    } 
                    else 
                    {
                        Serial.println("- delete failed");
                    }                
                }
            }
            file = root.openNextFile();
        }
    }
    return entries;
}

//~~ Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*!
    \brief  Check for any updated files on reboot
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int
checkForFileUpdates()
{
    const char *  src;
    char *  pch = NULL;
    int     srcLen;
    char    fileName[FILE_NAME_SZ];
    char *  pFileName = &fileName[0];
    
    File root = FILESYS.open("/");

    if (!root){
        Serial.println("- failed to open directory");
        return 0;
    }
    if (!root.isDirectory()){
        Serial.println(" - not a directory");
        return 0;
    }
    File file = root.openNextFile();
    while (file)
    {
        if (!file.isDirectory())
        {
            src = file.name();
            if (strstr(src, "-update") != NULL)
            {
                strncpy(pFileName, src, FILE_NAME_SZ);
                pch = strstr(pFileName, "-update");
                *pch = 0;
                Serial.print("Attempting to delete file: ");
                Serial.println(pFileName);
                if (FILESYS.remove(pFileName))
                {
                    Serial.println("- file deleted");
                } 
                else 
                {
                    Serial.println("- delete failed");
                }                
                Serial.print("Renaming file : ");
                Serial.print(src);
                Serial.print(" to: ");
                Serial.println(pFileName);
                FILESYS.rename(src, pFileName);    
            }
            file = root.openNextFile();
        }
    }
    return 0;
}
