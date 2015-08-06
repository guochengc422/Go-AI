#include <ctime>
#include "gtp.h"
#include "zobrist.h"
#include "engine.h"

int main()
{
	srand((unsigned int)(time(NULL)));
  init_genrand64(16669666165875248481ULL);

  GTP gtp;
  gtp.GTP_loop();
  return 0;
}