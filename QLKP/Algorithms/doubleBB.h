#ifndef _DOUBLE_BB_H_
#define _DOUBLE_BB_H_

#include"../prepare.h"

extern int debug;
extern fraction_list *limitter;
extern Vector *states_list;
extern Vector *problems;

typedef struct{
    State *state;
    Vector *items;
    int weight;
}problem;
problem *problem_init(State*, Vector*, int);


void solve_Step1(void);
void branch_and_bound( Vector*, problem*, Penalties*);
void effective_state(Vector*, State*, int);
void drop_out(Vector*);
void reduce_and_LCM(Vector*);
void ws_to_cand(Vector*, int);
void equal_process(WeightedState*, Vector**);
#endif
