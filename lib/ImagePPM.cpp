#include "../include/ImagePPM.hpp"
#include <algorithm>
#include <math.h>
#include <string>
#include <random>
#include <iostream>
#include <fstream>
#include <thread>
#include <atomic>
#include <immintrin.h>
//#include <functional>

using namespace std;


/* Constructeurs et desctructeurs */

ImagePPM::ImagePPM(): Image(), _data(nullptr) {}

ImagePPM::ImagePPM(size_t width, size_t height): Image(width, height), _data(new octet[size() * 3]) {}

ImagePPM::ImagePPM(size_t width, size_t height, octet* data): Image(width, height), _data(new octet[size() * 3]) {
    copy(data, data + size() * 3, _data);
}

ImagePPM::ImagePPM(const ImagePPM& other): Image(other._width, other._height), _data(new octet[other.size() * 3]) {
    copy(other._data, other._data + size() * 3, _data);
}

ImagePPM::~ImagePPM() {
    delete[] _data;
}


/* Getters */

Color ImagePPM::average() const {
    /*Color _average;

    for (size_t i = 0; i < size() * 3; i += 3) {
        _average[0] += (float) _data[i];
        _average[1] += (float) _data[i];
        _average[2] += (float) _data[i];
    }

    _average /= (float) size();

    return _average;*/
    __m256i sum_r_epi32 = _mm256_setzero_si256();
    __m256i sum_g_epi32 = _mm256_setzero_si256();
    __m256i sum_b_epi32 = _mm256_setzero_si256();
    size_t num_pixels = _width * _height;
    size_t i = 0;

    for (; i + 8 <= num_pixels; ++i) {
        __m256i colors_i = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(_data + i * 3));

        // Extract R, G, B bytes and convert to 32-bit integers
        __m256i r_bytes = _mm256_shuffle_epi8(colors_i, _mm256_setr_epi8(0, -1, -1, -1, 3, -1, -1, -1, 6, -1, -1, -1, 9, -1, -1, -1, 12, -1, -1, -1, 15, -1, -1, -1, 18, -1, -1, -1, 21, -1, -1, -1));
        __m256i g_bytes = _mm256_shuffle_epi8(colors_i, _mm256_setr_epi8(-1, 1, -1, -1, -1, 4, -1, -1, -1, 7, -1, -1, -1, 10, -1, -1, -1, 13, -1, -1, -1, 16, -1, -1, -1, 19, -1, -1, -1, 22, -1, -1));
        __m256i b_bytes = _mm256_shuffle_epi8(colors_i, _mm256_setr_epi8(-1, -1, 2, -1, -1, -1, 5, -1, -1, -1, 8, -1, -1, -1, 11, -1, -1, -1, 14, -1, -1, -1, 17, -1, -1, -1, 20, -1, -1, -1, 23, -1));

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
        total_r += _data[i * 3 + 0];
        total_g += _data[i * 3 + 1];
        total_b += _data[i * 3 + 2];
    }

    if (num_pixels > 0) {
        return Color(total_r / num_pixels, total_g / num_pixels, total_b / num_pixels);
    } else {
        return Color(0.0f, 0.0f, 0.0f);
    }
}

float** ImagePPM::histo() const {
    float** _histo = new float*[3];

    for (size_t i = 0; i < 3; i++) {
        _histo[i] = new float[256]();
    }

    for (size_t i = 0; i < size() * 3; i += 3) {
        _histo[0][_data[i]]++;
        _histo[1][_data[i + 1]]++;
        _histo[2][_data[i + 2]]++;
    }

    return _histo;
}

float** ImagePPM::ddp() const {
    float** _ddp = histo();

    for (size_t i = 0; i < 3; i++) {
        for (size_t j = 0; j < 256; j++) {
            _ddp[i][j] /= (float) size();
        }
    }

    return _ddp;
}

float ImagePPM::mse(const ImagePPM& other) const {

    if (other.size() == size()) {
        float _mse = 0.0f;

        for (size_t i = 0; i < size() * 3; i++) {
            _mse += pow(((float) _data[i]) - ((float) other._data[i]), 2);
        }

        _mse = _mse / ((float) size() * 3);

        return _mse;
    }

    return numeric_limits<float>::quiet_NaN();
}

float ImagePPM::psnr(const ImagePPM& other) const {
    float _psnr = mse(other);

    if (!isnan(_psnr)) {
        if (_psnr > 0.0) {
            _psnr = 10.0f * log10(pow(255.0f, 2) / _psnr);
    
            return _psnr;
        }
    
        return numeric_limits<float>::infinity();
    }

    return numeric_limits<float>::quiet_NaN();
}

float ImagePPM::bhattacharyyaDist(const ImagePPM& other) const {
    float distR = 0.0f;
    float distG = 0.0f;
    float distB = 0.0f;

    float** self_ddp = ddp();
    float** other_ddp = other.ddp();

    for (size_t i = 0; i < 256; i++) {
        distR += sqrt(self_ddp[0][i] * other_ddp[0][i]);
        distG += sqrt(self_ddp[1][i] * other_ddp[1][i]);
        distB += sqrt(self_ddp[2][i] * other_ddp[2][i]);
    }

    distR = -log(distR);
    distG = -log(distG);
    distB = -log(distB);

    float dist = (distR + distG + distB) / 3.0f;

    for (size_t i = 0; i < 3; ++i) {
        delete[] self_ddp[i];
        delete[] other_ddp[i];
    }

    delete[] self_ddp;
    delete[] other_ddp;

    return dist;
}

float ImagePPM::chi2(const ImagePPM& other) const {
    float distR = 0.0f;
    float distG = 0.0f;
    float distB = 0.0f;

    float** self_ddp = ddp();
    float** other_ddp = other.ddp();

    for (size_t i = 0; i < 256; i++) {
        float sumR = self_ddp[0][i] + other_ddp[0][i];
        float sumG = self_ddp[1][i] + other_ddp[1][i];
        float sumB = self_ddp[2][i] + other_ddp[2][i];

        if (sumR > 0.0f) {
            distR += pow(self_ddp[0][i] - other_ddp[0][i], 2) / sumR;
        }

        if (sumG > 0.0f) {
            distG += pow(self_ddp[1][i] - other_ddp[1][i], 2) / sumG;
        }

        if (sumB > 0.0f) {
            distB += pow(self_ddp[2][i] - other_ddp[2][i], 2) / sumB;
        }
    }

    
    float dist = (distR + distG + distB) / 3.0f;

    for (size_t i = 0; i < 3; ++i) {
        delete[] self_ddp[i];
        delete[] other_ddp[i];
    }

    delete[] self_ddp;
    delete[] other_ddp;

    return dist;
}

ImagePGM ImagePPM::toPGM() {
    ImagePGM out_image(_width, _height);

    for (size_t i = 0; i < _height; i++) {
        for (size_t j = 0; j < _width * 3; j += 3) {
            float r = (float) _data[i * (_width * 3) + j];
            float g = (float) _data[i * (_width * 3) + (j + 1)];
            float b = (float) _data[i * (_width * 3) + (j + 2)];

            out_image[i][(j / 3)] = (octet)((r + g + b) / 3.0f);
        }
    }
    return out_image;
}


/* Setters */

void ImagePPM::resize(size_t new_width, size_t new_height) {
    if ((_width != 0 && _height != 0) && ((_width != new_width) || (_height != new_height))) {
        size_t new_size = new_width * new_height * 3;
        octet* new_data = new octet[new_size];

        float width_ratio = ((float) (_width - 1)) / ((float) (new_width - 1));
        float height_ratio = ((float) (_height - 1)) / ((float) (new_height - 1));

        for (size_t y = 0; y < new_height; y++) {
            for (size_t x = 0; x < new_width; x++) {
                float src_x = x * width_ratio;
                float src_y = y * height_ratio;

                int x1 = (int) (src_x);
                int y1 = (int) (src_y);
                int x2 = min(x1 + 1, (int) (_width - 1));
                int y2 = min(y1 + 1, (int) (_height - 1));

                float x_weight = src_x - x1;
                float y_weight = src_y - y1;

                for (size_t k = 0; k < 3; k++) {
                    float top_left = (float) (_data[(y1 * _width + x1) * 3 + k]);
                    float top_right = (float) (_data[(y1 * _width + x2) * 3 + k]);
                    float bottom_left = (float) (_data[(y2 * _width + x1) * 3 + k]);
                    float bottom_right = (float) (_data[(y2 * _width + x2) * 3 + k]);

                    float top = top_left * (1 - x_weight) + top_right * x_weight;
                    float bottom = bottom_left * (1 - x_weight) + bottom_right * x_weight;

                    new_data[(y * new_width + x) * 3 + k] = (octet) (top * (1 - y_weight) + bottom * y_weight);
                }
            }
        }

        delete[] _data;
        _data = new_data;
        Image::resize(new_width, new_height);
    }
}

void ImagePPM::segment(size_t block_size) {
    if ((block_size < _width && block_size < _height) && block_size != 0) {
        size_t new_width = block_size * (_width / block_size);
        size_t new_height = block_size * (_height / block_size);

        resize(new_width, new_height);

        for (size_t i = 0; i < _height; i += block_size) {
            for (size_t j = 0; j < _width; j += block_size) {
                Color _average;
                size_t index;

                for (size_t y = 0; y < block_size; y++) {
                    for (size_t x = 0; x < block_size; x++) {
                        index = ((i + y) * _width + (j + x)) * 3;
                        _average[0] += (float) _data[index];
                        _average[1] += (float) _data[index + 1];
                        _average[2] += (float) _data[index + 2];
                    }
                }

                _average /= pow(block_size, 2);

                for (size_t y = 0; y < block_size; y++) {
                    for (size_t x = 0; x < block_size; x++) {
                        index = ((i + y) * _width + (j + x)) * 3;

                        _data[index] = (octet)(_average[0]);
                        _data[index + 1] = (octet)(_average[1]);
                        _data[index + 2] = (octet)(_average[2]);

                    }
                }
            }
        }
    }

}

void ImagePPM::mosaic(size_t block_size, const char* lib_path, size_t lib_size, int mode) {
    size_t width_factor = _width / block_size;
    size_t height_factor = _height / block_size;
    size_t total_block = width_factor * height_factor;

    size_t new_width = block_size * width_factor;
    size_t new_height = block_size * height_factor;

    resize(new_width, new_height);
    

    octet* new_data = new octet[size() * 3];

    
    atomic<size_t> processed_blocks(0);

    auto computeMosaicBlock = [&](size_t thread_id, size_t start, size_t end) -> void {

        for (size_t block_id = start; block_id < end; block_id++) {
            size_t block_row = block_id / width_factor;
            size_t block_col = block_id % width_factor;
            size_t index;
            float min_dist = numeric_limits<float>::max();
    
            string best_path;
            ImagePPM tile, block;
            octet* block_data = new octet[block_size * block_size * 3];
    
            for (size_t i = 0; i < block_size; i++) {
                for (size_t j = 0; j < block_size; j++) {
                    index = ((block_row * block_size + i) * _width + (block_col * block_size + j)) * 3;
                    block_data[(i * block_size + j) * 3] = _data[index + 0];
                    block_data[(i * block_size + j) * 3 + 1] = _data[index + 1];
                    block_data[(i * block_size + j) * 3 + 2] = _data[index + 2];
                }
            }
    
            block = ImagePPM(block_size, block_size, block_data);
            delete[] block_data;


            for (size_t i = 0; i < lib_size; i++) {
                string current_path = string(lib_path) + "/" + to_string(i) + ".ppm";
                
                if (tile.read(current_path.c_str())) {
                    tile.resize(block_size, block_size);
                    float current_dist;

                    switch (mode) {
                        case 1:
                            current_dist = tile.bhattacharyyaDist(block);
                            break;

                        case 2:
                            current_dist = tile.chi2(block);
                            break;

                        default:
                        current_dist = (tile.average() - block.average()).norm();
                        break;
                    }

                    if (current_dist < min_dist) {
                        min_dist = current_dist;
                        best_path = current_path;
                    }
                }
            }
    
            if (!best_path.empty()) {
                if (tile.read(best_path.c_str())) {
                    tile.resize(block_size, block_size);

                    for (size_t i = 0; i < block_size; i++) {
                        size_t dest_row_start = (block_row * block_size + i) * _width * 3 + (block_col * block_size) * 3;
                        octet* src_row = tile[i];

                        size_t j = 0;

                        for (; j + 8 <= block_size; ++j) {
                            _mm256_storeu_si256((__m256i*) &new_data[dest_row_start + j * 3], _mm256_loadu_si256((const __m256i*) &src_row[j * 3]));
                        }
                        
                        for (; j < block_size; ++j) {
                            new_data[dest_row_start + j * 3 + 0] = src_row[j * 3 + 0];
                            new_data[dest_row_start + j * 3 + 1] = src_row[j * 3 + 1];
                            new_data[dest_row_start + j * 3 + 2] = src_row[j * 3 + 2];
                        }
                    }
                }
            }

            processed_blocks++;
        }

    };

    size_t total_thread = thread::hardware_concurrency();
    size_t blocks_per_thread = total_block / total_thread;
    size_t remaining_blocks = total_block % total_thread;
    thread threads[total_thread];

    size_t current_block = 0;
    for (size_t i = 0; i < total_thread; i++) {
        size_t start_block = current_block;
        size_t end_block = current_block + blocks_per_thread + (i < remaining_blocks ? 1 : 0);

        threads[i] = thread(computeMosaicBlock, i, start_block, end_block);
        current_block = end_block;
    }

    while (processed_blocks < total_block) {
        printPercent(processed_blocks, total_block);
    }

    for (size_t i = 0; i < total_thread; ++i) {
        threads[i].join();
    }

    printPercent(processed_blocks, total_block);
    cout << endl;

    delete[] _data;
    _data = new_data;
}


/* Opérateurs */

ImagePPM& ImagePPM::operator= (const ImagePPM& other) {
    if (this != &other) {
        delete[] _data;
        _width = other._width;
        _height = other._height;
        _data = new octet[size() * 3];

        copy(other._data, other._data + size() * 3, _data);
    }

    return *this;
}

octet* ImagePPM::operator[] (size_t i) {
    if (i < _height) {
        return _data + (i * (_width * 3));
    }

    __throw_out_of_range("Index out of bound.");
}


/* Méthodes de chiffrements */

size_t* ImagePPM::swap(size_t block_size) {
    size_t width_factor = _width / block_size;
    size_t height_factor = _height / block_size;
    size_t total_block = width_factor * height_factor;

    size_t new_width = block_size * width_factor;
    size_t new_height = block_size * height_factor;

    resize(new_width, new_height);

    octet* new_data = new octet[size()];
    size_t* key = new size_t[total_block]();


    for (size_t i = 0; i < total_block; i++) {
        key[i] = i;
    }

    random_device rd;
    mt19937 gen(rd());

    shuffle(key, key + total_block, gen);

    for (size_t block_id = 0; block_id < total_block; block_id++) {
        size_t swapped_block_id = key[block_id];

        size_t block_row = block_id / width_factor;
        size_t block_col = block_id % width_factor;
        size_t swapped_block_row = swapped_block_id / width_factor;
        size_t swapped_block_col = swapped_block_id % width_factor;

        for (size_t i = 0; i < block_size; i++) {
            for (size_t j = 0; j < block_size; j++) {
                size_t index = ((swapped_block_row * block_size + i) * _width + (swapped_block_col * block_size + j)) * 3;
                size_t swapped_index = ((block_row * block_size + i) * _width + (block_col * block_size + j)) * 3;

                for (size_t k = 0; k < 3; k++) {
                    new_data[index + k] = _data[swapped_index + k];
                }
            }
        }
    }

    delete[] _data;
    _data = new_data;
    return key;

};

void ImagePPM::sort(size_t block_size, size_t *key) {
    size_t width_factor = _width / block_size;
    size_t height_factor = _height / block_size;
    size_t total_block = width_factor * height_factor;

    octet* new_data = new octet[size()];

    for (size_t block_id = 0; block_id < total_block; block_id++) {
        size_t swapped_block_id = key[block_id];

        size_t block_row = block_id / width_factor;
        size_t block_col = block_id % width_factor;
        size_t swapped_block_row = swapped_block_id / width_factor;
        size_t swapped_block_col = swapped_block_id % width_factor;

        for (size_t i = 0; i < block_size; i++) {
            for (size_t j = 0; j < block_size; j++) {
                size_t index = ((swapped_block_row * block_size + i) * _width + (swapped_block_col * block_size + j)) * 3;
                size_t swapped_index = ((block_row * block_size + i) * _width + (block_col * block_size + j)) * 3;

                for (size_t k = 0; k < 3; k++) {
                    new_data[swapped_index + k] = _data[index + k];
                }
            }
        }
    }

    delete _data;
    _data = new_data;
}


/* Lecture et écriture  */

bool ImagePPM::read(const char *path) {
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
    if (fscanf(file, "%ld %ld %hhd", &_width, &_height, &maxValue) != 3) {
        cerr << "Erreur - Lecture des dimensions impossible" << endl;
        fclose(file);
        exit(EXIT_FAILURE);
    }
    fgetc(file); // Ignorer le saut de ligne
    
    if (maxValue != 255) {
        cerr << "Erreur - Valeur maximale éronée" << endl;
        fclose(file);
        exit(EXIT_FAILURE);}
    
    delete[] _data;
    _data = new octet[size() * 3];
    
    /* Lecture des valeurs des couleurs */
    if (fread(_data, sizeof(octet), size() * 3, file) != size() * 3) {
        cerr << "Erreur - Problème lors de la lecture des valeurs des couleurs" << endl;
        delete[] _data;
        fclose(file);
        exit(EXIT_FAILURE);
    }
    
    fclose(file);
    return true;
    
}
    
bool ImagePPM::write(const char *path, const char *comment) {
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
    fprintf(file, "%ld %ld\n255\n", _width, _height);
    
    /* Ecriture des valeurs des couleurs */
    if (_data != nullptr) {
        if (fwrite(_data, sizeof(octet), size() * 3, file) != size() * 3) {
            cerr << "Erreur - Problème lors de l'écriture des valeurs des couleurs" << endl;
        }
    }
    
    fclose(file);
    return true;
}