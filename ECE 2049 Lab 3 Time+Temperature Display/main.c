/************** ECE2049 DEMO CODE ******************/
/**************  28 March 2020   ******************/
/*********Redesigned by Hamayel Qureshi********/
/***************************************************/

#include <msp430.h>
#include <stdlib.h>

/* Peripherals.c and .h are where the functions that implement
 * the LEDs and keypad, etc are. It is often useful to organize
 * your code by putting like functions together in files.
 * You include the header associated with that file(s)
 * into the main file of your project. */
#include "peripherals.h"

//Global Variable

// Temperature Sensor Calibration Reading at 30 deg C is stored
// at addr 1A1Ah. See end of datasheet for TLV table mapping
#define CALADC12_15V_30C  *((unsigned int *)0x1A1A)
// Temperature Sensor Calibration Reading at 85 deg C is stored
// at addr 1A1Ch See device datasheet for TLV table mapping
#define CALADC12_15V_85C  *((unsigned int *)0x1A1C)

volatile int editMode = 0; //1 = on, 0 = off

int currentCol = 0;

volatile long unsigned int timer_cnt = 5679960; //2678390; //THIS CONTROLS START TIME

volatile int temp_cnt = 0;

long unsigned int daysDenom = 86400;
long unsigned int rightButton, leftButton;

int convertedTime[5];
bool needUpdate = true;
char monthDay[7] = { 'M', 'M', 'M', ' ', 'D', 'D', '\0' };
char hourMinuteSecond[9] = { 'H', 'H', ':', 'M', 'M', ':', 'S', 'S', '\0' };

volatile float temperatureList[10];
volatile float degC;
volatile float degF;
char tempC[8] = { 'd', 'd', 'd', '.', 'f', ' ', 'C', '\0' };
char tempF[8] = { 'd', 'd', 'd', '.', 'f', ' ', 'F', '\0' };

// Function Prototypes
char intToChar(int digit);
void runtimerA2();

void displayTime(long unsigned int inTime);
void displayTemp(float inAvgTempC);

void updateDisplay(bool update);
void convertMonthDay();
void convertHourMinuteSecond();
void editTime();
void configButtons();
void setupADC();

// Main
void main(void)
{

    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer. Always need to stop this!!
                              // You can then configure it properly, if desired

    configDisplay();
    configButtons(); //Sets up launchpad buttons for use
    runtimerA2(); //Set up clock for period of 1 second
    setupADC(); //Sets up ADC to start taking temperature readings

    //__enable_interrupt();
    _BIS_SR(GIE);

    while (1)
    {
        rightButton = P1IN & BIT1;
        if ((rightButton == 0) && (editMode == 0))
        { //If right launchpad button is pressed...
            editMode = 1; //Switch to edit mode
            editTime();
        }
    }
}

//===========================================================================================

void runtimerA2()
{
    TA2CTL = TASSEL_1 | MC_1 | ID_0;
    TA2CCR0 = 0x7FFF; //32767
    TA2CCTL0 = CCIE; // enable Timer A2 interrupt
}

void displayTime(long unsigned int inTime)
{
    //convertedTime[0] = Months (1-12)
    //convertedTime[1] = Days (1-31)
    //convertedTime[2] = Hours (0-23)
    //convertedTime[3] = Minutes (0-59)
    //convertedTime[4] = Seconds (0-59)

    //NOTE: Code is clearly truncating and causing things to be zero, find way to NOT deal with decimals by multiplying numerator
    //Months+Days are fine, Hours+Minutes+Seconds is NOT

    long unsigned int days;

    days = (inTime / (daysDenom)); //Takes rounded off days value - truncates decimal bc int

    //daysRemainder = (inTime / daysDenom) - days; //Records days remainder for next calculation
    switch (days + 1)
    {
    case 1 ... 31: //31 days in January
        convertedTime[0] = 1;
        convertedTime[1] = days + 1;
        break;
    case 32 ... 59: //28 days in February
        convertedTime[0] = 2;
        convertedTime[1] = days + 1 - 31;
        break;
    case 60 ... 90: //31 days in March
        convertedTime[0] = 3;
        convertedTime[1] = days + 1 - 59;
        break;
    case 91 ... 120: //30 days in April
        convertedTime[0] = 4;
        convertedTime[1] = days + 1 - 90;
        break;
    case 121 ... 151: //31 days in May
        convertedTime[0] = 5;
        convertedTime[1] = days + 1 - 120;
        break;
    case 152 ... 181: //30 days in June
        convertedTime[0] = 6;
        convertedTime[1] = days + 1 - 151;
        break;
    case 182 ... 212: //31 days in July
        convertedTime[0] = 7;
        convertedTime[1] = days + 1 - 181;
        break;
    case 213 ... 243: //31 days in August
        convertedTime[0] = 8;
        convertedTime[1] = days + 1 - 212;
        break;
    case 244 ... 273: //30 days in September
        convertedTime[0] = 9;
        convertedTime[1] = days + 1 - 243;
        break;
    case 274 ... 304: //31 days in October
        convertedTime[0] = 10;
        convertedTime[1] = days + 1 - 273;
        break;
    case 305 ... 334: //30 days in November
        convertedTime[0] = 11;
        convertedTime[1] = days + 1 - 304;
        break;
    case 335 ... 365: //31 days in December
        convertedTime[0] = 12;
        convertedTime[1] = days + 1 - 334;
        break;
    }

    convertedTime[4] = inTime % 60; //Takes rounded off seconds value - truncates decimal bc int

    convertedTime[3] = (inTime % 3600) / 60; //Takes rounded off minutes value - truncates decimal bc int
    //minutesRemainder = (hoursRemainder * 60) - convertedTime[3]; //Records minutes remainder for next calculation

    convertedTime[2] = (inTime % daysDenom) / 3600; //Takes rounded off hours value - truncates decimal bc int
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

void updateDisplay(bool update)
{
    if (update)
    {
        /*
         if (editMode == 1)
         {
         Graphics_clearDisplay(&g_sContext); // Clear the display
         }
         */
        Graphics_drawStringCentered(&g_sContext, monthDay,
        AUTO_STRING_LENGTH,
                                    64, 45, true);
        Graphics_drawStringCentered(&g_sContext, hourMinuteSecond, //hourMinuteSecond
                                    AUTO_STRING_LENGTH, 64, 55, true);
        Graphics_drawStringCentered(&g_sContext, tempC,
        AUTO_STRING_LENGTH,
                                    64, 65, true);
        Graphics_drawStringCentered(&g_sContext, tempF,
        AUTO_STRING_LENGTH,
                                    64, 75, true);
        if (editMode == 1)
        {
            Graphics_drawStringCentered(&g_sContext, "EDIT MODE:",
            AUTO_STRING_LENGTH,
                                        64, 85, true);

            switch (currentCol)
            {
            case 0:
                Graphics_drawStringCentered(&g_sContext, " MONTHS ",
                AUTO_STRING_LENGTH,
                                            64, 95, true);
                break;
            case 1:
                Graphics_drawStringCentered(&g_sContext, "  DAYS  ",
                AUTO_STRING_LENGTH,
                                            64, 95, true);
                break;
            case 2:
                Graphics_drawStringCentered(&g_sContext, " HOURS ",
                AUTO_STRING_LENGTH,
                                            64, 95, true);
                break;
            case 3:
                Graphics_drawStringCentered(&g_sContext, "MINUTES",
                AUTO_STRING_LENGTH,
                                            64, 95, true);
                break;
            case 4:
                Graphics_drawStringCentered(&g_sContext, "SECONDS",
                AUTO_STRING_LENGTH,
                                            64, 95, true);
                break;
            }
        }

        Graphics_flushBuffer(&g_sContext);
        needUpdate = false;

    }
}

void convertMonthDay()
{
    switch (convertedTime[0])
    {
    case 1:
        monthDay[0] = 'J';
        monthDay[1] = 'A';
        monthDay[2] = 'N';
        break;
    case 2:
        monthDay[0] = 'F';
        monthDay[1] = 'E';
        monthDay[2] = 'B';
        break;
    case 3:
        monthDay[0] = 'M';
        monthDay[1] = 'A';
        monthDay[2] = 'R';
        break;
    case 4:
        monthDay[0] = 'A';
        monthDay[1] = 'P';
        monthDay[2] = 'R';
        break;
    case 5:
        monthDay[0] = 'M';
        monthDay[1] = 'A';
        monthDay[2] = 'Y';
        break;
    case 6:
        monthDay[0] = 'J';
        monthDay[1] = 'U';
        monthDay[2] = 'N';
        break;
    case 7:
        monthDay[0] = 'J';
        monthDay[1] = 'U';
        monthDay[2] = 'L';
        break;
    case 8:
        monthDay[0] = 'A';
        monthDay[1] = 'U';
        monthDay[2] = 'G';
        break;
    case 9:
        monthDay[0] = 'S';
        monthDay[1] = 'E';
        monthDay[2] = 'P';
        break;
    case 10:
        monthDay[0] = 'O';
        monthDay[1] = 'C';
        monthDay[2] = 'T';
        break;
    case 11:
        monthDay[0] = 'N';
        monthDay[1] = 'O';
        monthDay[2] = 'V';
        break;
    case 12:
        monthDay[0] = 'D';
        monthDay[1] = 'E';
        monthDay[2] = 'C';
        break;

    }

    monthDay[3] = ' ';

    monthDay[4] = intToChar((convertedTime[1] - (convertedTime[1] % 10)) / 10);
    monthDay[5] = intToChar(convertedTime[1] % 10);
    needUpdate = true;

}

void convertHourMinuteSecond()
{

    hourMinuteSecond[0] = intToChar(
            (convertedTime[2] - (convertedTime[2] % 10)) / 10);
    hourMinuteSecond[1] = intToChar(convertedTime[2] % 10);

    hourMinuteSecond[2] = ':';

    hourMinuteSecond[3] = intToChar(
            (convertedTime[3] - (convertedTime[3] % 10)) / 10);
    hourMinuteSecond[4] = intToChar(convertedTime[3] % 10);

    hourMinuteSecond[5] = ':';

    hourMinuteSecond[6] = intToChar(
            (convertedTime[4] - (convertedTime[4] % 10)) / 10);
    hourMinuteSecond[7] = intToChar(convertedTime[4] % 10);

}

void editTime()
{
    currentCol = 0; // 0 = Months
                    // 1 = Days
                    // 2 = Hours
                    // 3 = Minutes
                    // 4 = Seconds

    //rightButton = P1IN & BIT1;
    //(rightButton == 0)
    //leftButton = P2IN & BIT1;
    //(leftButton == 0)

    while (rightButton == 0)
    {
        rightButton = P1IN & BIT1;
    }

    while (currentCol == 0 && rightButton != 0)
    { //Until the right button is pressed to move on to next col...
        rightButton = P1IN & BIT1;
        leftButton = P2IN & BIT1;
        if ((leftButton == 0) && convertedTime[0] < 12)
        { //If left button is pressed and not at Month = 12...
            switch (convertedTime[0])
            {
            case 1:
                timer_cnt += (32 - convertedTime[1]) * 86400; //31 Days to date in next month // (31 - convertedTime[1]) * 86400
                break;
            case 2:
                timer_cnt += (29 - convertedTime[1]) * 86400; //28 Days
                break;
            case 3:
                timer_cnt += (32 - convertedTime[1]) * 86400; //31 Days
                break;
            case 4:
                timer_cnt += (31 - convertedTime[1]) * 86400; //30 Days
                break;
            case 5:
                timer_cnt += (32 - convertedTime[1]) * 86400; //31 Days
                break;
            case 6:
                timer_cnt += (31 - convertedTime[1]) * 86400; //30 Days
                break;
            case 7:
                timer_cnt += (32 - convertedTime[1]) * 86400; //31 Days
                break;
            case 8:
                timer_cnt += (32 - convertedTime[1]) * 86400; //31 Days
                break;
            case 9:
                timer_cnt += (31 - convertedTime[1]) * 86400; //30 Days
                break;
            case 10:
                timer_cnt += (32 - convertedTime[1]) * 86400; //31 Days
                break;
            case 11:
                timer_cnt += (31 - convertedTime[1]) * 86400; //30 Days
                break;
            }

            while (leftButton == 0)
            {
                leftButton = P2IN & BIT1;
            }

        }
        else if ((leftButton == 0) && convertedTime[0] == 12)
        { //If left button is pressed and at Month = 12...
            timer_cnt -= 28857599;
            while (leftButton == 0)
            {
                leftButton = P2IN & BIT1;
            }
        }
    }

    currentCol++;

    while (rightButton == 0)
    {
        rightButton = P1IN & BIT1;
    }

    while (currentCol == 1 && rightButton != 0)
    { //Until the right button is pressed to move on to next col...
        rightButton = P1IN & BIT1;
        leftButton = P2IN & BIT1;
        if ((leftButton == 0) && convertedTime[1] < 28)
        { //If left button is pressed and not at Day = ???...
            timer_cnt += 86400;
            while (leftButton == 0)
            {
                leftButton = P2IN & BIT1;
            }

        }
        else if ((leftButton == 0) && convertedTime[1] >= 28)
        { //If left button is pressed and at Day =???...
            switch (convertedTime[0])
            {
            case 1:
                if (convertedTime[1] == 31)
                {
                    timer_cnt -= 2591999; //31 Days
                }
                else
                {
                    timer_cnt += 86400;
                }

                break;
            case 2:
                if (convertedTime[1] == 28)
                {
                    timer_cnt -= 2332799; //28 Days
                }
                else
                {
                    timer_cnt += 86400;
                }
                timer_cnt -= 2419200; //28 Days
                break;
            case 3:
                if (convertedTime[1] == 31)
                {
                    timer_cnt -= 2591999; //31 Days
                }
                else
                {
                    timer_cnt += 86400;
                }
                break;
            case 4:
                if (convertedTime[1] == 30)
                {
                    timer_cnt -= 2505599; //30 Days
                }
                else
                {
                    timer_cnt += 86400;
                }
                break;
            case 5:
                if (convertedTime[1] == 31)
                {
                    timer_cnt -= 2591999; //31 Days
                }
                else
                {
                    timer_cnt += 86400;
                }
                break;
            case 6:
                if (convertedTime[1] == 30)
                {
                    timer_cnt -= 2505599; //30 Days
                }
                else
                {
                    timer_cnt += 86400;
                }
                break;
            case 7:
                if (convertedTime[1] == 31)
                {
                    timer_cnt -= 2591999; //31 Days
                }
                else
                {
                    timer_cnt += 86400;
                }
                break;
            case 8:
                if (convertedTime[1] == 31)
                {
                    timer_cnt -= 2591999; //31 Days
                }
                else
                {
                    timer_cnt += 86400;
                }
                break;
            case 9:
                if (convertedTime[1] == 30)
                {
                    timer_cnt -= 2505599; //30 Days
                }
                else
                {
                    timer_cnt += 86400;
                }
                break;
            case 10:
                if (convertedTime[1] == 31)
                {
                    timer_cnt -= 2591999; //31 Days
                }
                else
                {
                    timer_cnt += 86400;
                }
                break;
            case 11:
                if (convertedTime[1] == 30)
                {
                    timer_cnt -= 2505599; //30 Days
                }
                else
                {
                    timer_cnt += 86400;
                }
                break;
            }
            while (leftButton == 0)
            {
                leftButton = P2IN & BIT1;
            }

        }
    }            //this one is gonna be more complicated

    currentCol++;

    while (rightButton == 0)
    {
        rightButton = P1IN & BIT1;
    }

    while (currentCol == 2 && rightButton != 0)
    { //Until the right button is pressed to move on to next col...
        rightButton = P1IN & BIT1;
        leftButton = P2IN & BIT1;
        if ((leftButton == 0) && convertedTime[2] < 23)
        { //If left button is pressed and not at Hour = 23...
            timer_cnt += 3600;
            while (leftButton == 0)
            {
                leftButton = P2IN & BIT1;
            }

        }
        else if ((leftButton == 0) && convertedTime[2] == 23)
        { //If left button is pressed and at Hour = 23...
            timer_cnt -= 82800;
            while (leftButton == 0)
            {
                leftButton = P2IN & BIT1;
            }

        }
    }

    currentCol++;

    while (rightButton == 0)
    {
        rightButton = P1IN & BIT1;
    }

    while (currentCol == 3 && rightButton != 0)
    { //Until the right button is pressed to move on to next col...
        rightButton = P1IN & BIT1;
        leftButton = P2IN & BIT1;
        if ((leftButton == 0) && convertedTime[3] < 59)
        { //If left button is pressed and not at Minute = 59...
            timer_cnt += 60;
            while (leftButton == 0)
            {
                leftButton = P2IN & BIT1;
            }

        }
        else if ((leftButton == 0) && convertedTime[3] == 59)
        { //If left button is pressed and at Minute = 59...
            timer_cnt -= 3540;
            while (leftButton == 0)
            {
                leftButton = P2IN & BIT1;
            }

        }
    }

    currentCol++;

    while (rightButton == 0)
    {
        rightButton = P1IN & BIT1;
    }

    while (currentCol == 4 && rightButton != 0)
    { //Until the right button is pressed to move on to next col...
        rightButton = P1IN & BIT1;
        leftButton = P2IN & BIT1;
        if ((leftButton == 0) && convertedTime[4] < 59)
        { //If left button is pressed and not at Second = 59...
            timer_cnt++;
            while (leftButton == 0)
            {
                leftButton = P2IN & BIT1;
            }

        }
        else if ((leftButton == 0) && convertedTime[4] == 59)
        { //If left button is pressed and at Second = 59...
            timer_cnt -= 59;
            while (leftButton == 0)
            {
                leftButton = P2IN & BIT1;
            }

        }
    }

    while (rightButton == 0)
    {
        rightButton = P1IN & BIT1;
    }

    Graphics_clearDisplay(&g_sContext); // Clear the display
    editMode = 0;

}

void configButtons()
{
//NOT CURRENTLY SET UP FOR P1.1 and P2.1!!!!!!!!!
    P1SEL &= ~(BIT1); //Set P1.1 (Left Button) for digital I/O
    P2SEL &= ~(BIT1); //Set P2.1 (Right Button) for digital I/O
    //P7SEL = ~(BIT0 | BIT4);

    P1DIR &= ~(BIT1); //Configure as input
    P2DIR &= ~(BIT1); //Configure as input
    //P7DIR &= ~(BIT0 | BIT4);

    P1REN |= (BIT1); //Pullup or pulldown resistor enabled
    P2REN |= (BIT1); //Pullup or pulldown resistor enabled
    //P7REN |= (BIT0 | BIT4);

    P1OUT |= (BIT1); //Pulldown selected
    P2OUT |= (BIT1); //Pulldown selected
    //P7OUT |= (BIT0 | BIT4);
}

void setupADC()
{

    // Reset REFMSTR to hand over control of internal reference
    // voltages to ADC12_A control registers
    REFCTL0 &= ~REFMSTR;
    // Initialize control register ADC12CTL0 = 0000 1001 0111 0000
    // SHT0x=9h (384 clk cycles), MCS=1=burst thru selected chans.,
    // REF2_5V = 1 (2.5V), REFON = 1 = use internal reference volts
    // and ADC12ON = 1 = turn ADC on
    ADC12CTL0 = ADC12SHT0_9 | ADC12REFON | ADC12ON; //Took ADC12REF2_5V out bc we want 1.5v reference

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

    // Set conversion memory control registers for the 2 channels
    // ADC12MCTL0:EOS = 0, SREF =001 = voltage refs = GND to Vref+
    // INCHx = 0000
    ADC12MCTL0 = ADC12SREF_1 + ADC12INCH_10;
    __delay_cycles(100);
    //ADC12MCTL1:EOS = 1, SREF =001 = voltage refs = GND to Vref+
    // INCHx = 1010ADC12MCTL1 = ADC12SREF_1 + ADC12INCH_10 + ADC12EOS;
    //Set Port 6 Pins 0 to FUNCTION mode (=1) for ADC12
    P6SEL = P6SEL | BIT0;
    ADC12CTL0 &= ~ADC12SC; //Clear the start bit

    //ADC12IE = BIT1; //Interrupt??? - I ADDED - Do we need?

    //__enable_interrupt(); //Enables ADC interrupts

    ADC12CTL0 |= ADC12ENC + ADC12SC; //Turn on ADC

}

void displayTemp(float inAvgTempC)
{
    //char tempC[7];
    //char tempF[7];

    int movedTempC = inAvgTempC * 10;

    tempC[4] = intToChar(movedTempC % 10);
    tempC[2] = intToChar(((movedTempC % 100) - (movedTempC % 10)) / 10);
    tempC[1] = intToChar(((movedTempC % 1000) - (movedTempC % 100)) / 100);
    tempC[0] = intToChar(((movedTempC % 10000) - (movedTempC % 1000)) / 1000);

    degF = (inAvgTempC * 1.8) + 32;
    int movedTempF = degF * 10;

    tempF[4] = intToChar(movedTempF % 10);
    tempF[2] = intToChar(((movedTempF % 100) - (movedTempF % 10)) / 10);
    tempF[1] = intToChar(((movedTempF % 1000) - (movedTempF % 100)) / 100);
    tempF[0] = intToChar(((movedTempF % 10000) - (movedTempF % 1000)) / 1000);
}

// Timer A2 interrupt service routine
//CHANGE TO OUR LIKING
#pragma vector=TIMER2_A0_VECTOR
__interrupt void TimerA2_ISR(void)
{

    ADC12CTL0 &= ~ADC12SC;    // clear the start bit
    ADC12CTL0 |= ADC12SC;

    //ADC12CTL0 |= ADC12SC;

    while (ADC12CTL1 & ADC12BUSY)
        __no_operation();

    volatile float in_temp = ADC12MEM0 & 0x0FFF; // keep only low 12 bits
    temperatureList[temp_cnt % 10] = (float)((((long) in_temp - CALADC12_15V_30C) * (85 - 30))
           / (CALADC12_15V_85C - CALADC12_15V_30C) + 30.0);

    //float Vin = (float) ((in_temp / 4095) * 1.5);
    //temperatureList[temp_cnt % 10] = (float) ((Vin - .688) / (.15 / 70)); //TOGGLE THIS TO DEMONSTRATE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    int numCount = 0;
    float sumTemp = 0.0;
    int i;

    for (i = 0; i < 10; i++)
    {
        if (temp_cnt - i >= 0)
        {
            sumTemp += temperatureList[(temp_cnt - i) % 10];
            numCount += 1;
        }
        else
        {
            break;
        }
    }

    degC = (float) (sumTemp / numCount);

    if (editMode == 0)
    {
        timer_cnt++;
        temp_cnt++;
    }

    volatile long unsigned int timeCopy = timer_cnt;
    displayTime(timeCopy);
    convertMonthDay();
    convertHourMinuteSecond();
    displayTemp(degC);
    updateDisplay(needUpdate);

}
/*
 // ADC12 interrupt service routine
 #pragma vector=ADC12_VECTOR
 __interrupt void ADC12_ISR(void)
 {

 }
 */
//NOTES FOR FIXES
// Edit void setupADC();
//Work on array and averaging it
