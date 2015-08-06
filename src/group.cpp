#include "group.h"

Group::Group()
{
  color = 0;
  stones[0] = 0;
  liberties[0] = 0;
  nsts = 0;
  nlibs = 0;
}

void Group::set_up(int point, bool new_color, const PList &new_liberties)
{
  color = new_color;
  nsts = 0;
  stones[nsts++] = point;
  stones[nsts] = 0;
  nlibs = 0;
  for (int i = 0; i < new_liberties.length(); i++) {
    liberties[nlibs++] = new_liberties[i];
    liberties[nlibs] = 0;
  }
}

int Group::add_liberties(int i)
{
  for (int j = 0; j < nlibs; j++) {
    if (liberties[j] == i) return 0;
  }
  liberties[nlibs++] = i;
  liberties[nlibs] = 0;
  return nlibs;
}

int Group::erase_liberties(int lib)
{
  for (int j = 0; j < nlibs; j++) {
    if (liberties[j] == lib) {
      liberties[j] = liberties[--nlibs];
      liberties[nlibs]  = 0;
      return nlibs;
    }
  }
  return 0;
}

void Group::clear()
{
  nsts = 0;
  stones[nsts] = 0;
  nlibs = 0;
  liberties[nlibs] = 0;
}

void Group::attach_group(Group *attached)
{  
  for (int i = 0; i < attached->nsts; i++) {
    this->stones[nsts++] = attached->stones[i];
    this->stones[nsts] = 0;
  }
  for (int i = 0; i < attached->nlibs; i++) {
    this->add_liberties(attached->liberties[i]);
  }
}
void Group::print_group() const
{
  std::cerr << "Color: " << color;
  for (int i = 0; i < nsts; i++) std::cerr << " " << stones[i];
  std::cerr << "\n";
}
