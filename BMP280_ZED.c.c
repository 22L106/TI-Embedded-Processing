#include "xparameters.h"
#include "xspips.h"
#include "xil_printf.h"

#define SPI_DEVICE_ID    XPAR_XSPIPS_0_DEVICE_ID
#define MAX_DATA         128

XSpiPs SpiInstance;
u8 SendBuf[MAX_DATA];
u8 RecvBuf[MAX_DATA];
signed int dig_T1, dig_T2, dig_T3;

int SpiPs_Init(u16 SpiDeviceId)
{
    int Status;
    XSpiPs_Config *SpiConfig;

    SpiConfig = XSpiPs_LookupConfig(SpiDeviceId);
    if (SpiConfig == NULL)
        return XST_FAILURE;

    Status = XSpiPs_CfgInitialize(&SpiInstance, SpiConfig, SpiConfig->BaseAddress);
    if (Status != XST_SUCCESS) {
    xil_printf("CfgInitialize failed!\n");
    return XST_FAILURE;}

    Status = XSpiPs_SetOptions(&SpiInstance,XSPIPS_MASTER_OPTION | XSPIPS_FORCE_SSELECT_OPTION);
    if (Status != XST_SUCCESS)
        return XST_FAILURE;

    XSpiPs_SetClkPrescaler(&SpiInstance, XSPIPS_CLK_PRESCALE_64);

    XSpiPs_SetSlaveSelect(&SpiInstance, 0x00);

    XSpiPs_Enable(&SpiInstance);
    return XST_SUCCESS;
}

int Single_Read(u8 address, u8* RecvData)
{
int Status;
SendBuf[0] = address | 0x80;
SendBuf[1] = 0x00;

Status = XSpiPs_PolledTransfer(&SpiInstance, SendBuf, RecvBuf, 2);
RecvData[0] = RecvBuf[1];
xil_printf("Data at address %02X\n", address);
xil_printf("%02X ", RecvData[0]);
xil_printf("\n");

if (Status != XST_SUCCESS) {
       return XST_FAILURE;}
    else{return XST_SUCCESS;}
}

int Single_write(u8 address, u8 data)
{
int Status;
SendBuf[0] = address | 0x7F;
SendBuf[1] = data;

Status = XSpiPs_PolledTransfer(&SpiInstance, SendBuf, RecvBuf, 2);

if (Status != XST_SUCCESS) {
       return XST_FAILURE;}
    else{return XST_SUCCESS;}
}

int burstRead(u8 address, u8 NUM_BYTES, u8 *RecvData){
    int Status;
    SendBuf[0] = address | 0x80;
    for(u8 i=1;i<=NUM_BYTES;i++){
    SendBuf[i] = 0x00;
    }
    Status = XSpiPs_PolledTransfer(&SpiInstance, SendBuf, RecvBuf, NUM_BYTES+1);

    xil_printf("%d Data at address %02X\n", NUM_BYTES, address);

    for(u8 i=1;i<=NUM_BYTES;i++){
        RecvData[i-1] = RecvBuf[i];
        xil_printf("%02X ", RecvBuf[i]);
    }
    xil_printf("\n");

    if (Status != XST_SUCCESS) {
        return XST_FAILURE;}
        else{return XST_SUCCESS;}
}

int burstWrite(u8 address, u8* data, u8 NUM_BYTES){
    int Status;

    u8 j = 0;
    for(u8 i=0; i<NUM_BYTES; i++){
        SendBuf[j++] = address & 0x7F;
        SendBuf[j++] = data[i];
        address++;
    }
    Status = XSpiPs_PolledTransfer(&SpiInstance, SendBuf, RecvBuf, NUM_BYTES*2);

    if (Status != XST_SUCCESS) {
        return XST_FAILURE;}
        else{return XST_SUCCESS;}
}


int main()
{
    //other than receive buffer, an array is created to store the data that is received
    u8 RecvData[MAX_DATA];
    int Status;
    signed int dat1, dat2, dat3;
    u16 temper;

    Status = SpiPs_Init(SPI_DEVICE_ID);

    if (Status != XST_SUCCESS)
    {   xil_printf("SPI init failed\n");
        return XST_FAILURE;}
    else
    {   xil_printf("SPI init success\n");}

    /*Status = Single_Read(0xD0);
    if (Status != XST_SUCCESS) {
            xil_printf("Single read failed\n");
            return XST_FAILURE;}

    Status = Single_Read(0xFA);
    if (Status != XST_SUCCESS) {
                xil_printf("Single read failed\n");
                return XST_FAILURE;}

    Status = Single_write(0x60, 0xB6);
    if (Status != XST_SUCCESS) {
                    xil_printf("Single read failed\n");
                    return XST_FAILURE;}

    Status = Single_Read(0xFA);
        if (Status != XST_SUCCESS) {
                    xil_printf("Single read failed\n");
                    return XST_FAILURE;}

    SendBuf[0] = 0xD0;
    SendBuf[1] = 0x00;

    XSpiPs_PolledTransfer(&SpiInstance, SendBuf, RecvBuf, 2);

    xil_printf("%x",RecvBuf[1]);*/

    u8 data[2] = {0x23, 0x00};

    Status = burstWrite(0x74, data, 2);
    if (Status != XST_SUCCESS) {
        xil_printf("Burst write failed\n");
        return XST_FAILURE;}

    Status = burstRead(0x88, 6, RecvData);
    if (Status != XST_SUCCESS) {
        xil_printf("Burst read failed\n");
        return XST_FAILURE;
    }
    dat1 = (RecvData[1] << 8) | RecvData[0];
    dat2 = (RecvData[3] << 8) | RecvData[2];
    dat3 = (RecvData[5] << 8) | RecvData[4];

    dig_T1 = dat1;
    dig_T2 = dat2;
    dig_T3 = dat3;

    Status = burstRead(0xFA, 2, RecvData);
    if (Status != XST_SUCCESS) {
        xil_printf("Burst read failed\n");
        return XST_FAILURE;
    }
    temper = (RecvData[0] << 8) | RecvData[1];

    temperature(temper);
    return 0;

}
