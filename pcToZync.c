#include "xparameters.h"
#include "xgpio.h"
#include "Led_ip.h"
//====================================================

int main (void)
{
   XGpio dip, push;
   int i, psb_check, dip_check;

   xil_printf("-- Start of the Program --\r\n");

   // Initialize GPIO for DIP switches
   XGpio_Initialize(&dip, XPAR_SWITCHES_DEVICE_ID);
   XGpio_SetDataDirection(&dip, 1, 0xffffffff);

   // Initialize GPIO for push buttons
   XGpio_Initialize(&push, XPAR_BUTTONS_DEVICE_ID);
   XGpio_SetDataDirection(&push, 1, 0xffffffff);

   while (1)
   {
      // Read the status of the push buttons
      psb_check = XGpio_DiscreteRead(&push, 1);
      xil_printf("Push Buttons Status %x\r\n", psb_check);

      // Read the status of the DIP switches
      dip_check = XGpio_DiscreteRead(&dip, 1);
      xil_printf("DIP Switch Status %x\r\n", dip_check);

      // Output DIP switches value on LED_ip device
      LED_IP_mWriteReg(XPAR_LED_S_AXI_BASEADDR, 0, dip_check);

      // Blink LEDs based on DIP switch status
      if (dip_check) { // If any DIP switch is ON
          // Blink the LEDs
          for (i = 0; i < 5; i++) { // Blink 5 times
              LED_IP_mWriteReg(XPAR_LED_S_AXI_BASEADDR, 0, 0xFF); // Turn ON all LEDs
              for (int j = 0; j < 9999999; j++); // Delay
              LED_IP_mWriteReg(XPAR_LED_S_AXI_BASEADDR, 0, 0x00); // Turn OFF all LEDs
              for (int j = 0; j < 9999999; j++); // Delay
          }
      } else {
          // If no DIP switch is ON, keep LEDs OFF
          LED_IP_mWriteReg(XPAR_LED_S_AXI_BASEADDR, 0, 0x00);
      }
   }
}
