#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

void PrintArray(float* array,size_t size, char* name)
{
    printf("- %s -\n",name);
    for (size_t i = 0; i < size; i++)
    {
        printf("%f\n", array[i]);
    }
    printf("- End -\n\n");
}

//Result goes in arr1
void AddArray(float* arr1, float* arr2, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        arr1[i]+=arr2[i];
    }
}

void Mult(float cst, float* array, size_t size, float* result)
{
    for (size_t i = 0; i < size; i++)
    {
        result[i] = array[i]*cst;
    }
}

//Not a matrix multiplication
//but : c(i,j)=a(i,j)*b(i,j)
//Result goes in array1
void MultArrays(float* array1,float* array2, size_t size, float* result)
{
    for (size_t i = 0; i < size; i++)
    {
        result[i] = array1[i]*array2[i];
    }
}

void ApplySigmoidDeriv(float* array, size_t size, float* result)
{
    for(size_t i = 0; i < size; i++)
    {
        result[i] = array[i]*(1-array[i]);
    }
}


//fill array with random value between -1 and 1
void Randomize(float* array, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        array[i] = ((float)rand()/RAND_MAX)*2-1;
    }
}

size_t array_max_index(float array[], size_t len)
{
    size_t maxIndex = 0;
    for (size_t i = 0; i < len; i++) {
        if(array[i]>array[maxIndex])
            maxIndex = i;
    }
    return maxIndex;
}
