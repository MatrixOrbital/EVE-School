Example code for Bridgetech Eve2 FT810 running on Arduino.

- This code makes use of the Matrix Orbital Eve2 Library found here: 

  https://github.com/MatrixOrbital/EVE2-Library

  - While a copy of the library files (Eve2_81x.c and Eve2_81x.h) is included here, you may look for updated
    files if you wish.  This is optional, but the Eve2-Library is likely to contain an extension of what you
    have here in case you wish to make some more advanced screens.

- An Arduino shield with a connector for Matrix Orbital Eve2 displays is used to interface the Arduino to Eve.  
  This shield includes:
  - 20 contact FFC connector for Matrix Orbital Eve2 displays
  - 3 pushbuttons for application control without requiring a touchscreen (useful for initial calibration)
  - Audio amplifier and speaker for audio feedback
  - SD card holder
  - Additionally, the shield board is automatically level shifted for 5V Arduino and works with 3.3V Parallax Propeller ASC+ 