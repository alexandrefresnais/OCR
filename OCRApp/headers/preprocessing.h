#ifndef PREPROCESSING_H
#define PREPROCESSING_H

void ToGrayscale(SDL_Surface *image);

void Binarize(SDL_Surface *image);

void Noise_reduction(SDL_Surface *image_surface);

void RLSA(SDL_Surface* image);

#endif
