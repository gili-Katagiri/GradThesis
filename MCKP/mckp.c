#include<stdio.h>
#include<stdlib.h>
#include"mckp.h"

int P_inc,C_inc;

int main(void){
    int i;
    Fraction *t_profit;
    Vector **R;
    structures_init();
    DP_prepare();
    DP_solve();
    R = MCKP_prepare();
    solve(R);
    if( debug )printf("\n----------------------------Problem Solved!----------------------------\n\n");
    t_profit = frac_init(P_inc,LCM);
    printf("\ntotal_profit="); print_frac(t_profit);
    printf(", total_cost=%d\n\n",C_inc);
    for(i=0;i<Num_Group;i++){ print_solution(DP_solutions[i]); putchar('\n'); }
    return 0;
}

void solve(Vector **R){
    
    if( debug )printf("\n---------------------------------Solve---------------------------------\n\n");
    int i,ifs_idx,P_cp=0;
    Vector **vec;
    Fraction *P_cpr,*W_cpr;
    
    vec = Algorithm1(R); //Algorithm1を実行
    ifs_idx = R_feasible(R,&P_cpr,&W_cpr); //解として成立するか，しないならその原因のindex
    for(i=0;i<Num_Group;i++){ //繰り下げ解の準備 ifs_idx以外の和
        if( i==ifs_idx ) continue;
        Node *node = (Node*)(vec[i]->data);
        P_cp += node->item->profit;
    }
    
    if( ifs_idx == Num_Group ){ //X_CP_r is integer?
        update(vec,NULL,Num_Group);
        free(vec); free(P_cpr); free(W_cpr); all_free(R);
        return ;
    }
    else if( ifs_idx == -1 ){ //W_cpr <= W ? no->retrun;  P_cpr > P_inc ? no->return;
        free(vec); free(P_cpr); free(W_cpr); all_free(R);
        return;
    }
    
    Vector **R_left; //左問題のR 右問題はRを再利用
    Node *right = (Node*)(vec[ifs_idx]->data),*left = (Node*)((vec[ifs_idx]->next)->data); //LP解の左右ノード
    Fraction *border,*min_slope,*max_slope,*cur_slope; //min_slopeはLP解の左側最小　max_slopeは右側最大
    
    border = frac_plus( mul_int(right->x,right->item->weight) , mul_int(left->x,left->item->weight) ,1 );
    if( debug ){ printf("border = "); print_frac(border); putchar('\n'); }
    
    min_max_slope(R,&min_slope,&max_slope,ifs_idx); //Rを辿り最大，最小の傾きを求める ifs_idxは除く
    cur_slope = right->slope; //left~rightの傾き
    if( debug ){ printf("Current slope : "); print_frac(cur_slope); putchar('\n'); }
    
    R_left = relaxation2(R,border,right,ifs_idx); //子問題のRを作成 Rは再利用に留意 無駄となるデータは削除
    P_cp += ((Node*)(R_left[ifs_idx]->data))->item->profit; //整数近似解の完成
    if( debug ){
        printf("\nSeparate :\nRight Side--------------------\n");
        for(i=0;i<Num_Group;i++){ print_nodes(R[i]); putchar('\n'); }
        printf("Left Side---------------------\n");
        for(i=0;i<Num_Group;i++){ print_nodes(R_left[i]); putchar('\n'); }
        printf("------------------------------\n\n");
    }
    //calc_penalty pq+ , pq-
    Fraction *penal_plus,*penal_minus; //子問題における価値の"上昇量" ->下落するためマイナス値
    calc_penalty(R,R_left,&penal_plus,&penal_minus,border,max_slope,min_slope,cur_slope,ifs_idx);
    
    free(cur_slope); free(border); //cur_slope,borderはいらない ここでifs_idxのrightノードは不要な限り全て削除済み
    //max_slope,min_slopeはRのどこかの傾きであるためfreeしない
    if( cmp_left_over(penal_minus,penal_plus) ){ P_cpr=frac_plus(P_cpr,penal_minus,1); free(penal_plus); }
    else { P_cpr = frac_plus(P_cpr,penal_plus,1); free(penal_minus); }
    if( debug ){ printf("P_cpr="); print_frac(P_cpr); printf(", P_inc=%d\n\n",P_inc); }
    if( int_less_frac(P_inc,P_cpr) ){  //P_cp>P_inc
        if( P_cp > P_inc ) update(vec,left,ifs_idx); //update(P_inc); 繰り上げ価値 R_leftを暫定解とする
        free(P_cpr); free(W_cpr); free(vec);
        //二つの子問題をとく
        if( debug ) printf("Solve the Right Side");
        solve(R);
        if( debug ) printf("Solve the Left Side");
        solve(R_left);
    }else{
        if( debug ) printf("More optimal solution is NOT exist at next problem.\n");
        free(P_cpr); free(W_cpr); free(vec); all_free(R); all_free(R_left);
    }
}

Vector** MCKP_prepare(void){
    int i;
    //Vector *vec_ptr;
    Vector **Rk;
    
    P_inc = 0; //近似解の価値の初期値は0ここから上げていく
    Rk = (Vector**)malloc(Num_Group*sizeof(Vector*));
    for( i=0;i<Num_Group;i++){
        Rk[i] = NULL;
        //vec_ptr = DP_solutions[i]; //Candidate List
        vec_append(&Rk[i],(void*)(node_init(null_item,frac_zero)));//null_nodeは使えない 個々のグループで特有のノード
        
        rx_left(&Rk[i],frac_inf,i); //INF以下とすることで全ての擬似アイテムをたどる
        if( debug ){
            printf("\n----------------------------------------------------------------------\n");
            printf("R[%d]'s Lelax.\n",i);
            print_nodes(Rk[i]);
        }
    }
    if( debug ) printf("-----------------------MCKP Programing : READY------------------------\n\n");
    return Rk;
}


Vector** Algorithm1(Vector **R){
    int i,w=0,w_bar,min_idx,yq;
    Node *node;
    Fraction *min;
    Vector **vec;
    vec = (Vector**)malloc(Num_Group*sizeof(Vector*));
    for( i=0;i<Num_Group;i++ ){
        vec[i] = R[i];
        w += ((Node*)(vec[i]->data))->item->weight; //重み和
    }
    w_bar = w-Max_W; //改善するべき重み
    if( w_bar <= 0 ){ return vec; } //これ以上の改善はない
    
    while(1){
        min_idx = -1; min=frac_inf; yq=0;
        for( i=0;i<Num_Group;i++ ){ //傾き最小を見つける
            node = (Node*)(vec[i]->data);
            if( node->slope == frac_inf ) continue; //傾きがINFならその次にノードが存在しない
            if( cmp_left_over(min,node->slope) ){
                min_idx = i;
                min = node->slope;
                yq = node->width;
            }
        }
        
        if( min_idx==-1 ){ //解なし all x->0 総価値が0という扱い Rk_feasibleでは-1が返る
            if( debug )printf("This problem has no solution.\n");
            for(i=0;i<Num_Group;i++){ ((Node*)(vec[i]->data))->x = frac_zero; }
            return vec;
        }
        else if( w_bar==yq ){ //frac_one,frac_zeroの特殊ケース
            ((Node*)(vec[min_idx]->data))->x = frac_zero;
            vec[min_idx] = vec[min_idx]->next;
            ((Node*)(vec[min_idx]->data))->x = frac_one;
            return vec;
        }
        else if( (w_bar-yq) < 0 ){ //LP解が得られた
            ((Node*)(vec[min_idx]->data))->x = frac_init( yq-w_bar , yq );
            ((Node*)((vec[min_idx]->next)->data))->x = frac_init( w_bar , yq );
            return vec; //vec[min_idx]はrightのアドレス
        }
        else{
            w_bar -= yq;
            ((Node*)(vec[min_idx]->data))->x = frac_zero;
            vec[min_idx] = vec[min_idx]->next;
            ((Node*)(vec[min_idx]->data))->x = frac_one;
        }
    }
}

int R_feasible(Vector **R,Fraction **t_profit,Fraction **t_cost){
    int ifs_idx=-1,i,flag;
    *t_profit = frac_init(0,1); *t_cost = frac_init(0,1);
    for( i=0;i<Num_Group;i++ ){
        Fraction *profit,*cost;
        flag = Rk_pandc(R[i],&profit,&cost); //flagが1なら全ての係数が1または0で整数解
        if( debug ){ print_nodes(R[i]);
            printf("total: profit->"); print_frac(profit); putchar(' ');
            printf(" cost->"); print_frac(cost); printf("\n"); }
        *t_profit = frac_plus(*t_profit,profit,1);
        *t_cost = frac_plus(*t_cost,cost,1);
        if( flag==0 ){
            ifs_idx = i;
            if( debug ) printf("Not Integer\n");
        }
        if( debug ) putchar('\n');
    }
    if( debug ){
        printf("\nLP Total: profit->"); print_frac(*t_profit); putchar(' ');
        printf(" cost->"); print_frac(*t_cost); putchar('\n');
    }
    
    if( int_less_frac(Max_W,*t_cost) || int_less_frac(P_inc,*t_profit)==0 ){
        if( debug ) printf("INFEASIBLE Max_W < total_cost:%d, P_inc < t_profit:%d\n",int_less_frac(Max_W,*t_cost),int_less_frac(P_inc,*t_profit) );
        return -1;
    }else if( ifs_idx==-1 ){
        if( debug ) printf("All conditions are met.\n");
        return Num_Group;
    }else{
        if( debug )printf("Class%d is not Integer.\n",ifs_idx);
        return ifs_idx;
    }
}

Vector** relaxation2(Vector **R,Fraction *border,Node *right,int group){
    int i;
    Vector *R_right,**R_left,*vec; //return R_left; right,leftは必ず存在
    Node *node;
    R_right = R[group];
    
    R_left = (Vector**)malloc(Num_Group*sizeof(Vector*));
    for(i=0;i<Num_Group;i++){
        if( i==group ) continue;
        R_left[i] = copy_R(R[i]);
        ((Node*)(R[i]->data))->x = frac_one;
    }
    
    if( (Node*)(R_right->data) != right ){
        //R_rightのnextがrightになるまで
        while( (Node*)((R_right->next)->data) != right){ R_right = R_right->next; }
        //R_leftはR_rightの隣の隣
        R_left[group] = (R_right->next)->next;
        vec = R_right->next; node = (Node*)(vec->data);
        free(node->x); free(node); free(vec);
        R_right->next = NULL; //next->node->x とnext->nodeはfreeしてOK vectorとしてもいらん slopeはcur_slope
    }else{
        R_left[group] = R_right->next;
        node = (Node*)(R_right->data);
        free( node->x ); //xを再定義する必要がないのはこのノードが先頭だからである
        R_right->next = NULL; //node->xはいらない node自体は再利用 vectorも再利用
    }
    vec = R_left[group]; node=(Node*)(vec->data); free(node->x);
    //ここでright,leftのxは二つとも消えているとしていい さらにR[group]の先頭xはfrac_oneに
    ((Node*)(R[group]->data))->x = frac_one;
    //分離完了
    
    rx_right(R_right,border,group); //先頭が変わらない
    rx_left(&R_left[group],border,group); //先頭が変わる
    
    return R_left;
}

void rx_right(Vector *R,Fraction *border,int group){
    Vector *Nk=DP_solutions[group] ;
    Vector *n,*nn; Node *node,*pre_node;
    Item *right = ((Node*)(R->data))->item;
    
    //rightになるまで進める rightの次から判断すればよい
    while( ((Candidate*)(Nk->data))->item != right ){ Nk=Nk->next; }
    n = Nk->next; //nはNULLにならない
    
    Item *first=right,*second=((Candidate*)(n->data))->item,*third;
    
    while( (int_less_frac(second->weight,border))==0 ){ //second==borderの場合も入る
        
        nn = n->next; third=((Candidate*)(nn->data))->item; //nnはNULLにならない
        if( int_less_frac(third->weight,border) ){
            node = node_init(second,frac_zero);
            vec_append(&R,(void*)node);
            break;
        }
        else if( turn_left(first,second,third) ){
            node = node_init(second,frac_zero);
            vec_append(&R,(void*)node);
            first = second;
        }
        n=nn; second=third;
    }
    
    pre_node = (Node*)(R->data);
    if( R->next == NULL ) pre_node->width = 0;
    while( R->next!=NULL ){
        R = R->next;
        node = (Node*)(R->data);
        //pre_nodeのy,rを計算
        pre_node->width = (pre_node->item->weight) - (node->item->weight);
        pre_node->slope = frac_init( ( pre_node->item->profit - node->item->profit ),pre_node->width);
        pre_node = node;
    }
    pre_node->slope = frac_inf; //最後は傾き無限大
}

void rx_left(Vector **R,Fraction *border, int group){
    Vector *Nk=DP_solutions[group], *left_add=NULL;
    Item *left = ((Node*)((*R)->data))->item;
    Vector *n,*nn; Node *node,*pre_node;
    
    //Nk < border
    while( int_less_frac( ((Candidate*)(Nk->data))->item->weight,border) == 0 ){ Nk=Nk->next; }
    
    Item *first=( (Candidate*)(Nk->data) )->item,*second,*third;
    if( first==left ){ ((Node*)((*R)->data))->x=frac_one; return ;}
    //left_nodeのx=frac_zeroが確定
    ((Node*)((*R)->data))->x=frac_zero;
    node = node_init(first,frac_one); //firstに対応するNodeを作成
    vec_prepend(&left_add,(void*)node);
    
    n = Nk->next; //nはNULLにならない first!=left
    second = ( (Candidate*)(n->data) )->item;
    
    while( second!=left ){
        nn = n->next; third=((Candidate*)(nn->data))->item; //nnはNULLにならない
        
        if( turn_left(first,second,third) ){
            node = node_init(second,frac_zero);
            vec_prepend(&left_add,(void*)node);
            first = second;
        }
        n=nn; second = third;
    }
    pre_node=(Node*)((*R)->data);
    while( left_add!=NULL ){
        node = (Node*)(left_add->data);
        //nodeのy,rを計算
        node->width = (node->item->weight) - (pre_node->item->weight);
        node->slope = frac_init( ( node->item->profit - pre_node->item->profit ),node->width);
        vec_prepend(R,(void*)node);
        vec_predel(&left_add);
        pre_node = node;
    }
}

Vector* copy_R(Vector *R){
    Vector *vec,*cp_vec=NULL;
    Node *node,*cp_node;
    Fraction *slope;
    vec= R;
    
    while(vec!=NULL){
        node = (Node*)(vec->data);
        if( node->x == frac_one ) node->x = frac_zero; //R側のxを初期化 if文である必要はないが1以外でのデバッグにはなる
        
        cp_node = node_init(node->item,frac_zero);
        cp_node->width = node->width;
        
        slope = node->slope;
        if( slope==frac_inf ) cp_node->slope = frac_inf; //傾きはノードごとに変化するので注意
        else cp_node->slope = frac_init(slope->molecule,slope->denominator);
        
        vec_append(&cp_vec,(void*)cp_node);
        vec = vec->next;
    }
    ((Node*)(cp_vec->data))->x = frac_one; //先頭xを1に
    return cp_vec;
}

int Rk_pandc(Vector *Rk,Fraction **r_profit,Fraction **r_cost){
    *r_profit = frac_init(0,1); *r_cost = frac_init(0,1); //消される運命
    int flag = 1;
    while( Rk!=NULL ){
        Node *node = (Node*)(Rk->data);
        Item *item = node->item;
        if( node->x!=frac_zero ){
            Fraction *p_trans,*c_trans;
            p_trans = mul_int(node->x,item->profit);
            c_trans = mul_int(node->x,item->weight);
            *r_profit = frac_plus(*r_profit,p_trans,1);
            *r_cost = frac_plus(*r_cost,c_trans,1);
            if( node->x!=frac_one ) flag = 0;
            else { return flag; }
        }
        Rk = Rk->next;
    }
    return flag;
}

void min_max_slope(Vector **R,Fraction **min,Fraction **max,int group){
    int i;
    Vector *vec; Node *node;
    *min = frac_inf; *max = frac_zero;
    for(i=0;i<Num_Group;i++){
        if( i==group ) continue;
        vec = R[i]; node = (Node*)(vec->data);
        if( node->x==frac_one ){
            if( cmp_left_over(*min,node->slope) )*min=node->slope;
            continue;
        }
        while( ((Node*)(vec->next))->x == frac_zero ){ vec = vec->next; node=(Node*)(vec->data); }
        if( cmp_left_over(node->slope,*max) ) *max=node->slope;
        vec = vec->next; node=(Node*)(vec->data);
        if( cmp_left_over(*min,node->slope) ) *min=node->slope;
    }
    if( debug ){
        printf("Max-slope="); print_frac(*max);
        printf("  Min-slope="); print_frac(*min); putchar('\n');
    }
}

void calc_penalty(Vector **R,Vector **R_left,Fraction **penal_plus,Fraction **penal_minus,Fraction *border,Fraction *max_slope,Fraction* min_slope,Fraction *cur_slope,int ifs_idx){
    Vector *min_right=R[ifs_idx],*max_left=R_left[ifs_idx];
    while( min_right->next != NULL ){ min_right = min_right->next; }
    if( debug ){ printf("Right Min : \n"); print_node((Node*)(min_right->data));
        printf("Left Max : \n"); print_node((Node*)(max_left->data)); }
    Node *right_min=(Node*)(min_right->data), *left_max=(Node*)(max_left->data);
    Fraction *range,*tmp;
    
    tmp = frac_init(left_max->item->weight,1);
    //print_frac(tmp); putchar('\n');
    range = frac_minus(border , tmp); free(tmp);
    //print_frac(range); putchar('\n');
    tmp = frac_minus(max_slope , cur_slope);
    //printf("%p ",&tmp); print_frac(tmp); putchar('\n');
    *penal_minus = frac_mul(range,tmp); free(range); free(tmp);
    
    tmp = frac_init(right_min->item->weight,1);
    //print_frac(tmp); putchar('\n');
    range = frac_minus(border,tmp); free(tmp);
    //print_frac(range); putchar('\n');
    tmp = frac_minus(min_slope,cur_slope);
    //print_frac(tmp); putchar('\n');
    *penal_plus = frac_mul(range,tmp); free(range); free(tmp);
    
    if( debug ){
        printf("P_minus = "); print_frac(*penal_minus); putchar(' ');
        printf("P_plus = "); print_frac(*penal_plus); putchar('\n');
    }
}

void update(Vector **R,Node *left,int ifs_idx){
    int i,t_profit=0,t_cost=0;
    Vector *Nk;
    for(i=0;i<Num_Group;i++){
        Item *item;
        Nk=DP_solutions[i];
        if( i!=ifs_idx ){ item = ((Node*)(R[i]->data))->item; }
        else{ item = left->item; }
        while( ((Candidate*)(Nk->data))->item != item ){ ((Candidate*)(Nk->data))->use=0; Nk=Nk->next; }
        t_profit += (item->profit); t_cost += (item->weight);
        ((Candidate*)(Nk->data))->use=1; Nk=Nk->next;
        while( Nk!=NULL ){ ((Candidate*)(Nk->data))->use=0; Nk=Nk->next; }
    }
    P_inc = t_profit; C_inc = t_cost;
    if( debug ) printf("Update : total_profit=%d, total_cost=%d\n",t_profit,t_cost);
}

void all_free(Vector **R){
    int i;
    if( debug )printf("All Free R----------------\n");
    for(i=0;i<Num_Group;i++){
        while( R[i]!=NULL ){ //R_leftではnull_nodeじゃない可能性
            if( debug ){ printf("free: %p\n",R[i]); print_node((Node*)(R[i]->data)); }
            Node *node=(Node*)(R[i]->data);
            if( node->x!=frac_zero && node->x!=frac_one ) free(node->x);
            if( node->slope!=frac_inf ) free(node->slope);
            if( node!=null_node ) free(node);
            vec_predel(&R[i]);
        }
    }
    if( debug )printf("All Free END--------------\n");
}
