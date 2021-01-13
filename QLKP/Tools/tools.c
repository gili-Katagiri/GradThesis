#include<stdio.h>
#include<stdlib.h>
#include<limits.h>

#include"tools.h"

#define INF LLONG_MAX

int gcd(ull a,int b){
    int r=a%b;
    if( r==0 ) return b;
    else return gcd(b,r);
}
ull lcm(ull a,int b){
    ull c=a*b,g=gcd(a,b);
    return c/g;
}
int min(int a,int b){
    if( a>b ) return b;
    else return a;
}
int indexat(int row, int column, int max_column){
    return row*(max_column+1)+column;
}

static Fraction FRAC_ZERO={0,1},FRAC_ONE={1,1},FRAC_INF={INF,1};
Fraction *frac_zero,*frac_one,*frac_inf;

Fraction* frac_init(ull molecule,int denominator){
    Fraction *frac_ptr;
    int t;
    frac_ptr = (Fraction*)malloc(sizeof(Fraction));
    if( frac_ptr==NULL ){ fprintf(stderr, "ERROR : Frac init\n"); exit(1); }
    t = gcd(molecule,denominator);
    frac_ptr->molecule = molecule/t;
    frac_ptr->denominator = denominator/t;
    if( frac_ptr->denominator<0 ){ frac_ptr->molecule*=-1; frac_ptr->denominator*=-1; }
    return frac_ptr;
}

Fraction *frac_mul(Fraction *a,Fraction *b){
    Fraction *res_ptr;
    res_ptr = frac_init( (a->molecule) * (b->molecule) , (a->denominator) * (b->denominator) );
    return res_ptr;
}
Fraction *mul_int(Fraction* frac_ptr, ull s){
    ull molecule;
    int denominator;
    int t;
    molecule=frac_ptr->molecule; denominator=frac_ptr->denominator;
    t = gcd(s, denominator);
    s = s/t;
    return frac_init(molecule*s,denominator/t);
}
Fraction *frac_plus(Fraction *a,Fraction *b,int del_flag){
    Fraction *frac_ptr,*a_trans,*b_trans;
    int t;
    t = lcm(a->denominator,b->denominator);
    a_trans = mul_int(a,t); b_trans = mul_int(b,t);
    frac_ptr = frac_init( (a_trans->molecule) + (b_trans->molecule) ,t);
    free(a_trans); free(b_trans);
    if( del_flag ){ free(a); free(b); }
    return frac_ptr;
}

Fraction *frac_minus(Fraction *a,Fraction *b){
    Fraction *b_trans,*frac_ptr;
    b_trans = frac_init( -(b->molecule) , b->denominator );
    frac_ptr = frac_plus(a,b_trans,0); free(b_trans);
    return frac_ptr;
}

Fraction* stofrac(char *frac_s){
    Fraction *frac_ptr;
    int mole, deno;
    sscanf(frac_s, "%d/%d", &mole, &deno);
    frac_ptr = frac_init(mole,deno);
    return frac_ptr;
}

int int_less_frac(int l,Fraction *r){
    //printf("l=%d r=",l); print_frac(r); putchar('\n');
    if( l*(r->denominator) < r->molecule ) return 1;
    else return 0;
}
int cmp_left_over(Fraction* a,Fraction* b){ // a is larger than b ? : yes=1 , no=0
    if( a->molecule==frac_inf->molecule ){ return 1; }
    int denoa=a->denominator, denob=b->denominator;
    int s,t = lcm(denoa,denob);
    Fraction *a_trans=mul_int(a,t),*b_trans=mul_int(b,t);
    s = a_trans->molecule; t = b_trans->molecule;
    free(a_trans); free(b_trans);
    if( s > t ) return 1;
    else return 0;
}
int frac_equal(Fraction *a, Fraction *b){
    return (a->molecule==b->molecule) && (a->denominator==b->denominator);
}
void print_frac(Fraction *frac){
    if( frac == frac_inf ){ printf("INF"); return; }
    printf("%llu/%d",frac->molecule,frac->denominator);
}

void fractions_init(){
    frac_zero = &FRAC_ZERO;
    frac_one = &FRAC_ONE;
    frac_inf = &FRAC_INF;
}

fraction_list *fl_init(void){
    fraction_list *fl_ptr;
    
    fl_ptr = (fraction_list*)malloc(sizeof(fraction_list));
    if( fl_ptr==NULL ){ fprintf(stderr, "ERROR : FL init\n"); exit(1); }
    fl_ptr->size = 0;
    
    frac_append(fl_ptr, frac_inf);
    
    return fl_ptr;
}

void frac_append(fraction_list *fl_ptr, Fraction *frac_ptr){
    int size = fl_ptr->size;
    int new_size = size+10;
    Fraction **fractions;
    
    if( frac_ptr==NULL ){ frac_ptr = fl_ptr->frac_list[size-1]; }
    
    fractions = (Fraction**)malloc(new_size*sizeof(Fraction*));
    for(int i=0; i<size; i++) fractions[i] = fl_ptr->frac_list[i];
    for(int i=size;i<new_size;i++) fractions[i] = frac_init(frac_ptr->molecule, frac_ptr->denominator);
    
    if( size!=0 ) free(fl_ptr->frac_list);
    fl_ptr->frac_list = fractions;
    fl_ptr->size = new_size;
}

Fraction *get_frac(fraction_list* fl_ptr, int s){
    int size=fl_ptr->size;
    while( size <= s ){
        frac_append(fl_ptr, fl_ptr->frac_list[size-1]);
    }
    return fl_ptr->frac_list[s];
}

void update_fl(fraction_list *fl_ptr, Fraction *frac_ptr, int count){
    
    while( fl_ptr->size <= count ) frac_append(fl_ptr, NULL);
    
    for(int i=count; i<fl_ptr->size; i++){
        Fraction *frac_lim = fl_ptr->frac_list[i];
        //print_frac(frac_lim); putchar('&');
        //print_frac(frac_ptr); putchar('\n');
        if( cmp_left_over(frac_lim, frac_ptr) ){
            *frac_lim = *frac_ptr;
        }
    }
    
}
