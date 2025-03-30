// Bibliothèques standards
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

// Inlcudes
#include "include/ImagePPM.hpp"

using namespace std;

int main(int argc, char** argv){
    ImagePPM loutre, loutreEncryt, resizeLoutre;
    size_t blockSize = 12;
    size_t* key;

    loutre.read("loutre.ppm");
    loutreEncryt = loutre;
    resizeLoutre = loutre;

    loutre.segment(blockSize);
    loutreEncryt.segment(blockSize);


    key = loutreEncryt.swap(blockSize);

    loutreEncryt.sort(key, blockSize);

    resizeLoutre.resize(100, 512);

    loutre.write("segment_loutre.ppm", "Image Segmentée.");
    loutreEncryt.write("encrypt_loutre.ppm", "Image Chiffrée.");
    resizeLoutre.write("loutre_resize.ppm", "Image Chiffrée.");

    return EXIT_SUCCESS;
}