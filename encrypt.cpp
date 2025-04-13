#include <stdlib.h>
#include <iostream>
#include <filesystem>
#include <vector>
#include "include/ImagePPM.hpp"
#include "include/ImagePGM.hpp"

using namespace std;
using namespace filesystem;

int main(int argc, char* argv[]) {
    if (!(argc == 5 || argc == 6)) {
        cerr << "Usage : " << argv[0] << " path type mode key_name (block_size)*" << endl;
        return 1;
    }

    size_t block_size = 0;
    int mode;
    string path = argv[1];
    string type = argv[2];
    sscanf(argv[3], "%i", &mode);
    string key_path = argv[4];

    if (argc == 6) {
        sscanf(argv[5], "%lu", &block_size);
    }

    if (type == "ppm") {
        ImagePPM img;
        img.read(path.c_str());

        if (mode == 0 && block_size != 0) {
            size_t width_factor = img.width() / block_size;
            size_t height_factor = img.height() / block_size;
            size_t key_size = width_factor * height_factor;

            size_t* key = img.swap(block_size);
            writeSwapKey((key_path + ".prvt").c_str(), key, key_size, block_size);

            img.write(path.c_str(), "Encrypt by Mosaic Team.");

        } else if ( mode == 1) {
            img.vernamEncrypt((key_path + "." + type).c_str());
            img.write(path.c_str(), "Encrypt by Mosaic Team.");
        }
    } else if (type == "pgm") {
        ImagePGM img;
        img.read(path.c_str());

        if (mode == 0 && block_size != 0) {
            size_t width_factor = img.width() / block_size;
            size_t height_factor = img.height() / block_size;
            size_t key_size = width_factor * height_factor;

            size_t* key = img.swap(block_size);
            writeSwapKey((key_path + ".prvt").c_str(), key, key_size, block_size);

            img.write(path.c_str(), "Encrypt by Mosaic Team.");

        } else if ( mode == 1) {
            img.vernamEncrypt((key_path + "." + type).c_str());
            img.write(path.c_str(), "Encrypt by Mosaic Team.");
        }

    }



    /*ImagePPM img;
    size_t block_size = 128;

    img.read(path.c_str());

    size_t width_factor = img.width() / block_size;
    size_t height_factor = img.height() / block_size;
    size_t key_size = width_factor * height_factor;


    size_t* key = img.swap(block_size);

    size_t key_size = (img.width() / block_size) * (img.height() / block_size);


    writeSwapKey("key.prvt", key, key_size, block_size);
    cout << block_size << " " << total_block << endl;

    delete key;

    key = readSwapKey("key.prvt", &key_size, &block_size);

    cout << block_size << " " << key_size << endl;
    img.write("encrypt.ppm", "");

    img.sort(block_size, key);

    img.write("decrypt.ppm", "");*/

    return 0;
}