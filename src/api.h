#ifndef SPI_DRIVER_H
#define SPI_DRIVER_H

#include "xparameters.h"
#include "xspips.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xstatus.h"

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// Constants and Macros
// -----------------------------------------------------------------------------
#define MAX_DATA 128
#define NUM_SPI  4

#define TI_AFE_RET_EXEC_PASS   0
#define TI_AFE_RET_EXEC_FAIL   1

#define SPI_BITORDER_MSB_FIRST 0
#define SPI_BITORDER_LSB_FIRST 1

#define SPI_MODE_0   0
#define SPI_MODE_1   1
#define SPI_MODE_2   2
#define SPI_MODE_3   3

#define SPI_DATA_8BIT   8
#define SPI_DATA_16BIT  16

// -----------------------------------------------------------------------------
// Structures
// -----------------------------------------------------------------------------
typedef struct {
u32 clockHz;          // SPI clock frequency
u8  bitOrder;         // 0 = MSB first, 1 = LSB first
u8  mode;             // SPI mode (0–3)
u8  addrBits;         // Address width in bits
u8  dataBits;         // 8 or 16 bits per transfer
XSpiPs spiInstance;   // SPI driver instance (from Xilinx library)
u16 deviceId;         // SPI device ID (from xparameters.h)
u8  slaveSelect;      // Slave select line
} SpiConfig_t;

typedef enum {
TI_AFE_RET_EXEC_PASS_ENUM = 0,
TI_AFE_RET_EXEC_FAIL_ENUM = 1
} RetType_e;

// -----------------------------------------------------------------------------
// Global Variables
// -----------------------------------------------------------------------------
extern SpiConfig_t spiDevices[NUM_SPI];
extern u8 SendBuf[MAX_DATA];
extern u8 RecvBuf[MAX_DATA];

// -----------------------------------------------------------------------------
// Function Prototypes
// -----------------------------------------------------------------------------

/**

* @brief Initialize a specific SPI instance.
*
* @param afeInst  SPI instance index (0–NUM_SPI-1)
* @param clockHz  Desired SPI clock frequency
* @param bitOrder Bit order (SPI_BITORDER_MSB_FIRST / SPI_BITORDER_LSB_FIRST)
* @param mode     SPI mode (0–3)
* @param addrBits Address width in bits
* @param dataBits Data width (8 or 16)
* @return XST_SUCCESS or XST_FAILURE
  */
  uint32_t SPI_init(u8 afeInst, u32 clockHz, u8 bitOrder, u8 mode, u8 addrBits, u8 dataBits);

/**

* @brief Write a single register to SPI slave.
  */
  uint32_t afeSpiRawWrite(uint8_t afeInst, uint16_t addr, uint16_t data);

/**

* @brief Read a single register from SPI slave.
  */
  uint32_t afeSpiRawRead(uint8_t afeInst, uint16_t addr, uint16_t *readVal);

/**

* @brief Perform burst read from SPI slave.
  */
  uint32_t afeSpiBurstRead(uint8_t afeInst, uint16_t addr, uint16_t dataArraySize, uint8_t *data);

/**

* @brief Perform burst write to SPI slave.
  */
  uint32_t afeSpiBurstWrite(uint8_t afeInst, uint16_t addr, uint16_t dataArraySize, uint8_t *data);

/**

* @brief Write to multiple SPI slaves simultaneously.
  */
  uint32_t afeSpiRawWriteMulti(uint8_t afeInstSel, uint16_t addr, uint16_t data);

/**

* @brief Read from multiple SPI slaves simultaneously.
  */
  uint32_t afeSpiRawReadMulti(uint8_t afeInstSel, uint16_t addr, uint16_t *readVal);

/**

* @brief Perform burst write to multiple SPI slaves simultaneously.
  */
  uint32_t afeSpiBurstWriteMulti(uint8_t afeInstSel, uint16_t addr, uint8_t *data, uint16_t dataArraySize);

// -----------------------------------------------------------------------------
// Inline Helper Functions
// -----------------------------------------------------------------------------
static inline void packData(u8 *buf, u16 data, u8 bitOrder);
static inline u16 unpackData(u8 *buf, u8 bitOrder);

#ifdef __cplusplus
}
#endif

#endif // SPI_DRIVER_H
