#define MAPW 40
#define MAPH 40
#define MAPS ((MAPW * MAPH) / 8)
#define ITER 48

void clear_map(int a, uint8_t* map) {
  for (uint16_t i = 0; i < MAPS; i++) {
    map[i] = (a ? 0xff : 0x00);
  }
}

void copy_map(uint8_t* src, uint8_t* des) {
  for (uint16_t i = 0; i < MAPS; i++) {
    des[i] = src[i];
  }
}

bool get_map(uint8_t x, uint8_t y, uint8_t* map) {
  uint16_t i = x + y*MAPW;
  return (bool)(map[i / 8] & (0x01 << (i % 8)));
}

void set_map(boolean a, uint8_t x, uint8_t y, uint8_t* map) {
  uint16_t i = x + y*MAPW;
  if (a)   map[i / 8] |= (0x01 << (i % 8));
  else    map[i / 8] &= ~(0x01 << (i % 8));
}

uint8_t moore(uint8_t x, uint8_t y, uint8_t* map) {
  uint8_t m = 0;
  int tmp;

  tmp = x - 1;
  uint8_t xd = (tmp < 0) ? tmp + MAPW : tmp;
  tmp = x + 1;
  uint8_t xi = (tmp >= MAPW) ? tmp - MAPW : tmp;
  tmp = y - 1;
  uint8_t yd = (tmp < 0) ? tmp + MAPH : tmp;
  tmp = y + 1;
  uint8_t yi = (tmp >= MAPH) ? tmp - MAPH : tmp;

  if (get_map(xd, yd, map)) m++;
  if (get_map(x, yd, map)) m++;
  if (get_map(xi, yd, map)) m++;
  if (get_map(xd, y, map)) m++;
  if (get_map(xi, y, map)) m++;
  if (get_map(xd, yi, map)) m++;
  if (get_map(x, yi, map)) m++;
  if (get_map(xi, yi, map)) m++;

  return m;
}

uint8_t vn(uint8_t x, uint8_t y, uint8_t* map) {
  uint8_t m = 0;
  int tmp;

  tmp = x - 1;
  uint8_t xd = (tmp < 0) ? tmp + MAPW : tmp;
  tmp = x + 1;
  uint8_t xi = (tmp >= MAPW) ? tmp - MAPW : tmp;
  tmp = y - 1;
  uint8_t yd = (tmp < 0) ? tmp + MAPH : tmp;
  tmp = y + 1;
  uint8_t yi = (tmp >= MAPH) ? tmp - MAPH : tmp;

  //      if(get_map(xd, yd, map)) m++;
  if (get_map(x, yd, map)) m++;
  //      if(get_map(xi, yd, map)) m++;
  if (get_map(xd, y, map)) m++;
  if (get_map(xi, y, map)) m++;
  //      if(get_map(xd, yi, map)) m++;
  if (get_map(x, yi, map)) m++;
  //      if(get_map(xi, yi, map)) m++;

  return m;
}

void iterate(uint8_t* map_0, uint8_t* map_1) {
  uint8_t x, y, m;

  copy_map(map_0, map_1);

  for (x = 0; x < MAPW; x++) {
    for (y = 0; y < MAPH; y++) {
      m = moore(x, y, map_1);
      if (m == 3)                set_map(true, x, y, map_0);
      else if (m == 0 || m > 4)      set_map(false, x, y, map_0);
    }
  }

}

void room(uint8_t x, uint8_t y, uint8_t* map) {
  if (x < 2)         x = 2;
  if (y < 2)         y = 2;
  if (x > MAPW - 3)    x = MAPW - 3;
  if (y > MAPH - 3)    y = MAPH - 3;
  printf("room at %d, %d\n", x, y);
  set_map(false, x - 1, y - 1, map);
  set_map(false, x, y - 1, map);
  set_map(false, x + 1, y - 1, map);
  set_map(false, x - 1, y, map);
  set_map(false, x, y, map);
  set_map(false, x + 1, y, map);
  set_map(false, x - 1, y + 1, map);
  set_map(false, x, y + 1, map);
  set_map(false, x + 1, y + 1, map);
}


boolean search_fwd(uint8_t* map_0, uint8_t* map_1) {
  clear_map(false, map_1);
  set_map(true, 3, 3, map_1);
  uint8_t x, y, m, xl, yl, i;
  xl = 0; yl = 0;
  for (i = 0; i < ITER; i++) {
    for (x = 1; x < MAPW - 1; x++) {
      for (y = 1; y < MAPH - 1; y++) {
        if (get_map(x, y, map_0)) continue;
        m = vn(x, y, map_1);
        if (m > 0) {
          if (x + y > xl + yl) {
            xl = x; yl = y;
          }
          set_map(true, x, y, map_1);
        }
      }
    }
  }

  if (MAPW - xl < 4 && MAPH - yl < 4) return true;
  room(xl, yl, map_0);
  return false;
}

boolean search_back(uint8_t* map_0, uint8_t* map_1) {
  clear_map(false, map_1);
  set_map(true, MAPW - 4, MAPH - 4, map_1);
  uint8_t x, y, m, xl, yl, i;
  xl = MAPW; yl = MAPH;
  for (i = 0; i < ITER; i++) {
    for (x = MAPW - 2; x > 0; x--) {
      for (y = MAPH - 2; y > 0; y--) {
        if (get_map(x, y, map_0)) continue;
        m = vn(x, y, map_1);
        if (m > 0) {
          if (x + y < xl + yl) {
            xl = x; yl = y;
          }
          set_map(true, x, y, map_1);
        }
      }
    }
  }

  if (xl < 4 && yl < 4) return true;
  room(xl, yl, map_0);
  return false;
}

uint8_t map_0[MAPS];
int stage = 0;
int stage1I = 0;
boolean stage3Dir = true;

void resetGen()
{
  stage = 0;
  stage1I = 0;
  stage3Dir = true;
}

boolean genMap() {
  uint8_t x, y, i;
  uint32_t seed;
  uint8_t map_1[MAPS];

  if(stage == 0)
  {
    stage1I = 0;
    
    seed = 1;
    srand(seed);
  
    for (x = 0; x < MAPW; x++) {
      for (y = 0; y < MAPH; y++) {
        set_map((rand() % 13 == 0), x, y, map_0);
      }
    }
    stage++;
    return false;
    
  }
  else if (stage == 1)
  {
    iterate(map_0, map_1);
    stage1I++;
    if(stage1I == ITER)
    {
      stage++;
      return false;
    }
  }
  else if(stage == 2)
  {
    for (x = 0; x < MAPW; x++) {
      for (y = 0; y < MAPH; y++) {
        if (x == 0 || y == 0 || x == MAPW - 1 || y == MAPH - 1)            set_map(1, x, y, map_0);
        else if ((x < 5 && y < 5) || (x > MAPW - 6 && y > MAPH - 6))       set_map(0, x, y, map_0);
      }
    }
    stage3Dir = true;
    stage++;
    return false;    
  }
  else if(stage == 3)
  {
    boolean isOk = false;    
    if (stage3Dir) {
      isOk = search_fwd(map_0, map_1);
    }
    else {
      isOk = search_back(map_0, map_1);
    }
    stage3Dir = !stage3Dir;
    if(isOk)
    {
      stage++;
    }
    return false;
  }
  else if(stage == 4)
  {
    return true;
  }

}

