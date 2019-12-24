#include "../headers/spell_check.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <err.h>

#define max(a, b) a < b ? b : a
#define min(a, b) a < b ? a : b
#define MIN3(a, b, c) min( min(a,b), c)
int levenshtein(char* s1, char* s2){
    int s1len, s2len, x, y, lastdiag, olddiag;
    s1len = strlen(s1);
    s2len = strlen(s2);
    int column[100];
    for (y = 1; y <= s1len; y++)
        column[y] = y;
    for (x = 1; x <= s2len; x++) {
        column[0] = x;
        for (y = 1, lastdiag = x - 1; y <= s1len; y++) {
            olddiag = column[y];
            column[y] = MIN3(column[y] + 1, column[y - 1] + 1, lastdiag + (s1[y - 1] == s2[x - 1] ? 0 : 1));
            lastdiag = olddiag;
        }
    }

    return column[s1len];
}

double distanceL(char* s1, char* s2){
    return 1.0 - levenshtein(s1, s2) / (double)(max(strlen(s1), strlen(s2)));
}

double distJaccard(char* s1, char* s2, double d) {
    return d / (strlen(s1) + strlen(s2) - d);
}

char* trigram(char* s) {
    size_t len = strlen(s) + 3;
    char* sc = calloc(len, sizeof(char));
    strcpy(sc, "$");
    strcat(sc, s);
    strcat(sc, "$");

    char* m = calloc(len * 4, sizeof(char));

    // filling the matrix
    for (size_t i = 0; i < strlen(s); i++)
        for (size_t j = 0; j < 3; j++)
            m[i * 4 + j] = sc[j + i];
    free(sc);

    return m;
}

char* concat(char* str1, char* str2)
{
    size_t size = strlen(str1) + strlen(str2) + 1;
    char *str = malloc(size * sizeof(char));
    if(str == NULL)
        errx(1, "Not enough memory !");

    char *p = str;
    while(*str1 != 0)
    {
        *(p++) = *(str1++);
    }
    while(*str2!= 0)
    {
        *(p++) = *(str2++);
    }
    *p = 0;

    return str;
}

char* concat_char(char* str1, char* str2)
{
    size_t size = strlen(str1) + 2;
    char *str = malloc(size * sizeof(char));
    if(str == NULL)
        errx(1, "Not enough memory !");

    char *p = str;
    while(*str1 != 0)
    {
        *(p++) = *(str1++);
    }
    *(p++) = *(str2++);
    *p = 0;

    return str;
}

char* correction(char* s) {
    size_t len = strlen(s);
    char* triL = trigram(s); // list of trigram
    int* freq = calloc(82000, sizeof(int)); // word frenquency
    char* w = calloc(82000 * 30, sizeof(char)); // list of similar word
    if (w == NULL) return "Not enough memory !";

    // foreach trigram
    for (size_t i = 0; i < len; i++) {
        // searching the trigram file
        char tri[13] = ".trigram/";
        for (size_t j = 0; j < 4; j++) tri[j + 9] = triL[i * 4 + j];

        FILE* fTmp = fopen(tri, "r");
        if (fTmp != NULL) {
            // searching each word of a trigram in w
            char* sTmp = calloc(30, sizeof(char));
            while (fgets(sTmp, 30, fTmp) != NULL ) {
                size_t j = 0;
                size_t k = 0;
                while (w[j * 30] && k != strlen(sTmp) - 1) {
                    k = 0;
                    while (k < strlen(sTmp) - 1 && w[j * 30 + k] == sTmp[k]) k++;
                    if (k != strlen(sTmp) - 1) j++;
                }

                // changing the frequency
                if (k == strlen(sTmp) - 1 && w[j * 30 + k] == 0)
                    freq[j] += 1;
                else { // or adding the word in the list
                    freq[j] = 1;

                    k = 0;
                    while (sTmp[k] && sTmp[k] != '\n') { w[j * 30 + k] = sTmp[k]; k++; }
                    for (size_t z = k; z < 30; z++) w[j * 30 + z] = 0;
                }
            }
            free(sTmp);
            fclose(fTmp);
            fTmp = NULL;
        }
    }
    // delete word with the Jaccard distance
    for (size_t i = 0; i < 82000 && w[i * 30]; i++) {
        char s1[30];
        size_t j = 0;
        while (w[i * 30 + j]) { s1[j] = w[i * 30 + j]; j++;}
        s1[j] = 0;

        if (distJaccard(s, s1, freq[i]) <= 0.2)
            for (size_t k = 0; k < 30; k++) w[i * 30 + k] = 0;
    }

    double maxI = -1;
    char* maxW = calloc(30, sizeof(char));
    // choosing the best word with the levenshtien distance
    for (size_t i = 0; i < 82000; i++) {
        if (w[i * 30] == 0) continue;

        char s1[30];
        size_t j = 0;
        while (w[i * 30 + j]) { s1[j] = w[i * 30 + j]; j++; }
        s1[j] = 0;

        if (distanceL(s, s1) >= maxI) {
            maxI = distanceL(s, s1);
            strcpy(maxW, s1);
        }
    }

    free(freq);
    free(w);
    free(triL);
    return maxW;
}
