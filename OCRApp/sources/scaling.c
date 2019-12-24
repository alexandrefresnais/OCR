#include "../headers/scaling.h"

int Max(int a, int b)
{
    return (a > b ? a : b);
}

void PrintBinarizedImage(BinarizedImage image,size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        for (size_t j = 0; j < size; j++)
        {
            char c =' ';
            if(image.values[i*size+j] ==1)
                c = 'o';
            printf("%c ",c);
        }
        printf("\n");
    }
}

//Add white pixel (zeros) to make a square  matrix of size * size
void ToSquareMatrix(BinarizedImage *image, size_t size)
{
    int* newImage;
    newImage = malloc(sizeof(int)*size*size);
    //fill with zeros
    memset(newImage,0,sizeof(int)*size*size);

    //Get Image infos
    size_t h = image->height;
	size_t w = image->width;

    int top = 0;
    int left = 0;
    //If needs to be resized on height
    if(size != h)
        top = size/2-(h/2);
    //If needs to be resized on width
    if(size != w)
        left = size/2-(w/2);
    for (size_t y = 0; y < h; y++)
	{
		for (size_t x = 0; x < w; x++)
		{
			newImage[(top+y)*size+left+x] =  image->values[y*w+x];
		}
	}
    image->width = size;
    image->height = size;
    image->values = newImage;
}


//Return the average pixel in a zone of factor*factor
//At the top left position x,y
int_least16_t AveragePixel(BinarizedImage *image, size_t factor, int x, int y)
{
    int nbBlackPixel = 0;
    for (size_t i = 0; i < factor; i++)
    {
        for (size_t j = 0; j < factor; j++)
        {
            nbBlackPixel +=image->values[(y+i)*image->height+x+j];
        }
    }
    return (nbBlackPixel>(int)(factor*factor)/2);
}

void Reduce(BinarizedImage *image, int factor)
{
    int* newImage;
    size_t newSize = (image->height)/factor;//height = width
    newImage = malloc(sizeof(int)*newSize*newSize);

    for (size_t i = 0; i < newSize; i++)
    {
        for (size_t j = 0; j < newSize; j++)
        {
            newImage[i*newSize+j]=AveragePixel(image,factor,j*factor,i*factor);
        }
    }
    image->width = newSize;
    image->height = newSize;
    image->values = newImage;
}

//Main function
void Scale(BinarizedImage *image, size_t size)
{
    //if not squared matrix
    if((image->width != image->height) || ((image->width < size) && (image->height < size)))
    {
        //Making a square Matrix of a size of a divisor of "size"
        int maxLength = Max(image->width,image->height);
        while(maxLength % size != 0)
            maxLength++;
        ToSquareMatrix(image,maxLength);
    }
    if(image->width/size>0)
        Reduce(image,image->width/size);
}
