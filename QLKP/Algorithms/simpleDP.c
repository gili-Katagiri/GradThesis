#include<stdio.h>
#include<stdlib.h>
#include<time.h>

#include"simpleDP.h"

int debug=1;
Vector *states_list=NULL;

void solve_Step1(void){
    if( debug ) printf("\n----- Step1 by simpleDP -----\n");
    
    for( int group=0;group<GroupNum;group++ ){
        clock_t start, end;
        if( debug ){ printf("processing group: %d...",group); fflush(stdout); }
        start = clock();
        Vector *items = item_list[group]; // get items
        int n = ItemsNum[group],m=Max_W;
        Penalties *penal = penal_list[group]; //get penalties
        
        // make table
        State **cells = (State**)malloc((m+1)*(n+1)*sizeof(State*));
        for( int i=0;i<=n;i++) cells[(m+1)*i] = null_cell;
        for( int j=1;j<=m;j++) cells[j] = null_cell;
        // fill
        fill_table(cells,items,penal,n,m);
        if( debug ) printf("filled table! ");
        
        // linear relaxation for MCKP
        draw_out(cells, n, m, group);
        
        // post process
        // delete penalties but penalty is alive
        vec_delete(&(penal->args), 1);
        free(penal); free(cells);
        
        end = clock();
        
        if( debug )printf("time: %.2f [sec]\n", (double)(end-start)/CLOCKS_PER_SEC);
    }
    
    // LCM is completed >> input Candidates profit
    if( debug )printf("LCM: %llu\n", LCM);
    for( int group=0; group<GroupNum; group++ ){
        // input candidate's profit
        input_cand_profits(group);
    }
}

void fill_table(State **cells,Vector *items,Penalties *penal,int n,int m){
    
    for( int i=1;i<=n;i++){
        NamedItem *item; // get item
        int item_weight;
        ull item_profit;
        item = (NamedItem*)(items->data);
        item_weight = item->item->weight;
        item_profit = item->item->profit;
        
        for( int j=1;j<=m;j++ ){
            
            State *above, *left, *add;
            
            above = cells[indexat(i-1,j,m)]; // above cell
            if( j<item_weight ){
                cells[indexat(i,j,m)] = above;
                continue;
            }
            int slide = j - item_weight;
            left = cells[indexat(i-1,slide,m)]; //preadd cell(tmp)
            // make add cell
            add = state_add(left, item, get_penalty(penal, left->count+1));
            left = cells[indexat(i,j-1,m)]; // left cell
            
            // select most valuable cell
            cells[indexat(i,j,m)] = select(add,above,left);
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
