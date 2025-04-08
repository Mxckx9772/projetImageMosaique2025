#ifndef IMAGE_PPM_H
#define IMAGE_PPM_H

#include "Image.hpp"
#include "ImagePGM.hpp"
#include "Color.hpp"

class ImagePPM : public Image {
    private:
        octet* pixels;

    public:
        /* Constructeurs et destructeurs */
        ImagePPM();
        ImagePPM(size_t newWidth, size_t newHeight);
        ImagePPM(const ImagePPM& other);
        ~ImagePPM() override;

        /* Opérateur d'accès */
        Color* operator[] (size_t i);

        /* Accesseur */
        Color getPixel(size_t i, size_t j) const;

        void operator= (const ImagePPM &other);
        ImagePGM toPGM();
        Color average() const;

        /* Setter */
        void setPixel(size_t i, size_t j, const Color& color);
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