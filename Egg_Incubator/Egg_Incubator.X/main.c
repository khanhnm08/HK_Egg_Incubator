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

void init_ADC(void);
unsigned int ReadADC (void);
void delay_us(unsigned int uTime);

unsigned int ADCvalue = 0;
unsigned int temp;
unsigned int voltage;
unsigned int setPoint = 37;

unsigned int max_delay = 9000; //us
unsigned int PID_error = 0;
unsigned int previous_error = 0;
unsigned int PID_value = 0;
//PID constants
unsigned int kp = 203;   
unsigned int ki= 7.2;   
unsigned int kd = 1.04;
unsigned int PID_p = 0;    
unsigned int PID_i = 0;    
unsigned int PID_d = 0; 

int cnt = 0;

int main()
{
    char str[20];
    
    // Temperature
    init_ADC();
    TRISA0 = 1; 
        
    // LCD
    TRISD = 0x00;
    Lcd_Init();

    // Zero-cross
    TRISB0 = 1;         // B0 as INPUT for Interrupt
    TRISD0 = 0;         // D0 as OUTPUT
    RD0 = 0;
    
    // Config Timer1
    TMR1 = 0;                    // Clear the Timer1 register to start counting from 0                 
    TMR1CS = 0;                  // Clear the Timer1 clock select bit to choose local clock source                 
    T1CKPS0 = 0;                 // Prescaler ratio 1:1
    T1CKPS1 = 0;                 // Prescaler ratio 1:1
    TMR1ON = 1;                  // Switch ON Timer1
    
    
    // Config INTERRUPT
    GIE = 1;                     // Global Interrupt Enable bit
    INTEDG = 1;                 // Interrupt edge config bit (HIGH value means interrupt occurs every rising edge)
    INTE = 1;                   // IRQ (Interrupt request pin RB0) Interrupt Enable bit
    RBIE = 1;
    TMR1IE = 1;                 // Timer1 Interrupt enable bit
    TMR1IF = 0;                 // Clear the Interrupt flag bit for timer1        
    PEIE = 1;                   // Peripherals Interrupts enable bit 
    
    while(1)
    {
        if(cnt>=4)      // sampling
        {
            cnt = 0;
            ADCvalue = ReadADC();
            voltage = 5000.0f / 1023 * ADCvalue;
            temp = voltage / 10;
              
            PID_error = setPoint - temp;
            if(PID_error > 30)                              //integral constant will only affect errors below 30ºC             
                PID_i = 0;
            PID_p = kp * PID_error;                         //Calculate the P value
            PID_i = PID_i + (ki * PID_error);               //Calculate the I value
            
            /*
            timePrev = Time;                    // the previous time is stored before the actual time read
            Time = millis();                    // actual time read
            elapsedTime = (Time - timePrev) / 1000;   
            PID_d = kd*((PID_error - previous_error)/elapsedTime);  //Calculate the D value
            */
            PID_value = PID_p + PID_i + PID_d;                      //Calculate total PID value
            previous_error = PID_error; //Remember to store the previous error.

            
            if(PID_value < 0)
                PID_value = 0;       
            if(PID_value > 8600)
                PID_value = 8600;    
            
            Lcd_Clear();
            sprintf(str, "Set  = %d ", setPoint);
            Lcd_Set_Cursor(1,1);
            Lcd_Write_String(str);
            //__delay_ms(10);
     
            sprintf(str, "Real = %d ", temp);        
            Lcd_Set_Cursor(2,1);
            Lcd_Write_String(str);
            //__delay_ms(10);
        }
    }
    return 0;
}

void init_ADC (void)// adc
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
    
    if(INTF == 1)
    {  
        delay_us(max_delay-PID_value);
        RD0 = 1;
        __delay_us(1000);
        RD0 = 0;
        //RD0 = ~RD0;     
        INTF = 0; 
    }
    
    if(TMR1IF == 1)
    {               // Check the flag bit 
        cnt++;                
        TMR1IF = 0;                 // Clear interrupt bit for timer1
    }               
}

void delay_us(unsigned int uTime)
{
    unsigned int i = 0;
    while(i<uTime)
    {
        __delay_us(1);
        i++;
    }
}