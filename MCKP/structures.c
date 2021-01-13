#include<stdio.h>
#include<stdlib.h>
#include"structures.h"

static Item NULL_ITEM={0,0};
Item *null_item;

static NamedItem NULL_NITEM = { NULL,NULL };
NamedItem *null_nitem;

static DP_Cell NULL_CELL={ NULL,NULL,NULL,0,0 };
DP_Cell *null_cell;

static Candidate NULL_CAND={ NULL,NULL,0 };
Candidate *null_cand;

static Node NULL_NODE = { NULL,NULL,0,NULL };
Node *null_node;

void structures_init(void){
    fractions_init();
    null_item = &NULL_ITEM;
    null_nitem = &NULL_NITEM; null_nitem->item = null_item;
    null_cell = &NULL_CELL;
    null_cell->item = null_nitem; null_cell->penalty=frac_zero;
    null_cand = &NULL_CAND;
    null_cand->item = null_item; null_cand->cell = null_cell;
    null_node = &NULL_NODE;
    null_node->item = null_item; null_node->x=frac_zero; null_node->slope=frac_inf;
}

Status* status_init(){
    Status *status_ptr;
    status_ptr = (Status*)malloc(sizeof(Status));
    if( status_ptr==NULL ){ fprintf(stderr, "ERROR : Status init\n"); exit(1); }
    status_ptr->count = 0;
    status_ptr->bugjet = 0;
    status_ptr->max_cost = 0;
    return status_ptr;
}
void print_status(Status *status){ printf("ItemCounter:%d, MaxCount:%d, MaxCost:%d\n",
                                          status->count,status->bugjet,status->max_cost); }

Item* item_init(int profit, int weight){
    Item *item_ptr;
    item_ptr = (Item*)malloc(sizeof(Item));
    if( item_ptr==NULL ){fprintf(stderr,"ERROR:Item init\n"); exit(1); }
    item_ptr->profit = profit;
    item_ptr->weight = weight;
    return item_ptr;
}
void print_item(Item *item){ printf("profit=%3d, weight=%3d\n",item->profit,item->weight); }
int signed_area(Item *a,Item *b,Item *c){
    int ax,ay,bx,by,cx,cy;
    ax=a->weight; ay=a->profit; bx=b->weight; by=b->profit; cx=c->weight; cy=c->profit;
    return ax*by+bx*cy+cx*ay-ay*bx-by*cx-cy*ax ;
}
int turn_left(Item *a,Item *b,Item *c){
    int area=signed_area(a,b,c);
    if( area>0 ) return 1;
    else return 0;
}

NamedItem *nameditem_init(char *name,int profit,int weight){
    NamedItem *nitem_ptr;
    Item *item_ptr = item_init(profit,weight);
    nitem_ptr = (NamedItem*)malloc(sizeof(NamedItem));
    if( nitem_ptr==NULL ){fprintf(stderr,"ERROR:NamedItem init\n"); exit(1); }
    nitem_ptr->item = item_ptr;
    nitem_ptr->name = name;
    return nitem_ptr;
}
void print_nameditem(NamedItem *nitem){
    printf("Name=%10s : ",nitem->name); print_item(nitem->item);
}

DP_Cell* dpcell_init(NamedItem *item,DP_Cell *next,Fraction *penalty,int count,int total_profit){
    DP_Cell *dp_ptr;
    dp_ptr = (DP_Cell*)malloc(sizeof(DP_Cell));
    if( dp_ptr==NULL ){fprintf(stderr,"ERROR:DP_Cell init\n"); exit(1); }
    dp_ptr->item = item;
    dp_ptr->next = next;
    dp_ptr->penalty = penalty;
    dp_ptr->count = count;
    dp_ptr->total_profit = total_profit;
    return dp_ptr;
}
void save_used(DP_Cell *cell_ptr){ //使用されているcellのcountを0にする
    DP_Cell *cell;
    for( cell=cell_ptr; cell!=null_cell; cell=cell->next){ cell->count = 0; }
}
void print_items_of_cell(DP_Cell *cell_ptr){
    DP_Cell *cell=cell_ptr;
    while( cell!=NULL ){
        print_nameditem(cell->item);
        cell = cell->next;
    }
}

Candidate* cand_init(Item *item,DP_Cell *cell){
    Candidate *cand_ptr;
    cand_ptr = (Candidate*)malloc(sizeof(Candidate));
    if( cand_ptr==NULL ){ fprintf(stderr,"ERROR:Candidate init\n"); exit(1); }
    cand_ptr->item=item;
    cand_ptr->cell=cell;
    cand_ptr->use = 0;
    return cand_ptr;
}
void print_cand(Candidate *cand_ptr){
    printf("DP_Cell address : %p  Use?:%d\n",cand_ptr->cell,cand_ptr->use);
    print_item(cand_ptr->item);
    print_items_of_cell(cand_ptr->cell);
}

Node* node_init(Item *item,Fraction *x){
    Node *node_ptr;
    node_ptr = (Node*)malloc(sizeof(Node));
    if( node_ptr==NULL ){ fprintf(stderr, "ERROR:Node init\n"); exit(1); }
    node_ptr->item = item;
    node_ptr->x = x;
    node_ptr->width=0;
    node_ptr->slope=frac_inf;
    return node_ptr;
}

void print_node(Node *nd_ptr){
    print_item(nd_ptr->item);
    printf("Variable:"); print_frac(nd_ptr->x);
    printf(" y=%d, r=",nd_ptr->width); print_frac(nd_ptr->slope); putchar('\n');
}

