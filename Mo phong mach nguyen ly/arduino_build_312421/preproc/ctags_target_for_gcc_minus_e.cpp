# 1 "D:\\Mon hoc\\Ky thuat vi xu ly\\BTL\\Mo phong mach nguyen ly\\Mo_phong_mach_nguyen_ly\\Mo_phong_mach_nguyen_ly.ino"
int LAMP = 3;
int dim_val = 2;

void setup(){
  pinMode(LAMP, 0x1);
  attachInterrupt(((2) == 2 ? 0 : ((2) == 3 ? 1 : -1)), zero_cross, 3); //CHANGE
}

void zero_cross(){
  int dimming_time = (200*dim_val);
  delayMicroseconds(dimming_time);
  digitalWrite(LAMP, 0x1);
  delayMicroseconds(10);
  digitalWrite(LAMP, 0x0);
}

void loop() {
    int data=analogRead(A0);
    int data1 = map(data, 0, 1023,10,49);
    dim_val=data1;
}
