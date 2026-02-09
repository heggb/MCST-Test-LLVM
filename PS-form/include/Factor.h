#ifndef FACTOR_H
#define FACTOR_H

/* Тип данных множителя: коэффициент или переменная */
typedef enum { COEF = 0, VAR = 1 } DataType;

/* Структура множителя в слагаемом */
typedef struct Factor
{
    unsigned int type : 1; // COEF или VAR
    
    union 
    {
        long coef;
        char var;
    } data;

    struct Factor* next;
    
} Factor;

/* Создаёт новый множитель */
Factor* create_factor(DataType type, const void* data);

#endif
