#ifndef GAMEOBJECT
#define GAMEOBJECT
#include "vector.h"

class GameObject
{
  public:
    Vec2 m_position;
    byte m_type;
    byte m_health;
    byte m_direction;

    AngleSize m_cachedAngleSize;
};
#endif
