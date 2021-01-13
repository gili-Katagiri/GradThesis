#ifndef _DPFOR_KP_H_
#define _DPFOR_KP_H_

#include"Tools/vector.h"
#include"Tools/structures.h"
#include"Tools/penalty.h"

extern Vector **item_list;
extern Penalties **penal_list;
extern Vector **cell_list;
extern Vector **Cand_List;

extern int GroupNum,Max_W;
extern int *ItemsNum;
extern ull LCM;

void prepareStep1(void);
void read_problem(char*);
void read_items(char*, int);
void read_funcs(char*, int);

#endif
