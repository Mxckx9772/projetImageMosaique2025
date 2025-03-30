#include "../include/ImagePGM.hpp"
#include <stdlib.h>
#include <iostream>

using namespace std;

/* Constructeur et destructeurs */
ImagePGM::ImagePGM() : Image(), grey(nullptr) {}

ImagePGM::ImagePGM(size_t newWidth, size_t newHeight) : Image(newWidth, newHeight) {
    grey = (octet *)calloc(width * height, sizeof(octet));
}

ImagePGM::~ImagePGM() {
    free(grey);
}

/* Opérateur d'accès */
octet *ImagePGM::operator[](size_t i) {
    if (i < height)
    {
        return grey + (i * width);
    }

    cerr << "Indices hors des bornes." << endl;
    exit(EXIT_FAILURE);
}

void ImagePGM::operator= (const ImagePGM &other) {
    width = other.width;
    height = other.height;

    grey = (octet *) calloc(size(), sizeof(octet));
    for(size_t i = 0; i < size(); i++){
        grey[i] = other.grey[i];
    }


}

/* Setter */
void ImagePGM::resize(size_t newWidth, size_t newHeight) {
    if ((width != newWidth) || (height != newHeight))
    {
        size_t newSize;
        octet *newGrey;

        newSize = newWidth * newHeight;
        newGrey = (octet *)calloc(newSize, sizeof(octet));

        if (newGrey != NULL)
        {
            float widthRatio = (float)(width - 1) / (float)(newWidth - 1);
            float heightRatio = (float)(height - 1) / (float)(newHeight - 1);

            for (size_t y = 0; y < newHeight; ++y)
            {
                for (size_t x = 0; x < newWidth; ++x)
                {
                    float srcX = x * widthRatio;
                    float srcY = y * heightRatio;

                    int x1 = srcX;
                    int y1 = srcY;
                    int x2 = min(x1 + 1, (int)(width - 1));
                    int y2 = min(y1 + 1, (int)(height - 1));

                    float xWeight = srcX - x1;
                    float yWeight = srcY - y1;

                    octet topLeft = grey[y1 * width + x1];
                    octet topRight = grey[y1 * width + x2];
                    octet bottomLeft = grey[y2 * width + x1];
                    octet bottomRight = grey[y2 * width + x2];

                    float top = topLeft * (1 - xWeight) + topRight * xWeight;
                    float bottom = bottomLeft * (1 - xWeight) + bottomRight * xWeight;

                    newGrey[y * newWidth + x] = (top * (1 - yWeight) + bottom * yWeight);
                }
            }

            free(grey);
            Image::resize(newWidth, newHeight);
            grey = newGrey;
        }
    }
}

void ImagePGM::segment(size_t newBlockSize) {
    if ((newBlockSize >= width) || (newBlockSize >= height))
    {
        cerr << "Erreur - Taille des block supérieure aux dimenssion de l'image " << endl;
        exit(EXIT_FAILURE);
    }

    size_t newWidth, newHeight;
    float average;
    size_t squarednewBlockSize = newBlockSize * newBlockSize;

    newWidth = newBlockSize * (width / newBlockSize);
    newHeight = newBlockSize * (height / newBlockSize);

    resize(newWidth, newHeight);

    for (size_t i = 0; i < height; i += newBlockSize)
    {
        for (size_t j = 0; j < width; j += newBlockSize)
        {
            average = 0.0f;
            for (size_t x = 0; x < newBlockSize; x++)
            {
                for (size_t y = 0; y < newBlockSize; y++)
                {
                    average += grey[((i + x) * width) + (j + y)];
                }
            }

            average /= squarednewBlockSize;

            for (size_t x = 0; x < newBlockSize; x++)
            {
                for (size_t y = 0; y < newBlockSize; y++)
                {
                    grey[((i + x) * width) + (j + y)] = average;
                }
            }
        }
    }
}


size_t *ImagePGM::swap(size_t blockSize) {
    size_t widthFactor, heightFactor, nbBlock;
    size_t newWidth, newHeight;
    octet *newGrey;
    size_t *key;

    widthFactor = width / blockSize;
    heightFactor = height / blockSize;
    nbBlock = widthFactor * heightFactor;

    newWidth = blockSize * widthFactor;
    newHeight = blockSize * heightFactor;

    resize(newWidth, newHeight);

    key = (size_t *) calloc(nbBlock, sizeof(size_t));
    newGrey = (octet *) calloc(size(), sizeof(octet));

    for (size_t i = 0; i < nbBlock; i++) {
        key[i] = i;
    }

    srand(getNanoSecondSeed());

    for (size_t i = nbBlock - 1; i > 0; i--) {
        size_t j = rand() % (i + 1);
        size_t temp = key[i];
        key[i] = key[j];
        key[j] = temp;
    }


    size_t blockRow, blockCol, swappedBlockRow, swappedBlockCol;
    size_t index, swappedIndex;
    size_t swappedBlockId;

    for (size_t blockId = 0; blockId < nbBlock; blockId++) {
        swappedBlockId = key[blockId];

        blockRow = blockId / widthFactor;
        blockCol = blockId % widthFactor;
        swappedBlockRow = swappedBlockId / widthFactor;
        swappedBlockCol = swappedBlockId % widthFactor;

        for (size_t i = 0; i < blockSize; i++) {
            for (size_t j = 0; j < blockSize; j++) {
                index = (swappedBlockRow * blockSize + i) * width + (swappedBlockCol * blockSize + j);
                swappedIndex = (blockRow * blockSize + i) * width + (blockCol * blockSize + j);

                newGrey[index] = grey[swappedIndex];
            }
        }
    }

    free(grey);
    grey = newGrey;
    return key;
}

void ImagePGM::sort(size_t *key, size_t blockSize) {
    size_t widthFactor, heightFactor, nbBlock;
    octet *newGrey;

    widthFactor = width / blockSize;
    heightFactor = height / blockSize;
    nbBlock = widthFactor * heightFactor;

    newGrey = (octet *) calloc(size(), sizeof(octet));

    size_t blockRow, blockCol, swappedBlockRow, swappedBlockCol;
    size_t index, swappedIndex;
    size_t swappedBlockId;

    for (size_t blockId = 0; blockId < nbBlock; blockId++) {
        swappedBlockId = key[blockId];

        blockRow = blockId / widthFactor;
        blockCol = blockId % widthFactor;
        swappedBlockRow = swappedBlockId / widthFactor;
        swappedBlockCol = swappedBlockId % widthFactor;

        for (size_t i = 0; i < blockSize; i++) {
            for (size_t j = 0; j < blockSize; j++) {
                index = (swappedBlockRow * blockSize + i) * width + (swappedBlockCol * blockSize + j);
                swappedIndex = (blockRow * blockSize + i) * width + (blockCol * blockSize + j);

                newGrey[swappedIndex] = grey[index];
            }
        }
    }

    free(grey);
    grey = newGrey;
}

/* Lecture et écriture dans un fichier */
void ImagePGM::read(const char *path) {
    FILE *file;
    char format[3], c;
    octet maxValue;
    size_t size;

    /* Ouverture du fichier */
    file = fopen(path, "rb");

    if (file == NULL)
    {
        cerr << "Erreur - Pas d'accès en lecture sur l'image : " << path << endl;
        exit(EXIT_FAILURE);
    }

    /* Lecture du format */
    if (fscanf(file, "%2s", format) != 1)
    {
        cerr << "Erreur - Lecture du format impossible" << endl;
        fclose(file);
        exit(EXIT_FAILURE);
    }

    if (string(format) != "P5")
    {
        cerr << "Erreur - Format différent de P5" << endl;
        fclose(file);
        exit(EXIT_FAILURE);
    }

    /* Ignorer les commentaires */
    fgetc(file); // Ignorer le \n
    while ((c = fgetc(file)) == '#')
    {
        while ((c = fgetc(file)) != '\n')
        {
        }
    }

    ungetc(c, file); // Rejeter le caractère en trop

    /* Lecture des dimensions */
    fscanf(file, "%ld %ld %hhd", &width, &height, &maxValue);
    fgetc(file); // Ignorer le saut de ligne

    if (maxValue != 255)
    {
        cerr << "Erreur - Valeur maximal éronée" << endl;
        fclose(file);
        exit(EXIT_FAILURE);
    }

    size = width * height;

    /* Lecture des valeurs */
    grey = (octet *)calloc(width * height, sizeof(octet));

    if (fread(grey, sizeof(octet), size, file) != size)
    {
        cerr << "Erreur - Problème lors de la lecture des valeurs" << endl;
        free(grey);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    fclose(file);
}

void ImagePGM::write(const char *path, const char *comment) {
    FILE *file;
    size_t size;
    size = width * height;

    /* Ouveture du fichier */
    file = fopen(path, "wb");
    if (file == NULL)
    {
        cerr << "Erreur - Pas d'accès en écriture sur l'image : " << path << endl;
        exit(EXIT_FAILURE);
    }

    /* Ecriture du format */
    fprintf(file, "P5\r");

    /* Ecriture des commentaires */
    if (comment)
    {
        fprintf(file, "# ");
        fprintf(file, "%s\n", comment);
    }

    /* Ecriture des dimenssions */
    fprintf(file, "%ld %ld\r255\r", width, height);

    /* Eciture des valeurs */
    if (fwrite(grey, sizeof(octet), size, file) != size)
    {
        cerr << "Erreur - Problème lors de l'écriture de l'image" << endl;
        fclose(file);
        exit(EXIT_FAILURE);
    }

    fclose(file);
}

uint32_t getNanoSecondSeed() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint32_t)(ts.tv_nsec);
}