#ifndef _TOOLS_H_
#define _TOOLS_H_

typedef long long ull;

int gcd(ull, int);
ull lcm(ull, int);
int min(int, int);
int indexat(int,int,int);

typedef struct{
    ull molecule;
    int denominator;
}Fraction;

extern Fraction *frac_zero,*frac_one,*frac_inf;

Fraction* frac_init(ull, int);
Fraction* frac_mul(Fraction*,Fraction*);
Fraction* mul_int(Fraction*, ull);
Fraction* frac_plus(Fraction*, Fraction*, int);
Fraction* frac_minus(Fraction*, Fraction*);
Fraction* stofrac(char*);
int int_less_frac(int,Fraction*);
int cmp_left_over(Fraction*, Fraction*);
int frac_equal(Fraction*, Fraction*);
void print_frac(Fraction*);
void fractions_init(void);

typedef struct{
    Fraction **frac_list;
    int size;
}fraction_list;
fraction_list* fl_init(void);
void frac_append(fraction_list*, Fraction*);
Fraction* get_frac(fraction_list*, int);
void update_fl(fraction_list*, Fraction*, int);

#endif
