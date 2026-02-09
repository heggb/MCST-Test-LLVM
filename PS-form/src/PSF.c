#include <stdlib.h>
#include <string.h>

#include "SortFunc.h"
#include "PSF.h"


void initPSF(PSF* psf)
{
    if (!psf) return;
    psf->term_count = 0;
    psf->head = psf->tail = NULL;
}

PSF* createPSF(void)
{
    PSF* psf = (PSF*)malloc(sizeof(PSF));
    if (!psf)
        exit(EXIT_FAILURE);
    initPSF(psf);
    return psf;
}


void appendTerm(PSF* psf, Term* term)
{ 
    if (!psf || !term) return;
    
    if (psf->head == NULL)
    {
        psf->head = term;
        psf->tail = term;
    }
    else
    {
        psf->tail->next = term;
        psf->tail = term;
    }
    psf->term_count++;
}


void freePSF(PSF* psf)
{
    if (!psf) return;
    
    Term* cur_term = psf->head;
    while (cur_term)
    {
        Term* del_term = cur_term;
        cur_term = cur_term->next;
        freeTerm(del_term);
    }
    free(psf);
}


PSF* setPS(const char* psf_str)
{
    if (!psf_str) return NULL;
    
    PSF* psf = createPSF();

    
    size_t count_terms = 0, max_count_terms = 0, size_term = 1, max_size_term = 1;
    calcMaxCountAndSizeTerm(psf_str, &max_count_terms, &max_size_term);

    char* buff_term = malloc(max_size_term * sizeof(char));
    if (!buff_term)
        exit(EXIT_FAILURE);
    buff_term[0] = '\0';

    char** terms = malloc(max_count_terms * sizeof(char*));
    if (!terms)
    {
        free(buff_term);
        exit(EXIT_FAILURE);
    }
        
    /* Проходим по всем символам строки, выделяя знаки разделения слагаемых (+/-) и
        игнорируя пробелы и знаки умножения */
    for (const char* p = psf_str; *p; p++)
    {
        if (*p == ' ' || *p == '*') 
            continue;

        else if ((*p == '-' || *p == '+') && size_term > 1)
        {
            /* Выделяю коэффициент из слагаемого, и если в нём остались переменные, 
                то сортирую их в лексикографическом порядке */
            char *rest;
            long coef = strtol(buff_term, &rest, 10);
            if (coef != 0 || strlen(rest)+1 != size_term-1)
            {
                if (rest[0] == '-') rest += 1;
                size_t len = strlen(rest);

                qsort (rest, len, sizeof(char), compareChars);

                terms[count_terms] = malloc(strlen(buff_term) + 1);
                strcpy(terms[count_terms], buff_term);
                count_terms++;
            }
            
            memset(buff_term, 0, max_size_term);

            if (*p == '-')
            {
                size_term = 2;
                buff_term[0] = '-';
                buff_term[1] = '\0';
            }
            else 
            {
                size_term = 1;
                buff_term[0] = '\0';
            }

        }
            
        else 
        {
            buff_term[size_term - 1] = *p;
            buff_term[size_term] = '\0';
            ++size_term;
        }
    }

    char *rest;
    long number = strtol(buff_term, &rest, 10);
    if (number != 0 || strlen(rest)+1 != size_term-1)
    {
        if (rest[0] == '-') rest += 1;
        size_t len = strlen(rest);
        
        qsort (rest, len, sizeof(char), compareChars);

        terms[count_terms] = malloc(strlen(buff_term) + 1);
        strcpy(terms[count_terms], buff_term);
        count_terms++;
    }

    qsort(terms, count_terms, sizeof(const char*), compareStringsByVar);

    for (size_t i = 0; i < count_terms; i++)
    {
        appendTerm(psf, createTermFromString(terms[i]));
        free(terms[i]);
    }
    free(terms);
    free(buff_term);

    return psf;
}


PSF* copyPSF (PSF* src_psf)
{
    if (!src_psf) return NULL;
    
    PSF* new_psf = createPSF();

    for (Term* t = src_psf->head; t; t = t->next)
        appendTerm(new_psf, copyTerm(t));

    new_psf->term_count = src_psf->term_count;
    return new_psf;
}


void printPSF(PSF* psf)
{
    if (!psf) return;

    if (psf->head == NULL)
    {
        printf("0\n"); // Обозначение отсутствия слагаемых в PSF
        return;
    }
    
    long buff_coef = 0;

    for (Term* t = psf->head; t; t = t->next)
    {
        Factor* f = t->head;

        /* Выбор варианта отрисовки коэффицента слагаемого:
            1) в первом слагаемом
            2) в виде свободного +/- после какого-то слагаемого в завсимости от знака и модуля коэффициента */

        buff_coef = labs(f->data.coef);

        if (t == psf->head)
        {
            if (f->data.coef != 1 || t->factor_count == 1)
                printf("%ld", f->data.coef);
        }
        else
        {
            if (f->data.coef > 0)
                printf(" + ");
            else if (f->data.coef < 0)
                printf(" - ");

            if (buff_coef != 1)
                printf("%ld", buff_coef);
        }

        if (f->next == NULL)
            continue;
        if (buff_coef != 1)
            printf("*");

        f = f->next;

        for (; f; f = f->next)
        {
            printf("%c", f->data.var);
            if (f->next)
                printf("*");
        }
                    
    }
    printf("\n");
}


PSF* plusPSF(PSF* psf1, PSF* psf2)
{
    if (!psf1 || !psf2)
        return NULL;
    
    PSF* res_psf = createPSF();

    /* Определяем текущее рассматриваемое слагаемое, в каждой из PSF, 
        а также интервалы, на которых количество множителей одинаково. 
        На них сравниваются наборы переменных и при их совпадении производится соответсвующая операция над коэффициентами.
        Если в рамках одного интервала у какого-то слагаемого нет пары, то он соотвтетсвующим образом добавляется в итоговую PSF.
        Если у какого-то интервала нет пары, то соответсвующим образом из него добавляются все элементы в итоговую PSF. */

    Term *cur_term1 = psf1->head, *cur_term2 = psf2->head;
    Term *start_term1 = NULL, *end_term1 = NULL, *start_term2 = NULL, *end_term2 = NULL;

    start_term1 = cur_term1;
    start_term2 = cur_term2;

    while (start_term1 != NULL || start_term2 != NULL)
    {

        if (cur_term1 != NULL && (cur_term2 == NULL || cur_term1->factor_count < cur_term2->factor_count))
        {
            appendTerm(res_psf, copyTerm(cur_term1));
            cur_term1 = cur_term1->next;
            start_term1 = cur_term1;

        }
        else if (cur_term2 != NULL && (cur_term1 == NULL || cur_term1->factor_count > cur_term2->factor_count))
        {
            appendTerm(res_psf, copyTerm(cur_term2));
            cur_term2 = cur_term2->next;
            start_term2 = cur_term2;
        }
        else
        {
            end_term1 = cur_term1;
            cur_term1 = cur_term1->next;

            end_term2 = cur_term2;
            cur_term2 = cur_term2->next;

            while (cur_term1 != NULL && start_term1->factor_count == cur_term1->factor_count)
            {
                end_term1 = cur_term1;
                cur_term1 = cur_term1->next;
            }
            while (cur_term2 != NULL && start_term2->factor_count == cur_term2->factor_count)
            {
                end_term2 = cur_term2;
                cur_term2 = cur_term2->next;
            }

            cur_term1 = start_term1;
            cur_term2 = start_term2;

            while (start_term1 != end_term1->next || start_term2 != end_term2->next)
            {
                int compare = 0;

                /* Сравнение наборов перменных слагаемых и проверка на последний элемент интервала */
                if (cur_term1 == end_term1->next) compare = 1;
                else if (cur_term2 == end_term2->next) compare = -1;
                else compare = compareTermsWithoutCoef(cur_term1, cur_term2);

                switch (compare)
                {
                    case -1:
                        {
                            appendTerm(res_psf, copyTerm(cur_term1));
                            cur_term1 = cur_term1->next;
                            start_term1 = cur_term1;
                        }
                        break;

                    case 1:
                        {
                            appendTerm(res_psf, copyTerm(cur_term2));
                            cur_term2 = cur_term2->next;
                            start_term2 = cur_term2;            
                        }
                        break;

                    case 0:
                        {
                            long new_coef = cur_term1->head->data.coef + cur_term2->head->data.coef;
                            
                            if (new_coef)
                            {
                                appendTerm(res_psf, copyTerm(cur_term1));
                                res_psf->tail->head->data.coef = new_coef;
                            }
                            cur_term1 = cur_term1->next;
                            start_term1 = cur_term1;
                            cur_term2 = cur_term2->next;
                            start_term2 = cur_term2;
                        }
                        break;
                }
            }
        }
    }

    return res_psf;
}


PSF* minusPSF(PSF* psf1, PSF* psf2)
{
    if (!psf1 || !psf2)
        return NULL;

    PSF* res_psf = createPSF();

    /* Определяем текущее рассматриваемое слагаемое, в каждой из PSF, 
        а также интервалы, на которых количество множителей одинаково. 
        На них сравниваются наборы переменных и при их совпадении производится соответсвующая операция над коэффициентами.
        Если в рамках одного интервала у какого-то слагаемого нет пары, то он соотвтетсвующим образом добавляется в итоговую PSF.
        Если у какого-то интервала нет пары, то соответсвующим образом из него добавляются все элементы в итоговую PSF. */

    Term *cur_term1 = psf1->head, *cur_term2 = psf2->head;
    Term *start_term1 = NULL, *end_term1 = NULL, *start_term2 = NULL, *end_term2 = NULL;

    start_term1 = cur_term1;
    start_term2 = cur_term2;


    while (start_term1 != NULL || start_term2 != NULL)
    {

        if (cur_term1 != NULL && (cur_term2 == NULL || cur_term1->factor_count < cur_term2->factor_count))
        {
            appendTerm(res_psf, copyTerm(cur_term1));
            cur_term1 = cur_term1->next;
            start_term1 = cur_term1;
        }
        else if (cur_term2 != NULL && (cur_term1 == NULL || cur_term1->factor_count > cur_term2->factor_count))
        {
            appendTerm(res_psf, copyTerm(cur_term2));
            res_psf->tail->head->data.coef = -res_psf->tail->head->data.coef;
            cur_term2 = cur_term2->next;
            start_term2 = cur_term2;
        }
        else
        {
            end_term1 = cur_term1;
            cur_term1 = cur_term1->next;

            end_term2 = cur_term2;
            cur_term2 = cur_term2->next;

            while (cur_term1 != NULL && start_term1->factor_count == cur_term1->factor_count)
            {
                end_term1 = cur_term1;
                cur_term1 = cur_term1->next;
            }
            while (cur_term2 != NULL && start_term2->factor_count == cur_term2->factor_count)
            {
                end_term2 = cur_term2;
                cur_term2 = cur_term2->next;
            }

            cur_term1 = start_term1;
            cur_term2 = start_term2;

            while (start_term1 != end_term1->next || start_term2 != end_term2->next)
            {
                int compare = 0;

                /* Сравнение наборов перменных слагаемых и проверка на последний элемент интервала */
                if (cur_term1 == end_term1->next) compare = 1;
                else if (cur_term2 == end_term2->next) compare = -1;
                else compare = compareTermsWithoutCoef(cur_term1, cur_term2);

                switch (compare)
                {
                case -1:
                    {
                        appendTerm(res_psf, copyTerm(cur_term1));
                        cur_term1 = cur_term1->next;
                        start_term1 = cur_term1;
                    }
                    break;

                    case 1:
                    {
                        appendTerm(res_psf, copyTerm(cur_term2));
                        res_psf->tail->head->data.coef = -res_psf->tail->head->data.coef;
                        cur_term2 = cur_term2->next;
                        start_term2 = cur_term2;             
                    }
                    break;
                case 0:
                    {
                        long new_coef = cur_term1->head->data.coef - cur_term2->head->data.coef;
                        
                        if (new_coef)
                        {
                            appendTerm(res_psf, copyTerm(cur_term1));
                            res_psf->tail->head->data.coef = new_coef;
                        }
                        cur_term1 = cur_term1->next;
                        start_term1 = cur_term1;
                        cur_term2 = cur_term2->next;
                        start_term2 = cur_term2;
                    }
                    break;
                }
            }
        }
    }

    return res_psf;
}


PSF* multiplePSF_By_Term(PSF* psf, Term* term)
{
    if (!psf || !term)
        return NULL;

    PSF* res_psf = copyPSF(psf);
    if (psf->term_count != 0)
    {
        Term* cur_term = res_psf->head;
        while (cur_term)
        {
            Factor *cur_factor_psf = cur_term->head, *cur_factor_term = term->head;
            cur_factor_psf->data.coef *= cur_factor_term->data.coef;

            cur_factor_term = cur_factor_term->next;

            /* Встраиваем новые переменные из слагаемого в лексикографическом порядке */
            while (cur_factor_term)
            {
                while (cur_factor_psf->next)
                {
                    if (cur_factor_term->data.var <= cur_factor_psf->next->data.var)
                    {
                        insertFactor(cur_term, cur_factor_psf, cur_factor_term);
                        cur_factor_psf = cur_factor_psf->next;
                        break;
                    }

                    cur_factor_psf = cur_factor_psf->next;
                }

                if (cur_factor_psf->next == NULL)
                    insertFactor(cur_term, cur_factor_psf, cur_factor_term);
                
                cur_factor_term = cur_factor_term->next;
            }
            cur_term = cur_term->next;
        }
    }
    return res_psf;
}


PSF* multiplePSF(PSF* psf1, PSF* psf2)
{
    if (!psf1 || !psf2) 
        return NULL;

    PSF* larger_psf = psf1->term_count >= psf2->term_count ? psf1 : psf2;
    PSF* smaller_psf = psf1->term_count < psf2->term_count ? psf1 : psf2;

    PSF* res_psf = createPSF();

    /* Суммируем все промежуточные PSF, получаемые при умнажении большей PSF на слагаемое меньшей */
    for (Term* t = smaller_psf->head; t; t = t->next)
    {
        PSF* intermed_psf = multiplePSF_By_Term(larger_psf, t);
        PSF* sum_psf = plusPSF(res_psf, intermed_psf);
        freePSF(res_psf);
        freePSF(intermed_psf);
        res_psf = sum_psf;
    }
    
    return res_psf;
}


PSF* dividePSF(PSF* psf1, PSF* psf2)
{
    if (psf2->term_count != 1)
    {
        printf("error\n");
        return NULL;
    }

    if (psf1->term_count == 0)
        return createPSF();
     
    PSF* res_psf = copyPSF(psf1);

    Term* cur_term_psf2 = psf2->head;

    /* Проходимся по всем элементам PSF. Если коэффициенты нацело не делятся или в делителе присутствует такой набор множителей,
        которого нет хотя бы в одном множителе, то деление прерывается. В противном случае все ранее описанные действия осуществляются штатно
        над копией первой PSF*/
    for (Term* cur_term_res_psf = res_psf->head; cur_term_res_psf; cur_term_res_psf = cur_term_res_psf->next)
    {
        Factor* cur_factor_res_psf = cur_term_res_psf->head;
        Factor* cur_factor_psf2 = cur_term_psf2->head;

        if (cur_factor_res_psf->data.coef % cur_factor_psf2->data.coef != 0)
        {
            freePSF(res_psf);
            printf("error\n");
            return NULL;
        }
        cur_factor_res_psf->data.coef /= cur_factor_psf2->data.coef;
        cur_factor_psf2 = cur_factor_psf2->next;

        while (cur_factor_res_psf->next != NULL && cur_factor_psf2 != NULL)
        {
            if (cur_factor_res_psf->next->data.var == cur_factor_psf2->data.var)
            {
                eraseNextFactor(cur_term_res_psf, cur_factor_res_psf);
                cur_factor_psf2 = cur_factor_psf2->next;
            }
            
            else 
                cur_factor_res_psf = cur_factor_res_psf->next;
            
        }

        if (cur_factor_psf2 != NULL)
        {
            freePSF(res_psf);
            printf("error\n");
            return NULL;
        }
    }

    return res_psf;
}


void equalPSF(PSF* psf1 , PSF* psf2)
{
    Term* cur_term1 = psf1 ? psf1->head : NULL;
    Term* cur_term2 = psf2 ? psf2->head: NULL;

    /* Сравнение происходит почленно посредством поиска слагаемых с одинаковым набором переменных.
        Если такие найдены, то сравниваются коэффициенты, 
        в противном случае рассмотрение всех слагаемых в одной из 2-х PSF закончится раньше, чем в другой,
        что говорит о неравентсве двух форм.*/
    while (cur_term1 != NULL && cur_term2 != NULL)
    {
        if (cur_term1->factor_count < cur_term2->factor_count)
            cur_term1 = cur_term1->next;

        else if (cur_term1->factor_count > cur_term2->factor_count)
            cur_term2 = cur_term2->next;
        
        else
        {
            int compare = compareTermsWithoutCoef(cur_term1, cur_term2);

            switch (compare)
            {
            case -1:
                cur_term1 = cur_term1->next;
                break;

            case 1:
                cur_term2 = cur_term2->next;
                break;

            case 0:
                {
                    if (cur_term1->head->data.coef != cur_term2->head->data.coef)
                    {
                        printf("not equal\n");
                        return;
                    }
                    cur_term1 = cur_term1->next;
                    cur_term2 = cur_term2->next;
                }
                break;
            }
        }
    }

    if (cur_term1 || cur_term2)
    {
        printf("not equal\n");
        return;
    }

    printf("equal\n");
}
