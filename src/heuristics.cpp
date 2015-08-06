#include "goban.h"

int Goban::random_choose(const PList &list, bool(Goban::*Policy)(int, bool) const) const
{
  if (list.length() == 0) return 0;
  int first_choice = rand() % list.length();
  for (int i = first_choice; i < list.length(); i++) {
    int point = list[i];
    if ((this->*Policy)(point, side)) {
      return point;
    }
  }
  for (int i = 0; i < first_choice; i++) {
    int point = list[i];
    if ((this->*Policy)(point, side)) {
      return point;
    }
  }
  return PASS;
}

bool Goban::random_policy(int point, bool side) const
{  
  if (is_virtual_eye(point, side)) return false;
  if (!is_legal(point, side)) return false;

  return true;
}

bool Goban::heavy_policy(int point, bool side) const
{  
  if (is_virtual_eye(point, side)) return false;
  if (!is_legal(point, side)) return false;
  if (is_self_atari(point, side)) return false;
  if (fast_ladder(point,side)) return false;
  if (bad_self_atari(point,side)) return false;
  return true;
}

int Goban::play_random()
{
  return play_move(random_choose(empty_points, &Goban::random_policy));
}

int Goban::play_heavy()
{
  if (int move = last_atari_heuristic()) {
    return play_move(move);
  }

  if (last_point) {
    PointSet<MAXSIZE2> list;
    save_heuristic(last_point, list);
    if (int move = random_choose(list, &Goban::heavy_policy)) {
      return play_move(move);
    }
    list.clear();
    pattern_heuristic(last_point, list);
    if (int move = random_choose(list, &Goban::heavy_policy)) {
      return play_move(move);
    }
    list.clear();
#define CAPTURE_HEURISTICS 
#ifdef CAPTURE_HEURISTICS 
    capture_heuristic(last_point, list);
    if (int move = random_choose(list, &Goban::heavy_policy)) {
      return play_move(move);
    }
    list.clear();
#endif
    nakade_heuristic(last_point, list);
    if (int move = random_choose(list, &Goban::heavy_policy)) {
      return play_move(move);
    }
    list.clear();
  }

  return play_random();
}

int Goban::last_atari_heuristic() const
{
  if (Group *group = last_atari[!side]) {
    int move = group->get_liberty(0);
    if (is_legal(move, side) &&
        !is_self_atari(move, side) &&
        gains_liberties(move, group)) {
      return move;
    }
  }
  
  PointList<MAXSIZE2> save;
  if (last_atari[side]) {
    atari_escapes(last_atari[side], save);
    if (int move = random_choose(save, &Goban::heavy_policy)) {
      return move;
    }
  }
  return 0;
}

void Goban::nakade_heuristic(int point, PList &list) const
{
  for (int i = 0; i < 8; i++) {
    if (int v = vicinity[point][i]) {
      if (points[v] == 0 && (creates_eyes(v, 0) > 1 || creates_eyes(v, 1) > 1)) {
        list.add(v);
      }
    }
  }
}

void Goban::capture_heuristic(int point, PList &list) const
{
  for (int i = 0; i < 8; i++) {
    Group *group = points[vicinity[point][i]];
    if (group && group->get_color() != side && group->has_one_liberty()) {
      int lib = group->get_liberty(0);
      if (gains_liberties(lib, group)) {
          list.add(lib);
      }
    }
}
}

void Goban::save_heuristic(int point, PList &list) const
{
  if (points[point] && points[point]->get_nliberties() == 2) {
    for (int i = 0; i < 2; i++) {
      int lib = points[point]->get_liberty(i);
      if (gains_liberties(lib, points[point])) {
        list.add(lib);
      }
    }
  }
  GroupSet<4> neigh;
  int nneigh = neighbour_groups(point, neigh);
  for (int j = 0; j < nneigh; j++) {
    if (neigh[j]->get_color() == side && neigh[j]->get_nliberties() == 2) { // <2?
      atari_escapes(neigh[j], list);
    }
  }
  for (int i = 0; i < 8; i++) {
    if (int v = vicinity[point][i]) {
      if (points[v] != 0 && points[v]->get_color() == side && points[v]->get_nliberties() == 1) {
        atari_escapes(points[v], list);
      }
    }
  }

}

void Goban::pattern_heuristic(int point, PList &list) const
{
  for (int i = 0; i < 8; i++) {
    if (int v = vicinity[point][i]) {
      if (points[v] == 0 && match_mogo_pattern(v, side)) {
        list.add(v);
      }
    }
  }
}

bool Goban::stones_around(int point, int distance) const
{
  for (int i = 0; i < distance; i++) {
    for (int j = 0; j < 4*(i+1); j++) {
      int v =  within_manhattan[point][i][j];
      if (is_occupied(v)) return true;
    }
  }
  return false;
}

int Goban::total_liberties(int point, bool color, PList *liberties, int enough=0, const Group *exclude=0) const
{
  PointSet<MAXSIZE2> libs;
  if (liberties) point_liberties(point, *liberties);
  point_liberties(point, libs);
  if (enough && libs.length() > enough) return libs.length();
  
  GroupSet<4> neighbours;
  int nneigh = neighbour_groups(point, neighbours);
  for (int i = 0; i < nneigh; i++) {
    const Group *curr_neigh = neighbours[i];
    if (curr_neigh != exclude) {
      if (curr_neigh->get_color() == color) {
        for (Group::LibertyIterator lib(curr_neigh); lib; ++lib) {
          if (*lib != point) {
            if (liberties) liberties->add(*lib);
            libs.add(*lib);
            if (enough && libs.length() > enough) return libs.length();
          }
        }
      } else if (curr_neigh->get_nliberties() == 1) {
        for (Group::StoneIterator st(curr_neigh); st; ++st) {
          for (int j = 0; adjacent[*st][j]; j++) {
            if (adjacent[*st][j] == point) {
              libs.add(*st);
              if (enough && libs.length() > enough) return libs.length();
            } else if (points[adjacent[*st][j]] &&
                       points[adjacent[*st][j]]->get_color() == color) {
              for (int k = 0; k < nneigh; k++) {
                if (points[adjacent[*st][j]] == neighbours[k]) {
                  libs.add(*st);
                  if (enough && libs.length() > enough) return libs.length();
                }
              }
            }
          }
        }
      }
    }
  }
  return libs.length();
}

bool Goban::gains_liberties(int point, const Group *group) const
{
  int curr_liberties = 1;
  if (group) curr_liberties = group->get_nliberties();
  int nlibs = total_liberties(point, group->get_color(), 0, curr_liberties, group);
  return nlibs > curr_liberties;
}

bool Goban::is_self_atari(int point, bool color) const
{
  return (total_liberties(point, color, 0, 1) == 1);
}

int Goban::atari_last_liberty(int point, bool color) const
{
  PointSet<MAXSIZE2> liberties;
  if (total_liberties(point, color, &liberties, 1) == 1) return liberties[0]; //Maybe 0!
  return -1;
}

int Goban::atari_escapes(const Group *group, PList &escapes) const
{
  for (Group::LibertyIterator lib(group); lib; ++lib) {
    if (gains_liberties(*lib, group)) {
      escapes.add(*lib);
    }
  }
  GroupSet<MAXSIZE2/3> neighbours;
  int nneigh = neighbour_groups(group, !group->get_color(), group->get_nliberties(), neighbours);
  for (int i = 0; i < nneigh; i++) {
    for (Group::LibertyIterator lib(neighbours[i]); lib; ++lib) {
      if (gains_liberties(*lib, group)) {
        escapes.add(*lib);
      }
    }
  }
  return escapes.length();
}

bool Goban::fast_ladder(int point, bool color) const
{
  if (total_liberties(point, color, 0) != 2) return false;
  if (neighbour_groups(point, !color, 2, 0)) return false;
  PointList<5> liberties;
  point_liberties(point, liberties);
  for (int i = 0; i < liberties.length(); i++) {
    PointList<5> secondary_libs;
    int delta[2] = {0};
    if (point_liberties(liberties[i], secondary_libs) == 4) continue;
  
    delta[0] = liberties[i] - point;
    for (int j = 0; j < secondary_libs.length(); j++) {
      if (secondary_libs[j] != point && secondary_libs[j] != liberties[i] + delta[0]) {
        delta[1] = secondary_libs[j] - liberties[i];
        break;
      }
    }
    if (delta[1] == 0) return true;
    int p = point, act = 0;
    while (distance_to_edge[p] > 1) {
      p = p + delta[act];

      if (points[p]) {
        if (points[p]->get_color() == color) break;
        else return true;
      }
      if (points[p + delta[act]]) {
        if (points[p + delta[act]]->get_color() == color) break;
        else return true;
      }
      act = 1-act;
    }
    if (distance_to_edge[p] < 2) return true;
  }
  return false;
}

int Goban::creates_eyes(int point, bool color) const
{
  int neyes = 0;
  for (int i = 0; adjacent[point][i]; i++) {
    if (is_true_eye(adjacent[point][i], color, point)) {
      neyes++;
    }
  }
#ifdef DEBUG_INFO
  if (neyes) {
    std::cerr << point << " creates " << neyes << " eyes\n";
  }
#endif
  return neyes;
}

bool Goban::bad_self_atari(int point, bool color) const
{
  int last_lib = atari_last_liberty(point, color);
  if (last_lib == -1) return false;
#ifdef DEBUG_INFO
  std::cerr << "self_atari at " << point << "\n";
#endif
    if (total_liberties(last_lib, !color, 0) < 2) {
#ifdef DEBUG_INFO
      std::cerr << "snapback\n";
#endif  
      return false;
    }
    if (creates_eyes(point, !color) && !creates_eyes(last_lib, !color)) {
#ifdef DEBUG_INFO
        std::cerr << "throw-in\n";
#endif
      return false;
    }

  if (nakade_shape(point, color)) {
#ifdef DEBUG_INFO
    std::cerr << "nakade\n";
#endif
    return false;
  }
  return true;
}

int Goban::bulkiness(const Group *group, int point) const
{
  int max_bulkiness = 0;
  for (Group::StoneIterator st(group); st; ++st) {
    int bulk = 0;
    for (int i = 0; adjacent[*st][i]; i++) {
      if (points[adjacent[*st][i]] == group || adjacent[*st][i] == point) {
        bulk++;
      }
    }
    if (bulk > max_bulkiness) max_bulkiness = bulk;
  }
  return max_bulkiness;
}

int Goban::neighbour_bulkiness(int point, bool color) const
{
  GroupSet<4> neighbours;
  int nneigh = neighbour_groups(point, neighbours);
  int max_bulkiness = nneigh;
  for (int i = 0; i < nneigh; i++) {
    Group *neigh = neighbours[i];
    if (neigh->get_color() == color) {
      int bulk = bulkiness(neigh, point);
      if (bulk > max_bulkiness) max_bulkiness = bulk;
    }
  }
  return max_bulkiness;
}

bool Goban::nakade_shape(int point, bool color) const
{
  int bulk = neighbour_bulkiness(point, color);
      
  switch(neighbours_size(point, color)) {
    case 2:
      return true;
      break;
    case 3:
      if (bulk == 3) return true;
      break;
    case 4:
      if (bulk == 4 || bulk == 3) return true; 
      break;
    case 5:
      if (bulk == 4) return true;
      break;
    default:
      return false;
  }
  return false;
}

bool Goban::match_mogo_pattern(int point, bool side) const
{    
  if (point == 1 || point == size || point == size2 || point == size*(size-1)+1) return false;  //filter corners.
  
  if (point > size && point % size && point <= size*(size-1) && point % size != 1) {
    const int *vic = vicinity[point];
    for (int i = 1; i < 8; i+=2) {
      if (points[vic[i]]) {
        bool adj_color = points[vic[i]]->get_color();
        if (points[vic[i-1]] && points[vic[i-1]]->get_color() != adj_color) {
          if (points[vic[i+2]] == 0 && points[vic[i+6]] == 0) {
            if (points[vic[i+1]] && points[vic[i+1]]->get_color() != adj_color) {
              return true; 
            }
            if (points[vic[i+1]] == 0 && points[vic[i+4]] == 0) {
              return true;
            }
          }
          if (points[vic[i+2]] == 0 && points[vic[i+4]] == 0 &&
              points[vic[i+6]] && points[vic[i+6]]->get_color() != adj_color) {
              return true; 
          }
          if (points[vic[i+6]] && points[vic[i+6]]->get_color() == adj_color) {
            if ((points[vic[i+2]] || points[vic[i+4]] == 0 || points[vic[i+4]]->get_color() == adj_color) &&
                (points[vic[i+4]] || points[vic[i+2]] == 0 || points[vic[i+2]]->get_color() == adj_color)) {
              return true; 
            }
          }
        }
        if (points[vic[i+1]] && points[vic[i+1]]->get_color() != adj_color) {
          if (points[vic[i+2]] == 0 && points[vic[i+6]] == 0) {
            if (points[vic[i-1]] == 0 && points[vic[i+4]] == 0) {
              return true; 
            }
          }
          if (points[vic[i+4]] == 0 && points[vic[i+6]] == 0 &&
              points[vic[i+2]] && points[vic[i+2]]->get_color() != adj_color) {
              return true; 
          }
        }
        if (points[vic[i+2]] && points[vic[i+6]] && points[vic[i+2]]->get_color() != adj_color &&
            points[vic[i+6]]->get_color() != adj_color &&
            (points[vic[i+4]] == 0 || points[vic[i+4]]->get_color() == adj_color) &&
            (points[vic[i+3]] == 0 || points[vic[i+3]]->get_color() == adj_color) &&
            (points[vic[i+5]] == 0 || points[vic[i+5]]->get_color() == adj_color)) {
          return true; 
        }
        if (adj_color != side && points[vic[i-1]] && points[vic[i-1]]->get_color() == side &&
            points[vic[i+2]] == 0 && points[vic[i+4]] == 0 && points[vic[i+6]] == 0 &&
            points[vic[i+1]] && points[vic[i+1]]->get_color() == adj_color) {
          return true; 
        }
        if (adj_color != side && points[vic[i+1]] && points[vic[i+1]]->get_color() == side &&
            points[vic[i+2]] == 0 && points[vic[i+4]] == 0 && points[vic[i+6]] == 0 &&
            points[vic[i-1]] && points[vic[i-1]]->get_color() == adj_color) {
          return true; 
        }
      }
    }
  } else {
    for (int i = 1; i < 8; i+=2) {
      const int *vic = vicinity[point];
      if (vic[i]) {
        if (points[vic[i]]) {
          if (vic[i+2] && points[vic[i+2]] && points[vic[i+2]]->get_color() != points[vic[i]]->get_color() &&
             (vic[i+6]==0 || points[vic[i+6]] == 0 || points[vic[i+6]]->get_color() != points[vic[i]]->get_color())) {
            return true; 
          }
          if (vic[i+6] && points[vic[i+6]] && points[vic[i+6]]->get_color() != points[vic[i]]->get_color() &&
             (vic[i+2]== 0 || points[vic[i+2]] == 0 || points[vic[i+2]]->get_color() != points[vic[i]]->get_color())) {
            return true; 
          }
          if (points[vic[i]]->get_color() == side) {
            if ((vic[i+1] && points[vic[i+1]] && points[vic[i+1]]->get_color() != side)) {
              return true; 
            }
            if ((vic[i-1] && points[vic[i-1]] && points[vic[i-1]]->get_color() != side)) {
              return true; 
            }
          }
          if (points[vic[i]]->get_color() != side) {
            if (vic[i+1] && points[vic[i+1]] && points[vic[i+1]]->get_color() == side) {
              if (vic[i+2] == 0 || points[vic[i+2]] == 0 || points[vic[i+2]]->get_color() == side) {
                return true;
              }
              if (vic[i+2] && points[vic[i+2]] && points[vic[i+2]]->get_color() != side
                && vic[i+6] && points[vic[i+6]] && points[vic[i+6]]->get_color() == side) {
                return true;
              }
            }
            if (vic[i-1] && points[vic[i-1]] && points[vic[i-1]]->get_color() == side) {
              if (vic[i+6] == 0 || points[vic[i+6]] == 0 || points[vic[i+6]]->get_color() == side) {
                return true;  
              }
              if (vic[i+6] && points[vic[i+6]] && points[vic[i+6]]->get_color() != side &&
                  vic[i+2] && points[vic[i+2]] && points[vic[i+2]]->get_color() == side) {
                return true; 
              }
            }
          }
        } else if ((vic[i+6] && points[vic[i+6]] && vic[i-1] && points[vic[i-1]] &&
                    points[vic[i+6]]->get_color() != points[vic[i-1]]->get_color())
               || (vic[i+2] && points[vic[i+2]] && vic[i+1] && points[vic[i+1]] &&
                    points[vic[i+2]]->get_color() != points[vic[i+1]]->get_color())) {
          return true;  
        }
      }
    }
  }
  return false;
}
