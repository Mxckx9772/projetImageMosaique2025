#include "../include/ImagePPM.hpp"
#include <stdlib.h>
#include <iostream>
#include <string>

using namespace std;

/* Constructeurs et destructeurs */
ImagePPM::ImagePPM() : Image(), colors(new Color[0]) {}

ImagePPM::ImagePPM(size_t newWidth, size_t newHeight) : Image(newWidth, newHeight) {
    colors = new Color[size()];
}

ImagePPM::~ImagePPM() {
    delete[] colors;
}

/* Opérateur d'accès */
Color *ImagePPM::operator[](size_t i) {
    if (i >= height) {
        cerr << "Indices hors des bornes." << endl;
        exit(EXIT_FAILURE);
    }
    return colors + (i * width);
}

void ImagePPM::operator= (const ImagePPM &other) {
    width = other.width;
    height = other.height;

    delete [] colors;
    colors = new Color[size()];
    for(size_t i = 0; i < size(); i++){
        colors[i] = other.colors[i];
    }
}

ImagePGM ImagePPM::toPGM() {
    ImagePGM pgmImage(width, height);
    for (size_t i = 0; i < height; ++i) {
        for (size_t j = 0; j < width; ++j) {
            pgmImage[i][j] = (octet)((colors[i * width + j][0] + colors[i * width + j][1] + colors[i * width + j][2]) / 3.0f);
        }
    }
    return pgmImage;
}

Color ImagePPM::average() {
    Color avgColor(0.0f, 0.0f, 0.0f);

    for (size_t i = 0; i < size(); ++i) {
        avgColor += colors[i];
    }

    avgColor /= (float) size();
    return avgColor;
}

/* Setter */
void ImagePPM::resize(size_t newWidth, size_t newHeight) {
    if (((width != newWidth) || (height != newHeight)) && (width != 0 && height != 0) && (newWidth != 0 && newHeight != 0)) {
        size_t newSize;
        Color* newColors;
        float widthRatio, heightRatio;

        newSize = newWidth * newHeight;
        newColors = new Color[newSize];

        if (newColors != nullptr) {
            widthRatio = (float)(width - 1) / (float)(newWidth - 1);
            heightRatio = (float)(height - 1) / (float)(newHeight - 1);

            for (size_t y = 0; y < newHeight; ++y) {
                for (size_t x = 0; x < newWidth; ++x) {
                    float srcX = x * widthRatio;
                    float srcY = y * heightRatio;

                    int x1 = srcX;
                    int y1 = srcY;
                    int x2 = min(x1 + 1, (int)(width - 1));
                    int y2 = min(y1 + 1, (int)(height - 1));

                    float xWeight = srcX - x1;
                    float yWeight = srcY - y1;

                    Color topLeft = colors[y1 * width + x1];
                    Color topRight = colors[y1 * width + x2];
                    Color bottomLeft = colors[y2 * width + x1];
                    Color bottomRight = colors[y2 * width + x2];

                    Color top = topLeft * (1 - xWeight) + topRight * xWeight;
                    Color bottom = bottomLeft * (1 - xWeight) + bottomRight * xWeight; // j'usque ici

                    newColors[y * newWidth + x] = top * (1 - yWeight) + bottom * yWeight;
                }
            }

            
            delete[] colors;
            
            Image::resize(newWidth, newHeight);
            colors = newColors;
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
    Color averageColor;
    size_t squarednewBlockSize = newBlockSize * newBlockSize;

    newWidth = newBlockSize * (width / newBlockSize);
    newHeight = newBlockSize * (height / newBlockSize);

    resize(newWidth, newHeight);

    for (size_t i = 0; i < height; i += newBlockSize) {
        for (size_t j = 0; j < width; j += newBlockSize) {
            averageColor = Color(0.0f, 0.0f, 0.0f);
            for (size_t x = 0; x < newBlockSize; x++) {
                for (size_t y = 0; y < newBlockSize; y++) {
                    averageColor += colors[((i + x) * width) + (j + y)];
                }
            }
            averageColor /= (float) squarednewBlockSize;

            for (size_t x = 0; x < newBlockSize; x++) {
                for (size_t y = 0; y < newBlockSize; y++) {
                    colors[((i + x) * width) + (j + y)] = averageColor;
                }
            }
        }
    }
}

void ImagePPM::mosaic(size_t blockSize, const char* libPath, size_t libSize) {
    size_t widthFactor, heightFactor, nbBlock;
    size_t newWidth, newHeight;
    Color *newColors, currentDist, minDist;
    string currentName, name;
    size_t currentPercent, percent;
    ImagePPM sticker;

    widthFactor = width / blockSize;
    heightFactor = height / blockSize;
    nbBlock = widthFactor * heightFactor;

    newWidth = blockSize * widthFactor;
    newHeight = blockSize * heightFactor;

    resize(newWidth, newHeight);
    segment(blockSize);

    newColors = new Color[size()];

    size_t blockRow, blockCol;
    size_t index;
    percent = 0;
    for (size_t blockId = 0; blockId < nbBlock; blockId++) {

        blockRow = blockId / widthFactor;
        blockCol = blockId % widthFactor;
        minDist = Color(255.0f, 255.0f, 255.0f); // Initialize with max possible values

        for(size_t i = 0; i < libSize; i++) {
            currentName = (string(libPath) + "/" + to_string(i) + ".ppm");
            sticker.read(currentName.data());
            sticker.resize(blockSize, blockSize);

            index = (blockRow * blockSize) * width + (blockCol * blockSize);
            Color blockAverage = Color(0.0f, 0.0f, 0.0f);
            for(size_t y = 0; y < blockSize; ++y) {
                for(size_t x = 0; x < blockSize; ++x) {
                    blockAverage += colors[index + y * width + x];
                }
            }
            blockAverage /= (float)(blockSize * blockSize);

            Color stickerAverage = sticker.average();
            Color distance = Color(abs(stickerAverage[0] - blockAverage[0]),
                                   abs(stickerAverage[1] - blockAverage[1]),
                                   abs(stickerAverage[2] - blockAverage[2]));

            if (distance.squaredNorm() < minDist.squaredNorm()) {
                minDist = distance;
                name = currentName;
            }
        }
        sticker.read(name.data());
        sticker.resize(blockSize, blockSize);

        for (size_t i = 0; i < blockSize; i++) {
            for (size_t j = 0; j < blockSize; j++) {
                index = (blockRow * blockSize + i) * width + (blockCol * blockSize + j);
                newColors[index] = sticker[i][j];
            }
        }

        currentPercent = (((float) blockId / (float) nbBlock) * 100.0);

        if(currentPercent != percent || blockId == 0) {
            percent = currentPercent;
            printPercent(percent);
        }

    }
    printPercent(100);

    delete[] colors;
    colors = newColors;
}

size_t *ImagePPM::swap(size_t blockSize) {
    size_t widthFactor, heightFactor, nbBlock;
    size_t newWidth, newHeight;
    Color *newColors;
    size_t *key;

    widthFactor = width / blockSize;
    heightFactor = height / blockSize;
    nbBlock = widthFactor * heightFactor;

    newWidth = blockSize * widthFactor;
    newHeight = blockSize * heightFactor;

    resize(newWidth, newHeight);

    key = (size_t *) calloc(nbBlock, sizeof(size_t));
    newColors = new Color[size()];

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

                newColors[index] = colors[swappedIndex];
            }
        }
    }

    delete[] colors;
    colors = newColors;
    return key;
}

void ImagePPM::sort(size_t *key, size_t blockSize) {
    size_t widthFactor, heightFactor, nbBlock;
    Color *newColors;

    widthFactor = width / blockSize;
    heightFactor = height / blockSize;
    nbBlock = widthFactor * heightFactor;

    newColors = new Color[size()];

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

                newColors[swappedIndex] = colors[index];
            }
        }
    }

    delete[] colors;
    colors = newColors;
}

/* Lecture et écriture dans un fichier */
void ImagePPM::read(const char *path) {
    FILE *file;
    char format[3], c;
    octet maxValue, color[3];

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
    while ((c = fgetc(file)) == '#'){
        while ((c = fgetc(file)) != '\n'){

        }
    }

    ungetc(c, file); // Rejeter le caractère en trop

    /* Lecture des dimensions */
    if (fscanf(file, "%ld %ld %hhd", &width, &height, &maxValue) != 3) {
        cerr << "Erreur - Lecture des dimensions impossible" << endl;
        fclose(file);
        exit(EXIT_FAILURE);
    }
    fgetc(file); // Ignorer le saut de ligne

    if (maxValue != 255)
    {
        cerr << "Erreur - Valeur maximale éronée" << endl;
        fclose(file);
        exit(EXIT_FAILURE);
    }

    delete[] colors;
    colors = new Color[size()];

    /* Lecture des valeurs des couleurs */
    for (size_t i = 0; i < size(); i++) {
        
        if (fread(color, sizeof(octet), 3, file) != 3) {
            cerr << "Erreur - Problème lors de la lecture des valeurs des couleurs" << endl;
            delete[] colors;
            fclose(file);
            exit(EXIT_FAILURE);
        }

        colors[i][0] = color[0];
        colors[i][1] = color[1];
        colors[i][2] = color[2];                                                                                             
    }

    fclose(file);
}

void ImagePPM::write(const char *path, const char *comment) {
    FILE *file;
    octet color[3];

    /* Ouverture du fichier */
    file = fopen(path, "wb");
    if (file == NULL)
    {
        cerr << "Erreur - Pas d'accès en écriture sur l'image : " << path << endl;
        exit(EXIT_FAILURE);
    }

    /* Ecriture du format */
    fprintf(file, "P6\n");

    /* Ecriture des commentaires */
    if (comment)
    {
        fprintf(file, "# ");
        fprintf(file, "%s\n", comment);
    }

    /* Ecriture des dimensions */
    fprintf(file, "%ld %ld\n255\n", width, height);

    /* Ecriture des valeurs des couleurs */
    for (size_t i = 0; i < size(); ++i) {
        for (size_t j = 0; j < 3; j++){
            color[j] = (octet) colors[i][j];
        }

        fwrite(color, sizeof(octet), 3, file);
    }
    

    fclose(file);
}