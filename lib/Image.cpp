#include "../include/Image.hpp"
#include <iostream>
#include <iomanip>

using namespace std;

/* Définiton des couleurs */

#define COLOR_VIOLET "\033[95m"
#define BOLD "\033[1m"
#define RESET "\033[0m"


/* Constructeurs et desctructeurs */

Image::Image(): _width(0), _height(0) {}

Image::Image(size_t width, size_t height): _width(width), _height(height) {}

Image::~Image() {}


/* Getters */

size_t Image::width() const {
    return _width;
}

size_t Image::height() const {
    return _height;
}

size_t Image::size() const {
    return _width * _height;
}


/* Setters */

void Image::resize(size_t width, size_t height) {
    _width = width;
    _height = height;
}

void Image::segment(size_t block_size) {}

void Image::mosaic(size_t block_size, const char* lib_path, size_t lib_size, int mode) {}


/* Opérateurs */

size_t* Image::swap(size_t block_size) {
    return nullptr;
}

void Image::sort(size_t block_size, size_t* key) {}


bool Image::read(const char* path) {
    return true;
}

bool Image::write(const char* path, const char* comment) {
    return true;
}

void printPercent(size_t current, size_t max) {
    const int barreLongueur = 50;
    size_t percent = (size_t) (((float) (current) / max) * 100.0f);
    int nbBlocs = (int) (((float)(percent) / 100.0 * barreLongueur));

    cout << "[" << BOLD << COLOR_VIOLET << setw(3) << percent << "%" << RESET << "]";
    cout << "[";

    for (int i = 0; i < nbBlocs; ++i) {
        cout << "=";
    }

    for (int i = nbBlocs; i < barreLongueur; ++i) {
        cout << " ";
    }
    cout << "]\r"; // Le '\r' permet de revenir au début de la ligne
    cout.flush();   // Force l'affichage immédiat
}