#ifndef _VECTOR_H_
#define _VECTOR_H_
#include"structures.h"

typedef struct Vector{
    void *data;
    struct Vector *next;
}Vector;

Vector* vec_init(void* ,Vector*);
void vec_prepend(Vector**, void*);
Vector *vec_append(Vector**, void*);
void iteminsert_forcost(Vector**, NamedItem*);
void iteminsert_forprofit(Vector**, NamedItem*);
void* vec_get(Vector*, int);
void vec_predel(Vector**);
void vec_delete(Vector**, int);
void print_nameditems(Vector*);
int cell_is_in_vec(State*,Vector*);
void print_candidates(Vector*);
void print_nodes(Vector*);
void print_solution(Vector*);
void print_fracs(Vector*);

#endif
