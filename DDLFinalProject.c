/*
===============================================================================
 Name        : FinalProject.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#include <cr_section_macros.h>
#include <stdio.h>
#define FIO2DIR (*(volatile unsigned int *)0x2009c040)
#define FIO2PIN (*(volatile unsigned int *)0x2009c054)
#define PINMODE4 (*(volatile unsigned int *)0x4002c050)
#define PINSEL1 (*(volatile unsigned int *)0x4002c004)
#define I2C0SCLH (*(volatile unsigned int *)0x4001c010)
#define I2C0SCLL (*(volatile unsigned int *)0x4001c014)
#define I2C0CONSET (*(volatile unsigned int *)0x4001c000)
#define I2C0CONCLR (*(volatile unsigned int *)0x4001c018)
#define I2C0STAT  (*(volatile unsigned int *)0x4001c004)
#define I2C0DAT (*(volatile unsigned int *)0x4001c008)
#define PCONP (*(volatile unsigned int *)0x400fc0c4)
#define PINSEL0 (*(volatile unsigned int *)0x4002c000)
#define CLKSRCSEL (*(volatile unsigned int *)0x400fc10c)
#define CCR (*(volatile unsigned int *)0x40024008)
#define HOUR (*(volatile unsigned int *)0x40024028)
#define MIN (*(volatile unsigned int *)0x40024024)
#define SEC (*(volatile unsigned int *)0x40024020)
#define FIO0DIR (*(volatile unsigned int *)0x2009c000)
#define FIO0PIN (*(volatile unsigned int *)0x2009c014)
#define T0TCR (*(volatile unsigned int*)0x40004004)
#define T0TC (*(volatile unsigned int*)0x40004008)
int MC23017_Write_Opcode = 0b01000000;
int MC23017_Read_Opcode =  0b01000001;
int IODIRA = 0x00;
int IODIRB = 0x01;
int GPIOA = 0x12;
int GPIOB = 0x13;
int counter = 0;
int array[6] = {0, 0, 0, 0, 0, 0};
int hourD1, hourD2, minD1, minD2,secD1, secD2 =0;
void keypadInit(){
	//configuring input
	//assigning them to 1
	for(int i =0; i < 4; i++){
		FIO2DIR &=~ (1<<i);
	}
	//adding pull down for inputs
	int j =0, k = 1;
	for(int i =0; i < 4;i++){
		PINMODE4 |= (1<<j)|(1<<k);
		j = j+2;
		k = k+2;
	}
	//configuring outputs
	//they will be automatically 1 when pressed
	for(int j =4; j < 8; j++){
		FIO2DIR |= (1<<j);
	}
}

void checkKeypadCases(){
	for(int i =0; i < 4; i++){
		for(int j = 4; j < 8; j++){
			FIO2PIN |= (1<<j);
			timer(5);
			if((((FIO2PIN >> i)& 1) == 1 ) && (((FIO2PIN >> j) & 1) == 1)){
                int result = checkButton(j-4, i+4);
                FIO2PIN &= ~(1<<j);
                array[counter] = result;
                counter++;
                timer(1000000);
                return;
			}
			FIO2PIN &= ~ (1<<j);
		}
	}
}
int checkButton(int i, int j){
	if((i == 0 && j == 4)){
		return 1;
	}
	else if(i == 1 && j == 4){
		return 2;
	}
	else if(i == 2 && j == 4){
		return 3;
	}
	else if(i == 3 && j == 4){
		return 10; // A = 10;
	}
	else if(i == 0 && j == 5){
		return 4;
	}
	else if(i == 1 && j == 5){
		return 5;
	}
	else if(i == 2 && j == 5){
		return 6;
	}
	else if(i == 3 && j == 5){
		return 11; // B =11
	}
	else if(i == 0 && j == 6){
		return 7;
	}
	else if(i == 1 && j == 6){
		return 8;
	}
	else if(i == 2 && j == 6){
		return 9;
	}
	else if(i == 3 && j == 6){
		return 12; // C = 12;
	}
	else if(i == 0 && j == 7){
		return 13; // * = 13
	}
	else if(i == 1 && j == 7){
		return 0;
	}
	else if(i == 2 && j == 7){
		return 14; // # = 14
	}
	else if(i == 2 && j == 7){
		return 15; // D = 15
	}
	else {
	}
}
void init(void){
 //clearing P0.27, controlled by bit 22 and 23 on PINSEL1
	PINSEL1 &= ~(1<22);
	PINSEL1 &= ~(1<<23);
	//setting the P0[27] = 01 to set it to SDA0
	PINSEL1 |= (1<<22);
	//clearing P0.28, controlled by bit 24 and 25 on PINSEL1
	PINSEL1 &= ~(1<<24);
	PINSEL1 &= ~(1<<25);
	//setting the P0[27] = 01 to set it to SCL0
	PINSEL1 |= (1<<24);
	//define high duration of the i2c scl
	//SETTING 50% duty cycle
	I2C0SCLH = 5;
	I2C0SCLL = 5;
	I2C0CONCLR = (1<<6); //clearing it out
	I2C0CONSET = (1<<6); //enable the 6th bit to set I2 C interface enable.
}

void start_IC(){
	I2C0CONSET = (1<<3);
	I2C0CONSET = (1<<5); //set the 5th bit to 1 to start the condition//STA
	I2C0CONCLR = (1<<3); //clearing it out
	while(!(I2C0CONSET>> 3 & 1));
	I2C0CONCLR = (1<<5); //clearing it set it to 1
}

void write_IC(int data){
	I2C0DAT = data; //writing the data to data register
	I2C0CONCLR = (1<<3); //clearing the 3rd bit
	while(!(I2C0CONSET>> 3 & 1));
}

int read_IC(){
	I2C0CONCLR = (1<<2);
	I2C0CONCLR = (1<<3);
	while(!(I2C0CONSET>> 3 & 1));
	int tempData = I2C0DAT;
	return tempData;
}

void stop_IC(){
	I2C0CONSET = (1<<4); //set the 4th bit to 1 to start the condition //STO bit
	I2C0CONCLR = (1<<3); //clearing the 3rd bit
	while(!(I2C0CONSET>> 4 & 1));
}
void writeMC(int opCode, int registerAdd, int data){
     start_IC();
     write_IC(opCode); //opCode of device
     write_IC(registerAdd); //register to write to
     write_IC(data); //data that needs to be written
     stop_IC(); //stop condition
}
void timer(int us)

{
	T0TCR |= (1<<0); //sets bit to activate timer
	T0TC = 0; //sets timer to 0 for simplicity
	while(T0TC < us);
}
void RTC_Init(){
	PCONP |= (1 << 9); //Enable the RTC power clock bit
	CCR |= (1 << 0); //enable the clock
	//choose internal clock
	CLKSRCSEL &= ~(1<<1);
	CLKSRCSEL &= ~(1<<0);
	CCR &= ~(1 << 4); //Enabling an internal clock by setting 0 to 4th bit
}
int FirstDigit(int number)
{
    int ten = 10;
    int firstDigit = number/ten;
    if(number<10)
    {
        return 0;
    }
    return firstDigit;
}

int remainder(int number)
{
    return (number % 10);
}
int segment(int number)
{
	switch(number)
	{
	case 0: return 0b00110000;
	case 1: return 0b00110001;
	case 2: return 0b00110010;
	case 3: return 0b00110011;
	case 4: return 0b00110100;
	case 5: return 0b00110101;
	case 6: return 0b00110110;
	case 7: return 0b00110111;
	case 8: return 0b00111000;
	case 9: return 0b00111001;
	}
	return 0;
}

int switchState(int wdeviceOpCode, int GPIOAadd, int rdeviceOpCode){
	start_IC();
	write_IC(wdeviceOpCode);
	write_IC(GPIOAadd);
	start_IC();
	write_IC(rdeviceOpCode);
	int data = read_IC();
	stop_IC();
	return data;
}

void LCDCommand(int hexCode){
	//R/W is already grounded
	writeMC(MC23017_Write_Opcode,GPIOA, hexCode);
	FIO0PIN &= ~(1 << 3); //drive RS low
	//these2 commands help in generating pulse
	FIO0PIN |= (1 << 2); //drive E high
	FIO0PIN &= ~(1 << 2); //drive E low
	FIO0PIN &= ~(1 << 2); //drive E low to make sure the operation is complete
	timer(100); //add 100 microseconds wait
}

void LCDWriteData(int ASCIICode){
	writeMC(MC23017_Write_Opcode,GPIOA, ASCIICode);
	FIO0PIN |= (1 << 3); //drive RS high
	//RW is connected to ground so dont need to drive it low here
	//these2 commands help in generating pulse
    FIO0PIN |= (1 << 2); //drive E high
    FIO0PIN &= ~(1 << 2); //drive E low
	timer(100); //add 100 microseconds wait to make sure the writing command is complete
}
void LCDinit(){
	FIO0DIR |= (1 << 2); //Enable
	FIO0DIR |= (1 << 3); //RS
	//Drive RS and E low
	FIO0PIN &= ~(1 << 2);
	FIO0PIN &= ~(1 << 3);
	//wait 4ms
	timer(40);
	//configure data signals as output
	LCDCommand(0x38); //8 bit config, with 1 lines of display
	LCDCommand(0x06); //moves cursor to right
	LCDCommand(0x0c); //enables the display only
	LCDCommand(0x01); //clears the display
	//wait 4ms
	timer(40);
}

int timeDifferenceCalc(int hr, int min, int sec){
	int totalSecTime = (hr * 3600) + (min*60) + sec;
	return totalSecTime;
}

int concatenate(int x, int y) {
    unsigned pow = 10;
    while(y >= pow)
        pow *= 10;
    return x * pow + y;
}

int * setTime(int timeArray[]){
	int* output = malloc(3*sizeof(int));
    output[0] = concatenate(timeArray[0], timeArray[1]);
    output[1] = concatenate(timeArray[2], timeArray[3]);
    output[2] = concatenate(timeArray[4], timeArray[5]);
    return output;
}

int * DifferenceTimeConvert(totalSecTime){
	int* output = malloc(3*sizeof(int));
	output[0] = totalSecTime / 3600;
	output[1] = (totalSecTime - (output[0] * 3600))/ 60;
	output[2] = (totalSecTime - (output[0] * 3600)- (output[1] * 60));
    return output;
}

int main(void) {
	init();
	RTC_Init();
	keypadInit();
	int flag = 0, count = 1;
	int hour, min, sec;
	HOUR = 7, MIN = 50, SEC = 55;
	int startHr = 0, startMin = 0, startSec = 0;
    int stopHr = 0, stopMin = 0, stopSec = 0;
    int diffHr =0, diffMin = 0, diffSec = 0;
    int firstDigitHr,  secondDigitHr, firstDigitMin, secondDigitMin, firstDigitSec, secondDigitSec = 0;
    while(1){
    	writeMC(MC23017_Write_Opcode, IODIRA, 0x00);
    	LCDinit(); //it clears the display
    	LCDCommand(0b00001110); //function set
    	LCDCommand(0b00000110); //entry set mode
    	int sw = switchState(MC23017_Write_Opcode,GPIOB, MC23017_Read_Opcode);
    		hour = HOUR;
    		firstDigitHr = FirstDigit(hour);
    		secondDigitHr = remainder(hour);
    		min = MIN;
    		firstDigitMin = FirstDigit(min);
    		secondDigitMin = remainder(min);
    		sec = SEC;
    		firstDigitSec = FirstDigit(sec);
    		secondDigitSec = remainder(sec);
    	if((sw >> 0) & 1){
    		startHr = hour;
    		startMin = min;
    		startSec = sec;
    	}
    	if((sw >> 1) & 1){
    		stopHr = hour;
    		stopMin = min;
    		stopSec = sec;
    		int totalStartSeconds = timeDifferenceCalc(startHr, startMin, startSec);
    		int totalStopSeconds = timeDifferenceCalc(stopHr, stopMin, stopSec);
    		int totalDiffSec = totalStopSeconds - totalStartSeconds;
    		int * diffArray = DifferenceTimeConvert(totalDiffSec);
    		//diff calculating
    		diffHr = diffArray[0];
    		diffMin = diffArray[1];
    		diffSec = diffArray[2];
    	}
    	//reset condition
    	if((sw >> 2) & 1){
    		diffHr = 0;
    		diffMin =0;
    		diffSec =0;
    	}
    	if((sw >> 3) & 1) {
    		flag = 1;
    	}
    	if(flag == 1 && counter < 6){
    		checkKeypadCases();
    	}
    	if(counter == 6){
    		int * newTime = setTime(array);
    		HOUR = newTime[0], MIN = newTime[1], SEC = newTime[2];
    		flag = 0, counter = 0;
    	}
    		hour = HOUR;
    		firstDigitHr = FirstDigit(hour);
    		secondDigitHr = remainder(hour);
    		min = MIN;
    		firstDigitMin = FirstDigit(min);
    		secondDigitMin = remainder(min);
    		sec = SEC;
    		firstDigitSec = FirstDigit(sec);
    		secondDigitSec = remainder(sec);
    		LCDWriteData(segment(firstDigitHr));
    		LCDWriteData(segment(secondDigitHr));
    		LCDWriteData(0b00111010); // display ':'
    		LCDWriteData(segment(firstDigitMin));
    		LCDWriteData(segment(secondDigitMin));
    		LCDWriteData(0b00111010); // display ':'
    		LCDWriteData(segment(firstDigitSec));
    		LCDWriteData(segment(secondDigitSec));
    		//displaying difference
    		LCDWriteData(0b00010000); //displaying aero
    		LCDWriteData(segment(FirstDigit(diffHr)));
    		LCDWriteData(segment(remainder(diffHr)));
    		LCDWriteData(0b00111010); // display ':'
    		LCDWriteData(segment(FirstDigit(diffMin)));
    		LCDWriteData(segment(remainder(diffMin)));
    		LCDWriteData(0b00111010); // display ':'
    		LCDWriteData(segment(FirstDigit(diffSec)));
    		LCDWriteData(segment(remainder(diffSec)));
    }
    return 0 ;
}
