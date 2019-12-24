#ifndef IA_H
#define IA_H

void Initialize();
void Export(char* s);
int Import(char* s);
void FreeNetwork();
void TrainAllFile(size_t batch_size,char* inputFilePath, char* expectedFilePath);
char GiveAnswer(float* inputs);
void create_training_data(char* imagePath,char* exportPath);

#endif
