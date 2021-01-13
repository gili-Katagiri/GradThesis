#include<stdio.h>
#include<stdlib.h>
#include<time.h>

#include"simpleBB.h"

int debug=1;

int upper_weight;
Vector *states_list=NULL; // for reduce states

void solve_Step1(void){
    if( debug ) printf("\n----- Step1 by simpleBB -----\n");
    
    clock_t start,end;
    
    Vector **effective_cells; // WeightedState List
    effective_cells = (Vector**)malloc(GroupNum*sizeof(Vector*));
    
    //for( int group=9; group>=0; group-- ){
    for(int group=0; group<GroupNum; group++){
        if( debug ){ printf("processing group: %d...", group); fflush(stdout); }
        start = clock();
        
        upper_weight = Max_W;
        effective_cells[group] = NULL;
        vec_prepend( &effective_cells[group], (void*)null_ws );
        
        branch_and_bound(effective_cells[group], null_cell, item_list[group], penal_list[group], 0);
        
        vec_delete(&(penal_list[group]->args), 1);
        free(penal_list[group]);
        if( debug )printf("complete! ");
        
        end = clock();
        if( debug )printf("time: %.2f [sec]\n", (double)(end-start)/CLOCKS_PER_SEC);
        
        reduce_and_LCM(effective_cells[group]);
    }
    
    // calc LCM & delete unused state
    for( int group=0; group<GroupNum; group++){
        ws_to_cand(effective_cells[group], group); // Interface
        vec_delete( &(effective_cells[group]->next), 1); // be careful null_ws
    }
    if( debug )printf("LCM: %llu\n", LCM);
}

void branch_and_bound( Vector *ws_vec, State *state, Vector *item_vec, Penalties *penal, int weight){
    
    if( item_vec==NULL ) return; // end point
    
    // next node's prepare
    Vector *next_item = item_vec->next;
    
    // add(right) node test
    NamedItem *item = (NamedItem*)(item_vec->data); // get item
    Fraction *penalty = get_penalty(penal, state->count+1); // get penalty
    
    // make add state
    State *add = state_add(state, item, penalty);
    Fraction *current_val = eval(state), *add_val = eval(add); // get status value
    int add_weight = weight+item->item->weight;
    
    // compare
    if( cmp_left_over(add_val, current_val)==0 ){ // state_val >= add_val
        //upper_weight = weight; // update upper bound: only breadth-first
        free(add);
        return;
    }else if( add_weight > upper_weight ){
        free(add); // child nodes are also over the capacity
    }else {
        vec_prepend(&states_list, add); // new states: for reduce used memory
        effective_state(ws_vec, add, add_weight); // add test
        branch_and_bound(ws_vec, add, next_item, penal, add_weight); // next node(right)
    }
    
    free(current_val); free(add_val);
    
    branch_and_bound(ws_vec, state, next_item, penal, weight); // next node(reft)
    
}

void effective_state(Vector *ws_vec, State *add_state, int add_weight){
    Vector *next_vec = ws_vec->next;
    
    WeightedState *add_ws, *current_ws=null_ws, *next_ws;
    add_ws = ws_init(add_state, add_weight);
    
    while( next_vec != NULL ){ // find position of new WeightedState
        current_ws = (WeightedState*)(ws_vec->data);
        next_ws = (WeightedState*)( next_vec->data );
        
        if( add_ws->weight == next_ws->weight ){ // equation => replace
            equal_process(add_ws, &(ws_vec->next) );
            drop_out(ws_vec->next);
            return;
        }else if( add_ws->weight < next_ws->weight ) break; // get insert position
        ws_vec = next_vec; next_vec = ws_vec->next;
    }
    
    current_ws = (WeightedState*)(ws_vec->data);
    
    Fraction *add_val, *current_val;
    add_val = eval(add_state);
    current_val = eval(current_ws->state);
    
    if( cmp_left_over( add_val, current_val )==1 ){ // current_val < add_val => insert
        // insert
        // make add_vec
        Vector *add_vec=vec_init((void*)add_ws, next_vec);
        ws_vec->next = add_vec;
        
        // relaxation
        drop_out(add_vec); // delete unused vector
    }
    
    free(add_val); free(current_val);
}

void drop_out(Vector *ws_vec){
    Vector *next_vec=ws_vec->next, *del_vec=NULL;
    WeightedState *current_ws, *next_ws;
    Fraction *current_val, *next_val;
    
    current_ws = (WeightedState*)(ws_vec->data);
    current_val = eval(current_ws->state);
    
    //printf("Add: \n"); print_items_of_cell(current_ws->state); putchar('\n');
    
    while( next_vec != NULL ){
        next_ws = (WeightedState*)(next_vec->data);
        next_val = eval(next_ws->state);
        //printf("rm?: \n"); print_items_of_cell(next_ws->state); putchar('\n');
        if( cmp_left_over(next_val, current_val)==0 ){ // current_val >= next_val
            ws_vec->next = next_vec->next; // update next
            vec_prepend(&del_vec, (void*)next_vec); // delete flag
            next_vec = ws_vec->next;
            free(next_val);
        }else{ free(next_val); break; }
    }
    
    vec_delete(&del_vec, 1); // delete
    free(current_val);
}

void reduce_and_LCM(Vector *ws_vec){
    State *state_ptr;
    WeightedState *ws_ptr;
    
    while( ws_vec!=NULL ){ // select effective state and set 0
        ws_ptr = (WeightedState*)(ws_vec->data);
        state_ptr=ws_ptr->state;
        if( state_ptr->count!=0 ) save_used(state_ptr); // save
        LCM = lcm(LCM, state_ptr->penalty->denominator); // update LCM
        ws_vec=ws_vec->next;
    }
    
    while( states_list!=NULL ){ // free non-used state
        state_ptr=(State*)(states_list->data);
        if( state_ptr->count>0 ){
            //print_items_of_cell(state_ptr); putchar('\n');
            free(state_ptr);
        }
        states_list = states_list->next;
    }
    vec_delete(&states_list,0); // delete flame
}

void ws_to_cand(Vector *ws_vec, int group){
    vec_prepend( &Cand_List[group], (void*)null_cand); // for MCKP
    ws_vec = ws_vec->next; // initial data is null_ws
    while( ws_vec!=NULL ){
        WeightedState *ws_ptr=(WeightedState*)(ws_vec->data);
        // get profit
        Fraction *state_val, *profit;
        state_val = eval(ws_ptr->state);
        profit = mul_int(state_val,LCM);
        Item *item=item_init(profit->molecule, ws_ptr->weight); // make item
        
        //make Candidate and prepend for MCKP
        Candidate *cand_ptr;
        cand_ptr = cand_init(item, ws_ptr->state);
        vec_prepend( &Cand_List[group], (void*)cand_ptr);
        ws_vec = ws_vec->next;
        
        // post process
        free(state_val); free(profit);
    }
    
}

void equal_process(WeightedState *add_ws, Vector **next_vec){
    
    WeightedState *next_ws = (WeightedState*)((*next_vec)->data);
    Fraction *add_val=eval(add_ws->state), *next_val=eval(next_ws->state);
    
    // add_weight == next_weight
    if( cmp_left_over(add_val, next_val)==1 ){ // replace
        Vector *add_vec=vec_init((void*)add_ws, (*next_vec)->next );
        *next_vec = add_vec;
    }
    // post process
    free(add_val); free(next_val);
}
