#define FIXMATH_NO_ROUNDING
#define FIXMATH_NO_OVERFLOW

#include <Arduboy2.h>
#include <avr/pgmspace.h>
#include "fix16.h"
#include "mazegen.h"
#include "vector.h"
#include "lookup.h"

Arduboy2 arduboy;

const byte sprite[256] PROGMEM = {
0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,
0,0,0,0,1,0,1,1,1,1,0,1,0,0,0,0,
0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,
0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,
0,0,0,1,0,1,2,1,2,1,1,0,0,0,0,0,
0,0,0,1,0,1,1,1,1,1,1,0,0,0,0,0,
0,0,0,1,0,1,1,1,1,1,1,0,0,0,0,0,
0,0,0,1,0,1,1,1,1,1,0,1,1,1,0,0,
0,0,1,1,1,0,1,1,1,1,0,1,1,1,0,0,
0,0,0,1,1,0,1,1,1,1,0,1,1,1,0,0,
0,0,0,0,0,0,1,1,1,1,0,1,1,1,0,0,
0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,
0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,
0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,
0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,
0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0
};

const byte brickSprite[256] PROGMEM = {
0,1,1,1,1,1,1,2,0,1,1,1,1,1,1,2,
0,1,1,1,1,1,1,2,0,1,1,1,1,1,1,2,
0,2,2,2,2,2,2,2,0,2,2,2,2,2,2,2,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,1,1,2,0,1,1,1,1,1,1,2,0,1,1,1,
1,1,1,2,0,1,1,1,1,1,1,2,0,1,1,1,
2,2,2,2,0,2,2,2,2,2,2,2,0,2,2,2,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,1,1,1,1,1,1,2,0,1,1,1,1,1,1,2,
0,1,1,1,1,1,1,2,0,1,1,1,1,1,1,2,
0,2,2,2,2,2,2,2,0,2,2,2,2,2,2,2,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,1,1,2,0,1,1,1,1,1,1,2,0,1,1,1,
1,1,1,2,0,1,1,1,1,1,1,2,0,1,1,1,
1,1,1,2,0,1,1,1,1,1,1,2,0,1,1,1,
2,2,2,2,0,2,2,2,2,2,2,2,0,2,2,2
};

class SpriteObject
{
  public:
    Vec2 m_position;
    int m_sprite;
};

float s_maxDist = fix16_one * 9999;

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
        m_lengthSquared = s_maxDist;
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

Vec2 m_enemyPos = Vec2(4.4f,5.4f);
int m_playerAngDegrees = 0;

void setup() {
  // put your setup code here, to run once:
  arduboy.begin();
  arduboy.setFrameRate(30);
  arduboy.initRandomSeed();

  m_playerPos = Vec2(fix16_from_float(1.3f),fix16_from_float(1.4f));
  m_enemyPos = Vec2(fix16_from_float(3.3f),fix16_from_float(3.4f));
  resetGen(7);
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

void maingame();
void testSin();
boolean mapGenerated = false;
int loading = 32;

void loop() {
  if (!(arduboy.nextFrame()))
  {
    return;
  }

  arduboy.clear();

  if(mapGenerated == false)
  {
    arduboy.setCursor(0,0);
    
    mapGenerated = genMap();    
    drawSprite(loading,16,128);
    loading++;
    if(loading > 96)
    {
      loading=32;
    }
    if(mapGenerated)
    {
      arduboy.print("GENERATED DUNGEON");  
    }
    else
    {
      if(stage == 0)
      {
        arduboy.print("GENERATING DUNGEON 0");
      }
      else if(stage == 1)
      {
        arduboy.print("GENERATING DUNGEON 1");
      }
      else if(stage == 2)
      {
        arduboy.print("GENERATING DUNGEON 2");
      }
      else if(stage == 3)
      {
        arduboy.print("GENERATING DUNGEON 3");
      }
      else if(stage == 4)
      {
        arduboy.print("GENERATING DUNGEON 4");
      }
    }
  }
  else
  { 
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

  fix16_t incPerPix = fix16_div((fix16_one*16), (ssize*2*fix16_one));
  fix16_t u = 0;
  fix16_t v = 0;

  for(int x = startX; x < endX; ++x)
  {
    if(x >= 0 && x < 128 && ssize >= depths[x])
    {
      v = 0;
      int ui = fix16_to_int(fix16_floor(u));
      for(int y = startY; y < endY; ++y)
      {
        int vi = fix16_to_int(fix16_floor(v));
        if(ui < 16 && vi < 16)
        {
          int pixCol = pgm_read_byte_near(sprite + (ui+vi*16));
          if(pixCol == 1)
          {
            arduboy.drawPixel(x,y,1);
          }
          else if(pixCol == -1)
          {
            arduboy.drawPixel(x,y,0);
          }
          else if(pixCol != 0)
          {
            drawShadedPixel(x,y,pixCol);
          }
        }
        v = fix16_add(v,incPerPix);
      }
    }
    u = fix16_add(u,incPerPix);
    if(x>=rClip)
    {
      return;
    }
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

void drawWallSlice(int x1, int y1, int x2, int y2, fix16_t u, int shade)
{
  fix16_t incPerPix = fix16_div((fix16_one*16), ((y2-y1)*fix16_one));
  fix16_t v = 0;
  fix16_t startV = 0;
  if(y1 < 0)
  {
    startV = fix16_mul(incPerPix, fix16_from_int(-y1));
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
      pixCol = pgm_read_byte_near(brickSprite + (ui+vi*16));
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


AngleSize getAngleDistTo(Vec2 pos, Vec2 viewer, int playerAng)
{
  AngleSize result;
  fix16_t dx = pos.m_x - viewer.m_x;
  fix16_t dy = pos.m_y - viewer.m_y;

  fix16_t angle = fix16_atan2(dy,dx);

  fix16_t len = fix16_sqrt(fix16_mul(dx,dx) + fix16_mul(dy,dy));
  result.m_dist = len;
  
  result.m_angle = fix16_to_int(fix16_rad_to_deg(angle));
  
  result.m_angle-=playerAng;
  while(result.m_angle>180)
  {
    result.m_angle-=360;
  }
  while(result.m_angle<-180)
  {
    result.m_angle+=360;
  }
  
  return result;
}

void maingame()
{
  boolean test = false;
  int sliceWidth = 2;

  HitResult hitresult;

  for(int x = 0; x < 96; x+=sliceWidth)
  {    
    int angOffsetDegrees = (x-48);
   
    int angDegrees = m_playerAngDegrees + angOffsetDegrees;

    castRay(&m_playerPos, angDegrees, fix16_one * 64, &hitresult);

    if(hitresult.m_hit)
    {
      fix16_t dist = hitresult.m_finalHitDistance;
  
      fix16_t z = fix16_mul(dist, fastCos(angOffsetDegrees));
  
      int wallHeightI = 32;
      if(z > 0)
      {
        fix16_t wallHeight = fix16_div(16 * fix16_one, z);              
        wallHeightI = fix16_to_int(wallHeight);
      }
      
      int zInt = fix16_to_int(z);
      depths[x] = wallHeightI;
      depths[x+1] = wallHeightI;
  
      fix16_t part = hitresult.m_isVertical ? hitresult.m_hitY : hitresult.m_hitX;
      fix16_t tcX = fix16_mul(part & 0x0000FFFF, fix16_one*16);
      
      //drawShadedBox(x,32-wallHeightI,x+sliceWidth,32+wallHeightI,zInt);
      drawWallSlice(x,32-wallHeightI,x+sliceWidth,32+wallHeightI,tcX,zInt/3);
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

  int ePosX = fix16_to_int(m_enemyPos.m_x);
  int ePosY = fix16_to_int(m_enemyPos.m_y);
  arduboy.drawPixel(96+ePosX, ePosY, i%2);

  AngleSize toEnemy = getAngleDistTo(m_enemyPos, m_playerPos,m_playerAngDegrees);
  int xOff = 48+(toEnemy.m_angle);
  if(xOff>-32 && xOff<128)
  {
    fix16_t zte = fix16_mul(toEnemy.m_dist, fastCos(toEnemy.m_angle));
    int enHeightI = 32;
    if(zte > 0)
    {
      fix16_t enHeight = fix16_div(16 * fix16_one, zte);              
      enHeightI = min(fix16_to_int(enHeight),32);
    }
    drawSprite(xOff,enHeightI,96);
  }

  //

  fix16_t sinA = fastSin(m_playerAngDegrees);
  fix16_t cosA = fastCos(m_playerAngDegrees);

  if(arduboy.pressed(LEFT_BUTTON))
  {
    if(arduboy.pressed(A_BUTTON))
    {
      fix16_t newX = fix16_add(m_playerPos.m_x,fix16_div(sinA, fix16_one*5));
      fix16_t newY = fix16_sub(m_playerPos.m_y,fix16_div(cosA, fix16_one*5));
  
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
      fix16_t newX = fix16_sub(m_playerPos.m_x,fix16_div(sinA, fix16_one*5));
      fix16_t newY = fix16_add(m_playerPos.m_y,fix16_div(cosA, fix16_one*5));
  
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
