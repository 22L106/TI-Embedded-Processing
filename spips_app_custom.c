#include "spips.h"
u8 ReadBuf[MAX_DATA];
u8 SendBuf[MAX_DATA];

int main(void)
{
	int i =0;
	SpiPs_Init(SPI_DEVICE_ID);

	for(i=0;i<10;i++)
		SendBuf[i]=i;

	SpiPs_Send(SendBuf,10);
	SpiPs_Read(ReadBuf,10);

	for(i=0;i<10;i++)
		xil_printf("%d,",ReadBuf[i]);

	return 0;
}
