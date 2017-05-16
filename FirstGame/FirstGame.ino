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
#include "textures.h"
#include "raycast.h"

Arduboy2 arduboy;

#define MAX_GAMEOBJECTS 10

GameObject m_spriteObjects[MAX_GAMEOBJECTS];
uint8_t s_musicBuffer[BUFMAX];

byte get_tex(size_t id, int u, int v) {
  byte dat = pgm_read_byte_near(textures + (id*64) + (v*4) + (u/4));
  switch(u%4) {
    case 0: return (dat >> 6) & 0x03;
    case 1: return (dat >> 4) & 0x03;
    case 2: return (dat >> 2) & 0x03;
    case 3: return dat & 0x03;
  }  
}

int getMapI(int ix, int iy)
{
  if (ix < 0 || iy < 0 || ix >= MAPW || iy >= MAPH)
  {
    return 1;
  }
  else
  {
    return get_map(ix,iy,map_0);
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

class GameState
{
public:
  Vec2 m_playerPos = Vec2(1.3f,1.4f);  
  uint16_t m_playerAngDegrees = 0;
  uint32_t seed;
  uint16_t m_timeSincePlayerFire = 0;
  char worldname[NAMELEN];
};

GameState m_gameState;
ArduboyPlaytune m_musicPlayer;

void setup() {
  // put your setup code here, to run once:
  arduboy.begin();
  arduboy.setFrameRate(30);
  arduboy.initRandomSeed();

  m_gameState.seed = random();
  m_gameState.m_playerPos = Vec2(fix16_from_float(1.3f),fix16_from_float(1.4f));
  resetMazeGen(m_gameState.seed);
  resetMusicGen(m_gameState.seed);
  getname(m_gameState.seed, m_gameState.worldname);
  for(int i = 0; i < MAX_GAMEOBJECTS; ++i)
  {
    memset(&m_spriteObjects[i], 0, sizeof(GameObject));
    m_spriteObjects[i].m_type = 0;
  }
  //m_spriteObjects[0].m_type = GAMEOBJECT_ENEMY_WALK;
  //m_spriteObjects[0].m_position = Vec2(fix16_from_float(3.3f),fix16_from_float(3.4f));  

 // m_spriteObjects[1].m_type = GAMEOBJECT_ENEMY_WALK;
  //m_spriteObjects[1].m_position = Vec2(fix16_from_float(4.7f),fix16_from_float(2.8f));  
}

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
byte loading = 32;

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
    arduboy.print(m_gameState.worldname);
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

void updateWalkEnemy(GameObject *go)
{
  Vec2 delta = Vec2(fix16_sub(m_gameState.m_playerPos.m_x,go->m_position.m_x),
    fix16_sub(m_gameState.m_playerPos.m_y,go->m_position.m_y));

  fix16_t sqrMag = fix16_add(fix16_mul(delta.m_x,delta.m_x),fix16_mul(delta.m_y,delta.m_y));

  if(sqrMag < F16(0.5f))
  {
    
  }
  else
  {
    fix16_t distTimesInvSpeed = fix16_mul(fix16_sqrt(sqrMag),F16(10));
    Vec2 dSpeed = Vec2(fix16_div(delta.m_x,distTimesInvSpeed),fix16_div(delta.m_y,distTimesInvSpeed));

    fix16_t newX = fix16_add(go->m_position.m_x,dSpeed.m_x);
    fix16_t newY = fix16_add(go->m_position.m_y,dSpeed.m_y);

    if(getMap(newX,newY) == 0)
    {
      go->m_position.m_x = newX;
      go->m_position.m_y = newY;
    }
  }
}

void updatePlayerFireball(GameObject *go)
{
  fix16_t sinA = fastSin(go->m_direction);
  fix16_t cosA = fastCos(go->m_direction);

  fix16_t newX = fix16_add(go->m_position.m_x,fix16_div(cosA, F16(2)));
  fix16_t newY = fix16_add(go->m_position.m_y,fix16_div(sinA, F16(2)));

  if(getMap(newX,newY) == 0)
  {
    go->m_position.m_x = newX;
    go->m_position.m_y = newY;
  }
  else
  {
    // destroy...
    go->m_type == 0;
  }
}

void updateObject(GameObject *go)
{
  switch(go->m_type)
  {
    case GAMEOBJECT_NONE:
    break;
    case GAMEOBJECT_ENEMY_WALK:
    updateWalkEnemy(go);
    break;
    case GAMEOBJECT_PLAYER_FIREBALL:
    updatePlayerFireball(go);
    break;
  }
}

void maingame()
{
  boolean test = false;
  byte sliceWidth = 2;

  HitResult hitresult;

  for(byte x = 0; x < 96; x+=sliceWidth)
  {    
    int16_t angOffsetDegrees = (x-48);
   
    int16_t angDegrees = m_gameState.m_playerAngDegrees + angOffsetDegrees;

    castRay(&(m_gameState.m_playerPos), angDegrees, F16(64), &hitresult);

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
      
      int16_t zInt = fix16_to_int(z);
      depths[x] = wallHeightI;
      depths[x+1] = wallHeightI;
  
      fix16_t part = hitresult.m_isVertical ? hitresult.m_hitY : hitresult.m_hitX;
      fix16_t tcX = (part & 0x0000FFFF) * 16;
      
      //drawShadedBox(x,32-wallHeightI,x+sliceWidth,32+wallHeightI,zInt);
      drawWallSlice(x,32-wallHeightI,x+sliceWidth,32+wallHeightI,tcX,zInt/3,&hitresult);
    }
  }

  for(byte x = 0; x < MAPW; ++x)
  {
    for(byte y= 0; y < MAPH; ++y)
    {      
      arduboy.drawPixel(x+96,y,getMapI(x,y));
    }
  }

  uint16_t pPosX = fix16_to_int(m_gameState.m_playerPos.m_x);
  uint16_t pPosY = fix16_to_int(m_gameState.m_playerPos.m_y);
  arduboy.drawPixel(96+pPosX, pPosY, 1);

  for(byte i = 0; i < MAX_GAMEOBJECTS; ++i)
  {
    if(m_spriteObjects[i].m_type != 0)
    {
      getAngleDistTo(&(m_spriteObjects[i].m_position), &(m_gameState.m_playerPos),m_gameState.m_playerAngDegrees, &(m_spriteObjects[i].m_cachedAngleSize));     
      int ePosX = fix16_to_int(m_spriteObjects[i].m_position.m_x);
      int ePosY = fix16_to_int(m_spriteObjects[i].m_position.m_y);
      arduboy.drawPixel(96+ePosX, ePosY, i%2);
    }
  }

  for(byte i = 0; i < MAX_GAMEOBJECTS; ++i)
  {
    bool anySwaps = false;
    for(byte j = 0; j < MAX_GAMEOBJECTS-(i+1); ++j)
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

  for(byte i = 0; i < MAX_GAMEOBJECTS; ++i)
  {
    if(m_spriteObjects[i].m_type != 0)
    {
      int16_t xOff = 48+(m_spriteObjects[i].m_cachedAngleSize.m_angle);    
      if(xOff>-32 && xOff<128)
      {
        fix16_t zte = fix16_mul(m_spriteObjects[i].m_cachedAngleSize.m_dist, fastCos(m_spriteObjects[i].m_cachedAngleSize.m_angle));
        int16_t enHeightI = 32;
        if(zte > 0)
        {
          fix16_t enHeight = fix16_div(F16(16), zte);              
          enHeightI = fix16_to_int(enHeight);
        }
        drawSprite(xOff,enHeightI,96);
      }
    }
    updateObject(&m_spriteObjects[i]);
  }     

  //

  fix16_t sinA = fastSin(m_gameState.m_playerAngDegrees);
  fix16_t cosA = fastCos(m_gameState.m_playerAngDegrees);

  if(arduboy.pressed(LEFT_BUTTON))
  {
    if(arduboy.pressed(A_BUTTON))
    {
      fix16_t newX = fix16_add(m_gameState.m_playerPos.m_x,fix16_div(sinA, movementSpeed));
      fix16_t newY = fix16_sub(m_gameState.m_playerPos.m_y,fix16_div(cosA, movementSpeed));
  
      if(getMap(newX,newY) == 0)
      {
        m_gameState.m_playerPos.m_x = newX;
        m_gameState.m_playerPos.m_y = newY;
      }    
    }
    else
    {    
      m_gameState.m_playerAngDegrees -=6;
      if(m_gameState.m_playerAngDegrees < 0)
      {
        m_gameState.m_playerAngDegrees+=360;
      }
    }
  }
  if(arduboy.pressed(RIGHT_BUTTON))
  {
    if(arduboy.pressed(A_BUTTON))
    {
      fix16_t newX = fix16_sub(m_gameState.m_playerPos.m_x,fix16_div(sinA, movementSpeed));
      fix16_t newY = fix16_add(m_gameState.m_playerPos.m_y,fix16_div(cosA, movementSpeed));
  
      if(getMap(newX,newY) == 0)
      {
        m_gameState.m_playerPos.m_x = newX;
        m_gameState.m_playerPos.m_y = newY;
      }    
    }
    else
    {
      m_gameState.m_playerAngDegrees +=6;
      if(m_gameState.m_playerAngDegrees > 360)
      {
        m_gameState.m_playerAngDegrees-=360;
      }
    }
  }  
  if(arduboy.pressed(UP_BUTTON))
  {    
    fix16_t newX = fix16_add(m_gameState.m_playerPos.m_x,fix16_div(cosA, movementSpeed));
    fix16_t newY = fix16_add(m_gameState.m_playerPos.m_y,fix16_div(sinA, movementSpeed));

    if(getMap(newX,newY) == 0)
    {
      m_gameState.m_playerPos.m_x = newX;
      m_gameState.m_playerPos.m_y = newY;
    }    
  }
  if(arduboy.pressed(DOWN_BUTTON))
  {
    fix16_t newX = fix16_sub(m_gameState.m_playerPos.m_x,fix16_div(cosA, movementSpeed));
    fix16_t newY = fix16_sub(m_gameState.m_playerPos.m_y,fix16_div(sinA, movementSpeed));

    if(getMap(newX,newY) == 0)
    {
      m_gameState.m_playerPos.m_x = newX;
      m_gameState.m_playerPos.m_y = newY;
    }    
  }
  if(arduboy.pressed(B_BUTTON) && m_gameState.m_timeSincePlayerFire > 4)
  {
    for(byte i = 0; i < MAX_GAMEOBJECTS; ++i)
    {
      if(m_spriteObjects[i].m_type == 0)
      {
        m_spriteObjects[i].m_type = GAMEOBJECT_PLAYER_FIREBALL;
        m_spriteObjects[i].m_position = m_gameState.m_playerPos;
        m_spriteObjects[i].m_direction = m_gameState.m_playerAngDegrees;        
        break;
      }
    }
  }

  if(m_gameState.m_timeSincePlayerFire < 255)
  {
    m_gameState.m_timeSincePlayerFire++;
  }

}
