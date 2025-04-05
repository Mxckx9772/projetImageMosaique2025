#ifndef IMAGE_PGM_H
#define IMAGE_PGM_H

#include "Image.hpp"

class ImagePGM : public Image {
    private:
        octet* grey;
        char* comment;

    public:
        /* Constructeurs et destructeurs */
        ImagePGM();
        ImagePGM(size_t newWidth, size_t newHeight);
        ~ImagePGM() override;

        /* Opérateur d'accès */
        octet* operator[] (size_t i) override;

        void operator= (const ImagePGM &other);
        octet average() override;


        /* Setter */
        void resize(size_t newWidth, size_t newHeight) override;
        void segment(size_t blockSize) override;
        void mosaic(size_t blockSize, const char* libPath, size_t libSize) override;

        /* Chiffrement */
        size_t* swap(size_t blockSize) override;
        void sort(size_t* key, size_t blockSize) override;
        
        /* Lecture et écriture dans un fichier */
        void read(const char* path) override;
        void write(const char* path, const char* comment = nullptr) override;
};

#endif