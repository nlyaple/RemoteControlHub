/*
*/

#include "RemoteControl.h"
#include <ELECHOUSE_CC1101_SRC_DRV.h>

// This code uses the TI CC 1101 esclusively
#ifdef CC1101_RADIO    

//
// The following block of defines and constants are for describing the MinkaAire Aire Control remote
//
#define RT_303_MINKA_FAN        4   // "MinkaAire 6 speed fan with Light) (303 MHz)",

#define UNIT_MASK   0x1F
#define CMD_MASK    0x3F
#define UNIT_LEN    5
#define CMD_LEN     6

#define BUTTON_1    0x08
#define BUTTON_2    0x0A
#define BUTTON_3    0x10
#define BUTTON_4    0x18
#define BUTTON_5    0x22
#define BUTTON_6    0x20
#define BUTTON_STOP 0x02
#define BUTTON_DIR  0x04
#define BUTTON_LON  0x09
#define BUTTON_LOFF 0x01

const byte  minkaCmds[]    {BUTTON_1, BUTTON_2, BUTTON_3, BUTTON_4, BUTTON_5, BUTTON_6, BUTTON_STOP, BUTTON_DIR, BUTTON_LON, BUTTON_LOFF};

const char * fileName1 =     "Speed_1";
const char * fileName2 =     "Speed_2";
const char * fileName3 =     "Speed_3";
const char * fileName4 =     "Speed_4";
const char * fileName5 =     "Speed_5";
const char * fileName6 =     "Speed_6";
const char * fileNameS =     "Stop";
const char * fileNameD =     "Direction";
const char * fileNameL =     "Light_On";
const char * fileNameO =     "Light_Off";

const char * fileTable[] = {
    fileName1,
    fileName2,
    fileName3,
    fileName4,
    fileName5,
    fileName6,
    fileNameS,
    fileNameD,
    fileNameL,
    fileNameO,
};

/*
 * Minka Aire protocol, where data is encoded in every 3rd bit
 * The first 16 bits are the device address. Next is async byte of 0x06
 * Next is the command 16 bits with the last byte always 0x08.
 * 
 * The following is a bit mask of the protocol
 */
const byte  minkaProto[]    {0x92, 0x49, 0x64, 0x92, 0x48};

//
// End of MinkaAire specifics
//


//~~ Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*!
    \brief  Transmits stored the bit pattern for 303MHz devices
            remote type 4.
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
txDeviceProtocol(
    PDEVICE_PROTO    tx) 
{
    word    unitID;
    word    unitCmd;
    short   bitPos;
    byte    protoByte;
    byte    bitVal;
    byte    dataBit;
    
    Serial.print("Message Length: ");
    Serial.println(tx->packetByteLen);

    Serial.print("Unit bits: 0x");
    Serial.print(tx->unitNumber, HEX);
    unitID = tx->unitNumber << ((8 - tx->unitNumBits) + 8);
    Serial.print(", after shift: 0x");
    Serial.println(unitCmd, HEX);
    Serial.print("Cmd bits: 0x");
    Serial.print(tx->command, HEX);
    unitCmd = tx->command << ((8 - tx->cmdNumBits) + (8 - tx->unitNumBits));
    Serial.print(", after shift: 0x");
    Serial.println(unitCmd, HEX);
    unitCmd |= unitID;
    Serial.print("Unit & Cmd bits: 0x");
    Serial.println(unitCmd, HEX);
        
    ELECHOUSE_cc1101.setMHZ(tx->frequency);
    //ELECHOUSE_cc1101.setMHZ(303.89);

    ELECHOUSE_cc1101.SetTx();
    ELECHOUSE_cc1101.setPA(10);       // set TxPower. The following settings are possible depending on the frequency band.  (-30  -20  -15  -10  -6    0    5    7    10   11   12)   Default is max!
    digitalWrite(LED_GREEN, HIGH);   // turn on the Green LED 
    digitalWrite(RF_TX, LOW);       // turn off the transmitter 
    delay(250);
    // Insert the blank address command into the stream
    bitPos = 1;
    dataBit = 15;
    for (int idx = 0; idx < tx->packetByteLen ; idx++)
    {   
        protoByte = tx->protoData[idx];
        //Serial.print("Byte (");
        //Serial.print(idx);
        //Serial.print("): 0x");
        //Serial.print(protoByte, HEX);
        for (int8_t i = 7; i >= 0; i--) 
        {
            bitVal = 0;
            if (protoByte & (1 << i))
            {
                bitVal = 1;
            }
            else
            {
                if ((bitPos % 3) == 0)
                {
                    if (unitID & (1 << dataBit))
                    {
                        bitVal = 1;
                    }
                    dataBit--;
                }
            }
            //Serial.print(", ");
            //Serial.print(bitVal);
            digitalWrite(RF_TX, bitVal);
            delayMicroseconds(tx->bitCellTime);
            bitPos++;
            if (bitPos > tx->pulseCount)
                break;
        }
        //Serial.println("");
    }
    delay(20);
    for (int repeat = 0; repeat < 7; repeat++)
    {
        //Serial.print("Total pulses: ");
        //Serial.println(tx->pulseCount);
        
        bitPos = 1;
        dataBit = 15;
        for (int idx = 0; idx < tx->packetByteLen ; idx++)
        {   
            protoByte = tx->protoData[idx];
            //Serial.print("Byte (");
            //Serial.print(idx);
            //Serial.print("): 0x");
            //Serial.print(protoByte, HEX);
            for (int8_t i = 7; i >= 0; i--) 
            {
                bitVal = 0;
                if (protoByte & (1 << i))
                {
                    bitVal = 1;
                }
                else
                {
                    if ((bitPos % 3) == 0)
                    {
                        if (unitCmd & (1 << dataBit))
                        {
                            bitVal = 1;
                        }
                        dataBit--;
                    }
                }
                //Serial.print(", ");
                //Serial.print(bitVal);
                digitalWrite(RF_TX, bitVal);
                delayMicroseconds(tx->bitCellTime);
                bitPos++;
                if (bitPos > tx->pulseCount)
                    break;
            }
            //Serial.println("");
        }
        delay(20);
    }
    delay(500);
    digitalWrite(RF_TX, LOW);       // turn off the transmitter 
    digitalWrite(LED_GREEN, LOW);   // turn on the Green LED 
    ELECHOUSE_cc1101.setSidle();
}


//~~ Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*!
    \brief  Write the contents of wrData to the file name indicated
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int
createMinkeRemoteFiles(
    char *      deviceName,
    byte        unitNumber
)
{
    char    scratchBuf[256];
    PDEVICE_PROTO   pTx;

    pTx = (PDEVICE_PROTO)malloc(sizeof(DEVICE_PROTO) + 32);
    if (pTx != NULL)
    {
        pTx->pulseCount = 40;
        pTx->remoteType = 4;
        pTx->frequency = 303.882;
        pTx->bitCellTime = 325;
        pTx->unitNumber = unitNumber;
        pTx->unitNumBits = UNIT_LEN;
        pTx->cmdNumBits = CMD_LEN;
        pTx->packetByteLen = 5;
        for (int i = 0; i < pTx->packetByteLen; i++)
        {
            pTx->protoData[i] = minkaProto[i];
        }
        for (int i = 0; i < 10; i++)
        {
            convertDeviceAndActionNameToFile(deviceName, fileTable[i], (char *)scratchBuf);
            Serial.print("CMD: ");
            Serial.print(minkaCmds[i], HEX);
            pTx->command = minkaCmds[i];
            Serial.print(", File: ");
            Serial.println((char *)scratchBuf);
            fileWrite((char *)scratchBuf, (byte *)pTx, (sizeof(DEVICE_PROTO) + pTx->packetByteLen));
        }
        updateDevicesFile(deviceName, 4, 1);
        free(pTx);
    }
    return 0;
}



//~~ Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*!
    \brief  
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int
convertDeviceAndActionNameToFile(
    const char *    src,
    const char *    src2,
    char *  dst)
{
    int     noChars = 0;
    int     srcLen = strlen(src);
    
    *dst++ = '/';   // Add in the path delimiter
    for (int i=0; i < srcLen; i++)
    {
        // Convert spaces to underscores
        if (*src == ' ')
        {
            *dst++ = '_';
            src++;
            noChars++;
        }
        else if (*src != 0)
        {
            *dst++ = *src++;
            noChars++;
        }
        else
        {
            break;
        }
    }
    *dst++ = '~';
    *dst = 0;
    strcat(dst, src2);
    noChars += strlen(src2);
    strcat(dst, fileType);
    noChars += strlen(fileType);
    return noChars;
}

#endif // CC1101_RADIO    
