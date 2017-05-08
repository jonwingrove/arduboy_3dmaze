#define FIXMATH_NO_ROUNDING
#define FIXMATH_NO_OVERFLOW

#include <Arduboy2.h>
#include <avr/pgmspace.h>
#include "fix16.h"
#include "mazegen.h"
#include "vector.h"
#include "lookup.h"
#include "musicgen.h"
#include "namegen.h"
#include "ArduboyPlaytune.h"
#include "GameObject.h"

#define TEX_MAN 0
#define TEX_WALL 1
#define TEX_DOOR 2
#define TEX_IRON 3


Arduboy2 arduboy;

const byte textures[256] PROGMEM = {
    //TEX_MAN
0x00, 0x40, 0x01, 0x00, 
0x00, 0x45, 0x51, 0x00, 
0x00, 0x55, 0x55, 0x00, 
0x00, 0x55, 0x55, 0x00, 
0x00, 0x1a, 0xa4, 0x00, 
0x00, 0x15, 0x54, 0x00, 
0x00, 0x16, 0x94, 0x00, 
0x00, 0x16, 0x90, 0x00, 
0x00, 0x05, 0x50, 0x00, 
0x00, 0x15, 0x54, 0x00, 
0x00, 0x45, 0x51, 0x00, 
0x01, 0x45, 0x51, 0x40, 
0x01, 0x45, 0x51, 0x40, 
0x00, 0x04, 0x10, 0x00, 
0x00, 0x04, 0x10, 0x00, 
0x00, 0x14, 0x14, 0x00, 
  
    //TEX_WALL
    0x15, 0x56, 0x15, 0x56,
    0x15, 0x56, 0x15, 0x56,
    0x2a, 0xaa, 0x2a, 0xaa,
    0x00, 0x00, 0x00, 0x00,    
    0x56, 0x15, 0x56, 0x15,
    0x56, 0x15, 0x56, 0x15,
    0xaa, 0x2a, 0xaa, 0x2a,    
    0x00, 0x00, 0x00, 0x00,    
    0x15, 0x56, 0x15, 0x56,
    0x15, 0x56, 0x15, 0x56,
    0x2a, 0xaa, 0x2a, 0xaa,
    0x00, 0x00, 0x00, 0x00,    
    0x56, 0x15, 0x56, 0x15,
    0x56, 0x15, 0x56, 0x15,
    0x56, 0x15, 0x56, 0x15,
    0xaa, 0x2a, 0xaa, 0x2a,

    //TEX_DOOR
    0x55, 0x55, 0x55, 0x56,
    0x2a, 0xaa, 0xaa, 0xa8,
    0x66, 0xaa, 0xaa, 0x68,
    0x68, 0xaa, 0xaa, 0x88,
    0x2a, 0xaa, 0xaa, 0xa8,
    0x2a, 0xa0, 0x0a, 0xa8,
    0x2a, 0x80, 0x02, 0xa8,
    0x2a, 0x00, 0x00, 0xa8,
    0x2a, 0x00, 0x00, 0xa8,
    0x68, 0x00, 0x00, 0x28,
    0x68, 0x00, 0x00, 0x28,
    0x68, 0x00, 0x00, 0x28,
    0x68, 0x00, 0x00, 0x28,
    0x68, 0x00, 0x00, 0x28,
    0x68, 0x00, 0x00, 0x28,
    0x68, 0x00, 0x00, 0x28,


    //TEX_IRON
    0x55, 0x55, 0x55, 0x56,
    0x2a, 0xaa, 0xaa, 0xa8,
    0x66, 0xaa, 0xaa, 0x68,
    0x68, 0xaa, 0xaa, 0x88,
    0x2a, 0xaa, 0xaa, 0xa8,
    0x2a, 0xaa, 0xaa, 0xa8,
    0x2a, 0xaa, 0xaa, 0xa8,
    0x2a, 0xaa, 0xaa, 0xa8,
    0x2a, 0xaa, 0xaa, 0xa8,
    0x2a, 0xaa, 0xaa, 0xa8,
    0x2a, 0xaa, 0xaa, 0xa8,
    0x2a, 0xaa, 0xaa, 0xa8,
    0x66, 0xaa, 0xaa, 0x68,
    0x68, 0xaa, 0xaa, 0x88,
    0x2a, 0xaa, 0xaa, 0xa8,
    0x2a, 0xaa, 0xaa, 0xa8
   
    
};

#define MAX_GAMEOBJECTS 10

GameObject m_spriteObjects[MAX_GAMEOBJECTS];

#define MAX_DIST F16(9999);
uint8_t s_musicBuffer[BUFMAX];

class HitResult
{
  public:
    fix16_t m_finalHitDistance;
    fix16_t m_hitX;
    fix16_t m_hitY;
    bool m_isVertical;
    bool m_hit;
};

class VecStep
{
  public:
    fix16_t m_x;
    fix16_t m_y;
    fix16_t m_lengthSquared;

    fix16_t m_riseDivRunn;
    bool m_inverted;
    fix16_t m_runn;

    void reset(fix16_t rise, fix16_t runn, fix16_t x, fix16_t y, bool inverted)
    {
      if (runn == 0)
      {
        m_runn = 0;
        m_lengthSquared = MAX_DIST;
      }
      else
      {
        m_runn = runn;
        m_riseDivRunn = fix16_div(rise, runn);
        m_inverted = inverted;
        update(x,y);        
      }
    }

    void update(fix16_t x, fix16_t y)
    {
        if(m_runn != 0)
        {
          fix16_t dx = m_runn > 0 ? 
          fix16_sub(fix16_floor(fix16_add(x, fix16_one)), x) : 
          fix16_sub(fix16_ceil(fix16_sub(x, fix16_one)), x);
          
          fix16_t dy = fix16_mul(dx, m_riseDivRunn);
  
          m_x = m_inverted ? fix16_add(y, dy) : fix16_add(x, dx);
          m_y = m_inverted ? fix16_add(x, dx) : fix16_add(y, dy);
          m_lengthSquared = fix16_add(fix16_mul(dx, dx), fix16_mul(dy, dy));
        }
    }
};

byte get_tex(size_t id, int u, int v) {
  byte dat = pgm_read_byte_near(textures + (id*64) + (v*4) + (u/4));
  switch(u%4) {
    case 0: return (dat >> 6) & 0x03;
    case 1: return (dat >> 4) & 0x03;
    case 2: return (dat >> 2) & 0x03;
    case 3: return dat & 0x03;
  }  
}


fix16_t fastSin(int angDegrees)
{
  if(angDegrees < 0)
  {
    angDegrees+=360;
  }
  if(angDegrees > 360)
  {
    angDegrees-=360;
  }
  if(angDegrees > 180)
  {
    return fix16_sub(0,pgm_read_word_near(sinTab +(angDegrees)%180));
  }
  else
  {
    return pgm_read_word_near(sinTab +(angDegrees)%180);
  }
}

fix16_t fastCos(int angDegrees)
{
  return fastSin(angDegrees+90);
}

fix16_t slowSqrt(fix16_t val)
{
  float v = fix16_to_float(val);
  float sqr = sqrt(v);
  return fix16_from_float(sqr);
}


int getMapI(int ix, int iy)
{
  if (ix < 0 || iy < 0 || ix >= MAPW || iy >= MAPH)
  {
    return 1;
  }
  else
  {
    return get_map(ix,iy,map_0);//  //pgm_read_byte_near(maze +(ix + (iy * mazeSize)));
  }
}

int getMap(fix16_t x, fix16_t y)
{
  int ix = fix16_to_int(fix16_floor(x));
  int iy = fix16_to_int(fix16_floor(y));
  return getMapI(ix,iy);
}

VecStep stepX;
VecStep stepY;
void castRay(Vec2* pos, int angle, fix16_t range, HitResult* hr)
{
  fix16_t sinA = fastSin(angle);
  fix16_t cosA = fastCos(angle);

  fix16_t distance = 0;
  fix16_t x = pos->m_x;
  fix16_t y = pos->m_y;

  stepX.reset(sinA, cosA, x, y, false);
  stepY.reset(cosA, sinA, y, x, true);

  while (true)
  {

    if(stepX.m_lengthSquared < stepY.m_lengthSquared)
    {
      // this is 'coz at 1,0 going left, we're really looking to see what is in 0,0- looking at edges...
      fix16_t dx = cosA < 0 ? fix16_one : 0;

      fix16_t mapX = fix16_sub(stepX.m_x, dx);
      fix16_t mapY = stepX.m_y;

      distance = fix16_add(distance, fix16_sqrt(stepX.m_lengthSquared));

      int mapAtPos = getMap(mapX, mapY);
      if(mapAtPos == 1)
      {
        hr->m_hitX = mapX;
        hr->m_hitY = mapY;
        hr->m_isVertical = true;
        hr->m_finalHitDistance = distance;
        hr->m_hit = true;
        return;
      }

      x = stepX.m_x;
      y = stepX.m_y;
    }
    else
    {
      fix16_t dy = sinA < 0 ? fix16_one : 0;

      fix16_t mapX = stepY.m_x;
      fix16_t mapY = fix16_sub(stepY.m_y, dy);

      distance = fix16_add(distance, fix16_sqrt(stepY.m_lengthSquared));

      int mapAtPos = getMap(mapX, mapY);
      if(mapAtPos == 1)
      {
        hr->m_hitX = mapX;
        hr->m_hitY = mapY;
        hr->m_isVertical = false;
        hr->m_finalHitDistance = distance;
        hr->m_hit = true;
        return;
      }

      x = stepY.m_x;
      y = stepY.m_y;
    }

    stepX.update(x,y);
    stepY.update(y,x);

    if (distance > range)
    {
      hr->m_hit = false;
      return;
    }
  }
}

Vec2 m_playerPos = Vec2(1.3f,1.4f);
int m_playerAngDegrees = 0;
uint32_t seed;
char worldname[NAMELEN];

ArduboyPlaytune m_musicPlayer;

void setup() {
  // put your setup code here, to run once:
  arduboy.begin();
  arduboy.setFrameRate(30);
  arduboy.initRandomSeed();

  seed = random();
  m_playerPos = Vec2(fix16_from_float(1.3f),fix16_from_float(1.4f));
  resetMazeGen(seed);
  resetMusicGen(seed);
  getname(seed, worldname);
  for(int i = 0; i < MAX_GAMEOBJECTS; ++i)
  {
    memset(&m_spriteObjects[i], 0, sizeof(GameObject));
  }
  m_spriteObjects[0].m_type = 1;
  m_spriteObjects[0].m_position = Vec2(fix16_from_float(3.3f),fix16_from_float(3.4f));  

    m_spriteObjects[1].m_type = 1;
  m_spriteObjects[1].m_position = Vec2(fix16_from_float(4.7f),fix16_from_float(2.8f));  
}

int i = 0;

void drawShadedPixel(int x, int y, int colM)
{
  if(colM == 0)
  {
    arduboy.drawPixel(x,y,0);
  }
  else if(colM != 0 && (x+y)%colM == 0)
  {
    arduboy.drawPixel(x,y,1);
  }
  else
  {
    arduboy.drawPixel(x,y,0);
  }
}

void fillRectShaded(int x, int y, int w, int h)
{  
  for(int dy = y; dy < y+h; ++dy)
  {
    for(int dx = x; dx < x+w; ++dx)
    {
      arduboy.drawPixel(dx,dy,(dx+dy)%2==0);
    }
  }
}

void maingame();
void testSin();
boolean mapGenerated = false;
boolean musicGenerated = false;
int loading = 32;

void loop() {
  if (!(arduboy.nextFrame()))
  {
    return;
  }

  arduboy.clear();

  if(!mapGenerated)
  {
    arduboy.setCursor(0,0);
    
    mapGenerated = genMap();    
    drawSprite(loading,16,128);
    loading++;
    if(loading > 96)
    {
      loading=32;
    }
    arduboy.print("DUNGEON OF ");
    arduboy.print(worldname);
  }
  else
  { 
    if(!musicGenerated)
    {
      musicGenerated = true;
      generateTheme(s_musicBuffer);
      m_musicPlayer.initChannel(PIN_SPEAKER_1);
      m_musicPlayer.initChannel(PIN_SPEAKER_2);
      m_musicPlayer.playScore(s_musicBuffer);
    }
    maingame();
  }
  //testSin();

    arduboy.display();
}

void testSin()
{
  fix16_t neg1 = 0xFFFF0000;//fix16_from_int(-1);
  fix16_t zero = 0;
  for(int x = 0; x < 128; ++x)
  {
      //arduboy.drawPixel(x,16+fix16_to_int(fastCos(x*2)*16),1);
      arduboy.drawPixel(x,32-fix16_to_int(fastSin(x*4)*24),1);
      arduboy.drawPixel(x,32-fix16_to_int(neg1*24),1);
      arduboy.drawPixel(x,32-fix16_to_int(zero*24),1);
  }
}

int depths[128];

void drawSprite(int xPos, int ssize, int rClip)
{
  int startX = xPos-ssize;
  int endX = xPos+ssize;
  int startY = 32-ssize;
  int endY = 32+ssize;

  if(endX < 0)
  {
    return;
  }
  if(startX >= rClip)
  {
    return;
  }

  fix16_t incPerPix = fix16_div(F16(16), (ssize*2*fix16_one));
  fix16_t startU = 0;
  fix16_t u = 0;
  fix16_t v = 0;

  if(startX < 0)
  {
    startU = incPerPix * -startX;
    startX = 0;
  }
  if(endX > rClip)
  {
    endX = rClip;
  }
  if(endY > 96)
  {
    endY = 96;
  }
  if(startY < 0)
  {
    v += incPerPix * -startY;
    startY = 0;
  }

  int startUi = fix16_to_int(fix16_floor(startU));
    
  for(int y = startY; y < endY; )
  {
    int vi = fix16_to_int(fix16_floor(v));

    int pixelsToNext = max(fix16_to_int(fix16_div(fix16_sub(fix16_ceil(v),v),incPerPix)),1);
    
    u = startU;
    int ndx = 0;
    int ndui = startUi;
    for(int x = startX; x < endX; ++x)
    {      
      int ui = fix16_to_int(fix16_floor(u));
      bool thisPixelVisible = ssize >= depths[x];
      int drawTo = thisPixelVisible ? x : x - 1;
      bool shouldDraw = thisPixelVisible ? (ui != ndui || x == endX-1) : (ndx != x);
      if(shouldDraw)
      {             
        if(ui < 16 && vi < 16)
        {

          //int pixCol = pgm_read_byte_near(sprite + (ui+vi*16));
          int pixCol = get_tex(TEX_MAN, ui, vi);
          if(pixCol == 1)
          {
            arduboy.fillRect(ndx,y,(drawTo-ndx)+1,pixelsToNext,1);
          }
          else if(pixCol == -1)
          {
            arduboy.fillRect(ndx,y,(drawTo-ndx)+1,pixelsToNext,0);
          }
          else if(pixCol != 0)
          {
            fillRectShaded(ndx,y,(drawTo-ndx)+1,pixelsToNext);
          }
        }

        ndui = ui;
        ndx=x+1;
      }
      else if(!thisPixelVisible)
      {
        ndx=x+1;
      }
      u = fix16_add(u,incPerPix);
    }
    
    v = fix16_add(v,incPerPix*pixelsToNext);
    y+=pixelsToNext;
  }
}


void drawShadedBox(int x1, int y1, int x2, int y2, int shade)
{
  if(shade <= 1)
  {
    arduboy.fillRect(x1, y1, x2-x1, y2-y1, 1);
  }
  else
  {
    for(int dx=x1; dx < x2; ++dx)
    {
      arduboy.drawPixel(dx,y1,1);
      arduboy.drawPixel(dx,y2,1);
      for(int dy=y1+(dx%shade);dy<y2; dy+=shade)
      {
        arduboy.drawPixel(dx,dy,1);
      }
    }    
  }
}

void drawWallSlice(int x1, int y1, int x2, int y2, fix16_t u, int shade, HitResult* hit)
{
  fix16_t incPerPix = fix16_div(F16(16), ((y2-y1)*fix16_one));
  fix16_t v = 0;
  fix16_t startV = 0;
  char tid = TEX_WALL;
  int hx = fix16_to_int(fix16_floor(hit->m_hitX));
  int hy = fix16_to_int(fix16_floor(hit->m_hitY));
  
  if(hx==(MAPW-3)&&hy==(MAPH-1)) tid = TEX_DOOR;
  else if(hx==0||hy==0||hx==MAPW-1||hy==MAPH-1) tid = TEX_IRON;
  if(y1 < 0)
  {
    startV = incPerPix * -y1;
    y1=0;    
  }
  if(y2>96)
  {
    y2=96;
  }

  int ui = fix16_to_int(u)%16;
  int lvi = -1;
  int pixCol = 0;
  v = startV;
  int w = (x2-x1)+1;
  for(int y = y1; y < y2; y++)
  {
    int vi = fix16_to_int(fix16_floor(v))%16;
    if(vi != lvi)
    {
      //pixCol = pgm_read_byte_near(brickSprite + (ui+vi*16));
      pixCol = get_tex(tid, ui, vi);
      lvi = vi;
    }
    if(pixCol + shade == 1)
    {
      //drawShadedPixel(x,y,pixCol+shade);
      arduboy.drawFastHLine(x1,y,w,1);      
    }
    else if(pixCol + shade == 2)
    {
      arduboy.drawPixel(x1+(y%2),y,pixCol);
    }
    
    v = fix16_add(v,incPerPix);
  }
}


void getAngleDistTo(Vec2* pos, Vec2* viewer, int playerAng, AngleSize* result)
{
  fix16_t dx = pos->m_x - viewer->m_x;
  fix16_t dy = pos->m_y - viewer->m_y;

  fix16_t angle = fix16_atan2(dy,dx);

  fix16_t len = fix16_sqrt(fix16_mul(dx,dx) + fix16_mul(dy,dy));
  result->m_dist = len;
  
  result->m_angle = fix16_to_int(fix16_rad_to_deg(angle));
  
  result->m_angle-=playerAng;
  while(result->m_angle>180)
  {
    result->m_angle-=360;
  }
  while(result->m_angle<-180)
  {
    result->m_angle+=360;
  }
  
  return result;
}

const fix16_t movementSpeed = F16(5);

void maingame()
{
  boolean test = false;
  int sliceWidth = 2;

  HitResult hitresult;

  for(int x = 0; x < 96; x+=sliceWidth)
  {    
    int angOffsetDegrees = (x-48);
   
    int angDegrees = m_playerAngDegrees + angOffsetDegrees;

    castRay(&m_playerPos, angDegrees, F16(64), &hitresult);

    if(hitresult.m_hit)
    {
      fix16_t dist = hitresult.m_finalHitDistance;
  
      fix16_t z = fix16_mul(dist, fastCos(angOffsetDegrees));
  
      int wallHeightI = 32;
      if(z > 0)
      {
        fix16_t wallHeight = fix16_div(F16(16), z);              
        wallHeightI = fix16_to_int(wallHeight);
      }
      
      int zInt = fix16_to_int(z);
      depths[x] = wallHeightI;
      depths[x+1] = wallHeightI;
  
      fix16_t part = hitresult.m_isVertical ? hitresult.m_hitY : hitresult.m_hitX;
      fix16_t tcX = (part & 0x0000FFFF) * 16;
      
      //drawShadedBox(x,32-wallHeightI,x+sliceWidth,32+wallHeightI,zInt);
      drawWallSlice(x,32-wallHeightI,x+sliceWidth,32+wallHeightI,tcX,zInt/3,&hitresult);
    }
  }

  for(int x = 0; x < MAPW; ++x)
  {
    for(int y= 0; y < MAPH; ++y)
    {      
      arduboy.drawPixel(x+96,y,getMapI(x,y));
    }
  }

  int pPosX = fix16_to_int(m_playerPos.m_x);
  int pPosY = fix16_to_int(m_playerPos.m_y);
  arduboy.drawPixel(96+pPosX, pPosY, i%2);

  for(int i = 0; i < MAX_GAMEOBJECTS; ++i)
  {
    if(m_spriteObjects[i].m_type != 0)
    {
      getAngleDistTo(&(m_spriteObjects[i].m_position), &m_playerPos,m_playerAngDegrees, &(m_spriteObjects[i].m_cachedAngleSize));     
      int ePosX = fix16_to_int(m_spriteObjects[i].m_position.m_x);
      int ePosY = fix16_to_int(m_spriteObjects[i].m_position.m_y);
      arduboy.drawPixel(96+ePosX, ePosY, i%2);
    }
  }

  for(int i = 0; i < MAX_GAMEOBJECTS; ++i)
  {
    bool anySwaps = false;
    for(int j = 0; j < MAX_GAMEOBJECTS-(i+1); ++j)
    {
      bool swap = false;
      if(m_spriteObjects[j].m_type == 0 && m_spriteObjects[j+1].m_type != 0)
      {
        swap = true;
      }
      else if (m_spriteObjects[j].m_type != 0 && m_spriteObjects[j+1].m_type != 0)
      {
        if(m_spriteObjects[j].m_cachedAngleSize.m_dist < m_spriteObjects[j+1].m_cachedAngleSize.m_dist)
        {
          swap = true;
        }
      }

      if(swap)
      {
        GameObject tmp = m_spriteObjects[j];
        m_spriteObjects[j] = m_spriteObjects[j+1];
        m_spriteObjects[j+1] = tmp;
        anySwaps = true;
      }
    }
    if(!anySwaps)
    {
      break;
    }
  }

  for(int i = 0; i < MAX_GAMEOBJECTS; ++i)
  {
    if(m_spriteObjects[i].m_type != 0)
    {
      int xOff = 48+(m_spriteObjects[i].m_cachedAngleSize.m_angle);    
      if(xOff>-32 && xOff<128)
      {
        fix16_t zte = fix16_mul(m_spriteObjects[i].m_cachedAngleSize.m_dist, fastCos(m_spriteObjects[i].m_cachedAngleSize.m_angle));
        int enHeightI = 32;
        if(zte > 0)
        {
          fix16_t enHeight = fix16_div(F16(16), zte);              
          enHeightI = fix16_to_int(enHeight);
        }
        drawSprite(xOff,enHeightI,96);
      }
    }
  }     

  //

  fix16_t sinA = fastSin(m_playerAngDegrees);
  fix16_t cosA = fastCos(m_playerAngDegrees);

  if(arduboy.pressed(LEFT_BUTTON))
  {
    if(arduboy.pressed(A_BUTTON))
    {
      fix16_t newX = fix16_add(m_playerPos.m_x,fix16_div(sinA, movementSpeed));
      fix16_t newY = fix16_sub(m_playerPos.m_y,fix16_div(cosA, movementSpeed));
  
      if(getMap(newX,newY) == 0)
      {
        m_playerPos.m_x = newX;
        m_playerPos.m_y = newY;
      }    
    }
    else
    {    
      m_playerAngDegrees -=6;
      if(m_playerAngDegrees < 0)
      {
        m_playerAngDegrees+=360;
      }
    }
  }
  if(arduboy.pressed(RIGHT_BUTTON))
  {
    if(arduboy.pressed(A_BUTTON))
    {
      fix16_t newX = fix16_sub(m_playerPos.m_x,fix16_div(sinA, movementSpeed));
      fix16_t newY = fix16_add(m_playerPos.m_y,fix16_div(cosA, movementSpeed));
  
      if(getMap(newX,newY) == 0)
      {
        m_playerPos.m_x = newX;
        m_playerPos.m_y = newY;
      }    
    }
    else
    {
      m_playerAngDegrees +=6;
      if(m_playerAngDegrees > 360)
      {
        m_playerAngDegrees-=360;
      }
    }
  }  
  if(arduboy.pressed(UP_BUTTON))
  {    
    fix16_t newX = fix16_add(m_playerPos.m_x,fix16_div(cosA, movementSpeed));
    fix16_t newY = fix16_add(m_playerPos.m_y,fix16_div(sinA, movementSpeed));

    if(getMap(newX,newY) == 0)
    {
      m_playerPos.m_x = newX;
      m_playerPos.m_y = newY;
    }    
  }
  if(arduboy.pressed(DOWN_BUTTON))
  {
    fix16_t newX = fix16_sub(m_playerPos.m_x,fix16_div(cosA, movementSpeed));
    fix16_t newY = fix16_sub(m_playerPos.m_y,fix16_div(sinA, movementSpeed));

    if(getMap(newX,newY) == 0)
    {
      m_playerPos.m_x = newX;
      m_playerPos.m_y = newY;
    }    
  }

  i = (i+1)%4;
  arduboy.drawPixel(i,0,1);

}
