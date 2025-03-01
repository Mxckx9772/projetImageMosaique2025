#ifndef IMAGE_GREY_H
#define IMAGE_GREY_H

#include "Image.h"

class ImageGrey : public Image{
    protected:
        // Lecture de l'image
        void readImage(const char* imgFile) override;
    private:
        octet** grey;

    public:
        ~ImageGrey();
        ImageGrey(size_t width, size_t height);
        ImageGrey(const char* imgFile);

        // Accesseurs
        const octet getValue(size_t i, size_t j);
        const octet* getLine(size_t i);
        const octet* getColumn(size_t j);

        // Setters
        void setValue(octet value, size_t i, size_t j);
        void setAll(octet value);
        void setLine(octet value, size_t i);
        void setColumn(octet value, size_t j);
        void setLine(octet* values, size_t i);
        void setColumn(octet* values, size_t j);

        // Ecriture de l'image
        void writeImage(const char* imgFile) override;
};

#endif