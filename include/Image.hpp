#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <stdlib.h>

typedef unsigned char octet;

class Image {
    protected:
        size_t _width;
        size_t _height;

    public:

        /* Constructeurs et desctructeurs */
        /**
         * 
         * @brief Constructeur par défaut.
         *  
         * @note Initialise la largeur et la hauteur à 0.
         * 
         */
        Image();

        /**
         * 
         * @brief Constructeur à 2 paramêtres.
         * 
         * @param width Largeur de l'image.
         * @param height Hauteur de l'image.
         * 
         * @note Initialise la largeur et la hauteur
         * aux valeurs des paramêtres
         * 
         */
        Image(size_t width, size_t height);

        /**
         * 
         * @brief Destructeur purement virtuel.
         * 
         * @note Ne fait rien.
         */
        virtual ~Image();

        /* Getters */
        /**
         * 
         * @brief Accesseur sur la largeur.
         * 
         * @note Retourne la valeur de la largeur.
         * 
         */
        size_t width() const;

        /**
         * 
         * @brief Accesseur sur la hauteur.
         * 
         * @note Retourne la valeur de la hauteur.
         * 
         */
        size_t height() const;

        /**
         * 
         * @brief Accesseur sur la taille.
         * 
         * @note Retourne le nombre de pixels dans l'image.
         * 
         */
        size_t size() const;


        /* Setters */
        /**
         * 
         * @brief Setters sur la taille.
         * 
         * @param width Nouvelle largeur.
         * @param height Nouvelle hauteur.
         * 
         * @note Change les valeurs de la largeur
         * et de la hauteur avec les paramêtres.
         * 
         */
        virtual void resize(size_t width, size_t height);

        /**
         * 
         * @brief Méthode virtuelle de segmentation.
         * 
         * @param block_size Taille des blocs
         * 
         * @note Ne fait rien.
         * 
         */
        virtual void segment(size_t block_size);

        /**
         * 
         * @brief Méthode virtuelle de génération de mosaïques.
         * 
         * @param block_size Taille des blocs
         * @param lib_path Chemin d'accès à la base d'image. 
         * @param lib_size Nombre d'image souhaitée pour la 
         * génération.
         * 
         * @note Ne fait rien.
         * 
         */
        virtual void mosaic(size_t block_size, const char* lib_path, size_t lib_size, int mode);


        /* Méthodes de chiffrements */
        /**
         * 
         * @brief Méthode virtuelle de permutation.
         * 
         * @param block_size Taille des blocs.
         * 
         * @note Ne fait rien.
         * 
         */
        virtual size_t* swap(size_t block_size);

        /**
         * 
         * @brief Méthode virtuelle de trie.
         * 
         * @param block_size Taille des blocs.
         * 
         * @note Ne fait rien.
         * 
         */
        virtual void sort(size_t block_size, size_t* key);

        /**
         * 
         * @brief Méthode virtuelle de trie.
         * 
         * @param block_size Taille des blocs.
         * 
         * @note Ne fait rien.
         * 
         */
        virtual void vernamEncrypt(const char* key_path);

        /**
         * 
         * @brief Méthode virtuelle de trie.
         * 
         * @param block_size Taille des blocs.
         * 
         * @note Ne fait rien.
         * 
         */
        virtual void vernamDecrypt(const char* key_path);


        /* Lecture et écriture  */

        /**
         * 
         * @brief Méthode virtuelle de lecture d'image.
         * 
         * @param path Chemin d'accès à l'image.
         * 
         * @note Ne fait rien.
         * 
         */
        virtual bool read(const char* path);

        /**
         * 
         * @brief Méthode virtuelle d'écriture d'image.
         * 
         * @param path Chemin d'accès à l'image.
         * 
         * @note Ne fait rien.
         * 
         */
        virtual bool write(const char* path, const char* comment);
};

void printPercent(size_t current, size_t max);
bool writeSwapKey(const char* path, const size_t* key, size_t key_size, size_t block_size);
size_t* readSwapKey(const char* path, size_t* key_size, size_t* block_size);

#endif // IMAGE_HPP