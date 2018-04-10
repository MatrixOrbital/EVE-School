#ifndef PROCESS_H
#define PROCESS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>              // Find integer types like "uint8_t"  

// Since this is essentially the application layer, it is also where global defines related to the 
// application are defined.  Also application level variables. 
#define SCR_Calibrate         0
#define SCR_FTDI              1
#define SCR_FTDIFIFO          2
#define SCR_Buttons           3
#define SCR_BMP               4
#define SCR_JPG               5
#define SCR_RAW               6

void MakeScreen_Bitmap(uint8_t *filename, uint16_t Xsize, uint16_t Ysize, uint16_t Xloc, uint16_t Yloc, uint8_t Reference);
void MakeScreen_Bitmap_DL(uint8_t *filename, uint16_t Xsize, uint16_t Ysize, uint16_t Xloc, uint16_t Yloc, uint8_t Reference);
uint32_t Load_JPG(uint32_t BaseAdd, uint32_t Options, char *filename); 
void MakeScreen_Button(void);
void MakeScreen_Calibrate(void);
void SelectScreen(uint8_t ID);
uint8_t CheckKeys(void);
uint32_t Load_ZLIB(uint32_t BaseAdd, char *filename);
uint32_t Load_RAW(uint32_t BaseAdd, char *filename); 

#ifdef __cplusplus
}
#endif

#endif
