#ifndef IMAGE_H
#define IMAGE_H

#include <stdio.h>
#include <time.h>
#include <stdint.h>

typedef unsigned char octet;

class Image {
    protected:
        size_t width;
        size_t height;

    public:

        /* Constructeurs et destructeurs */
        Image();
        Image(size_t newWidth, size_t newHeight);
        virtual ~Image(); // Destructeur virtuel

        /* Accesseurs et opérateurs d'accès */
        size_t getWidth();
        size_t getHeight();
        size_t size();
        virtual octet* operator[] (size_t i); // Opérateur virtuel
        virtual octet average();

        /* Setter virtuel */
        virtual void resize(size_t newWidth, size_t newHeight);
        virtual void segment(size_t blockSize);
        virtual void mosaic(size_t blockSize, const char* libPath, size_t libSize);

        /* Chiffrement */
        virtual size_t* swap(size_t blockSize);
        virtual void sort(size_t* key, size_t blockSize);
    
        /* Lecture et écriture dans des fichiers */
        virtual void read(const char* path);
        virtual void write(const char* path, const char* comment = nullptr);
};

uint32_t getNanoSecondSeed();
void printPercent(int pourcentage);
#endif