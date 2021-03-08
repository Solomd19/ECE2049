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

// Function Prototypes
void swDelay(char numLoops);
void print1();
void print2();
void print3();
void print4();

// Main
void main(void)

{

    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer. Always need to stop this!!
                              // You can then configure it properly, if desired

    initLeds();
    configDisplay();
    configKeypad();

    unsigned char currKey = 0, dispSz = 3;
    unsigned char dispThree[3];
    int i, j, k, l, f;
    int currIndex;
    //char currKey;
    int level;
    int state = 0;  //0 = Start Screen
                    //1 = Start Countdown
                    //2 = game sequence
                    //3 = lose screen + fanfare
                    //4 = win screen + fanfare
                    //5 = error

    char sequence[10]; //2114312413

    for (f = 0; f <= 9; f++)
    {

        sequence[f] = rand() % 4 + 49;

    }

    while (1)
    {
        switch (state)
        {
        case 0: // Display welcome screen
            Graphics_clearDisplay(&g_sContext); // Clear the display
            Graphics_drawStringCentered(&g_sContext, "SIMON",
            AUTO_STRING_LENGTH,
                                        64, 60,
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

        case 1: //1 = Start Countdown
            Graphics_clearDisplay(&g_sContext); // Clear the display
            Graphics_drawStringCentered(&g_sContext, "3", AUTO_STRING_LENGTH,
                                        64, 60, TRANSPARENT_TEXT);
            Graphics_flushBuffer(&g_sContext);
            swDelay(2);
            Graphics_clearDisplay(&g_sContext); // Clear the display
            Graphics_drawStringCentered(&g_sContext, "2", AUTO_STRING_LENGTH,
                                        64, 60, TRANSPARENT_TEXT);
            Graphics_flushBuffer(&g_sContext);
            swDelay(2);
            Graphics_clearDisplay(&g_sContext); // Clear the display
            Graphics_drawStringCentered(&g_sContext, "1", AUTO_STRING_LENGTH,
                                        64, 60, TRANSPARENT_TEXT);
            Graphics_flushBuffer(&g_sContext);
            swDelay(2);
            state = 2;
            break;

        case 2: //2 = game sequence
            Graphics_clearDisplay(&g_sContext); // Clear the display
            Graphics_flushBuffer(&g_sContext);

            for (level = 1; level <= 10; level++)
            {
                if ((currKey != 0 && currKey != '1' && currKey != '2'
                        && currKey != '3' && currKey != '4' && state != 2)
                        || state == 3)
                {
                    break;
                }

                for (i = 1; i <= level; i++)
                { //displays sequence of current level
                    Graphics_clearDisplay(&g_sContext); // Clear the display

                    swDelay(1);

                    //Graphics_drawStringCentered(&g_sContext, sequence[i-1], AUTO_STRING_LENGTH, 64, 60, TRANSPARENT_TEXT); //sequence[i-1]
                    if (sequence[i - 1] == '1')
                    {
                        print1();
                        BuzzerOn();
                    }
                    else if (sequence[i - 1] == '2')
                    {
                        print2();
                        BuzzerOn();
                    }
                    else if (sequence[i - 1] == '3')
                    {
                        print3();
                        BuzzerOn();
                    }
                    else if (sequence[i - 1] == '4')
                    {
                        print4();
                        BuzzerOn();
                    }
                    else
                    {
                        Graphics_drawStringCentered(&g_sContext, "ERROR",
                        AUTO_STRING_LENGTH,
                                                    64, 60,
                                                    TRANSPARENT_TEXT);
                    }

                    Graphics_flushBuffer(&g_sContext);

                    swDelay(1);
                    BuzzerOff();

                }

                int currIndex = 0;
                while (currIndex < level)    // Forever loop
                {

                    Graphics_clearDisplay(&g_sContext); // Clear the display

                    // Check if any keys have been pressed on the 3x4 keypad
                    currKey = getKey();

                    if (currKey != 0 && currKey != '1' && currKey != '2'
                            && currKey != '3' && currKey != '4')
                    {
                        //Graphics_drawStringCentered(&g_sContext, "ERROR", AUTO_STRING_LENGTH, 64, 60, TRANSPARENT_TEXT);
                        //Graphics_flushBuffer(&g_sContext);
                        state = 5;
                        break;
                    }

                    if (currKey != 0 && currKey != sequence[currIndex])
                    {
                        //Graphics_drawStringCentered(&g_sContext, "ERROR", AUTO_STRING_LENGTH, 64, 60, TRANSPARENT_TEXT);
                        //Graphics_flushBuffer(&g_sContext);
                        state = 3;
                        break;
                    }

                    if (currKey == '1' && '1' == sequence[currIndex])
                    {
                        print1();
                        BuzzerOn();
                        currIndex++;
                        Graphics_flushBuffer(&g_sContext);
                        swDelay(1);
                        BuzzerOff();
                    }

                    if (currKey == '2' && '2' == sequence[currIndex])
                    {
                        print2();
                        BuzzerOn();
                        currIndex++;
                        Graphics_flushBuffer(&g_sContext);
                        swDelay(1);
                        BuzzerOff();
                    }

                    if (currKey == '3' && '3' == sequence[currIndex])
                    {
                        print3();
                        BuzzerOn();
                        currIndex++;
                        Graphics_flushBuffer(&g_sContext);
                        swDelay(1);
                        BuzzerOff();
                    }

                    if (currKey == '4' && '4' == sequence[currIndex])
                    {
                        print4();
                        BuzzerOn();
                        currIndex++;
                        Graphics_flushBuffer(&g_sContext);
                        swDelay(1);
                        BuzzerOff();
                    }

                } //end while loop

            }
            if (state == 2)
            {
                state = 4;
            }

            break;

        case 3: //3 = lose screen + fanfare
            Graphics_clearDisplay(&g_sContext); // Clear the display
            for (k = 0; k < 3; k++)
            {
                Graphics_drawStringCentered(&g_sContext, "LOSE",
                AUTO_STRING_LENGTH,
                                            64, 60,
                                            TRANSPARENT_TEXT);
                Graphics_flushBuffer(&g_sContext);
                BuzzerOn();
                swDelay(3);

                Graphics_clearDisplay(&g_sContext); // Clear the display
                BuzzerOff();
                swDelay(3);
            }
            state = 0;
            break;

        case 4: //4 = win screen + fanfare
            Graphics_clearDisplay(&g_sContext); // Clear the display

            Graphics_drawStringCentered(&g_sContext, "WIN!", AUTO_STRING_LENGTH,
                                        64, 60, TRANSPARENT_TEXT);
            Graphics_flushBuffer(&g_sContext);
            BuzzerOn();
            swDelay(.5);
            BuzzerOff();
            swDelay(.5);
            BuzzerOn();
            swDelay(5);

            Graphics_clearDisplay(&g_sContext); // Clear the display
            BuzzerOff();

            state = 0;
            break;

        case 5: //5 = error
            Graphics_clearDisplay(&g_sContext); // Clear the display
            for (j = 0; j < 3; j++)
            {
                Graphics_drawStringCentered(&g_sContext, "SIMON SAYS ERROR",
                AUTO_STRING_LENGTH,
                                            64, 60,
                                            TRANSPARENT_TEXT);
                Graphics_flushBuffer(&g_sContext);
                BuzzerOn();
                swDelay(3);

                Graphics_clearDisplay(&g_sContext); // Clear the display
                BuzzerOff();
                swDelay(3);
            }

            state = 0;
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

void print1()
{
    Graphics_drawStringCentered(&g_sContext, "1", AUTO_STRING_LENGTH, 16, 60,
    TRANSPARENT_TEXT);
}

void print2()
{
    Graphics_drawStringCentered(&g_sContext, "2", AUTO_STRING_LENGTH, 48, 60,
    TRANSPARENT_TEXT);
}

void print3()
{
    Graphics_drawStringCentered(&g_sContext, "3", AUTO_STRING_LENGTH, 80, 60,
    TRANSPARENT_TEXT);
}

void print4()
{
    Graphics_drawStringCentered(&g_sContext, "4", AUTO_STRING_LENGTH, 112, 60,
    TRANSPARENT_TEXT);
}
