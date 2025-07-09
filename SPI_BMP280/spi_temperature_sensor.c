#include<reg51.h>
sbit cs = P0^0;
sbit sck = P0^1;
sbit mosi = P0^2;
sbit miso = P0^3;

//control pins for 7 segment display
sbit ones_disp = P0^4;
sbit tens_disp = P0^5;
sbit hunds_disp = P0^6;
sbit thou_disp = P0^7;

signed long int dig_T1, dig_T2,dig_T3;
unsigned char a[]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x6f};
unsigned char thou,hunds,tens,ones;
unsigned int c,temp;

void delay(unsigned int x){
	unsigned int i,a;
	for(i=0;i<20;i++) 
	{		a = x;
			while(a--);}
}
void disp_delay(unsigned int x)
{
	while(x--);
}

//display induvidual digits using_timer0 interrupt_
void timer0() interrupt 1
{
	ones_disp = 1;
	P1 = a[ones];
	disp_delay(150);
	ones_disp = 0;
	
	tens_disp = 1;
	P1 = a[tens];
	disp_delay(150);
	tens_disp = 0;
	
	hunds_disp = 1;
	P1 = a[hunds];
	disp_delay(150);
	hunds_disp = 0;
	
	thou_disp = 1;
	P1 = a[thou];
	disp_delay(150);
	thou_disp = 0;
}

//send data_ at the falling edge and read at the rising edge - spi : mode 3
//cpha - 1; cpol = 1;
void restart(){ //sets chip select bar to 1; since we use mode 3, sck is initially high.
	cs = 1;
	sck = 1;
	mosi = 1;
	delay(100);
}
void start(){ //pull csb is pulled low to select the chip
	cs = 0;
	sck = 1;
	delay(100);
}

// 7 bit_address; msb i.e, the eigth bit_is read/ write bar. here, we have read, so msb is set to 1.
void send_address(unsigned char addr){
		unsigned char i;
		for(i=0;i<8;i++){
			mosi = (addr&0x80)?1:0;
			sck = 0;
			delay(100);
			sck = 1;
			addr<<=1;
			delay(100);
		}
		sck = 0;
		delay(100);
}

//the msb is set 0 for write and the data to be written is passed
void write_data(unsigned char dat){
		unsigned char i;
		for(i=0;i<8;i++){
			mosi = (dat&0x80)?1:0;
			sck = 0;
			delay(100);
			sck = 1;
			dat<<=1;
			delay(100);
			//sck = 1;
		}
		sck = 0;
		delay(100);
}

//to receive the 8 bits sent by slave, we send 8 clock pulses. since spi is full duplex, 
//master out pin is set to high
unsigned char receive_data(){
		unsigned char i;
		unsigned char dat = 0x00;
		for(i=0;i<8;i++){
			dat<<=1;
			mosi = 1;
			sck = 1;
			if(miso) dat = dat|0x01;
			delay(100);
			sck =0;
			delay(100);
		}
		return dat;
}

//stop set csb high, thus stopping the communication
void stop(){
	cs = 1;
	delay(100);
}

//sets bmp280 back to sleep mode
void reset(){
	start();
	send_address(0xE0);
	write_data(0xB6);
	stop();
}

//calibration
void temperature(signed long adc_T){
			signed long int t_fine,var1,var2;
			unsigned int T;
			var1 = (((adc_T>>3) - (dig_T1<<1)) * (dig_T2)) >> 11;
			var2 = (((((adc_T>>4) - (dig_T1<<1)) * ((adc_T>>4) - (dig_T2))) >> 12) * (dig_T3)) >> 14 ;
			t_fine = var1 + var2;
			T = ((t_fine * 5 + 128) >> 8);
	
			IE = 0x82; //t0 interrupts
			TMOD = 0x00; //timer0 in 13bit timer mode
			TR0 = 1;
			c = (unsigned short)(T&0x00FF);
			while(1)
			{
				thou = c/1000;
				temp = c%1000;
				hunds = temp/100;
				temp = temp%100;
				tens = temp/10;
				ones = temp%10;
				delay(10000);
			}
}

void main(){
	reset();
	delay(100);
	while(1)
	{	  
			unsigned int dat1;
			signed int dat2,dat3;
			signed long int adc_T;
			unsigned int temper;
			//mode set
			start();
			send_address(0x74); //config - tsb[2:0], filter[2:0], spi3wire enable
			write_data(0x23); //00000000
			send_address(0x75);
			write_data(0x00);
			//dig_T
			send_address(0x88);
			dat1 = receive_data();
			dat1 |= (receive_data()<<8);
			dig_T1 = dat1; 
			dat2 = receive_data();
			dat2 |= (receive_data()<<8);
			dig_T2 = dat2;
			dat3 = receive_data();
			dat3 |= (receive_data()<<8);
			dig_T3 = dat3;
			//reading temp
			send_address(0xFA);
			temper = receive_data(); 
			temper<<=8; 
			temper = receive_data();
			adc_T = temper;
			stop();
			//calculating temp
			temperature(adc_T);
			stop();
			while(1);
	}
}




