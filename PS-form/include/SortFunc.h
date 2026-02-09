#ifndef SORTFUNC_H
#define SORTFUNC_H

/* Вычисляет максимальное количество слагаемых и максимальный размер слагаемого,
   исходя из строки, где слагаемые разделяются символами '+' и '-'. */
void calcMaxCountAndSizeTerm(const char* str, size_t* max_count_terms, size_t* max_size_term);

/* Сравнение символов для сортировки переменных внутри слагаемого */
int compareChars(const void* a, const void* b);

/* Сравнение строк по части, содержащей переменные (без коэффициента).
   Строки сначала сортируются по длине, затем лексикографически. */
int compareStringsByVar(const void *a, const void *b);

#endif
