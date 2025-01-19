/*
   Setup for 433 MHz remotes with detection and transmission rotines
    
*/
#include "RemoteControl.h"
#include <ELECHOUSE_CC1101_SRC_DRV.h>

#define RT_433_3_BTN_UP     1   // 3 Button - Up/Stop/Down (433MHz)
#define RT_433_3_BTN_OPEN   2   // 3 Button - Open/Stop/Close (433 MHz)
#define RT_433_6_BTN        3   // 6 Button - Open/Stop/Close/Partial open (433 MHz)
#define RT_433_6_BTN_UP     5   // 6 Button - Up, Stop, Down, Tilt Up, Stop Tilt, Tilt Down


//DEVICE_CODE     rxTest;
DEVICE_CODE     txCode;

bool        foundCC1101 = false;


//~~ Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*!
    \brief  Transmits stored the bit pattern for 433MHz devices
            remote types 1 - 3.
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
txDeviceCode433(
    PDEVICE_CODE    tx) 
{
    int     repeatCnt = 4;

    if (tx->remoteType == 5)
        repeatCnt = 1;
        
    if (foundCC1101)
    {
        ELECHOUSE_cc1101.setMHZ(433.92);
        ELECHOUSE_cc1101.SetTx();
        ELECHOUSE_cc1101.setPA(10);       // set TxPower. The following settings are possible depending on the frequency band.  (-30  -20  -15  -10  -6    0    5    7    10   11   12)   Default is max!
    }
    digitalWrite(LED_GREEN, HIGH);   // turn on the Green LED 
    digitalWrite(RF_TX, LOW);       // turn off the transmitter 
    delay(250);
    for (int repeat = 0; repeat < repeatCnt; repeat++)
    {
        Serial.print("Total pulses: ");
        Serial.println(tx->pulseCount);
        for (int i=0; i < tx->pulseCount; i++)
        {
            digitalWrite(RF_TX, tx->bits[i].POLARITY);
            delayMicroseconds(tx->bits[i].CELL_TIME);
        }
        delay(500);
    }
    digitalWrite(RF_TX, LOW);       // turn off the transmitter 
    digitalWrite(LED_GREEN, LOW);   // turn on the Green LED 
    if (foundCC1101)
        ELECHOUSE_cc1101.setSidle();
    return; // 0;
}


//~~ Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*!
    \brief  Transmits stored the bit pattern for 433MHz devices
            remote types 1 - 3.
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
txDeviceCodeEx433(
    PDEVICE_CODE_EX     tx) 
{
    if (foundCC1101)
    {
        ELECHOUSE_cc1101.setMHZ(tx->frequency); 
        ELECHOUSE_cc1101.SetTx();
        ELECHOUSE_cc1101.setPA(10);       // set TxPower. The following settings are possible depending on the frequency band.  (-30  -20  -15  -10  -6    0    5    7    10   11   12)   Default is max!
    }
    digitalWrite(LED_GREEN, HIGH);   // turn on the Green LED 
    digitalWrite(RF_TX, LOW);       // turn off the transmitter 
    delay(250);
    for (int repeat = 0; repeat < tx->repeatCnt; repeat++)
    {
        Serial.print("Total pulses: ");
        Serial.println(tx->pulseCount);
        for (int i=0; i < tx->pulseCount; i++)
        {
            digitalWrite(RF_TX, tx->bits[i].POLARITY);
            delayMicroseconds(tx->bits[i].CELL_TIME);
        }
        delay(500);
    }
    digitalWrite(RF_TX, LOW);       // turn off the transmitter 
    digitalWrite(LED_GREEN, LOW);   // turn on the Green LED 
    if (foundCC1101)
        ELECHOUSE_cc1101.setSidle();
    return; // 0;
}


//~~ Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*!
    \brief  Reads the file and calls the function to do the actual transmit
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int
transmitCode(
    PACTION     pAction)
{
    PDEVICE_CODE    txBuf = NULL;
    int     bytesRead = 0;
    
    //bytesRead = fileRead(pAction->devFileName, (byte *)&txCode, sizeof(DEVICE_CODE));
    txBuf = (PDEVICE_CODE)fileReadBuffer(pAction->devFileName, (int *)&bytesRead);
    if ((txBuf != NULL) && (bytesRead > 0))
    {
        Serial.print("Reading file: ");
        Serial.print(pAction->devFileName);
        Serial.print(", bytes: ");
        Serial.print(bytesRead);
        Serial.print(", remote type: ");
        Serial.println(txBuf->remoteType);
#ifdef HELTEC
        Heltec.display->clear();
        Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
        Heltec.display->drawString(64, 0, "Transmiting");
        Heltec.display->display();
#else
        display.clear();
        display.setTextAlignment(TEXT_ALIGN_CENTER);
        display.drawString(64, 0, "Transmiting");
        display.display();
#endif    
        lastUpdateTime = millis();
        if (txBuf->remoteType == 4)
        {
            txDeviceProtocol((PDEVICE_PROTO)txBuf);
        }
        else // All but remoteType 4)
        {
            txDeviceCode433(txBuf);
        }
        clearDisplay = true;
        free(txBuf);
    }
    return 0;
}


//~~ Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*!
    \brief  Attempts to detect (sense) RF Remote and store the bit pattern
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int
detectDevice433(
    PDEVICE_CODE    rx) 
{
    unsigned long   readStartTime;
    unsigned long   elapsedTime;
    byte    tempBit = 1;
    byte    lastBit = 0;

    if (foundCC1101)
    {
        ELECHOUSE_cc1101.setMHZ(433.92);
        ELECHOUSE_cc1101.SetRx();       // set Receive on
        Serial.println("CC1101 Receiver waiting on 433MHz...");
    }
    for (int i=0; i < ARRAY_DEPTH; i++)
    {
        // Clear the receive buffer
        rx->bits[i].CELL_TIME = 0;
        rx->bits[i].POLARITY = 0;
    }
    rx->pulseCount = 0;
    Serial.println("Receiver waiting on 433MHz...");
    digitalWrite(LED_GREEN, HIGH);   // turn on the Green LED 
    readStartTime  = micros();
    while (true)
    {
        //digitalWrite(RF_CS, digitalRead(RF_RX)); // Scope trigger
        if (digitalRead(RF_RX) == tempBit)
        {
            //readStartTime  = micros();
            while (true)
            {
                // Wait for the transition and debounce the response
                if (digitalRead(RF_RX) != tempBit)
                {
                    if (digitalRead(RF_RX) != tempBit)
                    {
                        //digitalWrite(RF_CS, digitalRead(RF_RX)); // Scope trigger
                        if (digitalRead(RF_RX) != tempBit)
                            break;
                    }
                }
            }
            elapsedTime = micros() - readStartTime;
            if ((elapsedTime > 100) &&      // was 100
                (elapsedTime < 50000) &&
                (tempBit != lastBit))
            {
                if (rx->pulseCount < ARRAY_DEPTH)
                {
                    rx->bits[rx->pulseCount].POLARITY = tempBit;
                    rx->bits[rx->pulseCount].CELL_TIME = elapsedTime;
                    readStartTime  = micros();
                    lastBit = tempBit;
                    rx->pulseCount++;
                    tempBit ^= 1;
                }
            }
            else
            {
                if (rx->pulseCount > 150)
                {
                    dumpPulses(rx);
                    break;
                }   
                // Reset the start time
                //Serial.println("Resetting...");
                readStartTime  = micros();
                tempBit = 1;
                lastBit = 0;
                rx->pulseCount = 0;
            }
        }
        if (cancelDetecting)
        {
            cancelDetecting = false;
            rx->pulseCount = 0;
            break;
        }
        readStartTime  = micros();
    }
    if (foundCC1101)
        ELECHOUSE_cc1101.setSidle();
    digitalWrite(LED_GREEN, LOW);   // turn on the Green LED 
    Serial.println("Receiver captured device.");
    Serial.print("Total pulses: ");
    Serial.println(rx->pulseCount);

    return rx->pulseCount;
}

//~~ Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*!
    \brief  Function to detect any remotes and save to a file if successful
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int
detectDevice(
    PACTION     pAction)
{
    if (detectDevice433(&txCode) > 150)
    {
        txCode.remoteType = pAction->remoteType;
        Serial.print("Saving to file: ");
        Serial.println(pAction->devFileName);
        fileWrite(pAction->devFileName, (unsigned char *)&txCode, sizeof(DEVICE_CODE));
        return 0;
    }
    return -1;
}


//~~ Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*!
    \brief  Function to initialize the Software Defined Radio (if attached)
*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void 
initializeRadio() 
{
    pinMode(RF_RX, INPUT);
    pinMode(RF_CS, OUTPUT);
    pinMode(RF_TX, OUTPUT);
    digitalWrite(RF_CS, HIGH);      // Enable the rx
    digitalWrite(RF_TX, LOW);       // turn off the transmitter 

    if (ELECHOUSE_cc1101.getCC1101())
    {    // Check the CC1101 Spi connection.
        Serial.println("CC1101 Connection OK");
        //CC1101 Settings:                (Settings with "//" are optional!)
        ELECHOUSE_cc1101.Init();            // must be set to initialize the cc1101!
        //ELECHOUSE_cc1101.setRxBW(812.50);  // Set the Receive Bandwidth in kHz. Value from 58.03 to 812.50. Default is 812.50 kHz.
        //ELECHOUSE_cc1101.setPA(10);       // set TxPower. The following settings are possible depending on the frequency band.  (-30  -20  -15  -10  -6    0    5    7    10   11   12)   Default is max!
        ELECHOUSE_cc1101.setMHZ(433.92); // Here you can set your basic frequency. The lib calculates the frequency automatically (default = 433.92).The cc1101 can: 300-348 MHZ, 387-464MHZ and 779-928MHZ. Read More info from datasheet.
        foundCC1101 = true;
    }
    else
    {
        Serial.println("CC1101 Connection Failed");
    }
}


#if 1
void
dumpPulses(
    PDEVICE_CODE    rx)
{
    unsigned long   t;
    int             cCnt = 0;
    
    Serial.print("Total pulses: ");
    Serial.println(rx->pulseCount);
    for (int i=0; i < rx->pulseCount; i++)
    {
        t = rx->bits[i].CELL_TIME;
        if (t > 50)
        {
            while (t > 50)
            {
                if (rx->bits[i].POLARITY)
                    Serial.print("-");
                else
                    Serial.print("_");
                t -= 50;
                cCnt++;
                if ((cCnt % 160) == 0)
                {
                    Serial.println("");
                    Serial.print("  ");
                }
            }
        }
        else
        {
            if (rx->bits[i].POLARITY)
                Serial.print("-");
            else
                Serial.print("_");
        }
    }
    Serial.println("");

#ifdef TEXT_PRINT        
    for (int i=0; i < rx->pulseCount; i++)
    {
        Serial.print("#");
        Serial.print(i);
        Serial.print(", T:");
        Serial.print(rx->bits[i].CELL_TIME);
        if (rx->bits[i].POLARITY)
            Serial.print(", HI  ");
        else
            Serial.print(", LO  ");
        if ((i != 0) && ((i % 10) == 0))
            Serial.println("");
    }
    Serial.println("");
#endif // TEXT_PRINT            
}
#endif // 0
