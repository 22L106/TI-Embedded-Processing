#include "xparameters.h"
#include "xspips.h"
#include "xil_printf.h"

#define MAX_DATA 128

u8 SendBuf[MAX_DATA];
u8 RecvBuf[MAX_DATA];
#define NUM_SPI 4

typedef struct {
    u32 clockHz;          // SPI clock frequency
    u8 bitOrder;          // 0 = MSB first, 1 = LSB first
    u8 mode;              // SPI mode (0-3)
    u8 addrBits;          // Number of address bits
    u8 dataBits;          // Number of data bits per transfer
    XSpiPs spiInstance;     // SPI driver instance (from Xilinx library)
    u16 deviceId;         // SPI device ID (from xparameters.h)
    u8 slaveSelect;       // Slave select line
} SpiConfig_t;

SpiConfig_t spiDevices[NUM_SPI];

typedef enum RET_TYPE
{
    TI_AFE_RET_EXEC_PASS = 0,
    TI_AFE_RET_EXEC_FAIL = 1
} RetType_e;

uint32_t SPI_init(u8 afeInst, u32 clockHz, u8 bitOrder, u8 mode, u8 addrBits, u8 dataBits)
{
    if (afeInst >= NUM_SPI) return XST_FAILURE;

    int Status;
    XSpiPs_Config *SpiConfig;

    SpiConfig_t *dev = &spiDevices[afeInst];
    dev->clockHz = clockHz;
    dev->bitOrder = bitOrder;
    dev->mode = mode;
    dev->addrBits = addrBits;       
    dev->dataBits = dataBits;
    dev->deviceId = XPAR_XSPIPS_0_DEVICE_ID; 
    dev->slaveSelect = 0; 

    SpiConfig = XSpiPs_LookupConfig(dev->deviceId);
    if (!SpiConfig) return XST_FAILURE;

    Status = XSpiPs_CfgInitialize(&dev->spiInstance, SpiConfig, SpiConfig->BaseAddress);
    if (Status != XST_SUCCESS) return Status;

    switch(dev->mode) {
        case 0:
            // CPOL = 0, CPHA = 0
            Status = XSpiPs_SetOptions(&dev->spiInstance, XSPIPS_MASTER_OPTION | XSPIPS_FORCE_SSELECT_OPTION);
            break;
        case 1:
            // CPOL = 0, CPHA = 1
            Status = XSpiPs_SetOptions(&dev->spiInstance, XSPIPS_MASTER_OPTION | XSPIPS_FORCE_SSELECT_OPTION | XSPIPS_CLK_PHASE_1_OPTION);
            break;
        case 2:
            // CPOL = 1, CPHA = 0
            Status =  XSpiPs_SetOptions(&dev->spiInstance, XSPIPS_MASTER_OPTION | XSPIPS_FORCE_SSELECT_OPTION | XSPIPS_CLK_ACTIVE_LOW_OPTION);
            break;
        case 3:
            // CPOL = 1, CPHA = 1
            Status = XSpiPs_SetOptions(&dev->spiInstance, XSPIPS_MASTER_OPTION | XSPIPS_FORCE_SSELECT_OPTION | XSPIPS_CLK_PHASE_1_OPTION | XSPIPS_CLK_ACTIVE_LOW_OPTION);
            break;
        default:
            // CPOL = 0, CPHA = 0
            Status = XSpiPs_SetOptions(&dev->spiInstance, XSPIPS_MASTER_OPTION | XSPIPS_FORCE_SSELECT_OPTION);
            break;
    }
    if (Status != XST_SUCCESS) return Status;

    u32 inputClk = dev->spiInstance.Config.InputClockHz;
    u32 prescaler = 1;
    u32 clkDiv = inputClk / dev->clockHz;

    // Prescaler can only be 1,2,4,8,...,256
    while ((prescaler < 256) && (clkDiv > prescaler)) {
        prescaler <<= 1; // multiply by 2
    }

    switch (prescaler) {
        case 4:   Status = XSpiPs_SetClkPrescaler(&dev->spiInstance, XSPIPS_CLK_PRESCALE_4); break;
        case 8:   Status = XSpiPs_SetClkPrescaler(&dev->spiInstance, XSPIPS_CLK_PRESCALE_8); break;
        case 16:  Status = XSpiPs_SetClkPrescaler(&dev->spiInstance, XSPIPS_CLK_PRESCALE_16); break;
        case 32:  Status = XSpiPs_SetClkPrescaler(&dev->spiInstance, XSPIPS_CLK_PRESCALE_32); break;
        case 64:  Status = XSpiPs_SetClkPrescaler(&dev->spiInstance, XSPIPS_CLK_PRESCALE_64); break;
        case 128: Status = XSpiPs_SetClkPrescaler(&dev->spiInstance, XSPIPS_CLK_PRESCALE_128); break;
        case 256: Status = XSpiPs_SetClkPrescaler(&dev->spiInstance, XSPIPS_CLK_PRESCALE_256); break;
    }
    if (Status != XST_SUCCESS) return Status;
}
    
uint32_t afeSpiRawRead(uint8_t afeInst, uint16_t addr, uint8_t *readVal)
{
    int Status;

    XSpiPs_SetSlaveSelect(&spiDevices[afeInst].spiInstance, afeInst);
    XSpiPs_Enable(&spiDevices[afeInst].spiInstance);

    SendBuf[0] = (addr & 0xFF00) >> 8;
    SendBuf[1] = (addr & 0x00FF);

    Status = XSpiPs_PolledTransfer(&spiDevices[afeInst].spiInstance, SendBuf, RecvBuf, 3);
    *readVal = RecvBuf[2];

    if (Status != XST_SUCCESS) {
        return TI_AFE_RET_EXEC_FAIL;}
    return TI_AFE_RET_EXEC_PASS;
}

uint32_t afeSpiRawWrite(uint8_t afeInst, uint16_t addr, uint8_t data)
{
    int Status;

    XSpiPs_SetSlaveSelect(&spiDevices[afeInst].spiInstance, afeInst);
    XSpiPs_Enable(&spiDevices[afeInst].spiInstance);

    SendBuf[0] = (addr & 0xFF00) >> 8;
    SendBuf[1] = (addr & 0x00FF);
    SendBuf[2] = data;

    Status = XSpiPs_PolledTransfer(&spiDevices[afeInst].spiInstance, SendBuf, RecvBuf, 3);

    if (Status != XST_SUCCESS) {
        return TI_AFE_RET_EXEC_FAIL;}
    return TI_AFE_RET_EXEC_PASS;
}

uint32_t afeSpiBurstRead(uint8_t afeInst, uint16_t addr, uint16_t dataArraySize, uint8_t *data)
{
    int Status;
    uint16_t chunkSize;
    uint16_t offset = 0;

    XSpiPs_SetSlaveSelect(&spiDevices[afeInst].spiInstance, afeInst);
    XSpiPs_Enable(&spiDevices[afeInst].spiInstance);

    do{
        chunkSize = (dataArraySize > (MAX_DATA - 2)) ? (MAX_DATA - 2) : dataArraySize; 
        SendBuf[0] = (addr & 0xFF00) >> 8;
        SendBuf[1] = (addr & 0x00FF);
        addr = addr + 127;
        for (u16 i = 0; i < chunkSize; i++) {
            SendBuf[i + 2] = 0x00; // Dummy bytes to clock out data
        }

        Status = XSpiPs_PolledTransfer(&spiDevices[afeInst].spiInstance, SendBuf, RecvBuf, chunkSize + 2);

        for (u16 i = 0; i < chunkSize; i++) {
            data[offset + i] = RecvBuf[i + 2];
        }
        offset += chunkSize;
        addr += 127;
        dataArraySize -= chunkSize;

        if (Status != XST_SUCCESS) {
        return TI_AFE_RET_EXEC_FAIL;}

    }while(dataArraySize > 0);

    return TI_AFE_RET_EXEC_PASS;
}

uint32_t afeSpiBurstWrite(uint8_t afeInst, uint16_t addr, uint16_t dataArraySize, uint8_t *data)
{
    int Status;
    uint16_t chunkSize;
    uint16_t offset = 0;

    XSpiPs_SetSlaveSelect(&spiDevices[afeInst].spiInstance, afeInst);
    XSpiPs_Enable(&spiDevices[afeInst].spiInstance);

    do {
        // Each data byte takes 3 bytes on SPI (2 addr + 1 data)
        // Limit chunk so total frame ≤ MAX_DATA
        uint16_t maxBytes = (MAX_DATA / 3);  // how many bytes we can send per transfer
        chunkSize = (dataArraySize > maxBytes) ? maxBytes : dataArraySize;

        uint16_t sendIndex = 0;

        for (uint16_t i = 0; i < chunkSize; i++) {
            uint16_t currentAddr = addr + i;

            // Write frame for one data byte: [MSB][LSB][DATA]
            SendBuf[sendIndex++] = (currentAddr >> 8) & 0xFF;
            SendBuf[sendIndex++] = (currentAddr & 0xFF);
            SendBuf[sendIndex++] = data[offset + i];
        }

        Status = XSpiPs_PolledTransfer(&spiDevices[afeInst].spiInstance,
                                       SendBuf, RecvBuf, sendIndex);
        if (Status != XST_SUCCESS) {
            return TI_AFE_RET_EXEC_FAIL;
        }

        offset += chunkSize;
        addr += chunkSize;
        dataArraySize -= chunkSize;

    } while (dataArraySize > 0);

    return TI_AFE_RET_EXEC_PASS;
}

uint32_t afeSpiRawWriteMulti(uint8_t afeInstSel, uint16_t addr, uint8_t data)
{
    int Status;

    // Iterate through all possible AFE instances
    for (uint8_t afeInst = 0; afeInst < NUM_SPI; afeInst++) {

        // Check if this instance is selected (bit set in afeInstSel)
        if (afeInstSel & (1 << afeInst)) {

            XSpiPs_SetSlaveSelect(&spiDevices[afeInst].spiInstance, afeInst);
            XSpiPs_Enable(&spiDevices[afeInst].spiInstance);

            // Prepare transmit buffer: [ADDR_HIGH][ADDR_LOW][DATA]
            SendBuf[0] = (addr >> 8) & 0xFF;
            SendBuf[1] = (addr & 0xFF);
            SendBuf[2] = data;

            Status = XSpiPs_PolledTransfer(&spiDevices[afeInst].spiInstance,
                                           SendBuf, RecvBuf, 3);
            if (Status != XST_SUCCESS) {
                return TI_AFE_RET_EXEC_FAIL;
            }
        }
    }

    return TI_AFE_RET_EXEC_PASS;
}

uint32_t afeSpiRawReadMulti(uint8_t afeInstSel, uint16_t addr, uint8_t *readVal)
{
    int Status;

    // Initialize all read values to 0 for unselected instances
    for (uint8_t i = 0; i < NUM_SPI; i++) {
        readVal[i] = 0;
    }

    // Iterate through all SPI instances
    for (uint8_t afeInst = 0; afeInst < NUM_SPI; afeInst++) {

        // Check if this instance is selected
        if (afeInstSel & (1 << afeInst)) {

            XSpiPs_SetSlaveSelect(&spiDevices[afeInst].spiInstance, afeInst);
            XSpiPs_Enable(&spiDevices[afeInst].spiInstance);

            // Prepare transmit buffer: [ADDR_HIGH|0x80][ADDR_LOW][DUMMY]
            // The 0x80 sets the read bit in the MSB of high address byte.
            SendBuf[0] = ((addr >> 8) & 0x7F) | 0x80;  // read bit set
            SendBuf[1] = (addr & 0xFF);
            SendBuf[2] = 0x00;  // Dummy byte to clock out the response

            Status = XSpiPs_PolledTransfer(&spiDevices[afeInst].spiInstance,
                                           SendBuf, RecvBuf, 3);
            if (Status != XST_SUCCESS) {
                return TI_AFE_RET_EXEC_FAIL;
            }

            // The received byte is in RecvBuf[2]
            readVal[afeInst] = RecvBuf[2];
        }
    }

    return TI_AFE_RET_EXEC_PASS;
}

uint32_t afeSpiBurstWriteMulti(uint8_t afeInstSel, uint16_t addr, uint8_t *data, uint16_t dataArraySize)
{
    int Status;

    for (uint8_t afeInst = 0; afeInst < NUM_SPI; afeInst++) {

        // Check if this AFE instance is selected
        if (afeInstSel & (1 << afeInst)) {

            XSpiPs_SetSlaveSelect(&spiDevices[afeInst].spiInstance, afeInst);
            XSpiPs_Enable(&spiDevices[afeInst].spiInstance);

            uint16_t bytesRemaining = dataArraySize;
            uint16_t offset = 0;

            // Send in chunks (in case MAX_DATA limits transfer size)
            while (bytesRemaining > 0) {
                uint16_t maxBytes = (MAX_DATA / 3);  // each data = 3 bytes (2 addr + 1 data)
                uint16_t chunkSize = (bytesRemaining > maxBytes) ? maxBytes : bytesRemaining;
                uint16_t sendIndex = 0;

                // Build the SPI frame
                for (uint16_t i = 0; i < chunkSize; i++) {
                    uint16_t currAddr = addr + offset + i;

                    // Frame: [ADDR_HIGH][ADDR_LOW][DATA]
                    SendBuf[sendIndex++] = (currAddr >> 8) & 0xFF;
                    SendBuf[sendIndex++] = (currAddr & 0xFF);
                    SendBuf[sendIndex++] = data[offset + i];
                }

                Status = XSpiPs_PolledTransfer(&spiDevices[afeInst].spiInstance,
                                               SendBuf, RecvBuf, sendIndex);
                if (Status != XST_SUCCESS) {
                    return TI_AFE_RET_EXEC_FAIL;
                }

                offset += chunkSize;
                bytesRemaining -= chunkSize;
            }
        }
    }

    return TI_AFE_RET_EXEC_PASS;
}
