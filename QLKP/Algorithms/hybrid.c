#include<stdio.h>
#include<stdlib.h>
#include<limits.h>
#include<time.h>

#include"hybrid.h"

int debug=1;
Vector *states_list = NULL;

void solve_Step1(void){
    if( debug ) printf("\n-----  Step1 by hybrid  -----\n");
    clock_t start, end;
    
    for( int group=0; group<GroupNum; group++){
    
        if( debug ){ printf("processing group: %d...", group); fflush(stdout); }
        start = clock();
        
        Vector *items = item_list[group]; // get item list
        int n = ItemsNum[group], m = Max_W;
        Penalties *penal = penal_list[group];
        
        // make table
        State **cells = (State**)malloc((m+1)*(n+1)*sizeof(State*));
        for( int i=0;i<=n;i++) cells[indexat(i,0,m)] = null_cell;
        for( int j=1;j<=m;j++) cells[indexat(0,j,m)] = null_cell;
        
        // limitter for BB
        int *count_limit = (int*)malloc((m+1)*sizeof(int));
        for( int j=0; j<=m; j++){ count_limit[j] = INT_MAX; }
        count_limit[m] = 0; // sentinel
        
        // fill
        fill_table(cells, items, penal, n, m, count_limit);
        if( debug ) printf("filled table! ");
        
        // linear relaxation for mckp
        draw_out(cells, n, m, group);
        
        end = clock();
        if( debug ) printf("time: %.2f [sec]\n", (double)(end-start)/CLOCKS_PER_SEC);
        
        // post process
        vec_delete( &(penal->args), 1 );
        free(penal); free(count_limit); free(cells);
        
        
    }
    // LCM is completed >> input Candidates profit
    if( debug )printf("LCM: %llu\n", LCM);
    for( int group=0; group<GroupNum; group++ ){
        // input candidate's profit
        input_cand_profits(group);
    }
}

void fill_table(State **cells, Vector *items, Penalties *penal, int n, int m, int count_limit[]){
    
    for( int i=1; i<=n; i++ ){
        NamedItem *item;
        int item_weight; ull item_profit;
        item = (NamedItem*)(items->data);
        item_profit = item->item->profit;
        item_weight = item->item->weight;
        
        State *lim_cell = null_cell; // lower limit cell follows row
        
        int sp_m;
        int above_bool = 1;
        for( sp_m=m-item_weight; sp_m>=0; sp_m--){
            State *cell = cells[indexat(i-1,sp_m,m)];
            if( count_limit[sp_m] > cell->count ){
                break;
            }
        }
        
        sp_m += item_weight;
        
        for( int j=1; j<=sp_m; j++ ){ // standard process
            int this = indexat(i,j,m);
            State *above, *upper_left, *add;
            
            above = cells[this-m-1]; // get above cell
            
            if( j<item_weight ){
                cells[this] = above;
                lim_cell = above;
                continue;
            }else{
                int slide = j-item_weight;
                upper_left = cells[indexat(i-1, slide, m)];
                add = state_add(upper_left, item, get_penalty(penal, upper_left->count+1));
                
                // if add < upper_left update count_limit
                limit(upper_left, add, slide, count_limit);
                
                cells[this] = select(add,above,lim_cell);
                lim_cell = cells[this];
                
                above_bool = (cells[this] == above);
            }
        }
            
        for( int j=sp_m+1; j<=m; j++ ){ // add sealed
            if( above_bool ){ // pre cell is upper cell -> No competetion
                for( int k=j; k<=m; k++){
                    cells[indexat(i,k,m)] = cells[indexat(i-1,k,m)];
                }
                break; // end this row
            }
            else{
                State *above = cells[indexat(i-1,j,m)];
                Fraction *above_val, *lim_val;
                above_val = eval(above); lim_val = eval(lim_cell);
                
                if( cmp_left_over(above_val,lim_val) ){
                    cells[indexat(i,j,m)] = above;
                    above_bool = 1;
                }else{
                    cells[indexat(i,j,m)] = lim_cell;
                }
                
                free(above_val); free(lim_val);
            }
            // pre cell is upper cell -> No competetion
        }
        items = items->next;
    }
}

State *select(State *add, State *above, State *left){
    // calc penlized value
    Fraction *above_val,*left_val,*add_val;
    above_val = eval(above);
    left_val  = eval(left);
    add_val   = eval(add);
    State *cells[2] = { above, left };
    Fraction *values[2] = { above_val, left_val };
    
    // set current cell
    State *current=add; Fraction *current_val = add_val;
    
    // compare and update
    for( int i=0;i<2;i++ ){
        if( frac_equal(current_val,values[i]) ){ // current_val == values[i]
            if( cells[i]->count < current->count ){ // change current
                current = cells[i];
                current_val = values[i];
            }
        }else if( cmp_left_over( values[i], current_val ) ){
            // current_val < values[i]
            current = cells[i];
            current_val = values[i];
        }
    }
    
    // post process
    free(above_val); free(left_val); free(add_val);
    if( current != add ) free(add); // add cell is not selected
    else vec_prepend(&states_list, (void*)add); // for reduce cells
    
    return current;
}

void draw_out(State **cells, int n, int m ,int group){
    // Linear Relaxation 1
    
    Cand_List[group] = NULL;
    Fraction *min_val = frac_init(0,1); // NOT frac_zero: freed in this function
    
    vec_append(&Cand_List[group], (void*)null_cand);
    
    int start=indexat(n,0,m);
    for( int j=1;j<=m;j++ ){
        
        State *cell = cells[start+j];
        Fraction *cell_val = eval(cell);
        
        if( cmp_left_over(cell_val,min_val) ){
            
            // prepare a container, before input profit
            Item *item = item_init(0,j);
            Candidate *cand = cand_init(item,cell);
            vec_prepend(&Cand_List[group], (void*)cand);
            
            // min_val update
            free(min_val);
            min_val = cell_val;
            //max_valuable_weight = j;
            
            save_used(cell); // markes using cell
            LCM = lcm(LCM,cell->penalty->denominator); //update LCM
            
        }else{ free(cell_val); }
    }
    free(min_val);
    
    // delete unnecessary cells
    reduce_cells();
    
}

void reduce_cells(void){
    
    while( states_list!=NULL ){
        State *cell_ptr = (State*)(states_list->data);
        if( cell_ptr->count > 0 ){
            free(cell_ptr);
            vec_predel( &states_list );
        }else{
            states_list = states_list->next;
        }
    }
    
    vec_delete(&states_list, 0);
}

void input_cand_profits(int group){
    Vector *cand_list = Cand_List[group];
    
    while( cand_list!=NULL ){
        Candidate *cand_ptr=(Candidate*)(cand_list->data);
        // calc profit
        State *cell = (State*)(cand_ptr->cell);
        Fraction *cell_val = eval(cell);
        Fraction *profit = mul_int(cell_val, LCM);
        
        Candidate *cand = (Candidate*)(cand_list->data);
        
        // input profit
        cand->item->profit = profit->molecule;
        if( profit->denominator != 1 ){
            fprintf(stderr, "LCM:%llu is irregal!\n", LCM);
        }
        
        // next
        cand_list = cand_list->next;
        
        // post process
        free(cell_val); free(profit);
    }
    
}

void limit(State *pre_cell, State *add_cell, int k, int count_limit[]){
    Fraction *pre_val, *add_val;
    pre_val = eval(pre_cell); add_val = eval(add_cell);
    
    if( cmp_left_over( add_val, pre_val )==0 ){ // add_val <= pre_val
        int lim = add_cell->count;
        while( count_limit[k] > lim ){
            count_limit[k++] = lim;
        }
    }
    
}

