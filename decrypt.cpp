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
        cerr << "Usage : " << argv[0] << " path type mode key_path" << endl;
        return 1;
    }

    int mode;
    string path = argv[1];
    string type = argv[2];
    sscanf(argv[3], "%i", &mode);
    string key_path = argv[4];

    if (type == "ppm") {
        ImagePPM img;
        img.read(path.c_str());

        if (mode == 0) {
            size_t key_size, block_size;
            size_t* key = readSwapKey((key_path + ".prvt").c_str(), &key_size, &block_size);

            cout << block_size << endl;

            img.sort(block_size, key);

            img.write(path.c_str(), "Decrypt by Mosaic Team.");

        } else if ( mode == 1) {
            img.vernamDecrypt((key_path + "." + type).c_str());
            img.write(path.c_str(), "Encrypt by Mosaic Team.");
        }
    } else if (type == "pgm") {
        ImagePGM img;
        img.read(path.c_str());

        if (mode == 0) {
            size_t key_size, block_size;
            size_t* key = readSwapKey((key_path + ".prvt").c_str(), &key_size, &block_size);

            img.sort(block_size, key);

            img.write(path.c_str(), "Decrypt by Mosaic Team.");

        } else if ( mode == 1) {
            img.vernamDecrypt((key_path + "." + type).c_str());
            img.write(path.c_str(), "Encrypt by Mosaic Team.");
        }

    }

    return 0;
}