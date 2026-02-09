#include <stdlib.h>
#include <string.h>
#include "Term.h"

void initTerm(Term* term)
{
    if (!term) return;

    term->factor_count = 0;
    term->head = term->tail = NULL;
    term->next = NULL;
}


Term* createTerm(void)
{
    Term* new_term = (Term*)malloc(sizeof(Term));
    if (!new_term) 
        exit(EXIT_FAILURE);

    initTerm (new_term);
    return new_term;
}


void appendFactor(Term* term, DataType type, const void* data)
{
    if (!term) return;

    Factor* new_factor = create_factor(type, data);

    if (term->head == NULL)
    {
        term->head = new_factor;
        term->tail = new_factor;
    }
    else
    {
        term->tail->next = new_factor;
        term->tail= new_factor;
    }

    term->factor_count++;
}


Term* createTermFromString(const char* term_str)
{
    if (!term_str) 
        return NULL;

    Term* new_term = createTerm();

    size_t offset = 0;
    char* rest;

    /* Обрабатывается коэффициент для следующих случаев:
        1) коэффициент указан явно в выражении
        2) коэффициент указан неявно в выражении (-/+ перед слагаемым, начинающимся с переменной) */
    long coef = strtol(term_str, &rest, 10);
    if (strlen(rest) == strlen(term_str) && rest[0] != '-')
        coef = 1;
    else if (strlen(rest) == strlen(term_str) && rest[0] == '-')
    {
        coef = -1;
        offset = 1;
    }

    appendFactor(new_term, COEF, &coef);
    for (const char *p = rest + offset; *p; p++)
        appendFactor(new_term, VAR, p);
    
    return new_term;
}


Term* copyTerm (const Term* src_term)
{
    if (!src_term) 
        return NULL;

    Term* new_term = createTerm();

    for (Factor* f = src_term->head; f; f = f->next)
    {
        switch (f->type)
        {
        case COEF:
            appendFactor(new_term, f->type, &f->data.coef);
            break;

        case VAR:
            appendFactor(new_term, f->type, &f->data.var);
            break;
        }
    }

    return new_term;
}


void insertFactor(Term* term, Factor* cur_factor, Factor* ins_factor)
{
    if (!term || !cur_factor || !ins_factor) return;

    Factor* buff_factor = cur_factor->next;
    Factor* copy_ins_factor = NULL;
    switch (ins_factor->type)
    {
        case COEF:
            copy_ins_factor = create_factor(ins_factor->type, &ins_factor->data.coef);
            break;

        case VAR:
            copy_ins_factor = create_factor(ins_factor->type, &ins_factor->data.var);
            break;
    }
    cur_factor->next = copy_ins_factor;
    copy_ins_factor->next = buff_factor;
    term->factor_count++;
}


void eraseNextFactor (Term* term, Factor* cur_factor)
{
    if (!term || !cur_factor || !cur_factor->next) return;

    Factor* er_factor = cur_factor->next;
    cur_factor->next = er_factor->next;
    free(er_factor);
    term->factor_count--;
}


void printTerm (const Term* term)
{
    if(!term) return;

    for (Factor* f = term->head; f; f = f->next)
    {
        switch (f->type)
        {
        case COEF:
            printf("%ld", f->data.coef);
            break;

        case VAR:
            printf("*%c", f->data.var);
            break;
        }
    }
    printf("\n");
}


int compareTermsWithoutCoef(Term* term1, Term* term2)
{
    if (!term1 && !term2)
        return 0;
    if (!term1 || !term2) 
        exit(EXIT_FAILURE);

    Factor* f1 = term1->head ? term1->head->next : NULL;
    Factor* f2 = term2->head ? term2->head->next : NULL;
    for (; f1 && f2; f1=f1->next, f2=f2->next)
    {
        if (f1->data.var < f2->data.var) 
            return -1; 
        else if (f1->data.var > f2->data.var) 
            return 1;
    }
    return 0;
}


void freeTerm(Term* term)
{
    if (!term) return;

    Factor* cur_factor = term->head;
    while (cur_factor)
    {
        Factor* del_factor = cur_factor;
        cur_factor = cur_factor->next;
        free(del_factor); 
    }
    free(term);
}
