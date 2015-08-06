#include "goban.h"

int Goban::is_heuristic_available(bool side, int last_moves) const
{

  if (int move = last_atari_heuristic()) 
  {
    return move;
  }

  if (last_moves) 
  {
    PointSet<MAXSIZE2> list;
    save_heuristic(last_point, list);
    if (int move = random_choose(list, &Goban::heavy_policy)) {
      return move;
    }
    list.clear();
    pattern_heuristic(last_point, list);
    if (int move = random_choose(list, &Goban::heavy_policy)) {
      return move;
    }
    list.clear();
#define CAPTURE_HEURISTICS 
#ifdef CAPTURE_HEURISTICS 
    capture_heuristic(last_point, list);
    if (int move = random_choose(list, &Goban::heavy_policy)) {
      return move;
    }
    list.clear();
#endif
    nakade_heuristic(last_point, list);
    if (int move = random_choose(list, &Goban::heavy_policy)) {
      return move;
    }
    list.clear();
  }

  return 0;
}

int Goban::is_star_available(bool side, int last_moves) const
{
  int star[4] = {3*size+4,3*size+10,9*size+4,9*size+10}; //stars position
  int mid[4] = {3*size+7,6*size+4,6*size+10,9*size+7}; //middle of stars

  //Occupy Stars
  if (points[star[0]] == 0 && heavy_policy(star[0],side)) return star[0];
  if (points[star[1]] == 0 && heavy_policy(star[1],side)) return star[1];
  if (points[star[2]] == 0 && heavy_policy(star[2],side)) return star[2];
  if (points[star[3]] == 0 && heavy_policy(star[3],side)) return star[3];

  //Occupy Mids
  if(points[star[0]] != 0 && points[star[1]] != 0)
  {
    if(points[star[0]]->get_color() == side && points[star[1]]->get_color() == side 
       && points[mid[0]] == 0 && heavy_policy(mid[0],side))
      return mid[0];
  }

  if(points[star[0]] != 0 && points[star[2]] != 0)
  {
    if(points[star[0]]->get_color() == side && points[star[2]]->get_color() == side 
       && points[mid[1]] == 0 && heavy_policy(mid[1],side))
      return mid[1];
  }

  if(points[star[1]] != 0 && points[star[3]] != 0)
  {
    if(points[star[1]]->get_color() == side && points[star[3]]->get_color() == side 
       && points[mid[2]] == 0 && heavy_policy(mid[2],side))
      return mid[2];
  }

  if(points[star[2]] != 0 && points[star[3]] != 0)
  {
    if(points[star[2]]->get_color() == side && points[star[3]]->get_color() == side 
       && points[mid[3]] == 0 && heavy_policy(mid[3],side))
      return mid[3];
  }
  return 0;
}

int Goban::is_kakari_available(bool side, int last_moves) const
{
  int star[4] = {3*size+4,3*size+10,9*size+4,9*size+10}; //stars position

  // kakari
  int kakari;
  if (points[star[0]] != 0 && points[star[0]]->get_color() != side)
  {
      kakari = relative_point(star[0],2,-1);
      if(points[kakari] == 0 && heavy_policy(kakari,side)) return kakari;
      kakari = relative_point(star[0],-1,2);
      if(points[relative_point(star[0],2,-1)]->get_color() != side && points[kakari] == 0 && heavy_policy(kakari,side)) return kakari;
  }

  if (points[star[1]] != 0 && points[star[1]]->get_color() != side)
  {
      kakari = relative_point(star[1],1,2);
      if(points[kakari] == 0 && heavy_policy(kakari,side)) return kakari;
      kakari = relative_point(star[1],-2,-1);
      if(points[relative_point(star[1],1,2)]->get_color() != side && points[kakari] == 0 && heavy_policy(kakari,side)) return kakari;
  }

  if (points[star[2]] != 0 && points[star[2]]->get_color() != side)
  {
      kakari = relative_point(star[2],2,1);
      if(points[kakari] == 0 && heavy_policy(kakari,side)) return kakari;
      kakari = relative_point(star[2],-1,-2);
      if(points[relative_point(star[2],2,1)]->get_color() != side && points[kakari] == 0 && heavy_policy(kakari,side)) return kakari;
  }

  if (points[star[3]] != 0 && points[star[3]]->get_color() != side)
  {
      kakari = relative_point(star[3],1,-2);
      if(points[kakari] == 0 && heavy_policy(kakari,side)) return kakari;
      kakari = relative_point(star[3],-2,1);
      if(points[relative_point(star[3],1,-2)]->get_color() != side && points[kakari] == 0 && heavy_policy(kakari,side)) return kakari;
  }

  return 0;
}

int Goban::is_xiaomu_available(bool side, int last_moves) const
{
  int star[4] = {3*size+4,3*size+10,9*size+4,9*size+10}; //stars position
  int anti,xiaomu;

  // anti wu-you corner
  // around star0
  if (!(points[star[0]] != 0 && points[star[0]]->get_color() == side)){
    anti = relative_point(star[0],-1,1);
    xiaomu = relative_point(star[0],0,-1);
    if (points[xiaomu] != 0 && points[xiaomu]->get_color() != side
        && points[anti] == 0 && heavy_policy(anti,side))
        return anti;
    anti = relative_point(star[0],1,-1);
    xiaomu = relative_point(star[0],-1,0);
    if (points[xiaomu] != 0 && points[xiaomu]->get_color() != side
        && points[anti] == 0 && heavy_policy(anti,side))
        return anti;  
  }

  // around star1
  if (!(points[star[1]] != 0 && points[star[1]]->get_color() == side)){
    anti = relative_point(star[1],1,1);
    xiaomu = relative_point(star[1],0,-1);
    if (points[xiaomu] != 0 && points[xiaomu]->get_color() != side
        && points[anti] == 0 && heavy_policy(anti,side))
        return anti;
    anti = relative_point(star[1],-1,-1);
    xiaomu = relative_point(star[1],1,0);
    if (points[xiaomu] != 0 && points[xiaomu]->get_color() != side
        && points[anti] == 0 && heavy_policy(anti,side))
        return anti;
  }

  // around star2
  if (!(points[star[2]] != 0 && points[star[2]]->get_color() == side)){
    anti = relative_point(star[2],-1,-1);
    xiaomu = relative_point(star[2],0,1);
    if (points[xiaomu] != 0 && points[xiaomu]->get_color() != side
        && points[anti] == 0 && heavy_policy(anti,side))
        return anti;
    anti = relative_point(star[2],1,1);
    xiaomu = relative_point(star[2],-1,0);
    if (points[xiaomu] != 0 && points[xiaomu]->get_color() != side
        && points[anti] == 0 && heavy_policy(anti,side))
        return anti;
  }

  // around star3
  if (!(points[star[3]] != 0 && points[star[3]]->get_color() == side)){
    anti = relative_point(star[3],-1,1);
    xiaomu = relative_point(star[3],1,0);
    if (points[xiaomu] != 0 && points[xiaomu]->get_color() != side
        && points[anti] == 0 && heavy_policy(anti,side))
        return anti;
    anti = relative_point(star[3],1,-1);
    xiaomu = relative_point(star[3],0,1);
    if (points[xiaomu] != 0 && points[xiaomu]->get_color() != side
        && points[anti] == 0 && heavy_policy(anti,side))
        return anti;
  }

  return 0;
}

int Goban::is_anti_kakari_available(bool side, int last_moves) const
{
  int star[4] = {3*size+4,3*size+10,9*size+4,9*size+10}; //stars position

  // anti kakari
  // around star0
  int kakari1 = relative_point(star[0],2,-1);
  int kakari2 = relative_point(star[0],-1,2);
  if (points[star[0]] != 0 && points[star[0]]->get_color() == side){
    if (points[kakari1] != 0 && points[kakari1]->get_color() != side
       && points[kakari2] == 0 && heavy_policy(kakari2,side))
        return kakari2;
    if (points[kakari2] != 0 && points[kakari2]->get_color() != side
       && points[kakari1] == 0 && heavy_policy(kakari1,side))
        return kakari1;
  }

  // around star1
  kakari1 = relative_point(star[1],1,2);
  kakari2 = relative_point(star[1],-2,-1);
  if (points[star[1]] != 0 && points[star[1]]->get_color() == side){
    if (points[kakari1] != 0 && points[kakari1]->get_color() != side
       && points[kakari2] == 0 && heavy_policy(kakari2,side))
        return kakari2;
    if (points[kakari2] != 0 && points[kakari2]->get_color() != side
       && points[kakari1] == 0 && heavy_policy(kakari1,side))
        return kakari1;
  }

  // around star2
  kakari1 = relative_point(star[2],2,1);
  kakari2 = relative_point(star[2],-1,-2);
  if (points[star[2]] != 0 && points[star[2]]->get_color() == side){
    if (points[kakari1] != 0 && points[kakari1]->get_color() != side
       && points[kakari2] == 0 && heavy_policy(kakari2,side))
        return kakari2;
    if (points[kakari2] != 0 && points[kakari2]->get_color() != side
       && points[kakari1] == 0 && heavy_policy(kakari1,side))
        return kakari1;
  }

  // around star3
  kakari1 = relative_point(star[3],1,-2);
  kakari2 = relative_point(star[3],-2,1);
  if (points[star[3]] != 0 && points[star[3]]->get_color() == side){
    if (points[kakari1] != 0 && points[kakari1]->get_color() != side
       && points[kakari2] == 0 && heavy_policy(kakari2,side))
        return kakari2;
    if (points[kakari2] != 0 && points[kakari2]->get_color() != side
       && points[kakari1] == 0 && heavy_policy(kakari1,side))
        return kakari1;
  }

  return 0;
}

int Goban::is_anti_yijianjia_available(bool side, int last_moves) const
{
  int star[4] = {3*size+4,3*size+10,9*size+4,9*size+10}; //stars position

  // anti yijianjia
  // around star0
  int yijianjia1 = relative_point(star[0],2,0);
  int yijianjia2 = relative_point(star[0],0,2);
  if (points[star[0]] != 0 && points[star[0]]->get_color() == side){
    if (points[yijianjia1] != 0 && points[yijianjia1]->get_color() != side
       && points[yijianjia2] == 0 && heavy_policy(yijianjia2,side))
        return yijianjia2;
    if (points[yijianjia2] != 0 && points[yijianjia2]->get_color() != side
       && points[yijianjia1] == 0 && heavy_policy(yijianjia1,side))
        return yijianjia1;
  }

  // around star1
  yijianjia1 = relative_point(star[1],-2,0);
  yijianjia2 = relative_point(star[1],0,2);
  if (points[star[1]] != 0 && points[star[1]]->get_color() == side){
    if (points[yijianjia1] != 0 && points[yijianjia1]->get_color() != side
       && points[yijianjia2] == 0 && heavy_policy(yijianjia2,side))
        return yijianjia2;
    if (points[yijianjia2] != 0 && points[yijianjia2]->get_color() != side
       && points[yijianjia1] == 0 && heavy_policy(yijianjia1,side))
        return yijianjia1;
  }

  // around star2
  yijianjia1 = relative_point(star[2],2,0);
  yijianjia2 = relative_point(star[2],0,-2);
  if (points[star[2]] != 0 && points[star[2]]->get_color() == side){
    if (points[yijianjia1] != 0 && points[yijianjia1]->get_color() != side
       && points[yijianjia2] == 0 && heavy_policy(yijianjia2,side))
        return yijianjia2;
    if (points[yijianjia2] != 0 && points[yijianjia2]->get_color() != side
       && points[yijianjia1] == 0 && heavy_policy(yijianjia1,side))
        return yijianjia1;
  }

  // around star3
  yijianjia1 = relative_point(star[3],-2,0);
  yijianjia2 = relative_point(star[3],0,-2);
  if (points[star[3]] != 0 && points[star[3]]->get_color() == side){
    if (points[yijianjia1] != 0 && points[yijianjia1]->get_color() != side
       && points[yijianjia2] == 0 && heavy_policy(yijianjia2,side))
        return yijianjia2;
    if (points[yijianjia2] != 0 && points[yijianjia2]->get_color() != side
       && points[yijianjia1] == 0 && heavy_policy(yijianjia1,side))
        return yijianjia1;
  }

  return 0;
}

int Goban::is_anti_dian33_available(bool side, int last_moves) const
{
  int star[4] = {3*size+4,3*size+10,9*size+4,9*size+10}; //stars position

  // anti dian33
  // around star0
  int dian33 = relative_point(star[0],-1,-1);
  int anti = relative_point(star[0],-1,0);
  if (points[star[0]] != 0 && points[star[0]]->get_color() == side
      && points[dian33] != 0 && points[dian33]->get_color() != side
      && points[anti] == 0 && heavy_policy(anti,side))
      return anti;

  // around star1
  dian33 = relative_point(star[1],1,-1);
  anti = relative_point(star[1],1,0);
  if (points[star[1]] != 0 && points[star[1]]->get_color() == side
      && points[dian33] != 0 && points[dian33]->get_color() != side
      && points[anti] == 0 && heavy_policy(anti,side))
      return anti;

  // around star2
  dian33 = relative_point(star[2],-1,1);
  anti = relative_point(star[2],-1,0);
  if (points[star[2]] != 0 && points[star[2]]->get_color() == side
      && points[dian33] != 0 && points[dian33]->get_color() != side
      && points[anti] == 0 && heavy_policy(anti,side))
      return anti;

  // around star3
  dian33 = relative_point(star[3],1,1);
  anti = relative_point(star[3],1,0);
  if (points[star[3]] != 0 && points[star[3]]->get_color() == side
      && points[dian33] != 0 && points[dian33]->get_color() != side
      && points[anti] == 0 && heavy_policy(anti,side))
      return anti;

  return 0;
}