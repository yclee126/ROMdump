/*
 * ROM dump program by yclee126
 * Original code from TeensySdioDemo by greiman 
 * SdFatSdioEX is used for accessing SD card.
 * 
 * More info on https://github.com/yclee126/ROMdump
 * 
 */


#include "SdFat.h"
// uncomment below line to dump in word mode
//#define WORDMODE

// 1 Mb (125 KB) buffer
const size_t BUF_DIM = 125000;

// 32 Mb (4000 KB) file
const uint32_t FILE_SIZE = 32UL * BUF_DIM;

SdFatSdioEX sdEx;

File file;

uint8_t buf[BUF_DIM];

void errorHalt(const char* msg) {
  sdEx.errorHalt(msg);
}
//-----------------------------------------------------------------------------
void startDump(uint8_t* romSizeMb) {
  if (!file.open("ROM.hex", O_WRITE | O_CREAT)) {
    errorHalt("open failed");
  }
  file.truncate(0);

  uint32_t t = micros();
  uint32_t adr = 0;
  for (uint8_t n = 0; n < *romSizeMb; n++) {// count by 1Mb

    for (size_t a = 0; a < BUF_DIM; a++) {// count by 1byte
      adrWrite(&adr);
      delayMicroseconds(1);
      
#ifdef WORDMODE
      if(adr%1 == 0){
#endif
        for (uint8_t i = 16; i < 24; i ++) { // count by 1bit
          bitWrite(buf[a], i-16, digitalRead(i));
        }
#ifdef WORDMODE
      }
      else{
        for (uint8_t i = 33; i < 40; i ++) {
          bitWrite(buf[a], i-33, digitalRead(i));
        }
        bitWrite(buf[a], 7, digitalRead(14));
      }
#endif
      adr ++;
      
    }
    
    if (BUF_DIM != file.write(buf, BUF_DIM)) {
      errorHalt("write failed");
    }
    Serial.print(n+1);
    Serial.print(" / ");
    Serial.println(*romSizeMb);
  }

  t = micros() - t;
  Serial.print("Run time (sec): ");
  Serial.println(t / 1000000.0);

  file.close();
  Serial.println("");
  Serial.println("Operation Finished!");
  while (1);
}

void adrWrite(uint32_t* adr){

  for (uint8_t i = 0; i < 13; i ++) {
    digitalWrite(i, bitRead(*adr, i));
  }
  for (uint8_t i = 24; i < 33; i ++) {
    digitalWrite(i, bitRead(*adr, i-11));
  }
  
}
//-----------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);

  //Address output pins
  for (uint8_t i = 0; i < 13; i ++) {
    pinMode(i, OUTPUT);
  }
  for (uint8_t i = 24; i < 33; i ++) {
    pinMode(i, OUTPUT);
  }

  //Data input pins
  for (uint8_t i = 16; i < 24; i ++) {
    pinMode(i, INPUT);
  }

#ifdef WORDMODE
  //Word mode Data input pins
  pinMode(14, INPUT);
  pinMode(39, INPUT);
  pinMode(38, INPUT);
  pinMode(37, INPUT);
  pinMode(36, INPUT);
  pinMode(35, INPUT);
  pinMode(34, INPUT);
  pinMode(33, INPUT);
#endif

  while(!Serial);

}
//-----------------------------------------------------------------------------
void loop() {
  do {
    delay(10);
  } while (Serial.available() && Serial.read());
  
  Serial.println("Type your ROM size in Mbit.");
  Serial.println("");
  while (!Serial.available()) {
  }
  uint8_t romSizeMb = 0;
  uint8_t d1 = Serial.read();
  uint8_t d2 = Serial.read();
  uint8_t d3 = Serial.read();
  if(d2 == 255){
    romSizeMb += d1-48;
  }
  else if(d3 == 255){
    romSizeMb += d2-48 + (d1-48)*10;
  }
  else{
    romSizeMb += d3-48 + (d2-48)*10 + (d1-48)*100;
  }
  Serial.print("The ROM size is ");
  Serial.print(romSizeMb);
  Serial.print("Mb (");
  uint32_t inByte = 125000UL*romSizeMb;
  Serial.print(inByte);
  Serial.println("byte)");

  if (!sdEx.begin()) {
    sdEx.initErrorHalt("SdFatSdioEX begin() failed");
  }
  // make sdEx the current volume.
  sdEx.chvol();

  Serial.println("Dump start");
  startDump(&romSizeMb);
}
