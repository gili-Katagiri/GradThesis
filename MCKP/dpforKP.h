#ifndef _DPFOR_KP_H_
#define _DPFOR_KP_H_
#include"vector.h"

extern Vector **item_list,**DP_solutions;
extern Status **status;
extern int LCM,debug,Num_Item,Num_Group,Max_W;

void DP_prepare(void);
void DP_solve(void);
void read_file(void);
Vector* part_solve(int);
void apply_table(DP_Cell**,Vector*,Fraction**,int,int,int);
Vector* relaxation1(int,int,DP_Cell**);
void reduce_cells(int,int,DP_Cell**);
int idx(int,int,int);
int calc_lcm(void);

#endif
