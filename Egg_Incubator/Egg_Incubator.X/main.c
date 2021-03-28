#include <stdio.h>
#define _XTAL_FREQ 4000000

#define RS RD2
#define EN RD3
#define D4 RD4
#define D5 RD5
#define D6 RD6
#define D7 RD7

#include <xc.h>
#include "lcd.h";

// BEGIN CONFIG
#pragma config FOSC = XT // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF // Watchdog Timer Enable bit (WDT enabled)
#pragma config PWRTE = ON // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = ON // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF // Flash Program Memory Code Protection bit (Code protection off)
//END CONFIG

void ADCinit (void);
unsigned int ReadADC (void);
unsigned int ADCvalue = 0;
unsigned int temp;
unsigned int voltage;
unsigned int setPoint = 37;

int main()
{
    char str[20];
    
    // Temperature
    ADCinit();
    TRISA0 = 1; 
        
    // LCD
    TRISD = 0x00;
    Lcd_Init();

    // Zero cross
    TRISB0 = 1;         // B0 as INPUT for Interrupt
    TRISD0 = 0;         // D0 as OUTPUT
    RD0 = 0;
    
    // Config INTERRUPT
    GIE = 1;                     // Global Interrupt Enable bit
    INTEDG = 1;                 // Interrupt edge config bit (HIGH value means interrupt occurs every rising edge)
    INTE = 1;                   // IRQ (Interrupt request pin RB0) Interrupt Enable bit
    RBIE = 1;
    
    while(1)
    {

        ADCvalue = ReadADC();
        voltage = 5000.0f / 1023 * ADCvalue;
        temp = voltage / 10;
        
        Lcd_Clear();
        sprintf(str, "Set  = %d ", setPoint);
        Lcd_Set_Cursor(1,1);
        Lcd_Write_String(str);
        __delay_ms(10);
     
        sprintf(str, "Real = %d ", temp);        
        Lcd_Set_Cursor(2,1);
        Lcd_Write_String(str);
        __delay_ms(10);
    }
    return 0;
}

void ADCinit (void)// adc
{
    // chon tan so clock cho bo adc
    ADCON1bits.ADCS2 = 0, ADCON0bits.ADCS1 = 0,ADCON0bits.ADCS0 = 1;
    // chon kenh adc la kenh an0
    ADCON0bits.CHS2 = 0, ADCON0bits.CHS1 = 0, ADCON0bits.CHS0 = 0;
    // chon cach luu data
    ADCON1bits.ADFM = 1;
    // cau hinh cong vao
    ADCON1bits.PCFG3 = 1,  ADCON1bits.PCFG2 = 1,  ADCON1bits.PCFG1 = 1,  ADCON1bits.PCFG0 = 0;
    // cap nguon cho khoi adc
    ADCON0bits.ADON = 1;
}

unsigned int ReadADC (void)// doc len 7 doan
{
    unsigned int TempValue = 0;
    ADCON0bits.GO_nDONE = 1;
    while(ADCON0bits.GO_nDONE);
    TempValue = ADRESH*256 + ADRESL;
    return (TempValue);
}

void __interrupt() ISR(void)
{  
    if(RBIF == 1) 
    {
        if(RB4 == 1) 
            setPoint++;
        if(RB5 == 1)
            setPoint--;
        RBIF = 0;
    }
}