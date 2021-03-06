#ifndef GROUPH
#define GROUPH

#include "size.h"
#include <iostream>
#include <algorithm>

class PList{
 public:
  virtual void add(int p) = 0;
  virtual void remove(int p) = 0;
  virtual int operator[](int i)const = 0;
  virtual int length() const = 0;

};

class Group{
 private:
  bool color;
  int stones[MAXSIZE2];
  int nsts;
  int liberties[MAXSIZE2*2/3];
  int nlibs;

 public:
  Group();
  void set_up(int point, bool color, const PList &liberties);
  void clear();
  void attach_group(Group *attached);
  
  bool get_color() const { return color; }
  int get_nstones() const { return nsts; }
  int get_stone(int i) const { return stones[i]; }
  const int *get_stones() const { return stones; }
  
  int get_nliberties() const { return nlibs; }
  int get_liberty(int i) const { return liberties[i]; }
  const int *get_liberties() const { return liberties; }
  
  class StoneIterator{
   protected:
    const int *it;
   public:
    StoneIterator() { it = 0; }
    StoneIterator(const Group *gr) { it = gr->get_stones(); }
    void operator++() { ++it; }
    int operator*() const { return *it; }
    operator bool() const { return *it != 0; }
  };
  
  class LibertyIterator : public StoneIterator{
   public:
    LibertyIterator(const Group *gr) { it = gr->get_liberties(); }
  };
  
  bool has_one_liberty() const { return nlibs == 1; }
  bool has_two_liberties() const { return nlibs == 2; }
  int add_liberties(int lib);
  int erase_liberties(int lib);
  void print_group() const;
};

template<const int S> class GroupSet{
 protected:
  Group *groups[S];
  int multiplicity[S];
  int len;

 public:
  GroupSet()
  {
    len = 0;
    groups[0] = 0;
  }

  bool add(Group *gr)
  {
    if (gr == 0) return false;
    for (int i = 0; i < len; i++) {
      if (groups[i] == gr) {
        multiplicity[i]++;
        return  false;
      }
    }
    multiplicity[len] = 1;
    groups[len++] = gr;
    return true;
  }

  Group *operator[](int i) const { return groups[i]; }
  int get_multiplicity(int i) const
  {
    #ifdef DEBUG_INFO
    if(i >= len){
      std::cerr << "WARNING: No multiplicity(i)\n";
      return 0;
    }
    #endif
    return multiplicity[i];
  }
  int length() const{ return len; }
};

template<int S> class PointList : public PList {
protected:
  int points[S];
  int len;

public:
  PointList()
  {
    len = 0;
    points[0] = 0;
  }
  void clear()
  {
    len = 0;
    points[0] = 0;
  }
  virtual void add(int p)
  {
    if (len == S) {
    #ifdef DEBUG_INFO
      std::cerr << "long list\n";
    #endif
      return;
    }
    points[len++] = p;
    points[len] = 0;
  }
  void remove(int p)
  {
    for (int j = 0; j < len; j++) {
      if (points[j] == p) {
        points[j] = points[--len];
        points[len] = 0;
        break;
      }
    }
  }

  int operator[](int i) const { return points[i]; }
  int length() const { return len; }

  void shuffle() { std::random_shuffle(points, points+len); }

};

template<int S> class PointSet : public PointList<S> {
 public:
  void add(int p)
  {
    if (PointList<S>::len == S) {
    #ifdef DEBUG_INFO
      std::cerr << "long set\n";
    #endif
      return;
    }
    for (int i = 0; i < PointList<S>::len; i++) {
      if (PointList<S>::points[i] == p) return;
    }
    PointList<S>::points[PointList<S>::len++] = p;
    PointList<S>::points[PointList<S>::len] = 0;
  }
};
#endif
