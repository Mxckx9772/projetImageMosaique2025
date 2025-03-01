#ifndef IMAGE_H
#define IMAGE_H

#include <stdio.h>
#include <stdlib.h>

typedef unsigned char octet;

enum format {
    PGM,
    PPM
};

class Image {
    protected:

        size_t width = 0;
        size_t height = 0;
        size_t size = 0;

        Image(size_t width,  size_t height);
        Image();

        bool inImg(size_t i, size_t j);
        bool inImgLine(size_t i);
        bool inImgColumn(size_t);

        octet** allocateImgPixel(size_t width, size_t height);
        void freeImgPixel(octet** array, size_t height);

        format readHeader(FILE* imgFileName);
        void skipComment(FILE* imgFileName);

        virtual void readImage(const char* imgFileName){};

    public:
        virtual ~Image();

        size_t getWidth() const;
        size_t getHeight() const;
        size_t getSize() const;
        
        virtual void writeImage(const char* imgFileName){};
};

#endif // IMAGE_H