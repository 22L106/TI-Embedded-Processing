#include "xparameters.h"
#include "xspips.h"
#include "xil_printf.h"

#define MAX_DATA 128
#define NUM_SPI 4

u8 SendBuf[MAX_DATA];
u8 RecvBuf[MAX_DATA];

typedef struct {
u32 clockHz;          // SPI clock frequency
u8 bitOrder;          // 0 = MSB first, 1 = LSB first
u8 mode;              // SPI mode (0-3)
u8 addrBits;          // Number of address bits
u8 dataBits;          // 8 or 16 bits per transfer
XSpiPs spiInstance;   // SPI driver instance (from Xilinx library)
u16 deviceId;         // SPI device ID (from xparameters.h)
u8 slaveSelect;       // Slave select line
} SpiConfig_t;

SpiConfig_t spiDevices[NUM_SPI];

typedef enum RET_TYPE
{
TI_AFE_RET_EXEC_PASS = 0,
TI_AFE_RET_EXEC_FAIL = 1
} RetType_e;

// Helper functions for packing/unpacking based on bit order
static inline void packData(u8 *buf, u16 data, u8 bitOrder)
{
if (bitOrder == 0) { // MSB first
buf[0] = (data >> 8) & 0xFF;
buf[1] = data & 0xFF;
} else { // LSB first
buf[0] = data & 0xFF;
buf[1] = (data >> 8) & 0xFF;
}
}

static inline u16 unpackData(u8 *buf, u8 bitOrder)
{
if (bitOrder == 0)
return ((u16)buf[0] << 8) | buf[1];
else
return ((u16)buf[1] << 8) | buf[0];
}

uint32_t SPI_init(u8 afeInst, u32 clockHz, u8 bitOrder, u8 mode, u8 addrBits, u8 dataBits)
{
if (afeInst >= NUM_SPI) return XST_FAILURE;

```
int Status;
XSpiPs_Config *SpiConfig;

SpiConfig_t *dev = &spiDevices[afeInst];
dev->clockHz = clockHz;
dev->bitOrder = bitOrder;
dev->mode = mode;
dev->addrBits = addrBits;
dev->dataBits = dataBits; // 8 or 16
dev->deviceId = XPAR_XSPIPS_0_DEVICE_ID;
dev->slaveSelect = 0;

SpiConfig = XSpiPs_LookupConfig(dev->deviceId);
if (!SpiConfig) return XST_FAILURE;

Status = XSpiPs_CfgInitialize(&dev->spiInstance, SpiConfig, SpiConfig->BaseAddress);
if (Status != XST_SUCCESS) return Status;

switch (dev->mode) {
    case 0: Status = XSpiPs_SetOptions(&dev->spiInstance, XSPIPS_MASTER_OPTION | XSPIPS_FORCE_SSELECT_OPTION); break;
    case 1: Status = XSpiPs_SetOptions(&dev->spiInstance, XSPIPS_MASTER_OPTION | XSPIPS_FORCE_SSELECT_OPTION | XSPIPS_CLK_PHASE_1_OPTION); break;
    case 2: Status = XSpiPs_SetOptions(&dev->spiInstance, XSPIPS_MASTER_OPTION | XSPIPS_FORCE_SSELECT_OPTION | XSPIPS_CLK_ACTIVE_LOW_OPTION); break;
    case 3: Status = XSpiPs_SetOptions(&dev->spiInstance, XSPIPS_MASTER_OPTION | XSPIPS_FORCE_SSELECT_OPTION | XSPIPS_CLK_PHASE_1_OPTION | XSPIPS_CLK_ACTIVE_LOW_OPTION); break;
    default: Status = XSpiPs_SetOptions(&dev->spiInstance, XSPIPS_MASTER_OPTION | XSPIPS_FORCE_SSELECT_OPTION); break;
}
if (Status != XST_SUCCESS) return Status;

u32 inputClk = dev->spiInstance.Config.InputClockHz;
u32 prescaler = 1;
u32 clkDiv = inputClk / dev->clockHz;

while ((prescaler < 256) && (clkDiv > prescaler)) prescaler <<= 1;

switch (prescaler) {
    case 4:   Status = XSpiPs_SetClkPrescaler(&dev->spiInstance, XSPIPS_CLK_PRESCALE_4); break;
    case 8:   Status = XSpiPs_SetClkPrescaler(&dev->spiInstance, XSPIPS_CLK_PRESCALE_8); break;
    case 16:  Status = XSpiPs_SetClkPrescaler(&dev->spiInstance, XSPIPS_CLK_PRESCALE_16); break;
    case 32:  Status = XSpiPs_SetClkPrescaler(&dev->spiInstance, XSPIPS_CLK_PRESCALE_32); break;
    case 64:  Status = XSpiPs_SetClkPrescaler(&dev->spiInstance, XSPIPS_CLK_PRESCALE_64); break;
    case 128: Status = XSpiPs_SetClkPrescaler(&dev->spiInstance, XSPIPS_CLK_PRESCALE_128); break;
    case 256: Status = XSpiPs_SetClkPrescaler(&dev->spiInstance, XSPIPS_CLK_PRESCALE_256); break;
}
return Status;
```

}

uint32_t afeSpiRawRead(uint8_t afeInst, uint16_t addr, uint16_t *readVal)
{
int Status;
SpiConfig_t *dev = &spiDevices[afeInst];

```
XSpiPs_SetSlaveSelect(&dev->spiInstance, afeInst);
XSpiPs_Enable(&dev->spiInstance);

SendBuf[0] = (addr >> 8) & 0xFF;
SendBuf[1] = addr & 0xFF;

if (dev->dataBits == 16) {
    SendBuf[2] = 0x00;
    SendBuf[3] = 0x00;
    Status = XSpiPs_PolledTransfer(&dev->spiInstance, SendBuf, RecvBuf, 4);
    *readVal = unpackData(&RecvBuf[2], dev->bitOrder);
} else {
    SendBuf[2] = 0x00;
    Status = XSpiPs_PolledTransfer(&dev->spiInstance, SendBuf, RecvBuf, 3);
    *readVal = RecvBuf[2];
}

return (Status == XST_SUCCESS) ? TI_AFE_RET_EXEC_PASS : TI_AFE_RET_EXEC_FAIL;
```

}

uint32_t afeSpiRawWrite(uint8_t afeInst, uint16_t addr, uint16_t data)
{
int Status;
SpiConfig_t *dev = &spiDevices[afeInst];

```
XSpiPs_SetSlaveSelect(&dev->spiInstance, afeInst);
XSpiPs_Enable(&dev->spiInstance);

SendBuf[0] = (addr >> 8) & 0xFF;
SendBuf[1] = addr & 0xFF;

if (dev->dataBits == 16) {
    packData(&SendBuf[2], data, dev->bitOrder);
    Status = XSpiPs_PolledTransfer(&dev->spiInstance, SendBuf, RecvBuf, 4);
} else {
    SendBuf[2] = (u8)data;
    Status = XSpiPs_PolledTransfer(&dev->spiInstance, SendBuf, RecvBuf, 3);
}

return (Status == XST_SUCCESS) ? TI_AFE_RET_EXEC_PASS : TI_AFE_RET_EXEC_FAIL;
```

}

uint32_t afeSpiBurstRead(uint8_t afeInst, uint16_t addr, uint16_t dataArraySize, uint8_t *data)
{
int Status;
SpiConfig_t *dev = &spiDevices[afeInst];
uint16_t chunkSize;
uint16_t offset = 0;
u8 bytesPerData = (dev->dataBits == 16) ? 2 : 1;

```
XSpiPs_SetSlaveSelect(&dev->spiInstance, afeInst);
XSpiPs_Enable(&dev->spiInstance);

do {
    chunkSize = (dataArraySize > (MAX_DATA - 2) / bytesPerData)
                    ? (MAX_DATA - 2) / bytesPerData
                    : dataArraySize;

    SendBuf[0] = (addr >> 8) & 0xFF;
    SendBuf[1] = addr & 0xFF;
    for (u16 i = 0; i < chunkSize * bytesPerData; i++) {
        SendBuf[i + 2] = 0x00;
    }

    Status = XSpiPs_PolledTransfer(&dev->spiInstance, SendBuf, RecvBuf, 2 + chunkSize * bytesPerData);

    for (u16 i = 0; i < chunkSize * bytesPerData; i++) {
        data[offset * bytesPerData + i] = RecvBuf[i + 2];
    }

    offset += chunkSize;
    addr += chunkSize;
    dataArraySize -= chunkSize;

    if (Status != XST_SUCCESS) return TI_AFE_RET_EXEC_FAIL;
} while (dataArraySize > 0);

return TI_AFE_RET_EXEC_PASS;
```

}

uint32_t afeSpiBurstWrite(uint8_t afeInst, uint16_t addr, uint16_t dataArraySize, uint8_t *data)
{
int Status;
SpiConfig_t *dev = &spiDevices[afeInst];
uint16_t chunkSize;
uint16_t offset = 0;
u8 bytesPerData = (dev->dataBits == 16) ? 2 : 1;

```
XSpiPs_SetSlaveSelect(&dev->spiInstance, afeInst);
XSpiPs_Enable(&dev->spiInstance);

do {
    uint16_t maxBytes = (MAX_DATA / (2 + bytesPerData));
    chunkSize = (dataArraySize > maxBytes) ? maxBytes : dataArraySize;

    uint16_t sendIndex = 0;
    for (uint16_t i = 0; i < chunkSize; i++) {
        uint16_t currentAddr = addr + i;
        SendBuf[sendIndex++] = (currentAddr >> 8) & 0xFF;
        SendBuf[sendIndex++] = currentAddr & 0xFF;

        if (dev->dataBits == 16)
            packData(&SendBuf[sendIndex], ((u16 *)data)[offset + i], dev->bitOrder),
            sendIndex += 2;
        else
            SendBuf[sendIndex++] = data[offset + i];
    }

    Status = XSpiPs_PolledTransfer(&dev->spiInstance, SendBuf, RecvBuf, sendIndex);
    if (Status != XST_SUCCESS) return TI_AFE_RET_EXEC_FAIL;

    offset += chunkSize;
    addr += chunkSize;
    dataArraySize -= chunkSize;
} while (dataArraySize > 0);

return TI_AFE_RET_EXEC_PASS;
```

}

// Multi-instance functions (inherit same 8/16-bit support)
uint32_t afeSpiRawWriteMulti(uint8_t afeInstSel, uint16_t addr, uint16_t data)
{
for (uint8_t afeInst = 0; afeInst < NUM_SPI; afeInst++) {
if (afeInstSel & (1 << afeInst)) {
SpiConfig_t *dev = &spiDevices[afeInst];
int Status;

```
        XSpiPs_SetSlaveSelect(&dev->spiInstance, afeInst);
        XSpiPs_Enable(&dev->spiInstance);

        SendBuf[0] = (addr >> 8) & 0xFF;
        SendBuf[1] = addr & 0xFF;

        if (dev->dataBits == 16) {
            packData(&SendBuf[2], data, dev->bitOrder);
            Status = XSpiPs_PolledTransfer(&dev->spiInstance, SendBuf, RecvBuf, 4);
        } else {
            SendBuf[2] = (u8)data;
            Status = XSpiPs_PolledTransfer(&dev->spiInstance, SendBuf, RecvBuf, 3);
        }

        if (Status != XST_SUCCESS) return TI_AFE_RET_EXEC_FAIL;
    }
}
return TI_AFE_RET_EXEC_PASS;
```

}

uint32_t afeSpiRawReadMulti(uint8_t afeInstSel, uint16_t addr, uint16_t *readVal)
{
for (uint8_t i = 0; i < NUM_SPI; i++) readVal[i] = 0;

```
for (uint8_t afeInst = 0; afeInst < NUM_SPI; afeInst++) {
    if (afeInstSel & (1 << afeInst)) {
        SpiConfig_t *dev = &spiDevices[afeInst];
        int Status;

        XSpiPs_SetSlaveSelect(&dev->spiInstance, afeInst);
        XSpiPs_Enable(&dev->spiInstance);

        SendBuf[0] = ((addr >> 8) & 0x7F) | 0x80;
        SendBuf[1] = addr & 0xFF;

        if (dev->dataBits == 16) {
            SendBuf[2] = SendBuf[3] = 0x00;
            Status = XSpiPs_PolledTransfer(&dev->spiInstance, SendBuf, RecvBuf, 4);
            readVal[afeInst] = unpackData(&RecvBuf[2], dev->bitOrder);
        } else {
            SendBuf[2] = 0x00;
            Status = XSpiPs_PolledTransfer(&dev->spiInstance, SendBuf, RecvBuf, 3);
            readVal[afeInst] = RecvBuf[2];
        }

        if (Status != XST_SUCCESS) return TI_AFE_RET_EXEC_FAIL;
    }
}

return TI_AFE_RET_EXEC_PASS;
```

}

uint32_t afeSpiBurstWriteMulti(uint8_t afeInstSel, uint16_t addr, uint8_t *data, uint16_t dataArraySize)
{
for (uint8_t afeInst = 0; afeInst < NUM_SPI; afeInst++) {
if (afeInstSel & (1 << afeInst)) {
SpiConfig_t *dev = &spiDevices[afeInst];
int Status;

```
        XSpiPs_SetSlaveSelect(&dev->spiInstance, afeInst);
        XSpiPs_Enable(&dev->spiInstance);

        uint16_t bytesRemaining = dataArraySize;
        uint16_t offset = 0;
        u8 bytesPerData = (dev->dataBits == 16) ? 2 : 1;

        while (bytesRemaining > 0) {
            uint16_t maxBytes = (MAX_DATA / (2 + bytesPerData));
            uint16_t chunkSize = (bytesRemaining > maxBytes) ? maxBytes : bytesRemaining;
            uint16_t sendIndex = 0;

            for (uint16_t i = 0; i < chunkSize; i++) {
                uint16_t currAddr = addr + offset + i;
                SendBuf[sendIndex++] = (currAddr >> 8) & 0xFF;
                SendBuf[sendIndex++] = currAddr & 0xFF;

                if (dev->dataBits == 16)
                    packData(&SendBuf[sendIndex], ((u16 *)data)[offset + i], dev->bitOrder),
                    sendIndex += 2;
                else
                    SendBuf[sendIndex++] = data[offset + i];
            }

            Status = XSpiPs_PolledTransfer(&dev->spiInstance, SendBuf, RecvBuf, sendIndex);
            if (Status != XST_SUCCESS) return TI_AFE_RET_EXEC_FAIL;

            offset += chunkSize;
            bytesRemaining -= chunkSize;
        }
    }
}
return TI_AFE_RET_EXEC_PASS;


}
