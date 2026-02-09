#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "SortFunc.h"
#include "Factor.h"
#include "Term.h"
#include "PSF.h"

/* Выполняет указанную функцию над двумя PSF */
PSF* operationPSF(PSF* psf1, PSF* psf2, char* oper)
{
    if (!psf1 || !psf2 || !oper || strlen(oper) != 1)
        return NULL;
    
    PSF* res_psf = NULL;

    switch (*oper)
    {
        case '+':
            res_psf = plusPSF(psf1, psf2);
            break;
    
        case '-':
            res_psf = minusPSF(psf1, psf2);
            break;
        
        case '*':
            res_psf = multiplePSF(psf1, psf2);
            break;

        case '/':
            res_psf = dividePSF(psf1, psf2);
            break;

        case '=':
            equalPSF(psf1, psf2);
            break;

    default:
        exit(EXIT_FAILURE);
    }
    return res_psf;
}

/* Динамический ввод */
// char* readLine(FILE* file)
// {
//     size_t chunk_size = 256;
//     char chunk[chunk_size];

//     size_t total_len = 0;     
//     size_t buf_size = 256;    
//     char *buffer = malloc(buf_size);
//     if (!buffer) 
//         return NULL;

//     buffer[0] = '\0';  

//     /* Читаем файл кусками */ 
//     while (fgets(chunk, sizeof(chunk), file))
//     {
//         size_t chunk_len = strlen(chunk);
        
//         if (total_len + chunk_len + 1 > buf_size) 
//         {
//             buf_size *= 2;
//             char *new_buf = realloc(buffer, buf_size);
//             if (!new_buf) 
//             {
//                 free(buffer);
//                 return NULL;
//             }
//             buffer = new_buf;
//         }

//         strcat(buffer, chunk);
//         total_len += chunk_len;

//         if (chunk[chunk_len - 1] == '\n') 
//             break;
//     }

//     if (total_len > 0 && buffer[total_len - 1] == '\n') 
//         buffer[total_len - 1] = '\0';

//     return buffer;
// }

int main (void)
{
    //char* ps1_str = "2*x*x + 4*x*x*y - 8*x*y*y";
    //char* ps2_str = "2*x*x + 4*x*x*y - 8*x*y*y";

    PSF *psf1 = NULL, 
        *psf2 = NULL, 
        *psf3 = NULL;

    // char* oper = readLine(stdin);
    // char* psf1_str = readLine(stdin);
    // char* psf2_str = readLine(stdin);

    size_t len = 0;
    int read;

    char* oper = NULL;
    char* psf1_str = NULL;
    char* psf2_str = NULL;

    if ((read = getline(&oper, &len, stdin)) == -1) return 1;
    if (oper[read - 1] == '\n') oper[read - 1] = '\0';

    if ((read = getline(&psf1_str, &len, stdin)) == -1) return 1;
    if (psf1_str[read - 1] == '\n') psf1_str[read - 1] = '\0';

    if ((read = getline(&psf2_str, &len, stdin)) == -1) return 1;
    if (psf2_str[read - 1] == '\n') psf2_str[read - 1] = '\0';

    psf1 = setPS(psf1_str);
    psf2 = setPS(psf2_str);
    
    psf3 = operationPSF(psf1, psf2, oper);

    if (psf3) printPSF(psf3);

    freePSF(psf1);
    freePSF(psf2);
    freePSF(psf3);
    
    free(psf1_str);
    free(psf2_str);
    free(oper);
    return 0;
}
