#include "../headers/segmentation.h"
#include "../headers/pixel_operations.h"
#include <SDL/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>
//=================BINARIZATION=======================
void SurfaceToBinarizedImage(SDL_Surface *image, BinarizedImage *target)
{
    target->height = image->h;
    target->width = image->w;
    size_t h = target->height;
    size_t w = target->width;
    target->values= malloc(sizeof(int)*h*w);

    for (size_t y = 0; y < h; y++)
    {
        for (size_t x = 0; x < w; x++)
        {
            Uint32 pixelValue = get_pixel(image,x,y);
            Uint8 r,g,b;
            SDL_GetRGB(pixelValue, image->format, &r, &g, &b);
            if(r>0)
			{
                target->values[y*w+x] = 0;
			}
            else
			{
                target->values[y*w+x] = 1;
			}
        }
    }
}

//Delete lines of zeros by increasing begin_corner.y
void ResizeCharHeight(ListOfChar chr, BinarizedImage* fullImage)
{
	for (size_t y = chr->begin_corner.y; y < chr->end_corner.y; y++)
	{
		for (size_t x = chr->begin_corner.x; x < chr->end_corner.x; x++)
		{
			if(fullImage->values[y*fullImage->width+x] == 1)
			{
				return;
			}
		}
		//if we are here we found no black pixel
		chr->begin_corner.y++;
	}
}

//Convert char to binarized image. Original is the full image of the text
void CharToBinarizedImage(BinarizedImage* original, ListOfChar chr,
										BinarizedImage* target)
{
	ResizeCharHeight(chr,original);
	target->height = chr->end_corner.y - chr->begin_corner.y;
	target->width = chr->end_corner.x - chr->begin_corner.x;
	size_t h = target->height;
	size_t w = target->width;
	target->values= malloc(sizeof(int)*h*w);
	for (size_t y = 0; y < h; y++)
	{
		for (size_t x = 0; x < w; x++)
		{
			target->values[y*w+x] = original->values[
				original->width*(chr->begin_corner.y+y)+chr->begin_corner.x+x
			];
		}
	}
}


ListOfChar Segmentate(BinarizedImage *image)
{
    //Getting every Line
	ListOfLine split_line = Splitting_L(image, 0);
    split_line = insert_paragraph(split_line);
    //Getting every char
    ListOfChar split_char = Splitting_C(image, 0, split_line, 1);
    split_char = insert_space(split_char);

    return split_char;
}

//========== SEGMENTATION ============

size_t max4(size_t a, size_t b, size_t c, size_t d)
{
    size_t max1 = (((a)>(b))?(a):(b));
    size_t max2 = (((c)>(d))?(c):(d));
    return (((max1)>(max2))?(max1):(max2));
}

size_t FarestPixel(BinarizedImage *visited,size_t x,size_t y)
{
    if(x==visited->width)
        return x-1;

    if(y>= visited->height)
        return 0;

    if(visited->values[y*visited->width+x]==0)
    {
        return x;
    }

    visited->values[y*visited->width+x]=0;

	if(x > 0 && y > 0)
		return max4(
    	FarestPixel(visited,x+1,y),
    	FarestPixel(visited,x-1,y),
    	FarestPixel(visited,x,y+1),
    	FarestPixel(visited,x,y-1));

	if(x == 0)
	{
		if(y == 0)
		{
			return max4(
    		FarestPixel(visited,x+1,y),
    		0,
    		FarestPixel(visited,x,y+1),
    		0);
		}

		return max4(
    	FarestPixel(visited,x+1,y),
    	0,
    	FarestPixel(visited,x,y+1),
    	FarestPixel(visited,x,y-1));
	}

	return max4(
		FarestPixel(visited,x+1,y),
    	FarestPixel(visited,x-1,y),
    	FarestPixel(visited,x,y+1),
    	0);
}

int CheckChar(Coordinates topLeft,Coordinates bottomRight,BinarizedImage image)
{
    BinarizedImage visited;
    size_t width = bottomRight.x - topLeft.x;
    size_t height = bottomRight.y - topLeft.y;
    visited.height = height;
    visited.width = width;
    visited.values = malloc(width*height*sizeof(int));
    for (size_t y = topLeft.y; y < topLeft.y+height; y++)
    {
        for (size_t x = topLeft.x; x < topLeft.x+width; x++)
        {
            visited.values[(y-topLeft.y)*width+(x-topLeft.x)]= image.values[y*image.width+x];
        }
    }
    size_t i =0;
    size_t j = 0;
    int found = 0;
	int error = 0;

    while(error == 0 && found ==0)
    {
        while(j<height&&found==0)
        {
            if(visited.values[j*width+i])
                found =1;
            if(found == 0)
                j++;
        }
        if(found == 0)
        {
            j=0;
			if(i < width)
            	i++;
			else
				error = 1;
        }
    }
	if(error == 1)
		errx(1, "Segmentated element with no pixel!!");
	else
    	return FarestPixel(&visited,i,j);
}

ListOfLine Splitting_L(BinarizedImage* image, size_t i) //Use XY-Cut
{
    size_t heigth = image->height;
    size_t width = image->width;
    size_t begin = 0;
    int check = -1;

    ListOfLine lines;

    while(i < heigth && check == -1) //search for black pixels
    {
        size_t j = 0;

        while(j < width && check == -1)
        {
            int pixel = image->values[i*width + j];

            if(pixel == 1)					  //Black pixel found
            {
                if (i != 0)
                    begin = i-1;		  //Upper limit set
                check = 0;
            }

            j++;
        }

        i++;
    }

    if(i == heigth) //No more line found
        return NULL;

    size_t j = 0;

    while(i < heigth && j < width) //search for Bottom Limit
    {
        int pixel = image->values[i*width + j];

        if (pixel == 1) 		  //Black pixel found...
        {
            i++; 		  //... not the end of the line
            j = 0; 		  // try again
        }
        else
            j++;
    }

    if(j == width) //Only White pixels = end of the line
    {
        lines = insert_line(begin, i,
                Splitting_L(image, i), 0);
    }
    else //End of the line = end of the image
    {
        lines = insert_line(begin, i-1, NULL, 0);;
    }

    return lines;
}

ListOfChar Splitting_C(BinarizedImage* image, size_t j, ListOfLine line, int boool)
{
    size_t width = image->width;
    int x_check = -1;
    size_t x_begin = 0;
    size_t save_begin = j; //only used if no more char on the line

    ListOfChar characters;

	if(line->special_case == 1)
	{
		characters = insert_char(__init_coord(j, line->begin),
				__init_coord(width-1, line->end), 3,
				Splitting_C(image, 0, line->follower, 1));

		//Free memory space
		free(line);

		return characters;
	}

    while(j < width && x_check == -1)  //search for black pixels
    {
        size_t i = line->begin+1;

        while(i < line->end && x_check == -1)
        {
            int pixel = image->values[i*width + j];

            if(pixel == 1) //black pixel found
            {
                x_begin = j-1;  //left limit set
                x_check = 0;
			}
            i++;
        }

        j++;
    }

    if(j == width) //end of line = no more char on this line
    {
        if(line->follower == NULL) //no more line = no more char
		{
			if(boool == 1)
				free(line);

            return NULL;
		}

        //Add "\n" character and continue with next line
        characters = insert_char(__init_coord(save_begin, line->begin),
                __init_coord(width-1, line->end), 1,
                Splitting_C(image, 0, line->follower, 1));

        //Free memory space
       	if(boool == 1)
            free(line);

        return characters;
    }

    size_t i = line->begin+1;

   	while (j < width && i < line->end) //search for rigth limit
    {
        size_t pixel = image->values[i*width + j];

        if(pixel == 1) //black pixel found == still not the limit
        {
            j++;
            i = line->begin+1;
        }
        else
            i++;
    }

    if(i == line->end) //Only White Pixels on this line
    {
        int farestCommonPixel = CheckChar(__init_coord(x_begin, line->begin),
                __init_coord(j, line->end),*image);

        //We have two characters in one
        if(x_begin+farestCommonPixel+1 < j)
        {
            //First one
            characters = insert_char(__init_coord(x_begin, line->begin),
                __init_coord(x_begin+farestCommonPixel-1, line->end), 0,
                NULL);
            //Second one
            characters->follower =
            insert_char(__init_coord(x_begin+farestCommonPixel-1, line->begin),
                __init_coord(j, line->end), 0,
                Splitting_C(image, j+1, line, 0));
        }
        else
        {
            //Add this character and continue with next one
            characters = insert_char(__init_coord(x_begin, line->begin),
                    __init_coord(j, line->end), 0,
                    Splitting_C(image, j+1, line, 0));
        }
    }
    else //end of the line
    {
        if(line->follower == NULL) //no more line = no char following
        {
            characters = insert_char(__init_coord(x_begin, line->begin),
                    __init_coord(j-1, line->end), 0, NULL);
        }

        else //add "/n" character (next call of the function) AND
            //COntinue with next line
        {
            characters = insert_char(__init_coord(x_begin, line->begin),
                    __init_coord(j-1, line->end), 0,
                    Splitting_C(image, j, line, 0));
        }
    }

	if(boool == 1)
    	 free(line);

    return characters;
}

//==================INSERT METHODS======================

ListOfLine insert_line(size_t up, size_t bottom, ListOfLine Q, int special)
{
    ListOfLine L;

    L = (ListOfLine)malloc(sizeof(struct Line));
	if(L == NULL)
		errx(1, "Line: Not enough memory!\n");
    L->begin = up;
    L->end = bottom;
	L->special_case = special;
    L->follower = Q;

	if(Q == NULL)
	{
		L->sumOfGap = 0;
		L->nbOfGap = 0;
	}
	else
	{
		L->sumOfGap = (Q->begin - bottom) + Q->sumOfGap;
		L->nbOfGap = 1 + Q->nbOfGap;
	}

    return (L);
}

ListOfChar insert_char(Coordinates ul_corner, Coordinates br_corner,
        int special, ListOfChar Q)
{
    ListOfChar C;

    C = (ListOfChar)malloc(sizeof(struct Character));
	if(C == NULL)
		errx(1, "Character: Not enough memory!\n");
    C->begin_corner = ul_corner;
    C->end_corner = br_corner;

	C->size = br_corner.x - ul_corner.x;

    C->special_case = special;

    C->follower = Q;

	if(Q == NULL)
	{
		C->sumOfGap = 0;
		C->nbOfGap = 0;
		C->maxSize = C->size;
	}
	else
	{
		if(Q->special_case > 0)
		{
			C->sumOfGap = Q->follower->sumOfGap;
			C->nbOfGap = Q->follower->nbOfGap;

			if(C->size > Q->follower->maxSize)
				C->maxSize = C->size;
			else
				C->maxSize = Q->follower->maxSize;
		}
		else
		{
			C->sumOfGap = (Q->begin_corner.x - br_corner.x) + Q->sumOfGap;
			C->nbOfGap = 1 + Q->nbOfGap;

			if(C->size > Q->maxSize)
				C->maxSize = C->size;
			else
				C->maxSize = Q->maxSize;
		}
	}

    return (C);
}


//Add "space" character between words
ListOfChar insert_space(ListOfChar characters)
{
    ListOfChar temp = characters;

	size_t gapAverage = SIZE_MAX;
	if(characters->nbOfGap > 0)
		gapAverage = characters->sumOfGap / characters->nbOfGap;

    size_t first = temp->end_corner.x;
    size_t celling = temp->begin_corner.y;
    ListOfChar previous = temp;

    temp = temp->follower;

    while(temp != NULL)
    {
        if(temp->special_case < 1)
        {
            size_t second = temp->begin_corner.x;

            if(second - first > 2*gapAverage)
            {
                //ListOfChar suite = temp->follower;
                previous->follower = insert_char(
                        __init_coord(first, celling),
                        __init_coord(temp->begin_corner.x,
                            temp->end_corner.y),
                        2, temp);
            }

            first = temp->end_corner.x;
            celling = temp->begin_corner.y;
            previous = temp;
            temp = temp->follower;
        }

        else
        {
            temp = temp->follower;
            first = temp->end_corner.x;
            celling = temp->begin_corner.y;
            previous = temp;
            temp = temp->follower;
        }

    }

    return characters;
}

ListOfLine insert_paragraph(ListOfLine lines)
{
	ListOfLine temp = lines;
	size_t gapAverage = SIZE_MAX;

	if(lines->nbOfGap > 0)
		gapAverage = lines->sumOfGap / lines->nbOfGap;

	size_t end = temp->end;
	ListOfLine previous = lines;

	temp = temp->follower;

	while(temp != NULL)
	{
		if(temp->special_case < 1)
		{
			size_t begin = temp->begin;

			if(begin - end >= 2*gapAverage)
			{
				previous->follower = insert_line(end, begin, temp, 1);
			}

			end = temp->end;
			previous = temp;
			temp = temp->follower;
		}
		else
		{
			temp = temp->follower;
			end = temp->end;
			previous = temp;
			temp = temp->follower;
		}
	}

	return lines;
}
//===================INIT COORDINATES====================

Coordinates __init_coord(size_t x, size_t y)
{
    Coordinates pos;

    pos.x = x;
    pos.y = y;

    return pos;
}

//====================OTHER==============================

void draw_cell(SDL_Surface *image, size_t i, size_t j)
{
    // Getting splitting information
    BinarizedImage bin_image;
    SurfaceToBinarizedImage(image, &bin_image);
    ListOfLine split_line = Splitting_L(&bin_image, i);
	split_line = insert_paragraph(split_line);

    // Pixel red
    Uint32 redPixel = SDL_MapRGB(image->format, 255, 0, 0);
    // Pixel green
    Uint32 greenPixel = SDL_MapRGB(image->format, 0, 255, 0);

    ListOfLine temp = split_line;
    testLine(split_line);

    while (temp != NULL)
    {
        for (i = 0; i < (size_t)image->w; i++)
        {
            put_pixel(image, i, temp->begin, redPixel);
            put_pixel(image, i, temp->end, redPixel);
        }

        temp = temp->follower;
    }

    ListOfChar split_char = Splitting_C(&bin_image, j, split_line, 1);
    testCharBeforeSpace(split_char);
    split_char = insert_space(split_char); //add " " Char

    while (split_char != NULL)
    {
        if(split_char->special_case < 1)
        {
            for (j = split_char->begin_corner.y; j <= split_char->end_corner.y; j++)
            {
                put_pixel(image, split_char->begin_corner.x, j, greenPixel);
                put_pixel(image, split_char->end_corner.x, j, greenPixel);
            }
        }
        else
        {
            if(split_char->special_case == 2)
            {
                for (j = split_char->begin_corner.y; j <= split_char->end_corner.y; j++)
                    for(i = split_char->begin_corner.x ; i <= split_char->end_corner.x; i++)
                    {
                        put_pixel(image, i, j, greenPixel);
                    }
            }
			if(split_char->special_case == 3)
			{
				for (j = split_char->begin_corner.y; j <= split_char->end_corner.y; j++)
                    for(i = split_char->begin_corner.x ; i <= split_char->end_corner.x; i++)
                    {
                        put_pixel(image, i, j, redPixel);
                    }
			}
        }

        ListOfChar tempo = split_char->follower;

        //Free memory used by current Struct
        free(split_char);
        split_char = tempo;
    }

    SDL_SaveBMP(image, "tmp/after_segment_binarized.bmp");
}


void testCharBeforeSpace(ListOfChar chara)
{
    ListOfChar test = chara;
    int nbOfChar = 0;

    //printf("===========TEST============\n");

    while(test != NULL)
    {
        //printf("char num %i :\n", nbOfChar);
        //printf("\tBegin: x=%zd y=%zd\n", test->begin_corner.x,test->begin_corner.y);
        //printf("\tEnd  : x=%zd y=%zd\n", test->end_corner.x,test->end_corner.y);
        //printf("\tSpe = %i\n\n", test->special_case);
        nbOfChar++;
        test = test->follower;
    }

    //printf("Number of char: %i\n", nbOfChar);
    //printf("==============END OF TEST==========\n\n");
}

void testLine(ListOfLine line)
{
    ListOfLine test = line;
    int nbOfLine = 0;

    //printf("===========TEST============\n");

    while(test != NULL)
    {
        //printf("Line num %i :\n", nbOfLine);
        //printf("\tBegin: y=%zd\n", test->begin);
        //printf("\tEnd  : y=%zd\n", test->end);
        nbOfLine++;
        test = test->follower;
    }

    //printf("Number of Line: %i\n", nbOfLine);
    //printf("==============END OF TEST==========\n\n");
}
