#include "ImageGrey.h"

ImageGrey::ImageGrey(size_t width, size_t height): Image(width, height){
    setAll(0);
}
ImageGrey::~ImageGrey() {}
ImageGrey::ImageGrey(const char* imgFileName){
    readImage(imgFileName);
}

// Accesseurs
const octet ImageGrey::getValue(size_t i, size_t j) {
    if(inImg(i, j)){
        return grey[i][j];
    }else{
        fprintf(stderr, "Erreur - Mauvais indices : (%li, %li)", i, j);
        return -1;
    }
}
const octet* ImageGrey::getLine(size_t i) {
    octet* line = (octet*) malloc(sizeof(octet) * width);
    if(inImgLine(i)){
        for(size_t j = 0; j < height; j++){
            line[j] = grey[i][j];
        }
    }else{
        fprintf(stderr, "Erreur - Mauvais indice de ligne : %li", i);
        return NULL;
    }

    return line;
}

const octet* ImageGrey::getColumn(size_t j) {
    octet* column = (octet*) malloc(sizeof(octet) * height);
    if(inImgColumn(j)){
        for(size_t i = 0; i < width; i++){
            column[i] = grey[i][j];
        }
    }else{
        fprintf(stderr, "Erreur - Mauvais indice de colonne : %li", j);
        return NULL;
    }

    return column;
}

// Setters
void ImageGrey::setValue(octet value, size_t i, size_t j) {
    if(inImg(i, j)){
        grey[i][j] = value;
    }else{
        fprintf(stderr, "Erreur - Mauvais indices : (%li, %li)", i, j);
    }
}
void ImageGrey::setAll(octet value) {
    for(size_t i = 0; i < height; i++){
        for(size_t j = 0; j < width; j++){
            grey[i][j] = value;
        }
    }
}
void ImageGrey::setLine(octet value, size_t i) {
    if(inImgLine(i)){
        for(size_t j = 0; j < width; j++){
            grey[i][j] = value;
        }
    }
}

void ImageGrey::setColumn(octet value, size_t j) {
    if(inImgColumn(j)){
        for(size_t i = 0; i < height; i++){
            grey[i][j] = value;
        }
    }
}

// Lecture et ecriture de fichiers
void ImageGrey::readImage(const char* imgFileName){
    FILE* imgFile;

    imgFile = fopen(imgFileName, "rb");

    if(imgFile){
        if(readHeader(imgFile) == PGM){
            
        }else{
            fprintf(stderr, "Erreur - ImageColor requis pour : %s", imgFileName);
        }
    }else{
        fprintf(stderr, "Erreur - Impossible d'accéder au fichier : %s\n", imgFileName);
    }


    // Allocation des nuances de gris
    grey = allocateImgPixel(width, height);

    if (!grey) {
        fprintf(stderr, "Erreur - Echec de l'allocation des tableau de composantes couleurs");


        // Vidage du tableau
        freeImgPixel(grey, height);

        fclose(imgFile); // Fermeture du fichier

        return;
    }


    // Lecture des valeurs RGB
    for (int i = 0; i < height; i++) {
        //for (int j = 0; j < width; j++) {

            // Lecture des valeurs rouges
            if (fread(grey[i], sizeof(octet), width, imgFile) != width) {

                fprintf(stderr, "Erreur - Lecture impossible de la valeur gris d'indices (%d)\n", i);
                return;

            }
        //}

    }

    fclose(imgFile);

}

void ImageGrey::writeImage(const char* imgName) {
    FILE *imgFile;

    imgFile = fopen(imgName, "wb");

    if(!imgFile){
        fprintf(stderr, "Impossible d'acceder au fichier %s\n", imgName);
        return;

    }else{

        // En-tête
        fprintf(imgFile, "P5\r");
        fprintf(imgFile, "%ld %ld\r255\r", width, height);


        // Ecriture des valeurs de gris
        for (size_t i = 0; i < height; i++) {
            //for (size_t j = 0; j < width; j++) {

                if (fwrite(grey[i], sizeof(octet), width, imgFile) != width) {
                    fprintf(stderr, "Erreur - Ecriture impossible de la valeur rouge d'indices (%ld)\n", width);
                    fclose(imgFile);
                    return;
                }

                fprintf(stdin, "Erreur - Ecriture impossible de la valeur rouge d'indices (%ld)\n", width);

            //}


        }

        fclose(imgFile);

    }


}