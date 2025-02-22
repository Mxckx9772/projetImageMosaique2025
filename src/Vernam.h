#ifndef VERNAM_H
#define VERNAM_H

#include <stdio.h>
#include <stdlib.h>

typedef unsigned char octet;

class Vernam {
private:
    octet** key; // Clé de chiffrement (pixels)
    size_t size;

public:
    // Constructeur sauvegardant la taille de l'image.
    Vernam(size_t imgSize);

    // Accesseurs aux données de chiffrement
    const octet** getKey() const;
    int getSize(); // Dans l'optique de sauvegarder une valeur de clés pour un éventuelle partage.


    // Setters sur les données de chiffrement
    void setKey(); // Dans le cas d' un déchiffrement a clés connue issue d'une potentiel partage de clés
    void setSize(); // Cas de réutilisation pour des image de taille differente. (Génère une nouvelle clés de la bonne taille)


    // Fonction de chiffrement
    void gen(); // Génération aléatoire d'une clé de taille size;
    void encrypt(octet** img); // (Chiffrement à masque jetable) Donc génération d'une nouvelle clés après chaque chiffrement;
    void decrypt(octet** img);
};

#endif // VERNAM_H