
#include "spips.h"

#define SPI_DEVICE_ID    XPAR_XSPIPS_0_DEVICE_ID
#define MAX_DATA         10

  // keep it static to this file
XSpiPs SpiInstance;
int SpiPs_Init(u16 SpiDeviceId, int clk_option)
{
    int Status;
    XSpiPs_Config *SpiConfig;

    SpiConfig = XSpiPs_LookupConfig(SpiDeviceId);
    if (SpiConfig == NULL)
        return XST_FAILURE;

    Status = XSpiPs_CfgInitialize(&SpiInstance, SpiConfig, SpiConfig->BaseAddress);
    if (Status != XST_SUCCESS) {
        xil_printf("CfgInitialize failed!\n");
        return XST_FAILURE;
    }
    xil_printf("CfgInitialize success!\n");

    switch(clk_option) {
           case 1:
               XSpiPs_SetClkPrescaler(&SpiInstance, XSPIPS_CLK_PRESCALE_16);
               xil_printf("Using prescaler 8 - 10.4Mhz\n");
               break;
           case 2:
               XSpiPs_SetClkPrescaler(&SpiInstance, XSPIPS_CLK_PRESCALE_32);
               xil_printf("Using prescaler 32 - 5.2Mhz\n");
               break;
           case 3:
               XSpiPs_SetClkPrescaler(&SpiInstance, XSPIPS_CLK_PRESCALE_64);
               xil_printf("Using prescaler 128 - 2.6Mhz\n");
               break;
           case 4:
               XSpiPs_SetClkPrescaler(&SpiInstance, XSPIPS_CLK_PRESCALE_128);
               xil_printf("Using prescaler 128 - 1.3Mhz\n");
               break;
           default:
               XSpiPs_SetClkPrescaler(&SpiInstance, XSPIPS_CLK_PRESCALE_64);
               xil_printf("Using default prescaler 64 - 2.6Mhz\n");
               break;
       }

    Status = XSpiPs_SetOptions(&SpiInstance,
                               XSPIPS_MASTER_OPTION |
                               XSPIPS_FORCE_SSELECT_OPTION);


    if (Status != XST_SUCCESS)
        return XST_FAILURE;



    // Select slave 0 (adjust depending on your wiring)
    XSpiPs_SetSlaveSelect(&SpiInstance, 0x00);

    XSpiPs_Enable(&SpiInstance);
    return XST_SUCCESS;
}

