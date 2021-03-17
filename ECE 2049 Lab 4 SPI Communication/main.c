#include <msp430.h>

#include <stdlib.h>
#include "peripherals.h"

//NOTES:
//Used MSP430 board for lab
//Connect a pot to P6.4, with Vcc = 3.3v

//This is to use for slave configurations
#define SLAVE_PORT_SPI_SEL P4SEL
#define SLAVE_PORT_SPI_DIR P4DIR
#define SLAVE_PORT_SPI_OUT P4OUT

#define SLAVE_PORT_CS_SEL P4SEL
#define SLAVE_PORT_CS_DIR P4DIR
#define SLAVE_PORT_CS_OUT P4OUT
#define SLAVE_PORT_CS_REN P4REN

#define SLAVE_PIN_SPI_MOSI BIT1
#define SLAVE_PIN_SPI_MISO BIT2
#define SLAVE_PIN_SPI_SCLK BIT3
#define SLAVE_PIN_SPI_CS BIT0

#define SLAVE_SPI_REG_CTL0 UCB1CTL0
#define SLAVE_SPI_REG_CTL1 UCB1CTL1
#define SLAVE_SPI_REG_BRL UCB1BR0
#define SLAVE_SPI_REG_BRH UCB1BR1
#define SLAVE_SPI_REG_IFG UCB1IFG
#define SLAVE_SPI_REG_STAT UCB1STAT
#define SLAVE_SPI_REG_TXBUF UCB1TXBUF
#define SLAVE_SPI_REG_RXBUF UCB1RXBUF

//This is needed to configure P8.2 to use it as CS by MSP430
#define MSP_PORT_CS_SEL P8SEL
#define MSP_PORT_CS_DIR P8DIR
#define MSP_PORT_CS_OUT P8OUT
#define MSP_PIN_CS BIT2

#define MSP_SPI_REG_TXBUF UCB0TXBUF
#define MSP_SPI_REG_RXBUF UCB0RXBUF
#define MSP_SPI_REG_IFG UCB0IFG

void runtimerA2();
void updateDisplayTime();
float readVoltage();
void initSlaveSPI();
unsigned char slaveSPIRead();
void masterSPIWrite(unsigned int data);
void convertTime(long unsigned int inTime);
char intToChar(int digit);
void convertHourMinuteSecond();
void configADC12();
void convertVoltage(float voltageIn);
void updateDisplayVoltage();

volatile long unsigned int timer_cnt = 0;
long unsigned int daysDenom = 86400;
int convertedTime[3];
float convertedVoltage;

unsigned char w = 0x55;
unsigned char r = 0x11;

long unsigned int testRead;
long unsigned int testWrite = 86451;

long unsigned int masterData = 0x00000000;

char hourMinuteSecond[15] = { 'T', 'i', 'm', 'e', ':', ' ', 'H', 'H', ':', 'M',
                              'M', ':', 'S', 'S', '\0' }; //14 chars+null
char voltageDisplay[19] = { 'V', 'o', 'l', 't', 'a', 'g', 'e', ':', ' ', 'V',
                            '.', 'V', ' ', 'V', 'o', 'l', 't', 's', '\0' }; //18 chars+null
//char monthDay[7] = { 'M', 'M', 'M', ' ', 'D', 'D', '\0' };

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    configDisplay(); //Sets up UCB0 as Master
    runtimerA2(); //Set up clock for period of 1 second
    configADC12();
    initSlaveSPI();

    //__enable_interrupt();
    _BIS_SR(GIE);

    //0xFFFFFFFF
    //Maybe put the 4 values into a global list?
    //Divide the time into 4 bytes
    //How many bits is long unsigned int? - 32 / 4 bytes
    //data is sent MSB first
    long unsigned int timerCopy;
    unsigned int timerCopy1, timerCopy2, timerCopy3, timerCopy4;
    unsigned int voltageCopy1, voltageCopy2, voltageCopy3, voltageCopy4;
    long unsigned int voltageCopy;

    while (1)
    {
        masterData = 0x00000000;

        timerCopy = timer_cnt;
        timerCopy1 = (timerCopy & 0xFF000000) >> 24;
        timerCopy2 = (timerCopy & 0x00FF0000) >> 16;
        timerCopy3 = (timerCopy & 0x0000FF00) >> 8;
        timerCopy4 = (timerCopy & 0x000000FF);

        masterSPIWrite(timerCopy1);
        masterData |= ((long unsigned int) (slaveSPIRead()) << 24);

        masterSPIWrite(timerCopy2);
        masterData |= ((long unsigned int) (slaveSPIRead()) << 16);

        masterSPIWrite(timerCopy3);
        masterData |= ((long unsigned int) (slaveSPIRead()) << 8);

        masterSPIWrite(timerCopy4);
        masterData |= (long unsigned int) (slaveSPIRead());

        long unsigned int masterDataCopy = masterData;
        convertTime(masterDataCopy);

        convertedVoltage = readVoltage();

        masterData = 0x00000000;

        voltageCopy = convertedVoltage * 10;
        voltageCopy1 = (voltageCopy & 0xFF000000) >> 24;
        voltageCopy2 = (voltageCopy & 0x00FF0000) >> 16;
        voltageCopy3 = (voltageCopy & 0x0000FF00) >> 8;
        voltageCopy4 = (voltageCopy & 0x000000FF);

        masterSPIWrite (voltageCopy1);
        masterData |= ((long unsigned int) (slaveSPIRead()) << 24);

        masterSPIWrite (voltageCopy2);
        masterData |= ((long unsigned int) (slaveSPIRead()) << 16);

        masterSPIWrite (voltageCopy3);
        masterData |= ((long unsigned int) (slaveSPIRead()) << 8);

        masterSPIWrite (voltageCopy4);
        masterData |= (long unsigned int) (slaveSPIRead());

        float convertedVoltageCopy = (float)((float)masterData / (float)10.0);
        convertVoltage(convertedVoltageCopy);

    }
}

void runtimerA2()
{
    TA2CTL = TASSEL_1 | MC_1 | ID_0;
    TA2CCR0 = 0x7FFF; //32767
    TA2CCTL0 = CCIE; // enable Timer A2 interrupt
}

void updateDisplayTime()
{
    Graphics_drawStringCentered(&g_sContext, hourMinuteSecond, //hourMinuteSecond
                                AUTO_STRING_LENGTH, 64, 55, true);

    Graphics_flushBuffer(&g_sContext);
}

void initSlaveSPI()
{
    // Configure SCLK, MISO and MOSI for peripheral mode
    SLAVE_PORT_SPI_SEL |= (SLAVE_PIN_SPI_MOSI | SLAVE_PIN_SPI_MISO
            | SLAVE_PIN_SPI_SCLK);

    // Configure the Slave CS as an Input P4.0
    SLAVE_PORT_CS_SEL &= ~SLAVE_PIN_SPI_CS;
    SLAVE_PORT_CS_DIR &= ~SLAVE_PIN_SPI_CS;
    SLAVE_PORT_CS_REN |= SLAVE_PIN_SPI_CS;
    SLAVE_PORT_CS_OUT |= SLAVE_PIN_SPI_CS;

    // Configure the CS output of MSP430 P8.2. It will set P4.0 high or low.
    MSP_PORT_CS_SEL &= ~MSP_PIN_CS;
    MSP_PORT_CS_DIR |= MSP_PIN_CS;
    MSP_PORT_CS_OUT |= MSP_PIN_CS;

    // Disable the module so we can configure it
    SLAVE_SPI_REG_CTL1 |= UCSWRST;

    SLAVE_SPI_REG_CTL0 &= ~(0xFF); // Reset the controller config parameters
    SLAVE_SPI_REG_CTL1 &= ~UCSSEL_3; // Reset the clock configuration

    // Set SPI clock frequency (which is the same frequency as SMCLK so this can apparently be 0)
    SPI_REG_BRL = ((uint16_t) SPI_CLK_TICKS) & 0xFF; // Load the low byte
    SPI_REG_BRH = (((uint16_t) SPI_CLK_TICKS) >> 8) & 0xFF; // Load the high byte

    //capture data on first edge - UCCKPH (btw this is on on page 986 of user guide)
    //inactive low polarity - dont add
    //MSB first - UCMSB
    //8 bit - dont add
    //Slave Mode - dont add
    //4 wire - SPI active low - UCMODE_2
    //Synchronous mode - UCSYNC
    SLAVE_SPI_REG_CTL0 |= UCCKPH + UCMSB + UCMODE_2 + UCSYNC; //NEED TO FILL IN OURSELVES

    // Reenable the module
    SLAVE_SPI_REG_CTL1 &= ~UCSWRST;
    SLAVE_SPI_REG_IFG &= ~UCRXIFG;
}

unsigned char slaveSPIRead()
{
    unsigned char c;
    while (!(SLAVE_SPI_REG_IFG & UCRXIFG))
    {
        c = SLAVE_SPI_REG_RXBUF;
    }

    return (c & 0xFF);
}

void masterSPIWrite(unsigned int data)
{
    // Start SPI transmission by de-asserting CS
    MSP_PORT_CS_OUT &= ~(MSP_PIN_CS);    //NEED TO FILL IN OURSELVES

    // Write data/ 1-byte at a time
    uint8_t byte = (unsigned char) ((data) & 0xFF);

    // Send byte
    MSP_SPI_REG_TXBUF = byte;

    // Wait for SPI peripheral to finish transmitting
    while (!(MSP_SPI_REG_IFG & UCTXIFG))
    {
        _no_operation();
    }

    // Assert CS
    MSP_PORT_CS_OUT |= MSP_PIN_CS;    //NEED TO FILL IN OURSELVES
}

void convertTime(long unsigned int inTime)
{

    convertedTime[2] = inTime % 60; //Takes rounded off seconds value - truncates decimal bc int

    convertedTime[1] = (inTime % 3600) / 60; //Takes rounded off minutes value - truncates decimal bc int
    //minutesRemainder = (hoursRemainder * 60) - convertedTime[3]; //Records minutes remainder for next calculation

    convertedTime[0] = (inTime % daysDenom) / 3600; //Takes rounded off hours value - truncates decimal bc int
    //hoursRemainder = (daysRemainder * 24) - convertedTime[2]; //Records hours remainder for next calculation

}

char intToChar(int digit)
{
    switch (digit)
    {
    case 0:
        return '0';
    case 1:
        return '1';
    case 2:
        return '2';
    case 3:
        return '3';
    case 4:
        return '4';
    case 5:
        return '5';
    case 6:
        return '6';
    case 7:
        return '7';
    case 8:
        return '8';
    case 9:
        return '9';
    }
    return '?';
}

void convertHourMinuteSecond()
{

    hourMinuteSecond[6] = intToChar(
            (convertedTime[0] - (convertedTime[0] % 10)) / 10);
    hourMinuteSecond[7] = intToChar(convertedTime[0] % 10);

    //hourMinuteSecond[2] = ':';

    hourMinuteSecond[9] = intToChar(
            (convertedTime[1] - (convertedTime[1] % 10)) / 10);
    hourMinuteSecond[10] = intToChar(convertedTime[1] % 10);

    //hourMinuteSecond[5] = ':';

    hourMinuteSecond[12] = intToChar(
            (convertedTime[2] - (convertedTime[2] % 10)) / 10);
    hourMinuteSecond[13] = intToChar(convertedTime[2] % 10);

}

void configADC12()
{

    // Reset REFMSTR to hand over control of internal reference
    // voltages to ADC12_A control registers
    REFCTL0 &= ~REFMSTR;
    // Initialize control register ADC12CTL0 = 0000 1001 0111 0000
    // SHT0x=9h (384 clk cycles), MCS=1=burst thru selected chans.,
    // REF2_5V = 1 (2.5V), REFON = 1 = use internal reference volts
    // and ADC12ON = 1 = turn ADC on
    ADC12CTL0 = ADC12SHT0_9 | ADC12ON; //Took ADC12REF2_5V out bc we want 1.5v reference | ADC12REF2_5V ADC12REFON |

    // Initialize control register ADC12CTL1 = 0000 0010 0000 0010
    // ADC12CSTART ADDx = 0000 =  start conversion with ADC12MEM0,
    // ADC12SHSx = 00 = use SW conversion trigger, ADC12SC bits
    // ADC12SHP = 1 = SAMPCON signal sourced from sampling timer,
    // ADC12ISSH = 0 = sample input signal not inverted,
    // ADC12DIVx = 000= divide ADC12CLK by 1,
    // ADC12SSEL=00= ADC clock ADC12OSC (~5 MHz),
    // ADC12CONSEQx = 01 = sequence of channels converted once
    // ADC12BUSY = 0 = no ADC operation active
    ADC12CTL1 = ADC12SHP; //QUESTION: Do we want ADC12SHP??? //Sample and hold is on by default

    //P6SEL |= BIT0; //Turn on A0 input on P6.0 - For ADC Vin - INCORRECT CODES, NO DISPLAY
    //P6SEL |= BIT6; //Turn on A0 input on P6.6 - For ADC Vin - GAVE CORRECT CODES, NO DISPLAY
    P6SEL |= BIT4; //Turn on A0 input on P6.6 - For ADC Vin - Correct Codes, Correct Display!

    // Set conversion memory control registers for the 2 channels
    // ADC12MCTL0:EOS = 0, SREF =001 = voltage refs = GND to Vref+
    // INCHx = 0000
    ADC12MCTL0 = ADC12SREF_0 | ADC12INCH_4;
    __delay_cycles(100);
    //ADC12MCTL1:EOS = 1, SREF =001 = voltage refs = GND to Vref+
    // INCHx = 1010ADC12MCTL1 = ADC12SREF_1 + ADC12INCH_10 + ADC12EOS;
    //Set Port 6 Pins 0 to FUNCTION mode (=1) for ADC12

    ADC12CTL0 &= ~ADC12SC; //Clear the start bit

    //ADC12IE = BIT1; //Interrupt??? - I ADDED - Do we need?

    //__enable_interrupt(); //Enables ADC interrupts

    ADC12CTL0 |= ADC12ENC; //Turn on ADC
}

float readVoltage()
{
    ADC12CTL0 &= ~ADC12SC;    // clear the start bit
    ADC12CTL0 |= ADC12SC;

    //ADC12CTL0 |= ADC12SC;

    while (ADC12CTL1 & ADC12BUSY)
        __no_operation();

    volatile float in_voltage;
    in_voltage = ADC12MEM0 & 0x0FFF; // keep only low 12 bits

    return (float) ((in_voltage / 4095) * 3.3);
}

void convertVoltage(float voltageIn)
{

    int movedVoltageIn = voltageIn * 10;

    voltageDisplay[9] = intToChar(
            (movedVoltageIn - (movedVoltageIn % 10)) / 10);
    voltageDisplay[11] = intToChar(movedVoltageIn % 10);

}

void updateDisplayVoltage()
{
    Graphics_drawStringCentered(&g_sContext, voltageDisplay,
    AUTO_STRING_LENGTH,
                                64, 65, true);
    Graphics_flushBuffer(&g_sContext);
}

#pragma vector=TIMER2_A0_VECTOR
__interrupt void TimerA2_ISR(void)
{
    if (timer_cnt < daysDenom)
    { //amount of seconds in a day, max of display
        timer_cnt++;
    }
    else
    {
        timer_cnt = 0;
    }
    convertHourMinuteSecond();
    updateDisplayTime();
    updateDisplayVoltage();
}
