#ifndef _SIMPLE_BB_H_
#define _SIMPLE_BB_H_

#include"../prepare.h"

extern int debug;
extern int upper_weight;
extern Vector *states_list;

void solve_Step1(void);
void branch_and_bound(Vector*, State*, Vector*, Penalties*, int);
void effective_state(Vector*, State*, int);
void drop_out(Vector*);
void reduce_and_LCM(Vector*);
void ws_to_cand(Vector*, int);
void equal_process(WeightedState*, Vector**);
#endif
