/*===================================CPEG222====================================
 * Program:		Project2_Template.c
 * Authors: 	Devereau Zeleznik & Nathan Ekanem
 * Date: 		10/13/2023
 * Description: 
 *          "Simon Says" type game
 * Input: Buttons U, C, D, L, & R
 * Output: SSD & LCD displays
==============================================================================*/
/*------------------ Board system settings. PLEASE DO NOT MODIFY THIS PART ----------*/
#ifndef _SUPPRESS_PLIB_WARNING //suppress the plib warning during compiling
#define _SUPPRESS_PLIB_WARNING
#endif
#pragma config FPLLIDIV = DIV_2 // PLL Input Divider (2x Divider)
#pragma config FPLLMUL = MUL_20 // PLL Multiplier (20x Multiplier)
#pragma config FPLLODIV = DIV_1 // System PLL Output Clock Divider (PLL Divide by 1)
#pragma config FNOSC = PRIPLL   // Oscillator Selection Bits (Primary Osc w/PLL (XT+,HS+,EC+PLL))
#pragma config FSOSCEN = OFF    // Secondary Oscillator Enable (Disabled)
#pragma config POSCMOD = XT     // Primary Oscillator Configuration (XT osc mode)
#pragma config FPBDIV = DIV_2   // Peripheral Clock Divisor (Pb_Clk is Sys_Clk/8)
    
/*----------------------------------------------------------------------------*/

#include <xc.h> //Microchip XC processor header which links to the PIC32MX370512L header
#include <stdio.h>
#include "config.h" //Also need to include i2c.h, i2c.c, utils.h, utils.c in the project header files.
#include "lcd.h"
#include "ssd.h"
#include "acl.h"
#include "i2c.h"
#include "utils.h"

/* --------------------------- Forward Declarations-------------------------- */
void initialize_ports();
void initialize_output_states();
void set_random_seed();
void toggle_LEDs();
void toggle_SSD();
void delay_ms(int ms);
void handle_raw_button_presses();
void set_random_number();
void logic_mode_one();
void logic_mode_two();
void logic_mode_three();
void logic_mode_four();
void logic_mode_five();
void user_input();
/* -------------------------- Definitions------------------------------------ */
#define SYS_FREQ (80000000L)
#define _80Mhz_ (80000000L)
#define LOOPS_NEEDED_TO_DELAY_ONE_MS_AT_80MHz 1426
#define LOOPS_NEEDED_TO_DELAY_ONE_MS (LOOPS_NEEDED_TO_DELAY_ONE_MS_AT_80MHz * (SYS_FREQ / _80Mhz_))
#define BtnC_RAW PORTFbits.RF0
#define BtnU_RAW PORTBbits.RB1
#define BtnR_RAW PORTBbits.RB8
#define BtnL_RAW PORTBbits.RB0
#define BtnD_RAW PORTAbits.RA15
#define TRUE 1
#define FALSE 0
#define BUTTON_DEBOUNCE_DELAY_MS 20
#define SSD_EMPTY_DIGIT 18

/*---------------Global Variables---------------*/
int buttonsLocked = FALSE;
int array [32];
int input [32];
int count = 3;
int fig1 = 3; 
char ssdIsOn = 0;

char pressedUnlockedBtnC = FALSE;
char pressedUnlockedBtnU = FALSE;
char pressedUnlockedBtnD = FALSE;
char pressedUnlockedBtnL = FALSE;
char pressedUnlockedBtnR = FALSE;
enum mode
{
    MODE1,
    MODE2,
    MODE3,
    MODE4,
    MODE5,
    
} 
mode = MODE1;

/*---------------State Machine---------------*/

int main(void)
{
    initialize_ports();
    initialize_output_states();
    set_random_seed();
    set_random_number();
    

    while (TRUE)
    {
        // next state logic
        handle_raw_button_presses();
       
        
        
        switch (mode){
            case MODE1:
                logic_mode_one();
                break;
            case MODE2:
                logic_mode_two();
                break;
            case MODE3:
                logic_mode_three();
                break;
            case MODE4:
                logic_mode_four();
                break;     
            case MODE5:
                logic_mode_five();
                break;
        
    }
        if (pressedUnlockedBtnC)
        {
            mode = !mode;
            SSD_WriteDigits(31, 31, 31, 31, 0, 0, 0, 0);
        }
    }
}

void initialize_ports()
{
    DDPCONbits.JTAGEN = 0; // Statement is required to use Pin RA0 as IO
    TRISBbits.TRISB1 = 1; // RB1 (BTNU) configured as input
    ANSELBbits.ANSB1 = 0; // RB1 (BTNU) disabled analog
    TRISBbits.TRISB0 = 1; // RB1 (BTNL) configured as input
    ANSELBbits.ANSB0 = 0; // RB1 (BTNL) disabled analog
    TRISFbits.TRISF4 = 1; // RF0 (BTNC) configured as input
    TRISBbits.TRISB8 = 1; // RB8 (BTNR) configured as input
    ANSELBbits.ANSB8 = 0; // RB8 (BTNR) disabled analog
    TRISAbits.TRISA15 = 1; // RA15 (BTND) configured as input
    TRISA &= 0xFF00;
    ACL_Init();
    SSD_Init();
    LCD_Init();
}

void initialize_output_states()
{
    LCD_WriteStringAtPos("    Template    ", 0, 0);
    LCD_WriteStringAtPos("                ", 1, 0);
    LATA &= 0xFF00;
    // clear the SSD
    SSD_WriteDigits(SSD_EMPTY_DIGIT, SSD_EMPTY_DIGIT, SSD_EMPTY_DIGIT, SSD_EMPTY_DIGIT, 0, 0, 0, 0);
}

void set_random_seed()
{
    float rgACLGVals[3];
    ACL_ReadGValues(rgACLGVals);
    int seed = rgACLGVals[0] * 10000;
    srand((unsigned)seed);
}

void toggle_LEDs()
{
    short topEigthLATABits = (LATA & 0xFF00);
    short invertertedBottomEightLATABits = (~LATA & 0x00FF);
    LATA = topEigthLATABits | invertertedBottomEightLATABits;
}

void toggle_SSD()
{
    int number;
    if (ssdIsOn)
    {
        // clear the SSD
        SSD_WriteDigits(SSD_EMPTY_DIGIT, SSD_EMPTY_DIGIT, SSD_EMPTY_DIGIT, SSD_EMPTY_DIGIT, 0, 0, 0, 0);
    }
    else
    {
        // show decimal number with leading zeroes
        number = rand() % 256;
        unsigned char d4 = (number / 1000) % 10;
        unsigned char d3 = (number / 100) % 10;
        unsigned char d2 = (number / 10) % 10;
        unsigned char d1 = number % 10;
        // logic to remove leading zeroes could go here
        SSD_WriteDigits(d1, d2, d3, d4, 0, 0, 0, 0);

    }
}

void delay_ms(int ms)
{
    int i;
    for (i = 0; i < ms * LOOPS_NEEDED_TO_DELAY_ONE_MS; i++)
    {    }
}

void handle_raw_button_presses()
{
    pressedUnlockedBtnC = FALSE;
    pressedUnlockedBtnU = FALSE;
    pressedUnlockedBtnR = FALSE;
    pressedUnlockedBtnL = FALSE;
    pressedUnlockedBtnD = FALSE;

    if ((BtnC_RAW || BtnU_RAW || BtnR_RAW || BtnL_RAW || BtnD_RAW) && !buttonsLocked)
    {
        delay_ms(BUTTON_DEBOUNCE_DELAY_MS); // debounce
        buttonsLocked = TRUE;
        pressedUnlockedBtnC = BtnC_RAW;
        pressedUnlockedBtnU = BtnU_RAW;
        pressedUnlockedBtnD = BtnD_RAW;
        pressedUnlockedBtnL = BtnL_RAW;
        pressedUnlockedBtnR = BtnR_RAW;
    }
    else if (!(BtnC_RAW || BtnU_RAW || BtnR_RAW || BtnL_RAW || BtnD_RAW) && buttonsLocked)
    {
        delay_ms(BUTTON_DEBOUNCE_DELAY_MS); // debounce
        buttonsLocked = FALSE;
    }
    
}
/*---------------MODE ONE---------------*/

void logic_mode_one(){
        LCD_WriteStringAtPos("      Ready       ", 0, 0);
        LCD_WriteStringAtPos("   Press BtnC     ", 1, 0);
        SSD_WriteDigits(31, 31, 31, 31, 0, 0, 0, 0);
        return;
    }
/*---------------MODE TWO---------------*/

void logic_mode_two() {
        LCD_WriteStringAtPos("    Watch the     ", 0, 0);
        LCD_WriteStringAtPos("    Sequence      ", 1, 0);
        
        for(int i = 0; i < count; i++ ){
            
        if(array[i] == 0){//up
            SSD_WriteDigits(31, 31, 24, 23, 0, 0, 0, 0);
        }
        else if(array[i] == 1){//down
            SSD_WriteDigits(22, 21, 18, 13, 0, 0, 0, 0);
        }
        else if(array[i] == 2){//left
            SSD_WriteDigits(26, 15, 14, 29, 0, 0, 0, 0);
        }
        else if(array[i] == 3){//right
            SSD_WriteDigits(14, 26, 27, 28, 0, 0, 0, 0);
        }
        delay_ms(450);
        SSD_WriteDigits(31, 31, 31, 31, 0, 0, 0, 0);
        delay_ms(50);
            }
         mode = MODE3;
    return;
}
/*---------------MODE THREE---------------*/
void logic_mode_three() {
    int fig1;
    fig1 = count;

        LCD_WriteStringAtPos("   Repeat the   ", 0, 0);
        LCD_WriteStringAtPos("    Sequence    ", 1, 0);
        SSD_WriteDigits(fig1, 31, 31, 31, 0, 0, 0, 0);
        SSD_WriteDigits(fig1, 31, 31, 31, 0, 0, 0, 0);
        
        int i = 0;
        int holder = fig1;
        SSD_WriteDigits(fig1, 31, 31, 31, 0, 0, 0, 0);
        
        while ( i < fig1) {
            handle_raw_button_presses();
            if(pressedUnlockedBtnC) {
                holder = fig1;
                
                for(int p = 0; p < 32; p++){
                    input[p]=0;
                }
                i = 0;
            }
            else if (pressedUnlockedBtnU){
                input[i] = 0;
                holder -= 1;
                SSD_WriteDigits(holder, 31, 31, 31, 0, 0, 0, 0); //displays number of remaining moves
                i ++;
            }
            else if (pressedUnlockedBtnR){
                input[i] = 3;
                holder -= 1;
                SSD_WriteDigits(holder, 31, 31, 31, 0, 0, 0, 0);
                i ++;
            }
            else if (pressedUnlockedBtnL){
                input[i] = 2;
                holder -= 1;
                SSD_WriteDigits(holder, 31, 31, 31, 0, 0, 0, 0);
                i ++;
            }
            else if (pressedUnlockedBtnD){
                input[i] = 1;
                holder -= 1;
                SSD_WriteDigits(holder, 31, 31, 31, 0, 0, 0, 0);
                i ++;
            }
        }
        
        handle_raw_button_presses();
        char correct = 1;
        for(int i = 0; i < fig1; i++){
            if(input[i] != array[i]){
                correct = 0;
            }
        }
        if(correct){
            mode = MODE4;
        }
        else{
            mode = MODE5;
        }
    return;
}
/*---------------MODE FOUR---------------*/
void logic_mode_four() {
    char val[32];
        SSD_WriteDigits(31, 31, 31, 31, 0, 0, 0, 0);
        LCD_WriteStringAtPos(val, 0, 0);
        sprintf(val,"Yes! Scored %d",count);
        LCD_WriteStringAtPos("Press BtnC     ", 1, 0);
        handle_raw_button_presses();
        if (pressedUnlockedBtnC){
            count+=1;
            mode=MODE1;
        }
        //mode = MODE5;
    return;
}

/*---------------MODE FIVE---------------*/

void logic_mode_five() {
        SSD_WriteDigits(31, 31, 31, 31, 0, 0, 0, 0);
        LCD_WriteStringAtPos("                 ", 0, 0);
        LCD_WriteStringAtPos("                 ", 1, 0);
        char val[32];
        sprintf(val,"Fail to score %d",count);
        LCD_WriteStringAtPos("Press BtnC     ", 1, 0);
        LCD_WriteStringAtPos(val, 0, 0);
        int number;
        while(!pressedUnlockedBtnC){
            handle_raw_button_presses();
            if (number<500){
                SSD_WriteDigits(14, 17, 10, 19, 0, 0, 0, 0);
                number++;
            }
            else if(number<1000){
                SSD_WriteDigits(28, 14, 30, 18, 0, 0, 0, 0);
                number++;
            }
            else{ number = 0;
            }
            delay_ms(1);
        }
                
        mode = MODE1;
    return;
}
/*---------------Generates A Random Number For The Array---------------*/
void set_random_number()
{
    for(int i=0; i<32; i++){
        array[i] = (rand()%4);
    }
}

/*---------------Checks If Button Input Matches Generated Value In Array---------------*/
void userinput()
{
    for(int i=0; i<32; i++){
        input[i] = array[i];
    }
}