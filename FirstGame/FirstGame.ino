#include <Arduboy2.h>
#include <avr/pgmspace.h>
#include "fix16.h"

Arduboy2 arduboy;

int mazeSize = 8;
const byte maze[64] PROGMEM = {
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 0, 0, 0, 0, 0, 1, 1,
  1, 1, 1, 0, 1, 0, 1, 1,
  1, 0, 1, 0, 0, 0, 1, 1,
  1, 0, 1, 1, 0, 1, 1, 1,
  1, 0, 1, 1, 0, 1, 1, 1,
  1, 0, 0, 0, 0, 0, 0, 1,
  1, 1, 1, 1, 1, 1, 1, 1
};

const byte sprite[64] PROGMEM = {
  0, 0, 0, 2, 2, 0, 0, 0,
  0, 2, 2, 1, 1, 2, 2, 0,
  2, 1, 1, 1, 1, 1, 1, 2,
  0, 2, 2, 1, 1, 2, 0, 0,
  0, 0, 2, 1, 1, 2, 0, 0,
  0, 2, 1, 2, 2, 1, 2, 0,
  0, 2, 1, 2, 2, 1, 2, 0,
  0, 0, 2, 2, 0, 2, 0, 0
};

const fix16_t sinTab[180] PROGMEM = {
0,1143,2287,3429,4571,5711,6850,7986,9120,10252,11380,12504,13625,14742,15854,16961,18064,19160,20251,21336,22414,23486,24550,25606,26655,27696,28729,29752,30767,31772,-32768,-31783,-30808,-29843,-28889,-27947,-27015,-26096,-25188,-24293,-23411,-22541,-21684,-20841,-20011,-19196,-18394,-17606,-16834,-16076,-15333,-14605,-13893,-13197,-12517,-11853,-11205,-10573,-9959,-9361,-8781,-8217,-7672,-7143,-6633,-6141,-5666,-5210,-4773,-4353,-3953,-3571,-3208,-2864,-2539,-2234,-1947,-1680,-1433,-1205,-996,-807,-638,-489,-360,-250,-160,-90,-40,-10,
-1,-10,-40,-90,-160,-250,-360,-489,-638,-807,-996,-1205,-1433,-1680,-1947,-2234,-2539,-2864,-3208,-3571,-3953,-4353,-4773,-5210,-5666,-6141,-6633,-7144,-7672,-8217,-8781,-9361,-9959,-10573,-11205,-11853,-12517,-13197,-13893,-14606,-15333,-16076,-16834,-17607,-18394,-19196,-20011,-20841,-21684,-22541,-23411,-24293,-25189,-26096,-27015,-27947,-28889,-29843,-30808,-31783,32767,31772,30767,29752,28729,27696,26655,25606,24550,23485,22414,21336,20251,19160,18064,16961,15854,14742,13625,12504,11380,10252,9120,7986,6850,5711,4571,3429,2287,1143
};

class vec2
{
  public:
    fix16_t m_x;
    fix16_t m_y;

    vec2(fix16_t x, fix16_t y)
    {
      m_x = x;
      m_y = y;
    }
};

class spriteObject
{
  public:
    fix16_t m_position;
    int m_sprite;
};

float s_maxDist = fix16_one * 9999;

class vecStep
{
  public:
    fix16_t m_x;
    fix16_t m_y;
    fix16_t m_lengthSquared;

    vecStep(fix16_t rise, fix16_t runn, fix16_t x, fix16_t y, bool inverted)
    {
      if (runn == 0)
      {
        m_lengthSquared = s_maxDist;
      }
      else
      {
        fix16_t dx = runn > 0 ? 
        fix16_sub(fix16_floor(fix16_add(x, fix16_one)), x) : 
        fix16_sub(fix16_ceil(fix16_sub(x, fix16_one)), x);
        
        fix16_t dy = fix16_mul(dx, fix16_div(rise, runn));

        m_x = inverted ? fix16_add(y, dy) : fix16_add(x, dx);
        m_y = inverted ? fix16_add(x, dx) : fix16_add(y, dy);
        m_lengthSquared = fix16_add(fix16_mul(dx, dx), fix16_mul(dy, dy));
      }
    }
};


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

fix16_t castRay(vec2 pos, int angle, fix16_t range)
{
  fix16_t sinA = fastSin(angle);
  fix16_t cosA = fastCos(angle);

  fix16_t distance = 0;
  fix16_t x = pos.m_x;
  fix16_t y = pos.m_y;

  while (true)
  {

    vecStep stepX = vecStep(sinA, cosA, x, y, false);
    vecStep stepY = vecStep(cosA, sinA, y, x, true);

    if(stepX.m_lengthSquared < stepY.m_lengthSquared)
    {
      // this is 'coz at 1,0 going left, we're really looking to see what is in 0,0- looking at edges...
      fix16_t dx = cosA < 0 ? fix16_one : 0;

      fix16_t mapX = fix16_sub(stepX.m_x, dx);
      fix16_t mapY = stepX.m_y;

      distance = fix16_add(distance, fix16_sqrt(stepX.m_lengthSquared));
      x = stepX.m_x;
      y = stepX.m_y;

      int mapAtPos = getMap(mapX, mapY);
      if(mapAtPos == 1)
      {
        return distance;
      }
    }
    else
    {
      fix16_t dy = sinA < 0 ? fix16_one : 0;

      fix16_t mapX = stepY.m_x;
      fix16_t mapY = fix16_sub(stepY.m_y, dy);

      distance = fix16_add(distance, fix16_sqrt(stepY.m_lengthSquared));
      x = stepY.m_x;
      y = stepY.m_y;

      int mapAtPos = getMap(mapX, mapY);
      if(mapAtPos == 1)
      {
        return distance;
      }
    }
    
    if (distance > range)
    {
      return range;
    }
  }
}

int getMap(fix16_t x, fix16_t y)
{
  int ix = fix16_to_int(fix16_floor(x));
  int iy = fix16_to_int(fix16_floor(y));
  if (ix < 0 || iy < 0 || ix >= mazeSize || iy >= mazeSize)
  {
    return 1;
  }
  else
  {
    return pgm_read_byte_near(maze +(ix + (iy * mazeSize)));
  }
}

vec2 m_playerPos = vec2(1.3f,1.4f);
int m_playerAngDegrees = 0;

void setup() {
  // put your setup code here, to run once:
  arduboy.begin();
  arduboy.setFrameRate(30);
  arduboy.initRandomSeed();

  m_playerPos = vec2(fix16_from_float(1.3f),fix16_from_float(1.4f));

}

int i = 0;

void drawShadedPixel(int x, int y, int colM)
{
  if((x+y)%colM == 0)
  {
    arduboy.drawPixel(x,y,1);
  }
}

void maingame();
void testSin();

void loop() {
  if (!(arduboy.nextFrame()))
  {
    return;
  }

  arduboy.clear();

  maingame();
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

void drawSprite(int xPos, int ssize)
{
  int startX = xPos-ssize;
  int endX = xPos+ssize;
  int startY = 32-ssize;
  int endY = 32+ssize;

  fix16_t incPerPix = fix16_div((fix16_one*8), (ssize*2*fix16_one));
  fix16_t u = 0;
  fix16_t v = 0;

  for(int x = startX; x < endX; ++x)
  {
    if(ssize >= depths[x])
    {
      v = 0;
      int ui = fix16_to_int(fix16_floor(u));
      for(int y = startY; y < endY; ++y)
      {
        int vi = fix16_to_int(fix16_floor(v));
        if(ui < 8 && vi < 8)
        {
          int pixCol = pgm_read_byte_near(sprite + (ui+vi*8));
          if(pixCol == 1)
          {
            arduboy.drawPixel(x,y,1);
          }
          else if(pixCol ==2)
          {
            arduboy.drawPixel(x,y,0);
          }
        }
        v = fix16_add(v,incPerPix);
      }
    }
    u = fix16_add(u,incPerPix);
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

void maingame()
{

  float dist = castRay(m_playerPos, 0, 4);
  float fovDegrees = 60;
  int sliceWidth = 2;

  for(int x = 0; x < 128; x+=sliceWidth)
  {    
    int angOffsetDegrees = (x-64)/2;
   
    int angDegrees = m_playerAngDegrees + angOffsetDegrees;

    fix16_t dist = castRay(m_playerPos, angDegrees, fix16_one * 4);

    fix16_t z = fix16_mul(dist, fastCos(angOffsetDegrees));

    int wallHeightI = 32;
    if(z > 0)
    {
      fix16_t wallHeight = fix16_div(16 * fix16_one, z);              
      wallHeightI = min(fix16_to_int(wallHeight),32);
    }
    
    int zInt = fix16_to_int(z);
    depths[x] = wallHeightI;
    depths[x+1] = wallHeightI;
    drawShadedBox(x,32-wallHeightI,x+sliceWidth,32+wallHeightI,zInt);
  }

  //drawSprite(64,16+i);

  if(arduboy.pressed(LEFT_BUTTON))
  {
    m_playerAngDegrees -=4;
    if(m_playerAngDegrees < 0)
    {
      m_playerAngDegrees+=360;
    }
  }
  if(arduboy.pressed(RIGHT_BUTTON))
  {
    m_playerAngDegrees +=4;
    if(m_playerAngDegrees > 360)
    {
      m_playerAngDegrees-=360;
    }
  }
  if(arduboy.pressed(UP_BUTTON))
  {
    fix16_t sinA = fastSin(m_playerAngDegrees);
    fix16_t cosA = fastCos(m_playerAngDegrees);
    fix16_t newX = fix16_add(m_playerPos.m_x,fix16_div(cosA, fix16_one*5));
    fix16_t newY = fix16_add(m_playerPos.m_y,fix16_div(sinA, fix16_one*5));

    if(getMap(newX,newY) == 0)
    {
      m_playerPos.m_x = newX;
      m_playerPos.m_y = newY;
    }    
  }
  if(arduboy.pressed(DOWN_BUTTON))
  {
    fix16_t sinA = fastSin(m_playerAngDegrees);
    fix16_t cosA = fastCos(m_playerAngDegrees);
    fix16_t newX = fix16_sub(m_playerPos.m_x,fix16_div(cosA, fix16_one*5));
    fix16_t newY = fix16_sub(m_playerPos.m_y,fix16_div(sinA, fix16_one*5));

    if(getMap(newX,newY) == 0)
    {
      m_playerPos.m_x = newX;
      m_playerPos.m_y = newY;
    }    
  }

  i = (i+1)%4;
  arduboy.drawPixel(i,0,1);

}
