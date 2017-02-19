#include <Arduboy2.h>
#include <avr/pgmspace.h>

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

//int cosTab[360];

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
        
        fix16_t dy = fix16_mult(dx, fix16_div(rise, runn));

        m_x = inverted ? fix16_add(y, dy) : fix16_add(x, dx);
        m_y = inverted ? fix16_add(x, dx) : fix16_add(y, dy);
        m_lengthSquared = fix16_add(fix16_mult(dx, dx), fix16_mult(dy * dy));
      }
    }
};

float castRay(vec2 pos, fix16_t angle, fix16_t range)
{
  fix16_t sinA = sin(angle);
  fix16_t cosA = cos(angle);

  float distance = 0;
  float x = pos.m_x;
  float y = pos.m_y;

  while (true)
  {

    vecStep stepX = vecStep(sinA, cosA, x, y, false);
    vecStep stepY = vecStep(cosA, sinA, y, x, true);

    if(stepX.m_lengthSquared < stepY.m_lengthSquared)
    {
      // this is 'coz at 1,0 going left, we're really looking to see what is in 0,0- looking at edges...
      int dx = cosA < 0 ? 1 : 0;

      fix16_t mapX = stepX.m_x - dx;
      fix16_t mapY = stepX.m_y;

      distance += sqrt(stepX.m_lengthSquared);
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
      int dy = sinA < 0 ? 1 : 0;

      fix16_t mapX = stepY.m_x;
      fix16_t mapY = stepY.m_y - dy;

      distance += sqrt(stepY.m_lengthSquared);
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
  int ix = floor(x);
  int iy = floor(y);
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
float m_playerAngDegrees = 0;
float m_fovDegrees = 60;

void setup() {
  // put your setup code here, to run once:
  arduboy.begin();
  arduboy.setFrameRate(40);
  arduboy.initRandomSeed();

  m_playerPos = vec2(1.3f,1.4f);

}

int i = 0;

void drawShadedPixel(int x, int y, int colM)
{
  if((x+y)%colM == 0)
  {
    arduboy.drawPixel(x,y,1);
  }
}

void loop() {

  if (!(arduboy.nextFrame()))
  {
    return;
  }

  arduboy.clear();

  float dist = castRay(m_playerPos, 0, 4);

  for(int x = 0; x < 128; x+=2)
  {
    fix16_t angOffset = ((x-64)*(m_fovDegrees/128.0f));
    fix16_t ang = m_playerAngDegrees+
              angOffset;

    fix16_t dist = castRay(m_playerPos, fix16_deg_to_rad(ang), 4);

    fix16_t z = dist * cos(ANG_TO_RAD(angOffset));

    fix16_t wallHeight = 32;
    if(z > 0)
    {
      wallHeight = 32/z;              
    }
    for(int dx=x; dx < x+2; ++dx)
    {
      for(int dy=32-wallHeight;dy<32+wallHeight;++dy)
      {
        drawShadedPixel(dx,dy,(int)(z+1));
      }
    }
  }

  if(arduboy.pressed(LEFT_BUTTON))
  {
    m_playerAngDegrees-=4;
  }
  if(arduboy.pressed(RIGHT_BUTTON))
  {
    m_playerAngDegrees+=4;
  }
  if(arduboy.pressed(UP_BUTTON))
  {
    fix16_t sinA = fix16_sin(fix16_deg_to_rad(m_playerAngDegrees));
    fix16_t cosA = fix16_cos(fix16_deg_to_rad(m_playerAngDegrees));
    fix16_t newX = m_playerPos.m_x+cosA*0.2f;
    fix16_t newY = m_playerPos.m_y+sinA*0.2f;

    if(getMap(newX,newY) == 0)
    {
      m_playerPos.m_x = newX;
      m_playerPos.m_y = newY;
    }    
  }

  i = (i+1)%128;
  arduboy.drawPixel(i,0,1);

  arduboy.display();
}
