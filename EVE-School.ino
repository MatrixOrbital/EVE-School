#include <stdint.h>         // Find types like "uint8_t"
#include <stdbool.h>        // Find type "bool"
#include <SPI.h>            // Arduino SPI library
#include <SD.h>             // Arduino SD card library


#include "Eve2_81x.h"       // Matrix Orbital EVE Library      
#include "Arduino_AL.h"     // Hardware abstraction layer for Arduino
#include "process.h"        // More application level code
#include "MatrixEve2Conf.h" // Display select

// The file of type File and named myFile is global and used for any and all files.  
// It is used for saving data on the SD card, such as touch calibration and image data.
// Your job is to make sure you are not trying to open it twice.
File myFile;

// A global buffer for use with debug messages.  It is always used up immediately so 
// never retains initialized values.  Prevent further allocations of RAM for logging
char LogBuf[LogBufSize];

void setup()
{
  // Initializations.  Order is important
  GlobalInit();                                     // EVE display interface initialization
  FT81x_Init(DISPLAY_43, BOARD_EVE2, TOUCH_TPC);    // Reset and initialize the EVE
  SD_Init();

  if (!LoadTouchMatrix())
  {
    // We failed to read calibration matrix values from the SD card.
    
    SelectScreen(SCR_Calibrate);
    
    if( SaveTouchMatrix() )  // If it saved, then
      LoadTouchMatrix();     // read it back from the file to compare values
    SelectScreen(SCR_FTDI);  // Go back to the initial screen
  }
  else
  {
    SelectScreen(SCR_FTDI); // Draw initial screen
  }

  // Slowly turn up the backlight for "dramatic effect"
  for(uint8_t duty = 0; duty <= 128; duty++)
  {
    wr8(REG_PWM_DUTY + RAM_REG, duty);      // Set backlight brightness
    HAL_Delay(15);
  }
}

void loop()
{
  MainLoop(); // jump to "main()"
}

// MainLoop is called from loop() and it never leaves (which is better than loop() which is called repeatedly)
// Note that if you use loop() instead, you will need to define variables "static" (since loop() is not actually a loop).
// In this infinite loop we are responding to many different signals including: Virtual key presses on screens, Real
// key presses of Real Keys, Slider touch, and timeouts.  We have to keep track of which screen we are on.
void MainLoop(void)
{
  uint8_t Tag = 0;
  uint8_t Key = 0;
  uint8_t ScreenNumber = SCR_FTDI;
  uint64_t TouchTimeout = 100;
  uint64_t Time2CheckKeys = 100;

  while(1)
  {
    if (millis() > TouchTimeout)
    {
      Tag = rd8(REG_TOUCH_TAG + RAM_REG);                    // Check for touches
      if(Tag)
      {
        Log("Touch Tag %d", Tag);
        switch (Tag)
        {
        case 1:
        case 2:
          SelectScreen(SCR_FTDI);
          ScreenNumber = SCR_FTDI;
          break;
        case 10:                                             // Sound Demo Screen (Makescreen_Button)
          Log(" - 10\n");
          SetPin(EveAudioEnable_PIN, HIGH);                  // Enable Audio
          wr8(REG_VOL_SOUND + RAM_REG, 0xFF);                // Set the volume to maximum
          wr16(REG_SOUND + RAM_REG, 0x4841);                 // Select Xylophone note C3
          wr8(REG_PLAY + RAM_REG, 1);                        // Play the sound
           break;
        case 11:                                             // Sound Demo Screen (Makescreen_Button) 
          Log(" - 11\n");
          SetPin(EveAudioEnable_PIN, HIGH);                  // Enable Audio
          wr8(REG_VOL_SOUND + RAM_REG, 0xFF);                // Set the volume to maximum
          wr16(REG_SOUND + RAM_REG, 0x7241);                 // Select Xylophone note C5
          wr8(REG_PLAY + RAM_REG, 1);                        // Play the sound
          break;
        case 12:                                             // Sound Demo Screen (Makescreen_Button) 
          Log(" - 12\n");
          SetPin(EveAudioEnable_PIN, HIGH);                  // Enable Audio
          wr8(REG_VOL_SOUND + RAM_REG, 0xFF);                // Set the volume to maximum
          wr16(REG_SOUND + RAM_REG, 0x4146);                 // Select Piano note F2
          wr8(REG_PLAY + RAM_REG, 1);                        // Play the sound
          break;
        case 13:                                             // Sound Demo Screen (Makescreen_Button) 
          Log(" - 13\n");
          SetPin(EveAudioEnable_PIN, HIGH);                  // Enable Audio
          wr8(REG_VOL_SOUND + RAM_REG, 0xFF);                // Set the volume to maximum
          wr16(REG_SOUND + RAM_REG, 0x5346);                 // Select Piano note F3
          wr8(REG_PLAY + RAM_REG, 1);                        // Play the sound
          break;
        default:                                             // Invalid tag value (importantly includes value 255)
          Log("\n");
          break;                                             // unrequired break
        } 
        while(rd8(REG_PLAY + RAM_REG));                      // Wait until sound finished
        SetPin(EveAudioEnable_PIN, LOW);                     // Disable Audio
      }
      TouchTimeout = millis() + 20;                          // This sets the sample rate for slider finger movement as well
    }


    if (millis() > Time2CheckKeys)
    {
      if(Key)              // If there was a key and there has not yet been a no-key read
      {
        Key = CheckKeys(); // When previous key released, this resets to zero
      }
      else
      {
        Key = CheckKeys();
        switch (Key)
        {
        case 0: break; // No key seen
        case 1:
          Log("Key 1\n");
          ScreenNumber++; 
          if(ScreenNumber > SCR_RAW) ScreenNumber = SCR_FTDI;
          SelectScreen(ScreenNumber);
          break;
        case 2:
          Log("Key 2\n");
          ScreenNumber--; 
          if(ScreenNumber < SCR_FTDI) ScreenNumber = SCR_RAW;
          SelectScreen(ScreenNumber);
          break;
        case 3:
          Log("Key 3\n");
          switch(ScreenNumber)
          {
          case SCR_FTDI: // we are on the initial screen so this is for calibrate
            SelectScreen(SCR_Calibrate); // Doing a touch calibration
            SaveTouchMatrix();
            SelectScreen(SCR_FTDI);  // So, we go back to the initial screen
            break;
          default:
            break;
          }
          break;
        }
      }
      Time2CheckKeys = millis() + 100; // Key check interval
    }
  }
}

// ************************************************************************************
// Following are wrapper functions for C++ Arduino functions so that they may be      *
// called from outside of C++ files.  These are also your opportunity to use a common *
// name for your hardware functions - no matter the hardware.  In Arduino-world you   *
// interact with hardware using Arduino built-in functions which are all C++ and so   *
// your "abstraction layer" must live in this xxx.ino file where C++ works.           *
//                                                                                    *
// Incidentally, this is also an alternative to ifdef-elif hell.  A different target  *
// processor or compiler will include different files for hardware abstraction, but   *
// the core "library" files remain unaltered - and clean.  Applications built on top  *
// of the libraries need not know which processor or compiler they are running /      *
// compiling on (in general and within reason)                                        *
// ************************************************************************************

void GlobalInit(void)
{
  Serial.begin(115200);
  while (!Serial) {;}                    // Wait for serial port to connect.
  
  // Keys Initialization
  Init_Keys();

  // Matrix Orbital Eve display interface initialization
  pinMode(EvePDN_PIN, OUTPUT);            // Pin setup as output for EVE PDN pin.
  digitalWrite(EvePDN_PIN, LOW);          // Apply a resetish condition on EVE
  pinMode(EveChipSelect_PIN, OUTPUT);     // SPI CS Initialization
  digitalWrite(EveChipSelect_PIN, HIGH);  // Deselect EVE
  pinMode(EveAudioEnable_PIN, OUTPUT);    // Audio Enable PIN
  digitalWrite(EveAudioEnable_PIN, LOW);  // Disable Audio
  SPI.begin();                            // Enable SPI
}

void Init_Keys(void)
{
  // Pin setup as input for each button pin
  pinMode(Button1_PIN, INPUT);
  pinMode(Button2_PIN, INPUT);
  pinMode(Button3_PIN, INPUT);
}

void DebugPrint(char *str)
{
  Serial.print(str);
}

// An abstracted pin write that may be called from outside this file.
void SetPin(uint8_t pin, bool state)
{
  digitalWrite(pin, state); 
}

// An abstracted pin read that may be called from outside this file.
uint8_t ReadPin(uint8_t pin)
{
  return(digitalRead(pin));
}

void SD_Init(void)
{
//  Log("Initializing SD card...\n");
  if (!SD.begin(SDChipSelect_PIN)) 
  {
    Log("SD initialization failed!\n");
    return;
  }
  Log("SD initialization done\n");
}

// Read the touch digitizer calibration matrix values from the Eve and write them to a file
bool SaveTouchMatrix(void)
{
  uint8_t count = 0;
  uint32_t data;
  uint32_t address = REG_TOUCH_TRANSFORM_A + RAM_REG;
  
//  Log("Enter SaveTouchMatrix\n");
  
  // If the file exists already from previous run, then delete it.
  if(SD.exists("tmatrix.txt"))
    SD.remove("tmatrix.txt");
  delay(50);
  
  FileOpen("tmatrix.txt", FILEWRITE);
  if(!myFileIsOpen())
  {
    Log("No create file\n");
    FileClose();
    return false;
  }
  
  do
  {
    data = rd32(address + (count * 4));
    Log("TMw: 0x%08lx\n", data);
    FileWriteByte(data & 0xff);                // Little endian file storage to match Eve
    FileWriteByte((data >> 8) & 0xff);
    FileWriteByte((data >> 16) & 0xff);
    FileWriteByte((data >> 24) & 0xff);
    count++;
  }while(count < 6);
  FileClose();
  Log("Matrix Saved\n");
}

// Read the touch digitizer calibration matrix values from a file and write them to the Eve.
bool LoadTouchMatrix(void)
{
  uint8_t count = 0;
  uint32_t data;
  uint32_t address = REG_TOUCH_TRANSFORM_A + RAM_REG;
  
  FileOpen("tmatrix.txt", FILEREAD);
  if(!myFileIsOpen())
  {
    Log("tmatrix.txt not open\n");
    FileClose();
    return false;
  }
  
  do
  {
    data = FileReadByte() +  ((uint32_t)FileReadByte() << 8) + ((uint32_t)FileReadByte() << 16) + ((uint32_t)FileReadByte() << 24);
    Log("TMr: 0x%08lx\n", data);
    wr32(address + (count * 4), data);
    count++;
  }while(count < 6);
  
  FileClose();
  Log("Matrix Loaded \n");
  return true;
}


// ************************************************************************************
// Following are abstracted file operations for Arduino.  This is possible by using a * 
// global pointer to a single file.  It is enough for our needs and it hides file     *
// handling details within the abstraction.                                           *
// ************************************************************************************
void FileOpen(char *filename, uint8_t mode)
{
  // Since one also loses access to defined values like FILE_READ from outside the .ino
  // I have been forced to make up values and pass them here (mode) where I can use the 
  // Arduino defines.
  switch(mode)
  {
  case FILEREAD:
    myFile = SD.open(filename, FILE_READ);
    break;
  case FILEWRITE:
    myFile = SD.open(filename, FILE_WRITE);
    break;
  default:;
  }
}

void FileClose(void)
{
  myFile.close();
  if(myFileIsOpen())
  {
    Log("Failed to close file\n");
  }
}

// Read a single byte from a file
uint8_t FileReadByte(void)
{
  return(myFile.read());
}

// Read bytes from a file into a provided buffer
void FileReadBuf(uint8_t *data, uint32_t NumBytes)
{
  myFile.read(data, NumBytes);
}

void FileWriteByte(uint8_t data)
{
  myFile.write(data);
}

uint32_t FileSize(void)
{
  return(myFile.size());
}

uint32_t FilePosition(void)
{
  return(myFile.position());
}

bool FileSeek(uint32_t offset)
{
  return(myFile.seek(offset));
}

bool myFileIsOpen(void)
{
  if(myFile)
    return true;
  else
    return false;
}
