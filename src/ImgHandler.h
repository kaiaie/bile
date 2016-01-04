/*** \file ImgHandler.h
*** \brief Handler for extracting metadata from image files (GIF, PNG, JPEG)
***
*** The image's dimensions are extracted and placed in the \c $image_width and 
*** \c $image_height variables. If the image contains text metadata, this is 
*** extracted into a variable called \c $comments.
**/
#ifndef IMGHANDLER_H
#define IMGHANDLER_H
#include <stdio.h>
#include "bool.h"
#include "Vars.h"
#include "FileHandler.h"

bool imgCanHandle(char *fileName);
void imgReadMetadata(char *fileName, Vars *data);
WriteStatus imgWriteOutput(char *fileName, WriteFormat format, FILE *output);

#endif /* IMGHANDLER_H */
