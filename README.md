# EVE Example code for an Arduino
  
![alt text](https://www.lcdforums.com/forums/download/file.php?id=1433)

Written for a Matrix Orbital [**EVE3**](https://www.matrixorbital.com/ftdi-eve/eve-bt815-bt816) or [**EVE4**](https://www.matrixorbital.com/ftdi-eve/eve-bt817-bt818) running on an Arduino UNO. The demo will work on EVE2 displays with the exception of touch.

This code will illustrate:
* EVE initilization
* Set colour
* Draw blue dot
* Write text
* Displaying an image
* Make a button
* Sound demo
------------------------------------------------------------------

**Instructions:**

The example is configured for a 4.3" display, to change the display used edit:

Eve2_81x.h --> // Panel specific setup parameters

The settings work for EVE3 and EVE4 displays.

1. Download the zip file
2. Rename the directory from EVE-School-master to EVE-School
3. Copy the 4 files located in 'Images for SD card' to the root of a SD Card
4. Put the SD Card into the [EVE2-Scoodo Sheild](https://www.matrixorbital.com/eve2-shield), connect the Shield to the UNO
5. Connect the [FFC cable](https://www.matrixorbital.com/ffc-20-pin) to the Shield and EVE display, BLUE UP on the FFC
6. Plugin the USB to the Arduino UNO and Power to the barrel jack
7. Load the INO file, and upload the program to the Arduino

This code when executed will display FTDI text and a red circle.


Button S1:
Button S2:
Button S3:


  
------------------------------------------------------------------

**Hardware Requirements:**

- Matrix Orbital EVE3/4 SPI TFT Displays
  https://www.matrixorbital.com/ftdi-eve  
![alt text](https://www.lcdforums.com/forums/download/file.php?id=1433)
  
  
-  Scoodo EVE TFT SHIELD for Arduino
  https://www.matrixorbital.com/eve2-shield  
![alt text](https://www.lcdforums.com/forums/download/file.php?id=1432)


 **EVE SPI TFT connected to a Scoodo SHIELD**
 
![alt text](https://www.lcdforums.com/forums/download/file.php?id=1430)

**LIBRARY**
- This code makes use of the Matrix Orbital EVE2 Library found here: 

  https://github.com/MatrixOrbital/EVE2-Library

  - While a copy of the library files (Eve2_81x.c and Eve2_81x.h) is included here, it will not be updated. For the most up to date files please use the files from the link above.

  **Support Forums**
 
 http://www.lcdforums.com/forums/viewforum.php?f=45
  

  


