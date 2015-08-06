#include <iostream>
#include <ctime>
#include "engine.h"



#define DEF_PLAYOUTS 50000
#define DEF_TREESIZE 5000000

#define MAX_BEGINNING_HEURISTICS_MOVE 30
#define THREAD_NUM 4



Engine::Engine(Goban *goban):tree(DEF_TREESIZE, goban), amaf(goban->get_size())
{
  main_goban = goban;
  max_playouts = DEF_PLAYOUTS;
  max_time = 10*CLOCKS_PER_SEC;
  //max_time = INFINITE;
  tree_size = DEF_TREESIZE;
  rand_movs = 0;
}

void Engine::reset()
{
  rand_movs = 0;
  tree.clear();
  amaf.set_up(main_goban->get_side(), main_goban->get_size());
}

void Engine::set_playouts(int playouts)
{
  max_playouts = playouts;
  max_time = INFINITE;
}

void Engine::set_times(int main_time, int byo_time, int stones)
{
  max_playouts = INFINITE;
  this->byo_time = byo_time;
  if (stones) {
    max_time = (main_time/10 + byo_time/stones)*CLOCKS_PER_SEC;
  } else {
    max_time = (0.9*byo_time + main_time/60)*CLOCKS_PER_SEC;
  }
}

void Engine::set_times(int time_left, int stones)
{
  max_playouts = INFINITE;
  if (stones) max_time = (0.9*time_left/stones)*CLOCKS_PER_SEC;
  else max_time = (0.9*time_left + byo_time)*CLOCKS_PER_SEC;
  std::cerr << max_time << "\n";
}

int Engine::play_random_game(bool heavy, int*simul_l = NULL, AmafBoard* tamaf = NULL, Goban* tgoban = NULL)
{
  int pass = 0;
  if (simul_l == NULL)
	  simul_l = &simul_len;
  if (tgoban == NULL)
	  tgoban = main_goban;
  tgoban->shuffle_empty();
  while (pass < 2) {
	  int move;
	if (heavy){
		move = tgoban->play_heavy();
		tamaf->play(move, ++(*simul_l));
	}
	else{
		move = tgoban->play_random();
		amaf.play(move, ++simul_len);
	}
    rand_movs++;
    if (move == Goban::PASS) pass++;
    else pass = 0;
#ifdef DEBUG_INFO
      tgoban->print_goban();
#endif
	  int mercy = tgoban->mercy();
    if (mercy != -1) {
      return 1-mercy;
    }
	if (*simul_l > 2 * tgoban->get_size2()) {

      std::cerr << "WARNING: Simulation exceeded max length.\n";
      discarded++;
      return -1;
    }
  }
  return (tgoban->chinese_count() > 0) ? 1 : 0;
}

int Engine::generate_move(bool early_pass)
{
  bool side = main_goban->get_side();

  int last_moves = main_goban->get_last_point();

  int dropped_stones_number = main_goban->get_game_history_length();

  if(dropped_stones_number < MAX_BEGINNING_HEURISTICS_MOVE)
  {
    int tmp_move;

    if (tmp_move = main_goban->is_heuristic_available(side,last_moves))
      return tmp_move;
    if (tmp_move = main_goban->is_xiaomu_available(side,last_moves))
      return tmp_move;
    if (tmp_move = main_goban->is_anti_kakari_available(side,last_moves))
      return tmp_move;    
    if (tmp_move = main_goban->is_anti_yijianjia_available(side,last_moves))
      return tmp_move;
    if (tmp_move = main_goban->is_anti_dian33_available(side,last_moves))
      return tmp_move;
    if (tmp_move = main_goban->is_star_available(side,last_moves))
      return tmp_move;
    if (tmp_move = main_goban->is_kakari_available(side,last_moves))
      return tmp_move;
  }

  const double RESIGN_THRESHOLD = 0.00, PASS_THRESHOLD = 0.9;

  Node *root = tree.get_root();
  rand_movs = 0, discarded = 0;
  fin_clock = clock();

  InitializeCriticalSection(&cs);
  HANDLE handles[THREAD_NUM];
  for (int i = 0; i < THREAD_NUM; i++){
	  handles[i] = (HANDLE)_beginthreadex(NULL, 0, Engine::ThreadFun,(void*)this, 0, 0);
  }

  WaitForMultipleObjects(THREAD_NUM, handles, TRUE, INFINITE);
  DeleteCriticalSection(&cs);
  for (int i = 0; i < THREAD_NUM; i++) CloseHandle(handles[i]);
  Node *best = tree.get_best();
  print_PV();
  if (best == Goban::PASS) return Goban::PASS;
  if (best->get_value(1) < RESIGN_THRESHOLD) return -1;
  if (early_pass && best->get_value(1) >= PASS_THRESHOLD && !root->get_move()) return Goban::PASS;

  return best->get_move();

}

unsigned __stdcall Engine::ThreadFun(void* p)
{
	int seed = GetCurrentThreadId() * time(NULL);
  srand(seed);
	Engine *tEngine = (Engine*)p;
	int EXPAND = 40;
	bool side = tEngine->main_goban->get_side();
	int tlegal_moves[MAXSIZE2+1];
	int nlegal = tEngine->main_goban->legal_moves(tlegal_moves);

	Goban tMain_goban = Goban(13);

	Node *root = tEngine->tree.get_root();

	while (/*root->get_visits() < 30000 && */clock() - tEngine->fin_clock < tEngine->max_time)
	{


		Node *node_history[3 * MAXSIZE2];
		int nnode_hist = 0, pass = 0;
		int simulate_len = 0;
		AmafBoard tamaf = AmafBoard(tEngine->main_goban->get_size());
		tamaf.set_up(tEngine->main_goban->get_side(), tEngine->main_goban->get_size());
		Node *node = root;

		tMain_goban.cpy_goban(tEngine->main_goban);


		EnterCriticalSection(&tEngine->cs); 
    if(clock() - tEngine->fin_clock >= tEngine->max_time){
      LeaveCriticalSection(&tEngine->cs); 
      break;
    }

		while (node->has_childs() && pass < 2) {
			node_history[nnode_hist++] = node;
			node = node->select_child();
			int move = node->get_move();
			if (move == Goban::PASS) pass++;
			else pass = 0;
			tMain_goban.play_move(move);
			tamaf.play(move, ++simulate_len);
		}
		if (node->get_visits() >= EXPAND || node == root) {
			Prior priors[MAXSIZE2 + 1] = { 0 };
			int legal_moves[MAXSIZE2 + 1];
			int nlegal = tMain_goban.legal_moves(legal_moves);
			//main_goban->init_priors(priors);
			tEngine->tree.expand(node, legal_moves, nlegal, priors); //TODO: break if expand fails.
		}
//#if PROTECTED
		LeaveCriticalSection(&tEngine->cs);             /* option to unlock area */
//#endif
		node_history[nnode_hist++] = node;
		int result = tEngine->play_random_game(tEngine->HEAVY, &simulate_len, &tamaf, &tMain_goban); //Black wins.
//#if PROTECTED
		EnterCriticalSection(&tEngine->cs);                           /* option to lock area */

//#endif
#ifdef DEBUG_INFO
		main_goban->print_goban();
		std::cerr << result << "\n";
#endif
		tEngine->main_goban->restore();
		//std::cout << result;
		if (result == -1) continue;
		if (side) result = 1 - result;
		tEngine->back_up_results(result, node_history, nnode_hist, side, &tamaf);
#ifdef DEBUG_INFO
		tree.print();
#endif
		LeaveCriticalSection(&tEngine->cs);          
}
	return 0;
}

void Engine::back_up_results(int result, Node *node_history[], int nnodes, bool side, AmafBoard* tamaf)
{
  for (int i = 0; i < nnodes; i++) {
    node_history[i]->set_results(1-result); 
    node_history[i]->set_amaf(result, *tamaf, side, i+1);
    side = !side;
    result = 1-result;
  }
}

void Engine::print_PV() const
{
  fin_clock = clock() - fin_clock;
  tree.print();
  int nplayouts = tree.get_root()->get_visits();
  std::cerr << "#Playouts: " << nplayouts
            << ", average length: " << rand_movs/nplayouts
            << ", discarded: " << discarded << ", playouts/sec: "
            << (float)nplayouts/fin_clock*CLOCKS_PER_SEC << "\n";
}

float Engine::score(std::vector<int> *dead)
{
  const int PLAYOUTS = 5000;
  int score = 0;
  int score_table[MAXSIZE2+1] = {0};
  for (int i = 0; i < PLAYOUTS; i++) {
    simul_len = 0;
    play_random_game(LIGHT);
    main_goban->score_area(score_table);
    main_goban->restore();
  }
  for (int i = 1; i <= main_goban->get_size2(); i++) {
    if (score_table[i] > PLAYOUTS/2) score_table[i] = 1;
    else if (score_table[i] < -PLAYOUTS/2) score_table[i] = -1;
    else score_table[i] = 0;

    if (dead && main_goban->get_value(i)
        && score_table[i] !=  main_goban->get_value(i)) {
      dead->insert(dead->end(), i);
    }
  }

  for (int i = 1; i <= main_goban->get_size2(); i++) {
    score += score_table[i];
  }
  return score - main_goban->get_komi();
}

void Engine::perft(int max)
{
  for (int i = 0; i < max; i++) {
    simul_len = 0;
    play_random_game(LIGHT);
    std::cerr << "restoring\n";
    main_goban->restore();
  }
}
