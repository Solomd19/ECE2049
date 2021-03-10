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
char interruptKey;
int state = 0;  //0 = Start Screen
//1 = Song List
//2 = Start Countdown
//3 = Music Player - Song 1
//4 = Music Player - Song 2
int songStatus = 1; //0 = paused
//1 = playing
int songSpeed; //1-5, 3 is default
long unsigned int timer_cnt = 0;
char lastKey;

// Function Prototypes
void swDelay(char numLoops);
void configUserLED(char inbits);
void note(float pitch);
void timerDelay(long unsigned int initialCount, int delay);
void speedChange(int speed);

// Main
void main(void)

{

    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer. Always need to stop this!!
                              // You can then configure it properly, if desired

    initLeds();
    configDisplay();
    configKeypad();

    //__enable_interrupt();
    _BIS_SR(GIE);

    unsigned char currKey = 0, dispSz = 3;
    unsigned char dispThree[3];
    int i, j, k, l, f;
    int currIndex;
    float period;
    float freq;
    float notePeriod;
    float clkPeriod;

    //char currKey;
    int level;
    int song = 0;

    //Create timer
    TA2CTL = TASSEL_1 + ID_0 + MC_1;
    TA2CCR0 = 163; // 163+1 = 164 ACLK tics = ~1/200 seconds
    TA2CCTL0 = CCIE;     // TA2CCR0 interrupt enabled

    while (1)
    {
        switch (state)
        {
        case 0: //0 = Start Screen
            Graphics_clearDisplay(&g_sContext); // Clear the display
            Graphics_drawStringCentered(&g_sContext, "MUSIC PLAYER",
            AUTO_STRING_LENGTH,
                                        64, 55,
                                        TRANSPARENT_TEXT);
            Graphics_drawStringCentered(&g_sContext, "Press '*' to Begin",
            AUTO_STRING_LENGTH,
                                        64, 65,
                                        TRANSPARENT_TEXT);
            Graphics_flushBuffer(&g_sContext);
            while (1)
            {
                if (getKey() == '*')
                {
                    state = 1;
                    break;
                }
            }
            break;

        case 1: //1 = Song List
            Graphics_clearDisplay(&g_sContext); // Clear the display
            Graphics_drawStringCentered(&g_sContext, "1-The Final Countdown",
            AUTO_STRING_LENGTH,
                                        64, 55,
                                        TRANSPARENT_TEXT);
            Graphics_drawStringCentered(&g_sContext, "2-Coffin Dance",
            AUTO_STRING_LENGTH,
                                        64, 65,
                                        TRANSPARENT_TEXT);
            Graphics_flushBuffer(&g_sContext);
            while (1)
            {
                if (getKey() == '1')
                {
                    song = 1;
                    state = 2;
                    break;
                }
                if (getKey() == '2')
                {
                    song = 2;
                    state = 2;
                    break;
                }
            }
            break;

        case 2: //2 = Start Countdown
            speedChange(3);
            Graphics_clearDisplay(&g_sContext); // Clear the display
            Graphics_drawStringCentered(&g_sContext, "3", AUTO_STRING_LENGTH,
                                        64, 60, TRANSPARENT_TEXT);
            Graphics_flushBuffer(&g_sContext);
            configUserLED(1);
            timerDelay(timer_cnt, 200);
            Graphics_clearDisplay(&g_sContext); // Clear the display
            Graphics_drawStringCentered(&g_sContext, "2", AUTO_STRING_LENGTH,
                                        64, 60, TRANSPARENT_TEXT);
            Graphics_flushBuffer(&g_sContext);
            configUserLED(2);
            timerDelay(timer_cnt, 200);
            Graphics_clearDisplay(&g_sContext); // Clear the display
            Graphics_drawStringCentered(&g_sContext, "1", AUTO_STRING_LENGTH,
                                        64, 60, TRANSPARENT_TEXT);
            Graphics_flushBuffer(&g_sContext);
            configUserLED(1);
            timerDelay(timer_cnt, 200);
            Graphics_clearDisplay(&g_sContext); // Clear the display
            Graphics_drawStringCentered(&g_sContext, "GO", AUTO_STRING_LENGTH,
                                        64, 60, TRANSPARENT_TEXT);
            Graphics_flushBuffer(&g_sContext);
            configUserLED(3);
            timerDelay(timer_cnt, 200);
            configUserLED(0);
            if (song == 1)
            {
                state = 3;
            }
            else if (song == 2)
            {
                state = 4;
            }
            else
            {
                state = 0;
            }
            break;

        case 3: //3 = Music Player - Song 1
            Graphics_clearDisplay(&g_sContext); // Clear the display
            Graphics_drawStringCentered(&g_sContext, "Playing:",
            AUTO_STRING_LENGTH,
                                        64, 55,
                                        TRANSPARENT_TEXT);
            Graphics_drawStringCentered(&g_sContext, "1-The Final Countdown",
            AUTO_STRING_LENGTH,
                                        64, 65,
                                        TRANSPARENT_TEXT);
            Graphics_drawStringCentered(&g_sContext, "1- pause/play",
            AUTO_STRING_LENGTH,
                                        64, 75,
                                        TRANSPARENT_TEXT);
            Graphics_drawStringCentered(&g_sContext, "2- slower",
            AUTO_STRING_LENGTH,
                                        64, 85,
                                        TRANSPARENT_TEXT);
            Graphics_drawStringCentered(&g_sContext, "3- faster",
            AUTO_STRING_LENGTH,
                                        64, 95,
                                        TRANSPARENT_TEXT);
            Graphics_flushBuffer(&g_sContext);
            Graphics_drawStringCentered(&g_sContext, "4- return",
            AUTO_STRING_LENGTH,
                                        64, 105,
                                        TRANSPARENT_TEXT);
            Graphics_flushBuffer(&g_sContext);

            //Update songStatus to "playing"
            songStatus = 1;
            configUserLED(2); //Light green LED

            //Update songSpeed to default of 3
            songSpeed = 3;

            //Set timer to 0 before starting song
            timer_cnt = 0;

            //One quarter note = 100 cycles of delay

            //timerDelay(timer_cnt, 200); //Half note
            //timerDelay(timer_cnt, 100); //Quarter note
            //timerDelay(timer_cnt, 50); //Eighth note
            //timerDelay(timer_cnt, 25); //Sixteenth note

            /*
             BuzzerOff();
             while(timer_cnt <= 25){
             note(554);//C#5
             }
             BuzzerOff();
             */
            /*
             while (timer_cnt <= 25)
             {
             note(554);//C#5
             }
             */
            for (i = 0; i < 2; i++)
            {
                BuzzerOff();
                note(554); //C#5
                timerDelay(timer_cnt, 25);

                BuzzerOff();
                note(493); //B4
                timerDelay(timer_cnt, 25);

                BuzzerOff();
                note(554); //C#5
                timerDelay(timer_cnt, 100); //Quarter note

                BuzzerOff();
                note(369); //F#4
                timerDelay(timer_cnt, 250);

                BuzzerOff();
                note(587); //D5
                timerDelay(timer_cnt, 25);

                BuzzerOff();
                note(554); //C#5
                timerDelay(timer_cnt, 25);

                BuzzerOff();
                note(587); //D5
                timerDelay(timer_cnt, 50);

                BuzzerOff();
                note(554); //C#5
                timerDelay(timer_cnt, 50);

                BuzzerOff();
                note(493); //B4
                timerDelay(timer_cnt, 250);

                BuzzerOff();
                note(587); //D5
                timerDelay(timer_cnt, 25);

                BuzzerOff();
                note(554); //C#5
                timerDelay(timer_cnt, 25);

                BuzzerOff();
                note(587); //D5
                timerDelay(timer_cnt, 100);

                BuzzerOff();
                note(369); //F#4
                timerDelay(timer_cnt, 250);

                BuzzerOff();
                note(493); //B4
                timerDelay(timer_cnt, 25);

                BuzzerOff();
                note(440); //A4
                timerDelay(timer_cnt, 25);

                BuzzerOff();
                note(493); //B4
                timerDelay(timer_cnt, 50);

                BuzzerOff();
                note(440); //A4
                timerDelay(timer_cnt, 50);

                BuzzerOff();
                note(415); //G#4
                timerDelay(timer_cnt, 50);

                BuzzerOff();
                note(493); //B4
                timerDelay(timer_cnt, 50);

                BuzzerOff();
                note(440); //A4
                timerDelay(timer_cnt, 150);

            }

            BuzzerOff();
            state = 0;
            configUserLED(0);
            speedChange(3);
            break;

        case 4: //4 = Music Player - Song 2
            Graphics_clearDisplay(&g_sContext); // Clear the display
            Graphics_drawStringCentered(&g_sContext, "Playing:",
            AUTO_STRING_LENGTH,
                                        64, 55,
                                        TRANSPARENT_TEXT);
            Graphics_drawStringCentered(&g_sContext, "2-Coffin Dance",
            AUTO_STRING_LENGTH,
                                        64, 65,
                                        TRANSPARENT_TEXT);
            Graphics_drawStringCentered(&g_sContext, "1- pause/play",
            AUTO_STRING_LENGTH,
                                        64, 75,
                                        TRANSPARENT_TEXT);
            Graphics_drawStringCentered(&g_sContext, "2- slower",
            AUTO_STRING_LENGTH,
                                        64, 85,
                                        TRANSPARENT_TEXT);
            Graphics_drawStringCentered(&g_sContext, "3- faster",
            AUTO_STRING_LENGTH,
                                        64, 95,
                                        TRANSPARENT_TEXT);

            Graphics_drawStringCentered(&g_sContext, "4- return",
            AUTO_STRING_LENGTH,
                                        64, 105,
                                        TRANSPARENT_TEXT);
            Graphics_flushBuffer(&g_sContext);

            //Update songStatus to "playing"
            songStatus = 1;
            configUserLED(2); //Light green LED

            //Update songSpeed to default of 3
            songSpeed = 3;

            //Set timer to 0 before starting song
            timer_cnt = 0;

            //One quarter note = 100 cycles of delay

            //timerDelay(timer_cnt, 200); //Half note
            //timerDelay(timer_cnt, 100); //Quarter note
            //timerDelay(timer_cnt, 50); //Eighth note
            //timerDelay(timer_cnt, 25); //Sixteenth note
            /*
             BuzzerOff();
             note(554); //C#5
             timerDelay(timer_cnt, 25);
             */
            for (i = 0; i < 2; i++)
            {
                //B4
                BuzzerOff();
                note(493); //B4
                timerDelay(timer_cnt, 77);

                //A4
                BuzzerOff();
                note(440); //A4
                timerDelay(timer_cnt, 77);

                //G#4
                BuzzerOff();
                note(415); //G#4
                timerDelay(timer_cnt, 77);

                //E4
                BuzzerOff();
                note(329); //E4
                timerDelay(timer_cnt, 77);

                //F#4
                BuzzerOff();
                note(369); //F#4
                timerDelay(timer_cnt, 150);

                //C#5
                BuzzerOff();
                note(554); //C#5
                timerDelay(timer_cnt, 77);

                //B4
                BuzzerOff();
                note(493); //B4
                timerDelay(timer_cnt, 120);

                //A4
                BuzzerOff();
                note(440); //A4
                timerDelay(timer_cnt, 120);

                //G#4
                BuzzerOff();
                note(415); //G#4
                timerDelay(timer_cnt, 180);

                //B4
                BuzzerOff();
                note(493); //B4
                timerDelay(timer_cnt, 150);

                //A4
                BuzzerOff();
                note(440); //A4
                timerDelay(timer_cnt, 77);

                //G#4
                BuzzerOff();
                note(415); //G#4
                timerDelay(timer_cnt, 77);

                //F#4
                BuzzerOff();
                note(369); //F#4
                timerDelay(timer_cnt, 150);

                //A5
                BuzzerOff();
                note(880); //A5
                timerDelay(timer_cnt, 77);

                //G#5
                BuzzerOff();
                note(830); //G#5
                timerDelay(timer_cnt, 77);

                //A5
                BuzzerOff();
                note(880); //A5
                timerDelay(timer_cnt, 77);

                //G#5
                BuzzerOff();
                note(830); //G#5
                timerDelay(timer_cnt, 77);

                //A5
                BuzzerOff();
                note(880); //A5
                timerDelay(timer_cnt, 77);

                //F#4
                BuzzerOff();
                note(369); //F#4
                timerDelay(timer_cnt, 180);

                //A5
                BuzzerOff();
                note(880); //A5
                timerDelay(timer_cnt, 77);

                //G#5
                BuzzerOff();
                note(830); //G#5
                timerDelay(timer_cnt, 77);

                //A5
                BuzzerOff();
                note(880); //A5
                timerDelay(timer_cnt, 77);

                //G#5
                BuzzerOff();
                note(830); //G#5
                timerDelay(timer_cnt, 77);

                //A5
                BuzzerOff();
                note(880); //A5
                timerDelay(timer_cnt, 77);
            }
            /*for (i = 0; i < 1; i++)
             {
             BuzzerOff();
             note(784); //G5
             swDelay(1);

             BuzzerOff();
             swDelay(1);
             note(932.3); //Bb5
             swDelay(1);

             BuzzerOff();
             swDelay(1);
             note(1047); //C6
             swDelay(2);

             //========================

             BuzzerOff();
             swDelay(1);
             note(784); //G5
             swDelay(1);

             BuzzerOff();
             swDelay(1);
             note(932.3); //Bb5
             swDelay(1);

             BuzzerOff();
             swDelay(1);
             note(1109); //C#6
             swDelay(1);

             BuzzerOff();
             note(1047); //C6
             swDelay(3);

             //========================

             BuzzerOff();
             swDelay(1);
             note(784); //G5
             swDelay(1);

             BuzzerOff();
             swDelay(1);
             note(932.3); //Bb5
             swDelay(1);

             BuzzerOff();
             swDelay(1);
             note(1047); //C6
             swDelay(3);

             BuzzerOff();
             note(932.3); //Bb5
             swDelay(1);

             BuzzerOff();
             swDelay(1);
             note(784); //G5
             swDelay(3);
             }*/

            BuzzerOff();
            state = 0;
            configUserLED(0);
            speedChange(3);
            break;

        }
    }
}

/*

 // Useful code starts here
 initLeds();
 configDisplay();
 configKeypad();

 // *** Intro Screen ***
 Graphics_clearDisplay(&g_sContext); // Clear the display

 // Write some text to the display
 Graphics_drawStringCentered(&g_sContext, "Welcome", AUTO_STRING_LENGTH, 64, 50, TRANSPARENT_TEXT);
 Graphics_drawStringCentered(&g_sContext, "to", AUTO_STRING_LENGTH, 64, 60, TRANSPARENT_TEXT);
 Graphics_drawStringCentered(&g_sContext, "ECE2049-C21!", AUTO_STRING_LENGTH, 64, 70, TRANSPARENT_TEXT);

 // Draw a box around everything because it looks nice
 Graphics_Rectangle box = {.xMin = 3, .xMax = 125, .yMin = 3, .yMax = 125 };
 Graphics_drawRectangle(&g_sContext, &box);

 // We are now done writing to the display.  However, if we stopped here, we would not
 // see any changes on the actual LCD.  This is because we need to send our changes
 // to the LCD, which then refreshes the display.
 // Since this is a slow operation, it is best to refresh (or "flush") only after
 // we are done drawing everything we need.
 Graphics_flushBuffer(&g_sContext);

 dispThree[0] = ' ';
 dispThree[2] = ' ';

 while (1)    // Forever loop
 {
 // Check if any keys have been pressed on the 3x4 keypad
 currKey = getKey();

 if (currKey == '*')
 {
 BuzzerOn();
 P1OUT |= BIT0;      // Set the P1.0 as 1 (High)
 }

 if (currKey == '#')
 {
 BuzzerOff();
 P4OUT |= BIT7;      // Set the P4.7 as 1 (High)
 }

 if (currKey)
 {
 dispThree[1] = currKey;
 // Draw the new character to the display
 Graphics_drawStringCentered(&g_sContext, dispThree, dispSz, 64, 90, OPAQUE_TEXT);

 // Refresh the display so it shows the new data
 Graphics_flushBuffer(&g_sContext);

 // wait awhile before clearing LEDs
 swDelay(1);
 setLeds(0);
 }

 }  // end while (1)
 }
 */

void swDelay(char numLoops)
{
    // This function is a software delay. It performs
    // useless loops to waste a bit of time
    //
    // Input: numLoops = number of delay loops to execute
    // Output: none
    //
    // smj, ECE2049, 25 Aug 2013
    // hamayel qureshi, 28 march 2020

    volatile unsigned int i, j;	// volatile to prevent removal in optimization
                                // by compiler. Functionally this is useless code

    for (j = 0; j < numLoops; j++)
    {
        i = 50000;					// SW Delay
        while (i > 0)				// could also have used while (i)
            i--;
    }
}

void timerDelay(long unsigned int initialCount, int delay)
{

    volatile unsigned int i, j; // volatile to prevent removal in optimization
                                // by compiler. Functionally this is useless code

    while (timer_cnt < (initialCount + delay))
    {

    }

}

void speedChange(int speed)
{

    switch (speed)
    {
    case 5:
        TA2CCR0 = 81; // 81+1 = 82 ACLK tics = ~1/400 seconds ///1.5x speed
        break;

    case 4:
        TA2CCR0 = 122; // 122+1 = 123 ACLK tics = ~3/800 seconds //1.25x speed
        break;

    case 3:
        TA2CCR0 = 163; // 163+1 = 164 ACLK tics = ~1/200 seconds //1x speed
        break;

    case 2:
        TA2CCR0 = 204; // 204+1 = 205 ACLK tics = ~1/160 seconds //.75x speed
        break;

    case 1:
        TA2CCR0 = 245; // 245+1 = 246 ACLK tics = ~3/400 seconds //.5x speed
        break;

    }

}

void configUserLED(char inbits)
{

    P1SEL &= ~(BIT0);
    P1DIR |= (BIT0);

    P4SEL &= ~(BIT7);
    P4DIR |= (BIT7);

    if (inbits == 3) //Both LEDs are lit if 3 is passed
    {
        P1OUT |= (BIT0);
        P4OUT |= (BIT7);
    }
    else if (inbits == 2) //Lights LED2 if 2 is passed, turns off LED1
    {
        P1OUT &= ~(BIT0);
        P4OUT |= (BIT7);
    }
    else if (inbits == 1) //Lights LED1 if 1 is passed, turns off LED2
    {
        P1OUT |= (BIT0);
        P4OUT &= ~(BIT7);
    }
    else //Neither LED is lit if 0 is passed
    {
        P1OUT &= ~(BIT0);
        P4OUT &= ~(BIT7);
    }

    //LED1 = P1.0
    //LED2 = P4.7
}

void note(float freq)
{
    //pitch is frequency in Hz
    //duration is time in ms

    float notePeriod = 1.0 / freq;
    float clkPeriod = 1.0 / 32768.0;
    float period = notePeriod / clkPeriod;

    // Initialize PWM output on P3.5, which corresponds to TB0.5
    P3SEL |= BIT5; // Select peripheral output mode for P3.5
    P3DIR |= BIT5;

    TB0CTL = (TBSSEL__ACLK | ID__1 | MC__UP); // Configure Timer B0 to use ACLK, divide by 1, up mode
    TB0CTL &= ~TBIE;           // Explicitly Disable timer interrupts for safety

    // Now configure the timer period, which controls the PWM period
    // Doing this with a hard coded values is NOT the best method
    // We do it here only as an example. You will fix this in Lab 2.
    TB0CCR0 = period;                    // Set the PWM period in ACLK ticks
    TB0CCTL0 &= ~CCIE;                  // Disable timer interrupts

    // Configure CC register 5, which is connected to our PWM pin TB0.5
    TB0CCTL5 = OUTMOD_7;                   // Set/reset mode for PWM
    TB0CCTL5 &= ~CCIE;                     // Disable capture/compare interrupts
    TB0CCR5 = TB0CCR0 / 2;                  // Configure a 50% duty cycle
}

// Timer A2 interrupt service routine
//CHANGE TO OUR LIKING
#pragma vector=TIMER2_A0_VECTOR
__interrupt void TimerA2_ISR(void)
{

    if (lastKey != getKey())
    {
        char interruptKey = getKey();
        lastKey = interruptKey;

        if (interruptKey == '#')
        {
            configUserLED(2);
            state = 0;
            BuzzerOff();
            main();
            //break; //Will this bring us back to state 0?
        }

        if (((state == 3) || (state == 4)) && interruptKey == '1')
        { //pause/play
            if (songStatus == 1)
            {
                songStatus = 0; //set to pause
                BuzzerOff();
                configUserLED(1); //Lights LED 1 (red one)
                //WRITE SOMETHING TO STOP SONG AT THIS POINT
            }
            else if (songStatus == 0)
            {
                songStatus = 1; //set to play
                configUserLED(2); //Lights LED 1 (green one)
                //WRITE SOMETHING TO PLAY SONG AT THE STOPPED POINT
            }
        }

        if (((state == 3) || (state == 4)) && interruptKey == '2')
        { //slower
            if (songSpeed > 1)
            {
                songSpeed--;
                speedChange(songSpeed);
            }
        }

        if (((state == 3) || (state == 4)) && interruptKey == '3')
        { //faster
            if (songSpeed < 5)
            {
                songSpeed++;
                speedChange(songSpeed);
            }
        }

        if (state != 0 && interruptKey == '4') //return to song select
        {
            configUserLED(2);
            state = 1;
            BuzzerOff();
            main();
            //break; //Will this bring us back to state 1?
        }
    }

    if (songStatus == 1)
    {
        timer_cnt++;
    }

}

//NOTES FOR FIXES
//Song must be 28 notes long w/ 8 different pitches - can use a loop to make longer

//Office Hours Questions:
//-How do you change the state and reset the switch case in an interrupt?
//How to debounce button? So controls dont activate more than once per press

/*
 BuzzerOff();
 while(timer_cnt <= 25){
 note(554);//C#5
 }
 BuzzerOff();
 */
/*
 while(timer_cnt <= 50){
 note(493);//B4
 }
 BuzzerOff();

 while(timer_cnt <= 150){
 note(554);//C#5
 }
 BuzzerOff();
 */
