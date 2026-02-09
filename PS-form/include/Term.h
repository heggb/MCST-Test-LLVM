#ifndef TERM_H
#define TERM_H

#include <stdio.h>
#include "Factor.h"

/* Структура слагаемого */
typedef struct Term
{
    size_t factor_count;

    Factor* head;
    Factor* tail;
    
    struct Term* next;
} Term;

/* Инициализирует слагаемое */
void initTerm(Term* term);

/* Создаёт новое пустое слагаемое */
Term* createTerm(void);

/* Добавляет множитель в конец слагаемого */
void appendFactor(Term* term, DataType type, const void* data);

/* Создаёт слагаемое из строки */
Term* createTermFromString(const char* term_str);

/* Копирует слагаемое, создавая независимую копию */
Term* copyTerm (const Term* src_term);

/* Добавляет после указанного множителя слагаемого копию другого множителя */
void insertFactor(Term* term, Factor* cur_factor, Factor* ins_factor);

/* Удаляет следующий множитель после указанного в слагаемом */
void eraseNextFactor (Term* term, Factor* cur_factor);

/* Печатает слагаемое для отладки */
void printTerm (const Term* term);

/* Сравнивает слагаемые без учёта коэффициентов (лексикографически по переменным) */
int compareTermsWithoutCoef(Term* term1, Term* term2);

/* Освобождает память, выделенную под слагаемое */
void freeTerm(Term* term);

#endif
