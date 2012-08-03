/**************************************************************************/
/*! 
    @file     mifareclassic_format_long.ino
    @author   Odopod (based on work by Adafruit Industries)
	@license  BSD (see license.txt)

    This example attempts to format a Mifare Classic
    card for NDEF Records and writes an NDEF URI or MIME Record (see comments)
    
    Note, this sketch require a fork of the Adafruit PN532 library found here: 
        https://github.com/davidbliss/Adafruit_NFCShield_I2C
   
    Note that you need the baud rate to be 115200 because we need to print
	out the data and read from the card at the same time!

This is an example sketch for the Adafruit PN532 NFC/RFID breakout boards
This library works with the Adafruit NFC breakout 
  ----> https://www.adafruit.com/products/364
 
Check out the links above for our tutorials and wiring diagrams 
These chips use SPI to communicate, 4 required to interface

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!
*/
/**************************************************************************/
#include <Adafruit_NFCShield_I2C.h>
#include <Wire.h>

#define IRQ   (2)
#define RESET (3)  // Not connected by default on the NFC Shield

Adafruit_NFCShield_I2C nfc(IRQ, RESET);
uint8_t url[ ] = "odopod.com/blog/planning-ness-2012-connected-personal-objects/";
uint8_t bitmapdata[220] = {0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x12, 0x00, 0x12, 0x00, 0xb3, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xff, 0xff, 0x99, 0xff, 0xcc, 0x99, 0xff, 0xcc, 0x66, 0xff, 0xcc, 0x33, 0xcc, 0x99, 0x33, 0xcc,0x99, 0x00, 0x99, 0x66, 0x00, 0x66, 0x66, 0x00, 0x66, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x0a, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00, 0x00, 0x21, 0xf9, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x12, 0x00, 0x12, 0x00, 0x00, 0x04, 0x7c, 0x10, 0xc8, 0x99, 0x6a, 0x45, 0x33, 0x53, 0x74, 0x8a, 0x37, 0x07, 0x92, 0x68, 0x52, 0x72, 0x10, 0x43, 0x9a, 0x12, 0x20, 0x96, 0x25, 0x45, 0x2a, 0x04, 0xb4, 0xb0, 0x1a, 0x63, 0x79, 0xc8, 0x74, 0xaf, 0x12, 0x87, 0x1c, 0x02, 0x35, 0x53, 0x28, 0x68, 0xc6, 0x1b, 0xc6, 0x94, 0x42, 0x1e, 0x03, 0x46, 0xdb, 0x00, 0x58, 0x31, 0x0c, 0x66, 0xd0, 0x67, 0xf4, 0x56, 0x1d, 0xf4, 0x8c, 0xce, 0x5f, 0xa1, 0x8b, 0xed, 0x09, 0xa4, 0xab, 0x71, 0xc2, 0x40, 0xac, 0xa9, 0x7e, 0xac, 0xca, 0x69, 0xa0, 0x78, 0xbf, 0x15, 0x04, 0x2a, 0x00, 0xc1, 0xa6, 0xdb, 0xe9, 0x79, 0x06, 0x2e, 0x26, 0x79, 0x74, 0x46, 0x87, 0x79, 0x7a, 0x12, 0x7c, 0x89, 0x8d, 0x89, 0x38, 0x1a, 0x26, 0x6c, 0x8e, 0x20, 0x39, 0x91, 0x1c, 0x06, 0x99, 0x2d, 0x96, 0x24, 0x00, 0x16, 0x16, 0x24, 0x11, 0x00, 0x3b};
uint8_t mimetype[ ] = "image/gif";

void setup(void) {
  Serial.begin(115200);
  Serial.println("Looking for PN532...");

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // configure board to read RFID tags
  nfc.SAMConfig();
  
  Serial.println("");
  Serial.println("Place your Mifare Classic card on the reader to format with NDEF");
  Serial.println("and press any key to continue ...");
  // Wait for user input before proceeding
  Serial.flush();
  while (!Serial.available());
  Serial.flush();
}

void loop(void) {
  uint8_t success;                          // Flag to check if there was an error with the PN532
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  bool authenticated = false;               // Flag to indicate if the sector is authenticated

  // Use the default key
  uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    
  // Wait for an ISO14443A type card (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  
  if (success) 
  {
    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    Serial.println("");
    
    // Make sure this is a Mifare Classic card
    if (uidLength != 4)
    {
      Serial.println("Ooops ... this doesn't seem to be a Mifare Classic card!"); 
      return;
    }
    
    // We probably have a Mifare Classic card ... 
    Serial.println("Seems to be a Mifare Classic card (4 byte UID)");

    // Try to format the card for NDEF data
    success = nfc.mifareclassic_AuthenticateBlock (uid, uidLength, 0, 0, keya);
    if (!success)
    {
      Serial.println("Unable to authenticate block 0 to enable card formatting!");
      return;
    }
    success = nfc.mifareclassic_FormatNDEF();
    if (!success)
    {
      Serial.println("Unable to format the card for NDEF");
      return;
    }
    
    Serial.println("Card has been formatted for NDEF data using MAD1");
    
    
//  write a ndef long with the defined url (uncomment to test)
//    success = nfc.mifareclassic_WriteNDEFURI_Long(NDEF_URIPREFIX_HTTP_WWWDOT, url, keya, uid, uidLength);

//  write a mime payload - need to prepend the mimetype string to the front of the image bytearray data
    //create a new array with a defined size - bitmapdata + mimetype string
    const uint8_t len = sizeof(bitmapdata) + sizeof(mimetype);
    uint8_t payload[len];
    //concat into one array 
    memcpy(payload, mimetype, sizeof(mimetype)-1);
    memcpy(payload+sizeof(mimetype)-1, bitmapdata, sizeof(bitmapdata)); 
    //write it
    success = nfc.mifareclassic_WriteNDEFMIME(payload, len, keya, uid, uidLength);
    
    
    
    if (success)
    {
      Serial.println("Record written :)");
    }
    else
    {
      Serial.println("Record failed! :(");
    }
  }
  
  // Wait a bit before trying again
  Serial.println("\n\nDone!");
  Serial.flush();
  while (!Serial.available());
  Serial.flush();
}
