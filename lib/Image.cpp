#include "../include/Image.hpp"
#include <iostream>
#include <iomanip> 

// Codes ANSI pour la couleur violette et le gras
#define COLOR_VIOLET "\033[95m"
#define BOLD "\033[1m"
#define RESET "\033[0m"

using namespace std;

/* Constructeurs et destructeurs */
Image::Image(): width(0), height(0) {}

Image::Image(size_t newWidth, size_t newHeight): width(newWidth), height(newHeight) {}

Image::~Image() {} // Destructeur virtuel


/* Accesseur et opérateurs d'accès */
size_t Image::getWidth() {
    return width;
}

size_t Image::getHeight() {
    return height;
}

size_t Image::size() {
    return (width * height);
}

octet* Image::operator[] (size_t i){
    return nullptr;
} // Opérateur virtuel

octet Image::average() {
    return 0;
};

/* Setter virtuel */
void Image::resize(size_t newWidth, size_t newHeight) {
    width = newWidth;
    height = newHeight;
}

void Image::segment(size_t blockSize) {}

void Image::mosaic(size_t blockSize, const char* libPath, size_t libSize) {}


/* Chiffrement virtuel */
size_t* Image::swap(size_t blockSize) {
    return nullptr;
}

void Image::sort(size_t* key, size_t blockSize) {}

/* Lecture et écriture dans un fichier */
void Image::read(const char* path) {}

void Image::write(const char* path, const char* comment) {}



/* Fonctions d'aides */

void printPercent(int pourcentage) {
    const int barreLongueur = 100;
    int nbBlocs = (int) (((float)(pourcentage) / 100.0 * barreLongueur));

    cout << "[" << BOLD << COLOR_VIOLET << setw(3) << pourcentage << "%" << RESET << "]";
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

uint32_t getNanoSecondSeed() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint32_t)(ts.tv_nsec);
}