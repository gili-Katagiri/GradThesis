#include<stdio.h>
#include<stdlib.h>

#include"penalty.h"

Penalties* penal_init(void){
    Penalties *pen_ptr;
    pen_ptr = (Penalties*)malloc(sizeof(Penalties));
    if( pen_ptr==NULL ){ fprintf(stderr, "ERROR : Penal init\n"); exit(1); }
    pen_ptr->penals = vec_init((void*)frac_zero,NULL); //f(0) = 0;
    pen_ptr->length = 0;
    return pen_ptr;
}
Fraction* get_penalty(Penalties *pen, int x){
    Fraction *frac_ptr;
    while(pen->length <= x){
        pen->length++;
        frac_ptr = pen->create(pen->args, pen->length);
        if( frac_ptr->denominator < frac_ptr->molecule ){
            fprintf(stderr, "Create: Fraction value is over the 1\n");
            exit(1);
        }else if( frac_ptr->denominator * frac_ptr->molecule < 0 ){
            fprintf(stderr, "Create: Fraction value is negative\n");
            exit(1);
        }
        vec_append(&pen->penals,(void*)frac_ptr);
    }
    frac_ptr = (Fraction*)vec_get(pen->penals,x);
    return frac_ptr;
}
void print_penalties(Penalties *penal_ptr){
    printf("Function ID: %c\n",penal_ptr->identity);
    printf("Arguments: ");
    switch (penal_ptr->identity) {
        case 'f':
            printf(" s=%d\n",*( (int*)( penal_ptr->args->data) ) );
            break;
        case 'l':
            printf(" slope=");
            printf("1/%d", *((int*)(penal_ptr->args->data)));
            //print_frac( (Fraction*)(penal_ptr->args->data) );
            printf(" section=");
            printf("%d", *((int*)(penal_ptr->args->next->data)));
            //print_frac( (Fraction*)(penal_ptr->args->next->data) );
            putchar('\n');
            break;
        default:
            printf("Not Implemented\n");
            break;
    }
    printf("Known Penalties List: ");
    print_fracs(penal_ptr->penals);
}
//以下個別の関数

//f(x) = 2sx/(x^2+s^2)
void func_init_f(Penalties *pen_ptr,int s, int slide){
    pen_ptr->identity = 'f';
    int *s_ptr, *slide_ptr;
    s_ptr = (int*)malloc(sizeof(int));
    slide_ptr = (int*)malloc(sizeof(int));
    *s_ptr = s;
    *slide_ptr = slide;
    pen_ptr->create = create_f;
    pen_ptr->args = vec_init((void*)slide_ptr,NULL);
    vec_prepend( &pen_ptr->args, (void*)s_ptr);
}
Fraction* create_f(Vector *args,int x){
    Fraction *frac_ptr;
    int s, slide, x_slide;
    s = *( (int*)(vec_get(args,0)) );
    slide = *( (int*)(vec_get(args,1)) );
    x_slide = x - slide;
    if( x <= slide ){
        return frac_init(1,1);
    }
    frac_ptr = frac_init(2*s*x_slide,x_slide*x_slide+s*s);
    return frac_ptr ;
}

//f(x) = (a+1-x)/a
void func_init_l(Penalties *pen_ptr, int a, int slide){
    pen_ptr->identity = 'l';
    int *a_ptr, *slide_ptr;
    a_ptr = (int*)malloc(sizeof(int));
    slide_ptr = (int*)malloc(sizeof(int));
    *a_ptr = a;
    *slide_ptr = slide;
    pen_ptr->create = create_l;
    pen_ptr->args = vec_init((void*)slide_ptr,NULL);
    vec_prepend( &pen_ptr->args, (void*)a_ptr);
}
Fraction *create_l(Vector *args,int x){
    Fraction *frac_ptr;
    int a, slide, x_slide;
    a = *( (int*)(vec_get(args,0)) );
    slide = *( (int*)(vec_get(args,1)) );
    x_slide = x - slide;
    if(x <= slide){
        return frac_init( 1,1 );
    }
    frac_ptr = frac_init( a+1-x_slide , a );
    return frac_ptr;
}
