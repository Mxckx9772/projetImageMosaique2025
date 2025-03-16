// Première version de l'implémentation du TP Image
// MacOS : brew install opencv
// g++ mosaique.cpp -o mosaique $(pkg-config --cflags --libs opencv4) -std=c++17
// ./mosaique
// Linux : sudo apt update
// sudo apt upgrade
// sudo apt install libopencv-dev
// g++ mosaique.cpp -o mosaique $(pkg-config --cflags --libs opencv4) -std=c++17
// ./mosaique
// Les résultats sont disponibles dans le dossier ImageTP :
// Image_mosaique.pgm : Image découpée en bloc
// Image_best.pgm : Image mosaïque avec compression des imagettes
// Image_HD.pgm : Image haute qualité avec imagettes sans compression
// Banque d'image originale : http://vision.stanford.edu/aditya86/ImageNetDogs/
// Lien vers la banque d'image : https://drive.google.com/drive/folders/1qA-8ZMroFYy72y2pfmN4nWWo_82tpCod?usp=sharing


#include <stdio.h>
#include "image_ppm.h"
#include <iostream>
#include <cmath>
#include <filesystem>
#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <opencv2/opencv.hpp>
#include <omp.h>
#include <unordered_set>

namespace fs = std::filesystem;
int DIMENSION_IMAGETTE = 128;
int DIMENSION_CRITERE = 4;
const std::string DossierSortie = "imageTP";  // Dossier de sortie global
const std::string ImageLue = "in/Lena.pgm";
const std::string ImageEcrite = "Lena";
int TAILLE_BASE = 141988;

void afficherProgression(int current, int total) {
    int largeur = 50; // Largeur de la barre
    int progress = (current * largeur) / total;

    std::cout << "[";
    for (int i = 0; i < largeur; i++) {
        if (i < progress)
            std::cout << "=";
        else
            std::cout << " ";
    }
    std::cout << "] " << (current * 100) / total << "%\r";
    std::cout.flush();
}


// Fonction pour convertir et redimensionner toutes les images de la banque : 
// O(N*M)
// N = nombre d'images ; M = taille d'une image
void convertir_ppm(const std::string& source_folder, const std::string& destination_folder, int size = DIMENSION_IMAGETTE) {
    if (fs::exists(destination_folder)) {
        std::cout << "Le dossier " << destination_folder << " existe déjà. Conversion ignorée." << std::endl;
        return;
    }
    if (!fs::exists(source_folder)) {
        std::cout << "Le dossier " << source_folder << " n'existe pas. Téléchargez-le ici : "
                  << "https://drive.google.com/drive/folders/1qA-8ZMroFYy72y2pfmN4nWWo_82tpCod?usp=sharing" << std::endl;
        return;
    }

    fs::create_directories(destination_folder);
    std::vector<std::string> images;

    // Récupération des chemins d'images à convertir
    for (const auto& entry : fs::recursive_directory_iterator(source_folder)) {
        if (entry.is_regular_file()) {
            std::string ext = entry.path().extension().string();
            if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tiff") {
                images.push_back(entry.path().string());
            }
        }
    }

    int cpt = 0;

    // Traitement des images en parallèle si OpenMP est activé
    #pragma omp parallel for schedule(dynamic) reduction(+:cpt)
    for (size_t i = 0; i < images.size(); ++i) {
        const std::string& img_path = images[i];

        // Chargement de l'image en couleur
        cv::Mat img = cv::imread(img_path, cv::IMREAD_COLOR);
        if (img.empty()) continue;

        // Redimensionnement
        cv::resize(img, img, cv::Size(size, size));

        // Conversion BGR -> RGB
        cv::Mat img_rgb;
        cv::cvtColor(img, img_rgb, cv::COLOR_BGR2RGB);

        // Définition du chemin de sortie
        fs::path rel_path = fs::relative(img_path, source_folder);
        fs::path save_path = fs::path(destination_folder) / rel_path.replace_extension(".ppm");

        // Création du dossier cible si nécessaire
        fs::create_directories(save_path.parent_path());

        // Sauvegarde de l'image en PPM format (binaire P6)
        std::ofstream out(save_path.string(), std::ios::binary);
        if (!out) {
            std::cerr << "Erreur lors de l'écriture du fichier : " << save_path << std::endl;
            continue;
        }

        // Écriture de l'en-tête PPM
        out << "P6\n" << img.cols << " " << img.rows << "\n255\n";
        
        // Écriture des données de l'image
        out.write(reinterpret_cast<const char*>(img_rgb.data), img_rgb.total() * img_rgb.elemSize());
        out.close();

        cpt++;
        afficherProgression(cpt,images.size());
    }

    std::cout << "Conversion terminée ! " << cpt << " images créées !" << std::endl;
    TAILLE_BASE = cpt;
}



// Fonction permettant d'enregistrer les critères de chaque image dans un fichier :
// O(N)
// N = taille de base_images
void sauver_base_Image(const std::map<std::string, std::vector<double>>& base_images, const std::string& fichier){
	std::ofstream out(fichier);
    if (!out) {
        std::cerr << "Erreur d'ouverture du fichier pour la sauvegarde." << std::endl;
        return;
    }
    for (const auto& pair : base_images) {
        out << pair.first << " " << pair.second[0] << " " << pair.second[1] << " " << pair.second[2] << std::endl;
    }
    std::cout << "Base d'images sauvegardée dans : " << fichier << std::endl;
}

//Méthode pour charger les données enregistrées précédemment : 
//O(N log N)
//N = taille de base_images
std::map<std::string, std::vector<double>> charger_base_images_depuis_fichier_ppm(const std::string& fichier) {
    std::map<std::string, std::vector<double>> base_images;
    std::ifstream in(fichier);

    if (!in) {
        std::cerr << "Erreur d'ouverture du fichier pour la lecture : " << fichier << std::endl;
        return base_images;
    }

    std::string line;
    while (std::getline(in, line)) {
        std::istringstream iss(line);
        std::string path;
        double moyenne_rouge, moyenne_vert, moyenne_bleu;

        if (iss >> path >> moyenne_rouge >> moyenne_vert >> moyenne_bleu) {
            base_images[path] = {moyenne_rouge, moyenne_vert, moyenne_bleu};
        } else {
            std::cerr << "Format incorrect dans le fichier : " << line << std::endl;
        }
    }

    std::cout << "Base d'images chargée depuis : " << fichier << " (" << base_images.size() << " images)" << std::endl;
    return base_images;
}

// Méthode pour découper l'image en blocs de taille choisie : 
// O(nH*nW)
void decoupe(OCTET*Imgin,OCTET*ImgOut,int nH,int nW,int size){
	int nTaille = nH * nW;
    int nTaille3 = nTaille*3;
	if(nH%size!=0 || nW%size!=0){
		std::cout << "Erreur : les dimensions de l'image ne sont pas un multiple des dimensions de la mosaique" << std::endl;
		return;
	}
	for (int i=0; i < nH; i+=size){
        for(int j=0;j<nW*3;j+=size*3){
			int moyenneR = 0; int moyenneG = 0; int moyenneB = 0;
			for (int k=0; k < size; k++){
				for (int l=0; l < size*3; l+=3){
					moyenneR += Imgin[(i+k)*nW*3+j+l];
                    moyenneG += Imgin[(i+k)*nW*3+j+l+1];
                    moyenneB += Imgin[(i+k)*nW*3+j+l+2];
				}
			}
			moyenneR /= size*size; moyenneG /= size*size; moyenneB/= size*size;
			for (int k=0; k < size; k++){
				for (int l=0; l < size*3; l+=3){
					ImgOut[(i+k)*nW*3+j+l] = moyenneR;
                    ImgOut[(i+k)*nW*3+j+l+1] = moyenneG;
                    ImgOut[(i+k)*nW*3+j+l+2] = moyenneB;
				}
			}
		}
    }
}

//Calcul du critère moyenneur :
// O(nH*nW)
std::vector<double> critere_img_mean_ppm(OCTET* ImgIn,int nH,int nW){
	double moyR = 0; double moyG =0; double moyB = 0;
	int nTaille = nH*nW;
    int nTaille3 = nTaille*3;
	for(int i=0;i<nTaille3;i+=3){
		moyR+= ImgIn[i];
        moyG+= ImgIn[i+1];
        moyB+= ImgIn[i+2];
	}
	return {moyR/nTaille,moyG/nTaille,moyB/nTaille};
}

// Fonction pour calculer la base d'images et stocker leurs moyennes : 
// O(N log N)
std::map<std::string, std::vector<double>> charger_base_images_ppm(const std::string& dossier) {
    std::map<std::string, std::vector<double>> base_images;
	OCTET* ImgTmp;
    int cpt = 0;
	allocation_tableau(ImgTmp, OCTET,DIMENSION_IMAGETTE*DIMENSION_IMAGETTE*3);
    for (const auto& entry : fs::recursive_directory_iterator(dossier)) {
        if (entry.path().extension() == ".ppm") {
            lire_image_ppm(const_cast<char*>(entry.path().string().c_str()), ImgTmp, DIMENSION_IMAGETTE*DIMENSION_IMAGETTE);
			base_images[entry.path().string()] = critere_img_mean_ppm(ImgTmp,DIMENSION_IMAGETTE,DIMENSION_IMAGETTE);
        }
        cpt++;
        afficherProgression(cpt, TAILLE_BASE);
    }
    return base_images;
}

// Redimensionner l'image ImgTmp à une taille spécifiée (size x size) par sous-échantillonnage
void redimensionnerImage(OCTET* ImgTmp, OCTET* ImgResized, int tailleOriginale, int tailleRedimensionnee) {
    int blocTaille = tailleOriginale / tailleRedimensionnee; // Taille du bloc correspondant

    // Pour chaque pixel de l'image redimensionnée
    for (int y = 0; y < tailleRedimensionnee; ++y) {
        for (int x = 0; x < tailleRedimensionnee; ++x) {
            // Calculer les limites du bloc dans l'image d'origine
            int srcYStart = y * blocTaille;
            int srcXStart = x * blocTaille;
            int srcYEnd = (y + 1) * blocTaille;
            int srcXEnd = (x + 1) * blocTaille;

            int sommeRouge = 0, sommeVert = 0, sommeBleu = 0;
            int compteur = 0;

            // Somme les valeurs RGB des pixels dans le bloc
            for (int sy = srcYStart; sy < srcYEnd; ++sy) {
                for (int sx = srcXStart; sx < srcXEnd; ++sx) {
                    int idx = (sy * tailleOriginale + sx) * 3;
                    sommeRouge += ImgTmp[idx];       // Rouge
                    sommeVert += ImgTmp[idx + 1];    // Vert
                    sommeBleu += ImgTmp[idx + 2];    // Bleu
                    compteur++;
                }
            }

            // Calculer la moyenne des valeurs RGB du bloc
            int destIdx = (y * tailleRedimensionnee + x) * 3;
            ImgResized[destIdx] = sommeRouge / compteur;      // Rouge
            ImgResized[destIdx + 1] = sommeVert / compteur;   // Vert
            ImgResized[destIdx + 2] = sommeBleu / compteur;   // Bleu
        }
    }
}


// Méthode de génération de l'image mosaïque à partir des données calculées : 
//O( nH*nW*M / size^2 )
void generationImage_ppm(OCTET* ImgIn, OCTET* ImgOut, int nH, int nW, int size, std::map<std::string, std::vector<double>> base_images) {
    int nTaille = nH * nW;
    int nTaille3 = nTaille * 3;
    OCTET* ImgTmp;
    // Allocation mémoire pour ImgTmp (une imagette complète)
    allocation_tableau(ImgTmp, OCTET, DIMENSION_IMAGETTE * DIMENSION_IMAGETTE * 3);
    OCTET* ImgResized ;
    allocation_tableau(ImgResized,OCTET, size * size * 3);
    std::unordered_set<std::string> uset;

    // Boucles pour parcourir l'image d'entrée par blocs de 'size x size'
    for (int i = 0; i < nH; i += size) {
        for (int j = 0; j < nW; j += size) {
            int distanceMin = 256;
            std::string path = "";

            // Trouver l'image la plus proche en couleur moyenne
            for (const auto& pair : base_images) {
                if (uset.find(pair.first) == uset.end()) {
                    double distance = sqrt(
                        pow(pair.second[0] - ImgIn[(i * nW + j) * 3], 2) +
                        pow(pair.second[1] - ImgIn[(i * nW + j) * 3 + 1], 2) +
                        pow(pair.second[2] - ImgIn[(i * nW + j) * 3 + 2], 2)
                    );

                    if (distance < distanceMin) {
                        distanceMin = distance;
                        path = pair.first;
                    }

                    if (distanceMin == 0) {
                        break;
                    }
                }
            }
            uset.insert(path);

            // Lire l'image sélectionnée et stocker dans ImgTmp
            lire_image_ppm(const_cast<char*>(path.c_str()), ImgTmp, DIMENSION_IMAGETTE * DIMENSION_IMAGETTE);

            // Redimensionner l'imagette à la taille cible (size x size)
            redimensionnerImage(ImgTmp, ImgResized, DIMENSION_IMAGETTE, size);

            // Insérer l'image redimensionnée dans ImgOut à la position (i, j)
            for (int y = 0; y < size; y++) {
                for (int x = 0; x < size; x++) {
                    int imgOutIdx = ((i + y) * nW + (j + x)) * 3;
                    int resizedIdx = (y * size + x) * 3;

                    // Récupérer les valeurs RGB et les insérer en RGB dans ImgOut
                    ImgOut[imgOutIdx] = ImgResized[resizedIdx];        // Rouge
                    ImgOut[imgOutIdx + 1] = ImgResized[resizedIdx + 1]; // Vert
                    ImgOut[imgOutIdx + 2] = ImgResized[resizedIdx + 2]; // Bleu
                }
            }
        }
    }

    // Libération de la mémoire
    free(ImgResized);
    free(ImgTmp);
}



// Génération de l'image mosaïque haute qualité : 
// O(nH*nW*M)
void generationImageHD(OCTET* ImgIn, int nH, int nW, int size, std::map<std::string, std::vector<double>> base_images) {
    int newnH = ((nH / size) * DIMENSION_IMAGETTE);
    int newnW = ((nW / size) * DIMENSION_IMAGETTE);

    OCTET* ImgTmp;
    OCTET* ImgOut;
    allocation_tableau(ImgTmp, OCTET, DIMENSION_IMAGETTE * DIMENSION_IMAGETTE * 3);
    allocation_tableau(ImgOut, OCTET, newnH * newnW * 3);
    std::unordered_set<std::string> uset;



    for (int i = 0; i < nH; i += size) {
        for (int j = 0; j < nW; j += size) {
            double minDistance = DBL_MAX;
            std::string bestMatchPath = "";

            // Recherche de l'image la plus proche en couleur
            for (const auto& pair : base_images) {
                if (uset.find(pair.first) == uset.end()) {
                    double dist = sqrt(
                        pow(pair.second[0] - ImgIn[(i * nW + j) * 3], 2) +
                        pow(pair.second[1] - ImgIn[(i * nW + j) * 3 + 1], 2) +
                        pow(pair.second[2] - ImgIn[(i * nW + j) * 3 + 2], 2)
                    );

                    if (dist < minDistance) {
                        minDistance = dist;
                        bestMatchPath = pair.first;
                    }
                }
            }
            
            // Chargement de l'imagette sélectionnée
            if (!bestMatchPath.empty()) {
                uset.insert(bestMatchPath);
                lire_image_ppm(const_cast<char*>(bestMatchPath.c_str()), ImgTmp, DIMENSION_IMAGETTE * DIMENSION_IMAGETTE);
                
                for (int y = 0; y < DIMENSION_IMAGETTE; y++) {
                    for (int x = 0; x < DIMENSION_IMAGETTE; x++) {
                        int srcIdx = (y * DIMENSION_IMAGETTE + x) * 3;
                        int destIdx = (((i / size) * DIMENSION_IMAGETTE + y) * newnW + ((j / size) * DIMENSION_IMAGETTE + x)) * 3;
                        
                        ImgOut[destIdx] = ImgTmp[srcIdx];       // R
                        ImgOut[destIdx + 1] = ImgTmp[srcIdx + 1]; // G
                        ImgOut[destIdx + 2] = ImgTmp[srcIdx + 2]; // B
                    }
                }
            }
        }
    }

    // Sauvegarde de l'image mosaïque en PPM
    ecrire_image_ppm("out/glorp_HD.ppm", ImgOut, newnH, newnW);

    // Libération de la mémoire
    free(ImgTmp);
    free(ImgOut);
}


// Main pour effectuer toutes les étapes de génération de l'image mosaïque :
// O(nH⋅nW⋅M)
int main(int argc, char* argv[])
{
	
	std::cout<<"Conversion des images : "<<std::endl;
	convertir_ppm("../base_images","out/imagette_ppm",DIMENSION_IMAGETTE);
	std::cout<<"Découpe de l'image"<<std::endl;
	int nH, nW, nTaille;

	OCTET *ImgIn,*ImgOut;

	lire_nb_lignes_colonnes_image_ppm("in/glorp128.ppm", &nH, &nW);
	nTaille = nH * nW; int nTaille3 = nTaille*3;
	allocation_tableau(ImgIn, OCTET, nTaille3);
	lire_image_ppm("in/glorp128.ppm", ImgIn, nH * nW);
	allocation_tableau(ImgOut, OCTET, nTaille3);

	decoupe(ImgIn, ImgOut, nH, nW, DIMENSION_CRITERE);
	ecrire_image_ppm("out/glorp_decoupe.ppm", ImgOut, nH, nW);
	std::cout<<"Calcul des critères des imagettes"<<std::endl;
	OCTET*ImgOut2;
	allocation_tableau(ImgOut2, OCTET, nTaille3);
	std::string fichier_base_images = "base_images_ppm.txt";
	std::map<std::string, std::vector<double>> base_images;
	if (fs::exists(fichier_base_images)) {
	 	base_images = charger_base_images_depuis_fichier_ppm(fichier_base_images);
	} else {
		base_images = charger_base_images_ppm("out/imagette_ppm");
 		sauver_base_Image(base_images, fichier_base_images);  // Sauvegarder pour la prochaine fois
	}		
	std::cout<<"Recherche des imagette et génération de l'image finale"<<std::endl;
	generationImage_ppm(ImgOut,ImgOut2,nH,nW,DIMENSION_CRITERE,base_images);
	ecrire_image_ppm("out/glorp_mosaique.ppm", ImgOut2, nH, nW);
    std::cout<<"Image mosaïque générée"<<std::endl;
	generationImageHD(ImgIn, nH,nW,DIMENSION_CRITERE,base_images);
	free(ImgIn);
	free(ImgOut);
	return 0;
} 