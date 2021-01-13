#include<stdio.h>
#include<stdlib.h>
#include<time.h>

#include"doubleBB.h"

int debug=1;

fraction_list *limitter;
Vector *states_list=NULL; // for reduce states
Vector *problems=NULL;
Vector *last_in;

void solve_Step1(void){
    if( debug ) printf("\n----- Step1 by doubleBB -----\n");
    
    clock_t start,end;
    
    Vector **effective_cells; // WeightedState List
    effective_cells = (Vector**)malloc(GroupNum*sizeof(Vector*));
    problem *prob_ptr;
    
    for(int group=0; group<GroupNum; group++){
        if( debug ){ printf("processing group: %d...", group); fflush(stdout); }
        start = clock();
        
        effective_cells[group] = NULL;
        vec_prepend( &effective_cells[group], (void*)null_ws );
        
        limitter = fl_init();
        
        prob_ptr=problem_init(null_cell, item_list[group], 0); // root problem
        last_in = vec_append( &problems, (void*)prob_ptr); // quick enqueue
        
        int count=0;
        while( problems!=NULL ){
            prob_ptr = (problem*)(problems->data);
            branch_and_bound(effective_cells[group], prob_ptr, penal_list[group]);
            free( problems->data );
            vec_predel( &problems );
            count++;
        }
        printf("Number of nodes: %d\n",count);
        vec_delete(&(penal_list[group]->args), 1);
        free(penal_list[group]);
        
        if( debug )printf("complete! ");
        
        end = clock();
        if( debug )printf("time: %.2f [sec]\n", (double)(end-start)/CLOCKS_PER_SEC);
        
        for( int i=0; i<limitter->size; i++) free( limitter->frac_list[i] );
        free(limitter->frac_list);
        free(limitter);
        
        reduce_and_LCM(effective_cells[group]);

    }
    
    // calc LCM & delete unused state
    for( int group=0; group<GroupNum; group++){
        ws_to_cand(effective_cells[group], group); // Interface
        vec_delete( &(effective_cells[group]->next), 1); // be careful null_ws
    }
    if( debug )printf("LCM: %llu\n", LCM);
}

void branch_and_bound( Vector *ws_vec, problem *prob_ptr, Penalties *penal){
    // get problem params
    State *state = prob_ptr->state; Fraction *current_val = eval(state);
    Vector *item_vec = prob_ptr->items;
    int weight = prob_ptr->weight;
    
    problem *new_prob_ptr; // prepare for child problem
    
    if( item_vec==NULL ) return; // end point
    if( cmp_left_over( get_frac(limitter, state->count), eval(state))==0 ){ //cor2
        free(current_val);
        return;
    }
    // next node's prepare
    Vector *next_item = item_vec->next;
    
    // add(right) node test
    NamedItem *item = (NamedItem*)(item_vec->data); // get item
    Fraction *penalty = get_penalty(penal, state->count+1); // get penalty
    // make add state
    State *add = state_add(state, item, penalty);
    Fraction *add_val = eval(add); // get add status value
    int add_weight = weight+item->item->weight;
    
    // compare
    if( cmp_left_over(add_val, current_val)==0 ){ // state_val >= add_val
        // update upper bound
        update_fl(limitter, current_val, state->count);
        free(add);
        return;
    }else if( add_weight > Max_W ){
        free(add); // child nodes are also over the capacity
    }else {
        vec_prepend(&states_list, add); // new states: for reduce used memory
        effective_state(ws_vec, add, add_weight); // add test
        //branch_and_bound(ws_vec, add, next_item, penal, add_weight); // next node(right)
        new_prob_ptr = problem_init(add, next_item, add_weight);
        vec_append( &last_in, (void*)new_prob_ptr);
    }
    
    free(current_val); free(add_val);
    new_prob_ptr = problem_init( state, next_item, weight);
    last_in = vec_append( &last_in, (void*)new_prob_ptr);
    //branch_and_bound(ws_vec, state, next_item, penal, weight); // next node(left)
    
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


problem *problem_init(State *state, Vector *items, int weight){
    problem *prob_ptr;
    prob_ptr = (problem*)malloc(sizeof(problem));
    if( prob_ptr==NULL ){ fprintf(stderr, "ERROR: prob_init\n"); exit(1); }
    prob_ptr->state = state;
    prob_ptr->items = items;
    prob_ptr->weight = weight;
    
    return prob_ptr;
}
