#include <xc.h>
#include <stdio.h>
//#include <string.h>
#include "LCD.h"

#pragma config FOSC=HSPLL
#pragma config WDTEN=OFF
#pragma config XINST=OFF

void InitPins(void);
void ConfigInterrupts(void);
void ConfigPeriph(void);

int count;
volatile char buttonState;

void putch(char c);

void main(void) {
    char currentState = buttonState;
    char temp;

    OSCTUNEbits.PLLEN = 1;
    LCDInit();
    LCDClear();
    InitPins();
    ConfigPeriph();
    ConfigInterrupts();
    count = 0;
    CREN1 = 1;
    while (PIR1bits.RC1IF == 0);
    temp = RCREG1;
    CREN1 = 0;
    while (1) {
        if (currentState != buttonState) {
            currentState = buttonState;
            if (currentState == 0) {
                lprintf(1, "Pressed");
                printf("Pressed");
            } else {
                lprintf(1, "Released");
                printf("Released");
            }
        }
        lprintf(0, "Count = %d", count);
        printf("Count = %d\n", count);
        LATD = ~LATD;
        __delay_ms(1000);
        ++count;
    }
}

void InitPins(void) {
    LATD = 0; //LED's are outputs
    TRISD = 0; //Turn off all LED's
    LATB = 0;
    TRISB = 0b00000001; //Button0 is input;
    RBPU = 0; //enable weak pullups on port B

    //Set TRIS bits for any required peripherals here.
    TRISC = 0b10000000; //RC7 is RX, RC6 is TX

}

void ConfigInterrupts(void) {

    RCONbits.IPEN = 0; //no priorities.  This is the default.

    //Configure your interrupts here

    //set up INT0 to interrupt on falling edge
    INTCON2bits.INTEDG0 = 0; //interrupt on falling edge
    INTCONbits.INT0IE = 1; //Enable the interrupt
    //note that we don't need to set the priority because we disabled priorities (and INT0 is ALWAYS high priority when priorities are enabled.)
    INTCONbits.INT0IF = 0; //Always clear the flag before enabling interrupts
    buttonState = 0xff; //Set current button state (Up);

    INTCONbits.GIE = 1; //Turn on interrupts
}

void ConfigPeriph(void) {

    //Configure peripherals here

    //Configure the USART for 9600 baud asynchronous transmission
    SPBRG1 = 831;       //9600 baud
    SPBRGH1 = 831 >> 8;
    TXSTA1bits.BRGH = 1;
    BAUDCON1bits.BRG16 = 1;
    TXSTA1bits.SYNC = 0; //asynchronous mode
    RCSTA1bits.SPEN = 1; //Enable the serial port
    TXSTA1bits.TXEN = 1; //Enable transmission
}

void __interrupt(high_priority) HighIsr(void) {
    //Check the source of the interrupt
    if (INT0IF == 1) {
        //source is INT0
        buttonState = ~buttonState; //Toggle button state
        INTCON2bits.INTEDG0 = ~INTCON2bits.INTEDG0; //Toggle edge detection - watch for next change
        INT0IF = 0; //must clear the flag to avoid recursive interrupts
    }
}

void putch(char c) {
    while (TX1IF == 0);
    TXREG1 = c;
}
