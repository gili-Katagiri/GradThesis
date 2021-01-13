#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include"prepare.h"

#define DEFAULT_PATH_LENGTH 32
#define ITEM_NAME_LENGTH 8

Vector **item_list; // (NamedItem List) * GroupNum
Penalties **penal_list; // functions of penalty
Vector **cell_list; // Effective Cell List
Vector **Cand_List; // sham items list for MCKP

int GroupNum,Max_W;
int *ItemsNum;
ull LCM;

void prepareStep1(void){
    char dirname[DEFAULT_PATH_LENGTH];
    printf("input problem directory name: "); scanf("%s", dirname);
    //char *dirname = "experiment";
    char paramspath[2*DEFAULT_PATH_LENGTH];
    printf("Problem read from \"%s/\"\n",dirname);
    
    // read GroupNum, Max_W
    sprintf(paramspath,"%s/problem_params",dirname);
    read_problem(paramspath);
    printf("Number of groups: %d\n",GroupNum);
    printf("Capacity of Knapsack: %d\n",Max_W);
    
    // read item_list, penalties_list
    item_list = (Vector**)malloc(GroupNum*sizeof(Vector*));
    ItemsNum = (int*)malloc(GroupNum*sizeof(int));
    penal_list = (Penalties**)malloc(GroupNum*sizeof(Penalties*));
    
    for( int i=0;i<GroupNum;i++ ){
        printf("group: %d...", i);
        char grouppath[2*DEFAULT_PATH_LENGTH];
        sprintf(grouppath,"%s/group%d",dirname,i);
        read_items(grouppath,i);
        read_funcs(grouppath,i);
        
        //if(i==1){
        printf("Number of Items: %d\n",ItemsNum[i]);
        //print_nameditems(item_list[i]);
        print_penalties(penal_list[i]);
        //}
    }
    
    LCM = 1; // fraction->integer variable
    cell_list = (Vector**)malloc(GroupNum*sizeof(Vector*));
    Cand_List = (Vector**)malloc(GroupNum*sizeof(Vector*));
    
}

void read_problem(char *paramspath){
    // read GroupNum, Max_W
    FILE *fp_pp;
    fp_pp = fopen(paramspath,"r");
    if( fp_pp==NULL ){
        fprintf(stderr, "Failure: open \"%s\"\n",paramspath);
    }
    fscanf(fp_pp,"%d,%d",&GroupNum,&Max_W);
    fclose(fp_pp);
}

void read_items(char *grouppath, int group){
    // read items from Group[i]
    char itempath[2*DEFAULT_PATH_LENGTH+5];
    sprintf(itempath,"%s/items",grouppath); // items path
    
    FILE *fp_item;
    fp_item = fopen(itempath,"r");
    if( fp_item==NULL ){
        fprintf(stderr, "Failure: open \"%s\"\n",itempath);
    }
    
    item_list[group] = NULL; //set NULL
    fscanf(fp_item,"%d",&ItemsNum[group]); // set Number of Item
    
    for( int i=1;i<=ItemsNum[group];i++ ){
        // define item's parameter
        int profit, weight;
        fscanf(fp_item, "%d, %d", &profit, &weight);
        char *name = (char*)malloc(ITEM_NAME_LENGTH*sizeof(char));
        snprintf(name,ITEM_NAME_LENGTH,"%d_%d",group,i);
        
        // make
        NamedItem *item;
        item = nameditem_init(name,profit,weight);
        
        // insert following profit order
        iteminsert_forprofit(&item_list[group],item);
    }
    
    fclose(fp_item);
}

void read_funcs(char *grouppath, int group){
    char funcpath[2*DEFAULT_PATH_LENGTH+4];
    char identity;
    
    sprintf(funcpath,"%s/func",grouppath);
    
    penal_list[group] = penal_init();
    FILE *fp_func;
    fp_func = fopen(funcpath,"r");
    if( fp_func==NULL ){
        fprintf(stderr, "Failure: open \"%s\"\n",funcpath);
    }
    fscanf(fp_func,"%c",&identity);
    
    int s,slide;
    
    switch (identity) {
        case 'f':
            fscanf(fp_func,"%d, %d",&s, &slide);
            func_init_f(penal_list[group],s, slide);
            break;
        case 'l':
            fscanf(fp_func,"%d, %d",&s, &slide);
            func_init_l(penal_list[group],s,slide);
            break;
        default:
            fprintf(stderr,"Error: %c is NOT feasible\n", identity);
            break;
    }
    fclose(fp_func);
}
