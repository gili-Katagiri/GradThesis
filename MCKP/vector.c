#include<stdio.h>
#include<stdlib.h>
#include"vector.h"

Vector* vec_init(void *data, Vector *next){
    Vector *vec_ptr;
    vec_ptr = (Vector*)malloc(sizeof(Vector));
    if(vec_ptr==NULL){ fprintf(stderr,"ERROR:Vector init\n"); exit(1); }
    vec_ptr->data = data;
    vec_ptr->next = next;
    return vec_ptr;
}
void vec_prepend(Vector **vec_ptrptr, void *data){
    Vector *vec_ptr;
    vec_ptr = vec_init(data,*vec_ptrptr);
    if( vec_ptr==NULL ){ fprintf(stderr,"ERROR:Vec prepend\n"); exit(1); }
    *vec_ptrptr = vec_ptr;
}
void vec_append(Vector **vec_ptrptr,void *data){
    Vector *vec_ptr;
    vec_ptr = vec_init(data,NULL);
    while( *vec_ptrptr!=NULL ){ vec_ptrptr = &( (*vec_ptrptr)->next ); }
    *vec_ptrptr = vec_ptr;
}
void iteminsert_forcost(Vector **vec_ptrptr, NamedItem *item){
    Vector *vec_ptr;
    int cost = item->item->weight;
    while( *vec_ptrptr!=NULL ){
        NamedItem *nitem;
        nitem = (NamedItem*)((*vec_ptrptr)->data);
        if( nitem->item->weight<cost) break;
        vec_ptrptr = &( (*vec_ptrptr)->next );
    }
    vec_ptr = vec_init((void*)item,*vec_ptrptr);
    *vec_ptrptr = vec_ptr;
}
void iteminsert_forprofit(Vector **vec_ptrptr,NamedItem *item){
    Vector *vec_ptr;
    int profit = item->item->profit;
    while( *vec_ptrptr!=NULL ){
        NamedItem *nitem;
        nitem = (NamedItem*)((*vec_ptrptr)->data);
        if( nitem->item->profit < profit ) break;
        vec_ptrptr = &( (*vec_ptrptr)->next );
    }
    vec_ptr = vec_init((void*)item,*vec_ptrptr);
    *vec_ptrptr = vec_ptr;
}
void* vec_get(Vector *vec_ptr, int idx){
    int i;
    for(i=0;i<idx;i++){
        vec_ptr = vec_ptr->next;
    }
    return vec_ptr->data;
}
void vec_predel(Vector **vec_ptrptr){
    Vector *vec_ptr;
    if( *vec_ptrptr!=NULL ){
        vec_ptr = (*vec_ptrptr)->next;
        free(*vec_ptrptr);
        *vec_ptrptr = vec_ptr;
    }else{ fprintf(stderr,"ERROR : Vector predel\n"); exit(1); }
}
int cell_is_in_vec(DP_Cell *cell,Vector *vec_ptr){
    DP_Cell *cell_ptr;
    while( vec_ptr!=NULL ){
        cell_ptr = (DP_Cell*)(vec_ptr->data);
        if( cell_ptr==cell ) return 1;
        vec_ptr = vec_ptr->next;
    }
    return 0;
}
void print_nameditems(Vector *nitem_vec){
    Vector *vec_ptr;
    NamedItem *item_ptr;
    for( vec_ptr=nitem_vec; vec_ptr!=NULL; vec_ptr=vec_ptr->next){
        item_ptr = (NamedItem*)(vec_ptr->data);
        print_nameditem(item_ptr);
    }
}
void print_candidates(Vector *vec_ptr){
    Candidate *cand;
    while( vec_ptr!=NULL ){ //null_candとすればnullが表示されない extern推奨
        cand = (Candidate*)(vec_ptr->data);
        print_cand(cand);
        vec_ptr = vec_ptr->next;
    }
}
void print_nodes(Vector *vec_ptr){
    Node *node;
    while( vec_ptr!=NULL ){
        node = (Node*)(vec_ptr->data);
        print_node(node);
        vec_ptr = vec_ptr->next;
    }
}

void print_solution(Vector *vec_ptr){
    Candidate *cand;
    while( vec_ptr!=NULL ){
        cand = (Candidate*)(vec_ptr->data);
        if( cand->use==1 ){ print_cand(cand); }
        vec_ptr = vec_ptr->next;
    }
}
