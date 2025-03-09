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

namespace fs = std::filesystem;
int DIMENSION_IMAGETTE = 128;
int DIMENSION_CRITERE = 8;
const std::string DossierSortie = "imageTP";  // Dossier de sortie global
const std::string ImageLue = "imageTP/Lena.pgm";
const std::string ImageEcrite = "Lena";

// Fonction pour convertir et redimensionner toutes les images de la banque :
void convertir_pgm(std::string source_folder, std::string destination_folder, int size=DIMENSION_IMAGETTE){
	if (fs::exists(destination_folder)) {
		std::cout << "Le dossier " << destination_folder << " existe déjà. Conversion ignorée." << std::endl;
		return;
	}
	int cpt=0;
	fs::create_directories(destination_folder);

	for (const auto& entry : fs::recursive_directory_iterator(source_folder)) {
		if (entry.is_regular_file()) {
			std::string img_path = entry.path().string();
			std::string ext = entry.path().extension().string();
			if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tiff") {
				cv::Mat img = cv::imread(img_path, cv::IMREAD_GRAYSCALE); // Convertir en niveaux de gris
				if (!img.empty()) {
					cv::resize(img, img, cv::Size(size, size));
					std::string rel_path = fs::relative(entry.path(), source_folder).string();
					std::string save_path = destination_folder + "/" + rel_path.substr(0, rel_path.find_last_of('.')) + ".pgm";
					fs::create_directories(fs::path(save_path).parent_path());
					cv::imwrite(save_path, img);
				}
			}
		}
	cpt++;
}
std::cout << "Conversion terminée ! " <<cpt<<" Images créées !"<< std::endl;
}

// Fonction permettant d'enregistrer les critères de chaque image dans un fichier :
void sauver_base_Image(const std::map<std::string, double>& base_images, const std::string& fichier){
	std::ofstream out(fichier);
    if (!out) {
        std::cerr << "Erreur d'ouverture du fichier pour la sauvegarde." << std::endl;
        return;
    }
    for (const auto& pair : base_images) {
        out << pair.first << " " << pair.second << std::endl;
    }
    std::cout << "Base d'images sauvegardée dans : " << fichier << std::endl;
}

// Méthode pour charger les données enregistrées précédemment : 
std::map<std::string, double> charger_base_images_depuis_fichier(const std::string& fichier) {
    std::map<std::string, double> base_images;
    std::ifstream in(fichier);
    if (!in) {
        std::cerr << "Erreur d'ouverture du fichier pour la lecture." << std::endl;
        return base_images;
    }
    std::string line;
    while (std::getline(in, line)) {
        std::istringstream iss(line);
        std::string path;
        double moyenne;
        if (iss >> path >> moyenne) {
            base_images[path] = moyenne;
        }
    }
    std::cout << "Base d'images chargée depuis : " << fichier << std::endl;
    return base_images;
}

// Méthode pour découper l'image en blocs de taille choisie : 
void decoupe(OCTET*Imgin,OCTET*ImgOut,int nH,int nW,int size){
	int nTaille = nH * nW;
	if(nH%size!=0 || nW%size!=0){
		std::cout << "Erreur : les dimensions de l'image ne sont pas un multiple des dimensions de la mosaique" << std::endl;
		return;
	}
	for (int i=0; i < nH; i+=size){
		for (int j=0; j < nW; j+=size){
			int moyenne = 0;
			for (int k=0; k < size; k++){
				for (int l=0; l < size; l++){
					moyenne += Imgin[(i+k)*nW+j+l];
				}
			}
			moyenne /= size*size;
			for (int k=0; k < size; k++){
				for (int l=0; l < size; l++){
					ImgOut[(i+k)*nW+j+l] = moyenne;
				}
			}
		}
	}
}

//Calcul du critère moyenneur :
int critere_img_mean(OCTET* ImgIn,int nH,int nW){
	int moy = 0;
	int nTaille = nH*nW;
	for(int i=0;i<nTaille;i++){
		moy+= ImgIn[i];
	}
	return moy/nTaille;
}

// Fonction pour calculer la base d'images et stocker leurs moyennes : 
std::map<std::string, double> charger_base_images(const std::string& dossier) {
    std::map<std::string, double> base_images;
	OCTET* ImgTmp;
	allocation_tableau(ImgTmp, OCTET,DIMENSION_IMAGETTE*DIMENSION_IMAGETTE);
    for (const auto& entry : fs::recursive_directory_iterator(dossier)) {
        if (entry.path().extension() == ".pgm") {
			lire_image_pgm(const_cast<char*>(entry.path().string().c_str()), ImgTmp, DIMENSION_IMAGETTE*DIMENSION_IMAGETTE);
			base_images[entry.path().string()] = critere_img_mean(ImgTmp,DIMENSION_IMAGETTE,DIMENSION_IMAGETTE);
        }
    }
    return base_images;
}

// Méthode de génération de l'image mosaïque à partir des données calculées : 
void generationImage(OCTET* ImgIn, OCTET* ImgOut, int nH, int nW, int size,std::map<std::string, double> base_images ){
	int nTaille = nH*nW; OCTET* ImgTmp;
	cv::Mat ImgTmpMat(DIMENSION_IMAGETTE, DIMENSION_IMAGETTE, CV_8U, ImgTmp);
    cv::Mat ImgResized;
	allocation_tableau(ImgTmp,OCTET,DIMENSION_IMAGETTE*DIMENSION_IMAGETTE);
	for(int i=0;i<nH;i+=size){
		for(int j=0;j<nW;j+=size){
			int distanceMin = 256;
			std::string path = "";
			for (const auto& pair : base_images) {
				if (abs(pair.second-ImgIn[i*nW+j]) < distanceMin) {
					distanceMin = abs(pair.second-ImgIn[i*nW+j]);
					path = pair.first;
				}
				if(distanceMin == 0){
					break;
				}
			}
			lire_image_pgm(const_cast<char*>(path.c_str()),ImgTmp, DIMENSION_IMAGETTE*DIMENSION_IMAGETTE);
			// Convertir l'imagette en une matrice OpenCV
			ImgTmpMat = cv::Mat(DIMENSION_IMAGETTE, DIMENSION_IMAGETTE, CV_8U, ImgTmp);

			// Redimensionner l'imagette à la taille 'size*size'
			cv::resize(ImgTmpMat, ImgResized, cv::Size(size, size));

			// Insérer l'image redimensionnée dans ImgOut à la position (i, j)
			for (int y = 0; y < size; y++) {
				for (int x = 0; x < size; x++) {
					// S'assurer que l'on ne dépasse pas les dimensions de l'image de sortie
					if (i + y < nH && j + x < nW) {
						ImgOut[(i + y) * nW + (j + x)] = ImgResized.at<uchar>(y, x);
					}
				}
			}
		}
	}
}

// Génération de l'image mosaïque haute qualité : 
void generationImageHD(OCTET* ImgIn, int nH, int nW, int size,std::map<std::string, double> base_images){
	int nTaille = nH*nW; OCTET* ImgTmp,*ImgOut;
	int newnH = ((nH/size)*DIMENSION_IMAGETTE);
	int newnW = ((nW/size)*DIMENSION_IMAGETTE);
	allocation_tableau(ImgTmp,OCTET,DIMENSION_IMAGETTE*DIMENSION_IMAGETTE);
	allocation_tableau(ImgOut,OCTET,newnH*newnW);
	for(int i=0;i<nH;i+=size){
		for(int j=0;j<nW;j+=size){
			int distanceMin = 256;
			std::string path = "";
			for (const auto& pair : base_images) {
				if (abs(pair.second-ImgIn[i*nW+j]) < distanceMin) {
					distanceMin = abs(pair.second-ImgIn[i*nW+j]);
					path = pair.first;
				}
				if(distanceMin == 0){
					break;
				}
			}
			lire_image_pgm(const_cast<char*>(path.c_str()),ImgTmp, DIMENSION_IMAGETTE*DIMENSION_IMAGETTE);
			for (int y = 0; y < DIMENSION_IMAGETTE; y++) {
				for (int x = 0; x < DIMENSION_IMAGETTE; x++) {
					ImgOut[((i/size)*DIMENSION_IMAGETTE + y) * newnW + ((j/size)*DIMENSION_IMAGETTE + x)] = ImgTmp[y*DIMENSION_IMAGETTE+x];
				}
			}
		}
	}
	ecrire_image_pgm("imageTP/Lena_HD.pgm",ImgOut,newnH,newnW);
}

// Main pour effectuer toutes les étapes de génération de l'image mosaïque :
int main(int argc, char* argv[])
{
	std::cout<<"Conversion des images : "<<std::endl;
	convertir_pgm("images","images_pgm",DIMENSION_IMAGETTE);
	std::cout<<"Découpe de l'image"<<std::endl;
	int nH, nW, nTaille;

	OCTET *ImgIn,*ImgOut;

	lire_nb_lignes_colonnes_image_pgm("imageTP/Lena.pgm", &nH, &nW);
	nTaille = nH * nW;
		allocation_tableau(ImgIn, OCTET, nTaille);
		lire_image_pgm("imageTP/Lena.pgm", ImgIn, nH * nW);
		allocation_tableau(ImgOut, OCTET, nTaille);

		decoupe(ImgIn, ImgOut, nH, nW, DIMENSION_CRITERE);
		ecrire_image_pgm("imageTP/Lena_decoupe.pgm", ImgOut, nH, nW);
		std::cout<<"Calcul des critères des imagettes"<<std::endl;
		OCTET*ImgOut2;
		allocation_tableau(ImgOut2, OCTET, nTaille);
		std::string fichier_base_images = "base_images.txt";
		std::map<std::string, double> base_images;
		if (fs::exists(fichier_base_images)) {
			base_images = charger_base_images_depuis_fichier(fichier_base_images);
		} else {
			base_images = charger_base_images("images_pgm");
			sauver_base_Image(base_images, fichier_base_images);  // Sauvegarder pour la prochaine fois
		}		
		std::cout<<"Recherche des imagette et génération de l'image finale"<<std::endl;
		generationImage(ImgOut,ImgOut2,nH,nW,DIMENSION_CRITERE,base_images);
		ecrire_image_pgm("imageTP/Lena_mosaique.pgm", ImgOut2, nH, nW);
		generationImageHD(ImgIn, nH,nW,DIMENSION_CRITERE,base_images);
		free(ImgIn);
		free(ImgOut);
	return 0;
}
