#include "../headers/spell_check.h"
#include <stdio.h>
#include <string.h>

int isUppercase(char c)
{
    return (c>=65 && c<=90);
}

int isLetter(char c)
{
    return (c>=65 && c<=90)||(c>=97 && c<=122);
}

int isEndOfWord(char c)
{
    return (c == 0 || c=='.' || c==' '|| c==';' || c==',');
}

void ToLowerCase(char* letter)
{
    (*letter)+='a'-'A';
}

void ToUpperCase(char* letter)
{
    (*letter)-='a'-'A';
}

char* CorrectSentence(char* sentence)
{
    char *s1 = ""; //s1 => Contains words
    char *s2 = ""; //s2 => Corrected sentence
    while(*sentence != '\0')
    {
        s1 = "";
        while(isLetter(*sentence))
        {
            s1 = concat_char(s1, sentence);
            sentence++;
        }
        char *s3 = correction(s1);
        if(strlen(s3) == 0)
        {
            s2 = concat(s2, s1);
        }
        else
        {
            s2 = concat(s2, correction(s1));
        }
        if(*sentence != '\0')
        {
            s2 = concat_char(s2, sentence);
            sentence++;
        }

    }
    return s2;
}

//Correcting uppercases problems from neural network.
//If there is a point, next char is Uppercase
//If the word is fully uppercase, we do not change it.
//First char of text is uppercase
void CaseCorrecter(char* text)
{
    char* p = text;
    //First char is is uppercase by default
    int firstCharIsUppercase = 1;
    while(*p!=0)
    {
        //if the word is fully uppercase
        int isFullUppercase = 1;
        char* k = p;
        while(!isEndOfWord(*k))
        {
            if(!isUppercase(*k))
            {
                isFullUppercase = 0;
                break;
            }
            k++;
        }
        k = p;
        if(firstCharIsUppercase && !isUppercase(*k))
        {
            ToUpperCase(k);
        }
        else if(!firstCharIsUppercase && isUppercase(*k)&&!isFullUppercase)
        {
            ToLowerCase(k);
        }
        firstCharIsUppercase = 0;
        k++;
        while(!isEndOfWord(*k))
        {
            if(!isFullUppercase && isUppercase(*k))
            {
                ToLowerCase(k);
            }
            k++;
        }
        p=k;
        //Going to next word
        while(*p!=0 && !isLetter(*p))
        {
            if(*p == '.')
                firstCharIsUppercase = 1;
            p++;
        }
    }
}
