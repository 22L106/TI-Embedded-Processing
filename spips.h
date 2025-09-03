#ifndef SPI_DRIVER_H
#define SPI_DRIVER_H

#include "xparameters.h"
#include "xspips.h"
#include "xil_printf.h"
#include "xspips.h"

// Global SPI instance declaration
extern XSpiPs SpiInstance;

// Function prototypes
int SpiPs_Init(u16 SpiDeviceId, int clk_option);

#endif // SPI_DRIVER_H
