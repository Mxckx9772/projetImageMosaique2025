#include "../include/ImagePPM.hpp"
#include <stdlib.h>
#include <iostream>

using namespace std;

/* Constructeur et destructeurs */
ImagePPM::ImagePPM() : Image(), colors(nullptr) {}

ImagePPM::ImagePPM(size_t newWidth, size_t newHeight) : Image(newWidth, newHeight) {
    colors = (octet *)calloc(width * height * 3, sizeof(octet));
}

ImagePPM::~ImagePPM() {
    free(colors);
}

/* Opérateur d'accès */
octet *ImagePPM::operator[](size_t i) {
    if (i < height)
    {
        return colors + (i * width * 3);
    }

    cerr << "Indices hors des bornes." << endl;
    exit(EXIT_FAILURE);
}

void ImagePPM::operator= (const ImagePPM &other) {
    width = other.width;
    height = other.height;

    colors = (octet *) calloc(size() * 3, sizeof(octet));
    for(size_t i = 0; i < size() * 3; i++){
        colors[i] = other.colors[i];
    }
}

/* Setter */
void ImagePPM::resize(size_t newWidth, size_t newHeight) {
    if ((width != newWidth) || (height != newHeight))
    {
        size_t newSize;
        octet *newcolors;

        newSize = newWidth * newHeight * 3;
        newcolors = (octet *)calloc(newSize, sizeof(octet));

        if (newcolors != NULL)
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

                    for(int color = 0; color < 3; color++){

                        octet topLeft = colors[(y1 * width + x1) * 3 + color];
                        octet topRight = colors[(y1 * width + x2) * 3 + color];
                        octet bottomLeft = colors[(y2 * width + x1) * 3 + color];
                        octet bottomRight = colors[(y2 * width + x2) * 3 + color];

                        float top = topLeft * (1 - xWeight) + topRight * xWeight;
                        float bottom = bottomLeft * (1 - xWeight) + bottomRight * xWeight;

                        newcolors[(y * newWidth + x) * 3 + color] = (top * (1 - yWeight) + bottom * yWeight);
                    }
                }
            }

            free(colors);
            Image::resize(newWidth, newHeight);
            colors = newcolors;
        }
    }
}

void ImagePPM::segment(size_t newBlockSize) {
    if ((newBlockSize >= width) || (newBlockSize >= height))
    {
        cerr << "Erreur - Taille des block supérieure aux dimensions de l'image " << endl;
        exit(EXIT_FAILURE);
    }

    size_t newWidth, newHeight;
    float average[3];
    size_t squarednewBlockSize = newBlockSize * newBlockSize;

    newWidth = newBlockSize * (width / newBlockSize);
    newHeight = newBlockSize * (height / newBlockSize);

    resize(newWidth, newHeight);

    for (size_t i = 0; i < height; i += newBlockSize)
    {
        for (size_t j = 0; j < width; j += newBlockSize)
        {
            average[0] = 0.0f;
            average[1] = 0.0f;
            average[2] = 0.0f;

            for (size_t x = 0; x < newBlockSize; x++)
            {
                for (size_t y = 0; y < newBlockSize; y++)
                {
                    average[0] += colors[((i + x) * width + (j + y)) * 3 + 0];
                    average[1] += colors[((i + x) * width + (j + y)) * 3 + 1];
                    average[2] += colors[((i + x) * width + (j + y)) * 3 + 2];
                }
            }

            average[0] /= squarednewBlockSize;
            average[1] /= squarednewBlockSize;
            average[2] /= squarednewBlockSize;

            for (size_t x = 0; x < newBlockSize; x++)
            {
                for (size_t y = 0; y < newBlockSize; y++)
                {
                    colors[((i + x) * width + (j + y)) * 3 + 0] = average[0];
                    colors[((i + x) * width + (j + y)) * 3 + 1] = average[1];
                    colors[((i + x) * width + (j + y)) * 3 + 2] = average[2];
                }
            }
        }
    }
}

size_t *ImagePPM::swap(size_t blockSize) {
    size_t widthFactor, heightFactor, nbBlock;
    size_t newWidth, newHeight;
    octet *newcolors;
    size_t *key;

    widthFactor = width / blockSize;
    heightFactor = height / blockSize;
    nbBlock = widthFactor * heightFactor;

    newWidth = blockSize * widthFactor;
    newHeight = blockSize * heightFactor;

    resize(newWidth, newHeight);

    key = (size_t *) calloc(nbBlock, sizeof(size_t));
    newcolors = (octet *) calloc(size() * 3, sizeof(octet));

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

                for(int color = 0; color < 3; color++){
                    newcolors[(index * 3) + color] = colors[(swappedIndex * 3) + color];
                }

            }
        }
    }

    free(colors);
    colors = newcolors;
    return key;
}

void ImagePPM::sort(size_t *key, size_t blockSize) {
    size_t widthFactor, heightFactor, nbBlock;
    octet *newcolors;

    widthFactor = width / blockSize;
    heightFactor = height / blockSize;
    nbBlock = widthFactor * heightFactor;

    newcolors = (octet *) calloc(size() * 3, sizeof(octet));

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

                for(int color = 0; color < 3; color++){
                    newcolors[(swappedIndex * 3) + color] = colors[(index * 3) + color];
                }
            }
        }
    }

    free(colors);
    colors = newcolors;
}

/* Lecture et écriture dans un fichier */
void ImagePPM::read(const char *path) {
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

    if (string(format) != "P6")
    {
        cerr << "Erreur - Format différent de P6" << endl;
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

    size = width * height * 3;

    /* Lecture des valeurs */
    colors = (octet *)calloc(size, sizeof(octet));

    if (fread(colors, sizeof(octet), size, file) != size)
    {
        cerr << "Erreur - Problème lors de la lecture des valeurs" << endl;
        free(colors);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    fclose(file);
}

void ImagePPM::write(const char *path, const char *comment) {
    FILE *file;
    size_t size;
    size = width * height * 3;

    /* Ouveture du fichier */
    file = fopen(path, "wb");
    if (file == NULL)
    {
        cerr << "Erreur - Pas d'accès en écriture sur l'image : " << path << endl;
        exit(EXIT_FAILURE);
    }

    /* Ecriture du format */
    fprintf(file, "P6\r");

    /* Ecriture des commentaires */
    if (comment)
    {
        fprintf(file, "# ");
        fprintf(file, "%s\n", comment);
    }

    /* Ecriture des dimensions */
    fprintf(file, "%ld %ld\r255\r", width, height);

    /* Ecriture des valeurs */
    if (fwrite(colors, sizeof(octet), size, file) != size)
    {
        cerr << "Erreur - Problème lors de l'écriture de l'image" << endl;
        fclose(file);
        exit(EXIT_FAILURE);
    }

    fclose(file);
}