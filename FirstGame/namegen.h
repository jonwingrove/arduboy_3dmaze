#include <stdint.h>
#include <stdlib.h>

#define NAMELEN 10

//32bit seed to dungeony sounding name

const char dblcon[8] PROGMEM = {'T', 'H', 'G', 'R', 'K', 'H', 'Z', 'H'};
const char sngcon[8] PROGMEM = {'M', 'N', 'K', 'G', 'D', 'R', 'X', 'B'};
const char sngvwl[6] PROGMEM = {'A', 'E', 'I', 'O', 'U', '\''};

void getname(uint32_t seed, char* namebuf) {

  uint8_t roll, t1, t2;
  uint8_t last=0;
  size_t i=0;
  
  while(i<NAMELEN-2) {
    roll = seed >> i ^ 93;
    if(last=0) {t1=180; t2=220;}
    else if(last=1) {t1=60; t2=120;}
    else {t1=40; t2=80;}
    if(roll < t1) {
      namebuf[i++]=pgm_read_byte_near(dblcon+roll%4);  
      namebuf[i++]=pgm_read_byte_near(dblcon+roll%4+1);  
      last=2;
    } else if (roll < t2) {
      namebuf[i++]=pgm_read_byte_near(sngcon+roll%8);  
      last=1;
    } else {
      namebuf[i++]=pgm_read_byte_near(sngvwl+roll%6);  
      last=0;
    }
  }
  while(i<NAMELEN) {
    if(i<NAMELEN-1)
    namebuf[i++]=' ';
    else namebuf[i++]=NULL;
  }
}
