#ifndef _MCKP_H_
#define _MCKP_H_

#include"dpforKP.h"

extern int P_inc,C_inc;

void solve(Vector**);
Vector** MCKP_prepare(void);
Vector** Algorithm1(Vector**);
int R_feasible(Vector**,Fraction**,Fraction**);
Vector** relaxation2(Vector**,Fraction*,Node*,int);
void rx_right(Vector*,Fraction*,int);
void rx_left(Vector**,Fraction*,int);
int Rk_pandc(Vector*,Fraction**,Fraction**);
Vector* copy_R(Vector*);
void min_max_slope(Vector**,Fraction**,Fraction**,int);
void calc_penalty(Vector**,Vector**,Fraction**,Fraction**,Fraction*,Fraction*,Fraction*,Fraction*,int);
void update(Vector**,Node*,int);
void all_free(Vector**);

#endif
