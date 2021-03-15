#include <stdint.h>              
#include <SPI.h>          // Arduino SPI library
#include "arduino_al.h"
#include "hw_api.h"

// Send a series of bytes (contents of a buffer) through SPI
void HAL_SPI_WriteBuffer(uint8_t *Buffer, uint32_t Length)
{
  SPI.beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE0));
  digitalWrite(EveChipSelect_PIN, LOW);

  SPI.transfer(Buffer, Length);
      
  digitalWrite(EveChipSelect_PIN, HIGH);
  SPI.endTransaction();
}

// Send a byte through SPI as part of a larger transmission.  Does not enable/disable SPI CS
void HAL_SPI_Write(uint8_t data)
{
  SPI.transfer(data);
  return 0;
}

// Read a series of bytes from SPI and store them in a buffer
void HAL_SPI_ReadBuffer(uint8_t *Buffer, uint32_t Length)
{
  uint8_t a = SPI.transfer(0x00); // dummy read

  while (Length--)
  {
    *(Buffer++) = SPI.transfer(0x00);
  }
}

// Enable SPI by activating chip select line
void HAL_SPI_Enable(void)
{
  SPI.beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE0));
  digitalWrite(EveChipSelect_PIN, LOW);
}

// Disable SPI by deasserting the chip select line
void HAL_SPI_Disable(void)
{
  digitalWrite(EveChipSelect_PIN, HIGH);
  SPI.endTransaction();
}

// A millisecond delay wrapper for the Arduino function
void HAL_Delay(uint32_t DLY)
{
  uint32_t wait;
  wait = millis() + DLY; while(millis() < wait);
}

void HAL_Eve_Reset_HW(void)
{
  // Reset Eve
  SetPin(EvePDN_PIN, 0);                    // Set the Eve PDN pin low
  HAL_Delay(50);                             // delay
  SetPin(EvePDN_PIN, 1);                    // Set the Eve PDN pin high
  HAL_Delay(100);                            // delay
}
