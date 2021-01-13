/*
 プログラムに気持ち悪い点がいくつかある．まずペナルティは関数化するべきである．1次関数のみ対応すれば良いと作ったが，現状これはまずい．
 それに伴って，構造体，statusがよくないことをしている．1次関数であればこれは効率的に働く可能性があるが，汎用性にかけており，
 現状気味の悪さを演出するだけとなっている．
 まずペナルティの改善についてだが，グループごとに定義されることから，もっと汎用性に富んだ方式とすべきだ．これはLCMの計算も含まれる．
 グループごとに関数を持つのがベストなような気がするが，それはCで実現できるのかが不明．
 よって代わりに，ある程度のモデル関数を使用できるようにし，パラメータを設定させる．
 個々のパラメータは関数内でstaticな変数とし，solve_partの始めにmallocで定義，そのグループを解き終えた時解放する．
 ペナルティは動的に作成され，一度作られたペナルティは再利用される．
 個々の関数モデルへのアクセスについては，なんらかの識別子とif文で定義できる．
 オーバヘッドも気になるため，念のため関数を持つことができないかは調べてみるべき．
 LCMの計算については，relaxation1()内での更新がベストに思える．
 特に，save_used()では，整数化に必要なペナルティをもれなく無駄なく触ることができる．
*/

#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include"dpforKP.h"

Vector **item_list; //named
Status **status;
Vector **DP_solutions; //Linear Relaxation 1

int LCM; //ペナルティの整数化定数
int debug=0;
int Num_Item,Num_Group,Max_W; //アイテム数，グループ数，最大許容量
const char filename[] = "test.txt";

void DP_prepare(void){ // 下準備
    int i,j;
    read_file();
    DP_solutions = (Vector**)malloc(Num_Group*sizeof(Vector*));
    for( i=0;i<Num_Group;i++){
        int m=0; Vector *tmp;
        
        printf("Group[%d]\n",i);
        print_nameditems(item_list[i]);
        //これは気持ち悪い
        status[i]->bugjet = (status[i]->count)/2; //とり合えず総量の半分程度にしておく
        tmp = item_list[i];
        for(j=0;j<status[i]->bugjet;j++){
            m+=( ((NamedItem*)(tmp->data))->item->weight);
            tmp = tmp->next;
        }
        status[i]->max_cost = m; //これも気持ち悪い
        if( debug ){ print_status(status[i]); putchar('\n'); }
    }
    LCM = calc_lcm();
    if( debug ){
        printf("NumOfItems=%d, NumOfGroups=%d, CapaOfKnapsack=%d, LCM=%d\n\n",Num_Item,Num_Group,Max_W,LCM);
        printf("----------------------Dynamic Programing : READY----------------------\n\n");
    }
}

void DP_solve(void){
    int i;
    for(i=0;i<Num_Group;i++){
        DP_solutions[i] = part_solve(i);
        if( debug ) printf("\n----------------------------------------------------------------------\n\n");
    }
    if( debug ){
        printf("Debug print All Candidates\n\n");
        for(i=0;i<Num_Group;i++){
            print_candidates(DP_solutions[i]);
            printf("----------------------------------------------------------------------\n\n");
        }
    }
}

void read_file(void){
    int i,k,w,p;
    FILE *fp;
    NamedItem *item;
    fp = fopen(filename,"r");
    if( fp==NULL ){ fprintf(stderr,"ERROR:Can't open file\n"); exit(1); }
    if( debug ) printf("\nItem and Problem Data from \'%s\'\n",filename);
    fscanf(fp,"%d %d %d",&Num_Item,&Num_Group,&Max_W);
    
    item_list = (Vector**)malloc(Num_Group*sizeof(Vector*));
    status = (Status**)malloc(Num_Group*sizeof(Status*));
    
    for( i=0;i<Num_Group;i++ ){
        item_list[i] = NULL;
        status[i] = status_init();
    }
    while( fscanf(fp,"%d %d %d",&k,&w,&p)!=EOF ){
        (status[k]->count)+=1;
        char *name = (char*)malloc(8*sizeof(char));
        snprintf(name,8,"%d_%d",k,status[k]->count);
        item = nameditem_init(name,p,w);
        //iteminsert_forcost(&item_list[k],item);
        iteminsert_forprofit(&item_list[k],item);
    }
    fclose(fp);
}

Vector* part_solve(int group){
    if( debug ) printf("Solve Group[%d]\'s DP.\n",group);
    DP_Cell **cells; Vector *vec=item_list[group];
    int i,j,n=status[group]->count,bugjet=status[group]->bugjet,mc=min(status[group]->max_cost,Max_W);
    Fraction **penaltys;
    
    cells = (DP_Cell**)malloc((n+1)*(mc+1)*sizeof(DP_Cell*)); //Cellアドレスの作成
    for( i=0;i<=n;i+=1) cells[idx(i,0,mc)] = null_cell;
    for( j=0;j<=mc;j++) cells[j] = null_cell;
    
    //ここが気持ち悪い元凶．関数化するべきだし，対応しているのが1次関数のみ．気持ち悪い．
    //分数型をapply_tableではcellが分数型を保持することになり，呼び出すごとに作成するのは確かに効率が悪い．
    //が，ある程度採用個数には限度があるのもわかるだろう．作られた分数はヒープ領域に動的に確保していくくらいでいい．
    penaltys = (Fraction**)malloc((bugjet+1)*sizeof(Fraction*)); //Penaltyの設定
    penaltys[0] = frac_zero;
    penaltys[1] = frac_one;
    for(i=1;i<=bugjet-1;i++){ penaltys[i+1] = frac_init(bugjet-i,bugjet); } //とりあえず線形
    
    if( debug ){
        printf("Definition Complete.\n");
        print_status(status[group]);
        print_nameditems(vec); printf("penaltys: ");
        for(i=0;i<=bugjet;i++){ print_frac(penaltys[i%(bugjet+1)]); putchar(i==bugjet?'\n':' '); }
    }
    
    apply_table(cells,vec,penaltys,n,bugjet,mc);
    vec = relaxation1(n,mc,cells);
    reduce_cells(n,mc,cells);
    free(cells); free(penaltys);
    return vec;
}

void apply_table(DP_Cell **cells,Vector *vec,Fraction **penaltys,int n,int bugjet,int mc){
    int i,j;
    for( i=1;i<=n;i++ ){
        NamedItem *item; Fraction *min_val=frac_init(0,1);
        int mv_j,item_w,item_p;
        item = (NamedItem*)(vec->data);
        item_w = item->item->weight;
        item_p = item->item->profit;
        for( j=1;j<=mc;j++ ){
            //printf("i=%d, j=%d\n",i,j);
            Fraction *above_val,*cell_val;
            DP_Cell *cell,*above,*left;
            above = cells[idx(i-1,j,mc)];
            above_val = mul_int(above->penalty,above->total_profit);
            if( j < item_w ){
                cells[idx(i,j,mc)]=above;
                free(min_val); min_val = above_val;
                continue;
            }
            mv_j = j - item_w;
            left = cells[idx(i-1,mv_j,mc)];
            cell = dpcell_init(item,left,penaltys[(left->count+1)%(bugjet+1)],
                               left->count+1,left->total_profit+item_p);
            cell_val = mul_int(cell->penalty,cell->total_profit);
            if( cmp_left_over(cell_val,above_val) ) {
                if( cmp_left_over(cell_val,min_val) ){ cells[idx(i,j,mc)] = cell; free(min_val); min_val=cell_val; }
                else{ cells[idx(i,j,mc)]=cells[idx(i,j-1,mc)]; free(cell); free(cell_val); }
                free(above_val);
            }else{
                if( cmp_left_over(above_val,min_val) ){ cells[idx(i,j,mc)]=above; free(min_val); min_val=above_val; }
                else{ cells[idx(i,j,mc)]=cells[idx(i,j-1,mc)]; free(above_val); }
                free(cell); free(cell_val);
            }
        }
        free(min_val);
        vec = vec->next;
    }
}

Vector* relaxation1(int item_num,int max_cost,DP_Cell **cells){
    Vector *vec_ptr=NULL; vec_prepend(&vec_ptr,(void*)null_cand);
    Fraction *min;
    Item *item;
    int j;
    min = frac_init(0,1); //minはfreeされる frac_zeroはfreeさせないためにあえてmalloc
    
    for( j=1;j<=max_cost;j++ ){
        DP_Cell *cell=cells[idx(item_num,j,max_cost)];
        Fraction *cell_val = mul_int(cell->penalty,cell->total_profit);
        if( cmp_left_over(cell_val,min) ){
            Fraction *profit; Candidate *cand;
            profit = mul_int(cell_val,LCM);
            item = item_init(profit->molecule,j); //MCKP用の擬似itemの作成 null_item->itemが使える
            free(profit);
            
            //N_kの作成
            cand = cand_init(item,cell);
            //Vectorに追加
            vec_prepend(&vec_ptr,(void*)cand);
            //使用しているcellにマーク
            save_used(cell);
            
            free(min); min=cell_val;
        }else{ free(cell_val); }
    }
    free(min);
    return vec_ptr;
}

void reduce_cells(int item_num,int max_cost,DP_Cell **cells){
    //不要となったcellの解放
    int i,j,count=0;
    Vector *vec_ptr=NULL;
    DP_Cell *cell;
    for(i=1;i<=item_num;i++) for(j=1;j<=max_cost;j++){
        cell = cells[idx(i,j,max_cost)];
        if( cell->count!=0 && cell_is_in_vec(cell,vec_ptr)==0 ) vec_prepend(&vec_ptr,(void*)cell);
    }
    while(vec_ptr!=NULL){
        free((DP_Cell*)(vec_ptr->data));
        vec_predel(&vec_ptr);
        count++;
    }
    if( debug) printf("Non-used DP_Cell is deleted : %d \n",count);
}


int idx(int i,int j,int max_cost){ return i*(max_cost+1)+j; }

int calc_lcm(void){ //ペナルティ仕様を変更するならここも
    int i,l = 1;
    for(i=0;i<Num_Group;i++){ l = lcm(l,status[i]->bugjet); }
    return l;
}

