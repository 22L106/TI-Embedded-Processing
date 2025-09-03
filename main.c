#include "afeSpi.h"

int main() {
    int Status;
    uint8_t readVal;

    xil_printf("AFE SPI Driver Test...\n");


    //Initialize
    Status = SpiPs_Init(XPAR_XSPIPS_0_DEVICE_ID, 2);  // prescaler option
    if (Status == TI_AFE_RET_EXEC_FAIL) {
        xil_printf("SPI init failed!\n");
        return TI_AFE_RET_EXEC_FAIL;
    }


    //Write
    Status = afeSpiRawWrite(0, 0x0010, 0x55);
    if(Status==TI_AFE_RET_EXEC_FAIL){
    	xil_printf("Write failed...\n");
    	return TI_AFE_RET_EXEC_FAIL;
    }


    //read
    Status = afeSpiRawRead(0, 0x0010, &readVal);
    if(Status==TI_AFE_RET_EXEC_FAIL){
       	xil_printf("Read failed...\n");
       	return TI_AFE_RET_EXEC_FAIL;
       }





    uint8_t burstData[4] = {0x11, 0x22, 0x33, 0x44};

    // Burst write to address 0x0020
    Status = afeSpiBurstWrite(0, 0x0020, burstData, 4);
    if(Status == TI_AFE_RET_EXEC_FAIL) {
        xil_printf("Burst Write failed!\n");
    }

       uint8_t txData[300];
       uint8_t rxVal;

       xil_printf("AFE SPI Burst Write Test...\n");

       // Initialize SPI
       Status = SpiPs_Init(XPAR_XSPIPS_0_DEVICE_ID, 2);  // prescaler option
       if (Status == TI_AFE_RET_EXEC_FAIL) {
           xil_printf("SPI init failed!\n");
           return TI_AFE_RET_EXEC_FAIL;
       }

       // Prepare test data >128 bytes
       for (int i = 0; i < 300; i++) {
           txData[i] = i & 0xFF;   // pattern 00,01,02...
       }

       // Perform burst write
       Status = afeSpiBurstWrite(0, 0x0020, txData, 300);
       if (Status == TI_AFE_RET_EXEC_FAIL) {
           xil_printf("Burst Write failed!\n");
           return TI_AFE_RET_EXEC_FAIL;
       }

       xil_printf("Burst Write complete!\n");

    return TI_AFE_RET_EXEC_PASS;
}
