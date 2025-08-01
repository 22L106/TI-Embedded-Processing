#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xgpio.h"
#include "xparameters.h"
#include "xuartlite.h"
#include "sleep.h"  // for usleep()

#define UART_DEVICE_ID XPAR_UARTLITE_0_DEVICE_ID
#define GPIO_DEVICE_ID XPAR_AXI_GPIO_0_DEVICE_ID

XUartLite uart;
XGpio gpio;

int delay_us = 500000;  // initial delay: 0.5 seconds
int blinking = 0;

void gpio_init() {
    int status = XGpio_Initialize(&gpio, GPIO_DEVICE_ID);
    if (status == XST_SUCCESS)
        xil_printf("GPIO initialized\n");
    else
        xil_printf("GPIO init failed\n");

    XGpio_SetDataDirection(&gpio, 1, 0x0000);  // channel 1 as output
}

void uart_init() {
    int status = XUartLite_Initialize(&uart, UART_DEVICE_ID);
    if (status == XST_SUCCESS)
        xil_printf("UART initialized\n");
    else
        xil_printf("UART init failed\n");
}

int main() {
    init_platform();
    gpio_init();
    uart_init();

    xil_printf("Commands: s=start, p=pause, +=faster, -=slower\n");

    u32 led_vals[] = {0x1111, 0x2222, 0x1212, 0x3333, 0xFFFF, 0x0000};
    int index = 0;

    while (1) {
        u8 recv;
        if (XUartLite_Recv(&uart, &recv, 1) == 1) {
            char ch = (char)recv;

            switch (ch) {
                case 's':
                    blinking = 1;
                    xil_printf("Blinking started\n");
                    break;
                case 'p':
                    blinking = 0;
                    xil_printf("Blinking paused\n");
                    break;
                case '+':
                    if (delay_us > 100000) delay_us -= 100000;  // reduce delay by 0.1s
                    xil_printf("Speed increased, delay = %d ms\n", delay_us / 1000);
                    break;
                case '-':
                    delay_us += 100000;  // increase delay by 0.1s
                    xil_printf("Speed decreased, delay = %d ms\n", delay_us / 1000);
                    break;
                case '\n':
                case '\r':
                    break;  // ignore newline and carriage return
                default:
                    xil_printf("Unknown command: %c\n", ch);
            }
        }

        if (blinking) {
            XGpio_DiscreteWrite(&gpio, 1, led_vals[index]);
            xil_printf("LED: 0x%04X\n", led_vals[index]);
            index = (index + 1) % 6;
            usleep(delay_us);  // finer delay control
        }
    }

    cleanup_platform();
    return 0;
}
