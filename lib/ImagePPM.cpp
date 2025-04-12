#include "../include/ImagePPM.hpp"
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <cmath>
#include <immintrin.h> // AVX

using namespace std;

/* Constructeurs et destructeurs */
ImagePPM::ImagePPM() : Image(), pixels(nullptr) {}

ImagePPM::ImagePPM(size_t newWidth, size_t newHeight) : Image(newWidth, newHeight) {
    pixels = new octet[size() * 3];
}

ImagePPM::ImagePPM(const ImagePPM& other) : Image(other.width, other.height) {
    pixels = new octet[size() * 3];
    std::copy(other.pixels, other.pixels + size() * 3, pixels);
}

ImagePPM::~ImagePPM() {
    delete[] pixels;
}

/* Opérateur d'accès */
Color ImagePPM::getPixel(size_t i, size_t j) const {
    if (i >= height || j >= width) {
        cerr << "Indices hors des bornes." << endl;
        exit(EXIT_FAILURE);
    }
    size_t index = (i * width + j) * 3;
    return Color(pixels[index], pixels[index + 1], pixels[index + 2]);
}

void ImagePPM::setPixel(size_t i, size_t j, const Color& color) {
    if (i >= height || j >= width) {
        cerr << "Indices hors des bornes." << endl;
        exit(EXIT_FAILURE);
    }
    size_t index = (i * width + j) * 3;
    pixels[index] = static_cast<octet>(color[0]);
    pixels[index + 1] = static_cast<octet>(color[1]);
    pixels[index + 2] = static_cast<octet>(color[2]);
}

Color* ImagePPM::operator[](size_t i) {
    cerr << "Warning: operator[] for ImagePPM returns a pointer to a Color object, which might not be the most efficient way to access pixel data. Consider using getPixel and setPixel." << endl;
    if (i >= height) {
        cerr << "Indices hors des bornes." << endl;
        exit(EXIT_FAILURE);
    }
    Color* row = new Color[width];
    for (size_t j = 0; j < width; ++j) {
        row[j] = getPixel(i, j);
    }
    return row; // Memory leak here, the caller needs to delete[] the returned pointer.
}

void ImagePPM::operator= (const ImagePPM &other) {
    if (this == &other) {
        return;
    }
    width = other.width;
    height = other.height;

    delete[] pixels;
    pixels = new octet[size() * 3];

    std::copy(other.pixels, other.pixels + size() * 3, pixels);
}

ImagePGM ImagePPM::toPGM() {
    ImagePGM pgmImage(width, height);
    float r, g, b;
    for (size_t i = 0; i < height; i++) {
        for (size_t j = 0; j < width * 3; j += 3) {
            r = (float) pixels[i * (width * 3) + j];
            g = (float) pixels[i * (width * 3) + (j + 1)];
            b = (float) pixels[i * (width * 3) + (j + 2)];
            pgmImage[i][(j / 3)] = (octet)((r + g + b) / 3.0f);
        }
    }
    return pgmImage;
}

Color ImagePPM::average() const {
    __m256i sum_r_epi32 = _mm256_setzero_si256();
    __m256i sum_g_epi32 = _mm256_setzero_si256();
    __m256i sum_b_epi32 = _mm256_setzero_si256();
    size_t num_pixels = width * height;
    size_t i = 0;

    for (; i + 8 <= num_pixels; ++i) {
        __m256i pixels_i = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(pixels + i * 3));

        // Extract R, G, B bytes and convert to 32-bit integers
        __m256i r_bytes = _mm256_shuffle_epi8(pixels_i, _mm256_setr_epi8(0, -1, -1, -1, 3, -1, -1, -1, 6, -1, -1, -1, 9, -1, -1, -1, 12, -1, -1, -1, 15, -1, -1, -1, 18, -1, -1, -1, 21, -1, -1, -1));
        __m256i g_bytes = _mm256_shuffle_epi8(pixels_i, _mm256_setr_epi8(-1, 1, -1, -1, -1, 4, -1, -1, -1, 7, -1, -1, -1, 10, -1, -1, -1, 13, -1, -1, -1, 16, -1, -1, -1, 19, -1, -1, -1, 22, -1, -1));
        __m256i b_bytes = _mm256_shuffle_epi8(pixels_i, _mm256_setr_epi8(-1, -1, 2, -1, -1, -1, 5, -1, -1, -1, 8, -1, -1, -1, 11, -1, -1, -1, 14, -1, -1, -1, 17, -1, -1, -1, 20, -1, -1, -1, 23, -1));

        sum_r_epi32 = _mm256_add_epi32(sum_r_epi32, _mm256_cvtepu8_epi32(_mm256_extracti128_si256(r_bytes, 0)));
        sum_r_epi32 = _mm256_add_epi32(sum_r_epi32, _mm256_cvtepu8_epi32(_mm256_extracti128_si256(r_bytes, 1)));

        sum_g_epi32 = _mm256_add_epi32(sum_g_epi32, _mm256_cvtepu8_epi32(_mm256_extracti128_si256(g_bytes, 0)));
        sum_g_epi32 = _mm256_add_epi32(sum_g_epi32, _mm256_cvtepu8_epi32(_mm256_extracti128_si256(g_bytes, 1)));

        sum_b_epi32 = _mm256_add_epi32(sum_b_epi32, _mm256_cvtepu8_epi32(_mm256_extracti128_si256(b_bytes, 0)));
        sum_b_epi32 = _mm256_add_epi32(sum_b_epi32, _mm256_cvtepu8_epi32(_mm256_extracti128_si256(b_bytes, 1)));
    }

    uint32_t sum_r_arr[8], sum_g_arr[8], sum_b_arr[8];
    _mm256_storeu_si256(reinterpret_cast<__m256i*>(sum_r_arr), sum_r_epi32);
    _mm256_storeu_si256(reinterpret_cast<__m256i*>(sum_g_arr), sum_g_epi32);
    _mm256_storeu_si256(reinterpret_cast<__m256i*>(sum_b_arr), sum_b_epi32);

    float total_r = 0, total_g = 0, total_b = 0;
    for (int j = 0; j < 8; ++j) {
        total_r += sum_r_arr[j];
        total_g += sum_g_arr[j];
        total_b += sum_b_arr[j];
    }

    for (; i < num_pixels; ++i) {
        total_r += pixels[i * 3 + 0];
        total_g += pixels[i * 3 + 1];
        total_b += pixels[i * 3 + 2];
    }

    if (num_pixels > 0) {
        return Color(total_r / num_pixels, total_g / num_pixels, total_b / num_pixels);
    } else {
        return Color(0.0f, 0.0f, 0.0f);
    }
}

float ImagePPM::PSNR(const ImagePPM& other) const {
    double eqm = 0.0;
    size_t num_pixels = size() * 3;
    for (size_t i = 0; i < num_pixels; ++i) {
        eqm += pow(pixels[i] - other.pixels[i], 2);
    }
    eqm /= num_pixels;
    if(eqm == 0) {
        return INFINITY;
    }
    double psnr = 10 * log10(pow(255.0,2)/eqm);
    return psnr;
}

void ImagePPM::resize(size_t newWidth, size_t newHeight) {
    if (((width != newWidth) || (height != newHeight)) && (width != 0 && height != 0) && (newWidth != 0 && newHeight != 0)) {
        size_t newSize = newWidth * newHeight;
        octet* newPixels = new octet[newSize * 3];
        float widthRatio = static_cast<float>(width - 1) / (newWidth - 1);
        float heightRatio = static_cast<float>(height - 1) / (newHeight - 1);

        for (size_t y = 0; y < newHeight; ++y) {
            for (size_t x = 0; x < newWidth; ++x) {
                float srcX = x * widthRatio;
                float srcY = y * heightRatio;

                int x1 = static_cast<int>(srcX);
                int y1 = static_cast<int>(srcY);
                int x2 = std::min(x1 + 1, static_cast<int>(width - 1));
                int y2 = std::min(y1 + 1, static_cast<int>(height - 1));

                float xWeight = srcX - x1;
                float yWeight = srcY - y1;

                Color topLeft = getPixel(y1, x1);
                Color topRight = getPixel(y1, x2);
                Color bottomLeft = getPixel(y2, x1);
                Color bottomRight = getPixel(y2, x2);

                Color top = topLeft * (1 - xWeight) + topRight * xWeight;
                Color bottom = bottomLeft * (1 - xWeight) + bottomRight * xWeight;

                Color finalColor = top * (1 - yWeight) + bottom * yWeight;

                size_t newIndex = (y * newWidth + x) * 3;
                newPixels[newIndex + 0] = static_cast<octet>(finalColor[0]);
                newPixels[newIndex + 1] = static_cast<octet>(finalColor[1]);
                newPixels[newIndex + 2] = static_cast<octet>(finalColor[2]);
            }
        }

        delete[] pixels;
        Image::resize(newWidth, newHeight);
        pixels = newPixels;
    }
}

void ImagePPM::segment(size_t newBlockSize) {
    if ((newBlockSize >= width) || (newBlockSize >= height)) {
        cerr << "Erreur - Taille des block supérieure aux dimensions de l'image " << endl;
        exit(EXIT_FAILURE);
    }

    size_t newWidth = newBlockSize * (width / newBlockSize);
    size_t newHeight = newBlockSize * (height / newBlockSize);

    resize(newWidth, newHeight);

    for (size_t i = 0; i < height; i += newBlockSize) {
        for (size_t j = 0; j < width; j += newBlockSize) {
            __m256i sum_r_epi32 = _mm256_setzero_si256();
            __m256i sum_g_epi32 = _mm256_setzero_si256();
            __m256i sum_b_epi32 = _mm256_setzero_si256();
            size_t pixel_count = 0;

            for (size_t x = 0; x < newBlockSize; ++x) {
                for (size_t y = 0; y < newBlockSize; y += 8) {
                    if (i + x < height && j + y + 7 < width) {
                        __m256i pixels_vec = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(pixels + ((i + x) * width + (j + y)) * 3));

                        __m256i r_bytes = _mm256_shuffle_epi8(pixels_vec, _mm256_setr_epi8(0, -1, -1, -1, 3, -1, -1, -1, 6, -1, -1, -1, 9, -1, -1, -1, 12, -1, -1, -1, 15, -1, -1, -1, 18, -1, -1, -1, 21, -1, -1, -1));
                        __m256i g_bytes = _mm256_shuffle_epi8(pixels_vec, _mm256_setr_epi8(-1, 1, -1, -1, -1, 4, -1, -1, -1, 7, -1, -1, -1, 10, -1, -1, -1, 13, -1, -1, -1, 16, -1, -1, -1, 19, -1, -1, -1, 22, -1, -1));
                        __m256i b_bytes = _mm256_shuffle_epi8(pixels_vec, _mm256_setr_epi8(-1, -1, 2, -1, -1, -1, 5, -1, -1, -1, 8, -1, -1, -1, 11, -1, -1, -1, 14, -1, -1, -1, 17, -1, -1, -1, 20, -1, -1, -1, 23, -1));

                        sum_r_epi32 = _mm256_add_epi32(sum_r_epi32, _mm256_cvtepu8_epi32(_mm256_extracti128_si256(r_bytes, 0)));
                        sum_r_epi32 = _mm256_add_epi32(sum_r_epi32, _mm256_cvtepu8_epi32(_mm256_extracti128_si256(r_bytes, 1)));

                        sum_g_epi32 = _mm256_add_epi32(sum_g_epi32, _mm256_cvtepu8_epi32(_mm256_extracti128_si256(g_bytes, 0)));
                        sum_g_epi32 = _mm256_add_epi32(sum_g_epi32, _mm256_cvtepu8_epi32(_mm256_extracti128_si256(g_bytes, 1)));

                        sum_b_epi32 = _mm256_add_epi32(sum_b_epi32, _mm256_cvtepu8_epi32(_mm256_extracti128_si256(b_bytes, 0)));
                        sum_b_epi32 = _mm256_add_epi32(sum_b_epi32, _mm256_cvtepu8_epi32(_mm256_extracti128_si256(b_bytes, 1)));
                        pixel_count += 8;
                    } else {
                        for (size_t yy = 0; yy < newBlockSize && j + yy < width; ++yy) {
                            Color pixel = getPixel(i + x, j + yy);
                            sum_r_epi32 = _mm256_add_epi32(sum_r_epi32, _mm256_set1_epi32(static_cast<uint8_t>(pixel[0])));
                            sum_g_epi32 = _mm256_add_epi32(sum_g_epi32, _mm256_set1_epi32(static_cast<uint8_t>(pixel[1])));
                            sum_b_epi32 = _mm256_add_epi32(sum_b_epi32, _mm256_set1_epi32(static_cast<uint8_t>(pixel[2])));
                            pixel_count++;
                        }
                        break;
                    }
                }
            }

            uint32_t sum_r_arr[8], sum_g_arr[8], sum_b_arr[8];
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(sum_r_arr), sum_r_epi32);
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(sum_g_arr), sum_g_epi32);
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(sum_b_arr), sum_b_epi32);

            float avgR = 0.0f, avgG = 0.0f, avgB = 0.0f;
            for (int k = 0; k < 8; ++k) {
                avgR += sum_r_arr[k];
                avgG += sum_g_arr[k];
                avgB += sum_b_arr[k];
            }

            if (pixel_count > 0) {
                avgR /= pixel_count;
                avgG /= pixel_count;
                avgB /= pixel_count;
                Color averageColor(avgR, avgG, avgB);

                for (size_t x = 0; x < newBlockSize; ++x) {
                    for (size_t y = 0; y < newBlockSize; ++y) {
                        setPixel(i + x, j + y, averageColor);
                    }
                }
            }
        }
    }
}

void ImagePPM::mosaic(size_t blockSize, const char* libPath, size_t libSize) {
    size_t widthFactor = width / blockSize;
    size_t heightFactor = height / blockSize;
    size_t nbBlock = widthFactor * heightFactor;

    size_t newWidth = blockSize * widthFactor;
    size_t newHeight = blockSize * heightFactor;

    resize(newWidth, newHeight);
    segment(blockSize); // Initial segmentation might benefit from SIMD as well

    octet* newPixels = new octet[size() * 3];

    size_t blockRow, blockCol;
    size_t percent = 0;
    ImagePPM sticker;

    for (size_t blockId = 0; blockId < nbBlock; ++blockId) {
        blockRow = blockId / widthFactor;
        blockCol = blockId % widthFactor;
        Color minDist(256.0f, 256.0f, 256.0f);
        string bestMatchPath;

        Color blockAverage(0.0f, 0.0f, 0.0f);
        for (size_t i = 0; i < blockSize; ++i) {
            for (size_t j = 0; j < blockSize; ++j) {
                blockAverage += getPixel(blockRow * blockSize + i, blockCol * blockSize + j);
            }
        }
        blockAverage /= (float)(blockSize * blockSize);

        for (size_t i = 0; i < libSize; ++i) {
            string currentName = string(libPath) + "/" + to_string(i) + ".ppm";
            sticker.read(currentName.c_str());
            sticker.resize(blockSize, blockSize);
            Color stickerAverage = sticker.average();
            Color distance(abs(stickerAverage[0] - blockAverage[0]),
                            abs(stickerAverage[1] - blockAverage[1]),
                            abs(stickerAverage[2] - blockAverage[2]));

            if (distance.squaredNorm() < minDist.squaredNorm()) {
                minDist = distance;
                bestMatchPath = currentName;
            }
        }

        if (!bestMatchPath.empty()) {
            sticker.read(bestMatchPath.c_str());
            sticker.resize(blockSize, blockSize);
            for (size_t i = 0; i < blockSize; ++i) {
                for (size_t j = 0; j < blockSize; ++j) {
                    Color pixel = sticker.getPixel(i, j);
                    size_t index = ((blockRow * blockSize + i) * width + (blockCol * blockSize + j)) * 3;
                    newPixels[index + 0] = static_cast<octet>(pixel[0]);
                    newPixels[index + 1] = static_cast<octet>(pixel[1]);
                    newPixels[index + 2] = static_cast<octet>(pixel[2]);
                }
            }
        }

        size_t currentPercent = static_cast<size_t>((static_cast<float>(blockId + 1) / nbBlock) * 100.0f);
        if (currentPercent != percent || blockId == 0) {
            percent = currentPercent;
            printPercent(percent);
        }
    }
    printPercent(100);

    delete[] pixels;
    pixels = newPixels;
}

size_t* ImagePPM::swap(size_t blockSize) {
    size_t widthFactor = width / blockSize;
    size_t heightFactor = height / blockSize;
    size_t nbBlock =widthFactor * heightFactor;

size_t newWidth = blockSize * widthFactor;
size_t newHeight = blockSize * heightFactor;

resize(newWidth, newHeight);

size_t* key = new size_t[nbBlock];
for (size_t i = 0; i < nbBlock; ++i) {
    key[i] = i;
}

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> distrib(0, nbBlock - 1);

for (size_t i = nbBlock - 1; i > 0; --i) {
    size_t j = distrib(gen);
    std::swap(key[i], key[j]);
}

octet* newPixels = new octet[size() * 3];

for (size_t blockId = 0; blockId < nbBlock; ++blockId) {
    size_t swappedBlockId = key[blockId];

    size_t blockRow = blockId / widthFactor;
    size_t blockCol = blockId % widthFactor;
    size_t swappedBlockRow = swappedBlockId / widthFactor;
    size_t swappedBlockCol = swappedBlockId % widthFactor;

    for (size_t i = 0; i < blockSize; ++i) {
        for (size_t j = 0; j < blockSize; j += 8) {
            if (i < blockSize && j + 7 < blockSize) {
                __m256i source_pixels = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(pixels + ((swappedBlockRow * blockSize + i) * width + (swappedBlockCol * blockSize + j)) * 3));
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(newPixels + ((blockRow * blockSize + i) * width + (blockCol * blockSize + j)) * 3), source_pixels);
            } else {
                for (size_t jj = 0; jj < blockSize; ++jj) {
                    size_t src_index = ((swappedBlockRow * blockSize + i) * width + (swappedBlockCol * blockSize + jj)) * 3;
                    size_t dest_index = ((blockRow * blockSize + i) * width + (blockCol * blockSize + jj)) * 3;
                    newPixels[dest_index + 0] = pixels[src_index + 0];
                    newPixels[dest_index + 1] = pixels[src_index + 1];
                    newPixels[dest_index + 2] = pixels[src_index + 2];
                }
                break; // Move to the next i
            }
        }
    }
}

delete[] pixels;
pixels = newPixels;
return key;

}

void ImagePPM::sort(size_t* key, size_t blockSize) {
size_t widthFactor = width / blockSize;
size_t heightFactor = height / blockSize;
size_t nbBlock = widthFactor * heightFactor;

octet* newPixels = new octet[size() * 3];

for (size_t blockId = 0; blockId < nbBlock; ++blockId) {
    size_t swappedBlockId = key[blockId];

    size_t blockRow = blockId / widthFactor;
    size_t blockCol = blockId % widthFactor;
    size_t swappedBlockRow = swappedBlockId / widthFactor;
    size_t swappedBlockCol = swappedBlockId % widthFactor;

    for (size_t i = 0; i < blockSize; ++i) {
        for (size_t j = 0; j < blockSize; j += 8) {
            if (i < blockSize && j + 7 < blockSize) {
                __m256i source_pixels = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(pixels + ((swappedBlockRow * blockSize + i) * width + (swappedBlockCol * blockSize + j)) * 3));
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(newPixels + ((blockRow * blockSize + i) * width + (blockCol * blockSize + j)) * 3), source_pixels);
            } else {
                for (size_t jj = 0; jj < blockSize; ++jj) {
                    size_t src_index = ((swappedBlockRow * blockSize + i) * width + (swappedBlockCol * blockSize + jj)) * 3;
                    size_t dest_index = ((blockRow * blockSize + i) * width + (blockCol * blockSize + jj)) * 3;
                    newPixels[dest_index + 0] = pixels[src_index + 0];
                    newPixels[dest_index + 1] = pixels[src_index + 1];
                    newPixels[dest_index + 2] = pixels[src_index + 2];
                }
                break; // Move to the next i
            }
        }
    }
}

delete[] pixels;
pixels = newPixels;

}

/* Lecture et écriture dans un fichier */
void ImagePPM::read(const char *path) {
FILE *file;
char format[3], c;
octet maxValue;

/* Ouverture du fichier */
file = fopen(path, "rb");
if (file == NULL) {
    cerr << "Erreur - Pas d'accès en lecture sur l'image : " << path << endl;
    exit(EXIT_FAILURE);
}

/* Lecture du format */
if (fscanf(file, "%2s", format) != 1) {
    cerr << "Erreur - Lecture du format impossible" << endl;
    fclose(file);
    exit(EXIT_FAILURE);
}

if (string(format) != "P6") {
    cerr << "Erreur - Format différent de P6" << endl;
    fclose(file);
    exit(EXIT_FAILURE);
}

/* Ignorer les commentaires */
fgetc(file); // Ignorer le \n
while ((c = fgetc(file)) == '#') {
    while ((c = fgetc(file)) != '\n') {
        // Lire jusqu'à la fin de la ligne de commentaire
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

if (maxValue != 255) {
    cerr << "Erreur - Valeur maximale éronée" << endl;
    fclose(file);
    exit(EXIT_FAILURE);}

delete[] pixels;
pixels = new octet[size() * 3];

/* Lecture des valeurs des couleurs */
if (fread(pixels, sizeof(octet), size() * 3, file) != size() * 3) {
    cerr << "Erreur - Problème lors de la lecture des valeurs des couleurs" << endl;
    delete[] pixels;
    fclose(file);
    exit(EXIT_FAILURE);
}

fclose(file);

}

void ImagePPM::write(const char *path, const char *comment) {
FILE *file;

/* Ouverture du fichier */
file = fopen(path, "wb");
if (file == NULL) {
    cerr << "Erreur - Pas d'accès en écriture sur l'image : " << path << endl;
    exit(EXIT_FAILURE);
}

/* Ecriture du format */
fprintf(file, "P6\n");

/* Ecriture des commentaires */
if (comment) {
    fprintf(file, "# %s\n", comment);
}

/* Ecriture des dimensions */
fprintf(file, "%ld %ld\n255\n", width, height);

/* Ecriture des valeurs des couleurs */
if (pixels != nullptr) {
    if (fwrite(pixels, sizeof(octet), size() * 3, file) != size() * 3) {
        cerr << "Erreur - Problème lors de l'écriture des valeurs des couleurs" << endl;
    }
}

fclose(file);

}