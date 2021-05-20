#include <xc.h>
#include <stdint.h>

//-------------[ Servo Routines ]------------------
//-------------------------------------------------

void init_CPP_COMP()
{
    CCP1CON = 0x09;  // Compare Mode & Clear CCP Pin On Match!
    CCP1IE = 0;
    PWM1_DIR = 0;
    CCPR1 = 0xB1E0;
}
void PWM1_16Bit_DC(unsigned int DC)
{
    CCPR1 = DC;
}
void init_Servo()
{
    init_CPP_COMP();
}
void Servo_MoveTo(unsigned int Angle) 
{
   // Angle is ranging from 0 upto 180 
   unsigned int DC = Angle*30+7335; 
   PWM1_16Bit_DC(DC);
}
void Servo_Sweep()
{
   for(uint16_t i=7335; i<12735; i++)
   {
     PWM1_16Bit_DC(i);
     __delay_us(250);
   }
   for(uint16_t i=12735; i>7335; i--)
   {
     PWM1_16Bit_DC(i);
     __delay_us(250);
   }
}

//--------------------------------------------------------
