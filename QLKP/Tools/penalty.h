#ifndef _PENALTY_H_
#define _PENALTY_H_

#include"vector.h"
#include"tools.h"

typedef struct{ //~までの追加(create)，取り出し(get)を分けて考える．
    char identity;
    Vector *penals; //可変長分数リスト
    int length; //その長さ
    Fraction* (*create)(Vector*, int); //create関数
    Vector *args; //引数のアドレスリスト
}Penalties;

Penalties* penal_init(void); //呼び出し後，別途create，argsの設定．
Fraction* get_penalty(Penalties *pen, int x);//取り出し，存在しなければcreateに従い，f(x)を作成．
void print_penalties(Penalties*);

//以下関数定義．
void func_init_f(Penalties*, int, int);
Fraction* create_f(Vector*, int); //f(x) = 2sx/(x^2+s^2) args={int s}

void func_init_l(Penalties*, int, int);
Fraction* create_l(Vector*, int);
#endif
