
#include<stdio.h>
#include<stdlib.h>
#include"prepare.h"
//#include"Algorithms/simpleDP.h"
//#include"Algorithms/simpleBB.h"
//#include"Algorithms/hybrid.h"
//#include"Algorithms/doubleBB.h"
#include"Algorithms/tripleBB.h"
#include"Algorithms/mckp.h"

int main(void){
    //char *dirname = "problem";
    
    structures_init();
    prepareStep1();
    
    solve_Step1(); // select Algorithm
    
    if( debug ){ printf("Debug print All Candidates\n\n");
        //Vector *vec_ptr;
        for(int i=0;i<GroupNum;i++){
            print_candidates(Cand_List[i]);
            /*
            vec_ptr = Cand_List[i];
            while( vec_ptr!=NULL ){
                Candidate *cand = (Candidate*)(vec_ptr->data);
                print_item(cand->item);
                vec_ptr = vec_ptr->next;
            }
            */
            printf("----------------------------------------------------------------------\n\n");
        }
    }
    
    int i;
    Fraction *t_profit;
    Vector **R;
    
    R = MCKP_prepare();
    debug=0;
    
    solve(R);
    
    printf("\n----------------------------Problem Solved!----------------------------\n\n");
    t_profit = frac_init(P_inc,LCM);
    printf("\ntotal_profit="); print_frac(t_profit);
    printf(", total_cost=%d\n\n",C_inc);
    for(i=0;i<GroupNum;i++){ print_solution(Cand_List[i]); putchar('\n'); }
    
    return 0;
}

