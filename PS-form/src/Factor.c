#include <stdio.h>
#include <stdlib.h>
#include "Factor.h"

Factor* create_factor(DataType type, const void* data)
{
    if (!data) return NULL;
    
    Factor* new_factor = (Factor*)malloc(sizeof(Factor));
    if (!new_factor) 
        exit(EXIT_FAILURE);

    new_factor->type = type;
    switch (type)
    {
        case COEF:
            new_factor->data.coef = *(long*)data;
            break;

        case VAR:
            new_factor->data.var = *(char*)data;
            break;
    }
    new_factor->next = NULL;

    return new_factor;
}
