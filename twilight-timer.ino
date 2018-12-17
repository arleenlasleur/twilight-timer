#include <Wire.h>
#include <RTClib.h>
#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <avr/pgmspace.h>

#define out_dis (3)
#define out_en  (4)

#define btn1 (5)
#define btn2 (9)
#define btn3 (8)
#define btn4 (6)
#define btn5 (7)

#define sel1 (A7)

#define reprate    (200)    // repeat rate, ms
#define btnsingthr (700)    // hold thresholds, ms
#define btndecthr  (2000)
#define btnhundthr (4500)

LiquidCrystal lcd(1,0,A0,A1,A2,A3); //RS,E,D4-D7
RTC_DS1307 rtc;

/* 
Keycodes:

  1: up    7: <
  2: dn    4: >
  5: set;

Level:

  1..5   - press
  11..15 - hold
  21..25 - hold more
  31..35 - hold even more
  0      - no key

Mode:

  0: idle

  1: set day    4: hour
  2: mon        5: min
  3: year       6: sec
  
  7: set ON offset
  8: set OFF time

  9: set ON time override
  10: set ON time

  11: show ON time
  12: show OFF time       */

const uint8_t dom[12] PROGMEM={31,28,31,30,31,30,31,31,30,31,30,31};
const uint8_t twhr[12][31] PROGMEM={
  {17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17},
  {17,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18},
  {18,18,18,18,18,18,18,18,18,18,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19},
  {19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20},
  {20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,21,21,21,21,21,21,21,21,21,21,21},
  {21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21},
  {21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,20,20},
  {20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,19,19,19},
  {19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,18,18,18,18,18,18,18,18},
  {18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17},
  {17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17},
  {17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17}
};
const uint8_t twmn[12][31] PROGMEM={
  {17,18,19,20,21,22,23,24,25,26,28,29,30,32,33,34,36,37,38,40,41,43,44,46,47,49,50,52,54,55,57},
  {58, 0, 2, 3, 5, 6, 8,10,11,13,15,16,18,19,21,23,24,26,28,29,31,33,34,36,38,39,41,42,44,44,44},
  {44,46,47,49,51,52,54,55,57,59, 0, 2, 4, 5, 7, 8,10,12,13,15,17,18,20,22,23,25,27,28,30,31,33},
  {35,36,38,40,41,43,45,47,48,50,52,53,55,57,58, 0, 2, 4, 5, 7, 9,11,12,14,16,18,19,21,23,25,26},
  {26,28,30,32,33,35,37,38,40,42,44,45,47,49,50,52,54,55,57,58, 0, 2, 3, 5, 6, 8, 9,10,12,13,14},
  {16,17,18,19,20,21,22,23,24,25,26,27,27,28,29,29,30,30,31,31,31,31,32,32,32,32,32,31,31,31,31},
  {31,30,30,29,29,28,27,27,26,25,24,23,22,21,20,19,18,17,15,14,13,11,10, 8, 7, 5, 4, 2, 1,59,57},
  {55,54,52,50,48,46,44,42,41,39,37,35,33,30,28,26,24,22,20,18,16,14,11, 9, 7, 5, 3, 0,58,56,54},
  {51,49,47,45,42,40,38,35,33,31,29,26,24,22,20,17,15,13,10, 8, 6, 4, 1,59,57,55,52,50,48,46,44},
  {44,41,39,37,35,33,31,28,26,24,22,20,18,16,14,12,10, 8, 6, 4, 2, 0,59,57,55,53,51,50,48,46,44},
  {43,41,40,38,36,35,33,32,31,29,28,27,25,24,23,22,21,20,18,17,16,16,15,14,13,12,12,11,10,10, 9},
  { 9, 9, 8, 8, 8, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 9, 9,10,10,11,12,12,13,14,15,16}
};

byte mode=100;         // mode
byte onOffset=0;       // ON offset time
int  offtime=0;        // OFF absolute time
int  ontime=0;         // ON absolute time
byte tmp;              // data buffer

int timeNow, timeOn, timeOff;                // time buffers
byte time[7]={0,0,0,0,0,0,99};

volatile boolean       bKeyDown=false,       // keyboard data
                       bPrevKeyDown=false;
volatile byte          nKey,prevKey;
unsigned long tot_key,last_key;
boolean  bKeyPressing=false;

boolean  onFlag=false;                       // flags
boolean  bypFlag=false;
boolean  onOverrideFlag=false;

// ----------------------- Mode --------------------------------------------------------------------------------

void changeMode(byte newMode){
  DateTime now;

  if(mode>=1 && mode<=6 && time[6]!=99) rtc.adjust(DateTime(time[0]+2000, time[1], time[2], time[3], time[4], time[5])); // store RTC time
  
  if(mode==7){                                       // store ON offset time
    if(EEPROM.read(0)!=onOffset) EEPROM.write(0,onOffset);
  }
  
  if(mode==8){                                       // store OFF time
    if(EEPROM.read(1)!=highByte(offtime)) EEPROM.write(1,highByte(offtime));
    if(EEPROM.read(2)!=lowByte(offtime)) EEPROM.write(2,lowByte(offtime));
  }
  
  if(mode==9){                                       // store ON time override flag
    if(onOverrideFlag) tmp=1;
     else tmp=0;
    if(EEPROM.read(3)!=tmp) EEPROM.write(3,tmp);
  }
  
  if(mode==10){                                       // store ON time
    if(EEPROM.read(4)!=highByte(ontime)) EEPROM.write(4,highByte(ontime));
    if(EEPROM.read(5)!=lowByte(ontime)) EEPROM.write(5,lowByte(ontime));
  }

  now=rtc.now();
  mode=newMode;
  lcd.clear();
  if(mode>0 && mode<=10){               // cursor mode
    lcd.blink();
    lcd.cursor();
  }else{
    lcd.noBlink();
    lcd.noCursor();
  }
  lcd.setCursor(0,0);                   // initial print
  switch(mode){
  case 7:
    lcd.print("ON offset:");
    showOnOffsetTime();
  break;
  case 8:
    lcd.print("OFF time:");
    showOffTime();
  break;
  case 9:
    lcd.print("ON override:");
    showOnOverride();
  break;
  case 10:
    lcd.print("ON time:");
    showOnTime();
  break;
  }
  if(mode>=1 && mode<=6){
    lz(now.day());       // d
    lcd.print("/");
    lz(now.month());     // m
    lcd.print("/20");
    lz(now.year()-2000); // y
    lcd.setCursor(0,1);
    lz(now.hour());      // hh
    lcd.print(":");
    lz(now.minute());    // mm
    lcd.print(":");
    lz(now.second());    // ss
  }
  switch(mode){                         // initial cursor
    case 1: lcd.setCursor(0,0); break;
    case 2: lcd.setCursor(3,0); break;
    case 3: lcd.setCursor(8,0); break;
    case 4: lcd.setCursor(0,1); break;
    case 5: lcd.setCursor(3,1); break;
    case 6: lcd.setCursor(6,1); break;
  }
}

// ----------------------- Keyboard ----------------------------------------------------------------------------

void kbdFlag(){
  if(digitalRead(2)){          // rising
    prevKey=nKey;
    nKey=0;
    bKeyDown=false;
  }else{                       // falling
    nKey=0;
    if(!digitalRead(btn1)) nKey=1;
    if(!digitalRead(btn2)) nKey=2;
    if(!digitalRead(btn3)) nKey=7;  // patch
    if(!digitalRead(btn4)) nKey=4;
    if(!digitalRead(btn5)) nKey=5;
    bKeyDown=true;
  }
}

byte kbdMain(){  
  byte kbdres;
  if(bPrevKeyDown!=bKeyDown){        // interrupt noise reduction
    bPrevKeyDown=bKeyDown;
    delay(10);
  }
  if(!bKeyPressing && bKeyDown){     // press
    if(nKey!=0) tot_key=millis();
    bKeyPressing=true;
    return 0;
  }
  if(bKeyPressing && bKeyDown){      // repeat
    if(millis()-tot_key>=btnhundthr) kbdres=nKey+30;
     else if(millis()-tot_key>=btndecthr) kbdres=nKey+20;
      else if(millis()-tot_key>=btnsingthr) kbdres=nKey+10;
  }
  if(bKeyPressing && !bKeyDown){     // release
    if(millis()-tot_key<btnsingthr) kbdres=prevKey;
     else kbdres=0;
    bKeyPressing=false;
  }
  if(kbdres<10) return kbdres;       // return press code immediately
   else{
    if(millis()-last_key>=reprate){  // return hold code at repeat rate
      last_key=millis();
      return kbdres;
    }
  }
}

// ------------------------------------------------- Leading zero
void lz(byte arg){
 if(arg<10) lcd.print("0");
 lcd.print(arg);
}

// ------------------------------------------------- Show clock time while change
void showTime(byte x,byte y,byte data){
  lcd.setCursor(x,y);
  lz(data);
  lcd.setCursor(x,y);
};

// ------------------------------------------------- Show ON offset time while change
void showOnOffsetTime(){
  lcd.setCursor(0,1);
  lcd.print("      ");
  lcd.setCursor(0,1);
  if(onOffset<120){
    lcd.print("-");
    tmp=(byte)(onOffset/60);
    if(onOffset==60 || onOffset==0) tmp--;
    lz(1-tmp);
  }else{
    lcd.print("+");
    tmp=(byte)((onOffset-120)/60);
    lz(tmp);
  }
  lcd.print(":");
  if(onOffset<120){
    tmp=60-onOffset%60;
    if(tmp==60) tmp=0;
    lz(tmp);
  }else{
    lz((onOffset-120)%60);
  }
  lcd.setCursor(0,1);
};

// ------------------------------------------------- Show OFF time while change
void showOffTime(){
  lcd.setCursor(0,1);
  lcd.print("     ");
  lcd.setCursor(0,1);
  lz((byte)(offtime/60));
  lcd.print(":");
  lz(offtime%60);
  lcd.setCursor(0,1);
}
// ------------------------------------------------- Show ON time override flag
void showOnOverride(){
  lcd.setCursor(0,1);
  if(onOverrideFlag) lcd.print("enabled ");
   else lcd.print("disabled");
  lcd.setCursor(0,1);
}
// ------------------------------------------------- Show ON time while change
void showOnTime(){
  lcd.setCursor(0,1);
  lcd.print("     ");
  lcd.setCursor(0,1);
  if(onOverrideFlag) lz((byte)(ontime/60));
  else lz((byte)(timeOn/60));
  lcd.print(":");
  if(onOverrideFlag) lz(ontime%60);
  else lz(timeOn%60);
  lcd.setCursor(0,1);
}

// ----------------------- Setup -------------------------------------------------------------------------------

void setup(){
  Wire.begin();
  rtc.begin();
  lcd.begin(16,2);
//  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));     // set system time (uncomment before uploading)

  pinMode(2,INPUT_PULLUP);     // int0
  pinMode(btn1,INPUT_PULLUP);  // btns
  pinMode(btn2,INPUT_PULLUP);
  pinMode(btn3,INPUT_PULLUP);
  pinMode(btn4,INPUT_PULLUP);
  pinMode(btn5,INPUT_PULLUP);
  pinMode(sel1,INPUT);

  attachInterrupt(0,kbdFlag,CHANGE);                     // keyboard
  last_key=millis();
  nKey=0;

  onOffset=EEPROM.read(0);                                                   // switch ON offset read
  if(onOffset>240) onOffset=120;                                             // validate
  if(EEPROM.read(0)!=onOffset) EEPROM.write(0,onOffset);                     // store

  offtime=word(EEPROM.read(1),EEPROM.read(2));                               // switch OFF time read
  if(offtime>1439 || offtime<0) offtime=60;                                  // validate
  if(EEPROM.read(1)!=highByte(offtime)) EEPROM.write(1,highByte(offtime));   // store
  if(EEPROM.read(2)!=lowByte(offtime)) EEPROM.write(2,lowByte(offtime));

  tmp=EEPROM.read(3);                                                        // override flag read
  if(tmp==0) onOverrideFlag=false;                                           // validate
   else if(tmp==1) onOverrideFlag=true;
    else{
      onOverrideFlag=false;
      tmp=0;
    }
  if(EEPROM.read(3)!=tmp) EEPROM.write(3,tmp);                               // store

  ontime=word(EEPROM.read(4),EEPROM.read(5));                                // switch ON time read
  if(ontime>1439 || ontime<0) ontime=30;                                     // validate
  if(EEPROM.read(4)!=highByte(ontime)) EEPROM.write(4,highByte(ontime));     // store
  if(EEPROM.read(5)!=lowByte(ontime)) EEPROM.write(5,lowByte(ontime));

  changeMode(0);
}

// ----------------------- Main --------------------------------------------------------------------------------

void loop (){
  DateTime now = rtc.now();
  byte daymax;

  switch(mode){
  //--------------------------------------------------------- idle
  case 0:
    time[0]=now.year()-2000;                                  // get new time
    time[1]=now.month();
    time[2]=now.day();
    time[3]=now.hour();
    time[4]=now.minute();
    time[5]=now.second();

                                                              // get on time
    if(!onOverrideFlag){
      timeOn=pgm_read_byte(&(twhr[time[1]-1][time[2]-1]));
      timeOn*=60;
      timeOn+=pgm_read_byte(&(twmn[time[1]-1][time[2]-1]));
    }
     else timeOn=ontime;
    if(onOffset<120){                                         // offset
      timeOn-=onOffset;
      if(timeOn<0) timeOn+=1440;
    }else{
      timeOn+=(onOffset-120);
      if(timeOn>1439) timeOn-=1440;
    }
    timeOff=offtime;                                          // get off time

    if(timeOn>1439) timeOn=1439;                              // validate
    if(timeOff>1439) timeOff=1438;

    timeNow=time[3];                                          // get current time
    timeNow*=60;
    timeNow+=time[4];        //[4]*60+[5] for MM:SS

    if(timeOff==timeOn){                                      // restrict full cycle
      if(timeOff<1439) timeOff++;
       else if(timeOn>0) timeOn--;
        else timeOn=1438;
    }

    if(timeOn<timeOff){                                       // compare time
      if(timeNow>=timeOn && timeNow<timeOff) onFlag=true;
       else onFlag=false;
    }else{
      if(timeNow<timeOn && timeNow>=timeOff) onFlag=false;
       else onFlag=true;
    }

    tmp=analogRead(sel1)>>7;                                  // check for bypass
    if(tmp<=1){
      bypFlag=true;
      onFlag=false;
    }else if(tmp>1 && tmp<6){
      bypFlag=false;
    }else if(tmp>=6){
      bypFlag=true;
      onFlag=true;
    }

    if(onFlag){                                               // trigger
       digitalWrite(out_dis,0);
       digitalWrite(out_en,1);
    }else{
       digitalWrite(out_dis,1);
       digitalWrite(out_en,0);
    }

    if(time[5]!=time[6]){                                     // new time, show on LCD
      time[6]=time[5];                  // store seconds
      lcd.setCursor(0,0);
      lz(time[2]);     // d
      lcd.print("/");
      lz(time[1]);     // m
      lcd.print("/20");
      lz(time[0]);     // y
      lcd.setCursor(0,1);
      lz(time[3]);     // hh
      lcd.print(":");
      lz(time[4]);     // mm
      lcd.print(":");
      lz(time[5]);     // ss
      lcd.setCursor(13,0);              // show trigger state
      if(onFlag) lcd.print(" On");
       else lcd.print("Off");
      lcd.setCursor(10,1);              // show bypass
      if(bypFlag) lcd.print("Bypass");
       else lcd.print("      ");
    }
    tmp=kbdMain();
    if(tmp==7) changeMode(11);
    if(tmp==4) changeMode(12);
    if(tmp==15) changeMode(1);
  break;
  //--------------------------------------------------------- set day
  case 1:
    switch(kbdMain()){
    case 7:                                     // mode
      if(onOverrideFlag) changeMode(10);
       else changeMode(9);
    break;
    case 4: changeMode(2); break;
    case 5: changeMode(0); break;
    case 1:                                     // change
    case 11:
      daymax=pgm_read_byte(dom+time[1]-1);
      if(time[1]==2 && time[0]%4==0) daymax++;
      if(time[2]<daymax) time[2]++;
      showTime(0,0,time[2]);
    break;
    case 21:
      daymax=pgm_read_byte(dom+time[1]-1);
      if(time[1]==2 && time[0]%4==0) daymax++;
      if(time[2]<daymax-9) time[2]+=10;
       else time[2]=daymax;
      showTime(0,0,time[2]);
    break;
    case 2:
    case 12:
      if(time[2]>1) time[2]--;
      showTime(0,0,time[2]);
    break;
    case 22:
      if(time[2]>10) time[2]-=10;
       else time[2]=1;
      showTime(0,0,time[2]);
    break;
    }
  break;
  //--------------------------------------------------------- set month
  case 2:
    switch(kbdMain()){
    case 7: changeMode(1); break;               // mode
    case 4: changeMode(3); break;
    case 5: changeMode(0); break;
    case 1:                                     // change
    case 11:
      if(time[1]<12) time[1]++;
      showTime(3,0,time[1]);
    break;
    case 2:
    case 12:
      if(time[1]>1) time[1]--;
      showTime(3,0,time[1]);
    break;
    }
  break;
  //--------------------------------------------------------- set year
  case 3:
    switch(kbdMain()){
    case 7: changeMode(2); break;               // mode
    case 4: changeMode(4); break;
    case 5: changeMode(0); break;
    case 1:                                     // change
    case 11:
      if(time[0]<99) time[0]++;
      showTime(8,0,time[0]);
    break;
    case 21:
      if(time[0]<90) time[0]+=10;
       else time[0]=99;
      showTime(8,0,time[0]);
    break;
    case 2:
    case 12:
      if(time[0]>0) time[0]--;
      showTime(8,0,time[0]);
    break;
    case 22:
      if(time[0]>9) time[0]-=10;
       else time[0]=0;
      showTime(8,0,time[0]);
    break;
    }
  break;
  //--------------------------------------------------------- set hour
  case 4:
    switch(kbdMain()){
    case 7: changeMode(3); break;               // mode
    case 4: changeMode(5); break;
    case 5: changeMode(0); break;
    case 1:                                     // change
    case 11:
      if(time[3]<23) time[3]++;
      showTime(0,1,time[3]);
    break;
    case 21:
      if(time[3]<14) time[3]+=10;
       else time[3]=23;
      showTime(0,1,time[3]);
    break;
    case 2:
    case 12:
      if(time[3]>0) time[3]--;
      showTime(0,1,time[3]);
    break;
    case 22:
      if(time[3]>9) time[3]-=10;
       else time[3]=0;
      showTime(0,1,time[3]);
    break;
    }
  break;
  //--------------------------------------------------------- set minute
  case 5:
    switch(kbdMain()){
    case 7: changeMode(4); break;               // mode
    case 4: changeMode(6); break;
    case 5: changeMode(0); break;
    case 1:                                     // change
    case 11:
      if(time[4]<59) time[4]++;
      showTime(3,1,time[4]);
    break;
    case 21:
      if(time[4]<50) time[4]+=10;
       else time[4]=59;
      showTime(3,1,time[4]);
    break;
    case 2:
    case 12:
      if(time[4]>0) time[4]--;
      showTime(3,1,time[4]);
    break;
    case 22:
      if(time[4]>9) time[4]-=10;
       else time[4]=0;
      showTime(3,1,time[4]);
    break;
    }
  break;
  //--------------------------------------------------------- set second
  case 6:
    switch(kbdMain()){
    case 7: changeMode(5); break;               // mode
    case 4: changeMode(7); break;
    case 5: changeMode(0); break;
    case 1:                                     // change
    case 11:
      if(time[5]<59) time[5]++;
      showTime(6,1,time[5]);
    break;
    case 21:
      if(time[5]<50) time[5]+=10;
       else time[5]=59;
      showTime(6,1,time[5]);
    break;
    case 2:
    case 12:
      if(time[5]>0) time[5]--;
      showTime(6,1,time[5]);
    break;
    case 22:
      if(time[5]>9) time[5]-=10;
       else time[5]=0;
      showTime(6,1,time[5]);
    break;
    }
  break;
  //--------------------------------------------------------- set ON offset
  case 7:
    switch(kbdMain()){
    case 7: changeMode(6); break;
    case 4: changeMode(8); break;
    case 5: changeMode(0); break;
    case 1:
    case 11:
      if(onOffset<240) onOffset++;
      showOnOffsetTime();
    break;
    case 21:
      if(onOffset<230) onOffset+=10;
       else onOffset=240;
      showOnOffsetTime();
    break;
    case 2:
    case 12:
      if(onOffset>0) onOffset--;
      showOnOffsetTime();
    break;
    case 22:
      if(onOffset>9) onOffset-=10;
       else onOffset=0;
      showOnOffsetTime();
    break;
    }
  break;
  //--------------------------------------------------------- set OFF time
  case 8:
    switch(kbdMain()){
    case 7: changeMode(7); break;
    case 4: changeMode(9); break;
    case 5: changeMode(0); break;
    case 1:
    case 11:
      if(offtime<1439) offtime++;
      showOffTime();
    break;
    case 21:
      if(offtime<1430) offtime+=10;
       else offtime=1439;
      showOffTime();
    break;
    case 31:
      if(offtime<1380) offtime+=60;
       else offtime=1439;
      showOffTime();
    break;
    case 2:
    case 12:
      if(offtime>0) offtime--;
      showOffTime();
    break;
    case 22:
      if(offtime>9) offtime-=10;
       else offtime=0;
      showOffTime();
    break;
    case 32:
      if(offtime>59) offtime-=60;
       else offtime=0;
      showOffTime();
    break;
    }
  break;
  //--------------------------------------------------------- set ON time override flag
  case 9:
    switch(kbdMain()){
    case 7: changeMode(8); break;
    case 4:
      if(onOverrideFlag) changeMode(10);
       else changeMode(1);
    break;
    case 5: changeMode(0); break;
    case 1:
      if(!onOverrideFlag) onOverrideFlag=true;
      showOnOverride();
    break;
    case 2:
      if(onOverrideFlag) onOverrideFlag=false;
      showOnOverride();
    break;
    }
  break;
  //--------------------------------------------------------- set ON time
  case 10:
    switch(kbdMain()){
    case 7: changeMode(9); break;
    case 4: changeMode(1); break;
    case 5: changeMode(0); break;
    case 1:
    case 11:
      if(ontime<1439) ontime++;
      showOnTime();
    break;
    case 21:
      if(ontime<1430) ontime+=10;
       else ontime=1439;
      showOnTime();
    break;
    case 31:
      if(ontime<1380) ontime+=60;
       else ontime=1439;
      showOnTime();
    break;
    case 2:
    case 12:
      if(ontime>0) ontime--;
      showOnTime();
    break;
    case 22:
      if(ontime>9) ontime-=10;
       else ontime=0;
      showOnTime();
    break;
    case 32:
      if(ontime>59) ontime-=60;
       else ontime=0;
      showOnTime();
    break;
    }
  break;
  //--------------------------------------------------------- show ON time
  case 11:
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Today ON time:");
    showOnTime();
    delay(1500);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Override:");
    showOnOverride();
    delay(1500);
    changeMode(0);
  break;
  //--------------------------------------------------------- show ON time
  case 12:
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("OFF time:");
    showOffTime();
    delay(1500);
    changeMode(0);
  break;
  }
}
