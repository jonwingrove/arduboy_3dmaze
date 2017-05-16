#ifndef GAMEOBJECT
#define GAMEOBJECT
#include "vector.h"

#define GAMEOBJECT_NONE 0
#define GAMEOBJECT_ENEMY_WALK 1
#define GAMEOBJECT_ENEMY_SHOOT 2
#define GAMEOBJECT_PLAYER_FIREBALL 3
#define GAMEOBJECT_ENEMY_FIREBALL 4

class GameObject
{
  public:
    Vec2 m_position;
    byte m_type;
    byte m_health;
    int16_t m_direction;
    byte m_spriteIdx;

    AngleSize m_cachedAngleSize;
};
#endif
