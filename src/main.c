#include "afeSpi.h"
#include "spips.h"
#include <stdio.h>
#include <stdint.h>
#include "xil_printf.h"
#include "afeSpi.h" // Your consolidated SPI driver header

#define MAX_DATA 128
u8 tx_buf[MAX_DATA]= {1,2,3,4,5,6};
// External variables from afeSpi.c
extern XSpiPs SpiInstance;
extern signed int dig_T1, dig_T2, dig_T3;

int main()
{
    uint8_t RecvData[MAX_DATA];
    uint32_t Status;
    int32_t dat1, dat2, dat3; // Use int32_t for signed integer data
    int32_t temper;


    xil_printf("Initializing SPI for BME280 sensor...");

    // Initialize the SPI peripheral
    Status = SpiPs_Init(XPAR_XSPIPS_0_DEVICE_ID,2);
    if (Status != TI_AFE_RET_EXEC_PASS) {
        xil_printf("SPI initialization failed!\n");
        return XST_FAILURE;
    } else {
        xil_printf("SPI initialization success!\n");
    }

    // Step 1: Read the sensor's ID to verify communication
    uint8_t sensor_id;
    Status = afeSpiRawRead(0, 0xD0, &sensor_id);
    if (Status != TI_AFE_RET_EXEC_PASS) {
        xil_printf("Sensor ID verification failed");
        return XST_FAILURE;
    }
    xil_printf("BME280 sensor found. ID: 0x%02X\n", sensor_id);


    Status = afeSpiRawWrite(0, 0xF4, 0x27);
    if (Status != TI_AFE_RET_EXEC_PASS) {
        xil_printf("Sensor configuration failed!\n");
        return XST_FAILURE;
    }
    xil_printf("Sensor configuration write success.\n");

    Status = afeSpiBurstWrite(0, 0x88, &tx_buf,6);
    if (Status != TI_AFE_RET_EXEC_PASS) {
            xil_printf("Burst write of calibration data failed!\n");
            return XST_FAILURE;
        }

    Status = afeSpiBurstRead(0, 0x88, 6, RecvData);
    if (Status != TI_AFE_RET_EXEC_PASS) {
        xil_printf("Burst read of calibration data failed!\n");
        return XST_FAILURE;
    }

    // Combine 8-bit registers into signed 16-bit calibration values
    dat1 = (RecvData[1] << 8) | RecvData[0];
    dat2 = (RecvData[3] << 8) | RecvData[2];
    dat3 = (RecvData[5] << 8) | RecvData[4];

    // Store them in the global variables for the temperature calculation function
    dig_T1 = dat1;
    dig_T2 = dat2;
    dig_T3 = dat3;

    xil_printf("Calibration data read successfully.\n");

    // Step 4: Read raw temperature data from 0xFA (3 bytes)
    // The temperature data on BME280 is a 20-bit value
    Status = afeSpiBurstRead(0, 0xFA, 3, RecvData);
    if (Status != TI_AFE_RET_EXEC_PASS) {
        xil_printf("Burst read of temperature data failed!\n");
        return XST_FAILURE;
    }


    // Combine 3 bytes into a 20-bit raw ADC value
    temper = (RecvData[0] << 12) | (RecvData[1] << 4) | (RecvData[2] >> 4);
    xil_printf("Raw temperature ADC value: %ld\n", temper);

    // Step 5: Calculate and print the temperature using the compensation algorithm
    temperature(temper);

    return 0;
}
