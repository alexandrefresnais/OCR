#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include "headers/preprocessing.h"
#include "headers/ai.h"
#include "headers/segmentation.h"
#include "headers/array.h"
#include "headers/scaling.h"
#include "headers/corrector.h"
#include "headers/user_interface.h"
#include "headers/spell_check.h"

#undef main


void Preprocessing(SDL_Surface *image)
{
    ToGrayscale(image);
    SDL_SaveBMP(image,"tmp/grayscaled.bmp");
    Binarize(image);
    SDL_SaveBMP(image,"tmp/binarized.bmp");
    RLSA(image);
    SDL_SaveBMP(image,"tmp/RLSA.bmp");
    SDL_SaveBMP(image,"tmp/final.bmp");
    printf("End of preprocessing\n");
}

char* ConvertToText(SDL_Surface *image)
{
    BinarizedImage bin_image;
    SurfaceToBinarizedImage(image, &bin_image);
    printf("Segmentation\n");
    ListOfChar char_list = Segmentate(&bin_image);
    printf("Recognizing text\n");
    int length = 0;
    char* text = malloc(sizeof(char)*1);
    while(char_list != NULL)
    {
        text = realloc(text,length+1);
        if(char_list->special_case==1)
            text[length]='\n';
        else if(char_list->special_case ==2)
            text[length]=' ';
        else if(char_list->special_case ==3)
            text[length]='\n';
        else
        {
            //Getting char by neural network
            BinarizedImage chr;
            CharToBinarizedImage(&bin_image,char_list,&chr);
            Scale(&chr,28);
            float inputs[784] = {0};
            for (size_t i = 0; i < 784; i++)
            {
                inputs[i] = chr.values[i];
            }
            fflush(stdout);
            text[length] = GiveAnswer(inputs);
        }

		ListOfChar temp = char_list->follower;
        free(char_list);
		char_list = temp;
        length++;
    }
    text = realloc(text,length+2);
    text[length]='\n';
    text[length]=0;
    draw_cell(image,0,0);
    SDL_FreeSurface(image);
    return text;
}

int IATrainMain()
{
    //For generating random numbers used in NeuralNetworks
    time_t t;
    srand((unsigned)time(&t));

    //Initializing with weights and biases
    Initialize();

    int i = 0;
    while(i<40)
    {
        printf("Training %d\n",i);
        fflush(stdout);
        TrainAllFile(50,"training_data/best_training.nn","training_data/chars.txt");
        i++;
    }

    Export("export/save.txt");
    FreeNetwork();
    return 0;
}
int IAImportAndTrain()
{
    if(Import("export/save.txt") == 1)
        return 1;
    //For generating random numbers used in NeuralNetworks
    time_t t;
    srand((unsigned)time(&t));

    int i = 0;
    while(i<100)
    {
        printf("Training %d\n",i);
        fflush(stdout);
        TrainAllFile(50,"training_data/best_training.nn","training_data/chars.txt");
        i++;
    }

    Export("export/save.txt");
    FreeNetwork();
    return 0;
}

char* DoTheJob(char* filepath, int spell, int cases)
{
    printf("Beginning of preprocessing.\n");
    SDL_Surface *image = IMG_Load(filepath);
    if(!image)
    {
        printf("Could find this image.\n");
        return "";
    }
    Preprocessing(image);
    char *text = ConvertToText(image);
    printf("Result before case correcter :\n\n");
    printf("%s\n\n",text);
    if(spell || cases)
    {
        printf("Result after case correcter :\n\n");
        if(spell)
            text = CorrectSentence(text);
        if(cases)
            CaseCorrecter(text);
        printf("%s\n",text);
    }
    return text;
}

int main()
{
    printf("Initializing AI\n");
	if(Import("export/save.txt") == 1)
	{
		printf("Could not find a neural network save.\n");
		return 1;
	}
    UIMain();
    FreeNetwork();
    return 0;
}
