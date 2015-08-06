#ifndef TREEH
#define TREEH
#include "amaf.h"
#include "goban.h"

class Node{
private:
  int move;
  double visits, results;
  double rave_visits, rave_results;
  double prior_visits, prior_results;
  Node *child, *sibling;
  int visiting = 0;
public:
  void reset();
  void copy_values(const Node *orig);
  void add_child(Node *child);
  void set_move(int, const Prior &prior);
  void set_results(int result);
  void set_amaf(int result, const AmafBoard &amaf, bool side, int depth);
  
  int get_move() const{ return move; }
  double get_results() const{ return results; };
  double get_visits() const{ return visits; };
  double get_rave_results() const{ return rave_results; };
  double get_rave_visits() const{ return rave_visits; };
  Node *get_child() const{ return child; }
  Node *get_sibling() const{ return sibling; }
  void print(int boardsize) const;


  bool has_childs() const{ return child != 0; };
  double get_value(double parent_visits) const;
  Node *select_child() const;
  Node *get_best_child() const;
};

class Tree{
private:
  Node *root[2];
  int size[2], maxsize, active;
  const Goban *goban;
public:
  Tree(int maxsize, Goban *goban);
  ~Tree();
  void clear();
  void clear_active();
  int promote(int new_root);
  Node *insert(Node *parent, int move, const Prior &prior);
  Node *insert(Node *parent, const Node *orig);
  void copy_recursive(Node *parent, const Node *orig);
  int expand(Node *parent, const int *moves, int nmoves, const Prior[]);
  Node *get_best() const{ return root[active]->get_best_child(); }
  Node *get_root() const{ return root[active]; }
  int get_size() const{ return size[active];}
  void print() const;
  void print(Node *node, int threshold, int depth) const;
};
#endif
