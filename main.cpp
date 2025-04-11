#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <vector>
#include <string>
#include <iostream>
#include <ctime>
#include <iomanip>

#include "include/ImagePPM.hpp"
#include "include/ImagePGM.hpp"

using namespace std;

size_t BLOCK_SIZE = 8;
size_t STICKER_SIZE = 128;
const char* LIB_PATH = "pp_img";
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
    char imgInType[10] = "ppm"; // Default to ppm
    ImagePPM imgPPM, stickerPPM;
    ImagePGM imgPGM, stickerPGM;
    size_t newWidth, newHeight, percent, currentPercent, maxDim;
    string stickerName;
    vector<char*> library;
    clock_t start, end;
    double duration;

    switch (argc) {
        case 2:
            sscanf(argv[1], "%s", imgPath);
            break;
        case 3:
            sscanf(argv[1], "%s", imgPath);
            sscanf(argv[2], "%s", imgInType);
            break;
        case 4:
            sscanf(argv[1], "%s", imgPath);
            sscanf(argv[2], "%s", imgInType);
            sscanf(argv[3], "%zu", &BLOCK_SIZE);
            break;
        case 5:
            sscanf(argv[1], "%s", imgPath);
            sscanf(argv[2], "%s", imgInType);
            sscanf(argv[3], "%zu", &BLOCK_SIZE);
            sscanf(argv[4], "%zu", &STICKER_SIZE);
            break;
        case 6:
            sscanf(argv[1], "%s", imgPath);
            sscanf(argv[2], "%s", imgInType);
            sscanf(argv[3], "%zu", &BLOCK_SIZE);
            sscanf(argv[4], "%zu", &STICKER_SIZE);
            sscanf(argv[5], "%zu", &LIB_SIZE);
            break;
        default:
            cerr << "Usage: imgPath [imgInType(ppm|pgm)]* [blockSize]* [stickerSize]* [libSize]*" << endl;
            return 1;
            break;
    }

    // Préparation de la librairie d'image
    getPathList("img", &library);
    percent = 0;
    cout << "Préparation de la librairie d'image..." << endl;
    start = clock();
    for(size_t i = 0; i < LIB_SIZE; i++){
        string outStickerName;
        if (string("pgm") == imgInType) {
            string baseName = string(LIB_PATH) + string("/") + to_string(i);
            outStickerName = baseName + string(".pgm");
            stickerPPM.read(library[i]);
            stickerPGM = stickerPPM.toPGM();
            stickerPGM.resize(STICKER_SIZE, STICKER_SIZE);
            stickerPGM.write(outStickerName.data());
        } else {
            outStickerName = string(LIB_PATH) + string("/") + to_string(i) + string(".ppm");
            stickerPPM.read(library[i]);
            stickerPPM.resize(STICKER_SIZE, STICKER_SIZE);
            stickerPPM.write(outStickerName.data());
        }

        currentPercent = (((float) i / (float) LIB_SIZE) * 100.0);

        if(currentPercent != percent || i == 0) {
            percent = currentPercent;
            printPercent(percent);
        }
    }
    end = clock();
    duration = ((end - start) / CLOCKS_PER_SEC) / 60.0;
    printPercent(100);

    cout << endl << endl;
    cout << "Temps de traitement de la librairie : " << fixed << setprecision(3) << duration << " min" << endl;

    cout << "Préparation de l'image d'entrée..." << endl;
    if (strcmp(imgInType, "pgm") == 0) {
        imgPGM.read(imgPath);
        maxDim = max(imgPGM.getWidth(), imgPGM.getHeight());
        if(maxDim > 512) {
            newWidth = (size_t) (((float) imgPGM.getWidth() / (float) imgPGM.getHeight()) * 512.0);
            newHeight = (size_t) (((float) imgPGM.getHeight() / (float) imgPGM.getWidth()) * 512.0);
            imgPGM.resize(
                imgPGM.getWidth() == maxDim ? 512 : newWidth,
                imgPGM.getHeight() == maxDim ? 512 : newHeight
            );
        }
        newWidth = (imgPGM.getWidth() / BLOCK_SIZE) * STICKER_SIZE;
        newHeight = (imgPGM.getHeight() / BLOCK_SIZE) * STICKER_SIZE;
        imgPGM.resize(newWidth, newHeight);

        cout << "Création de la mosaique PGM.." << endl;
        start = clock();
        imgPGM.mosaic(STICKER_SIZE, LIB_PATH, LIB_SIZE);
        end = clock();
        duration = ((end - start) / CLOCKS_PER_SEC) / 60.0;
        cout << "Temps de traitement de la mosaïque : " << fixed << setprecision(3) << duration << " min" << endl;
        imgPGM.write("./out/mosaic.pgm");

    } else { // Default to PPM
        imgPPM.read(imgPath);
        maxDim = max(imgPPM.getWidth(), imgPPM.getHeight());
        if(maxDim > 512) {
            newWidth = (size_t) (((float) imgPPM.getWidth() / (float) imgPPM.getHeight()) * 512.0);
            newHeight = (size_t) (((float) imgPPM.getHeight() / (float) imgPPM.getWidth()) * 512.0);
            imgPPM.resize(
                imgPPM.getWidth() == maxDim ? 512 : newWidth,
                imgPPM.getHeight() == maxDim ? 512 : newHeight
            );
        }
        newWidth = (imgPPM.getWidth() / BLOCK_SIZE) * STICKER_SIZE;
        newHeight = (imgPPM.getHeight() / BLOCK_SIZE) * STICKER_SIZE;
        imgPPM.resize(newWidth, newHeight);

        cout << "Création de la mosaique PPM.." << endl;
        start = clock();
        imgPPM.mosaic(STICKER_SIZE, LIB_PATH, LIB_SIZE);
        end = clock();
        duration = ((end - start) / CLOCKS_PER_SEC) / 60.0;
        cout << "Temps de traitement de la mosaïque : " << fixed << setprecision(3) << duration << " min" << endl;
        imgPPM.write("./out/mosaic.ppm");
    }

    for (char* path : library) {
        free(path);
    }
    library.clear();

    return 0;
}