#include "Image.h"


// Constructeurs
Image::Image(octet** redChannel, octet** greenChannel, octet** blueChannel, int width, int height)
    :red(redChannel), green(greenChannel), blue(blueChannel), width(width), height(height) {
        size = width * height;
    }

Image::Image(int width, int height): width(width), height(height){
    size = width * height;
    printf("hey");
    red = allocateImgPixel(width, height);
    green = allocateImgPixel(width, height);
    blue = allocateImgPixel(width, height);
}

Image::Image(const char* imgName){
    readImage(imgName);
}

// Acesseurs
const octet** Image::getRed() const {
    return (const octet**) red;
}

const octet** Image::getGreen() const {
    return (const octet**) green;
}

const octet** Image::getBlue() const {
    return (const octet**) blue;
}

int Image::getWidth() const {
    return width;
}

int Image::getHeight() const {
    return height;
}


// Setters
void Image::setRed(octet** newRed) {
    red = newRed;
}

void Image::setGreen(octet** newGreen) {
    green = newGreen;
}

void Image::setBlue(octet** newBlue) {
    blue = newBlue;
}


void skipComment(FILE* file){
    
}

// Lecture et ecriture de fichiers
void Image::readImage(const char* imgName) {
    FILE* imgFile;
    int maxValue;

    imgFile = fopen(imgName, "rb");

    if (!imgFile) {
        fprintf(stderr, "Erreur - Impossible d'accéder au fichier : %s\n", imgName );
        return;
    }


    // Lecture du format
    char format[3];
    if (fscanf(imgFile, "%2s", format) != 1) {
        fprintf(stderr, "Erreur - Problème lors de la lecture du format.");
        fclose(imgFile);
        return;
    }

    if (format[0] != 'P' && format[1] != '6') {
        fprintf(stderr, "Erreur - Format invalide pour une image ppm.");
        fclose(imgFile);
        return;
    }

    // Ignorer les commentaires
    octet c;
    c = fgetc(imgFile);
    while ((c = fgetc(imgFile)) == '#') {
        while ((c = fgetc(imgFile)) != '\n');
    }
    ungetc(c, imgFile);

    
	// Lecture de la taille de la  largeur et de la hauteur
	fscanf(imgFile, "%d %d %d", &width, &height, &maxValue);

    c = fgetc(imgFile); // Ignorer le saut de ligne ('\n')

    size = width * height; // Calcule de la taille

    // Allocation dynamique des tableaux des composantes couleurs
    red = allocateImgPixel(width, height);
    green = allocateImgPixel(width, height);
    blue = allocateImgPixel(width, height);

    if (!red || !green || !blue) {
        fprintf(stderr, "Erreur - Echec de l'allocation des tableau de composantes couleurs");

        // Vidage des tableaux
        freeImgPixel(red, height);
        freeImgPixel(green, height);
        freeImgPixel(blue, height);
        fclose(imgFile); // Fermeture du fichier
        return;
    }


    // Lecture des valeurs RGB
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width * 3; j += 3) {

            // Lecture des valeurs rouges
            if (fread(&red[i][j/3], sizeof(octet), 1, imgFile) != 1) {
                fprintf(stderr, "Erreur - Lecture impossible de la valeur rouge d'indices (%d, %d)\n", i, j/3);
                return;
            }

            // Lecture des valeurs vertes
            if (fread(&green[i][j/3], sizeof(octet), 1, imgFile) != 1) {
                fprintf(stderr, "Erreur - Lecture impossible de la valeur verte d'indices (%d, %d)\n", i, j/3);
                return;
            }

            // Lecture des valeurs bleus
            if (fread(&blue[i][j/3], sizeof(octet), 1, imgFile) != 1) {
                fprintf(stderr, "Erreur - Lecture impossible de la valeur bleu d'indices (%d, %d)\n", i, j/3);
                return;
            }
        }
    }

    fclose(imgFile);
}

void Image::writeImage(const char* imgName) {
    FILE *imgFile;

    imgFile = fopen(imgName, "wb");

    if(!imgFile){
        fprintf(stderr, "Impossible d'acceder au fichier %s\n", imgName);
        return;
    }else{


        // En-tête
        fprintf(imgFile, "P6\r");                               /*ecriture entete*/
        fprintf(imgFile, "%d %d\r255\r", width, height);
        
        // Ecriture des valeurs RGB
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width * 3; j += 3) {

                // Lecture des valeurs rouges
                if (fwrite(&red[i][j/3], sizeof(octet), 1, imgFile) != 1) {
                    fprintf(stderr, "Erreur - Ecriture impossible de la valeur rouge d'indices (%d, %d)\n", i, j/3);
                    return;
                }

                // Lecture des valeurs vertes
                if (fwrite(&green[i][j/3], sizeof(octet), 1, imgFile) != 1) {
                    fprintf(stderr, "Erreur - Ecriture impossible de la valeur verte d'indices (%d, %d)\n", i, j/3);
                    return;
                }

                // Lecture des valeurs bleus
                if (fwrite(&blue[i][j/3], sizeof(octet), 1, imgFile) != 1) {
                    fprintf(stderr, "Erreur - Ecriture impossible de la valeur bleu d'indices (%d, %d)\n", i, j/3);
                    return;
                }
            }
        }

        fclose(imgFile);

    }
}

octet** Image::allocateImgPixel(int width, int height) {

    octet** array = (octet**) malloc(sizeof(octet*) * height);

    if(!array){
        return NULL;
    }

    for (int i = 0; i < height; i++) {
        array[i] = (octet*) malloc(sizeof(octet) * height); // Allocation d'un sous tableau

        if(!array[i]) { // En cas de problème d'allocation
            for (int j = 0; j < i; ++j) { // Liberation des sous-tableaux
                free(array[j]);
            }
            free(array); // Libération de la matrice
            return NULL;
        }
    }

    return array; // retour de l'adresse en cas de succès
    printf("alloué !");
}

void Image::freeImgPixel(octet** array, int height) {
    if(array){ // Si l'adresse existe
        for (int i = 0; i < height; ++i) {
            free(array[i]); // Libération des sous-tableaux
        }

        free(array); // Libération de la matrice
    } 
}