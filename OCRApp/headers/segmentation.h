#ifndef SEGMENTATION_H
#define SEGMENTATION_H

#include <SDL/SDL.h>

typedef struct BinarizedImage
{
	int* values;

	size_t width;
	size_t height;
}BinarizedImage;

void SurfaceToBinarizedImage(SDL_Surface *image, BinarizedImage *target);

//structure for pixels coordinates
typedef struct Coordinates Coordinates;
struct Coordinates
{
		size_t x;
		size_t y;
};

//structure for line detection and splitting
typedef struct Line Line;
struct Line
{
		size_t begin;
		size_t end;

		size_t sumOfGap;
		size_t nbOfGap;

		int special_case; // 0 = Line / 1 = New Paragraph

		struct Line *follower;
};

typedef struct Line *ListOfLine;

//structure for character detection and splitting
typedef struct Character Character;
struct Character
{
		Coordinates begin_corner;
		Coordinates end_corner;

		size_t sumOfGap;
		size_t nbOfGap;

		size_t size;
		size_t maxSize;

		int special_case; // 0 = Normal Char / 1 = "\n" / 2 = " " (space) /
						  // 3 = New Paragraph

		struct Character *follower;
};

typedef struct Character *ListOfChar;

void CharToBinarizedImage(BinarizedImage* original, ListOfChar chr,
										BinarizedImage* target);

ListOfChar Segmentate(BinarizedImage *image);

//Segmentation
ListOfLine Splitting_L(BinarizedImage* image, size_t i);
ListOfChar Splitting_C(BinarizedImage* image, size_t j,
							ListOfLine line, int bool);

//Init Coordinates
Coordinates __init_coord(size_t x, size_t y);

//Insert Methods
ListOfLine insert_line(size_t up, size_t bottom, ListOfLine Q, int special_case);
ListOfChar insert_char(Coordinates ul_corner, Coordinates br_corner,
				int special, ListOfChar Q);

ListOfChar insert_space(ListOfChar characters);
ListOfLine insert_paragraph(ListOfLine lines);

//Other

void draw_cell(SDL_Surface *image, size_t i, size_t j);
void testCharBeforeSpace(ListOfChar chara);
void testLine(ListOfLine line);
#endif
