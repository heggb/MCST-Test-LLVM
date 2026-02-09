#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "SortFunc.h"

void calcMaxCountAndSizeTerm(const char* str, size_t* max_count_terms, size_t* max_size_term)
{
    size_t current_length_term = 1;

    *max_count_terms = 0;
    *max_size_term = 1;

    /* Проходим по всем символам строки, выделяя знаки разделения слагаемых (+/-) и
        игнорируя пробелы и знаки умножения */
    for (const char* p = str; *p; p++)
    {
        if (*p == '+')
        {
            *max_size_term = *max_size_term > current_length_term ? *max_size_term : current_length_term;
            current_length_term = 1;
            (*max_count_terms)++;
        }

        else if (*p == '-') 
        {
            *max_size_term = *max_size_term > current_length_term ? *max_size_term : current_length_term;
            current_length_term = 2;
            /* Если минус в начале строки, то не учитываем его как разделитель слагаемых */
            if (p != str) (*max_count_terms)++;
        }
        else if (*p != ' ' && *p != '*') 
            current_length_term++;
    }
    *max_size_term = *max_size_term > current_length_term ? *max_size_term : current_length_term;
    (*max_count_terms)++;
}

int compareChars(const void* a, const void* b) 
{
    return (*(const char *)a - *(const char *)b);
}

int compareStringsByVar(const void *a, const void *b)
{
    const char **str1 = (const char **)a;
    const char **str2 = (const char **)b;

    char *rest1, *rest2;

    (void)strtol(*str1, &rest1, 10);
    (void)strtol(*str2, &rest2, 10);

    if (rest1[0] == '-') rest1++;
    if (rest2[0] == '-') rest2++;
    
    size_t len1 = strlen(rest1);
    size_t len2 = strlen(rest2);

    if (len1 < len2) 
        return -1; 
    if (len1 > len2) 
        return 1;
    return strcmp(rest1, rest2);
}
