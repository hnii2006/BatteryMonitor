// serial receiver and display percent
// for VIVIWARE CORE

#include <M5StickCPlus.h>

#define PowerDownTime 30
#define UsbMinVoltage 4.5
#define WorkingMaxVoltage 4.25
#define M5LED GPIO_NUM_10

int charge;
unsigned long cha_time, pwr_time;
long cha_lapse, pwr_lapse;
int cha_next;
int vbat_o, BatPercentage_o;
int BatPercentage_c;
float vlipo_o;

#define CHARGE_PERIOD 60
void setup() {
  M5.begin();
  M5.Beep.setBeep(4000, 300);
  Serial.println("Start");
  Serial1.begin(250000,SERIAL_8N1,0,26);
  pinMode(M5LED, OUTPUT);
  digitalWrite(M5LED, HIGH);
  Serial1.write('0');
  // LCD display
  M5.Lcd.setRotation( 1 );
  M5.Lcd.setTextColor(TFT_GREEN);
  M5.Lcd.setTextFont(2);
  M5.Lcd.setTextSize(1);  
  M5.Lcd.println(" VIVIWARE Lipo meter v1.43");
  charge = 0;
  cha_time = millis();
  cha_next = 10;
  BatPercentage_c = 0;
  pwr_time = millis();
  vlipo_o = 0;
}

void loop() {
  int eof = 0;
  int inInt, BatPercentage;
  float vcc;
  int cur;
  int pwr_btn;
  M5.update();
  M5.Beep.update();
  do {
    inInt = Serial1.read();
  } while(inInt !='\n');
  inInt = Serial1.parseInt();
  vcc = 0.001 * Serial1.parseInt();
  cur = Serial1.parseInt();
  Serial.print(inInt);
  float vlipo = inInt/1000.0;
  BatPercentage = Volt2percent2(inInt);  
  float vbat = M5.Axp.GetBatVoltage();
  cha_lapse = (millis()-cha_time)/1000;
  pwr_lapse = (millis()-pwr_time)/1000;
  pwr_btn = M5.Axp.GetBtnPress();
  if((vlipo > WorkingMaxVoltage)&&(vlipo_o <= WorkingMaxVoltage)) {
    pwr_time = millis();
  }
  if((vlipo <= WorkingMaxVoltage)&&(vlipo_o > WorkingMaxVoltage)) {
    pwr_time = millis();
  }

  if(M5.BtnA.wasPressed()) {
    if((charge == 0)&&(vlipo < 4.2)) {
      Serial1.write('1');
      cha_time = millis();
      cha_next = CHARGE_PERIOD;
      //M5.Beep.tone(880,1000);
      charge = 1;
      BatPercentage_c = BatPercentage;
    } //else {
      //M5.Beep.beep();
    //}
    pwr_time = millis();
  }
  if(charge > 1) {
    charge--;
    if(charge == 1) {
      BatPercentage_c = BatPercentage;
      digitalWrite(M5LED, HIGH);
      Serial1.write('1');
    }
  }
  if(cha_lapse > cha_next) {
    cha_next+=CHARGE_PERIOD;
    if(charge == 1) {
      charge = 4;
      digitalWrite(M5LED, LOW);
      Serial1.write('0');
    }
  }
  if((charge == 1)&&(cur < 10)&&(cha_lapse % 10 == 5)) {
    Serial1.write('0');
    sound_beep(5);
    charge = 0;
    while(Serial1.available())Serial1.read();
  }
  if(M5.BtnB.wasPressed()) {
    Serial1.write('2');
    pwr_time = millis();
  }
  
  if((charge==0)||(charge>=1)) {
    BatPercentage_o = BatPercentage;
    vbat_o = vbat;
    /// screen clear
    M5.Lcd.fillRect(0,13,M5.Lcd.width(),M5.Lcd.height(),TFT_BLACK);
    /// debug info
    M5.Lcd.setTextFont(1);
    M5.Lcd.setTextSize(1.8);
    M5.Lcd.setCursor(3,M5.Lcd.height()-10);
    M5.Lcd.print(vbat);
    M5.Lcd.print("v (M5 battery), ");
    M5.Lcd.print(vcc);  
    M5.Lcd.print("v (M5 outputVolt)");
    M5.Lcd.setCursor(3,M5.Lcd.height()-20);
    M5.Lcd.print("Charge:");
    M5.Lcd.print(cha_lapse);
    M5.Lcd.print("s,Lazy: ");
    M5.Lcd.print(pwr_lapse);
    M5.Lcd.print("s, Mode:");
    if(charge == 0 ) { M5.Lcd.print("Meter"); 
    } else { M5.Lcd.print("Charge");
    }  
    /// normal info
    M5.Lcd.setTextFont(2);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(3,M5.Lcd.height()-50);
    M5.Lcd.print(vlipo);
    M5.Lcd.print("v, ");
    M5.Lcd.print(cur);
    M5.Lcd.print("mA");
    M5.Lcd.setTextFont(2);
    M5.Lcd.setTextSize(5);
    M5.Lcd.setCursor(0,12);
    if(charge>0) {
      M5.Lcd.setTextColor(TFT_RED);
      BatPercentage = BatPercentage_c;
    }
    if(BatPercentage>=0) {
      M5.Lcd.printf(" %03d",BatPercentage);
    } else {
      M5.Lcd.print(" ---");    
    }
    M5.Lcd.println("%");
    M5.Lcd.setTextColor(TFT_GREEN);  
  }
  if((M5.Axp.GetVBusVoltage() < UsbMinVoltage) && (pwr_lapse>PowerDownTime)) {
    M5.Axp.PowerOff();
  }
  if(pwr_btn > 0) {
    pwr_time = millis();
    if(pwr_btn <= 2) {
      M5.Axp.PowerOff();
    }
  }
  vlipo_o = vlipo;
}
void sound_beep(int n) {
  for(int i=0; i<n; i++) {
    M5.Beep.beep();
    M5.Beep.update();
    delay(500);
    M5.Beep.update();
    delay(500);
  }
}
int BufCnt = 11;
int BufMap_d[12]={3614,3685,3726,3748,3776,3814,3867,3934,4013,4095,4220, 9999}; //discharge
int Volt2percent2(int inInt) {
  int i;
  for (i=0; i< BufCnt; i++) {
    if(charge==0) {
      if(inInt < BufMap_d[i]) break;
    } else {
      if(inInt < BufMap_d[i]) break;      
    }
  }
  i = i * 10;
  if (i>100) i = -1;
  return i;
}
