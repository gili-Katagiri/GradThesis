#ifndef _SIMPLE_DP_H_
#define _SIMPLE_DP_H_

#include"../prepare.h"

extern int debug;

void solve_Step1(void);
void fill_table(State**,Vector*,Penalties*,int,int);
State *select(State*, State*, State*);
void draw_out(State**, int, int, int);
void reduce_cells();
void input_cand_profits(int);

#endif
