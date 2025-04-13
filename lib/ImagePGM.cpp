#include "../include/ImagePGM.hpp"
#include <algorithm>
#include <math.h>
#include <string>
#include <random>
#include <iostream>
#include <fstream>
#include <thread>
#include <atomic>
#include <immintrin.h>
#include <mutex>
//#include <functional>

using namespace std;


/* Constructeurs et desctructeurs */

ImagePGM::ImagePGM(): Image(), _data(nullptr) {}

ImagePGM::ImagePGM(size_t width, size_t height): Image(width, height), _data(new octet[size()]) {}

ImagePGM::ImagePGM(size_t width, size_t height, octet* data): Image(width, height), _data(new octet[size()]) {
    copy(data, data + size(), _data);
}

ImagePGM::ImagePGM(const ImagePGM& other): Image(other._width, other._height), _data(new octet[other.size()]) {
    copy(other._data, other._data + size(), _data);
}

ImagePGM::~ImagePGM() {
    delete[] _data;
}


/* Getters */

octet ImagePGM::average() const {
    float sum = 0.0f;
    size_t i = 0;

    size_t avx_end = (size() / 32) * 32;
    __m256i sum_vec = _mm256_setzero_si256();
    while (i < avx_end) {
        __m256i grey_val = _mm256_loadu_si256((__m256i *)&_data[i]);

        __m256i sum_lo = _mm256_add_epi32(_mm256_unpacklo_epi16(_mm256_cvtepu8_epi16(_mm256_extracti128_si256(grey_val, 0)), _mm256_setzero_si256()),
                                          _mm256_unpackhi_epi16(_mm256_cvtepu8_epi16(_mm256_extracti128_si256(grey_val, 0)), _mm256_setzero_si256()));
        __m256i sum_hi = _mm256_add_epi32(_mm256_unpacklo_epi16(_mm256_cvtepu8_epi16(_mm256_extracti128_si256(grey_val, 1)), _mm256_setzero_si256()),
                                          _mm256_unpackhi_epi16(_mm256_cvtepu8_epi16(_mm256_extracti128_si256(grey_val, 1)), _mm256_setzero_si256()));

        sum_vec = _mm256_add_epi32(sum_vec, sum_lo);
        sum_vec = _mm256_add_epi32(sum_vec, sum_hi);
        i += 32;
    }

    int32_t temp_sum[8];
    _mm256_storeu_si256((__m256i *)temp_sum, sum_vec);

    for (size_t j = 0; j < 8; ++j) {
        sum += temp_sum[j];
    }

    for (; i < size(); ++i) {
        sum += _data[i];
    }

    return (octet)(sum / (float)size());
}

float* ImagePGM::histo() const {
    float* _histo = new float[256]();

    for (size_t i = 0; i < size(); i++) {
        _histo[_data[i]]++;
    }

    return _histo;
}

float* ImagePGM::ddp() const {
    float* _ddp = histo();

    for (size_t i = 0; i < 256; i++) {
        _ddp[i] /= (float) size();
    }

    return _ddp;
}

float ImagePGM::mse(const ImagePGM& other) const {

    if (other.size() == size()) {
        float _mse = 0.0f;

        for (size_t i = 0; i < size(); i++) {
            _mse += pow(((float) _data[i]) - ((float) other._data[i]), 2);
        }

        _mse = _mse / ((float) size());

        return _mse;
    }

    return numeric_limits<float>::quiet_NaN();
}

float ImagePGM::psnr(const ImagePGM& other) const {
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

float ImagePGM::bhattacharyyaDist(const ImagePGM& other) const {
    float dist = 0.0f;
    float* self_ddp = ddp();
    float* other_ddp = other.ddp();

    for (size_t i = 0; i < 256; i++) {
        dist += sqrt(self_ddp[i] * other_ddp[i]);
    }

    dist = -log(dist);

    delete[] self_ddp;
    delete[] other_ddp;

    return dist;
}

float ImagePGM::chi2(const ImagePGM& other) const {
    float dist = 0.0f;
    float* self_ddp = ddp();
    float* other_ddp = other.ddp();

    for (size_t i = 0; i < 256; i++) {
        float sum = self_ddp[i] + other_ddp[i];
        if (sum > 0.0f) {
            dist += pow(self_ddp[i] - other_ddp[i], 2) / sum;
        }
    }

    delete[] self_ddp;
    delete[] other_ddp;

    return dist;
}

float ImagePGM::hellingerDist(const ImagePGM& other) const {
    float dist_sq = 0.0f;
    float* self_ddp = ddp();
    float* other_ddp = other.ddp();

    for (size_t i = 0; i < 256; i++) {
        dist_sq += pow(sqrt(self_ddp[i]) - sqrt(other_ddp[i]), 2);
    }

    delete[] self_ddp;
    delete[] other_ddp;

    return sqrt(0.5f * dist_sq);
}

/* Setters */

void ImagePGM::resize(size_t new_width, size_t new_height) {
    if ((_width != 0 &&  _height != 0)  && ((_width != new_width) || (_height != new_height))) {
        size_t new_size = new_width * new_height;
        octet* new_data = new octet[new_size];

        float width_ratio = ((float) (_width - 1)) / ((float) (new_width - 1));
        float height_ratio = ((float) (_height - 1)) / ((float) (new_height - 1));

        for (size_t y = 0; y < new_height; y++) {
            for (size_t x = 0; x < new_width; x++) {
                float src_x = x * width_ratio;
                float src_y = y * height_ratio;

                int x1 = src_x;
                int y1 = src_y;
                int x2 = min(x1 + 1, (int) (_width - 1));
                int y2 = min(y1 + 1, (int) (_height - 1));

                float x_weight = src_x - x1;
                float y_weight = src_y - y1;


                octet top_left = _data[(y1 * _width) + x1];
                octet top_right = _data[(y1 * _width) + x2];
                octet bottom_left = _data[(y2 * _width) + x1];
                octet bottom_right = _data[(y2 * _width) + x2];
                
                octet top = top_left * (1 - x_weight) + top_right * x_weight;
                octet bottom = bottom_left * (1 - x_weight) + bottom_right * x_weight;

                new_data[y * new_width + x] = (top * (1 - y_weight) + bottom * y_weight);
            }
        }

        delete[] _data;
        _data = new_data;
        Image::resize(new_width, new_height);
    }
}

void ImagePGM::segment(size_t block_size) {
    if ((block_size < _width && block_size < _height) && block_size != 0) {
        size_t block_dim = pow(block_size, 2);
        size_t new_width = block_size * (_width / block_size);
        size_t new_height = block_size * (_height / block_size);

        resize(new_width, new_height);

        for (size_t i = 0; i < _height; i += block_size) {
            for (size_t j = 0; j < _width; j += block_size) {
                float _average = 0.0f;

                for (size_t x = 0; x < block_size; x++) {
                    for (size_t y = 0; y < block_size; y++) {
                        _average += _data[((i + x) * _width) + (j + y)];
                    }
                }
    
                _average /= block_dim ;
    
                for (size_t x = 0; x < block_size; x++) {
                    for (size_t y = 0; y < block_size; y++) {
                        _data[((i + x) * _width) + (j + y)] = _average;
                    }
                }
            }
        }
    }
    
}

void ImagePGM::mosaic(size_t block_size, const char* lib_path, size_t lib_size, int mode) {
    size_t width_factor = _width / block_size;
    size_t height_factor = _height / block_size;
    size_t total_block = width_factor * height_factor;

    size_t new_width = block_size * width_factor;
    size_t new_height = block_size * height_factor;

    resize(new_width, new_height);

    octet* new_data = new octet[size()];

    
    atomic<size_t> processed_blocks(0);
    mutex mtx;


    auto computeMosaicBlock = [&](size_t thread_id, size_t start, size_t end) -> void {

        for (size_t block_id = start; block_id < end; block_id++) {
            size_t block_row = block_id / width_factor;
            size_t block_col = block_id % width_factor;
    
            float min_dist = numeric_limits<float>::max();
            string best_path;
            ImagePGM tile;
            ImagePGM block(block_size, block_size);

            for (size_t i = 0; i < block_size; i++) {
                for (size_t j = 0; j < block_size; ++j) {
                    size_t index = ((block_row * block_size + i) * _width + (block_col * block_size + j));
                    block[i][j] = _data[index];
                }
            }

            for (size_t i = 0; i < lib_size; i++) {
                string current_path = string(lib_path) + "/" + to_string(i) + ".pgm";
                
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
                        
                        case 3:
                            current_dist = tile.hellingerDist(block);
                            break;

                        default:
                        current_dist = abs(tile.average() - block.average());
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
                        size_t dest_row_start = (block_row * block_size + i) * _width + (block_col * block_size);
                        octet* src_row = tile[i];
        
                        size_t j = 0;
                        for (; j + 32 <= block_size; j += 32) {
                            _mm256_storeu_si256((__m256i*) &new_data[dest_row_start + j], _mm256_loadu_si256((const __m256i*) &src_row[j]));
                        }
        
                        for (; j < block_size; ++j) {
                            new_data[dest_row_start + j] = src_row[j];
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
        mtx.lock();
        printPercent(processed_blocks, total_block);
        mtx.unlock();
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

ImagePGM& ImagePGM::operator= (const ImagePGM& other) {
    if (this != &other) {
        delete[] _data;
        _width = other._width;
        _height = other._height;
        _data = new octet[size()];

        copy(other._data, other._data + size(), _data);
    }

    return *this;
}

octet* ImagePGM::operator[] (size_t i) {
    if (i < _height) {
        return _data + (i * _width);
    }

    __throw_out_of_range("Index out of bound.");
}


/* Méthodes de chiffrements */

size_t* ImagePGM::swap(size_t block_size) {
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
                size_t index = (swapped_block_row * block_size + i) * _width + (swapped_block_col * block_size + j);
                size_t swapped_index = (block_row * block_size + i) * _width + (block_col * block_size + j);

                new_data[index] = _data[swapped_index];
            }
        }
    }

    delete[] _data;
    _data = new_data;
    return key;

};

void ImagePGM::sort(size_t block_size, size_t *key) {
    size_t width_factor = _width / block_size;
    size_t height_factor = _height / block_size;
    size_t total_block = width_factor * height_factor;

    octet* new_data = new octet[size()];

    for (size_t block_id = 0; block_id < total_block; block_id++) {
        size_t swapped_block_id = key[block_id];

        size_t block_row = block_id / width_factor;
        size_t block_col = block_id % width_factor;
        size_t swapped_index_row = swapped_block_id / width_factor;
        size_t swapped_index_col = swapped_block_id % width_factor;

        for (size_t i = 0; i < block_size; i++) {
            for (size_t j = 0; j < block_size; j++) {
                size_t index = (swapped_index_row * block_size + i) * _width + (swapped_index_col * block_size + j);
                size_t swapped_index = (block_row * block_size + i) * _width + (block_col * block_size + j);

                new_data[swapped_index] = _data[index];
            }
        }
    }

    delete _data;
    _data = new_data;
}

void ImagePGM::vernamEncrypt(const char* key_path) {
    ImagePGM key(_width, _height);

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distrib(0, 255);

    for (size_t i = 0; i < size(); i++) {
        key._data[i] = distrib(gen);
    }

    for (size_t i = 0; i < size(); i++) {
        _data[i] ^= key._data[i];
    }

    key.write(key_path, "Vernam key created by Mosaic Team");
}

void ImagePGM::vernamDecrypt(const char* key_path) {
    ImagePGM key;
    key.read(key_path);
    if (key._width != _width || key._height != _height) {
        cerr << "Error: Key image dimensions do not match." << endl;
        return;
    }

    for (size_t i = 0; i < size(); ++i) {
        _data[i] ^= key._data[i];
    }
}

/* Lecture et écriture  */

bool ImagePGM::read(const char* path) {
    fstream file(path);

    if (!file.is_open()) {
        cerr << "Erreur - Pas d'accès en lecture sur le fichier " << path << "." << endl;
        return false;
    }

    string format;
    if (!(file >> format)) {
        cerr << "Erreur - Impossible de lire le format." << endl;
        file.close();
        return false;
    }

    if (format != "P5") {
        cerr << "Erreur - Format différent de PGM (attendu P5)." << endl;
        file.close();
        return false;
    }

    char c;
    file.get(c);

    while (file.peek() == '#') {
        string comment;
        getline(file, comment);
    }

    int maxValue;
    if (!(file >> _width >> _height >> maxValue)) {
        cerr << "Erreur - Problèmes lors de la lecture des dimensions." << endl;
        file.close();
        return false;
    }

    if (maxValue != 255) {
        cerr << "Erreur - Valeur maximale erronée (attendu 255)." << endl;
        file.close();
        return false;
    }

    size_t _size = size();

    delete[] _data;
    _data = new octet[_size];

    if (!file.read((char *) _data, _size)) {
        cerr << "Erreur - Problèmes survenus lors de la lecture des données binaires (P5)." << endl;
        delete[] _data;
        file.close();
        return false;
    }

    file.close();
    return true;
}

bool ImagePGM::write(const char* path, const char* comment) {
    ofstream file(path, ios::binary);

    if (!file.is_open()) {
        cerr << "Erreur - Pas d'accès en écriture sur l'image " << path << "." << endl;
        return false;
    }

    file << "P5\n";

    if (comment != nullptr && comment[0] != '\0') {
        file << "# " << comment << "\n";
    }

    file << _width << " " << _height << "\n255\n";

    size_t _size = size();
    file.write((char *) (_data), _size);

    if (file.fail()) {
        cerr << "Erreur - Problème lors de l'écriture des grey_val de l'image" << endl;
        file.close();
        return false;
    }

    file.close();
    return true;
}