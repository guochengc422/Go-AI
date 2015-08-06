#include "zobrist.h"

Zobrist::Zobrist()
{
  zob_key = 0;
  zob_side = genrand64_int64();
  for(int i = 0; i < MAXSIZE2; i++){
    zob_points [0][i] = genrand64_int64();
    zob_points [1][i] = genrand64_int64();
    zob_ko[i] = genrand64_int64();
  }
  clear_history();
}

unsigned long long Zobrist::get_key() const
{
  return zob_key;  
}

void Zobrist::set_key(unsigned long long key)
{
  zob_key = key;
}

void Zobrist::reset()
{
  zob_key = 0;
  clear_history();
}

void Zobrist::toggle_side()
{
#ifdef SITUATIONAL_SUPERKO
  zob_key ^= zob_side;
#endif
}

void Zobrist::toggle_ko(int ko_point)
{
  //if(ko_point) zob_key ^= zob_ko[ko_point];
}

void Zobrist::update(int point, bool color)
{
  if(point) zob_key ^= zob_points[color][point-1];
}

void Zobrist::update(const int *moves, const int moves_len)
{
  bool side = 0;
  reset();
  for(int i = 0; i < moves_len; i++){
    if(moves[i]) zob_key ^= zob_points[side][moves[i]-1];
    side = !side;
  }
  if(side) zob_key ^= zob_side;
}

void Zobrist::record_key()
{
  zob_history[active] = zob_key;
  active = (active+1)%6;
}

void Zobrist::clear_history()
{
  for(int i = 0; i < 6; i++){
    zob_history[i] = 0;
  }
  active = 0;
}

bool Zobrist::check_history(unsigned long long key) const
{
  for(int i = 0; i < 6; i++){
    if(zob_history[i] == key) return true;
  }
  return false;
}
