#include "../include/ImagePGM.hpp"
#include <stdlib.h>
#include <iostream>
#include <string>
#include <immintrin.h> // AVX/SSE
#include <cstring>
#include <cmath>

using namespace std;

/* Constructeur et destructeurs */
ImagePGM::ImagePGM() : Image(), grey(nullptr) {}

ImagePGM::ImagePGM(size_t newWidth, size_t newHeight) : Image(newWidth, newHeight) {
    grey = (octet *)calloc(width * height, sizeof(octet));
}

ImagePGM::ImagePGM(size_t newWidth, size_t newHeight, octet* newGrey) : Image(newWidth, newHeight) {
    grey = (octet *)calloc(width * height, sizeof(octet));
    std::copy(newGrey, newGrey + size() * 3, grey);
}

ImagePGM::~ImagePGM() {
    free(grey);
}

/* Opérateur d'accès */
octet* ImagePGM::operator[](size_t i) {
    if (i < height) {
        return grey + (i * width);
    }

    cerr << "Indices hors des bornes." << endl;
    exit(EXIT_FAILURE);
}

octet ImagePGM::average() {
    if (size() == 0 || grey == nullptr) {
        return 0;
    }

    float sum = 0.0f;
    size_t i = 0;

#ifdef __AVX2__
    size_t avx_end = (size() / 32) * 32;
    __m256i sum_vec = _mm256_setzero_si256();
    while (i < avx_end) {
        __m256i pixels = _mm256_loadu_si256((__m256i *)&grey[i]);

        __m256i sum_lo = _mm256_add_epi32(_mm256_unpacklo_epi16(_mm256_cvtepu8_epi16(_mm256_extracti128_si256(pixels, 0)), _mm256_setzero_si256()),
                                          _mm256_unpackhi_epi16(_mm256_cvtepu8_epi16(_mm256_extracti128_si256(pixels, 0)), _mm256_setzero_si256()));
        __m256i sum_hi = _mm256_add_epi32(_mm256_unpacklo_epi16(_mm256_cvtepu8_epi16(_mm256_extracti128_si256(pixels, 1)), _mm256_setzero_si256()),
                                          _mm256_unpackhi_epi16(_mm256_cvtepu8_epi16(_mm256_extracti128_si256(pixels, 1)), _mm256_setzero_si256()));

        sum_vec = _mm256_add_epi32(sum_vec, sum_lo);
        sum_vec = _mm256_add_epi32(sum_vec, sum_hi);
        i += 32;
    }
    int32_t temp_sum[8];
    _mm256_storeu_si256((__m256i *)temp_sum, sum_vec);
    for (int j = 0; j < 8; ++j) {
        sum += temp_sum[j];
    }
#endif

    for (; i < size(); ++i) {
        sum += grey[i];
    }

    return (octet)(sum / (float)size());
}

float* ImagePGM::histo() const {
    float* _histo = (float*) calloc(256, sizeof(float));
    if (!_histo) {
        throw std::bad_alloc();
    }

    size_t num_pixels = size();
    const unsigned char* pixel_data = grey;

    // Use an integer array for initial counting
    unsigned int _histo_int[256] = {0};

    // Process pixels in chunks of 32 (8 x __m256i) for AVX2
    size_t i = 0;
    size_t vector_size = sizeof(__m256i); // 32 bytes, holds 32 unsigned chars
    size_t num_vector_iterations = num_pixels / vector_size;

    for (; i < num_vector_iterations * vector_size; i += vector_size) {
        // Create a vector of indices (0 to 31)
        __m256i indices = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);
        indices = _mm256_mullo_epi32(indices, _mm256_set1_epi32(4)); // Scale by 4 for byte offsets

        // Gather the counts into the integer histogram
        for (size_t j = 0; j < vector_size; ++j) {
            _histo_int[pixel_data[i + j]]++;
        }
    }

    // Process remaining pixels
    for (; i < num_pixels; ++i) {
        _histo_int[pixel_data[i]]++;
    }

    // Normalize the histogram to floats
    float inv_size = 1.0f / (float)num_pixels;
    for (size_t j = 0; j < 256; ++j) {
        _histo[j] = (float)_histo_int[j] * inv_size;
    }

    return _histo;
}


/* Comparaison */

float ImagePGM::bhattacharyyaDist(const ImagePGM& other) const {
    float dist = 0.0f;
    float *histo1, *histo2;

    histo1 = histo();
    histo2 = other.histo();

    __m256 sum_vec = _mm256_setzero_ps();
    size_t vector_length = sizeof(__m256) / sizeof(float); // 8 floats

    for (size_t i = 0; i < 256 - vector_length + 1; i += vector_length) {
        __m256 h1_vec = _mm256_loadu_ps(histo1 + i);
        __m256 h2_vec = _mm256_loadu_ps(histo2 + i);

        __m256 mult_vec = _mm256_mul_ps(h1_vec, h2_vec);
        __m256 sqrt_vec = _mm256_sqrt_ps(mult_vec);
        sum_vec = _mm256_add_ps(sum_vec, sqrt_vec);
    }

    // Horizontal sum of the vector
    float sum_arr[8];
    _mm256_storeu_ps(sum_arr, sum_vec);
    for (size_t j = 0; j < vector_length; ++j) {
        dist += sum_arr[j];
    }

    dist = -log(dist);

    free(histo1);
    free(histo2);

    return dist;
}

void ImagePGM::operator= (const ImagePGM &other) {
    if (this == &other) {
        return;
    }
    Image::operator=(other); // Copy width and height

    if (grey != nullptr) {
        free(grey);
    }

    grey = (octet *) calloc(size(), sizeof(octet));
    if (grey == nullptr) {
        cerr << "Erreur d'allocation mémoire dans l'opérateur d'assignation." << endl;
        exit(EXIT_FAILURE);
    }

    size_t num_bytes = size() * sizeof(octet);
    memcpy(grey, other.grey, num_bytes);
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

            // Fill the block with the average value using SIMD
            octet avg_octet = static_cast<octet>(average);
#ifdef __AVX2__
            __m256i avg_vec = _mm256_set1_epi8(avg_octet);
            for (size_t x = 0; x < newBlockSize; x++) {
                size_t row_start = ((i + x) * width) + j;
                size_t k = 0;
                for (; k + 32 <= newBlockSize; k += 32) {
                    _mm256_storeu_si256((__m256i *)&grey[row_start + k], avg_vec);
                }
                for (; k < newBlockSize; ++k) {
                    grey[row_start + k] = avg_octet;
                }
            }
#elif defined(__SSE2__)
            __m128i avg_vec = _mm_set1_epi8(avg_octet);
            for (size_t x = 0; x < newBlockSize; x++) {
                size_t row_start = ((i + x) * width) + j;
                size_t k = 0;
                for (; k + 16 <= newBlockSize; k += 16) {
                    _mm_storeu_si128((__m128i *)&grey[row_start + k], avg_vec);
                }
                for (; k < newBlockSize; ++k) {
                    grey[row_start + k] = avg_octet;
                }
            }
#else
            for (size_t x = 0; x < newBlockSize; x++)
            {
                for (size_t y = 0; y < newBlockSize; y++)
                {
                    grey[((i + x) * width) + (j + y)] = average;
                }
            }
#endif
        }
    }
}

void ImagePGM::mosaic(size_t blockSize, const char* libPath, size_t libSize) {
    size_t widthFactor, heightFactor, nbBlock;
    size_t newWidth, newHeight;
    octet* newGrey;
    float minDist;
    std::string currentName, name;
    size_t currentPercent, percent;
    ImagePGM sticker;

    widthFactor = width / blockSize;
    heightFactor = height / blockSize;
    nbBlock = widthFactor * heightFactor;

    newWidth = blockSize * widthFactor;
    newHeight = blockSize * heightFactor;

    resize(newWidth, newHeight);
    segment(blockSize); // It might be beneficial to SIMD-optimize this further if blockSize is large

    newGrey = (octet *) calloc(size(), sizeof(octet));
    if (newGrey == nullptr) {
        std::cerr << "Erreur d'allocation mémoire dans mosaic_bhattacharyya." << std::endl;
        exit(EXIT_FAILURE);
    }

    size_t blockRow, blockCol;
    percent = 0;
    for (size_t blockId = 0; blockId < nbBlock; blockId++) {
        blockRow = blockId / widthFactor;
        blockCol = blockId % widthFactor;
        minDist = std::numeric_limits<float>::max();
        ImagePGM currentBlock(blockSize, blockSize);

        // Extract the current block
        for (size_t i = 0; i < blockSize; ++i) {
            for (size_t j = 0; j < blockSize; ++j) {
                currentBlock[i][j] = grey[(blockRow * blockSize + i) * width + (blockCol * blockSize + j)];
            }
        }

        for(size_t i = 0; i < libSize; i++) {
            currentName = (std::string(libPath) + "/" + std::to_string(i) + ".pgm");
            sticker.read(currentName.data());
            if (sticker.getWidth() != blockSize || sticker.getHeight() != blockSize) {
                sticker.resize(blockSize, blockSize);
            }

            float currentDist = currentBlock.bhattacharyyaDist(sticker);

            if (minDist > currentDist) {
                minDist = currentDist;
                name = currentName;
            }
        }
        sticker.read(name.data());
        if (sticker.getWidth() != blockSize || sticker.getHeight() != blockSize) {
            sticker.resize(blockSize, blockSize);
        }

        // Copy the best matching sticker's pixels to the new image using SIMD
        for (size_t i = 0; i < blockSize; i++) {
            size_t dest_row_start = (blockRow * blockSize + i) * width + (blockCol * blockSize);
            const octet* src_row = sticker[i];
#ifdef __AVX2__
            size_t j = 0;
            for (; j + 32 <= blockSize; j += 32) {
                _mm256_storeu_si256((__m256i *)&newGrey[dest_row_start + j], _mm256_loadu_si256((const __m256i *)&src_row[j]));
            }
            for (; j < blockSize; ++j) {
                newGrey[dest_row_start + j] = src_row[j];
            }
#elif defined(__SSE2__)
            size_t j = 0;
            for (; j + 16 <= blockSize; j += 16) {
                _mm_storeu_si128((__m128i *)&newGrey[dest_row_start + j], _mm_loadu_si128((const __m128i *)&src_row[j]));
            }
            for (; j < blockSize; ++j) {
                newGrey[dest_row_start + j] = src_row[j];
            }
#else
            for (size_t j = 0; j < blockSize; j++) {
                newGrey[dest_row_start + j] = sticker[i][j];
            }
#endif
        }

        currentPercent = (((float) blockId / (float) nbBlock) * 100.0);
        if(currentPercent != percent || blockId == 0) {
            percent = currentPercent;
            printPercent(percent);
        }
    }
    printPercent(100);
    std::cout << std::endl;

    free(grey);
    grey = newGrey;
}

/*void ImagePGM::mosaic(size_t blockSize, const char* libPath, size_t libSize) {
    size_t widthFactor, heightFactor, nbBlock;
    size_t newWidth, newHeight;
    octet* newGrey;
    octet minDist;
    string currentName, name;
    size_t currentPercent, percent;
    ImagePGM sticker;

    widthFactor = width / blockSize;
    heightFactor = height / blockSize;
    nbBlock = widthFactor * heightFactor;

    newWidth = blockSize * widthFactor;
    newHeight = blockSize * heightFactor;

    resize(newWidth, newHeight);
    segment(blockSize); // It might be beneficial to SIMD-optimize this further if blockSize is large

    newGrey = (octet *) calloc(size(), sizeof(octet));
    if (newGrey == nullptr) {
        cerr << "Erreur d'allocation mémoire dans mosaic." << endl;
        exit(EXIT_FAILURE);
    }

    size_t blockRow, blockCol;
    percent = 0;
    for (size_t blockId = 0; blockId < nbBlock; blockId++) {
        blockRow = blockId / widthFactor;
        blockCol = blockId % widthFactor;
        minDist = 255;
        octet currentAvg = grey[(blockRow * blockSize) * width + (blockCol * blockSize)]; // Average of the current block

        for(size_t i = 0; i < libSize; i++) {
            currentName = (string(libPath) + "/" + to_string(i) + ".pgm");
            sticker.read(currentName.data());
            if (sticker.getWidth() != blockSize || sticker.getHeight() != blockSize) {
                sticker.resize(blockSize, blockSize);
            }

            octet stickerAvg = sticker.average();
            octet currentDist = abs((int)stickerAvg - (int)currentAvg);

            if (minDist > currentDist) {
                minDist = currentDist;
                name = currentName;
            }
        }
        sticker.read(name.data());
        if (sticker.getWidth() != blockSize || sticker.getHeight() != blockSize) {
            sticker.resize(blockSize, blockSize);
        }

        // Copy the best matching sticker's pixels to the new image using SIMD
        for (size_t i = 0; i < blockSize; i++) {
            size_t dest_row_start = (blockRow * blockSize + i) * width + (blockCol * blockSize);
            const octet* src_row = sticker[i];
#ifdef __AVX2__
            size_t j = 0;
            for (; j + 32 <= blockSize; j += 32) {
                _mm256_storeu_si256((__m256i *)&newGrey[dest_row_start + j], _mm256_loadu_si256((const __m256i *)&src_row[j]));
            }
            for (; j < blockSize; ++j) {
                newGrey[dest_row_start + j] = src_row[j];
            }
#elif defined(__SSE2__)
            size_t j = 0;
            for (; j + 16 <= blockSize; j += 16) {
                _mm_storeu_si128((__m128i *)&newGrey[dest_row_start + j], _mm_loadu_si128((const __m128i *)&src_row[j]));
            }
            for (; j < blockSize; ++j) {
                newGrey[dest_row_start + j] = src_row[j];
            }
#else
            for (size_t j = 0; j < blockSize; j++) {
                newGrey[dest_row_start + j] = sticker[i][j];
            }
#endif
        }

        currentPercent = (((float) blockId / (float) nbBlock) * 100.0);
        if(currentPercent != percent || blockId == 0) {
            percent = currentPercent;
            printPercent(percent);
        }
    }
    printPercent(100);
    cout << endl;

    free(grey);
    grey = newGrey;
}*/

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
    if (newGrey == nullptr || key == nullptr) {
        cerr << "Erreur d'allocation mémoire dans swap." << endl;exit(EXIT_FAILURE);
    }

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
    size_t swappedBlockId;

    for (size_t blockId = 0; blockId < nbBlock; blockId++) {
        swappedBlockId = key[blockId];

        blockRow = blockId / widthFactor;
        blockCol = blockId % widthFactor;
        swappedBlockRow = swappedBlockId / widthFactor;
        swappedBlockCol = swappedBlockId % widthFactor;

        size_t src_block_start = (swappedBlockRow * blockSize) * width + (swappedBlockCol * blockSize);
        size_t dest_block_start = (blockRow * blockSize) * width + (blockCol * blockSize);

        for (size_t i = 0; i < blockSize; ++i) {
            size_t src_row_start = src_block_start + i * width;
            size_t dest_row_start = dest_block_start + i * width;
#ifdef __AVX2__
            size_t j = 0;
            for (; j + 32 <= blockSize; j += 32) {
                _mm256_storeu_si256((__m256i *)&newGrey[dest_row_start + j], _mm256_loadu_si256((const __m256i *)&grey[src_row_start + j]));
            }
            for (; j < blockSize; ++j) {
                newGrey[dest_row_start + j] = grey[src_row_start + j];
            }
#elif defined(__SSE2__)
            size_t j = 0;
            for (; j + 16 <= blockSize; j += 16) {
                _mm_storeu_si128((__m128i *)&newGrey[dest_row_start + j], _mm_loadu_si128((const __m128i *)&grey[src_row_start + j]));
            }
            for (; j < blockSize; ++j) {
                newGrey[dest_row_start + j] = grey[src_row_start + j];
            }
#else
            for (size_t j = 0; j < blockSize; ++j) {
                newGrey[dest_row_start + j] = grey[src_row_start + j];
            }
#endif
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
    if (newGrey == nullptr) {
        cerr << "Erreur d'allocation mémoire dans sort." << endl;
        exit(EXIT_FAILURE);
    }

    size_t blockRow, blockCol, swappedBlockRow, swappedBlockCol;
    size_t swappedBlockId;

    for (size_t blockId = 0; blockId < nbBlock; blockId++) {
        swappedBlockId = key[blockId];

        blockRow = blockId / widthFactor;
        blockCol = blockId % widthFactor;
        swappedBlockRow = swappedBlockId / widthFactor;
        swappedBlockCol = swappedBlockId % widthFactor;

        size_t src_block_start = (swappedBlockRow * blockSize) * width + (swappedBlockCol * blockSize);
        size_t dest_block_start = (blockRow * blockSize) * width + (blockCol * blockSize);

        for (size_t i = 0; i < blockSize; ++i) {
            size_t src_row_start = src_block_start + i * width;
            size_t dest_row_start = dest_block_start + i * width;
#ifdef __AVX2__
            size_t j = 0;
            for (; j + 32 <= blockSize; j += 32) {
                _mm256_storeu_si256((__m256i *)&newGrey[dest_row_start + j], _mm256_loadu_si256((const __m256i *)&grey[src_row_start + j]));
            }
            for (; j < blockSize; ++j) {
                newGrey[dest_row_start + j] = grey[src_row_start + j];
            }
#elif defined(__SSE2__)
            size_t j = 0;
            for (; j + 16 <= blockSize; j += 16) {
                _mm_storeu_si128((__m128i *)&newGrey[dest_row_start + j], _mm_loadu_si128((const __m128i *)&grey[src_row_start + j]));
            }
            for (; j < blockSize; ++j) {
                newGrey[dest_row_start + j] = grey[src_row_start + j];
            }
#else
            for (size_t j = 0; j < blockSize; ++j) {
                newGrey[dest_row_start + j] = grey[src_row_start + j];
            }
#endif
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
        cerr << "Erreur - Valeur maximal éronée" << endl;
        fclose(file);
        exit(EXIT_FAILURE);
    }

    size = width * height;

    free(grey);

    /* Lecture des valeurs */
    grey = (octet *)calloc(width * height, sizeof(octet));
    if (grey == nullptr) {
        cerr << "Erreur d'allocation mémoire lors de la lecture de l'image." << endl;
        fclose(file);
        exit(EXIT_FAILURE);
    }

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

    /* Ecriture des valeurs */
    if (fwrite(grey, sizeof(octet), size, file) != size)
    {
        cerr << "Erreur - Problème lors de l'écriture de l'image" << endl;
        fclose(file);
        exit(EXIT_FAILURE);
    }

    fclose(file);
}