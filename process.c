// Process.c is the application layer.  All function calls are hardware ambivalent.

#include <stdint.h>              // Find integer types like "uint8_t"  
#include "Eve2_81x.h"            // Matrix Orbital Eve2 Driver
#include "Arduino_AL.h"          // include the hardware specific abstraction layer header for the specific hardware in use.
#include "process.h"             // Every c file has it's header and this is the one for this file

void SelectScreen(uint8_t ID)
{
  switch(ID)
  {
  case SCR_FTDI:
    MakeScreen_FTDI(20);            // FTDI Screen
    break;
  case SCR_FTDIFIFO:
    MakeScreen_FTDIFIFO(10);        // FTDI Screen made using FIFO
    break;
  case SCR_Calibrate:
    MakeScreen_Calibrate();         // Calibration Screen
    break;
  case SCR_Buttons:
    MakeScreen_Button();            // Button and text screen
    break;
  case SCR_BMP:
    MakeScreen_Bitmap("C480_272.bin", 480, 272, 0, 0, 0);              // File retrieval from SD Card
    break;
  case SCR_JPG:
    MakeScreen_JPEG("C480_272.jpg", 480, 272, 0, 0, 0);              // File retrieval from SD Card
    break;
  case SCR_RAW:
    MakeScreen_Bitmap_DL("L256_128.raw", 256, 128, (480-256)/2, (272-128)/2, 1);           // File retrieval from SD Card
    break;
  default:
    break;
  }
}

// red dot example (Straight into the display list without CoProcessor interaction)
// This is supposed to demonstrate the Display List (which you will never want to directly manipulate ever again)
// A value is passed in to set the size of the dot (to give visual feedback to the touch region we TAG-ed to the dot)
void MakeScreen_FTDI(uint8_t DotSize)
{
//  Log("Enter FTDI\n");

  wr32(RAM_DL + 0, CLEAR(1, 1, 1));                // clear screen (but not calibration)
  wr32(RAM_DL + 4, BEGIN(BITMAPS));                // start drawing bitmaps
  wr32(RAM_DL + 8, VERTEX2II(220, 110, 31, 'F'));  // ascii F in font 31
  wr32(RAM_DL + 12, VERTEX2II(244, 110, 31, 'T')); // ascii T
  wr32(RAM_DL + 16, VERTEX2II(272, 110, 31, 'D')); // ascii D
  wr32(RAM_DL + 20, VERTEX2II(299, 110, 31, 'I')); // ascii I
  wr32(RAM_DL + 24, END());                        // end placing bitmaps
  wr32(RAM_DL + 28, COLOR_RGB(192, 26, 26));       // change colour to reddish
  wr32(RAM_DL + 32, POINT_SIZE(DotSize * 16));     // set point size to DotSize pixels. Points = (pixels x 16)
  wr32(RAM_DL + 36, BEGIN(POINTS));                // start drawing points
  wr32(RAM_DL + 40, TAG(1));                       // Tag the red dot with a touch ID
  wr32(RAM_DL + 44, VERTEX2II(192, 133, 0, 0));    // place red point
  wr32(RAM_DL + 48, END());                        // end placing points
  wr32(RAM_DL + 52 , DISPLAY());                   // display the image
  wr8(REG_DLSWAP + RAM_REG, DLSWAP_FRAME);         // swap display lists
}

// red dot example using the FIFO
// Demonstrate the equivalency of using the FIFO to directly generate the Display List
void MakeScreen_FTDIFIFO(uint8_t DotSize)
{
//  Log("Enter FTDIFIFO\n");

  Send_CMD(CMD_DLSTART);
  Send_CMD(CLEAR(1, 1, 1));                // clear screen (but not calibration)
  Send_CMD(BEGIN(BITMAPS));                // start drawing bitmaps
  Send_CMD(VERTEX2II(220, 110, 31, 'F'));  // ascii F in font 31
  Send_CMD(VERTEX2II(244, 110, 31, 'T'));  // ascii T
  Send_CMD(VERTEX2II(272, 110, 31, 'D'));  // ascii D
  Send_CMD(VERTEX2II(299, 110, 31, 'I'));  // ascii I
  Send_CMD(END());                         // end placing bitmaps
  Send_CMD(COLOR_RGB(192, 26, 26));        // change colour to reddish
  Send_CMD(POINT_SIZE(DotSize * 16));      // set point size to DotSize pixels. Points = (pixels x 16)
  Send_CMD(BEGIN(POINTS));                 // start drawing points
  Send_CMD(TAG(2));                        // Tag the red dot with a touch ID
  Send_CMD(VERTEX2II(192, 133, 0, 0));     // place red point
  Send_CMD(END());                         // end placing points
  Send_CMD(DISPLAY());                     // display the image
  Send_CMD(CMD_SWAP);                      // Make this screen active
  UpdateFIFO();                            // Trigger the CoProcessor to start processing the FIFO
}

// Sound demo - written to the Co-processor ( AKA GPU and FIFO )
void MakeScreen_Button(void)
{
//  Log("Enter Button\n");

  Send_CMD(CMD_DLSTART);
  Cmd_Gradient(0, 0, 0xA1E1FF, 0, 250, 0x000080);           // The whole screen is a color gradient in blue
  
  Cmd_Text(470, 250, 26, OPT_RIGHTX, "matrixorbital.com");  // White text
  
  Send_CMD(COLOR_RGB(0xDE,0x00,0x08));                      // Change the color of following text to reddish
  Cmd_Text(240, 40, 31, OPT_CENTERX, "FT81x Sound Demo");
  
  Send_CMD(COLOR_RGB(64, 64, 64));                          // Change color of following text to grey
  Cmd_FGcolor(0x228B22);                                    // Change button face color to a mild green
  Send_CMD(TAG(10));                                        // Tag the following button as a touch region with a return value of 10
  Cmd_Button(100, 125, 120, 48, 28, 0, "Xylo C3");
  Send_CMD(TAG(11));
  Cmd_Button(250, 125, 120, 48, 28, 0, "Xylo C5");
  Send_CMD(TAG(12));
  Cmd_Button(100, 190, 120, 48, 28, 0, "Piano F2");
  Send_CMD(TAG(13));
  Cmd_Button(250, 190, 120, 48, 28, 0, "Piano F4");
  
  Send_CMD(DISPLAY());
  Send_CMD(CMD_SWAP);  
  UpdateFIFO();                                            // Trigger the CoProcessor to start processing the FIFO
}

// Decompress a ZLIB compressed image from SD card into RAM_G
void MakeScreen_Bitmap(uint8_t *filename, uint16_t Xsize, uint16_t Ysize, uint16_t Xloc, uint16_t Yloc, uint8_t Reference)
{
  uint32_t tmp;
  uint32_t BMPBaseAdd = RAM_G;                                       // Hard-coded Address lacks panache

  tmp = Load_ZLIB(BMPBaseAdd, filename);                             // Load a bitmap into RAM_G
  Log("ZLIB used 0x%08lx\n", tmp - BMPBaseAdd); 
  
  // Screen start
  Send_CMD(CMD_DLSTART);                                             // Start a new display list
  Send_CMD(CLEAR_COLOR_RGB(255,255,255));                            // Set the color for clearing to full white
  Send_CMD(CLEAR(1,1,1));                                            // clear screen 
  
  // Define the bitmap
  Send_CMD(BITMAP_HANDLE(Reference));                                // handle for this bitmap
  Cmd_SetBitmap(BMPBaseAdd, RGB565, Xsize, Ysize);                   // Use the CoPro Command to fill in the bitmap parameters (Eve PG 5.6.5)
  
  // Place the bitmap
  Send_CMD(BEGIN(BITMAPS));
  Send_CMD(BITMAP_HANDLE(Reference));
  Send_CMD(VERTEX2II(Xloc, Yloc, Reference, 0));                     // Define the placement position of the previously defined holding area.
  Send_CMD(END());                                                   // end placing bitmaps
  
  Send_CMD(COLOR_RGB(0x20,0xFF,0x20));                               // Set the text color
  Cmd_Text(11, 11, 26, 0, filename);
  
  Send_CMD(DISPLAY());                                               // End display list
  Send_CMD(CMD_SWAP);                                                // Activate this display list
  UpdateFIFO();                                                      // Trigger the CoProcessor to start processing the FIFO

  Log("Leave Bitmap\n");
}

// Using raw uncompressed bitmap data, the CoProcessor is not needed for inflation and the data is 
// stored into RAM_G without the use of the FIFO / CoProcessor.
// This function uses only the Display List and not FIFO
void MakeScreen_Bitmap_DL(uint8_t *filename, uint16_t Xsize, uint16_t Ysize, uint16_t Xloc, uint16_t Yloc, uint8_t Reference)
{
  uint32_t BMPBaseAdd = RAM_G + 0x40000UL;                               // Hard-coded Address lacks panache
  
  Load_RAW(BMPBaseAdd, filename);                                        // Load a bitmap into RAM_G
  Log("after loadRAW \n"); 
  
  // Screen start
  wr32(RAM_DL + 0, CLEAR_COLOR_RGB(200,200,200));                        // Set the color for clearing to whitish
  wr32(RAM_DL + 4, CLEAR(1, 1, 1));                                      // clear screen
  
  // Define the bitmap
  wr32(RAM_DL + 8, BITMAP_HANDLE(Reference));                            // handle for this bitmap
  wr32(RAM_DL + 12, BITMAP_SOURCE(BMPBaseAdd));                          // Starting address in GRAM
  wr32(RAM_DL + 16, BITMAP_LAYOUT(RGB565, Xsize*2, Ysize));              // format, stride, height 
  wr32(RAM_DL + 20, BITMAP_SIZE(NEAREST, BORDER, BORDER, Xsize, Ysize)); // define the holding area for the bitmap
  
  // Place the bitmap
  wr32(RAM_DL + 24, BEGIN(BITMAPS));
  wr32(RAM_DL + 28, BITMAP_HANDLE(Reference));
  wr32(RAM_DL + 32, VERTEX2II(Xloc, Yloc, Reference, 0));                // Define the placement position of the previously defined holding area.

                                                                         // We are already placing bitmaps, so no need to END and BEGIN bitmap commands again
  wr32(RAM_DL + 36, COLOR_RGB(0x20,0xA0,0x20));                          // Set the text color
  wr32(RAM_DL + 40, VERTEX2II(11, 11, 26, filename[0]));                 // Assume a 8.3 filename
  wr32(RAM_DL + 44, VERTEX2II(19, 11, 26, filename[1]));
  wr32(RAM_DL + 48, VERTEX2II(27, 11, 26, filename[2]));
  wr32(RAM_DL + 52, VERTEX2II(35, 11, 26, filename[3]));
  wr32(RAM_DL + 56, VERTEX2II(43, 11, 26, filename[4]));
  wr32(RAM_DL + 60, VERTEX2II(51, 11, 26, filename[5]));
  wr32(RAM_DL + 64, VERTEX2II(59, 11, 26, filename[6]));
  wr32(RAM_DL + 68, VERTEX2II(67, 11, 26, filename[7]));
  wr32(RAM_DL + 72, VERTEX2II(75, 11, 26, filename[8]));                 // This will be the period
  wr32(RAM_DL + 76, VERTEX2II(78, 11, 26, filename[9]));
  wr32(RAM_DL + 80, VERTEX2II(83, 11, 26, filename[10]));
  wr32(RAM_DL + 84, VERTEX2II(91, 11, 26, filename[11]));
  wr32(RAM_DL + 88, END());                                              // end placing bitmaps

  wr32(RAM_DL + 92, DISPLAY());                                          // End display list
  wr8(REG_DLSWAP + RAM_REG, DLSWAP_FRAME);                               // swap display lists

  Log("Leave Bitmap DL\n");
}

// Decompress a JPEG compressed image from SD card into RAM_G
void MakeScreen_JPEG(uint8_t *filename, uint16_t Xsize, uint16_t Ysize, uint16_t Xloc, uint16_t Yloc, uint8_t Reference)
{
  uint32_t tmp;
  uint32_t BMPBaseAdd = RAM_G + 0x80000UL;                           // Hard-coded Address lacks panache

  tmp = Load_JPG(BMPBaseAdd, 0, filename);                           // Load a bitmap into RAM_G
  Log("JPG used 0x%08lx\n", tmp - BMPBaseAdd); 
  
  // Screen start
  Send_CMD(CMD_DLSTART);                                             // Start a new display list
  Send_CMD(CLEAR_COLOR_RGB(255,255,255));                            // Set the color for clearing to full white
  Send_CMD(CLEAR(1,1,1));                                            // clear screen 
  
  // Define the bitmap
  Send_CMD(BITMAP_HANDLE(Reference));                                // handle for this bitmap
  Cmd_SetBitmap(BMPBaseAdd, RGB565, Xsize, Ysize);                   // Use the CoPro Command to fill in the bitmap parameters (Eve PG 5.6.5)
  
  // Place the bitmap
  Send_CMD(BEGIN(BITMAPS));
  Send_CMD(BITMAP_HANDLE(Reference));
  Send_CMD(VERTEX2II(Xloc, Yloc, Reference, 0));                     // Define the placement position of the previously defined holding area.
  Send_CMD(END());                                                   // end placing bitmaps
  
  Send_CMD(COLOR_RGB(0x20,0xFF,0x20));                               // Set the text color
  Cmd_Text(11, 11, 26, 0, filename);
  
  Send_CMD(DISPLAY());                                               // End display list
  Send_CMD(CMD_SWAP);                                                // Activate this display list
  UpdateFIFO();                                                      // Trigger the CoProcessor to start processing the FIFO

  Log("Leave jpeg\n");
}

// A calibration screen for the touch digitizer
void MakeScreen_Calibrate(void)
{
  Log("Enter Calibrate\n");
  
  Send_CMD(CMD_DLSTART);
  Send_CMD(CLEAR_COLOR_RGB(0,0,0));
  Send_CMD(CLEAR(1,1,1));
  Cmd_Text(100, 240, 27, OPT_CENTER, "Tap on the dots");
  Cmd_Calibrate(0);                                           // This widget generates a blocking screen that doesn't unblock until 3 dots have been touched
  Send_CMD(DISPLAY());
  Send_CMD(CMD_SWAP);
  UpdateFIFO();                                               // Trigger the CoProcessor to start processing commands out of the FIFO
  
  Wait4CoProFIFOEmpty();                                      // wait here until the coprocessor has read and executed every pending command.
  MyDelay(100);

  Log("Leaving Calibrate\n");
}

// This define is for the size of the buffer we are going to use for data transfers.  It is 
// sitting here so uncomfortably because it is a silly tiny buffer in Arduino Uno and you
// will want a bigger one if you can get it.  Redefine this and add a nice buffer to Load_ZLIB()
#define COPYBUFSIZE WorkBuffSz

// Load a compressed bitmap from SD card into RAM_G at address "BaseAdd"
// The file will have been processed by "img_cvt.exe" from FTDI
// Return value is the last RAM_G address used during the inflation operation - you might want it
uint32_t Load_ZLIB(uint32_t BaseAdd, char *filename) 
{
  uint32_t Remaining;
  uint16_t ReadBlockSize = 0;

  // Open the file on SD card by name
  FileOpen(filename, FILEREAD);
  if(!myFileIsOpen())
  {
    Log("%s not open\n", filename);
    FileClose();
    return false;
  }
  
  Remaining = FileSize();                                    // Store the size of the currently opened file
  
  Send_CMD(CMD_INFLATE);                                     // Tell the CoProcessor to prepare for compressed data
  Send_CMD(BaseAdd);                                         // This is the address where decompressed data will go 

  while (Remaining)
  {
    if (Remaining > COPYBUFSIZE)
      ReadBlockSize = COPYBUFSIZE;
    else
      ReadBlockSize = Remaining;
    
    FileReadBuf(LogBuf, ReadBlockSize);                      // Read a block of data from the file
    CoProWrCmdBuf(LogBuf, ReadBlockSize);                    // write the block to FIFO - Does FIFO triggering
    Remaining -= ReadBlockSize;                              // Reduce remaining data value by amount just read
  }
  FileClose();

  Wait4CoProFIFOEmpty();                                     // wait here until the coprocessor has read and executed every pending command.

  // Get the address of the last RAM location used during inflation
  Cmd_GetPtr();                                              // FifoWriteLocation is updated twice so the data is returned to it's updated location - 4
  UpdateFIFO();                                              // force run the GetPtr command
  return (rd32(FifoWriteLocation + RAM_CMD - 4));            // The result is stored at the FifoWriteLocation - 4 (because FTDI is Chaotic Evil)
}         

// Load a JPEG image from SD card into RAM_G at address "BaseAdd"
// Return value is the last RAM_G address used during the jpeg decompression operation.
// The function is virtually identical to Load_ZLIB as the only difference is the memory operation
uint32_t Load_JPG(uint32_t BaseAdd, uint32_t Options, char *filename) 
{
  uint32_t Remaining;
  uint16_t ReadBlockSize = 0;

  // Open the file on SD card by name
  FileOpen(filename, FILEREAD);
  if(!myFileIsOpen())
  {
    Log("%s not open\n", filename);
    FileClose();
    return false;
  }
  
  Remaining = FileSize();                                    // Store the size of the currently opened file
  
  Send_CMD(CMD_LOADIMAGE);                                   // Tell the CoProcessor to prepare for compressed data
  Send_CMD(BaseAdd);                                         // This is the address where decompressed data will go 
  Send_CMD(Options);                                         // Send options (options are mostly not obviously useful)

  while (Remaining)
  {
    if (Remaining > COPYBUFSIZE)
      ReadBlockSize = COPYBUFSIZE;
    else
      ReadBlockSize = Remaining;
    
    FileReadBuf(LogBuf, ReadBlockSize);                      // Read a block of data from the file
    CoProWrCmdBuf(LogBuf, ReadBlockSize);                    // write the block to FIFO - Does FIFO triggering
    Remaining -= ReadBlockSize;                              // Reduce remaining data value by amount just read
  }
  FileClose();

  Wait4CoProFIFOEmpty();                                     // wait here until the coprocessor has read and executed every pending command.

  // Get the address of the last RAM location used during inflation
  Cmd_GetPtr();                                              // FifoWriteLocation is updated twice so the data is returned to it's updated location - 4
  UpdateFIFO();                                              // force run the GetPtr command
  return (rd32(FifoWriteLocation + RAM_CMD - 4));            // The result is stored at the FifoWriteLocation - 4 (Yes, this is unexpected and random)
}         

// Load a raw (image data) bitmap from SD card into RAM_G at address 0
// The file will have been processed by "img_cvt.exe" from FTDI
// While the memory usage is not a mystery, the next available address is returned for convenience.
uint32_t Load_RAW(uint32_t BaseAdd, char *filename)                                 
{
  uint32_t Remaining;
  uint16_t ReadBlockSize = 0;
  uint32_t Add_GRAM = BaseAdd;

  // Open the file on SD card by name
  FileOpen(filename, FILEREAD);
  if(!myFileIsOpen())
  {
    Log("%s not open\n", filename);
    FileClose();
    return false;
  }
  
  Remaining = FileSize();                                      // Store the size of the currently opened file

  while (Remaining)
  {
    if (Remaining > COPYBUFSIZE)
      ReadBlockSize = COPYBUFSIZE;
    else
      ReadBlockSize = Remaining;
    
    FileReadBuf(LogBuf, ReadBlockSize);                        // Read a block of data from the file
    Add_GRAM = WriteBlockRAM(Add_GRAM, LogBuf, ReadBlockSize); // write the block to RAM_G
    Remaining -= ReadBlockSize;                                // Reduce remaining data value by amount just read
  }
  FileClose();
  return (Add_GRAM);
}

// Is there a physical key press?
uint8_t CheckKeys(void)
{
  if(!ReadPin(Button1_PIN))
    return(1);
  if(!ReadPin(Button2_PIN))
    return(2);
  if(!ReadPin(Button3_PIN))
    return(3);

  return(0);
}


