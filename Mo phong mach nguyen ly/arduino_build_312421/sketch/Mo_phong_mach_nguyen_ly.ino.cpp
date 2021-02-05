#include <Arduino.h>
#line 1 "D:\\Mon hoc\\Ky thuat vi xu ly\\BTL\\Mo phong mach nguyen ly\\Mo_phong_mach_nguyen_ly\\Mo_phong_mach_nguyen_ly.ino"
int LAMP = 3;
int dim_val = 2;

#line 4 "D:\\Mon hoc\\Ky thuat vi xu ly\\BTL\\Mo phong mach nguyen ly\\Mo_phong_mach_nguyen_ly\\Mo_phong_mach_nguyen_ly.ino"
void setup();
#line 9 "D:\\Mon hoc\\Ky thuat vi xu ly\\BTL\\Mo phong mach nguyen ly\\Mo_phong_mach_nguyen_ly\\Mo_phong_mach_nguyen_ly.ino"
void zero_cross();
#line 17 "D:\\Mon hoc\\Ky thuat vi xu ly\\BTL\\Mo phong mach nguyen ly\\Mo_phong_mach_nguyen_ly\\Mo_phong_mach_nguyen_ly.ino"
void loop();
#line 4 "D:\\Mon hoc\\Ky thuat vi xu ly\\BTL\\Mo phong mach nguyen ly\\Mo_phong_mach_nguyen_ly\\Mo_phong_mach_nguyen_ly.ino"
void setup(){
  pinMode(LAMP, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(2), zero_cross, RISING); //CHANGE
}

void zero_cross(){
  int dimming_time = (200*dim_val);
  delayMicroseconds(dimming_time);  
  digitalWrite(LAMP, HIGH);
  delayMicroseconds(10);
  digitalWrite(LAMP, LOW);
}

void loop() {
    int data=analogRead(A0);
    int data1 = map(data, 0, 1023,10,49); 
    dim_val=data1;
}

