// VIVIWARE電池残量計測
// ATTINY412, 20MHz, millies support, serial event support, 64mS, 4.2V BOD, BOD enable

#define DAOUT A6
#define VIN A7
#define PWRON A3
#define PWRCUR A0

#define CNTREF 100
#define CNTMAX 50
#define CNTCUR 10
uint16_t val;
uint8_t cnt;
uint8_t Mode;
uint8_t CurMax, CurNum, CurTbl[4]={16,65,128,255};

void setup() {
  VREF.CTRLA |= 0x07 & 1; //DAC_reference 1:1.1v, 2:2.5V, 3:4.3V, 4:1.5V
  VREF.CTRLA |= (0x07 & 1)<<4; //ADC_reference 1:1.1v, 2:2.5V, 3:4.3V, 4:1.5V
  delayMicroseconds(25);
  DAC0.CTRLA = 0x41; // 7:RUNSTDBY, 6:OUTEN, 0:ENABLE
  Serial.swap(1);
  Serial.begin(250000);
  Serial.println("VIVIWARE: vlipo,vcc,curr");
  Mode = 0;
  CurNum = 1;
  CurMax = 4;
  DAC0.DATA = CurTbl[CurNum];
  pinMode(PWRON, OUTPUT);
  digitalWrite(PWRON, LOW);
}
void loop() {  
  uint16_t adc_reading, maxval=0, vcc=0, cur = 0; // 1.1V内部基準電圧でのA/D変換結果
  for(int i=0; i<CNTMAX; i++) { 
    adc_reading = analogRead(VIN);
    if(maxval < adc_reading) {
      maxval = adc_reading;
    }
    delayMicroseconds(400);    
  }
  Serial.print('\n');
  adc_reading = analogRead(ADC_INTREF);
  for(int i=0; i<CNTREF; i++) {
    vcc += analogRead(ADC_INTREF);
  }
  adc_reading = analogRead(PWRCUR);
  for(int i=0; i<CNTCUR; i++) {
    cur += analogRead(PWRCUR);
  }
  float v = vcc / CNTREF;
  float c = cur / CNTCUR;
  float x = 1.1* 1024/v;
  float y = x - maxval * x/1024;
  int z = y * 1000;
  Serial.print(z);
  Serial.print(" ");
  z = x * 1000;
  Serial.print(z);
  if(Mode == 1)   c = c * x / 1024;
  else c = 0.011 * CurTbl[CurNum] / 256;
  z = c * 1000 + 0.5;
  Serial.print(" ");
  Serial.print(z);
  GetCmd();
  delay(100);
}

void GetCmd() {
  int c;
  while(Serial.available() > 0) {
    c = Serial.read();
    switch (c) {
      case '0':
        Mode = 0;
        digitalWrite(PWRON, LOW);
        break;
      case '1':
        Mode = 1;
        digitalWrite(PWRON, HIGH);
        break;
      case '2':
        CurNum++;
        if(CurNum>=CurMax) CurNum = 0;
        DAC0.DATA = CurTbl[CurNum];
        break;
      default:
        Serial.println("0: stop charge, 1: start charge, 2:change current value");      
    }
  }
}
void SetCur(uint8_t type) {
  
}
