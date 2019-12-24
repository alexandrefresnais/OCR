#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "../headers/array.h"
#include "../headers/segmentation.h"
#include "../headers/scaling.h"
#include "../headers/preprocessing.h"
#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <err.h>

#define CHAR_SIZE 28
#define NB_ENTRIES 784
#define NB_NEURONS_IN_HIDDEN_LAYER 1200
#define NB_OUTPUTS 56

#define LEARNING_RATE 0.1

/* WARNING */
/* Most functions assume there is only one hidden layer */

//Declaring functions to use them on top of their initialization
int Import(char* s);
int load_training_data(char* filepath,float** inputs);
char NeurakNetworkResponseToChar(float* outputs);
void CharIndexToExpectedOutput(float** expectedOutput,int expectedCharIndex);
void CharToExpectedOutput(float** expectedOutput,char expectedChar);

//Recognized chars : if you add or delete some, change output
char chars[56]={
    'a','b','c','d','e','f','g','h','i','j','k','l','m','n',
        'o','p','q','r','s','t','u','v','w','x','y','z',
    'A','B','C','D','E','F','G','H','I','J','K','L','M','N',
        'O','P','Q','R','S','T','U','V','W','X','Y','Z',
    '.',',','?','!'
};

typedef struct Neuron
{
    float* in_weights;
    float bias;
    float value;
} Neuron;

typedef struct Layer
{
    Neuron* neurons;
} Layer;

typedef struct NeuralNetwork
{
    Layer layers[3];
} NeuralNetwork;

NeuralNetwork net;

//Activation function
float sigmoid(float x)
{
    return 1.0/(1.0+exp(-x));
}
float softmax(float i, float sum)
{
    return (exp(i)/sum);
}
/*Computes the value of a neuron according to its inputs*/
float dot_summation(float* inputs, float* weights, size_t size, float bias)
{
    float result  = 0;
    for (size_t i = 0; i < size; i++)
    {
        result += inputs[i]*weights[i];
    }
    return sigmoid(result+bias);
}

/*Computes the value of a neuron according to its inputs*/
float dot_summation_soft(float* inputs, float* weights, size_t size, float bias)
{
    float result  = 0;
    for (size_t i = 0; i < size; i++)
    {
        result += inputs[i]*weights[i];
    }
    return result+bias;
}

/* Returns the NeuralNetwork Response for a given entry */
float* feedForward(float* inputs)
{
    float layerValues[NB_NEURONS_IN_HIDDEN_LAYER];
    for (size_t i = 0; i < NB_NEURONS_IN_HIDDEN_LAYER; i++)
    {
        layerValues[i] = dot_summation(inputs,
            net.layers[1].neurons[i].in_weights,NB_ENTRIES,
            net.layers[1].neurons[i].bias);
        net.layers[1].neurons[i].value = layerValues[i];
    }
    static float neuralNetworkResponse[NB_OUTPUTS];
    float sum =0;
    for (size_t i = 0; i < NB_OUTPUTS; i++)
    {
        neuralNetworkResponse[i] = dot_summation(layerValues,
                net.layers[2].neurons[i].in_weights,
                    NB_NEURONS_IN_HIDDEN_LAYER, net.layers[2].neurons[i].bias);
        sum += exp(neuralNetworkResponse[i]);
    }
    for (size_t i = 0; i < NB_OUTPUTS; i++)
    {
        neuralNetworkResponse[i] = softmax(neuralNetworkResponse[i],sum);
    }
    return neuralNetworkResponse;
}

void AllocMemory()
{
    //Allocating necessary memory for neural network
    net.layers[1].neurons = malloc(sizeof(Neuron)*NB_NEURONS_IN_HIDDEN_LAYER);
    net.layers[2].neurons = malloc(sizeof(Neuron)*NB_OUTPUTS);

    for (size_t i = 0; i < NB_NEURONS_IN_HIDDEN_LAYER; i++)
    {
        net.layers[1].neurons[i].in_weights = malloc(sizeof(float)*NB_ENTRIES);
    }

    for (size_t i = 0; i < NB_OUTPUTS; i++)
    {
        net.layers[2].neurons[i].in_weights =
            malloc(sizeof(float)*NB_NEURONS_IN_HIDDEN_LAYER);
    }
}

//Initialize the network
void Initialize()
{
    AllocMemory();
    //Initializing weights and biases randomly
    for (size_t i = 0; i < NB_NEURONS_IN_HIDDEN_LAYER; i++)
    {
        Randomize(net.layers[1].neurons[i].in_weights,NB_ENTRIES);
        net.layers[1].neurons[i].bias = ((float)rand()/RAND_MAX)-0.5;
    }
    for (size_t i = 0; i < NB_OUTPUTS; i++)
    {
        Randomize(net.layers[2].neurons[i].in_weights,NB_NEURONS_IN_HIDDEN_LAYER);
        net.layers[2].neurons[i].bias = ((float)rand()/RAND_MAX)-0.5;
    }
}

char GiveAnswer(float* inputs)
{
    float* neuralNetworkResponse = feedForward(inputs);
    char response = NeurakNetworkResponseToChar(neuralNetworkResponse);
    if(response == 0)
        return '0';
    return response;
}

//Gradient descent
void CalculateDeltas(float* deltaHiddenWeights, float* deltaOutputWeights,
                        float* deltaHiddenBiases, float* deltaOutputBiases,
                        float *errors, float *hiddenErrors,
                        float* outputs, float* hiddenLayerOutputs, float* inputs)
{
    //Calculate output gradient
    float* gradient;
    gradient = malloc(sizeof(float)*NB_OUTPUTS);
    ApplySigmoidDeriv(outputs, NB_OUTPUTS,gradient);
    MultArrays(errors,gradient,NB_OUTPUTS,gradient);//Hadamard product
    Mult(LEARNING_RATE,gradient,NB_OUTPUTS,gradient);

    //Calculate hidden gradient
    float* hiddenGradient;
    hiddenGradient = malloc(sizeof(float)*NB_NEURONS_IN_HIDDEN_LAYER);
    ApplySigmoidDeriv(hiddenLayerOutputs, NB_NEURONS_IN_HIDDEN_LAYER,hiddenGradient);
    //Hadamard product
    MultArrays(hiddenErrors, hiddenGradient, NB_NEURONS_IN_HIDDEN_LAYER,hiddenGradient);
    Mult(LEARNING_RATE,hiddenGradient, NB_NEURONS_IN_HIDDEN_LAYER, hiddenGradient);

    //Calculating deltas weights
    //Matrix product to finally get deltaWeights
    for (size_t i = 0; i < NB_OUTPUTS; i++)
    {
        for (size_t j = 0; j < NB_NEURONS_IN_HIDDEN_LAYER; j++)
        {
            deltaOutputWeights[i*NB_NEURONS_IN_HIDDEN_LAYER+j] =
                gradient[i]*hiddenLayerOutputs[j];
        }
        //Delta Biases is just the gradient
        deltaOutputBiases[i] = gradient[i];
    }
    for (size_t i = 0; i < NB_NEURONS_IN_HIDDEN_LAYER; i++)
    {
        for (size_t j = 0; j < NB_ENTRIES; j++)
        {
            deltaHiddenWeights[i*NB_ENTRIES+j]=hiddenGradient[i]*inputs[j];
        }
        //Delta Biases is just the gradient
        deltaHiddenBiases[i] = hiddenGradient[i];
    }
    free(hiddenGradient);
    free(gradient);
}

//Weights + deltaWeights
void updateWeights(float* deltaHiddenWeights, float* deltaOuptutWeights)
{
    for (size_t i = 0; i < NB_NEURONS_IN_HIDDEN_LAYER; i++)
    {
        for (size_t j = 0; j < NB_ENTRIES; j++)
        {
            net.layers[1].neurons[i].in_weights[j] += deltaHiddenWeights[i*NB_ENTRIES+j];
        }
    }
    for (size_t i = 0; i < NB_OUTPUTS; i++)
    {
        for (size_t j = 0; j < NB_NEURONS_IN_HIDDEN_LAYER; j++)
        {
            net.layers[2].neurons[i].in_weights[j] +=
                    deltaOuptutWeights[i*NB_NEURONS_IN_HIDDEN_LAYER+j];
        }
    }
}

//Biases + deltaBiases
void updateBiases(float* deltaHiddenBiases, float* deltaOutputBiases)
{
    for (size_t i = 0; i < NB_NEURONS_IN_HIDDEN_LAYER; i++)
    {
        net.layers[1].neurons[i].bias += deltaHiddenBiases[i];
    }
    for (size_t i = 0; i < NB_OUTPUTS; i++)
    {
        net.layers[2].neurons[i].bias += deltaOutputBiases[i];
    }
}

//Trains parsing the whole training file
//Uses mini batch technique
//Input file contains matrices of characters on each line
//Expected file contains the characters
void TrainAllFile(size_t batch_size,char* inputFilePath, char* expectedFilePath)
{
    FILE *inputFile;
    inputFile = fopen(inputFilePath, "r");
    if(inputFile == NULL)
    {
        printf("Could not access inputs file\n");
        exit(EXIT_FAILURE);
    }

    FILE *expectedFile;
    expectedFile = fopen(expectedFilePath, "r");
    if(expectedFile == NULL)
    {
        printf("Could not access expected outputs file\n");
        exit(EXIT_FAILURE);
    }

    int fileIsFinish = 0;
    while(!fileIsFinish)
    {
        float deltaOutputWeights[NB_NEURONS_IN_HIDDEN_LAYER*NB_OUTPUTS] = {0};
        float deltaHiddenWeights[NB_ENTRIES*NB_NEURONS_IN_HIDDEN_LAYER]= {0};
        float deltaOutputBiases[NB_OUTPUTS]= {0};
        float deltaHiddenBiases[NB_NEURONS_IN_HIDDEN_LAYER]= {0};
        for (size_t i = 0; i < batch_size; i++)
        {
            char line[NB_ENTRIES+2];
            if(fgets(line, sizeof(line), inputFile) == NULL)
            {
                fileIsFinish = 1;
                break;
            }
            float inputs[NB_ENTRIES];
            for (size_t i = 0; i < NB_ENTRIES; i++)
            {
                inputs[i]= line[i]-48;
            }
            char expectedChar = (char)fgetc(expectedFile);
            float* expectedOutput = malloc(sizeof(float)*NB_OUTPUTS);
            CharToExpectedOutput(&expectedOutput,expectedChar);

            //Forward and Backprop
            float* outputs;
            outputs = malloc(sizeof(float)*NB_OUTPUTS);
            outputs = feedForward(inputs);
            //Calculating errors
            float errors[NB_OUTPUTS];
            for (size_t i = 0; i < NB_OUTPUTS; i++)
            {
                errors[i] = (expectedOutput[i]-outputs[i]);
            }

            //Hidden neuron error is the error of the output
            //times the outweights of the neuron
            float hidden_layer_errors[NB_NEURONS_IN_HIDDEN_LAYER];
            for (size_t i = 0; i < NB_NEURONS_IN_HIDDEN_LAYER; i++)
            {
                hidden_layer_errors[i] = 0;
                for (size_t j = 0; j < NB_OUTPUTS; j++)
                {
                    hidden_layer_errors[i] += net.layers[2].neurons[j].in_weights[i]*errors[j];
                }
            }

            //Getting hiddenLayerValues that are stored in our Network
            float hiddenLayerValues[NB_NEURONS_IN_HIDDEN_LAYER];
            for (size_t i = 0; i < NB_NEURONS_IN_HIDDEN_LAYER; i++)
            {
                hiddenLayerValues[i] = net.layers[1].neurons[i].value;
            }


            //Calculating change we should do in weights and biases
            float _deltaOutputWeights[NB_NEURONS_IN_HIDDEN_LAYER*NB_OUTPUTS];
            float _deltaHiddenWeights[NB_ENTRIES*NB_NEURONS_IN_HIDDEN_LAYER];
            float _deltaOutputBiases[NB_OUTPUTS];
            float _deltaHiddenBiases[NB_NEURONS_IN_HIDDEN_LAYER];

            CalculateDeltas(_deltaHiddenWeights,_deltaOutputWeights,
                                _deltaHiddenBiases,_deltaOutputBiases,
                                errors, hidden_layer_errors,
                                    outputs,hiddenLayerValues ,inputs);

            AddArray(deltaOutputWeights,_deltaOutputWeights,NB_NEURONS_IN_HIDDEN_LAYER*NB_OUTPUTS);
            AddArray(deltaHiddenWeights,_deltaHiddenWeights,NB_ENTRIES*NB_NEURONS_IN_HIDDEN_LAYER);
            AddArray(deltaHiddenBiases,_deltaHiddenBiases,NB_NEURONS_IN_HIDDEN_LAYER);
            AddArray(deltaOutputBiases,_deltaOutputBiases,NB_OUTPUTS);
        }
        //Changing the weights
        updateWeights(deltaHiddenWeights,deltaOutputWeights);
        updateBiases(deltaHiddenBiases,deltaOutputBiases);
    }

    fclose(inputFile);
    fclose(expectedFile);
}

void Export(char* s) //s must be /export/[name-of-file] format !
{
    FILE *export;
    export = fopen(s, "w+");

    for(size_t j = 0; j < NB_NEURONS_IN_HIDDEN_LAYER; j++)
    {
        for(int k = 0; k < NB_ENTRIES; k++)
        {
            fprintf(export, "%f ", net.layers[1].neurons[j].in_weights[k]);
        }
        fprintf(export,"%f ",net.layers[1].neurons[j].bias);

        fprintf(export, "\n");
    }
    for(size_t j = 0; j < NB_OUTPUTS; j++)
    {
        for(int k = 0; k < NB_NEURONS_IN_HIDDEN_LAYER; k++)
        {
            fprintf(export, "%f ", net.layers[2].neurons[j].in_weights[k]);
        }
        fprintf(export,"%f ",net.layers[2].neurons[j].bias);
        fprintf(export, "\n");
    }
    fclose(export);
}

//Import a NeuralNetwork Save
int Import(char* s) //s must be /export/[name-of-file] format !
{
    AllocMemory();
    FILE *import;
    char buffer[255];
    import = fopen(s, "r");
    if(!import)
    {
        printf("Save not found !\n");
        return 1;
    }
    for(size_t j = 0; j < NB_NEURONS_IN_HIDDEN_LAYER; j++)
    {
        for(int k = 0; k < NB_ENTRIES; k++)
        {
            fscanf(import, "%s", buffer);
            float current_weight = atof(buffer);
            net.layers[1].neurons[j].in_weights[k] = current_weight;
        }
        fgets(buffer, 255, (FILE*)import);
        net.layers[1].neurons[j].bias = atof(buffer);
    }

    for(size_t j = 0; j < NB_OUTPUTS; j++)
    {
        for(int k = 0; k < NB_NEURONS_IN_HIDDEN_LAYER; k++)
        {
            fscanf(import, "%s", buffer);
            float current_weight = atof(buffer);
            net.layers[2].neurons[j].in_weights[k] = current_weight;
        }
        fgets(buffer, 255, (FILE*)import);
        net.layers[2].neurons[j].bias = atof(buffer);
    }
    fclose(import);
    return 0;
}

//Free Memory
void FreeNetwork()
{
    for (size_t i = 0; i < NB_NEURONS_IN_HIDDEN_LAYER; i++)
    {
        free(net.layers[1].neurons[i].in_weights);
    }
    for (size_t i = 0; i < NB_OUTPUTS;i++)
    {
        free(net.layers[2].neurons[i].in_weights);
    }
    free(net.layers[1].neurons);
    free(net.layers[2].neurons);
}

char NeurakNetworkResponseToChar(float* outputs)
{
    size_t charIndex = array_max_index(outputs, NB_OUTPUTS);
    return chars[charIndex];
}

//Returns an array with zeros and a 1 on the expected output
void CharIndexToExpectedOutput(float** expectedOutput,int expectedCharIndex)
{
    for (size_t i = 0; i < NB_OUTPUTS; i++)
    {
        (*expectedOutput)[i]=0;
    }
    (*expectedOutput)[expectedCharIndex] = 1;
}

//Returns an array with zeros and a 1 on the expected output
void CharToExpectedOutput(float** expectedOutput,char expectedChar)
{
    int index = -1;
    for (size_t i = 0; i < NB_OUTPUTS; i++)//NB_OUTPUTS is also length of chars[]
    {
        if(chars[i]==expectedChar)
        {
            index  = i;
            break;
        }
    }
    if(index == -1)
        errx(1,"Unknown char : %c",expectedChar);
    CharIndexToExpectedOutput(expectedOutput,index);
}

//Load every char matrices and returns nbOfLines (which is nb of characters
//in source image)
int load_training_data(char* filepath,float** inputs)
{
    FILE *file;
    file = fopen(filepath, "r");
    if(file == NULL)
        exit(EXIT_FAILURE);
    char c;
    (*inputs)=malloc(sizeof(float)*784);
    int charCount = 0;
    int lineCount = 1;
    while(fscanf(file,"%c",&c) != EOF)
    {
        if(c != '\n')
        {
            (*inputs)[charCount] = c-48;
            charCount++;
        }
        else
        {
            lineCount++;
            (*inputs) = realloc((*inputs),sizeof(float)*784*lineCount);
        }
    }
    fclose(file);
    return lineCount;
}

//Is not use in neural network, just to create the training file
//Create a file with 28x28 characters
//Characters are store line by line
//Source file must have characters in same order than chars[]
void create_training_data(char* imagePath,char* exportPath)
{
    //Getting pixel matrix
    SDL_Surface *image = IMG_Load(imagePath);
    ToGrayscale(image);
    Binarize(image);
    BinarizedImage bin_image;
	SurfaceToBinarizedImage(image, &bin_image);
    SDL_FreeSurface(image);
    //Getting every Line
	ListOfLine split_line = Splitting_L(&bin_image, 0);
    split_line = insert_paragraph(split_line);
    //Getting every char
    ListOfChar split_char = Splitting_C(&bin_image, 0, split_line, 1);
    split_char = insert_space(split_char);
    //Preparing export file
    FILE *export;
    export = fopen(exportPath, "w+");
    //Treating every char
    while(split_char != NULL)
    {
        if(split_char->special_case == 0)
        {
            BinarizedImage chr;
            CharToBinarizedImage(&bin_image,split_char,&chr);
            Scale(&chr,28);
            int nbBit = 0;
            for (size_t y = 0; y < chr.height; y++)
            {
                for (size_t x = 0; x < chr.width; x++)
                {
                    nbBit++;
                    if(nbBit>784)
                        break;
                    //Printing every value on a line
                    fprintf(export,"%d",chr.values[y*chr.width+x]);
                }
            }
            //If not last character
            if(split_char->follower != NULL)
                fprintf(export,"\n");
        }
        split_char = split_char->follower;
    }
    fclose(export);
}
