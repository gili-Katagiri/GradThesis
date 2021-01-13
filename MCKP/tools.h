#ifndef _TOOLS_H_
#define _TOOLS_H_

int gcd(int, int);
int lcm(int, int);
int min(int, int);

typedef struct{
    int molecule;
    int denominator;
}Fraction;

extern Fraction *frac_zero,*frac_one,*frac_inf;

Fraction* frac_init(int, int);
Fraction* frac_mul(Fraction*,Fraction*);
Fraction* mul_int(Fraction*,int);
Fraction* frac_plus(Fraction*, Fraction*, int);
Fraction* frac_minus(Fraction*, Fraction*);
int int_less_frac(int,Fraction*);
int cmp_left_over(Fraction*, Fraction*);
void print_frac(Fraction*);
void fractions_init(void);

#endif
