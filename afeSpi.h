#ifndef AFE_SPI_H
#define AFE_SPI_H

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include "xparameters.h"
#include "xspips.h"
#include "xil_printf.h"

// Return type enum
#define TI_AFE_RET_EXEC_PASS  0
#define TI_AFE_RET_EXEC_FAIL  1


// Public API
int SpiPs_Init(u16 SpiDeviceId, int clk_option);

uint32_t afeSpiRawWrite(uint8_t afeInst, uint16_t addr, uint8_t data);
uint32_t afeSpiRawRead(uint8_t afeInst, uint16_t addr, uint8_t *readVal);
uint32_t afeSpiBurstWrite(uint8_t afeInst, uint16_t addr, uint8_t *data, uint16_t dataArraySize);
uint32_t afeSpiBurstRead(uint8_t afeInst, uint16_t addr, uint16_t dataArraySize, uint8_t *data);

#endif
