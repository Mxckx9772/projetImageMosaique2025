#include <stdlib.h>
#include <iostream>
#include <filesystem>
#include <vector>
#include "include/ImagePPM.hpp"
#include "include/ImagePGM.hpp"

using namespace std;
using namespace filesystem;

const char* LIB_PATH = "img";


void processImageBase(const string& folder, const string& type, size_t lib_size, size_t tile_size) {
    vector<string> files;

    path folderPath(folder);

    if (!is_directory(folderPath)) {
        cerr << "Erreur - Impossible d'ouvrir le dossier : " << folder << endl;
        return;
    }

    for (const auto& entry : directory_iterator(folderPath)) {
        if (entry.is_regular_file() || entry.is_directory()) {
            if (entry.path().filename() != "." && entry.path().filename() != "..") {
                files.push_back(entry.path().string());
            }
        }
    }

    if (type == "ppm") {
        ImagePPM img;
        string out_path;

        for (size_t i = 0; i < lib_size; i++){
            out_path = type + "/" + to_string(i) + "." + type;

            img.read(files[i].c_str());
            img.resize(tile_size, tile_size);

            img.write(out_path.c_str(), "Create by Mosaic Team in 2025.");

            printPercent(i, lib_size);
        }
    } else if (type == "pgm") {
        ImagePPM temp;
        ImagePGM img;
        string out_path;

        for (size_t i = 0; i < lib_size; i++){
            out_path = type + "/" + to_string(i) + "." + type;

            temp.read(files[i].c_str());
            img = temp.toPGM();
            img.resize(tile_size, tile_size);


            img.write(out_path.c_str(), "Create by Mosaic Team in 2025");

            printPercent(i, lib_size);
        }
    }
}


int main(int argc, char* argv[]) {
    if( argc != 7){
        cerr << "Usage : " << argv[0] << " path type block_size tile_size lib_size mode" << endl;
        return 1;
    }

    string path, type, out_path;
    size_t block_size, tile_size, lib_size;
    int mode;


    path = argv[1];
    type = argv[2];
    sscanf(argv[3], "%lu", &block_size);
    sscanf(argv[4], "%lu", &tile_size);
    sscanf(argv[5], "%lu", &lib_size);
    sscanf(argv[6], "%i", &mode);

    if (type == "ppm") {
        ImagePPM img;
        img.read(path.c_str());

        cout << "Traitement des images " << type << " :"  << endl;
        processImageBase(LIB_PATH, type, lib_size, tile_size);

        size_t new_width = (img.width() / block_size) * tile_size;
        size_t new_height = (img.height() / block_size) * tile_size;

        cout << "Creation de la mosaique (PPM)" << endl;
        img.resize(new_width, new_height);
        img.mosaic(tile_size, "ppm", lib_size, mode);
        img.write("./out/mosaic.ppm", "Created by Mosaïc Team in 2025.");

    } else if (type == "pgm") {
        ImagePGM img;
        img.read(path.c_str());

        cout << "Traitement des images " << type << " :"  << endl;
        processImageBase(LIB_PATH, type, lib_size, tile_size);

        size_t new_width = (img.width() / block_size) * tile_size;
        size_t new_height = (img.height() / block_size) * tile_size;

        cout << "Creation de la mosaique (PGM)" << endl;
        img.resize(new_width, new_height);
        img.mosaic(tile_size, "pgm", lib_size, mode);
        img.write("./out/mosaic.pgm", "Created by Mosaïc Team in 2025.");
    }

    return 0;
}