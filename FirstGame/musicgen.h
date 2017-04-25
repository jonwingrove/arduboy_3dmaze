#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define BUFMAX 256 //allocated for final output
#define SUBMAX 128  //allocated per sub-theme, can be disposed of after generation


typedef bool boolean;

const uint8_t modes[5][7] = {
  {0, 2, 3, 5, 7, 9, 10},
  {0, 1, 3, 5, 7, 8, 10},
  {0, 2, 3, 5, 7, 9, 10},
  {0, 1, 4, 5, 7, 8, 10},
  {0, 1, 4, 5, 7, 8, 11}
};

const uint8_t notedurs[6] = {1, 2, 3, 4, 6, 8};

void writerest(uint8_t* buf, uint16_t* ptr, uint16_t dur) {
  buf[(*ptr)++]=(uint8_t)((dur >> 8) & 0x7f);
  buf[(*ptr)++]=(uint8_t)(dur & 0xff);
}

void writenoteon(uint8_t* buf, uint16_t* ptr, uint8_t note, uint8_t chan) {
  buf[(*ptr)++]=(uint8_t)(0x90 | (chan & 0x0f));
  buf[(*ptr)++]=note;
}

void writenoteoff(uint8_t* buf, uint16_t* ptr, uint8_t chan) {
  buf[(*ptr)++]=(uint8_t)(0x80 | (chan & 0x0f));
}

uint16_t getrest(uint8_t* buf, uint16_t* ptr) {
  //if ptr points to a rest, return the duration of that rest and 
  //any rests immediately after, advancing the pointer
  //if not, return zero

  uint16_t total=0, dur;
  while((buf[*ptr] & 0x80) == 0x00) { //rest
    dur = (buf[(*ptr)++] & 0x7f) << 8;
    dur |= buf[(*ptr)++];
    if(dur==0) return 0;  //stop overrun
    total += dur;
  }
  return total;
}

bool copynotes(uint8_t* buf_s, uint16_t* ptr_s, uint8_t* buf_d, uint16_t* ptr_d, int8_t trnsp) {
  //if ptr_s points to a rest, return false
  //if not, copy events from buf_s to buf_d, advancing pointers,  
  //until you hit a rest, then return true

  if((buf_s[*ptr_s] & 0x80) == 0x00) return false; //rest

  do {
    if((buf_s[*ptr_s] & 0xf0) == 0x90) { //noteon, 2 bytes
      buf_d[(*ptr_d)++] = buf_s[(*ptr_s)++];
      buf_d[(*ptr_d)++] = buf_s[(*ptr_s)++] + trnsp;
    } else if((buf_s[*ptr_s] & 0xf0) == 0x80) { //noteoff, 1 byte
      buf_d[(*ptr_d)++] = buf_s[(*ptr_s)++];
    } //loop/stop events shouldn't be fed into mixdown; doesn't make sense

  } while((buf_s[*ptr_s] & 0x80) == 0x80); //anything but a rest
  return true;
}


uint16_t mixdown(uint8_t* buf_a, uint8_t len_a, int8_t trnsp_a, uint16_t delay_a,
    uint8_t* buf_b, uint8_t len_b, int8_t trnsp_b, uint16_t delay_b,
    uint8_t* buf_out) {

  uint16_t ptr_a=0, ptr_b=0, ptr_out=0;
  uint16_t dur_a=0, dur_b=0;
  dur_a=delay_a+getrest(buf_a, &ptr_a);
  dur_b=delay_b+getrest(buf_b, &ptr_b);

  while(ptr_a < len_a || ptr_b < len_b) {
    if(dur_a == dur_b) {
      //write one rest (or not if both zero) then events, done
      if(dur_a>0) writerest(buf_out, &ptr_out, dur_a);
      copynotes(buf_a, &ptr_a, buf_out, &ptr_out, trnsp_a);
      copynotes(buf_b, &ptr_b, buf_out, &ptr_out, trnsp_b);
      dur_a=getrest(buf_a, &ptr_a);
      dur_b=getrest(buf_b, &ptr_b);
    } else if (dur_a < dur_b) {
      //write dur_a rest and subtract dur_a from dur_b
      if(dur_a > 0) {
        writerest(buf_out, &ptr_out, dur_a);
        dur_b -= dur_a;
      }
      //write events from a
      copynotes(buf_a, &ptr_a, buf_out, &ptr_out, trnsp_a);
      //get next rest from a and continue->recompare durations
      dur_a=getrest(buf_a, &ptr_a);

    } else if (dur_a > dur_b) {
      //opposite of ^
      if(dur_b > 0) {
        writerest(buf_out, &ptr_out, dur_b);
        dur_a -= dur_b;
      }
      copynotes(buf_b, &ptr_b, buf_out, &ptr_out, trnsp_b);
      dur_b=getrest(buf_b, &ptr_b);
    }
  }

  return ptr_out;
}

void generateTheme(uint8_t* buf)
{
  uint8_t theme[SUBMAX];
  uint8_t bass[SUBMAX];

  uint32_t seed;
  time_t t;
  uint16_t i, ptr, dur_theme, len_theme, len_bass;
  uint8_t r, note, key, mode, semiq, dur;

  seed = 1;

  //srand(seed);

  for(i=0;i<BUFMAX;i++) {
    buf[i]=0x00;
  }
  for(i=0;i<SUBMAX;i++) {
    theme[i]=0x00;
    bass[i]=0x00;
  }

  ptr=0;
  dur_theme=0;
  mode=random()%5;
  key=random()%12;
  semiq=random()%128+128;

  //THEME

  //count off
  //writerest(theme, &ptr, (uint16_t)semiq * 8);

  //generate theme
  note = 12 + 2 * (random() % 3);
  dur = notedurs[random() % 6];


  while(dur_theme < 64) {
    writenoteon(theme, &ptr, 60 + key + modes[mode][note%12], 0);
    writerest(theme, &ptr, (uint16_t)semiq * dur);
    dur_theme += dur;
    r = random()%14;
    if(r<4) note--;
    else if(r<8) note++;
    else if(r<10) note-=2;
    else if(r<12) note+=2;
    else if(r==12) note-=3;
    else if(r==13) note+=3;
    r = random()%3;
    if(r==0) dur = notedurs[random() % 6];
    else if(r==1) dur = 4 - (dur_theme % 4);
    //else if r==2 keep the same duration value
  }

  //end
  writenoteoff(theme, &ptr, 0);
  //writerest(theme, &ptr, (uint16_t)semiq * 8);

  len_theme=ptr;

  //BASS

  ptr=0;

  //count off
  //writerest(bass, &ptr, (uint16_t)semiq * 8);

  //placeholder bass
  for(i=0;i<16;i++) {
    writenoteon(bass, &ptr, 48+key, 1);
    writerest(bass, &ptr, semiq * 2);
    writenoteoff(bass, &ptr, 1);
    writerest(bass, &ptr, semiq * 2);
  }

  //end
  //writerest(theme, &ptr, (uint16_t)semiq * 8);

  len_bass=ptr;

  ptr = mixdown(theme, len_theme, 0, 0, bass, len_bass, 0, 0, buf);
  //loop
  buf[ptr++] = 0xe0;
}

