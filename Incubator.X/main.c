#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#define _XTAL_FREQ 16000000

// define pin for LCD1602.
#define RS RD7
#define EN RD6
#define D4 RD5
#define D5 RD4
#define D6 RD3
#define D7 RD2
#define ZVC RB0

// define pin for Servo DG90.
#define PWM1_DIR TRISC2
#define PWM1 RC2

#include <xc.h>
#include "lcd.h"

// BEGIN CONFIG
#pragma config FOSC = HS // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF // Watchdog Timer Enable bit (WDT enabled)
#pragma config PWRTE = ON // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = ON // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF // Flash Program Memory Code Protection bit (Code protection off)
//END CONFIG

void Servo_MoveTo(int a);
void init_ADC(void);
void Read_Temp (void);
void Cal_PID(void);

int realValue;
int setPoint = 37;

int PID_error = 0;
int PID_error_1 = 0;
int PID_error_2 = 0;

int PID_value = 0;
int PID_value_1 = 0;

int kp = 203;   
int ki = 7;   
int kd = 1;
int PID_p = 0;    
int PID_i = 0;    
int PID_d = 0; 
unsigned int T = 1000;

unsigned int cnt1 = 0;
unsigned int cnt0 = 0;
unsigned int cnt2 = 0;
unsigned int samp = 0;


int main()
{
    char str[16];
    
    /*------------------------[ Config ADC_LM35 ]-----------------------------*/
    init_ADC(); 
    TRISA0 = 1; 
    //End.
     
    /*--------------------------[ Config Servo ]------------------------------*/
    TRISC2 = 0;
    RC2 = 1;
    CCP1CON = 0x09;    //Compare mode, clear output on match.
    CCP1IE = 0;
    CCP1IF = 0;
    Servo_MoveTo(0);
    //End.
    
    /*---------------------------[ Config LCD ]-------------------------------*/
    TRISD = 0x00;
    RD0 = 0;
    Lcd_Init();
    //End.
    
    /*--------------------[ Config PIN for Button ]---------------------------*/
    TRISB4 = 1;       //B4 as INPUT
    TRISB5 = 1;       //B5 as INPUT
    //End.

    /*------------------[ Config for Zero Voltage Crossing ]------------------*/
    
    TRISB0 = 1;         // B0 as INPUT for Interrupt
    //End.
    
    /*------------------------[ Config for Triac ]----------------------------*/
    TRISB3 = 0;
    TRISB7 = 0;         // RB7 as OUTPUT
    RB3 = 1;
    RB7 = 0;
    //End.
    
    /*-------------------------[ Config for Relay ]----------------------------*/
    TRISB6 = 0;
    RB6 = 1;
    //End.
    
    /*--------------------------[ Config Timer0 ]-----------------------------*/
    // To create interrupt for control AC lamp.
    TMR0 = 0x38;
    OPTION_REG = 0x00;              //Prescaler ratio 1:2 and choose local clock
    //End.
    
    /*--------------------------[ Config Timer1 ]-----------------------------*/
    // To control DC Servo-SG90.
    TMR1 = 0xB1E0;                                // preset Timer1                  
    TMR1CS = 0;                                   // Clear the Timer1 clock select bit to choose local clock source                 
    T1CKPS0 = 0;                                  // Prescaler ratio 1:4
    T1CKPS1 = 1;                                  // Prescaler ratio 1:4
    TMR1ON = 1;                                   // Switch ON Timer1
    //End.
    
    /*--------------------------[ Config Timer2 ]-----------------------------*/
    // To create 1s interrupt for sampling (Calculate PID_value).
    TMR2 = 0x06;                                 // preset Timer2
    T2CON = (1<<1)|(1<<2);                       // prescaler & postscaler 1:16 (bit 1)
                                                 // and set ON Timer2 (bit 2)
    //End.
    
    /*-----------------------[ Config INTERRUPT ]-----------------------------*/
    GIE = 1;                                       // Global Interrupt Enable bit
    PEIE = 1;                                      // Peripherals Interrupts enable bit 
    INTEDG = 1;                                    // Interrupt edge config bit (HIGH value means interrupt occurs every rising edge)
    INTE = 1;                                      // IRQ (Interrupt request pin RB0) Interrupt Enable bit
    RBIE = 1;
    TMR0IE = 0;
    TMR0IF = 0;
    TMR1IE = 1;                                    // Timer1 Interrupt enable bit
    TMR1IF = 0;                                    // Clear the Interrupt flag bit for timer1  
    TMR2IE = 1;
    TMR2IF = 0;
    //End.
    
    while(1)
    {
        Read_Temp();
           
        Lcd_Clear();
        sprintf(str, "Set  = %d ", setPoint);
        Lcd_Set_Cursor(1,1);
        Lcd_Write_String(str);
        sprintf(str, "Real = %d ", realValue);        
        Lcd_Set_Cursor(2,1);
        Lcd_Write_String(str);
        sprintf(str, "%d ", PID_value);        
        Lcd_Set_Cursor(1,12);
        Lcd_Write_String(str);

        if(realValue > setPoint)
        {
            RB6 = 0;
            Servo_MoveTo(90);
        }
        else 
        {
            RB6 = 1;
            Servo_MoveTo(0);
        }
        
        if(samp == 1)
        {
            Cal_PID();
            samp = 0;
        }
        
    }
    return 0;
}

void init_ADC (void)// adc
{
    // chon tan so clock cho bo adc
    ADCON1bits.ADCS2 = 0, ADCON0bits.ADCS1 = 1,ADCON0bits.ADCS0 = 0;
    // chon kenh adc la kenh AN5 - RE0
    ADCON0bits.CHS2 = 1, ADCON0bits.CHS1 = 0, ADCON0bits.CHS0 = 1;
    // chon cach luu data
    ADCON1bits.ADFM = 1;
    // cau hinh cong vao
    ADCON1bits.PCFG3 = 0,  ADCON1bits.PCFG2 = 0,  ADCON1bits.PCFG1 = 0,  ADCON1bits.PCFG0 = 0;
    // cap nguon cho khoi adc
    ADCON0bits.ADON = 1;
}

void Read_Temp (void)// doc len 7 doan
{
    float TempValue = 0;
    ADCON0bits.GO_nDONE = 1;
    while(ADCON0bits.GO_nDONE);
    TempValue = ADRESH*256 + ADRESL;
    TempValue = 5000.0f / 1023 * TempValue;
    realValue = TempValue / 10;
}

void Cal_PID(void)
{
    PID_error = setPoint - realValue;
    //integral constant only affect errors below 30ºC  
    if(PID_error > 30)                                   
        PID_i = 0;
    //Calculate the P value
    PID_p = kp * PID_error;       
    //Calculate the I value
    PID_i = PID_i + (ki * PID_error);         
    //Calculate the D value
    PID_d = kd*(PID_error - PID_error_1);  
    //Calculate total PID value
    PID_value = PID_p + PID_i + PID_d;   
   //Store the previous error.
    PID_error_1 = PID_error;               
    if(PID_value < 1600)
        PID_value = 1600;       
    if(PID_value > 8400)
        PID_value = 8400;   
    PID_value = (10000 - PID_value);
    PID_value = PID_value / 100;
      
//    PID_error = setPoint - realValue;
//    PID_value = PID_value_1;
//    PID_value = PID_value + (kp + ki*0.5 + kd*2)*PID_error;
//    PID_value = PID_value + (-kp - 2*kd*2)*PID_error_1;
//    PID_value = PID_value + kd*2*PID_error_2;
//    
//    PID_value_1 = PID_value;
//    PID_error_2 = PID_error_1;
//    PID_error_1 = PID_error;
//    
//    if(PID_value < 1600)
//        PID_value = 1600;       
//    if(PID_value > 8400)
//        PID_value = 8400; 
//    PID_value = (10000 - PID_value);
//    PID_value = PID_value / 100;
}

void __interrupt() ISR(void)
{  
    if(RBIF == 1) 
    {
        if(RB4 == 0) 
        {
            __delay_ms(100);
            setPoint++;
        }
        if(RB5 == 0)
        {
            __delay_ms(100);
            setPoint--;
        }
        RBIF = 0;
    }
   
    if(TMR0IF == 1)        
    {
        TMR0 = 0x38;
        cnt0++;
        if(cnt0 == PID_value)
        {
            RB7 = 1;
            __delay_us(10);
            RB7 = 0;
            cnt0 = 0;
            TMR0IE = 0;
        }
        TMR0IF = 0;
    }
    
    if(TMR1IF == 1)
    {
        CCP1CON = 0x00;
        RC2 = 1;
        CCP1CON = 0x09;
        TMR1 = 0xB1E0;
        TMR1IF = 0;
    }

    if(TMR2IF == 1)
    {
        TMR2 = 0x06;
        cnt2++;
        if(cnt2 == 1000)
        {
            cnt2 = 0;
            samp = 1;
        }
        TMR2IF = 0;
    }
    
    if(INTF == 1)
    {  
        TMR0IE = 1;
        INTF = 0; 
    }
}

void Servo_MoveTo(int a)
{
    if(a==0)
    {
        CCPR1 = 0xB430; 
    }
    if(a==90)
    {
        CCPR1 = 0xB7A8;  
    }
}