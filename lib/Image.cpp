#include "../include/Image.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <cstring>

using namespace std;

/* Définiton des couleurs */

#define COLOR_VIOLET "\033[95m"
#define COLOR_GREY "\033[90m"
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

void Image::vernamEncrypt(const char* key_path) {};
void Image::vernamDecrypt(const char* key_path) {};

bool Image::read(const char* path) {
    return true;
}

bool Image::write(const char* path, const char* comment) {
    return true;
}

void printPercent(size_t current, size_t max) {
    const int barreLongueur = 50;
    size_t percent = static_cast<size_t>(((float) (current) / max) * 100.0f);
    int nbBlocs = static_cast<int>(((float)(percent) / 100.0 * barreLongueur));

    cout << "[" << BOLD << COLOR_VIOLET << setw(3) << percent << "%" << RESET << "]";
    cout << "[";

    for (int i = 0; i < nbBlocs; ++i) {
        cout << BOLD << COLOR_VIOLET << "█" << RESET;
    }

    for (int i = nbBlocs; i < barreLongueur; ++i) {
        cout << COLOR_GREY << "░" << RESET;;
    }
    cout << "]\r";
    cout.flush();
}



bool writeSwapKey(const char* path, const size_t* key, size_t key_size, size_t block_size) {
    ofstream file(path, ios::binary); // Open in binary mode

    if (!file.is_open()) {
        cerr << "Erreur - Pas d'accès en écriture sur le fichier : " << path << endl;
        return false;
    }

    file.write((char*) (&block_size), sizeof(size_t));
    file << "\n/*-----BEGIN SWAP PRIVATE KEY-----*/\n";

    file.write((char*) (key), sizeof(size_t) * key_size);
    if (file.fail()) {
        cerr << "Erreur - Problème lors de l'écriture de la clé." << endl;
        file.close();
        return false;
    }

    file << "\n/*-----END SWAP PRIVATE KEY-----*/";

    file.close();
    return true;
}

size_t* readSwapKey(const char* path, size_t* key_size, size_t* block_size) {
    ifstream file(path, ios::binary); // Open in binary mode

    if (!file.is_open()) {
        cerr << "Erreur - Impossible d'ouvrir le fichier pour la lecture : " << path << endl;
        return nullptr;
    }

    if (!file.read((char*)(block_size), sizeof(size_t))) {
        cerr << "Erreur - Impossible de lire la taille du bloc." << endl;
        file.close();
        return nullptr;
    }

    string line;
    getline(file, line); // Read the newline character after block_size

    getline(file, line);
    if (line != "/*-----BEGIN SWAP PRIVATE KEY-----*/") {
        cerr << "Erreur - Format de fichier incorrect (header manquant)." << endl;
        file.close();
        return nullptr;
    }

    // Estimation de la taille de la clés.
    file.seekg(0, ios::end);
    size_t fileSize = file.tellg();
    size_t header_size = strlen("/*-----BEGIN SWAP PRIVATE KEY-----*/\n") + sizeof(size_t) + 1;
    size_t footer_size = strlen("\n/*-----END SWAP PRIVATE KEY-----*/");
    size_t key_bytes = fileSize - header_size - footer_size;

    if ((key_bytes % sizeof(size_t)) != 0 || key_bytes <= 0) {
        cerr << "Erreur - Taille de clé invalide dans le fichier." << endl;
        file.close();
        return nullptr;
    }
    (*key_size) = key_bytes / sizeof(size_t);
    size_t* key = new size_t[(*key_size)];

    // Retour au début de la clé
    file.seekg(header_size, ios::beg);

    // Lire la clés
    if (!file.read(reinterpret_cast<char*>(key), key_bytes)) {
        cerr << "Erreur - Problème lors de la lecture de la clé." << endl;
        delete[] key;
        file.close();
        return nullptr;
    }

    getline(file, line); // Read the newline before the footer
    getline(file, line);
    if (line != "/*-----END SWAP PRIVATE KEY-----*/") {
        cerr << "Avertissement - Footer de fichier incorrect." << endl;
        file.close();
        return nullptr;
    }

    file.close();
    return key;
}