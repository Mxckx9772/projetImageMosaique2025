#ifndef IMAGE_PPM_H
#define IMAGE_PPM_H

#include "Image.hpp"

class ImagePPM : public Image {
    private:
        octet* colors;

    public:
        /* Constructeurs et destructeurs */
        ImagePPM();
        ImagePPM(size_t newWidth, size_t newHeight);
        ~ImagePPM() override;

        /* Opérateur d'accès */
        octet* operator[] (size_t i) override;

        void operator= (const ImagePPM &other);

        /* Setter */
        void resize(size_t newWidth, size_t newHeight) override;
        void segment(size_t blockSize) override;

        /* Chiffrement */
        size_t* swap(size_t blockSize) override;
        void sort(size_t* key, size_t blockSize) override;

        /* Lecture et écriture dans un fichier */
        void read(const char* path) override;
        void write(const char* path, const char* comment = nullptr) override;
};

#endif