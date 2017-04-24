#ifndef VECTOR2
#define VECTOR2
class Vec2
{
  public:
    fix16_t m_x;
    fix16_t m_y;

    Vec2(fix16_t x, fix16_t y)
    :
    m_x(x),
    m_y(y)
    {
    }

    Vec2()
    {
      
    }
};

class AngleSize
{
  public:
    int m_angle;
    fix16_t m_dist;
};
#endif
