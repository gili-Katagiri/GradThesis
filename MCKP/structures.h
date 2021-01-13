#ifndef _STRUCTURES_H_
#define _STRUCTURES_H_

#include"tools.h"

extern Fraction *frac_zero,*frac_one;

void structures_init(void);


typedef struct{
    int count; // これはちょっとだけ気持ち悪い Num_Itemとキャラ被り
    int bugjet; // 気持ち悪い
    int max_cost; // これも気持ち悪い
}Status;
Status* status_init();
void print_status(Status*);


typedef struct{
    int profit;
    int weight;
}Item;
Item* item_init(int, int);
void print_item(Item*);
int signed_area(Item*,Item*,Item*);
int turn_left(Item*,Item*,Item*);
extern Item *null_item;


typedef struct{
    Item *item;
    char *name;
}NamedItem;
NamedItem* nameditem_init(char*,int,int);
void print_nameditem(NamedItem*);
extern NamedItem *null_nitem;


typedef struct Cell{
    NamedItem *item;
    struct Cell *next;
    Fraction *penalty;
    int count;
    int total_profit;
}DP_Cell;
DP_Cell* dpcell_init(NamedItem*,DP_Cell*,Fraction*,int,int);
void save_used(DP_Cell*);
void print_items_of_cell(DP_Cell*);
extern DP_Cell *null_cell;


typedef struct{
    Item *item;
    DP_Cell *cell;
    char use; //0,1
}Candidate;
Candidate* cand_init(Item*,DP_Cell*);
void print_cand(Candidate*);
extern Candidate *null_cand;


typedef struct{
    Item *item;
    Fraction *x;
    int width;
    Fraction *slope;
}Node;
Node* node_init(Item*,Fraction*);
void print_node(Node*);
extern Node *null_node;

#endif
