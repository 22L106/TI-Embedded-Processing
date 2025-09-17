
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include "xparameters.h"
#include "xspips.h"
#include "xil_printf.h"
#include "afeSpi.h"


 XSpiPs SpiInstance;
#define NUM_SPI 4

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
    uint8_t txBuf[3];
    uint8_t rxBuf[3];

    // Format depends on AFE datasheet — example: [CMD+ADDRH][ADDRL][DATA]
    txBuf[0] = (addr >> 8) & 0xFF;
    txBuf[1] = addr & 0xFF;
    txBuf[2] = data;

    int Status = XSpiPs_PolledTransfer(&SpiInstance, txBuf, rxBuf, 3);
    if (Status != XST_SUCCESS){
        return TI_AFE_RET_EXEC_FAIL;
    }

    xil_printf("Self loopback test\n We have sent (%x(afeInstance), %x(address), %x(data)) as transmit buffer. As it is loopback, we get the same data in recieve buffer\n", afeInst,addr,data);
    xil_printf("rxBuf[0]= %x\n",rxBuf[0]);
    xil_printf("rxBuf[1]= %x\n",rxBuf[1]);
    xil_printf("rxBuf[2]= %x\n",rxBuf[2]);


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
    uint8_t txBuf[3];
    uint8_t rxBuf[3];

    // Format: [CMD+ADDRH][ADDRL][DUMMY]
    txBuf[0] = ((addr >> 8) & 0xFF) | 0x80;  // assuming MSB = read bit
    txBuf[1] = addr & 0xFF;
    txBuf[2] = 0x00;  // dummy byte

    int Status = XSpiPs_PolledTransfer(&SpiInstance, txBuf, rxBuf, 3);
    if (Status != XST_SUCCESS){
        return TI_AFE_RET_EXEC_FAIL;
    }

    *readVal = rxBuf[2];  // last byte is actual read
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
    uint16_t bytesRemaining = dataArraySize;
    uint16_t chunkSize;
    uint16_t offset = 0;

    uint8_t txBuf[128];
    uint8_t rxBuf[128];

    while (bytesRemaining > 0) {
        // Each chunk can carry (128 - 2) = 126 data bytes
        chunkSize = (bytesRemaining > 126) ? 126 : bytesRemaining;

        // Fill header
        txBuf[0] = (addr >> 8) & 0xFF;   // Address high
        txBuf[1] = addr & 0xFF;          // Address low

        // Copy data for this chunk
        memcpy(&txBuf[2], &data[offset], chunkSize);

        // SPI transfer: address(2) + data(chunkSize)
        Status = XSpiPs_PolledTransfer(&SpiInstance, txBuf, rxBuf, chunkSize + 2);
        if (Status != XST_SUCCESS) {
            xil_printf("SPI Burst Write failed...\n");
            return TI_AFE_RET_EXEC_FAIL;
        }

     
        xil_printf("Chunk Addr=0x%04X, Len=%d\n", addr, chunkSize);

        xil_printf("TX: ");
        for (int i = 0; i < chunkSize + 2; i++) {
            xil_printf("%02X ", txBuf[i]);
        }
        xil_printf("\n");

        xil_printf("RX: ");
        for (int i = 0; i < chunkSize + 2; i++) {
            xil_printf("%02X ", rxBuf[i]);
        }
        xil_printf("\n");

        // Update counters
        offset += chunkSize;
        bytesRemaining -= chunkSize;


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

    return TI_AFE_RET_EXEC_PASS;
}
