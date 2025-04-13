#include <stdlib.h>
#include <iostream>
#include <filesystem>
#include <vector>
#include "include/ImagePPM.hpp"
#include "include/ImagePGM.hpp"
#include <chrono>
#include <fstream>

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

    printPercent(lib_size, lib_size);
    cout << endl;
}


int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Usage : " << argv[0] << " folder_path type lib_size" << endl;
        return 1;
    }

    string folder_path = argv[1];
    string type = argv[2];
    size_t lib_size;

    sscanf(argv[3], "%lu", &lib_size);

    ofstream results("resultats.csv");
    results << "Image,BlockSize,Mode,PSNR,Bhattacharyya,Chi2,Time(min:sec)" << endl;

    vector<string> images;
    for (const auto& entry : directory_iterator(folder_path)) {
        if (entry.is_regular_file()) {
            images.push_back(entry.path().string());
        }
    }

    for (size_t block_size = 8; block_size <= 128; block_size *= 2) {
        size_t tile_size = block_size;

        cout << "\n==============================\n";
        cout << "Traitement pour block_size = tile_size = " << block_size << endl;

        // Process once for this tile size
        processImageBase(LIB_PATH, type, lib_size, tile_size);

        for (const string& image_path : images) {
            string filename = path(image_path).filename().string();
            cout << "\n  → Image : " << filename << endl;

            for (int mode = 0; mode < 3; mode++) {
                cout << "    Mode : " << mode << endl;

                auto start = chrono::high_resolution_clock::now();

                if (type == "ppm") {
                    ImagePPM img;                    
                    ImagePPM original;
                    img.read(image_path.c_str());

                    size_t new_width = (img.width() / block_size) * tile_size;
                    size_t new_height = (img.height() / block_size) * tile_size;

                    while (new_width > 8000 || new_height > 8000) {
                        img.resize(img.width() / 2, img.height() / 2);
                        new_width = (img.width() / block_size) * tile_size;
                        new_height = (img.height() / block_size) * tile_size;
                    }

                    img.resize(new_width, new_height);
                    original = img;
                    img.mosaic(tile_size, "ppm", lib_size, mode);

                    string out_path = "./out/mosaic_" + to_string(block_size) + "_mode" + to_string(mode) + "_" + filename;
                    img.write(out_path.c_str(), "Created by Mosaic Team 2025");

                    auto end = chrono::high_resolution_clock::now();
                    chrono::duration<double> duration = end - start;
                    int minutes = static_cast<int>(duration.count()) / 60;
                    int seconds = static_cast<int>(duration.count()) % 60;


                    double psnr = original.psnr(img);
                    double bhat = original.bhattacharyyaDist(img);
                    double chi2 = original.chi2(img);

                    results << filename << "," << block_size << "," << mode << "," << psnr << "," << bhat << "," << chi2 << ","
                            << minutes << ":" << seconds << endl;
                } else if (type == "pgm") {
                    ImagePGM img;ImagePGM original;
                    img.read(image_path.c_str());

                    size_t new_width = (img.width() / block_size) * tile_size;
                    size_t new_height = (img.height() / block_size) * tile_size;

                    while (new_width > 8000 || new_height > 8000) {
                        img.resize(img.width() / 2, img.height() / 2);
                        new_width = (img.width() / block_size) * tile_size;
                        new_height = (img.height() / block_size) * tile_size;
                    }

                    img.resize(new_width, new_height);
                    original = img;
                    img.mosaic(tile_size, "pgm", lib_size, mode);

                    string out_path = "./out/mosaic_" + to_string(block_size) + "_mode" + to_string(mode) + "_" + filename;
                    img.write(out_path.c_str(), "Created by Mosaic Team 2025");

                    auto end = chrono::high_resolution_clock::now();
                    chrono::duration<double> duration = end - start;
                    int minutes = static_cast<int>(duration.count()) / 60;
                    int seconds = static_cast<int>(duration.count()) % 60;

                    double psnr = original.psnr(img);
                    double bhat = original.bhattacharyyaDist(img);
                    double chi2 = original.chi2(img);

                    results << filename << "," << block_size << "," << mode << "," << psnr << "," << bhat << "," << chi2 << ","
                            << minutes << ":" << seconds << endl;
                }
            }
        }
    }

    results.close();
    cout << "\nTous les résultats enregistrés dans resultats.csv" << endl;
    return 0;
}


