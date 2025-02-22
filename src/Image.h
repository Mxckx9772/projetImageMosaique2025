#ifndef IMAGE_H
#define IMAGE_H

#include <stdio.h>
#include <stdlib.h>

typedef unsigned char octet;

class Image {
    private:
        octet** red;
        octet** green;
        octet** blue;
        int width;
        int height;
        int size;

    public:
        // Constructeur d'image
        Image(int width, int height);
        Image(const char* imgFileName);
        Image(octet** redChannel, octet** greenChannel, octet** blueChannel, int width, int height);

        // Accesseurs aux données d'image
        const octet** getRed() const;
        const octet** getGreen() const;
        const octet** getBlue() const;
        int getWidth() const;
        int getHeight() const;

        // Setters sur les données d'image
        void setRed(octet** newRed);
        void setGreen(octet** newRed);
        void setBlue(octet** newRed);

        // Ecriture et lecture de fichiers d'image
        void readImage(const char* imgFileName);
        void writeImage(const char* imgFileName);


        // Opérations sur les matrices de données
        octet** allocateImgPixel(int width, int height);
        void freeImgPixel(octet** array, int height);

};

#endif // IMAGE_H