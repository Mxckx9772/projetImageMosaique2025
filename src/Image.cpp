#include "Image.h"

/* Constructeur abstrait */
Image::Image(){}
Image::~Image() {}

/* Constructeur initialisant les paramètres de taille */
Image::Image(size_t width, size_t height): width(width), height(height){
    size = width * height;
}

/* Retourne si oui ou non un indice correspond a une ligne de l'image */
bool Image::inImgLine(size_t i){
    return i < height;
}

/* Retourne si oui ou non un indice correspond a une colonne de l'image */
bool Image::inImgColumn(size_t j){
    return  j < width;
}

/* Retourne si oui ou non un couple d'indice fait partie de l'image */
bool Image::inImg(size_t i, size_t j){
    return inImgLine(i) && inImgColumn(j);
};

/* Fonction d'allocation dynamique des matrices de pixels */
octet** Image::allocateImgPixel(size_t width, size_t height) {

    // Allocation des colonnes 
    octet** array = (octet**) malloc(sizeof(octet*) * height);

    // Gestion de l'erreur d'allocation
    if(!array){
        return NULL;
    }

    // Allocation des lignes
    for (size_t i = 0; i < height; i++) {
        array[i] = (octet*) malloc(sizeof(octet) * width);

        // Gestion de l'erreur d'allocation
        if(!array[i]) {

            // Liberation des sous-tableaux
            for (size_t j = 0; j < i; ++j) {
                free(array[j]);
            }

            free(array); 
            return NULL;
        }
    }

    return array;
}



/* Fonction de libération des matrices de pixels */
void Image::freeImgPixel(octet** array, size_t height) {
    if(array){ 
        for (size_t i = 0; i < height; ++i) {
            if(array[i]){
                free(array[i]); // Libération des sous-tableaux
            }
        }

        free(array); // Libération de la matrice
    } 
}


/* Accesseur sur la largeur de l'image */
size_t Image::getWidth() const {
    return width;
}


/* Accesseur sur la hauteur de l'image */
size_t Image::getHeight() const {
    return height;
}

/* Accesseur sur la taille de l'image */
size_t Image::getSize() const {
    return size;
}


/* Lis l'en-tête d'un fichier image et retourne son format */
format Image::readHeader(FILE* imgFile){
    octet maxValue;

    // Lecture du format
    char format[3];
    if (fscanf(imgFile, "%2s", format) != 1) {
        fprintf(stderr, "Erreur - Lecture du format impossible.");
        fclose(imgFile);
        exit(EXIT_FAILURE);
    }

    if (format[0] != 'P' && !(format[1] == '6' || format[1] == '5')) {
        fprintf(stderr, "Erreur - Format invalide pour une image ppm.");
        fclose(imgFile);
        exit(EXIT_FAILURE);
    }

    skipComment(imgFile);

    // Lecture de la largeur et de la hauteur
    fscanf(imgFile, "%ld %ld %hhd", &width, &height, &maxValue);

    fgetc(imgFile); // Ignorer le saut de ligne ('\n')

    size = width * height;

    return format[1] == '6' ? PPM : PGM;
}

/* Saute les possibles commentaires du fichier */
void Image::skipComment(FILE* imgFile){
    octet c;
    c = fgetc(imgFile);
    while ((c = fgetc(imgFile)) == '#') {
        while ((c = fgetc(imgFile)) != '\n');
    }
    ungetc(c, imgFile);
}
