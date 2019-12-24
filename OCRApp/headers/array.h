#ifndef ARRAY_H
#define ARRAY_H

void PrintArray(float* array,size_t size, char* name);
void AddArray(float* arr1, float* arr2, size_t size);
void Mult(float cst, float* array, size_t size, float* result);
void MultArrays(float* array1,float* array2, size_t size, float* result);
void ApplySigmoidDeriv(float* array,size_t size, float* result);
void Randomize(float* array, size_t size);
size_t array_max_index(float array[], size_t len);
#endif
