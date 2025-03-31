// Première version de l'implémentation du TP Image
// MacOS : brew install opencv
//     g++ base_code_mosaique.cpp -o base_code_mosaique $(pkg-config --cflags --libs opencv4) -std=c++17
//     ./base_code_mosaique
// Linux : sudo apt update
//     sudo apt upgrade
//     sudo apt install libopencv-dev
//     sudo apt install nlohmann-json3-dev
//     g++ base_code_mosaique.cpp -o mosaique $(pkg-config --cflags --libs opencv4) -std=c++17 -ljpeg -pthread
//     ./base_code_mosaique
// Les résultats sont disponibles dans le dossier ImageTP :
//     Image_mosaique.pgm : Image découpée en bloc
//     Image_best.pgm : Image mosaïque avec compression des imagettes
//     Image_HD.pgm : Image haute qualité avec imagettes sans compression

// Lien vers la banque d'image : https://drive.google.com/drive/folders/1qA-8ZMroFYy72y2pfmN4nWWo_82tpCod?usp=sharing
// lien pour le csv.h : https://github.com/ben-strasser/fast-cpp-csv-parser/blob/master/csv.h
// Lien vers la banque prête à l'emploi : https://filesender.renater.fr/?s=download&token=300ef385-64e3-481a-9cf5-e1baf1fc9430

#include <cstdio>
#include "image_ppm.h"
#include <iostream>
#include <cmath>
#include <filesystem>
#include <string>
#include <fstream>
#include <map>
#include <sstream>
#include <opencv2/opencv.hpp>
#include <unordered_set>
#include "../src/csv.h"
#include <chrono>
#include <thread>
#include <vector>

using namespace std;

// Paramètres globaux
int DIMENSION_IMAGETTE;
int TAILLE_BASE = 141988;
string CRITERE = "moyenne"; // Critère par défaut


//-------------------------------------------------------------------------------------------------------------
// ------------------------------------------- Fonctions génériques -------------------------------------------
//-------------------------------------------------------------------------------------------------------------

// Fonction pour afficher la progression de la conversion des images
void afficherProgression(int current, int total, chrono::steady_clock::time_point start_time) {
    int largeur = 50; // Largeur de la barre
    int progress = (current * largeur) / total;
    cout << "[";
    for (int i = 0; i < largeur; i++) {
        if (i < progress) cout << "=";
        else cout << " ";
    }
    cout << "] " << (current * 100) / total << "%";
    auto now = chrono::steady_clock::now();
    auto elapsed = chrono::duration_cast<chrono::seconds>(now - start_time).count();
    int remaining = (current > 0) ? ((elapsed * (total - current)) / current) : 0;
    int minutes = remaining / 60;
    int seconds = remaining % 60;
    if(seconds < 10) cout << " Temps restant estimé : " << minutes << "m 0" << seconds << "s\r";
    else cout << " Temps restant estimé : " << minutes << "m " << seconds << "s\r";
    cout.flush();
}

// Fonction pour convertir une image JPEG en PPM
void convertir_jpg_en_ppm(const string& chemin_jpg, const string& chemin_ppm,int size) {
    cv::Mat img = cv::imread(chemin_jpg, cv::IMREAD_COLOR);
    if (img.empty()) {
        cerr << "Erreur : Impossible de lire l'image " << chemin_jpg << endl;
        return;
    }
    cv::Mat img_resized;
    cv::resize(img,img_resized,cv::Size(size,size));
    if (!cv::imwrite(chemin_ppm, img_resized)) {
        cerr << "Erreur : Impossible d'écrire l'image " << chemin_ppm << endl;
        return;
    }
}

// Fonction pour convertir une image PPM en PGM
void convertir_ppm_pgm(OCTET* ImgIn, OCTET* ImgOut, int nH, int nW) {
    for (int i = 0; i < nH; i++) {
        for (int j = 0; j < nW; j++) {
            ImgOut[i * nW + j] = 0.299 * ImgIn[(i * nW + j) * 3] + 0.587 * ImgIn[(i * nW + j) * 3 + 1] + 0.114 * ImgIn[(i * nW + j) * 3 + 2];
        }
    }
}

//-------------------------------------------------------------------------------------------------------------
// ------------------------------------ Pré-traitement de la base d'images ------------------------------------
//-------------------------------------------------------------------------------------------------------------

void redimensionner_image(OCTET*& ImgIn, int& nH, int& nW) {
    int newH = ceil(nH / DIMENSION_IMAGETTE) * DIMENSION_IMAGETTE;
    int newW = ceil(nW / DIMENSION_IMAGETTE) * DIMENSION_IMAGETTE;
    if (newH == nH && newW == nW) {
        cout << "L'image est déjà un multiple de " << DIMENSION_IMAGETTE << "." << endl;
        return;
    }
    cout << "Redimensionnement de l'image pour être un multiple de " << DIMENSION_IMAGETTE << "." << endl;
    cout << "Dimensions originales : " << nH << "x" << nW << ", nouvelles dimensions : " << newH << "x" << newW << "." << endl;
    OCTET* ImgResized;
    allocation_tableau(ImgResized, OCTET, newH * newW * 3);
    for (int i = 0; i < newH; i++) {
        for (int j = 0; j < newW; j++) {
            ImgResized[(i * newW + j) * 3] = ImgIn[(i * nW + j) * 3];       // Rouge
            ImgResized[(i * newW + j) * 3 + 1] = ImgIn[(i * nW + j) * 3 + 1]; // Vert
            ImgResized[(i * newW + j) * 3 + 2] = ImgIn[(i * nW + j) * 3 + 2]; // Bleu
        }
    }
    free(ImgIn);
    ImgIn = ImgResized;
    nH = newH;
    nW = newW;
}

            //-------------------------------------------------------------------------------------
            // ------------------------------------ Partie PGM ------------------------------------
            //-------------------------------------------------------------------------------------

// Fonction permettant d'enregistrer les critères de chaque image dans un fichier :
void sauver_base_Image(const unordered_map<string, unordered_map<string, vector<double>>>& base_images, const string& fichier) {
    ofstream out(fichier);
    if (!out) {
        cerr << "Erreur d'ouverture du fichier pour la sauvegarde." << endl;
        return;
    }

    // Écrire les en-têtes
    out << "Chemin,Moyenne,Histogramme\n";

    // Écrire les données
    for (const auto& pair : base_images) {
        out << pair.first; // Chemin de l'image

        // Écrire la moyenne
        const auto& moyenne = pair.second.at("moyenne");
        if (!moyenne.empty()) {
            out << "," << moyenne[0];
        } else {
            out << ",0"; // Valeur par défaut si la moyenne est vide
        }

        // Écrire l'histogramme
        const auto& histogramme = pair.second.at("histogramme");
        out << ",";
        for (size_t i = 0; i < histogramme.size(); ++i) {
            out << histogramme[i];
            if (i < histogramme.size() - 1) {
                out << ";"; // Séparateur pour les valeurs de l'histogramme
            }
        }

        out << "\n"; // Nouvelle ligne pour chaque image
    }

    cout << "Base d'images sauvegardée dans : " << fichier << endl;
}

// Méthode pour charger les données enregistrées précédemment : 
unordered_map<string, unordered_map<string, vector<double>>> charger_base_images_depuis_fichier(const string& fichier) {
    unordered_map<string, unordered_map<string, vector<double>>> base_images;
    ifstream in(fichier);

    if (!in) {
        cerr << "Erreur d'ouverture du fichier pour la lecture : " << fichier << endl;
        return base_images;
    }

    string line;
    while (getline(in, line)) {
        istringstream iss(line);
        string path, histogramme_str;
        double moyenne;

        // Lire le chemin et la moyenne
        if (getline(iss, path, ',') && iss >> moyenne && getline(iss, histogramme_str, ',')) {
            // Convertir l'histogramme (séparé par des points-virgules) en vecteur de doubles
            vector<double> histogramme;
            istringstream hist_stream(histogramme_str);
            string hist_value;
            while (getline(hist_stream, hist_value, ';')) {
                histogramme.push_back(stod(hist_value));
            }

            // Ajouter les données dans la structure
            base_images[path]["moyenne"] = {moyenne};
            base_images[path]["histogramme"] = histogramme;
        } else {
            cerr << "Format incorrect dans le fichier : " << line << endl;
        }
    }

    cout << "Base d'images chargée depuis : " << fichier << " (" << base_images.size() << " images)" << endl;
    return base_images;
}

// Méthode pour découper l'image en blocs de taille choisie : 
void decoupe(OCTET*Imgin,OCTET*ImgOut,int nH,int nW,int size){
	if(nH%size!=0 || nW%size!=0){
		cout << "Erreur : les dimensions de l'image ne sont paun multiple des dimensions de la mosaique" << endl;
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

// //Calcul du critère moyenneur :
// int critere_img_mean_pgm(OCTET* ImgIn,int nH,int nW){
// 	int moy = 0;
// 	int nTaille = nH*nW;
//     int nTaille3 = nTaille*3;
// 	for(int i=0;i<nTaille3;i+=3){
// 		moy+= 0.299*ImgIn[i] + 0.587*ImgIn[i+1] + 0.114*ImgIn[i+2];
// 	}
// 	return moy/nTaille;
// }

//Calcul du critère moyenneur :
unordered_map<string, vector<double>> calculer_criteres_pgm(OCTET* ImgIn, int nH, int nW, double& moyenne, vector<double>& histogramme ) {
    moyenne = 0.0;
    histogramme.assign(256, 0.0);
    int nTaille = nH * nW;
    for (int i = 0; i < nTaille; i++) {
        int val = ImgIn[i];
        moyenne += val;
        histogramme[val]++;
    }
    moyenne /= nTaille;
    return {{"moyenne", {moyenne}}, {"histogramme", histogramme}};
}

// Fonction pour calculer la base d'images et stocker leurs moyennes : 
unordered_map<string,unordered_map<string, vector<double>>> charger_base_images(const string& dossier, int size) {
    unordered_map<string,unordered_map<string, vector<double>>>base_images;
    vector<filesystem::directory_entry> fichiers;

    // Collecter tous les fichiers PPM dans le dossier
    for (const auto& entry : filesystem::recursive_directory_iterator(dossier)) {
        if (entry.path().extension() == ".ppm") {
            fichiers.push_back(entry);
        }
    }

    // Nombre de threads à utiliser
    int num_threads = min(8u, thread::hardware_concurrency());
    cout << "Nombre de threads : " << num_threads << endl;

    // Diviser les fichiers en groupes pour chaque thread
    size_t total_fichiers = fichiers.size();
    size_t fichiers_par_thread = (total_fichiers + num_threads - 1) / num_threads;

    mutex mtx; // Mutex pour protéger l'accès à `base_images`
    mutex progress_mtx; // Mutex pour protéger l'affichage de la progression
    int cpt = 0; // Compteur partagé pour la progression
    auto start_time = chrono::steady_clock::now();

    auto traiter_fichiers = [&](size_t start, size_t end) {
        OCTET* ImgTmp;
        allocation_tableau(ImgTmp, OCTET, size * size * 3);

        for (size_t i = start; i < end; ++i) {
            const auto& entry = fichiers[i];
            string chemin = entry.path().string();

            // Lire l'image et calculer le critère
            lire_image_ppm(const_cast<char*>(chemin.c_str()), ImgTmp, size * size);
            double moyenne;
            vector<double> histogramme;
            auto critere = calculer_criteres_pgm(ImgTmp, size, size, moyenne, histogramme);

            // Protéger l'accès à `base_images` avec un mutex
            {
                lock_guard<mutex> lock(mtx);
                base_images[chemin] = critere;
            }

            // Mettre à jour la progression
            {
                lock_guard<mutex> progress_lock(progress_mtx);
                cpt++;
                afficherProgression(cpt, total_fichiers, start_time);
            }
        }

        free(ImgTmp);
    };

    // Lancer les threads
    vector<thread> threads;
    for (int t = 0; t < num_threads; ++t) {
        size_t start = t * fichiers_par_thread;
        size_t end = min(start + fichiers_par_thread, total_fichiers);
        if (start < end) {
            threads.emplace_back(traiter_fichiers, start, end);
        }
    }

    // Attendre la fin de tous les threads
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    cout << endl; // Pour terminer la ligne de progression proprement
    return base_images;
}


            //-------------------------------------------------------------------------------------
            // ------------------------------------ Partie PPM ------------------------------------
            //-------------------------------------------------------------------------------------

// Fonction permettant d'enregistrer les critères de chaque image dans un fichier :
// O(N)
// N = taille de base_images
void sauver_base_Image_ppm(const unordered_map<string, unordered_map<string, vector<double>>>& base_images, const string& fichier) {
    ofstream out(fichier);
    if (!out) {
        cerr << "Erreur : Impossible d'ouvrir le fichier " << fichier << " pour l'écriture." << endl;
        return;
    }

    // Écrire les en-têtes
    out << "Chemin,Moyenne_R,Moyenne_G,Moyenne_B,Histogramme_R,Histogramme_G,Histogramme_B\n";

    // Écrire les données
    for (const auto& pair : base_images) {
        out << pair.first; // Chemin de l'image

        // Vérifier et écrire les moyennes
        if (pair.second.find("moyenne") != pair.second.end() && pair.second.at("moyenne").size() == 3) {
            const auto& moyenne = pair.second.at("moyenne");
            out << "," << moyenne[0] << "," << moyenne[1] << "," << moyenne[2];
        } else {
            out << ",0,0,0"; // Valeurs par défaut si les moyennes sont manquantes
        }

        // Vérifier et écrire les histogrammes
        for (const string& hist_key : {"hist_R", "hist_G", "hist_B"}) {
            out << ",";
            if (pair.second.find(hist_key) != pair.second.end()) {
                const auto& histogramme = pair.second.at(hist_key);
                for (size_t i = 0; i < histogramme.size(); ++i) {
                    out << histogramme[i];
                    if (i < histogramme.size() - 1) {
                        out << ";"; // Séparateur pour les valeurs de l'histogramme
                    }
                }
            } else {
                out << "0"; // Valeur par défaut si l'histogramme est manquant
            }
        }

        out << "\n"; // Nouvelle ligne pour chaque image
    }

    cout << "Données sauvegardées dans le fichier : " << fichier << endl;
}
//Méthode pour charger les données enregistrées précédemment : 
//O(N log N)
//N = taille de base_images
unordered_map<string, unordered_map<string, vector<double>>> charger_base_images_depuis_fichier_ppm(const string& fichier) {
    unordered_map<string, unordered_map<string, vector<double>>> base_images;
    ifstream in(fichier);

    if (!in) {
        cerr << "Erreur d'ouverture du fichier pour la lecture : " << fichier << endl;
        return base_images;
    }

    string line;
    while (getline(in, line)) {
        istringstream iss(line);
        string path, hist_R_str, hist_G_str, hist_B_str;
        double moyenne_R, moyenne_G, moyenne_B;
        char delimiter;

        // Lire le chemin, les moyennes et les histogrammes
        if (getline(iss, path, ',') &&
            iss >> moyenne_R >> delimiter &&
            iss >> moyenne_G >> delimiter &&
            iss >> moyenne_B &&
            getline(iss, hist_R_str, ',') &&
            getline(iss, hist_G_str, ',') &&
            getline(iss, hist_B_str, ',')) {
            
            // Convertir les histogrammes (séparés par des points-virgules) en vecteurs de doubles
            vector<double> hist_R, hist_G, hist_B;
            istringstream hist_R_stream(hist_R_str), hist_G_stream(hist_G_str), hist_B_stream(hist_B_str);
            string hist_value;

            while (getline(hist_R_stream, hist_value, ';')) {
                hist_R.push_back(stod(hist_value));
            }
            while (getline(hist_G_stream, hist_value, ';')) {
                hist_G.push_back(stod(hist_value));
            }
            while (getline(hist_B_stream, hist_value, ';')) {
                hist_B.push_back(stod(hist_value));
            }

            // Ajouter les données dans la structure
            base_images[path]["moyenne"] = {moyenne_R, moyenne_G, moyenne_B};
            base_images[path]["hist_R"] = hist_R;
            base_images[path]["hist_G"] = hist_G;
            base_images[path]["hist_B"] = hist_B;
        } else {
            cerr << "Format incorrect dans le fichier : " << line << endl;
        }
    }

    cout << "Base d'images PPM chargée depuis : " << fichier << " (" << base_images.size() << " images)" << endl;
    return base_images;
}

void decoupe_ppm(OCTET* Imgin, OCTET* ImgOut, int nH, int nW, int size){
	if(nH%size!=0 || nW%size!=0){
		cout << "Erreur : les dimensions de l'image ne sont pas un multiple des dimensions de la mosaique" << endl;
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
// vector<double> critere_img_mean_ppm(OCTET* ImgIn,int nH,int nW){
// 	double moyR = 0; double moyG =0; double moyB = 0;
// 	int nTaille = nH*nW;
//     int nTaille3 = nTaille*3;
// 	for(int i=0;i<nTaille3;i+=3){
// 		moyR+= ImgIn[i];
//         moyG+= ImgIn[i+1];
//         moyB+= ImgIn[i+2];
// 	}
// 	return {moyR/nTaille,moyG/nTaille,moyB/nTaille};
// }

//Calcul du critère moyenneur :
// O(nH*nW)
unordered_map<string,vector<double>> critere_img_mean_ppm(OCTET* ImgIn,int nH,int nW){
    vector<double> moyenne;
    vector<double> hist_R;
    vector<double> hist_G;
    vector<double> hist_B;
	moyenne = {0.0,0.0,0.0};
    hist_R.assign(256,0.0);
    hist_G.assign(256,0.0);
    hist_B.assign(256,0.0);
	int nTaille = nH*nW;
    int nTaille3 = nTaille*3;
	for(int i=0;i<nTaille3;i+=3){
        int R = ImgIn[i];
        int G = ImgIn[i+1];
        int B = ImgIn[i+2];
		moyenne[0]+= R;
        moyenne[1]+= G;
        moyenne[2]+= B;
        hist_R[R]++;
        hist_G[G]++;
        hist_B[B]++;
	}
	for(int i=0;i<3;i++){
        moyenne[i] /= nTaille;
    }
    return {{"moyenne", moyenne}, {"hist_R", hist_R}, {"hist_G", hist_G}, {"hist_B", hist_B}};
}


// Fonction pour calculer la base d'images et stocker leurs moyennes :
unordered_map<string, unordered_map<string,vector<double>>> charger_base_images_ppm(const string& dossier,int size) {
    unordered_map<string, unordered_map<string,vector<double>>> base_images;
	OCTET* ImgTmp;
    int cpt = 0;
	allocation_tableau(ImgTmp, OCTET,size*size*3);
    auto start_time = chrono::steady_clock::now();
    for (const auto& entry : filesystem::recursive_directory_iterator(dossier)) {
        if (entry.path().extension() == ".ppm") {
            lire_image_ppm(const_cast<char*>(entry.path().string().c_str()), ImgTmp, size*size);
			base_images[entry.path().string()] = critere_img_mean_ppm(ImgTmp,size,size);
        }
        cpt++;
        afficherProgression(cpt, TAILLE_BASE, start_time);
    }
    return base_images;
}



// Génération de la base d'images à partir d'un dossier source
void convertir(const string& source_folder, const string& ppm_folder, int size) {
    if (filesystem::exists(ppm_folder)) {
        cout << "Le dossier " << ppm_folder << " existent déjà. Conversion ignorée." << endl;
        return;
    }
    int cpt = 0;
    filesystem::create_directories(ppm_folder);
    auto start_time = chrono::steady_clock::now();
    vector<thread> threads;
    int num_threads = min(8u, thread::hardware_concurrency());
    cout << "Nombre de threads : " << num_threads << endl;
    mutex mtx;
    condition_variable cv;
    int active_threads = 0;
    for (const auto& entry : filesystem::recursive_directory_iterator(source_folder)) {
        if (entry.is_regular_file()) {
            string extension = entry.path().extension().string();
            if (extension == ".jpg" || extension == ".jpeg") { // Vérifier l'extension
                unique_lock<mutex> lock(mtx);
                cv.wait(lock, [&] { return active_threads < num_threads; });
                active_threads++;
                threads.emplace_back([&, img_path = entry.path().string(), cpt]() {
                    convertir_jpg_en_ppm(img_path, ppm_folder + "/" + to_string(cpt) + ".ppm", size);
                    {
                        lock_guard lock(mtx);
                        active_threads--;
                    }
                    cv.notify_one();
                });
                cpt++;
                afficherProgression(cpt, TAILLE_BASE, start_time);
            } else {
                cerr << "Fichier ignoré (non JPEG) : " << entry.path() << endl;
            }
        }
    }
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    cout << "Conversion terminée ! " << cpt << " Images créées !" << endl;
    TAILLE_BASE = cpt;
}

void generer_base_imagettes(const string& dossier_source, const string& dossier_destination) {
    if(!filesystem::exists(dossier_source)){
        cout<<"Le dossier source n'existe pas."<<endl;
        return;
    }
    for(int i=2; i<=7 ;i++){
        int taille_imagette = pow(2,i);
        string dossier_sortie = dossier_destination + "/imagettes_" + to_string(taille_imagette);
        cout<<"Génération de la base pour les imagettes de taille : "<<taille_imagette<<endl;
        if(!filesystem::exists(dossier_sortie)){
            convertir(dossier_source, dossier_sortie, taille_imagette);
            filesystem::create_directories(dossier_sortie);
            string fichier_base_images = dossier_sortie + "/base_images_" + to_string(taille_imagette)+".csv";
            string fichier_base_images_ppm = dossier_sortie + "/base_images_ppm_" + to_string(taille_imagette)+".csv";
            auto base_images = charger_base_images(dossier_sortie , taille_imagette);
            sauver_base_Image(base_images, fichier_base_images);
            auto base_images_ppm = charger_base_images_ppm(dossier_sortie, taille_imagette);
            sauver_base_Image_ppm(base_images_ppm, fichier_base_images_ppm);
        }
    }
}

vector<int> choix_taille_imagette(OCTET* ImgIn, int nH, int nW){
    vector<int> tailles_imagettes;
    vector<int> tailles = {20,40,80};
    int nv_entree;
    for(int blocs : tailles){
        int taille_imagette = nW/blocs;
        nv_entree = max(4,min(128, static_cast<int>(pow(2,round(log2(taille_imagette))))));
        if(find(tailles_imagettes.begin(), tailles_imagettes.end(), nv_entree) == tailles_imagettes.end()){
            tailles_imagettes.push_back(nv_entree);
        }
    }
    return tailles_imagettes;
}

//-------------------------------------------------------------------------------------------------------------
// ------------------------------------------ Traitement de l'image -------------------------------------------
//-------------------------------------------------------------------------------------------------------------

            //-------------------------------------------------------------------------------------
            // ------------------------------------ Partie PGM ------------------------------------
            //-------------------------------------------------------------------------------------

// Méthode de génération de l'image mosaïque à partir des données calculées : 
void generationImage(OCTET* ImgIn, OCTET* ImgOut, int nH, int nW, int size, unordered_map<string,unordered_map<string, vector<double>>>& base_images) {
    int nTaille = nH * nW;
    OCTET* ImgTmp, *ImgTmp_pgm;
    allocation_tableau(ImgTmp, OCTET, DIMENSION_IMAGETTE * DIMENSION_IMAGETTE * 3);
    allocation_tableau(ImgTmp_pgm, OCTET, DIMENSION_IMAGETTE * DIMENSION_IMAGETTE);
    unordered_set<string> uset;
    auto start_time = chrono::steady_clock::now();
    for (int i = 0; i < nH; i += size) {
        for (int j = 0; j < nW; j += size) {
            int distanceMin = 256;
            string path;
            for (const auto& pair : base_images) {
                if (uset.find(pair.first) == uset.end()) {
                    if (abs(pair.second.at(CRITERE)[0] - ImgIn[i * nW + j]) < distanceMin) {
                        distanceMin = abs(pair.second.at(CRITERE)[0] - ImgIn[i * nW + j]);
                        path = pair.first;
                    }
                    if (distanceMin == 0) {
                        break;
                    }
                }
            }
            uset.insert(path);
            lire_image_ppm(const_cast<char*>(path.c_str()), ImgTmp, DIMENSION_IMAGETTE * DIMENSION_IMAGETTE);
            convertir_ppm_pgm(ImgTmp, ImgTmp_pgm, DIMENSION_IMAGETTE, DIMENSION_IMAGETTE);

            for (int y = 0; y < size; y++) {
                for (int x = 0; x < size; x++) {
                    if (i + y < nH && j + x < nW) {
                        ImgOut[(i + y) * nW + (j + x)] = ImgTmp_pgm[y * size + x];
                    }
                }
            }
        afficherProgression(i * nW + j, nTaille, start_time);
        }
    }
    free(ImgTmp);
}

            //-------------------------------------------------------------------------------------
            // ------------------------------------ Partie PPM ------------------------------------
            //-------------------------------------------------------------------------------------

// Méthode de génération de l'image mosaïque à partir des données calculées : 
//O( nH*nW*M / size^2 )
void generationImage_ppm(OCTET* ImgIn, OCTET* ImgOut, int nH, int nW, int size, unordered_map<string,unordered_map<string, vector<double>>>& base_images) {
    int nTaille = nH * nW;
    OCTET* ImgTmp;
    // Allocation mémoire pour ImgTmp (une imagette complète)
    allocation_tableau(ImgTmp, OCTET, DIMENSION_IMAGETTE * DIMENSION_IMAGETTE * 3);
    OCTET* ImgResized ;
    allocation_tableau(ImgResized,OCTET, size * size * 3);
    unordered_set<string> uset;
    auto start_time = chrono::steady_clock::now();
    // Boucles pour parcourir l'image d'entrée par blocs de 'size x size'
    for (int i = 0; i < nH; i += size) {
        for (int j = 0; j < nW; j += size) {
            int distanceMin = 256;
            string path;

            // Trouver l'image la plus proche en couleur moyenne
            for (const auto& pair : base_images) {
                if (uset.find(pair.first) == uset.end()) {
                    double distance = sqrt(
                        pow(pair.second.at(CRITERE)[0] - ImgIn[(i * nW + j) * 3], 2) +
                        pow(pair.second.at(CRITERE)[1] - ImgIn[(i * nW + j) * 3 + 1], 2) +
                        pow(pair.second.at(CRITERE)[2] - ImgIn[(i * nW + j) * 3 + 2], 2)
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

            // Insérer l'image redimensionnée dans ImgOut à la position (i, j)
            for (int y = 0; y < size; y++) {
                for (int x = 0; x < size; x++) {
                    int imgOutIdx = ((i + y) * nW + (j + x)) * 3;
                    int resizedIdx = (y * size + x) * 3;

                    // Récupérer les valeurs RGB et les insérer en RGB dans ImgOut
                    ImgOut[imgOutIdx] = ImgTmp[resizedIdx];        // Rouge
                    ImgOut[imgOutIdx + 1] = ImgTmp[resizedIdx + 1]; // Vert
                    ImgOut[imgOutIdx + 2] = ImgTmp[resizedIdx + 2]; // Bleu
                }
            }
            afficherProgression(i * nW + j, nTaille, start_time);
        }
    }

    // Libération de la mémoire
    free(ImgResized);
    free(ImgTmp);
}


//-------------------------------------------------------------------------------------------------------------
// -------------------------------------------- Qualité de l'image --------------------------------------------
//-------------------------------------------------------------------------------------------------------------

double EQM_pgm(OCTET* img1, OCTET* img2, int nH, int nW) {
    double eqm = 0.0;
    int nTaille = nH * nW;
    
    for (int i = 0; i < nTaille; i++) {
        eqm += pow(img1[i] - img2[i], 2);
    }
    
    eqm /= nH * nW * 3;
    return eqm;
    }
    
double PSNR_pgm(OCTET* img1, OCTET* img2, int nH, int nW) {
    double eqm = EQM_pgm(img1, img2, nH, nW);
    if (eqm == 0) {
        return INFINITY;  // Les images sont identiques
    }
    double psnr = 10 * log10(pow(255.0,2) / eqm);
    return psnr;
}

double EQM_ppm(OCTET* img1, OCTET* img2, int nH, int nW) {
    double eqm = 0.0;
    int nTaille = nH * nW * 3; // Chaque pixel a 3 canaux (R, G, B)
    
    for (int i = 0; i < nTaille; i++) {
        eqm += pow(img1[i] - img2[i], 2);
    }
    
    eqm /= nTaille; // Diviser par le nombre total de valeurs (R, G, B)
    return eqm;
}

double PSNR_ppm(OCTET* img1, OCTET* img2, int nH, int nW) {
    double eqm = EQM_ppm(img1, img2, nH, nW);
    if (eqm == 0) {
        return INFINITY;  // Les images sont identiques
    }
    double psnr = 10 * log10(pow(255.0, 2) / eqm);
    return psnr;
}


// Main pour effectuer toutes les étapes de génération de l'image mosaïque :
int main(int argc, char* argv[])
{
    OCTET *ImgIn, *ImgOut, *ImgIn_pgm, *ImgOut_pgm, *ImgOut2;
    int nH, nW, nTaille;
    string ImageLue, ImageEcrite;
    bool continuer = true;

    unordered_map<string,unordered_map<string, vector<double>>> base_images;
    unordered_map<string,unordered_map<string, vector<double>>> base_images_pgm;

    cout<<"Bienvenue dans cette aplication de génération d'image mosaïque"<<endl;
    cout<<"Veuillez patienter pendant le traitement des images"<<endl;
	filesystem::create_directories("out");
	cout<<"Conversion des images : "<<endl;
    auto start_time = chrono::steady_clock::now();
	generer_base_imagettes("../base_images","out/imagettes_ppm");
    auto elapsed = chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now() - start_time).count();
    int minutes = elapsed / 60;
    int seconds = elapsed % 60;
    cout<<"Conversion des images terminée en : "<< minutes << "m "<< seconds <<"s"<<endl;
    while(continuer==true){
        cout<<"Veuillez sélectionner une image à convertir en mosaïque :"<<endl;
        cin>>ImageLue;
        while(!filesystem::exists(ImageLue)){
            cout<<"L'image n'existe pas"<<endl;
            cout<<"Veuillez sélectionner une image à convertir en mosaïque :"<<endl;
            cin>>ImageLue;
        }
        ImageEcrite = ImageLue.substr(4,ImageLue.size()-8);
        lire_nb_lignes_colonnes_image_ppm(const_cast<char*>(ImageLue.c_str()), &nH, &nW);
        nTaille = nH * nW;
        allocation_tableau(ImgIn, OCTET, nTaille*3);
        lire_image_ppm(const_cast<char*>(ImageLue.c_str()), ImgIn, nH * nW);
        cout<<"Calcul des tailles d'imagettes recommandées"<<endl;
        vector<int> tailles_imagettes = choix_taille_imagette(ImgIn,nH,nW);
        cout<<"Les tailles d'imagettes recommandées sont : "<<endl;
        vector<string> tailles_imagettes_str = {"Imagettes hautes qualités : ","Imagettes moyennes qualités : ","Imagettes basse qualités : "};
        for(int i=0;i<tailles_imagettes.size();i++){
            cout<<tailles_imagettes_str[i]<<tailles_imagettes[i]<<endl;
        }
        cout<<"Veuillez choisir une taille d'imagette : "<<endl;
        cin>>DIMENSION_IMAGETTE;

        redimensionner_image(ImgIn, nH,nW);
        nTaille = nH * nW;
        ecrire_image_ppm(const_cast<char*>(("./out/"+ImageEcrite+"_redim.ppm").c_str()), ImgIn, nH, nW);
        allocation_tableau(ImgIn_pgm, OCTET, nTaille);
        allocation_tableau(ImgOut_pgm, OCTET, nTaille);
        allocation_tableau(ImgOut, OCTET, nTaille*3);
        allocation_tableau(ImgOut2, OCTET, nTaille*3);
        convertir_ppm_pgm(ImgIn, ImgIn_pgm, nH, nW);
        cout<<"Récupération des critères des imagettes"<<endl;
        string fichier_base_images = "out/imagettes_ppm/imagettes_"+to_string(DIMENSION_IMAGETTE)+"/base_images_"+to_string(DIMENSION_IMAGETTE)+".csv";
        string fichier_base_images_ppm = "out/imagettes_ppm/imagettes_"+to_string(DIMENSION_IMAGETTE)+"/base_images_ppm_"+to_string(DIMENSION_IMAGETTE)+".csv";

        if (filesystem::exists(fichier_base_images)) {
            base_images_pgm = charger_base_images_depuis_fichier(fichier_base_images);
        } else {
            base_images_pgm = charger_base_images(fichier_base_images, DIMENSION_IMAGETTE);
            sauver_base_Image(base_images_pgm, fichier_base_images);
        }
        if (filesystem::exists(fichier_base_images_ppm)) {
            base_images = charger_base_images_depuis_fichier_ppm(fichier_base_images_ppm);
        } else {
            base_images = charger_base_images_ppm(fichier_base_images_ppm, DIMENSION_IMAGETTE);
            sauver_base_Image_ppm(base_images, fichier_base_images_ppm);
        }
    
        cout<<"Découpe de l'image en PPM"<<endl;
        string image_decoupe = "out/"+ImageEcrite+"_decoupe.ppm";
        decoupe_ppm(ImgIn, ImgOut, nH, nW, DIMENSION_IMAGETTE);
        ecrire_image_ppm(const_cast<char*>(image_decoupe.c_str()), ImgOut, nH, nW);
        cout<<"Découpe de l'image en PGM"<<endl;
        std:string image_decoupe_pgm = "out/"+ImageEcrite+"_decoupe.pgm";
        decoupe(ImgIn_pgm, ImgOut_pgm, nH, nW, DIMENSION_IMAGETTE);
        ecrire_image_pgm(const_cast<char*>(image_decoupe_pgm.c_str()), ImgOut_pgm, nH, nW);
        
        cout<<"Génération de l'image PGM"<<endl;
        start_time = chrono::steady_clock::now();
        generationImage(ImgOut_pgm, ImgOut_pgm, nH, nW, DIMENSION_IMAGETTE, base_images_pgm);
        string image_mosaique_pgm = "out/"+ImageEcrite+"_mosaique.pgm";
        ecrire_image_pgm(const_cast<char*>(image_mosaique_pgm.c_str()), ImgOut_pgm, nH, nW);
        elapsed = chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now() - start_time).count();
        minutes = elapsed / 60;
        seconds = elapsed % 60;
        cout<<"Mosaique PGM générée en : "<< minutes << "m "<< seconds <<"s"<<endl;
        cout<<"Génération de l'image PPM"<<endl;
        start_time = chrono::steady_clock::now();
        string image_mosaique_ppm = "out/"+ImageEcrite+"_mosaique.ppm";
        generationImage_ppm(ImgOut, ImgOut2, nH, nW, DIMENSION_IMAGETTE, base_images);
        ecrire_image_ppm(const_cast<char*>(image_mosaique_ppm.c_str()), ImgOut2, nH, nW);
        elapsed = chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now() - start_time).count();
        minutes = elapsed / 60;
        seconds = elapsed % 60;
        cout<<"Mosaique PPM générée en : "<< minutes << "m "<< seconds <<"s"<<endl;

        cout<<"Calcul de la qualité de l'image PGM"<<endl;
        double eqm_pgm = EQM_pgm(ImgIn_pgm, ImgOut_pgm, nH, nW);
        double psnr_pgm = PSNR_pgm(ImgIn_pgm, ImgOut_pgm, nH, nW);
        cout<<"EQM : "<<eqm_pgm<<endl;
        cout<<"PSNR : "<<psnr_pgm<<endl;

        cout<<"Calcul de la qualité de l'image PPM"<<endl;
        double eqm_ppm = EQM_ppm(ImgIn, ImgOut2, nH, nW);
        double psnr_ppm = PSNR_ppm(ImgIn, ImgOut2, nH, nW);
        cout<<"EQM : "<<eqm_ppm<<endl;
        cout<<"PSNR : "<<psnr_ppm<<endl;

        cout<<"Voulez-vous continuer ? [O/n]"<<endl;
        string reponse;
        cin>>reponse;
        if(reponse=="n" || reponse=="N" || reponse=="non" || reponse=="Non"){
            continuer = false;
        }
    }
	free(ImgIn);
	free(ImgOut);

	return 0;
}
