#include"tools.h"
#include<stdio.h>
#include<stdlib.h>
#include<limits.h>

#define INF INT_MAX

int gcd(int a,int b){
    int r=a%b;
    if( r==0 ) return b;
    else return gcd(b,r);
}
int lcm(int a,int b){
    int c=a*b,g=gcd(a,b);
    return c/g;
}
int min(int a,int b){
    if( a>b ) return b;
    else return a;
}

static Fraction FRAC_ZERO={0,1},FRAC_ONE={1,1},FRAC_INF={INF,1};
Fraction *frac_zero,*frac_one,*frac_inf;

Fraction* frac_init(int molecule,int denominator){
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
Fraction *mul_int(Fraction* frac_ptr,int s){
    int molecule,denominator;
    molecule=frac_ptr->molecule; denominator=frac_ptr->denominator;
    return frac_init(molecule*s,denominator);
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

int int_less_frac(int l,Fraction *r){
    //printf("l=%d r=",l); print_frac(r); putchar('\n');
    if( l*(r->denominator) < r->molecule ) return 1;
    else return 0;
}
int cmp_left_over(Fraction* a,Fraction* b){ // a is larger than b ? : yes=1 , no=0
    if( a==frac_inf ){ return 1; }
    int denoa=a->denominator, denob=b->denominator;
    int s,t = lcm(denoa,denob);
    Fraction *a_trans=mul_int(a,t),*b_trans=mul_int(b,t);
    s = a_trans->molecule; t = b_trans->molecule;
    free(a_trans); free(b_trans);
    if( s > t ) return 1;
    else return 0;
}
void print_frac(Fraction *frac){
    if( frac == frac_inf ){ printf("INF"); return; }
    printf("%d/%d",frac->molecule,frac->denominator);
}

void fractions_init(){
    frac_zero = &FRAC_ZERO;
    frac_one = &FRAC_ONE;
    frac_inf = &FRAC_INF;
}

