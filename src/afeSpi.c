#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h> // Required for memcpy
#include "xparameters.h"
#include "xspips.h"
#include "xil_printf.h"
#include "afeSpi.h"

// Assuming these are defined in afeSpi.h
#define TI_AFE_RET_EXEC_PASS 0
#define TI_AFE_RET_EXEC_FAIL 1

#define SPI_DEVICE_ID XPAR_XSPIPS_0_DEVICE_ID
#define MAX_DATA 128

XSpiPs SpiInstance;
u8 SendBuf[MAX_DATA];
u8 RecvBuf[MAX_DATA];
signed int dig_T1, dig_T2, dig_T3;


/**
  @brief AFE SPI Write driver function.
  @details AFE SPI Write driver function. The contents of this function should be replaced by host SPI driver function.
  @param afeInst AFE Instance of uint8_t type
  @param addr Address to be written to.
  @param data value to be written.
  @return Returns if the function execution passed or failed.
*/
uint32_t afeSpiRawWrite(uint8_t afeInst, uint16_t addr, uint8_t data)
{
    uint8_t txBuf[2];
    uint8_t rxBuf[2];
    txBuf[0] = addr & 0x7F;
    txBuf[1] = data;

    int Status = XSpiPs_PolledTransfer(&SpiInstance, txBuf, rxBuf, 2);
    if (Status != XST_SUCCESS){
        return TI_AFE_RET_EXEC_FAIL;
    }
    xil_printf("Write: Addr=0x%02X, Data=0x%02X\n", addr, data);
    return TI_AFE_RET_EXEC_PASS;
}

/**
  @brief AFE SPI read driver function.
  @details AFE SPI read driver function and returns the read value as pointer. The contents of this function should be replaced by host SPI driver function.
  @param afeInst AFE Instance of uint8_t type
  @param addr Address to be read from.
  @param readVal Pointer return of the value read.
  @return Returns if the function execution passed or failed.
*/
uint32_t afeSpiRawRead(uint8_t afeInst, uint16_t addr, uint8_t *readVal)
{
    uint8_t txBuf[2];
    uint8_t rxBuf[2];
    txBuf[0] = addr | 0x80;
    txBuf[1] = 0x00; // Dummy byte for read

    int Status = XSpiPs_PolledTransfer(&SpiInstance, txBuf, rxBuf, 2);
    if (Status != XST_SUCCESS){
        return TI_AFE_RET_EXEC_FAIL;
    }
    *readVal = rxBuf[1];
    xil_printf("Read: Addr=0x%02X, Data=0x%02X\n", addr, *readVal);
    return TI_AFE_RET_EXEC_PASS;
}


/**
  @brief AFE SPI Write driver function.
  @details AFE SPI Write driver function. The contents of this function should be replaced by host SPI driver function.
  @param afeInst AFE Instance of uint8_t type
  @param addr Address to be written to.
  @param data Array of values to be written.
  @param dataArraySize Side of the data array.
  @return Returns if the function execution passed or failed.
*/
uint32_t afeSpiBurstWrite(uint8_t afeInst, uint16_t addr, uint8_t *data, uint16_t dataArraySize)
{
    int Status;
    uint8_t txBuf[dataArraySize * 2];
    uint8_t rxBuf[dataArraySize * 2];
    uint8_t j = 0;

    for (uint16_t i = 0; i < dataArraySize; i++) {
        txBuf[j++] = (addr + i) & 0x7F;
        txBuf[j++] = data[i];
    }

    Status = XSpiPs_PolledTransfer(&SpiInstance, txBuf, rxBuf, dataArraySize * 2);

    if (Status != XST_SUCCESS) {
        xil_printf("SPI Burst Write failed...\n");
        return TI_AFE_RET_EXEC_FAIL;
    }
    xil_printf("Burst Write: StartAddr=0x%04X, Total Len=%d\n", addr, dataArraySize);
    return TI_AFE_RET_EXEC_PASS;
}

/**
  @brief AFE SPI Read driver function.
  @details AFE SPI Read driver function. The contents of this function should be replaced by host SPI driver function.
  @param afeInst AFE Instance of uint8_t type
  @param addr Address to be read from to.
  @param dataArraySize Number of reads to do.
  @param data Pointer return of the array of values read. Maximum value is 1024.
  @return Returns if the function execution passed or failed.
*/
uint32_t afeSpiBurstRead(uint8_t afeInst, uint16_t addr, uint16_t dataArraySize, uint8_t *data)
{
    int Status;
    uint8_t txBuf[dataArraySize + 1];
    uint8_t rxBuf[dataArraySize + 1];

    txBuf[0] = addr | 0x80;
    for (uint16_t i = 1; i <= dataArraySize; i++) {
        txBuf[i] = 0x00;
    }

    Status = XSpiPs_PolledTransfer(&SpiInstance, txBuf, rxBuf, dataArraySize + 1);
    if (Status != XST_SUCCESS) {
        xil_printf("SPI Burst Read failed...\n");
        return TI_AFE_RET_EXEC_FAIL;
    }

    for (uint16_t i = 1; i <= dataArraySize; i++) {
        data[i - 1] = rxBuf[i];
        xil_printf("%d \t",data[i]);
    }

    xil_printf("Burst Read: StartAddr=0x%04X, Total Len=%d\n", addr, dataArraySize);
    return TI_AFE_RET_EXEC_PASS;
}

/**
  @brief AFE SPI Write driver function.
  @details AFE SPI Write driver function. The contents of this function should be replaced by host SPI driver function.
  @param afeInstSel Bit wise enable for SPI instance.
  @param addr Address to be written to.
  @param data value to be written.
  @return Returns if the function execution passed or failed.
*/
uint32_t afeSpiRawWriteMulti(uint8_t afeInstSel, uint16_t addr, uint8_t data)
{
    // The provided code does not have a multi-slave implementation.
    // This function remains a placeholder.
    return TI_AFE_RET_EXEC_PASS;
}

/**
  @brief AFE SPI read driver function.
  @details AFE SPI read driver function and returns the read value as pointer. The contents of this function should be replaced by host SPI driver function.
  @param afeInstSel Bit wise enable for SPI instance.
  @param addr Address to be read from.
  @param readVal Pointer return array of size NUM_SPI containing value read for each SPI instance. The values of this array should be initialized to 0 for the slaves not selected by afeInstSel.
  @return Returns if the function execution passed or failed.
*/
uint32_t afeSpiRawReadMulti(uint8_t afeInstSel, uint16_t addr, uint8_t *readVal)
{
    // The provided code does not have a multi-slave implementation.
    // This function remains a placeholder.
    return TI_AFE_RET_EXEC_PASS;
}

/**
  @brief AFE SPI Write driver function.
  @details AFE SPI Write driver function. The contents of this function should be replaced by host SPI driver function.
  @param afeInstSel Bit wise enable for SPI instance.
  @param addr Address to be written to.
  @param data Array of values to be written.
  @param dataArraySize Side of the data array.
  @return Returns if the function execution passed or failed.
*/
uint32_t afeSpiBurstWriteMulti(uint8_t afeInstSel, uint16_t addr, uint8_t *data, uint16_t dataArraySize)
{
    // The provided code does not have a multi-slave implementation.
    // This function remains a placeholder.
    return TI_AFE_RET_EXEC_PASS;
}

void temperature(signed int adc_T)
{
    signed int var1, var2, T;
    // BME280 temperature calculation algorithm
    var1 = ((((adc_T >> 3) - ((signed int)dig_T1 << 1))) * ((signed int)dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((signed int)dig_T1)) * ((adc_T >> 4) - ((signed int)dig_T1))) >> 12) * ((signed int)dig_T3)) >> 14;
    T = var1 + var2;
    T = (T * 5 + 128) >> 8;
    xil_printf("Temperature is %d C\n", T / 100);
}
