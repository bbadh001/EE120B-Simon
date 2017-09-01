#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include "io.c"

//
//
//

volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks


void TimerOn() {
    // AVR timer/counter controller register TCCR1
    TCCR1B = 0x0B;// bit3 = 0: CTC mode (clear timer on compare)
    // bit2bit1bit0=011: pre-scaler /64
    // 00001011: 0x0B
    // SO, 8 MHz clock or 8,000,000 /64 = 125,000 ticks/s
    // Thus, TCNT1 register will count at 125,000 ticks/s
    
    // AVR output compare register OCR1A.
    OCR1A = 125;    // Timer interrupt will be generated when TCNT1==OCR1A
    // We want a 1 ms tick. 0.001 s * 125,000 ticks/s = 125
    // So when TCNT1 register equals 125,
    // 1 ms has passed. Thus, we compare to 125.
    // AVR timer interrupt mask register
    TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt
    
    //Initialize avr counter
    TCNT1=0;
    
    _avr_timer_cntcurr = _avr_timer_M;
    // TimerISR will be called every _avr_timer_cntcurr milliseconds
    
    //Enable global interrupts
    SREG |= 0x80; // 0x80: 1000000
}

void TimerOff() {
    TCCR1B = 0x00; // bit3bit1bit0=000: timer off
}

void TimerISR() {
    TimerFlag = 1;
}

// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect) {
    // CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
    _avr_timer_cntcurr--; // Count down to 0 rather than up to TOP
    if (_avr_timer_cntcurr == 0) { // results in a more efficient compare
        TimerISR(); // Call the ISR that the user uses
        _avr_timer_cntcurr = _avr_timer_M;
    }
}

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
    _avr_timer_M = M;
    _avr_timer_cntcurr = _avr_timer_M;
}

/*
 * Global Variables:
 */

unsigned char playerWon; //flag is set when player wins
unsigned char continueButton;

const unsigned long MAX_SEQUENCE_LENGTH = 9; //Length of LED Sequence:
//unsigned char sequence[] = {1,2,3,4,4,3,2,1,1}; //Holds randomly generated sequence
unsigned char sequence[9];

unsigned char SS_index; //Index for sequence array
unsigned char UI_index;

//
unsigned char currentRoundNum; //Round Number (0-8)
unsigned char nextRound; // 1 to go to next round
unsigned char newGame; // 1 to go to new game
unsigned char allowForInput;

/*
 * Helper Functions:
 */

//Initialize random LED sequence:
void initRandomSequence()
{
    unsigned char i = 0;
    for (i = 0; i < MAX_SEQUENCE_LENGTH; i++)
    {
        sequence[i] = (rand() % 4) + 1;
    }
}

//Helper function, converts button press to a digit 1-4:
unsigned char convertToButtonNum(unsigned char input)
{
    unsigned char buttonNum;
    if (input == 0x0F)
        buttonNum = 0;
    else if (input == 0x0E)
        buttonNum = 1;
    else if (input == 0x0D)
        buttonNum = 2;
    else if (input == 0x0B)
        buttonNum = 3;
    else if (input == 0x07)
        buttonNum = 4;
	   else
           buttonNum = 0;
    
    return buttonNum;
}

/*
 * LCD Tick Function:
 */

enum LCD_SM_states {LCD_initial,LCD_welcome,LCD_inGame,LCD_win,LCD_lose} LCD_SM_state;

void TICK_FUNC_LCD()
{
    switch (LCD_SM_state)
    {
        case LCD_initial:
            LCD_SM_state = LCD_welcome;
            LCD_DisplayString(1,"Welcome to SIMON  Press Start!");
            break;
        case LCD_welcome:
            if (continueButton)
            {
                LCD_SM_state = LCD_inGame;
                LCD_DisplayString(1,"Your Score Is: ");
                LCD_WriteData((currentRoundNum) + '0');
            }
            else
                LCD_SM_state = LCD_welcome;
            break;
        case LCD_inGame:
            if (playerWon)
            {
                LCD_SM_state = LCD_win;
                LCD_DisplayString(1,"  YOU WON     Play Again?");
            }
            else if (!playerWon && newGame)
            {
                LCD_SM_state = LCD_lose;
                LCD_DisplayString(1,"  YOU LOST    Play Again?");
            }
            else if (!playerWon && !newGame && nextRound)
            {
                LCD_SM_state = LCD_inGame;
                LCD_DisplayString(1,"Your Score Is: ");
                LCD_WriteData((currentRoundNum+1) + '0');
            }
            else if (!playerWon && !newGame && !nextRound)
                LCD_SM_state = LCD_inGame;
            break;
        case LCD_win:
            if (continueButton)
            {
                LCD_SM_state = LCD_inGame;
                LCD_DisplayString(1,"Your Score Is: ");
                LCD_WriteData(currentRoundNum + '0');
            }
            else
                LCD_SM_state = LCD_win;
            break;
        case LCD_lose:
            if (continueButton)
            {
                LCD_SM_state = LCD_inGame;
                LCD_DisplayString(1,"Your Score Is: ");
                LCD_WriteData(currentRoundNum + '0');
            }
            else
                LCD_SM_state = LCD_lose;
            break;
    }
    
    switch (LCD_SM_state)
    {
        case LCD_initial:
            break;
        case LCD_welcome:
            break;
        case LCD_inGame:
            break;
        case LCD_win:
            break;
        case LCD_lose:
            break;
    }
}


/*
 * Continue Button Tick Function:
 */

enum CB_SM_states {CB_init,CB_checkButton,CB_ignoreButton} CB_SM_state;

void TICK_FUNC_continueButton()
{
    switch(CB_SM_state)
    {
        case CB_init:
            CB_SM_state = CB_checkButton;
            break;
        case CB_checkButton:
            if (continueButton)
                CB_SM_state = CB_ignoreButton;
            else if (!continueButton)
                CB_SM_state = CB_checkButton;
            break;
        case CB_ignoreButton:
            if (newGame)
                CB_SM_state = CB_checkButton;
            break;
    }
    
    switch(CB_SM_state)
    {
        case CB_init:
            break;
        case CB_checkButton:
            continueButton = !((PINB & 0x10) >> 4);
            break;
        case CB_ignoreButton:
            break;
    }
}

/*
 * ShowSequence Tick Function:
 */

enum SS_SM_states {SS_init,SS_waitForContinue,SS_showSequence,SS_off,SS_waitForPlayer} SS_SM_state;

void TICK_FUNC_showSequence()
{
    switch(SS_SM_state)
    {
        case SS_init:
            newGame = 0;
            nextRound = 0;
            allowForInput = 0;
            currentRoundNum = 0;
            playerWon = 0;
            SS_index = 0;
            initRandomSequence();
            SS_SM_state = SS_waitForContinue;
            break;
        case SS_waitForContinue:
            if (continueButton)
                SS_SM_state = SS_showSequence;
            else
                SS_SM_state = SS_waitForContinue;
            break;
        case SS_showSequence:
            SS_SM_state = SS_off;
            break;
        case SS_off:
            if (SS_index < currentRoundNum)
                SS_SM_state = SS_showSequence;
            else if (SS_index >= currentRoundNum)
                SS_SM_state = SS_waitForPlayer;
            SS_index++;
            break;
        case SS_waitForPlayer:
            if (nextRound && newGame)
            {
                SS_index = 0;
                currentRoundNum = 0;
                SS_SM_state = SS_init;
            }
            else if (nextRound && !newGame)
            {
                SS_index = 0;
                currentRoundNum++;
                nextRound = 0;
                SS_SM_state = SS_showSequence;
            }
            else
            {
                SS_SM_state = SS_waitForPlayer;
            }
            break;
    }
    
    switch(SS_SM_state)
    {
            unsigned char curLED;
        case SS_init:
            break;
        case SS_waitForContinue:
            break;
        case SS_showSequence:
            curLED = sequence[SS_index];
            if (curLED == 1)
                PORTA = 0x01;
            else if (curLED == 2)
                PORTA = 0x02;
            else if (curLED == 3)
                PORTA = 0x04;
            else if (curLED == 4)
                PORTA = 0x08;
            break;
        case SS_off:
            PORTA = 0x00;
            break;
        case SS_waitForPlayer:
            allowForInput = 1;
            break;
    }
}

/*
 * UserInput Tick Function:
 */

unsigned char buttonNumber;
unsigned char input;

enum UI_SM_states {UI_init,UI_waitForSequence,UI_getInput,UI_waitForZero,UI_success,UI_fail} UI_SM_state;

void TICK_FUNC_UserInput()
{
    input = PINB & 0x0F;
    if (input == 0x0F)
        buttonNumber = 0;
    else if (input == 0x0E)
        buttonNumber = 1;
    else if (input == 0x0D)
        buttonNumber = 2;
    else if (input == 0x0B)
        buttonNumber = 3;
    else if (input == 0x07)
        buttonNumber = 4;
    else
        buttonNumber = 0;
    // buttonNumber = convertToButtonNum(input); //decimal number for sequence comparison
    switch(UI_SM_state)
    {
        case UI_init:
            UI_index = 0;
            UI_SM_state = UI_waitForSequence;
            break;
        case UI_waitForSequence:
            if (allowForInput)
                UI_SM_state = UI_getInput;
            else
                UI_SM_state = UI_waitForSequence;
            break;
        case UI_getInput:
            if (buttonNumber == 0)
                UI_SM_state = UI_getInput;
            else if (buttonNumber == sequence[UI_index] && UI_index  < currentRoundNum)
            {
                UI_index++;
                UI_SM_state = UI_waitForZero;
            }
            else if (buttonNumber == sequence[UI_index] && UI_index  == currentRoundNum)
                UI_SM_state = UI_success;
            else if (buttonNumber != sequence[UI_index])
                UI_SM_state = UI_fail;
            break;
        case UI_waitForZero:
            if (buttonNumber != 0)
                UI_SM_state = UI_waitForZero;
            else if (buttonNumber == 0)
                UI_SM_state = UI_getInput;
            break;
        case UI_success:
            UI_SM_state = UI_init;
            break;
        case UI_fail:
            UI_SM_state = UI_init;
            break;
    }
    
    switch(UI_SM_state)
    {
        case UI_init:
            break;
        case UI_waitForSequence:
            break;
        case UI_getInput:
            break;
        case UI_waitForZero:
            break;
        case UI_success:
            allowForInput = 0;
            nextRound = 1;
            if (currentRoundNum == 8)
            {
                newGame = 1;
                playerWon = 1;
            }
            break;
        case UI_fail:
            allowForInput = 0;
            nextRound = 1;
            newGame = 1;
            break;
    }    
}

void main(void)
{
    unsigned char seed = 100;
    srand(seed);
    
    //Timer Init:
    TimerSet(200);
    TimerOn();
    
    //Port Init:
    DDRA = 0xFF; PORTA = 0x00;
    DDRB = 0x00; PORTB = 0xFF;
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;
    
    //State Init:
    LCD_SM_state = LCD_initial;
    SS_SM_state = SS_init;
    UI_SM_state = UI_init;
    CB_SM_state = CB_init;
    
    //LCD init:
    LCD_init();
    
    //Continue button init:
    continueButton = 0;
    
    while(1)
    {
        TICK_FUNC_LCD();
        TICK_FUNC_continueButton();
        TICK_FUNC_showSequence();
        TICK_FUNC_UserInput();
        while(!TimerFlag);
        TimerFlag = 0;
    }
}
