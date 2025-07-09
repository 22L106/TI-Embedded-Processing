#include <reg51.h>

// Port 2 LEDs
sbit LED0 = P2^0;
sbit LED1 = P2^1;
sbit LED2 = P2^2;
sbit LED3 = P2^3;
sbit LED4 = P2^4;
sbit LED5 = P2^5;
sbit LED6 = P2^6;
sbit LED7 = P2^7;


void UART_Tx(char ch) {
    SBUF = ch;
    while (!TI);
    TI = 0;
}

void UART_SendString(char *str) {
    while (*str)
        UART_Tx(*str++);
}

void clear_all_leds() {
    LED0 = LED1 = LED2 = LED3 = LED4 = LED5 = LED6 = LED7 = 0;
}

void main() {
		SCON = 0x50;
    TMOD = 0x20;   // Timer1, Mode 2  [Gate- C/T'- M1- M0 (8bit autoreload mode)]
    TH1 = 0xFD;    // 9600 baud  
    SCON = 0x50;   // 8-bit UART mode
    TR1 = 1;       // Start timer    clear_all_leds();

    while (1) {
        if (RI) {
            char ch = SBUF;
            RI = 0;
            clear_all_leds();

            switch (ch) {
                case '0': LED0 = 1; UART_SendString("LED 0 is ON\r\n"); break;
                case '1': LED1 = 1; UART_SendString("LED 1 is ON\r\n"); break;
                case '2': LED2 = 1; UART_SendString("LED 2 is ON\r\n"); break;
                case '3': LED3 = 1; UART_SendString("LED 3 is ON\r\n"); break;
                case '4': LED4 = 1; UART_SendString("LED 4 is ON\r\n"); break;
                case '5': LED5 = 1; UART_SendString("LED 5 is ON\r\n"); break;
                case '6': LED6 = 1; UART_SendString("LED 6 is ON\r\n"); break;
                case '7': LED7 = 1; UART_SendString("LED 7 is ON\r\n"); break;
                default: UART_SendString("Invalid input\r\n"); break;
            }
        }
    }
}




















































