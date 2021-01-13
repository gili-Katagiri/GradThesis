#ifndef _STRUCTURES_H_
#define _STRUCTURES_H_

#include"tools.h"

extern Fraction *frac_zero,*frac_one;

void structures_init(void);

typedef struct{
    ull profit;
    int weight;
}Item;
Item* item_init(ull, int);
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
    NamedItem *item; //最後に追加したアイテム
    struct Cell *next; //追加前のセル
    Fraction *penalty; //このセルの協和度
    int count; //このセルのアイテム数
    int total_profit; //純粋総価値
}State;
State* state_init(NamedItem*,State*,Fraction*,int,int);
State* state_add(State*, NamedItem*, Fraction*);
Fraction* eval(State*);
void save_used(State*);
void print_items_of_cell(State*);
extern State *null_cell;

typedef struct {
    State *state;
    int weight;
}WeightedState;
WeightedState* ws_init(State*, int);
extern WeightedState *null_ws;

typedef struct{
    Item *item;
    State *cell;
    char use; //0,1
}Candidate;
Candidate* cand_init(Item*,State*);
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
