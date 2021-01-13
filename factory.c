#include<stdio.h>
#include<stdlib.h>

int rand_quot(int min,int max){ return (rand()%(max-min+1)) + min ; }

void mk_item(int n,int W,int group_num,int min_w,int max_w,int min_p,int max_p){
    int seed; //乱数シード
    int i,k,w,p;
    FILE* fp;
    printf("input seed:"); scanf("%d",&seed);
    char filename[] = "test_data.txt";
    fp = fopen(filename,"w");
    if( fp==NULL ){ fprintf(stderr, "ERROR:Can\'t open file\n"); exit(1); }
    srand(seed);
    fprintf(fp, "%d %d %d\n",n,group_num+1,W);
    for(i=0;i<n;i++){
        k=rand_quot(0,group_num);
        w=rand_quot(min_w,max_w);
        p=rand_quot(min_p,max_p);
        fprintf(fp, "%d %d %d\n",k,w,p);
    }
    fclose(fp);
}


int main(void){
    int n=5000,W=10000; //nは全てのアイテムの総数 Wは問題の最大容量
    int g_num=10; g_num-=1; //グループ数
    int min_w=10,max_w=500; //アイテムの重みの最小，最大値 min_w <= weight < max_w に留意
    int min_p=10,max_p=500; //アイテムの価値の最小，最大値 min_p <= profit < max_p に留意
    mk_item(n,W,g_num,min_w,max_w,min_p,max_p);
    return 0;
}

