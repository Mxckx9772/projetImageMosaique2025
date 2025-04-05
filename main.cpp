#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <vector>
#include <string>
#include <iostream>

#include "include/ImagePPM.hpp"
#include "include/ImagePGM.hpp"

using namespace std;

size_t BLOCK_SIZE = 8;
size_t STICKER_SIZE = 128;
const char* LIB_PATH = "pgm";
size_t LIB_SIZE = 20580;

void getPathList(const char *folder, vector<char*> *files) {
    
    /* Ouverture du dossier */
    DIR *dossier = opendir(folder);
    if (dossier == NULL) {
        perror("Erreur - Impossible d'ouvrir le dossier");
        return;
    }

    struct dirent *entree;
    while ((entree = readdir(dossier)) != NULL) {
        if (strcmp(entree->d_name, ".") != 0 && strcmp(entree->d_name, "..") != 0) {
            char* path = (char*) calloc(1024, sizeof(char));
    

            snprintf(path, sizeof(char) * 1024, "%s/%s", folder, entree->d_name);
            (*files).push_back(path);
        }
    }

    closedir(dossier);
}


int main(int argc, char** argv){
    char imgPath[250];
    ImagePPM stickerPPM;
    ImagePGM img, stickerPGM;
    size_t newWidth, newHeight, percent, currentPercent;
    string stickerName;
    vector<char*> library;
    
    switch (argc) {
        case 2:
            sscanf(argv[1], "%s", imgPath);
            break;

        case 3:
            sscanf(argv[1], "%s", imgPath);
            sscanf(argv[2], "%zu", &BLOCK_SIZE);
            break;

        case 4:
            sscanf(argv[1], "%s", imgPath);
            sscanf(argv[2], "%zu", &BLOCK_SIZE);
            sscanf(argv[3], "%zu", &STICKER_SIZE);
            break;

        case 5:
            sscanf(argv[1], "%s", imgPath);
            sscanf(argv[2], "%zu", &BLOCK_SIZE);
            sscanf(argv[3], "%zu", &STICKER_SIZE);
            sscanf(argv[4], "%zu", &LIB_SIZE);
            break;
    
        default:
            cerr << "Usage: imgPath blockSize(*) stickerSize(*) libSize(*)" << endl;
            return 1;
            break;
    }

    // Préparation de la librairie d'image
    getPathList("img", &library);
    percent = 0;
    cout << "Préparation de la librairie d'image..." << endl;
    for(size_t i = 0; i < LIB_SIZE; i++){
        string stickerName = string(LIB_PATH) + string("/") + to_string(i) + string(".pgm");
        stickerPPM.read(library[i]);
        stickerPPM.resize(STICKER_SIZE, STICKER_SIZE);

        stickerPGM = stickerPPM.toPGM();
        stickerPGM.write(stickerName.data(), to_string(stickerPGM.average()).data());

        currentPercent = (((float) i / (float) LIB_SIZE) * 100.0);

        if(currentPercent != percent || i == 0) {
            percent = currentPercent;
            cout << "[" << percent << "%]" << endl;
        }
    }
    cout << "[100%]" << endl;

    cout << "Préparation de l'image..." << endl;
    img.read(imgPath);

    size_t maxDim = max(img.getWidth(), img.getHeight());

    if(maxDim > 512) {
        newWidth = (size_t) (((float) img.getWidth() / (float) img.getHeight()) * 512.0);
        newHeight = (size_t) (((float) img.getHeight() / (float) img.getWidth()) * 512.0);

        img.resize(
            img.getWidth() == maxDim ? 512 : newWidth,
            img.getHeight() == maxDim ? 512 : newHeight
        );
    }

    newWidth = (img.getWidth() / BLOCK_SIZE) * STICKER_SIZE;
    newHeight = (img.getHeight() / BLOCK_SIZE) * STICKER_SIZE;

    

    img.resize(newWidth, newHeight);

    cout << "Création de la mosaique.." << endl;
    img.mosaic(STICKER_SIZE, LIB_PATH, LIB_SIZE);

    img.write("mosaic.pgm");
    
    return 0;
}