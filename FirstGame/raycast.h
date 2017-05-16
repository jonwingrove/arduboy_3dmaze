#define MAX_DIST F16(9999);

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

