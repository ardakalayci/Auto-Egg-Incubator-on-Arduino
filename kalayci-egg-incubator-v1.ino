/*
EEPROM.read(4); ısı
EEPROM.read(8); nem
EEPROM.read(12); calisma suresi
EEPROM.read(16); calisma modu lvl0

*/
#include <Servo.h>
// include the library code:
#include <LiquidCrystal.h>
// Makina acip kapatildiginda bilgileri hafızada tutmak icin kullanılan EEPROM'a kayıt yapabilmek icin kullanılan Kutuphane
#include <EEPROM.h>
// Zaman hesapları ve Saat makina kapalı olsada calismasi ve zamani tutmasi icin gerekli kutuphane
#include  <virtuabotixRTC.h>
// I2C ile calisan Saat (RTC) ve LCD ile haberlesmek icin gereken kutuphane
#include <Wire.h>
// Sıcaklik sensoru DHT11 veya DHT22 yi kullanabilmek icin gereken kutuphane
#include "DHT.h" // sıcaklık kütüphanesi
#define nem A2
#define isi A1
#define DHTPIN 2        // DHT11PIN olarak Dijital 3 belirtiyorum belirliyoruz.
#define DHTTYPE DHT22   // DHT 22 Sensoru Kullandigimizi belirtiyoruz.
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
//Sicaklık Tanımlaması Yapıyoruz
DHT dht(DHTPIN, DHTTYPE);
//Or     CLK -> 6 , DAT -> 7, Reset -> 8
virtuabotixRTC myRTC(11, 12, 13); //If you change the wiring change the pins here also
Servo myservo;
 


//init menu level
int pos;
int year=2019; //init year 2000-xxxx
int month=1; //init month 1-12
int day=1; //init month 1-31
int weekday=1; //init week day 0 - sunday
int hour=1;
int minute=1;
int lev0=0; //init level-0
int lev1=0; //init level-1
int sel_item=0; //Selected item
int first_item=0; //Selected menu first item
int last_item=0; //Selected menu last item
int qty=0; //number of menu items
byte menu_sym=0x7e; //menu symbol
int del=250; //delay
byte mcols=16; //lcd display number of cols
byte mrows=2; //lcd display number of rows
bool clr=false;
bool mstate=false;
float brightness=255; //default brightness
float isiayar=35; //default isiayar
float isiolcum;
float nemayar=50;
float nemolcum;
int kuluckasuresi=20;
long previousMillis = 0;  // ????? ????? ????????? ????? ?????????? ????????? ????????? ???????? ????????
long interval = 2000;      // ???????? ?????? ?????????? ? ?? ? ??? ?? ????? ???????? ??????????????? ????
long previousMillis2 = 0;  // ????? ????? ????????? ????? ?????????? ????????? ????????? ???????? ????????
float interval2 = 40;      // ???????? ????? ??????? ?????????? ? ??
bool infunc=false; //?????????? ??? ????????? ?????? ?????????? ??????? ????
int ekran =0;
int kalangun=0;
int saatekran=0;
int kalan,dakkalan;
void lcd_clr() {
//clear screen function
lcd.clear();
lcd.setCursor(0,0);
lcd.print("CLEAR SCREEN");
delay(1000);
lcd_menu();
}

//set Lmenu
typedef struct {
  unsigned char* itemname;
  byte level;
  byte sublevel;
  bool checked;
  bool selected;
  void (*function)();
  } Lmenu;

Lmenu lmenu[]= {

  
    { "!!!!DIKKAT!!!!",   0,  0,  false,  false, nextlevel },
    { "AYAR MENUSU OK",   0,  0,  false,  false, nextlevel },
    
    
    { "ISI AYAR",   0,  1,  false,  false, isi_ayar },
    { "NEM AYAR",   0,  1,  false,  false, nem_ayar },
    { "KULUCKA SURESI",   0,  1,  false,  false, sure_ayar },
    { "TARIH AYAR ",   0,  1,  false,  false, timelevel },
    { "ZAMAN",   0,  1,  false,  false, saatgoster },
    { "CALISTIR",   0,  1,  false,  false, calisma },
    { "DEVAM ETTIR",   0,  1,  false,  false, devam },
    { "TASARRUF MOD",   0,  1,  false,  false, onoff },
    { "YUKARI DON",   0,  1,  false,  false, endlev },
    { "!!!!DIKKAT!!!!",   0,  1,  false,  false, zero },
    { "  SIFIRLAMA",   0,  1,  false,  false, rootlev }, 
    
    
    { "SET TIME",   0,  2,  false,  false, prevlevel}, //level 4
    { "YIL",   0,  2,  false,  false, set_year },
    { "AY",   0,  2,  false,  false, set_month },
    { "GUN",   0,  2,  false,  false, set_day },
    { "HAFTA GUN",   0,  2,  false,  false, set_weekday },
    { "SAAT",   0,  2,  false,  false, set_hour },
    { "DAKIKA",   0,  2,  false,  false, set_minute },
    { "SAAT ONAY",   0,  2,  false,  false, saat_onay },
    { "GERI",   0,  2,  false,  false, prevlevel },
    { "YUKARI DON",   0,  2,  false,  false, endlev },
    
    { "GOSTERGE",   1,  0,  false,  false, gosterge },
    { "AYAR >",   1,  0,  false,  false, ayar },
    { "PARLAKLIK",   1,  0,  false,  false, set_brightness },
    { "TASARRUF MOD",   1,  0,  false,  false, onoff }

};


void level_recount() {
int i;
int j=0;
//?????? ?????? ??????? ????? ????
for (i=0; i<(sizeof(lmenu)/sizeof(Lmenu)); i++) {
if ((lev0==lmenu[i].level)&&(lev1==lmenu[i].sublevel)) {
if(j==0) {
sel_item=i;
first_item=i;
}
last_item=i;
j++;
}
}
qty=j;
}
void setup() {
pinMode(isi, OUTPUT);
  pinMode(nem, OUTPUT);
  digitalWrite(nem, HIGH);
  digitalWrite(isi, HIGH);
  lev0=EEPROM.read(16);
  myservo.attach(3);
  dht.begin();                       // Isı Sensorunu Baslatiyoruz
  pos=myservo.read();
for (pos=myservo.read(); pos < 90; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(35);                       // waits 15ms for the servo to reach the position
  }
  for (pos=myservo.read(); pos > 90; pos -= 1) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(35);                       // waits 15ms for the servo to reach the position
  }

if(lev0==1){
EEPROM_readAnything(4, isiayar);
nemayar=EEPROM.read(8);
kuluckasuresi=EEPROM.read(12);
}
else if(lev0==0)
{
lev0=0;
lev1=0;
}
myRTC.updateTime();
  // I2C iletişimi icin Wire protokolunu baslatiyoruz
  Wire.begin();                     
  
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

// ?????? ??????: lcd.print("\0");
  analogWrite(10,brightness); //??????? ?? 10-? ???? ??? DFRobot keypad shield 0-255
  lcd.clear();
  level_recount();
  lcd_menu();
  
}
 

byte key() {
//right 5 - 0
//up 3 - 98
//down 4 - 254
//left 2  - 408 (????? 409)
//select 1  - 640 (?????? ? 642)
//rst - RESET
int val=analogRead(0);
if (val<50)  return 5; 
else if (val<150)  return 3; 
else if (val<400)  return 4; 
else if (val<500)  return 2; 
else if (val<800)  return 1; 
else   return 0; 
}

byte ncomm (byte command) {
lcd.setCursor(0,0);
switch (command) {
case 0: 
lcd.print("ISI AYAR"); 
break; 
case 1: 
lcd.print("BRIGHTNESS"); 
break;
case 2: 
lcd.print("YEAR"); 
break;
case 3: 
lcd.print("MONTH"); 
break;
case 4: 
lcd.print("DAY"); 
break;
case 5: 
lcd.print("WEEKDAY"); 
break;
case 6: 
lcd.print("HOUR"); 
break;
case 7: 
lcd.print("MINUTE"); 
break;
case 8: 
lcd.print("NEM AYAR"); 
break;
case 9: 
lcd.print("KULUCKA SURESİ"); 
break;
}  
}
byte scomm (byte command, float val) {
lcd.setCursor(0,0);
switch (command) {
case 0: 
isiayar=val; 
break; 
case 1: 
brightness=val; 
bright(brightness);
break;
case 2: 
year=val; 
break; 
case 3: 
month=val; 
break; 
case 4: 
day=val; 
break; 
case 5: 
weekday=val; 
break; 
case 6: 
hour=val; 
break; 
case 7: 
minute=val; 
break; 
case 8: 
nemayar=val; 
break; 
case 9: 
kuluckasuresi=val; 
break; 
}  
}
float set_var(byte command, float val, float maxlevel, float minlevel, float steps) {
previousMillis = millis();  // ?????????? ??????? ?????
byte s;
lcd.clear();
ncomm(command);
infunc=true;
while(infunc==true) {
if (millis() - previousMillis > interval) {
s=key();
if((s==0)||(s==1))  {
infunc=false;
lcd_menu();
delay (500);
break;
} 
} 
lcd.setCursor(0,1);

if(val<10) {lcd.print(" ");}
if(val<100) {lcd.print(" ");}
if(val<1000) {lcd.print(" ");}
if(val<10000) {lcd.print(" ");}
if(val<100000) {lcd.print(" ");}
lcd.print(val);
delay(200);
switch(key()) {
case 1:
infunc=false;
lcd_menu();
delay (500);
break;
case 4:
 previousMillis = millis();  // ?????????? ??????? ?????
 if( val>minlevel) { val-=steps; }
 scomm(command,val);
 break;
 case 2:
 previousMillis = millis();  // ?????????? ??????? ?????
 if( val>minlevel) { val-=steps; }
 scomm(command,val);
 break;
 case 3: 
 previousMillis = millis();  // ?????????? ??????? ?????
 if( val<maxlevel) { val+=steps; }
 scomm(command,val);
 break;
 case 5: 
 previousMillis = millis();  // ?????????? ??????? ?????
 if( val<maxlevel) { val+=steps; }
 scomm(command,val);
 break;
}
}
return val;
}
void saatgoster(){
  lcd.clear();
saatekran=1;
  while (saatekran==1){
  delay(200);
    switch(key()) {
case 1:
saatekran=0 ;
infunc=false;
lcd_menu();
delay(150);
break;
default:

lcd.clear();
myRTC.updateTime();
 lcd.setCursor(0, 0);
  lcd.print("GUN :");
   lcd.print(myRTC.dayofmonth);
  lcd.print("SAAT: ");
  lcd.print(myRTC.hours);
  lcd.setCursor(0, 1);
   lcd.print("DK: ");
     lcd.print(myRTC.minutes);
    lcd.print("SN: ");
     lcd.print(myRTC.seconds);
myRTC.updateTime();
    
    loop();
   

}

  }

  
  

}
void saat_onay (){
    myRTC.updateTime();
 int minutes=minute;
  int hours=hour;
   int weekdays=weekday;
    int days=day;
     int months=month;
      int years=year;
// seconds, minutes, hours, day of the week, day of the month, month, year
myRTC.setDS1302Time(10, minutes, hours, weekdays, days, months, years);
level_recount();
delay(200);
infunc=false;
lcd_menu();
return;
}
void gosterge(){ lcd.clear();
ekran=1;
  while (ekran==1){
  delay(200);
    switch(key()) {
case 1:
ekran=0 ;
infunc=false;
lcd_menu();
delay(150);
break;
default:
   lcd.setCursor(0, 0);
    
    lcd.print(isiolcum); 
    lcd.print("c");
    lcd.setCursor(7, 0);
    lcd.print("%");
   lcd.print(nemolcum); 
   lcd.setCursor(0, 1);
   lcd.print("KALAN GUN: ");
   lcd.print(kalangun);
    loop();
   

}

  }
 
  
}
void ayar() {
lcd.clear();
infunc=true;
lev0=0;
lev1=0;
EEPROM.write(16,lev0);
level_recount();
delay(200);
infunc=false;
lcd_menu();
return;
}
void calisma() {
EEPROM_writeAnything(4, isiayar);
EEPROM.write(8,nemayar);
EEPROM.write(12,kuluckasuresi);
lcd.clear();
myRTC.setDS1302Time(10, 1, 1, 1, 1, 1, 2000);
infunc=true;
lev0=1;
lev1=0;
EEPROM.write(16,lev0);
level_recount();
delay(200);
infunc=false;
lcd_menu();
return;
  

}
void devam(){
  lcd.clear();
  infunc=true;
  lev0=1;
  lev1=0;
  EEPROM.write(16,lev0);
  level_recount();
delay(200);
infunc=false;
lcd_menu();
return;
}
void isi_ayar() {
set_var(0, isiayar, 42, 25, 0.1);
}
void set_brightness() {
set_var(1, brightness, 255, 0, 15);
}
void set_year() {
set_var(2, year, 2100, 2000, 1);
}
void set_month() {
set_var(3, month, 12, 1, 1);
}
void set_day() {
set_var(4, day, 31, 1, 1);
}
void set_weekday() {
set_var(5, weekday, 7, 1, 1);
}
void set_hour() {
set_var(6, hour, 23, 0, 1);
}
void set_minute() {
set_var(7, minute, 59, 0, 1);
}
void nem_ayar() {
set_var(8, nemayar, 95, 35, 2);
}
void sure_ayar() {
set_var(9, kuluckasuresi, 30, 1, 1);
}
void onoff() {
if(brightness<30){ brightness=255; } else { brightness=0; }
analogWrite(10,brightness);
delay(500);
}
void bright(float val) {
analogWrite(10,val);
}


void zero() {
//empty fuction
return;
}
void endlev() {
lcd.clear();
infunc=true;
level_recount();
sel_item=first_item;
delay(200);
infunc=false;
lcd_menu();
return;
}
void nextlevel() {
lcd.clear();
infunc=true;
lev1++;
level_recount();
delay(200);
infunc=false;
lcd_menu();
return;
}
void timelevel() {
lcd.clear();
infunc=true;
lev1=2;
level_recount();
delay(200);
infunc=false;
lcd_menu();
return;
}
void prevlevel() {
lcd.clear();
infunc=true;
if(lev1>=1) { lev1--; }
level_recount();
delay(200);
infunc=false;
lcd_menu();
return;
}

void rootlev() {
lcd.clear();
infunc=true;
lev0=0;
lev1=0;
EEPROM.put(16,0);
sel_item=0;
level_recount();
delay(200); 
infunc=false;
lcd_menu();
return;
}

void loop() {
  myRTC.updateTime();
  nemolcum = dht.readHumidity();
isiolcum = dht.readTemperature();

if(lev0==1){
 EEPROM_readAnything(4, isiayar);
nemayar=EEPROM.read(8);
kuluckasuresi=EEPROM.read(12);
}
else if(lev0==0)
{

}
  
kalangun=kuluckasuresi-myRTC.dayofmonth;
  

if(nemolcum-2>nemayar){
  digitalWrite(nem, HIGH);
  
}
 else if(nemolcum+2<nemayar){
  digitalWrite(nem, LOW);
  
}

if(isiolcum-0.2>isiayar){
  digitalWrite(isi, HIGH);
  
}
 else if(isiolcum+0.1<isiayar){
  digitalWrite(isi, LOW);
  
}
myRTC.updateTime();
dakkalan=myRTC.minutes%59;
kalan=myRTC.hours%2;
myRTC.updateTime();
if (dakkalan==1)
{
if (kalan==1){
  myservo.attach(3);
   pos=myservo.read();
for (pos=myservo.read(); pos < 68; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(35);                       // waits 15ms for the servo to reach the position
  }
  for (pos=myservo.read(); pos > 70; pos -= 1) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(35);                       // waits 15ms for the servo to reach the position
  }
 
}
else if (kalan==0){
  myservo.attach(3);
   pos=myservo.read();
for (pos=myservo.read(); pos < 110; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(45);                       // waits 15ms for the servo to reach the position
  }
  for (pos=myservo.read(); pos > 112; pos -= 1) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(45);                       // waits 15ms for the servo to reach the position
  }
  
}}
else if (dakkalan=!1){myservo.detach();}

//show menu
if (millis() - previousMillis > interval) {
previousMillis = millis();  // ?????????? ??????? ?????
mstate=true;
if(infunc==false) {

}
}
if(clr==false) {
switch(key()) {
  
case 0: //??? ???????
clr==false;
break;

case 1: //Select
if(infunc==false) {
run_menu();
} else {
infunc=false;
lcd_menu();
delay(1000);
}
break;


break;

case 3: //Up
if(infunc==false) {
sel_item--;
if(sel_item<first_item) { sel_item=first_item; }
clr=true;
}
break;

case 4: //Down
if(infunc==false) { 
if(sel_item<last_item) {  
sel_item++; 
}
clr=true;
}
break;

case 5: //right

break;
}

} else {
if(infunc!=true) {
if(key()==0) {
lcd_menu();
}
}
}


}



void lcd_menu() {
mstate=false;
infunc=false;
int i;
int j;
int row=0;
clr=false;
for (i=first_item; i<=last_item; i++) {
if ((i>=sel_item)&&(row<mrows)&&(lev0==lmenu[i].level)&&(lev1==lmenu[i].sublevel)) {
lcd.setCursor(0, row);
if(i==sel_item) {
lcd.print ((char)menu_sym); 
} else {
lcd.print(" "); 
}
//sprintf(nav," %d/%d",(i+1),qty);
int procent=ceil((sel_item-first_item)*100/qty);
char* item=lmenu[i].itemname;

//??????? ????????
if(strlen(item)>(mcols-2)) {mstate=true;}
for(j=0; j<strlen(item); j++) { 
if(j<(mcols-2)) {
lcd.print(item[j]);
}
}
//????????? ?????? ?? ?????
for(j=(strlen(item)+1); j<(mcols-1); j++) { 
lcd.print(" ");
}



row++;
}
}

//??????? ????????? ??????
if(row<=(mrows-1)) {
for(j=row; j<mcols; j++) {
lcd.setCursor(0, j);  
for(i=0; i<=mcols; i++) {
lcd.print(" ");
}
}
}
}

void run_menu() {
int j;
int i;
lmenu[sel_item].function();
}


void str_animate() {
int j;
int i;
if (infunc==false) {
char* item=lmenu[sel_item].itemname;
if((mstate==true)&&(strlen(item)>(mcols-2))) {
for(i=0; i<(strlen(item)+1); i++) { 
lcd.setCursor(1, 0); 
for(j=i; j<(strlen(item)+i); j++) { 
if((j<(mcols-2+i))&&(j<strlen(item))) {
lcd.print(item[j]);
if (millis() - previousMillis2 > interval2) {
previousMillis2 = millis();  // ?????????? ??????? ?????
delay(interval2);
int val=analogRead(0);
if(val<1000) {mstate==false; return;}
}
} else {
if(j<(mcols-2+i)) {
lcd.print(" "); 
} 
}
}
}
//?????????? ?????? ?????
mstate=false;
lcd.setCursor(1, 0);
for(j=0; j<strlen(item); j++) { 
if(j<(mcols-2)) {
lcd.print(item[j]);
}
}
} else {
lcd.setCursor(1, 0);
for(j=0; j<strlen(item); j++) { 
if(j<(mcols-2)) {
lcd.print(item[j]);
}
}
mstate=false;
return;
}
} else {
 if (millis() - previousMillis > interval) {
previousMillis = millis();  // ?????????? ??????? ?????
delay(interval);
infunc=false;
lcd_menu();
 } 
}
}
template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
    const byte* p = (const byte*)(const void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
          EEPROM.write(ee++, *p++);
    return i;
}

template <class T> int EEPROM_readAnything(int ee, T& value)
{
    byte* p = (byte*)(void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
          *p++ = EEPROM.read(ee++);
    return i;
}
