//* =+= Image Header File =+= *//

#pragma once
#include <iostream>
#include <cmath>
#include <istream>
#include <ostream>
#include <string>
#include <vector>

using namespace std;

//Pixels are read in the opposite order, so blue -> green -> red
struct Pixel
{
    unsigned char blue, green, red;
};

/* Image Class */
struct Header
{
    char idLength;
    char colorMapType;
    char dataTypeCode;
    short colorMapOrigin;
    short colorMapLength;
    char colorMapDepth;

    //Image data
    short xOrigin;
    short yOrigin;
    short width;
    short height;
    char bitsPerPixel;
    char imageDescriptor;
};

class Image
{
public:
    //Constructor
    Image()
    {}
    //Default constructor
    Image(Header h) {
        headerObject = h;
        pixelVector = vector<Pixel>(headerObject.width * headerObject.height);
    }

    vector<Pixel> pixelVector;
    Header headerObject;
};


