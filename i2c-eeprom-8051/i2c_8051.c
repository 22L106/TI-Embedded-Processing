#include <reg51.h>

sbit SDA = P0^0;    // I2C Data
sbit SCL = P0^1;    // I2C Clock
sbit DISP1 = P3^0;
sbit DISP2 = P3^1;
sbit BUTTON = P0^7;


unsigned char seg_code[16] = {
    0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,
    0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71
};
#define EEPROM_ADDRESS 0xA0  // 7-bit address




void delay_ms(unsigned int ms) {
    unsigned int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 123; j++);
}

void display_hex(unsigned char value) {
    unsigned char high = (value >> 4) & 0x0F;
    unsigned char low  = value & 0x0F;

    // Show low nibble
    P1 = seg_code[low];
    DISP1 = 0; DISP2 = 1;
    delay_ms(2);

    // Show high nibble
    P1 = seg_code[high];
    DISP1 = 1; DISP2 = 0;
    delay_ms(2);
}
void delay() {
    unsigned int i, j;

    for (i = 0; i < 255; i++) {
        for (j = 0; j < 500; j++) {
            // ~1us * 500 * 255 = ~127 ms
            // Looping 8 times ˜ ~1 second
        }
    }
}
void I2C_Delay() {
    unsigned char i;
    for (i = 0; i < 255; i++);  // Short delay for timing
}
void I2C_Start() {
    SDA = 1;
    SCL = 1;
    I2C_Delay();
    SDA = 0;
    I2C_Delay();
    SCL = 0;
}

void I2C_Stop() {
    SDA = 0;
    SCL = 1;
    I2C_Delay();
    SDA = 1;
    I2C_Delay();
}

bit I2C_Write_Byte(unsigned char dat) {
    unsigned char i;
    bit ack;

    for (i = 0; i < 8; i++) {
        SDA = (dat & 0x80);  // MSB first
        SCL = 1;
        I2C_Delay();
        SCL = 0;
        dat <<= 1;
    }

    SDA = 1;          // Release SDA for ACK
    SCL = 1;
    I2C_Delay();
    ack = SDA;        // ACK should be 0
    SCL = 0;

    return !ack;      // Return 1 if ACK received
}

unsigned char I2C_Read_Byte(bit ack) {
    unsigned char i, dat = 0;

    SDA = 1;  // Release SDA
    for (i = 0; i < 8; i++) {
        dat <<= 1;
        SCL = 1;
        I2C_Delay();
        if (SDA) dat |= 1;
        SCL = 0;
    }

    SDA = !ack;  // ACK = 0, NACK = 1
    SCL = 1;
    I2C_Delay();
    SCL = 0;

    return dat;
}
bit EEPROM_Write(unsigned char addr, unsigned char dat) {
    I2C_Start();
    if (!I2C_Write_Byte(EEPROM_ADDRESS  | 0))  // Write mode
        return 0;
    if (!I2C_Write_Byte(addr))
        return 0;
    if (!I2C_Write_Byte(dat))
        return 0;
    I2C_Stop();

    // Wait for EEPROM internal write (~5ms)
    I2C_Delay(); I2C_Delay(); I2C_Delay(); I2C_Delay(); I2C_Delay();
    return 1;
}
unsigned char EEPROM_Read(unsigned char addr) {
    unsigned char dat;

    I2C_Start();
    I2C_Write_Byte((EEPROM_ADDRESS) | 0);  // Write mode
    I2C_Write_Byte(addr);
    I2C_Start();  // Repeated Start
    I2C_Write_Byte((EEPROM_ADDRESS) | 1);  // Read mode
    dat = I2C_Read_Byte(0);  // NACK after 1 byte
    I2C_Stop();

    return dat;
}




void main() {
    unsigned char count;

    // Read previously saved value
    count = EEPROM_Read(0x00);

    while (1) {
        P1 = seg_code[count & 0x0F];  // Show lower nibble

        if (BUTTON == 0) {
            delay_ms(20);
            if (BUTTON == 0) {
                count = (count + 1) & 0x0F;   // Keep within 0–15
                EEPROM_Write(0x00, count);   // Save to EEPROM
                while (BUTTON == 0);         // Wait for release
            }
        }
    }
	}