#include <Arduino.h>

// Includes for LCD_I2C
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Defines for LCD
int lcdColumns = 16;
int lcdRows = 2;

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

// Includes for RFID
#include <SPI.h>
#include <MFRC522.h>

// Definitions for RFID R/W
#define SS_PIN 5
#define RST_PIN 0
#define NR_OF_READERS 2

MFRC522 rfid(SS_PIN, RST_PIN); // Create MFRC522 instance.
MFRC522::MIFARE_Key key;

// Init array that will store new NUID
byte nuidPICC[4];

// Definition of Data that will be written in the Tag / Card
int blockNum = 2;

// Change this name to desired name
byte blockData[16] = {"Tukiyem"};

byte bufferLen = 18;
byte readBlockData[18];
byte sector = 1;

MFRC522::StatusCode status;

/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
void printHex(byte *buffer, byte bufferSize)
{
  for (byte i = 0; i < bufferSize; i++)
  {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/**
 * Helper routine to dump a byte array as dec values to Serial.
 */
void printDec(byte *buffer, byte bufferSize)
{
  for (byte i = 0; i < bufferSize; i++)
  {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}

/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
void dump_byte_array(byte *buffer, byte bufferSize)
{
  for (byte i = 0; i < bufferSize; i++)
  {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/**************** Write Functions() ****************/

void writeDataToBlock(int blockNum, byte blockDatap[])
{
  // Authenticate using key A
  Serial.println(F("Authenticating using key A..."));
  status = (MFRC522::StatusCode)rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(rfid.uid));
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("Authentication Failed to Write: "));
    Serial.println(rfid.GetStatusCodeName(status));
    return;
  }
  else
  {
    Serial.println("Authentication Success");
  }

  /* Write Data to the block */
  Serial.print(F("Writing data into block "));
  Serial.print(blockNum);
  Serial.println(F(" ..."));
  dump_byte_array(blockData, 16);
  Serial.println();

  status = (MFRC522::StatusCode)rfid.MIFARE_Write(blockNum, blockData, 16);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(rfid.GetStatusCodeName(status));
  }
  else
  {
    Serial.println("Data was written into block successfully.");
  }

  Serial.println();
}

/**************** Write Functions() ****************/

void setup()
{
  Serial.begin(9600); // Initialize serial communications with the PC
  while (!Serial)
    ; // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)

  lcd.init();
  lcd.backlight();

  SPI.begin();     // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 card

  // Prepare the key (used both as key A and as key B)
  // using FFFFFFFFFFFFh which is the default at chip delivery from the factory
  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF;
  }

  Serial.println("Scan a RFID Tag to write data...");
  lcd.setCursor(0, 0);
  lcd.print("- Attendance System -");
  lcd.setCursor(0, 1);
  lcd.print("Scan a RFID Tag to write data...");
}

/**************** Read Functions() ****************/

void readDataFromBlock(int blockNum, byte readBlockData[])
{
  // Authenticate using key A
  Serial.println(F("Authenticating using key A..."));
  status = (MFRC522::StatusCode)rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(rfid.uid));
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("Authentication Failed to Write: "));
    Serial.println(rfid.GetStatusCodeName(status));
    return;
  }
  else
  {
    Serial.println("Authentication Success");
  }

  // Read data from the block (again, should now be what we have written)
  Serial.print(F("Reading data from block "));
  Serial.print(blockNum);
  Serial.println(F(" ..."));

  status = (MFRC522::StatusCode)rfid.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("MIFARE_Read() failed: "));
    Serial.println(rfid.GetStatusCodeName(status));
  }
  else
  {
    Serial.println("Successfully read data from block!");
  }

  Serial.print(F("Data in block "));
  Serial.print(blockNum);
  Serial.println(F(":"));
  dump_byte_array(readBlockData, 16);
  Serial.println();
}

void loop()
{

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if (!rfid.PICC_IsNewCardPresent())
    return;

  // Select one of the cards
  if (!rfid.PICC_ReadCardSerial())
    return;

  Serial.println("**Card Detected");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Card Detected!");

  // Show some details of the PICC (that is: the tag/card)
  Serial.print(F("Card UID:"));
  dump_byte_array(rfid.uid.uidByte, rfid.uid.size);

  Serial.println();

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check for compatibility
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI && piccType != MFRC522::PICC_TYPE_MIFARE_1K && piccType != MFRC522::PICC_TYPE_MIFARE_4K)
  {
    Serial.println(F("This sample only works with MIFARE Classic cards."));
    return;
  }

  /*---------------- Start to write data to block -----------------*/
  Serial.println("Writing data to block....");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Writing data to block...");
  writeDataToBlock(blockNum, blockData);
  /*---------------- Start to write data to block -----------------*/

  /*---------------- Start to read data to block -----------------*/
  Serial.println("Reading data from block...");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Reading data from block...");
  readDataFromBlock(blockNum, readBlockData);
  /*---------------- Start to read data to block -----------------*/

  /*---------------- Check the result of the block -----------------*/
  Serial.print("\n");
  Serial.print("Data in Block:");
  Serial.print(blockNum);
  Serial.print(" --> ");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Name Written:");
  for (int j = 0; j < 16; j++)
  {
    Serial.write(readBlockData[j]);
    lcd.setCursor(j, 1);
    lcd.write(readBlockData[j]);
  }
  /*---------------- Check the result of the block -----------------*/

  // Halt PICC
  rfid.PICC_HaltA();
  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}