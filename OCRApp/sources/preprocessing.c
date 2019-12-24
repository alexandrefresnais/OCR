#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include "../headers/pixel_operations.h"

//Convert a RGBA image to a grayscaled image
void ToGrayscale(SDL_Surface *image)
{
    size_t width = image->w;
    size_t height = image->h;
    for (size_t i = 0; i < width; i++)
    {
        for (size_t j = 0; j < height; j++)
        {
            Uint32 pixelValue= get_pixel(image,i,j);
            Uint8 r,g,b;
            SDL_GetRGB(pixelValue, image->format, &r, &g, &b);
            Uint8 average = 0.3*r + 0.59*g + 0.11*b;
            Uint32 newPixel = SDL_MapRGB(image->format, average, average, average);
            put_pixel(image,i,j,newPixel);
        }
    }
}

/* *********************************************** */
/* *************** Binarization  ***************** */
/* *********************************************** */

//Count every occurences of each gray shade
void fillHistogram(float *histogram, SDL_Surface *image)
{
    /*Getting pixel array*/
    Uint32 *pixels = (Uint32*)(image)->pixels;
    /*For each pixel*/
    for(int i  = 0; i < (image)->w * (image)->h;i++)
    {
        Uint32 currentPixel = pixels[i];
        Uint8 grayValue = currentPixel & 0xFF;
        histogram[grayValue]++;
    }
}

//Divide each number by nbPixels
void normalizeHistogram(float *histogram, int nbPixels)
{
    for(int i  = 0; i < 256; i++)
    {
        histogram[i]/=nbPixels;
    }
}

//Return the frequency between lowerBound and upperBound
float frequency(float *histogram, int lowerBound, int upperBound)
{
    float freq = 0.0;
    for (int i = lowerBound; i < upperBound; i++)
    {
        freq +=histogram[i];
    }
    return freq;
}

//Return the average value between lowerBound and upperBound
float average(float *histogram, float freq, int lowerBound, int upperBound)
{
    float avr = 0;
    for (int i = lowerBound; i < upperBound; i++)
    {
        avr+=(i*histogram[i]/freq);
    }
    return avr;
}

//Return variance between lowerBound and upperBound
float variance(float *histogram, float avr, int lowerBound, int upperBound)
{
    float variance = 0;
    for (int i = lowerBound; i < upperBound; i++)
    {
        variance+=((i-avr)*(i-avr)*histogram[i]);
    }
    return variance;
}

//Find the value  that maximize the inter-class variance ansd return the corresponding treshold
int otsuMethod(SDL_Surface *image)
{
    float histogram[256] = {0};
    fillHistogram(histogram, image);
    normalizeHistogram(histogram, image->w*image->h);
    float nMax = 0;
    int threshold = 0;
    for (int i = 0; i < 256; i++)
    {
        float freq0  = frequency(histogram,0,i);
        float freq1  = frequency(histogram,i,256);
        float avr0 = average(histogram, freq0,0,i);
        float avr1 = average(histogram, freq1,i,256);
        float varB =  freq0*freq1*(avr1-avr0)*(avr1-avr0);

        if(varB >= nMax)
        {
            nMax = varB;
            threshold = i;
        }
    }
    return threshold;
}

//Get threshold value from Otsu's Method
//and then convert gray pixels into black or white ones
void Binarize(SDL_Surface *image)
{
    int threshold = otsuMethod(image);
    //printf("Binarization threshold value is %d\n",threshold);

    size_t width = image->w;
    size_t height = image->h;
    for (size_t i = 0; i < width; i++)
    {
        for (size_t j = 0; j < height; j++)
        {
            Uint32 pixelValue= get_pixel(image,i,j);
            Uint8 r,g,b;
            SDL_GetRGB(pixelValue, image->format, &r, &g, &b);
            Uint32 newPixel;
            if(r>=threshold)
                newPixel = SDL_MapRGB(image->format, 255, 255, 255);
            else
                newPixel = SDL_MapRGB(image->format, 0, 0, 0);
            put_pixel(image,i,j,newPixel);
        }
    }
}

int check_pixel(SDL_Surface* s, int i, int j)
{
    Uint32 pixel = get_pixel(s, i, j);
    Uint8 r, g, b;
    SDL_GetRGB(pixel, s->format, &r, &g, &b);
    if(r < 200 && g < 200 && b < 200)//pixel noir
    {
        return 1;
    }
    return 0;
}


void Noise_reduction(SDL_Surface* image_surface)
{
    int width = image_surface->w;
    int height = image_surface->h;
    for(int i = 0; i < width; i++)
    {
        for(int j = 0; j < height; j++)
        {
            if(check_pixel(image_surface, i, j))//Pixel noir
            {
                int sum = 0;
                for(int cur_i = i - 5; cur_i < i + 5; cur_i++)
                {
                    for(int cur_j = j - 5; cur_j < j + 5; cur_j++)
                    {
                        if(cur_i < width && cur_i >= 0 &&
                           cur_j < height && cur_j >= 0)//Good coords
                        {
                            sum += check_pixel(image_surface, cur_i, cur_j);
                        }
                    }
                }
                if(sum > 0 && sum < 20)
                {
                    Uint32 pixel = SDL_MapRGB(image_surface->format, 255, 255, 255);
                    put_pixel(image_surface, i, j, pixel);
                }
            }
        }
    }
}

/*Run length smooth algorithm*/

void RLSAFillArrays(SDL_Surface *image,int* horArray, int* vertArray, size_t Chor, size_t Cvert)
{
    size_t h = image->h;
    size_t w = image->w;
    for (size_t i = 0; i < h; i++)
    {
        size_t horCount = 0;
        for (size_t j = 0; j < w; j++)
        {
            Uint32 pixelValue= get_pixel(image,j,i);
            Uint8 r,g,b;
            SDL_GetRGB(pixelValue, image->format, &r, &g, &b);
            //image is binarized so only looking to one color
            horArray[i*w+j] = (r == 0);
            if(r!=0 && j<w-1)
            {
                horCount++;
            }
            if((r!=0 && j == w-1) || r==0)
            {
                if(horCount > 0 && horCount<=Chor)
                {
                    size_t k =i*w+ (j-horCount);
                    while (k<=i*w+j)
                    {
                        horArray[k] = 1;
                        k++;
                    }
                }
                horCount=0;
            }
        }
    }

    for (size_t i = 0; i < w; i++)
    {
        size_t vertCount =0;
        for (size_t j = 0; j < h; j++)
        {
            Uint32 pixelValue= get_pixel(image,i,j);
            Uint8 r,g,b;
            SDL_GetRGB(pixelValue, image->format, &r, &g, &b);
            //image is binarized so only looking to one color
            vertArray[i*h+j] = (r == 0);
            if(r!=0 && j<h-1)
            {
                vertCount++;
            }
            if((r!=0 && j == h-1) || r==0)
            {
                if(vertCount > 0 && vertCount<=Cvert)
                {
                    size_t k =i*h+ (j-vertCount);
                    while (k<=i*h+j)
                    {
                        vertArray[k] = 1;
                        k++;
                    }
                }
                vertCount=0;
            }
        }
    }
}

void AND(int* hor, int* ver, int* resultArray, size_t w,size_t h)
{
    for (size_t i = 0; i < h; i++)
    {
        for (size_t j = 0; j < w; j++)
        {
            resultArray[i*w+j] = hor[i*w+j] && ver[j*h+i];
        }
    }
}

typedef struct Block
{
    size_t left_x;
    size_t top_y;
    size_t right_x;
    size_t bottom_y;
    int area;

    struct Block* next;
}Block;

size_t maxx(size_t a, size_t b, size_t c, size_t d)
{
    size_t max1 = (((a)>(b))?(a):(b));
    size_t max2 = (((c)>(d))?(c):(d));
    return (((max1)>(max2))?(max1):(max2));
}

size_t FarestLeftPixel(int* rlsaArray,size_t w,size_t farestBottom,size_t x,size_t y)
{
    int foundAPixel = 1;
    while(x>0 && foundAPixel)
    {
        foundAPixel = 0;
        size_t i = y;
        while(i<farestBottom)
        {
            if(rlsaArray[i*w+x])
            {
                foundAPixel = 1;
                break;
            }
            i++;
        }
        x--;
    }
    return x+1;
}

size_t FarestRightPixel(int* rlsaArray,size_t w,size_t farestBottom,size_t x,size_t y)
{
    int foundAPixel = 1;
    while(x<w && foundAPixel)
    {
        foundAPixel = 0;
        size_t i = y;
        while(i<farestBottom)
        {
            if(rlsaArray[i*w+x])
            {
                foundAPixel = 1;
                break;
            }
            i++;
        }
        x++;
    }
    return x-1;
}

size_t FarestBottomPixel(int* rlsaArray,size_t w,size_t h,size_t y)
{
    int foundAPixel = 1;
    while(y<h && foundAPixel)
    {
        foundAPixel = 0;
        size_t i = 0;
        while(i<w)
        {
            if(rlsaArray[y*w+i])
            {
                foundAPixel = 1;
                break;
            }
            i++;
        }
        y++;
    }
    return y-1;
}

void LabelingBlocks(Block *blocks,int* rlsaArray, int w,int h)
{
    int i = 0;
    int j = 0;
    int lastBlockY=0;
    int lastBlockX=0;
    int firstBlockX=0;

    while(i<h)
    {
        if(i<lastBlockY)
            j=lastBlockX+1;
        else
            j=0;
        while(j<w)
        {
            if(rlsaArray[i*w+j])
            {
                lastBlockY = FarestBottomPixel(rlsaArray,w,h,i);
                lastBlockX = FarestRightPixel(rlsaArray,w,lastBlockY,j,i);
                firstBlockX = FarestLeftPixel(rlsaArray,w,lastBlockY,j,i);
                j = (lastBlockX+1);
                Block *block = malloc(sizeof(Block));
                block->left_x = firstBlockX;
                block->top_y = i;
                block->right_x = lastBlockX;
                block->bottom_y = lastBlockY;
                block->area = (lastBlockY-i)*(lastBlockX-firstBlockX);
                block->next = NULL;
                blocks->next = block;
                blocks = blocks->next;
            }
            j++;
        }
        i++;
    }
}

int AverageBlockArea(Block *blocks)
{
    int average = 0;
    int count =0;
    while(blocks->next)
    {
        count++;
        average+=blocks->next->area;
        blocks=blocks->next;
    }
    return average/count;
}

void DeleteImages(SDL_Surface* image, Block *blocks,int averageArea, int factor)
{
    while(blocks->next)
    {
        blocks=blocks->next;
        if(blocks->area > factor*averageArea||blocks->area<20)
        {
            for (size_t i = blocks->top_y; i < blocks->bottom_y; i++)
            {
                for (size_t j = blocks->left_x; j < blocks->right_x; j++)
                {
                    Uint32 newPixel;
                    newPixel = SDL_MapRGB(image->format, 255, 255, 255);
                    put_pixel(image,j,i,newPixel);
                }
            }
        }
    }
}

void RLSA(SDL_Surface* image)
{
    const int Chor = 15;
    const int Cvert = 500;
    const int imageFactor = 6;
    printf("RLSA...\n");
    size_t h = image->h;
    size_t w = image->w;

    int *horizontalRLSA = malloc(sizeof(int)*h*w);
    int *verticalRLSA = malloc(sizeof(int)*h*w);

    int *ANDResult = malloc(sizeof(int)*h*w);
    //Running throught image to get pixel values
    RLSAFillArrays(image,horizontalRLSA,verticalRLSA, Chor,Cvert);

    AND(horizontalRLSA,verticalRLSA,ANDResult,w,h);

    free(horizontalRLSA);
    free(verticalRLSA);

    SDL_Surface *surface = IMG_Load("tmp/binarized.bmp");
    for (size_t i = 0; i < h; i++)
    {
        for (size_t j = 0; j < w; j++)
        {
            Uint32 newPixel;
            if(ANDResult[i*w+j])
                newPixel = SDL_MapRGB(surface->format, 0, 0, 0);
            else
                newPixel = SDL_MapRGB(surface->format, 255, 255, 255);
            put_pixel(surface,j,i,newPixel);
        }
    }
    SDL_SaveBMP(surface, "tmp/Block.bmp");
    SDL_FreeSurface(surface);

    Block blocks= {0,0,0,0,0,NULL};
    LabelingBlocks(&blocks,ANDResult,w,h);

    int averageArea = AverageBlockArea(&blocks);
    DeleteImages(image,&blocks, averageArea,imageFactor);

    free(ANDResult);
}
