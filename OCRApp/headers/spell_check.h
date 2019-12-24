#pragma once

// Algorithm of Levenshtien
int levenshtein(char* s1, char* s2);

// Disatnce of Levenshtein
double distanceL(char* s1, char* s2);

// Distance of Jaccard
double distJaccard(char* s1, char* s2, double d);

/*
Build all the trigram of a word and add $
at the beggining and at the end of a word

Example: accueil: [�$ac�, �acc�, �ccu�,
�cue�, �uei�, �eil�, �il$�]

- return a matrix with a length of a line
equal to 4 (3 + '\0')and the length of a column
equal to the length of the word
*/
char* trigram(char* s);

/*
Make a correction of a word

return the most similar word
*/
char* correction(char* s);

char* concat(char* str1, char* str2);

char* concat_char(char* str1, char* str2);
