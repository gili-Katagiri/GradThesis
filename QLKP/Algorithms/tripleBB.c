#include<stdio.h>
#include<stdlib.h>
#include<time.h>

#include"tripleBB.h"

int debug = 1;
Vector *all_ws_list = NULL;

void solve_Step1(void){
    if( debug ) printf("\n----- Step1 by tripleBB -----\n");
    
    clock_t start,end;
    
    Vector **effective_cells; // WeightedState List
    effective_cells = (Vector**)malloc(GroupNum*sizeof(Vector*));
    
    for(int group=0; group<GroupNum; group++){
        if( debug ){ printf("processing group: %d...", group); fflush(stdout); }
        start = clock();
        
        effective_cells[group] = NULL;
        vec_prepend( &effective_cells[group], (void*)null_ws ); // init
        
        // pre process
        Vector *node_ptr = NULL; // process vector
        Vector **node_ptrptr = &(node_ptr);
        vec_prepend( node_ptrptr, (void*)null_ws);
        Vector *bb_ctrl = NULL; // vector of node list
        vec_prepend( &bb_ctrl, (void*)(*node_ptrptr));

        //branch and bound
        while( item_list[group] != NULL ){
            NamedItem *item = (NamedItem*)(item_list[group]->data); // get item
            // initial bb_ctrl data is NULL
            if( (Vector*)(bb_ctrl->data) != NULL ) vec_prepend( &bb_ctrl, NULL);
            
            Vector *bb_counter = bb_ctrl; // itr
            Vector **pre_node_list = (Vector**)( &(bb_counter->data) ); // pointer
            bb_counter = bb_counter->next;
            
            while( bb_counter!=NULL ){
                // current target vector
                node_ptrptr = (Vector**)( &(bb_counter->data) );
                
                while( *node_ptrptr!=NULL ){
                    WeightedState *current_node = (WeightedState*)((*node_ptrptr)->data);
                    int del_flag;
                    // branch and bound
                    del_flag = mk_next_node( &(effective_cells[group]), pre_node_list, current_node, item, penal_list[group]);
                    if( del_flag ){ // limit operation
                        free( *node_ptrptr );
                        *node_ptrptr = (*node_ptrptr)->next;
                    }else{
                        node_ptrptr = &( (*node_ptrptr)->next );
                    }
                }
                pre_node_list = (Vector**)( &(bb_counter->data) );
                bb_counter = bb_counter->next;
            }
            item_list[group] = item_list[group]->next;
        }
        // calc LCM & delete unused state
        reduce_and_LCM(effective_cells[group]);
        
        if( debug ) printf("complete! ");
        end = clock();
        if( debug )printf("time: %.2f [sec]\n", (double)(end-start)/CLOCKS_PER_SEC);
        
        // post process
        if( debug ){
            print_penalties(penal_list[group]);
        }
        vec_delete( &(penal_list[group]->args), 1);
        free( penal_list[group] );
    }
    if( debug )printf("LCM: %llu\n", LCM);
    ws_to_cand(effective_cells);
    
}

int mk_next_node( Vector **ws_vec, Vector **pre_node_list, WeightedState *ws_ptr, NamedItem *item, Penalties *penal){
    State *state = ws_ptr->state; Fraction *current_val = eval(state);
    int weight = ws_ptr->weight, delete_flag=0;
    Fraction *penalty = get_penalty(penal, state->count+1);
    
    // make add state
    State *add = state_add( state, item, penalty);
    Fraction *add_val = eval(add);
    int add_weight = weight+item->item->weight;
    
    if( cmp_left_over(add_val, current_val)==0 ){ // state_val >= add_val
        free(add); delete_flag = 1; // cor1
    }else if( add_weight > Max_W ){
        free(add); // child nodes are also over the capacity
    }else {
        // vec_prepend(&states_list, add); // new states: for reduce used memory
        WeightedState *add_ws_ptr = ws_init( add, add_weight );
        pw_sort_add(pre_node_list, add_ws_ptr);
        pw_sort_add(ws_vec, add_ws_ptr);
        vec_prepend( &all_ws_list, add_ws_ptr);
    }
    
    free(current_val); free(add_val);
    return delete_flag;
}

int pw_sort_add(Vector **ws_ptrptr, WeightedState *add_ws){
    
    if( *ws_ptrptr==NULL ){
        vec_prepend( ws_ptrptr, (void*)add_ws); return 1;
    }
    WeightedState *current_ws = (WeightedState*)((*ws_ptrptr)->data);
    Fraction *current_val, *add_val;
    current_val = eval(current_ws->state); add_val = eval(add_ws->state);
    
    if( cmp_left_over( current_val, add_val)==0 ){ // add_val >= current_val
        free(current_val); free(add_val);
        vec_prepend( ws_ptrptr, (void*)add_ws);
        drop_out(ws_ptrptr);
        return 1;
    }
    
    //Vector *ws_ptr = *ws_ptrptr;
    Vector *next_vec = (*ws_ptrptr)->next;
    WeightedState *next_ws;
    Fraction *next_val;
    
    while( next_vec!=NULL ){
        next_ws = (WeightedState*)(next_vec->data);
        next_val = eval(next_ws->state);
        
        if( cmp_left_over(next_val, add_val)==0 ){ // add_val >= next_val
            free(next_val);
            break;
        }
        
        free(current_val);
        current_ws = next_ws; current_val = next_val;
        ws_ptrptr = &( (*ws_ptrptr)->next );
        next_vec = next_vec->next;
    }
    free(add_val); free(current_val);
    
    // current_val > add_val
    if( current_ws->weight > add_ws->weight ){
        Vector *add_vec = vec_init( (void*)add_ws, next_vec);
        (*ws_ptrptr)->next = add_vec; // insert
        drop_out( &((*ws_ptrptr)->next) );
        return 1;
    }else{ return 0; }
}

void drop_out( Vector **ws_ptrptr){
    Vector *next_vec = (*ws_ptrptr)->next, *del_vec=NULL;
    WeightedState *current_ws;
    Fraction *current_val, *next_val;
    current_ws = (WeightedState*)((*ws_ptrptr)->data);
    current_val = eval(current_ws->state);
    // current_val >= next_val
    while( next_vec!=NULL ){
        WeightedState *next_ws = (WeightedState*)(next_vec->data);
        next_val = eval(next_ws->state);
        //print_items_of_cell(current_ws->state);
        //print_items_of_cell(next_ws->state);
        if( frac_equal(current_val, next_val) ){
            if( current_ws->weight >= next_ws->weight ){
                // delete current
                free( *ws_ptrptr );
                *ws_ptrptr = next_vec;
                free(next_val);
                return;
            }else{
                //delete next
                (*ws_ptrptr)->next = next_vec->next;
                vec_prepend( &del_vec, (void*)next_vec);
                next_vec = next_vec->next;
            }
            free(next_val);
        }else if( current_ws->weight <= next_ws->weight ){
            (*ws_ptrptr)->next = next_vec->next;
            vec_prepend( &del_vec, (void*)next_vec);
            next_vec = next_vec->next;
            free(next_val);
        }else{ break; }
    }
    free(current_val);
    vec_delete(&del_vec, 0);
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
    
    while( all_ws_list!=NULL ){ // free non-used state
        ws_ptr = (WeightedState*)(all_ws_list->data);
        state_ptr=ws_ptr->state;
        if( state_ptr->count>0 ){
            //print_items_of_cell(state_ptr); putchar('\n');
            free(ws_ptr);
            free(state_ptr);
        }
        all_ws_list = all_ws_list->next;
    }
    vec_delete(&all_ws_list,0); // delete flame
}

void ws_to_cand(Vector **effective_cells){
    for( int group=0; group<GroupNum; group++){
        Vector *ws_vec = effective_cells[group], **cand_vec;
        cand_vec = &(Cand_List[group]);
        *cand_vec = NULL;
        while( ws_vec!=NULL ){
            WeightedState *ws_ptr=(WeightedState*)(ws_vec->data);
            // get cand profit
            Fraction *state_val, *profit;
            state_val = eval(ws_ptr->state);
            profit = mul_int(state_val,LCM);
            // make item
            Item *item=item_init(profit->molecule, ws_ptr->weight);
            //make Candidate and append for MCKP
            Candidate *cand_ptr;
            cand_ptr = cand_init(item, ws_ptr->state);
            //cand_vec = vec_append( cand_vec, (void*)cand_ptr);
            vec_append(cand_vec, (void*)cand_ptr);
            cand_vec = &((*cand_vec)->next);
            ws_vec = ws_vec->next;
            
            free(state_val); free(profit);
        }
        vec_append(cand_vec, (void*)null_cand);
    }
}
