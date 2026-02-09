#ifndef PSF_H
#define PSF_H

#include <stdio.h>
#include "Term.h"

/* Структура PS-формы */
typedef struct PSF
{
    size_t term_count;
    
    Term* head;
    Term* tail;
} PSF;

/* Создаёт новую пустую PSF */
PSF* createPSF(void);

/* Добавляет слагаемое в конец PSF */
void appendTerm(PSF* psf, Term* term);

/* Освобождает память, выделенную под PSF */
void freePSF(PSF* psf);

/* Парсит строку с PSF, сортируя слагаемые по возрастанию длины, 
    при этом в рамках одной величины длины сортирует слагаемые в лексикографическом порядке */
PSF* setPS(const char* psf_str);

/* Копирует PSF, создавая независимую копию */
PSF* copyPSF (PSF* src_psf);

/* Выводит PSF*/
void printPSF(PSF* psf);

/* Операция сложения 2-х PSF */
PSF* plusPSF(PSF* psf1, PSF* psf2);

/* Операция вычитания 2-ой PSF из 1-ой*/
PSF* minusPSF(PSF* psf1, PSF* psf2);

/* Операция умножения PSF на слагаемое*/
PSF* multiplePSF_By_Term(PSF* psf, Term* term);

/* Операция умножения 2-х PSF*/
PSF* multiplePSF(PSF* psf1, PSF* psf2);

/* Операция деления 1-ую PSF на 2-ую*/
PSF* dividePSF(PSF* psf1, PSF* psf2);

/* Операция сравнения PSF */
void equalPSF(PSF* psf1 , PSF* psf2);

#endif
