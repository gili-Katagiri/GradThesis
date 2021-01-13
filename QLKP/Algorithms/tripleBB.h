#ifndef _TRIPLE_BB_H_
#define _TRIPLE_BB_H_

#include"../prepare.h"

extern int debug;
extern Vector *all_ws_list;

void solve_Step1(void);
int mk_next_node( Vector**, Vector**, WeightedState*, NamedItem*, Penalties*);
int pw_sort_add( Vector**, WeightedState*);
void drop_out( Vector**);
void reduce_and_LCM(Vector*);
void ws_to_cand(Vector**);
#endif
